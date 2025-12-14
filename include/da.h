#ifndef DA_H
# define DA_H

# include <stddef.h>

typedef enum {
    DA_OK = 0,

    DA_ERR_NULL,		/* NULL pointer passed */
    DA_ERR_OOM,			/* malloc/realloc failure */
    DA_ERR_OOB,			/* out-of-bounds access */
    DA_ERR_EMPTY,		/* pop from empty array */
} da_err_t;

typedef void (*da_destructor_fn)(void *elem);

/* da does not take ownership of elem unless a destructor is provided */
typedef struct {
    void	**data;
    size_t	size;
    size_t	capacity;
    da_destructor_fn destroy;
} da_t;

/* Lifecycle */
da_err_t	da_init(da_t *da, size_t init_cap, da_destructor_fn destroy);
void		da_free(da_t *da);

/* Core operations */

/* Must call da_init first. Otherwise, behavior is undefined. */
da_err_t	da_push(da_t *da, void *elem);
da_err_t	da_pop(da_t *da, void **out);
da_err_t	da_get(const da_t *da, size_t index, void **out);
da_err_t	da_set(da_t *da, size_t index, void *elem);

/* Utilities */
size_t	da_size(const da_t *da);
size_t	da_capacity(const da_t *da);

#endif

