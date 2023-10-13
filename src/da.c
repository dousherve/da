#include "da.h"

#include <stdio.h>
#include <string.h>

void	data_realloc(void **data, size_t element_size, size_t *len, size_t *capacity)
{
	if ((*len) * element_size >= (*capacity))
	{
		*capacity = (*capacity == 0) ? DA_INIT_CAP : ((*capacity) * 2);
		*data = realloc(*data, (*capacity) * element_size);
		if (!(*data))
		{
			perror("Failed to (re)alloc dynamic array");
			exit(1);
		}
	}
}

static void	da_realloc(t_da *da)
{
	data_realloc(&da->data, da->element_size, &da->len, &da->capacity);
}

t_da	*da_init(size_t size)
{
	t_da	*da;

	da = malloc(sizeof(t_da));
	if (!da)
	{
		perror("Failed to alloc t_da");
		return (NULL);
	}
	da->data = NULL;
	da->element_size = size;
	da->len = 0;
	da->capacity = 0;
	return (da);
}

void	da_append(t_da *da, const void *element)
{
	da_realloc(da);
	memcpy(da->data + (da->len * da->element_size), element, da->element_size);
	da->len++;
}

void	da_append_arr(t_da *da, const void *elements, size_t count)
{
	const char	*src;
	size_t	i;

	src = (const char *) elements;
	i = 0;
	while (i < count)
	{
		da_append(da, src + (i * da->element_size));
		i++;
	}
}

void	da_append_many_null(t_da *da, ...)
{
	va_list	ap;
	void	*element;

	va_start(ap, da);
	while ((element = va_arg(ap, void *)))
		da_append(da, element);
	va_end(ap);
}

void	da_pop(t_da *da)
{
	if (da->len > 0)
		da->len--;
}

void	*da_get(t_da *da, size_t index)
{
	if (index >= da->len)
		return (NULL);
	return (da->data + (index * da->element_size));
}

void	da_free(t_da *da)
{
	free(da->data);
	free(da);
}

void da_dump(t_da *da)
{
	printf("da: element_size = %zu, len = %zu, capacity = %zu\n",
		da->element_size, da->len, da->capacity);
}
