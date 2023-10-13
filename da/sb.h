#ifndef SB_H
# define SB_H

# include "dap.h"

// String Builder
typedef struct s_dap t_sb;

void	sb_append(t_sb *sb, const char *str);
void	sb_append_n(t_sb *sb, const char *str, size_t n);
void	sb_append_arr(t_sb *sb, const char **strs, size_t count);
void	sb_append_many_null(t_sb *sb, ...);
char	*sb_build(t_sb *sb);
void	sb_dump(t_sb *sb);

# define sb_init() dap_init()
# define sb_get(sb, i) ((char *) dap_get((sb), (i)))
# define sb_append_ptr(sb, ptr) dap_append((sb), (ptr))
# define sb_pop(sb) ((char *) dap_pop((sb)))
# define sb_free(sb) dap_free((sb))
# define sb_pop_free(sb) dap_pop_free((sb))
# define sb_append_many(sb, ...) sb_append_many_null((sb), __VA_ARGS__, 0)

#endif
