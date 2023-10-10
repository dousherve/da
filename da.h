#ifndef DA_H
# define DA_H

# include <stdlib.h>
# include <stdarg.h>

# define DA_INIT_CAP 128

// Dynamic array
typedef struct s_da
{
	void	*data;
	size_t	element_size;
	size_t	len;
	size_t	capacity;
}	t_da;

void	da_init(t_da *da, size_t size);
void	da_append(t_da *da, const void *element);
void	da_append_arr(t_da *da, const void *elements, size_t count);
void	da_append_many_null(t_da *da, ...);
void	da_pop(t_da *da);
void	*da_get(t_da *da, size_t index);
void	da_free(t_da *da);
void	da_dump(t_da *da);

# define da_append_many(da, ...) da_append_many_null((da), __VA_ARGS__, 0)

// String builder
typedef struct s_da t_sb;

# define sb_init(sb) da_init((sb), sizeof(char))
# define sb_append(sb, str) da_append_arr((sb), str, strlen(str))
# define sb_append_many(sb, ...) da_append_many(sb, __VA_ARGS__)
# define sb_null_terminate(sb) da_append_arr((sb), "", 1)
# define sb_free(sb) da_free((sb))

char	*sb_cstr(t_sb *sb);
void	sb_dump(t_sb *sb);

#endif
