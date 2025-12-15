/* tests/test_da.c
 *
 * This test suite targets the libda static library and its public API.
 *
 * Intended usage with your Makefile:
 *   make test
 *   make run
 *   make leaks      (macOS)
 *   make valgrind   (Linux)
 *
 * Notes:
 * - Tests are plain C (no external framework).
 * - Uses a counted destructor to verify ownership semantics.
 * - Includes stress + randomized model tests.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#include "da.h"

#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL:%s:%d: %s\n", __FILE__, __LINE__, #cond); \
        exit(1); \
    } \
} while (0)

#define CHECK_OK(expr) CHECK((expr) == DA_OK)
#define CHECK_ERR(expr, code) CHECK((expr) == (code))

/* ---------- Helpers ---------- */

typedef struct {
    size_t calls;
} dtor_counter_t;

static dtor_counter_t g_dtor = {0};

static void counted_free(void *p)
{
    g_dtor.calls++;
    free(p);
}

static int *heap_int(int v)
{
    int *p = (int *)malloc(sizeof(int));
    CHECK(p != NULL);
    *p = v;
    return p;
}

/* deterministic PRNG: xorshift32 */
static uint32_t rng_u32(uint32_t *state)
{
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

/* simple dynamic array model for randomized testing */
typedef struct {
    void  **data;
    size_t  size;
    size_t  cap;
} model_t;

static void model_init(model_t *m)
{
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

static void model_free(model_t *m)
{
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

static void model_ensure(model_t *m, size_t need)
{
    if (m->cap >= need) return;
    size_t new_cap = (m->cap == 0) ? 8 : m->cap;
    while (new_cap < need) {
        CHECK(new_cap <= SIZE_MAX / 2);
        new_cap *= 2;
    }
    void **p = (void **)realloc(m->data, new_cap * sizeof(*p));
    CHECK(p != NULL);
    m->data = p;
    m->cap = new_cap;
}

static void model_insert(model_t *m, size_t index, void *elem)
{
    CHECK(index <= m->size);
    model_ensure(m, m->size + 1);
    if (index < m->size) {
        memmove(m->data + index + 1,
                m->data + index,
                (m->size - index) * sizeof(*m->data));
    }
    m->data[index] = elem;
    m->size++;
}

static void *model_remove(model_t *m, size_t index)
{
    CHECK(m->size > 0);
    CHECK(index < m->size);
    void *out = m->data[index];
    if (index < m->size - 1) {
        memmove(m->data + index,
                m->data + index + 1,
                (m->size - index - 1) * sizeof(*m->data));
    }
    m->size--;
    return out;
}

static void *model_pop(model_t *m)
{
    CHECK(m->size > 0);
    void *out = m->data[m->size - 1];
    m->size--;
    return out;
}

/* ---------- Tests ---------- */

static void test_init_basic(void)
{
    da_t da;

    CHECK_ERR(da_init(NULL, 0, NULL), DA_ERR_NULL);

    CHECK_OK(da_init(&da, 0, NULL));
    CHECK(da_size(&da) == 0);
    CHECK(da_capacity(&da) >= 4);
    da_free(&da);

    CHECK_OK(da_init(&da, 1, NULL));
    CHECK(da_capacity(&da) >= 4);
    da_free(&da);

    CHECK_OK(da_init(&da, 4, NULL));
    CHECK(da_capacity(&da) == 4);
    da_free(&da);

    /* da_free(NULL) should be a no-op */
    da_free(NULL);
}

static void test_push_pop_get_set_bounds(void)
{
    da_t da;
    void *out = NULL;

    CHECK_OK(da_init(&da, 0, NULL));

    CHECK_ERR(da_pop(&da, &out), DA_ERR_EMPTY);
    CHECK_ERR(da_get(&da, 0, &out), DA_ERR_OOB);
    CHECK_ERR(da_set(&da, 0, (void *)"x"), DA_ERR_OOB);

    CHECK_OK(da_push(&da, (void *)"a"));
    CHECK_OK(da_push(&da, (void *)"b"));
    CHECK_OK(da_push(&da, (void *)"c"));

    CHECK_OK(da_get(&da, 0, &out)); CHECK(strcmp((const char *)out, "a") == 0);
    CHECK_OK(da_get(&da, 1, &out)); CHECK(strcmp((const char *)out, "b") == 0);
    CHECK_OK(da_get(&da, 2, &out)); CHECK(strcmp((const char *)out, "c") == 0);

    CHECK_ERR(da_get(&da, 3, &out), DA_ERR_OOB);
    CHECK_ERR(da_set(&da, 3, (void *)"z"), DA_ERR_OOB);

    CHECK_OK(da_set(&da, 1, (void *)"B"));
    CHECK_OK(da_get(&da, 1, &out)); CHECK(strcmp((const char *)out, "B") == 0);

    CHECK_OK(da_pop(&da, &out)); CHECK(strcmp((const char *)out, "c") == 0);
    CHECK_OK(da_pop(&da, &out)); CHECK(strcmp((const char *)out, "B") == 0);
    CHECK_OK(da_pop(&da, &out)); CHECK(strcmp((const char *)out, "a") == 0);
    CHECK_ERR(da_pop(&da, &out), DA_ERR_EMPTY);

    /* NULL argument tests */
    CHECK_ERR(da_push(NULL, (void *)"x"), DA_ERR_NULL);
    CHECK_ERR(da_pop(NULL, &out), DA_ERR_NULL);
    CHECK_ERR(da_pop(&da, NULL), DA_ERR_NULL);
    CHECK_ERR(da_get(NULL, 0, &out), DA_ERR_NULL);
    CHECK_ERR(da_get(&da, 0, NULL), DA_ERR_NULL);
    CHECK_ERR(da_set(NULL, 0, (void *)"x"), DA_ERR_NULL);

    da_free(&da);
}

static void test_destructor_on_set_and_free(void)
{
    da_t da;
    void *out = NULL;

    g_dtor.calls = 0;

    CHECK_OK(da_init(&da, 0, counted_free));

    CHECK_OK(da_push(&da, heap_int(10)));
    CHECK_OK(da_push(&da, heap_int(20)));
    CHECK_OK(da_push(&da, heap_int(30)));
    CHECK(g_dtor.calls == 0);

    /* overwrite index 1 -> destructor must run once */
    CHECK_OK(da_set(&da, 1, heap_int(999)));
    CHECK(g_dtor.calls == 1);

    /* pop returns ownership to caller; no destructor call */
    CHECK_OK(da_pop(&da, &out));
    CHECK(out != NULL);
    CHECK(g_dtor.calls == 1);
    free(out);

    /* free destroys remaining two elements */
    da_free(&da);
    CHECK(g_dtor.calls == 3);
}

static void test_insert_remove_deterministic(void)
{
    da_t da;
    void *out = NULL;

    CHECK_OK(da_init(&da, 0, NULL));

    /* insert into empty at 0 */
    CHECK_OK(da_insert(&da, 0, (void *)"a"));
    CHECK(da_size(&da) == 1);

    /* insert at end (index == size) */
    CHECK_OK(da_insert(&da, da_size(&da), (void *)"c")); /* [a, c] */
    CHECK_OK(da_insert(&da, 1, (void *)"b"));           /* [a, b, c] */
    CHECK(da_size(&da) == 3);

    CHECK_OK(da_get(&da, 0, &out)); CHECK(strcmp((const char *)out, "a") == 0);
    CHECK_OK(da_get(&da, 1, &out)); CHECK(strcmp((const char *)out, "b") == 0);
    CHECK_OK(da_get(&da, 2, &out)); CHECK(strcmp((const char *)out, "c") == 0);

    /* remove middle */
    CHECK_OK(da_remove(&da, 1, &out)); CHECK(strcmp((const char *)out, "b") == 0);
    CHECK(da_size(&da) == 2);
    CHECK_OK(da_get(&da, 0, &out)); CHECK(strcmp((const char *)out, "a") == 0);
    CHECK_OK(da_get(&da, 1, &out)); CHECK(strcmp((const char *)out, "c") == 0);

    /* remove front */
    CHECK_OK(da_remove(&da, 0, &out)); CHECK(strcmp((const char *)out, "a") == 0);
    CHECK(da_size(&da) == 1);

    /* remove last */
    CHECK_OK(da_remove(&da, 0, &out)); CHECK(strcmp((const char *)out, "c") == 0);
    CHECK(da_size(&da) == 0);

    /* remove from empty */
    CHECK_ERR(da_remove(&da, 0, &out), DA_ERR_EMPTY);

    /* bounds: insert index > size */
    CHECK_ERR(da_insert(&da, 1, (void *)"x"), DA_ERR_OOB);

    /* bounds: remove index >= size (non-empty) */
    CHECK_OK(da_insert(&da, 0, (void *)"x"));
    CHECK_ERR(da_remove(&da, 1, &out), DA_ERR_OOB);

    /* NULL arg tests */
    CHECK_ERR(da_insert(NULL, 0, (void *)"x"), DA_ERR_NULL);
    CHECK_ERR(da_remove(NULL, 0, &out), DA_ERR_NULL);
    CHECK_ERR(da_remove(&da, 0, NULL), DA_ERR_NULL);

    da_free(&da);
}

static void test_growth_and_shrink_sanity(void)
{
    da_t da;
    void *out = NULL;
    size_t cap0, cap_grown;

    CHECK_OK(da_init(&da, 4, NULL));
    cap0 = da_capacity(&da);
    CHECK(cap0 >= 4);

    /* force some growth */
    for (int i = 0; i < 2000; i++)
        CHECK_OK(da_push(&da, (void *)(uintptr_t)(i + 1)));

    cap_grown = da_capacity(&da);
    CHECK(cap_grown >= cap0);

    /* remove most elements; should allow shrink (not asserting exact capacity) */
    while (da_size(&da) > 10) {
        CHECK_OK(da_pop(&da, &out));
    }

    CHECK(da_capacity(&da) >= 4);
    CHECK(da_capacity(&da) <= cap_grown);

    /* drain */
    while (da_size(&da) > 0) {
        CHECK_OK(da_pop(&da, &out));
    }

    da_free(&da);
}

static void test_stress_1m_push_pop(void)
{
    da_t da;
    void *out = NULL;

    CHECK_OK(da_init(&da, 0, NULL));

    for (int i = 0; i < 1000000; i++)
        CHECK_OK(da_push(&da, heap_int(i)));

    CHECK(da_size(&da) == 1000000);

    for (int i = 999999; i >= 0; i--) {
        CHECK_OK(da_pop(&da, &out));
        CHECK(out != NULL);
        CHECK(*(int *)out == i);
        free(out);
    }

    CHECK(da_size(&da) == 0);
    da_free(&da);
}

static void test_randomized_model_insert_remove(void)
{
    da_t da;
    model_t m;
    uint32_t state = 0x12345678u;

    CHECK_OK(da_init(&da, 0, NULL));
    model_init(&m);

    for (int step = 0; step < 200000; step++) {
        uint32_t r = rng_u32(&state);

        /* Choose operation: insert/remove/pop/push mix */
        uint32_t op = r % 4;

        if (op == 0) {
            /* push */
            int *p = heap_int((int)(r & 0x7fffffff));
            CHECK_OK(da_push(&da, p));
            model_insert(&m, m.size, p);
        } else if (op == 1) {
            /* pop */
            void *out = NULL;
            if (m.size == 0) {
                CHECK_ERR(da_pop(&da, &out), DA_ERR_EMPTY);
            } else {
                CHECK_OK(da_pop(&da, &out));
                void *exp = model_pop(&m);
                CHECK(out == exp);
                free(out);
            }
        } else if (op == 2) {
            /* insert at random index */
            size_t idx = (m.size == 0) ? 0 : (size_t)(rng_u32(&state) % (m.size + 1));
            int *p = heap_int((int)(rng_u32(&state) & 0x7fffffff));
            CHECK_OK(da_insert(&da, idx, p));
            model_insert(&m, idx, p);
        } else {
            /* remove at random index */
            void *out = NULL;
            if (m.size == 0) {
                CHECK_ERR(da_remove(&da, 0, &out), DA_ERR_EMPTY);
            } else {
                size_t idx = (size_t)(rng_u32(&state) % m.size);
                CHECK_OK(da_remove(&da, idx, &out));
                void *exp = model_remove(&m, idx);
                CHECK(out == exp);
                free(out);
            }
        }

        /* invariant checks */
        CHECK(da_size(&da) == m.size);
        CHECK(da_capacity(&da) >= 4);

        /* occasionally validate full content via da_get */
        if ((step % 2000) == 0) {
            for (size_t i = 0; i < m.size; i++) {
                void *got = NULL;
                CHECK_OK(da_get(&da, i, &got));
                CHECK(got == m.data[i]);
            }
        }
    }

    /* drain: free remaining heap ints */
    while (m.size > 0) {
        void *out = NULL;
        CHECK_OK(da_pop(&da, &out));
        void *exp = model_pop(&m);
        CHECK(out == exp);
        free(out);
    }

    da_free(&da);
    model_free(&m);
}

/* Optional: destructor semantics for insert/remove (should NOT call destructor) */
static void test_insert_remove_no_destructor_calls(void)
{
    da_t da;
    void *out = NULL;

    g_dtor.calls = 0;
    CHECK_OK(da_init(&da, 0, counted_free));

    /* insert elements */
    CHECK_OK(da_insert(&da, 0, heap_int(1)));
    CHECK_OK(da_insert(&da, 1, heap_int(2)));
    CHECK_OK(da_insert(&da, 1, heap_int(3))); /* now [1,3,2] */
    CHECK(g_dtor.calls == 0);

    /* remove returns ownership; still no destructor call */
    CHECK_OK(da_remove(&da, 1, &out));
    CHECK(g_dtor.calls == 0);
    free(out);

    /* pop returns ownership; still no destructor call */
    CHECK_OK(da_pop(&da, &out));
    CHECK(g_dtor.calls == 0);
    free(out);

    CHECK_OK(da_pop(&da, &out));
    CHECK(g_dtor.calls == 0);
    free(out);

    /* da_free on empty should not call destructor */
    da_free(&da);
    CHECK(g_dtor.calls == 0);
}

int main(void)
{
    test_init_basic();
    test_push_pop_get_set_bounds();
    test_destructor_on_set_and_free();

    test_insert_remove_deterministic();
    test_insert_remove_no_destructor_calls();

    test_growth_and_shrink_sanity();
    test_randomized_model_insert_remove();
    test_stress_1m_push_pop();

    printf("All tests passed.\n");
    return 0;
}

