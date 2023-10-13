#include "dap.h"

#include <stdio.h>

static void	dap_realloc(t_dap *dap)
{
	data_realloc((void *) &dap->ptrs, sizeof(void *), &dap->len, &dap->capacity);
}

t_dap	*dap_init()
{
	t_dap	*dap;

	dap = malloc(sizeof(t_dap));
	if (!dap)
	{
		perror("Failed to alloc t_dap");
		return (NULL);
	}
	dap->ptrs = NULL;
	dap->len = 0;
	dap->capacity = 0;
	return (dap);
}

void	dap_append(t_dap *dap, const void *ptr)
{
	dap_realloc(dap);
	dap->ptrs[dap->len++] = (void *) ptr;
}

void	dap_append_arr(t_dap *dap, const void **ptrs, size_t count)
{
	size_t	i;

	i = 0;
	while (i < count)
		dap_append(dap, ptrs[i++]);
}

void	dap_append_many_null(t_dap *dap, ...)
{
	va_list	ap;
	void	*ptr;

	va_start(ap, dap);
	while ((ptr = va_arg(ap, void *)))
		dap_append(dap, ptr);
	va_end(ap);
}

void	*dap_get(t_dap *dap, size_t index)
{
	return (dap->ptrs[index]);
}

void	*dap_pop(t_dap *dap)
{
	if (dap->len > 0)
		return (dap_get(dap, --dap->len));
	return (NULL);
}

void	dap_pop_free(t_dap *dap)
{
	free(dap_pop(dap));
}

void	dap_free(t_dap *dap)
{
	size_t	i;

	i = 0;
	while (i < dap->len)
		free(dap_get(dap, i++));
	free(dap);
}

void dap_dump(t_dap *dap)
{
	printf("dap: len = %zu, capacity = %zu\n", dap->len, dap->capacity);
}
