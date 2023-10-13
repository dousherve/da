#ifndef DA_H
# define DA_H

# include <stdlib.h>
# include <stdarg.h>

# define DA_INIT_CAP 8

void	data_realloc(void **data, size_t element_size, size_t *len, size_t *capacity);

// Dynamic array
typedef struct s_da
{
	void	*data;
	size_t	element_size;
	size_t	len;
	size_t	capacity;
}	t_da;

t_da	*da_init(size_t size);
void	da_append(t_da *da, const void *element);
void	da_append_arr(t_da *da, const void *elements, size_t count);
void	da_append_many_null(t_da *da, ...);
void	*da_get(t_da *da, size_t index);
void	da_pop(t_da *da);
void	da_free(t_da *da);
void	da_dump(t_da *da);

# define da_append_many(da, ...) da_append_many_null((da), __VA_ARGS__, 0)

#endif
