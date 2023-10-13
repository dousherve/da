#ifndef SB_H
# define SB_H

# include "da.h"

/*
	String Builder
	Though it is `typedef`'ed to a regular `struct s_da`,
	do NOT use the methods prefixed with `da_` with
	this type.
*/
typedef struct s_da t_sb;

# define sb_init(sb) da_init(sizeof(char *))
# define sb_get(sb, i) (*(char **) da_get((sb), (i)))

void	sb_append_ptr(t_sb *sb, const char *str);
void	sb_append(t_sb *sb, const char *str);
void	sb_append_n(t_sb *sb, const char *str, size_t n);
void	sb_append_arr(t_sb *sb, const char **strs, size_t count);
void	sb_append_many_null(t_sb *sb, ...);
char	*sb_pop(t_sb *sb);
void	sb_dump(t_sb *sb);
char	*sb_build(t_sb *sb);
void	sb_free(t_sb *sb);

# define sb_append_many(sb, ...) sb_append_many_null((sb), __VA_ARGS__, 0)

#endif
