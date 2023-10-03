#include "da.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void	da_init(t_da *da, size_t size)
{
	da->data = NULL;
	da->element_size = size;
	da->len = 0;
	da->capacity = 0;
}

void	da_append(t_da *da, const void *element)
{
	if (da->len >= da->capacity)
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

void	da_append_many(t_da *da, const void *elements, size_t count)
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

void	da_free(t_da *da)
{
	free(da->data);
	da->data = NULL;
	da->len = 0;
	da->capacity = 0;
}

char	*sb_cstr(t_sb *sb)
{
	char	*str;

	str = (char *) sb->data;
	if (str[sb->len - 1])
		sb_null_terminate(sb);
	return (str);
}

void	sb_append_many_null(t_sb *sb, ...)
{
	va_list		ap;
	const char	*str;

	va_start(ap, sb);
	while ((str = va_arg(ap, const char *)))
		sb_append(sb, str);
	va_end(ap);
}
