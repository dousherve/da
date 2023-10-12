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

static void	da_realloc(t_da *da)
{
	if (da->len * da->element_size >= da->capacity)
	{
		da->capacity = (da->capacity == 0) ? DA_INIT_CAP : (da->capacity * 2);
		da->data = realloc(da->data, da->capacity * da->element_size);
		if (!da->data)
		{
			perror("Failed to (re)alloc dynamic array data");
			exit(1);
		}
	}
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

/*
	Append the `malloc`'ed string pointed by `ptr` to `sb`.
	You must NOT `free` it manually, as it will be freed
	automatically when calling `sb_free`.
*/
void	sb_append_ptr(t_sb *sb, const char *ptr)
{
	da_realloc(sb);
	((char **) sb->data)[sb->len++] = (char *) ptr;
}

// Append a copy of `str` to `sb`.
void	sb_append(t_sb *sb, const char *str)
{
	da_realloc(sb);
	((char **) sb->data)[sb->len++] = strdup(str);
}

// See https://opensource.apple.com/source/Libc/Libc-262/i386/gen/strncpy.c.auto.html
static char	*sb_strncpy(char *dst, const char *src, size_t n)
{
	if (n != 0)
	{
		char *d = dst;
		const char *s = src;

		do {
			if ((*d++ = *s++) == 0) {
				while (--n != 0)
					*d++ = 0;
				break;
			}
		} while (--n != 0);
	}
	return (dst);
}

// Append a string composed of at most `n` characters of `str`.
void	sb_append_n(t_sb *sb, const char *str, size_t n)
{
	char	*to_append;

	if (strlen(str) <= n)
		sb_append(sb, str);
	else
	{
		to_append = malloc(n + 1);
		if (!to_append)
		{
			perror("Failed to allocate string");
			return;
		}
		sb_strncpy(to_append, str, n);
		sb_append(sb, to_append);
		free(to_append);
	}
}

void	sb_append_arr(t_sb *sb, const char **strs, size_t count)
{
	size_t	i;

	i = 0;
	while (i < count)
		sb_append(sb, strs[i++]);
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

/*
	Pop the last string added.
	It is now your responsiblity to free the returned pointer.
*/
char	*sb_pop(t_sb *sb)
{
	char	*str;

	str = NULL;
	if (sb->len > 0)
	{
		str = sb_get(sb, sb->len - 1);
		da_pop(sb);
	}
	return (str);
}

static size_t	sb_len(t_sb *sb)
{
	size_t	len;
	size_t	i;

	len = 0;
	i = 0;
	while (i < sb->len)
		len += strlen(sb_get(sb, i++));
	return (len);
}

char	*sb_build(t_sb *sb)
{
	char	*res;
	char	*str;
	size_t	res_len;
	size_t	len;
	size_t	i;

	i = 0;
	res = malloc(sb_len(sb) + 1);
	if (!res)
	{
		perror("Failed to allocate string");
		return (NULL);
	}
	res_len = 0;
	i = 0;
	while (i < sb->len)
	{
		str = sb_get(sb, i++);
		len = strlen(str);
		strncpy(res + res_len, str, len);
		res_len += len;
	}
	res[res_len] = '\0';
	return (res);
}

void sb_dump(t_sb *sb)
{
	char	*str;

	str = sb_build(sb);
	printf("Built \"%s\", len %zu\n", str, strlen(str));
	free(str);
	da_dump(sb);
}

void	sb_free(t_sb *sb)
{
	size_t	i;

	i = 0;
	while (i < sb->len)
		free(sb_get(sb, i++));
	da_free(sb);
}
