# da — Dynamic Array implementation in C

`da` is a small, robust, reusable **dynamic array (vector-like) module written in C11**.  
It can be built as `libda.a`, but is intended to be vendored/compiled directly in most projects.  
It is designed as a learning project focused on **correct memory management, ownership discipline, and API design**, while still being suitable for use in real projects.

---

## Features

* Generic container (`void *` elements)
* Explicit ownership model via optional destructor callback
* Automatic resizing (grow and shrink)
* Insert and remove at arbitrary positions
* Safe, explicit error handling via error codes
* Debug- and release-ready build system
* Sanitizer- and leak-check friendly
* No global state, no macros in public API

---

## Public API Overview

```c
typedef enum {
    DA_OK = 0,
    DA_ERR_NULL,
    DA_ERR_OOM,
    DA_ERR_OOB,
    DA_ERR_EMPTY
} da_err_t;

typedef void (*da_destructor_fn)(void *elem);

typedef struct {
    void **data;
    size_t size;
    size_t capacity;
    da_destructor_fn destroy;
} da_t;
````

---

## Lifecycle

```c
da_err_t da_init(da_t *da, size_t initial_capacity,
                 da_destructor_fn destroy);
void     da_free(da_t *da);
```

* `da_init` must be called before any other operation.
* `initial_capacity == 0` is allowed and will be normalized internally.
* `da_free(NULL)` is a no-op.
* After `da_free`, the structure is reset to an inert, reusable state.

---

## Core Operations

```c
da_err_t da_push(da_t *da, void *elem);
da_err_t da_pop(da_t *da, void **out);

da_err_t da_insert(da_t *da, size_t index, void *elem);
da_err_t da_remove(da_t *da, size_t index, void **out);

da_err_t da_get(const da_t *da, size_t index, void **out);
da_err_t da_set(da_t *da, size_t index, void *elem);
```

* `da_push` appends an element to the end.
* `da_pop` removes the last element and returns ownership to the caller.
* `da_insert` inserts an element at the specified index (shifting elements as needed).
* `da_remove` removes the element at the specified index and returns ownership to the caller.
* `da_get` reads an element without modifying the array.
* `da_set` replaces an element in-place and calls the destructor on the old element if one is provided.

---

## Capacity Management

* Capacity grows by **×2** when full.
* Capacity shrinks by **÷2** when `size <= capacity / 4`.
* A minimum capacity is enforced internally.
* Shrinking is **best-effort**: failure to shrink does not cause an error.

---

## Utility Functions

```c
size_t da_size(const da_t *da);
size_t da_capacity(const da_t *da);
```

* If `da == NULL`, these return `0`.

---

## Ownership Rules

* The array stores raw pointers (`void *`).

* The array **does not own elements by default**.

* If a destructor callback is provided:

  * It is called when:

    * an element is overwritten via `da_set`
    * the array is destroyed via `da_free`

  * It is **not** called on:

    * `da_pop`
    * `da_remove`

* Ownership of elements returned by `da_pop` and `da_remove` transfers to the caller.

Correct ownership handling is the caller’s responsibility.

---

## Error Handling

All fallible operations return a `da_err_t` value.
No function sets `errno`.

Common errors:

* `DA_ERR_NULL` — invalid `NULL` pointer passed
* `DA_ERR_OOM` — allocation or resize failure
* `DA_ERR_OOB` — index out of bounds
* `DA_ERR_EMPTY` — pop/remove on empty array

---

## Build System

The project uses a Makefile with **Debug** and **Release** builds.

### Debug (default)

```sh
make
make test
make run
```

Includes:

* AddressSanitizer
* UndefinedBehaviorSanitizer
* Debug symbols

### Release

```sh
make BUILD=release
make BUILD=release test
```

Optimized, no sanitizers.

### Outputs

```text
build/
├── debug/
│   ├── lib/libda.a
│   └── bin/test_da
└── release/
    ├── lib/libda.a
    └── bin/test_da
```

---

## Memory Checking

* **macOS**

  ```sh
  make leaks
  ```

* **Linux**

  ```sh
  make valgrind
  ```

All tests are expected to pass with **zero leaks**.

---

## Tests

The test suite includes:

* Unit tests for all operations
* Destructor and ownership verification
* Insert/remove correctness tests
* Stress tests (1M operations)
* Randomized model-based testing
* Shrink/grow behavior validation

Tests are written in plain C with no external frameworks.

---

## Status

* ✔ Dynamic array core complete
* ✔ Insert/remove operations implemented
* ✔ Extensive test coverage with sanitizers and leak checks
* ✔ Level 2.5 of a structured C mastery roadmap complete
