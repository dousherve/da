#ifndef DAP_H
# define DAP_H

# include "da.h"

// Dynamic array of pointers
typedef struct s_dap
{
	void	**ptrs;
	size_t	len;
	size_t	capacity;
}	t_dap;

t_dap	*dap_init();
void	dap_append(t_dap *dap, const void *ptr);
void	dap_append_arr(t_dap *dap, const void **ptrs, size_t count);
void	dap_append_many_null(t_dap *dap, ...);
void	*dap_get(t_dap *dap, size_t index);
void	*dap_pop(t_dap *dap);
void	dap_pop_free(t_dap *dap);
void	dap_free(t_dap *dap);
void	dap_dump(t_dap *dap);

# define dap_append_many(dap, ...) dap_append_many_null((dap), __VA_ARGS__, 0)

#endif
