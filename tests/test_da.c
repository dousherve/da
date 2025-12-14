#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "da.h"

#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL:%s:%d: %s\n", __FILE__, __LINE__, #cond); \
        exit(1); \
    } \
} while (0)

#define CHECK_OK(expr) CHECK((expr) == DA_OK)
#define CHECK_ERR(expr, errcode) CHECK((expr) == (errcode))

/* ---------- Test helpers ---------- */

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

/* Simple deterministic PRNG (xorshift32) */
static unsigned rng_u32(unsigned *state)
{
    unsigned x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

/* ---------- Tests ---------- */

static void test_init_basic(void)
{
    da_t da;

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

    CHECK_ERR(da_init(NULL, 4, NULL), DA_ERR_NULL);
}

static void test_push_pop_small(void)
{
    da_t da;
    void *out = NULL;

    CHECK_OK(da_init(&da, 4, NULL));
    CHECK(da_capacity(&da) == 4);

    CHECK_OK(da_push(&da, (void *)"a"));
    CHECK_OK(da_push(&da, (void *)"b"));
    CHECK_OK(da_push(&da, (void *)"c"));
    CHECK_OK(da_push(&da, (void *)"d"));
    CHECK(da_size(&da) == 4);
    CHECK(da_capacity(&da) == 4);

    /* trigger grow */
    CHECK_OK(da_push(&da, (void *)"e"));
    CHECK(da_size(&da) == 5);
    CHECK(da_capacity(&da) == 8);

    CHECK_OK(da_pop(&da, &out));
    CHECK(strcmp((const char *)out, "e") == 0);
    CHECK(da_size(&da) == 4);

    CHECK_OK(da_pop(&da, &out));
    CHECK(strcmp((const char *)out, "d") == 0);
    CHECK(da_size(&da) == 3);

    da_free(&da);

    CHECK_ERR(da_pop(NULL, &out), DA_ERR_NULL);
    CHECK_ERR(da_pop(&da, NULL), DA_ERR_NULL); /* da is inert after free; still NULL out triggers first */
}

static void test_get_set_bounds(void)
{
    da_t da;
    void *out = NULL;

    CHECK_OK(da_init(&da, 0, NULL));
    CHECK_ERR(da_get(&da, 0, &out), DA_ERR_OOB);
    CHECK_ERR(da_set(&da, 0, (void *)"x"), DA_ERR_OOB);

    CHECK_OK(da_push(&da, (void *)"x"));
    CHECK_OK(da_push(&da, (void *)"y"));

    CHECK_OK(da_get(&da, 0, &out));
    CHECK(strcmp((const char *)out, "x") == 0);

    CHECK_OK(da_get(&da, 1, &out));
    CHECK(strcmp((const char *)out, "y") == 0);

    CHECK_ERR(da_get(&da, 2, &out), DA_ERR_OOB);
    CHECK_ERR(da_set(&da, 2, (void *)"z"), DA_ERR_OOB);

    CHECK_ERR(da_get(NULL, 0, &out), DA_ERR_NULL);
    CHECK_ERR(da_get(&da, 0, NULL), DA_ERR_NULL);
    CHECK_ERR(da_set(NULL, 0, (void *)"z"), DA_ERR_NULL);

    da_free(&da);
}

static void test_destructor_on_set_and_free(void)
{
    da_t da;
    void *out = NULL;

    g_dtor.calls = 0;

    CHECK_OK(da_init(&da, 0, counted_free));

    /* push heap-owned objects */
    CHECK_OK(da_push(&da, heap_int(10)));
    CHECK_OK(da_push(&da, heap_int(20)));
    CHECK_OK(da_push(&da, heap_int(30)));
    CHECK(da_size(&da) == 3);
    CHECK(g_dtor.calls == 0);

    /* overwrite index 1; should destroy old element exactly once */
    CHECK_OK(da_set(&da, 1, heap_int(999)));
    CHECK(g_dtor.calls == 1);

    /* pop returns ownership to caller; pop should NOT call destructor */
    CHECK_OK(da_pop(&da, &out));
    CHECK(out != NULL);
    CHECK(g_dtor.calls == 1);
    free(out); /* caller frees popped element */

    /* free should destroy remaining elements */
    da_free(&da);
    CHECK(g_dtor.calls == 3); /* old[1] destroyed + remaining 2 destroyed at free */

    /* da_free(NULL) should be a no-op */
    da_free(NULL);
}

static void test_shrink_behavior(void)
{
    da_t da;
    void *out = NULL;
    size_t cap_after_grow;

    CHECK_OK(da_init(&da, 4, NULL));

    /* grow to 64 capacity by pushing 65 elements */
    for (int i = 0; i < 65; i++)
        CHECK_OK(da_push(&da, (void *)(uintptr_t)(i + 1)));

    cap_after_grow = da_capacity(&da);
    CHECK(cap_after_grow >= 64);

    /* pop down to trigger shrink multiple times */
    while (da_size(&da) > 8) {
        CHECK_OK(da_pop(&da, &out));
        (void)out;
    }

    /* capacity should have shrunk (not necessarily exactly, but should be less than after growth) */
    CHECK(da_capacity(&da) < cap_after_grow);
    CHECK(da_capacity(&da) >= 4);

    /* pop to empty, ensure DA_ERR_EMPTY */
    while (da_size(&da) > 0) {
        CHECK_OK(da_pop(&da, &out));
    }
    CHECK_ERR(da_pop(&da, &out), DA_ERR_EMPTY);

    da_free(&da);
}

static void test_stress_1m_push_pop(void)
{
    da_t da;
    void *out = NULL;

    CHECK_OK(da_init(&da, 0, NULL));

    /* Push 1,000,000 heap ints; pop and free them (caller owns on pop) */
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

static void test_randomized_ops(void)
{
    da_t da;
    unsigned state = 0xC0FFEEu;

    /* We'll manage our own “shadow stack” to validate LIFO behavior. */
    size_t shadow_cap = 1024;
    size_t shadow_sz = 0;
    int **shadow = (int **)malloc(shadow_cap * sizeof(*shadow));
    CHECK(shadow != NULL);

    CHECK_OK(da_init(&da, 0, NULL));

    for (int step = 0; step < 200000; step++) {
        unsigned r = rng_u32(&state);

        if ((r & 1u) == 0u) {
            /* push */
            int *p = heap_int((int)(r & 0x7fffffff));

            CHECK_OK(da_push(&da, p));

            if (shadow_sz == shadow_cap) {
                shadow_cap *= 2;
                shadow = (int **)realloc(shadow, shadow_cap * sizeof(*shadow));
                CHECK(shadow != NULL);
            }
            shadow[shadow_sz++] = p;
        } else {
            /* pop */
            if (shadow_sz == 0) {
                void *out = NULL;
                CHECK_ERR(da_pop(&da, &out), DA_ERR_EMPTY);
            } else {
                void *out = NULL;
                CHECK_OK(da_pop(&da, &out));
                CHECK(out == shadow[shadow_sz - 1]);
                shadow_sz--;
                free(out);
            }
        }

        CHECK(da_size(&da) == shadow_sz);
        CHECK(da_capacity(&da) >= 4);
    }

    /* drain */
    while (shadow_sz > 0) {
        void *out = NULL;
        CHECK_OK(da_pop(&da, &out));
        CHECK(out == shadow[shadow_sz - 1]);
        shadow_sz--;
        free(out);
    }

    da_free(&da);
    free(shadow);
}

/* ---------- Main ---------- */

int main(void)
{
    test_init_basic();
    test_push_pop_small();
    test_get_set_bounds();
    test_destructor_on_set_and_free();
    test_shrink_behavior();
    test_randomized_ops();
    test_stress_1m_push_pop();

    printf("All tests passed.\n");
    return 0;
}

