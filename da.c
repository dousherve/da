#include "da.h"

#include <stdio.h>
#include <string.h>

void	da_init(t_da *da, size_t size)
{
	da->data = NULL;
	da->element_size = size;
	da->len = 0;
	da->capacity = 0;
}

void	da_append(t_da *da, const void *element)
{
	if (da->len * da->element_size >= da->capacity)
	{
		da->capacity = (da->capacity == 0) ? DA_INIT_CAP : (da->capacity * 2);
		da->data = realloc(da->data, da->capacity * da->element_size);
		if (!da->data)
		{
			perror("Failed to (re)alloc dynamic array");
			exit(1);
		}
	}
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
	if (da->len < 1)
		return ;
	memset(da->data + (da->len - 1) * da->element_size, 0, da->element_size);
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
	da->data = NULL;
	da->len = 0;
	da->capacity = 0;
}

void da_dump(t_da *da)
{
	const char	*data;

	data = da->data;
	printf("da: element_size = %zu, len = %zu, capacity = %zu\n",
		da->element_size, da->len, da->capacity);
	printf("Raw: ");
	for (size_t i = 0; i < da->len * da->element_size; ++i)
	{
		printf("%02X", data[i]);
		if (i < da->len * da->element_size - 1 && (i + 1) % da->element_size == 0)
			printf(", ");
		else
			printf(" ");
	}
	printf("\n");
}

char	*sb_cstr(t_sb *sb)
{
	char	*str;

	str = (char *) sb->data;
	if (str[sb->len - 1])
		sb_null_terminate(sb);
	return (str);
}

void sb_dump(t_sb *sb)
{
	const char	*str;

	str = sb_cstr(sb);
	printf("Built \"%s\", len %zu\n", str, strlen(str));
	da_dump(sb);
}
