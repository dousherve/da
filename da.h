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
void	da_append_many(t_da *da, const void *elements, size_t count);
void	da_free(t_da *da);

// String builder
typedef struct s_da t_sb;

# define sb_init(sb) da_init((sb), sizeof(char))
# define sb_append(sb, str) da_append_many((sb), str, strlen(str))
# define sb_append_many(sb, ...) sb_append_many_null((sb), __VA_ARGS__, NULL)
# define sb_null_terminate(sb) da_append_many((sb), "", 1)
# define sb_free(sb) da_free((sb))

char	*sb_cstr(t_sb *sb);
void	sb_vappend_many_null(t_sb *sb, va_list ap);
void	sb_append_many_null(t_sb *sb, ...);

#endif
