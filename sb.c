#include "sb.h"

#include <stdio.h>
#include <string.h>

static void	sb_realloc(t_sb *sb)
{
	data_realloc(&sb->data, sizeof(char *), &sb->len, &sb->capacity);
}

/*
	Append the `malloc`'ed string pointed by `ptr` to `sb`.
	You must NOT `free` it manually, as it will be freed
	automatically when calling `sb_free`.
*/
void	sb_append_ptr(t_sb *sb, const char *ptr)
{
	sb_realloc(sb);
	((char **) sb->data)[sb->len++] = (char *) ptr;
}

// Append a copy of `str` to `sb`.
void	sb_append(t_sb *sb, const char *str)
{
	sb_realloc(sb);
	((char **) sb->data)[sb->len++] = strdup(str);
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
		strncpy(to_append, str, n);
		to_append[n] = '\0';
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
