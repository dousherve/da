#include <stdio.h>
#include <string.h>

#include "../da.h"

void dump_sb(t_sb *sb)
{
	char	*str;

	str = sb_cstr(sb);
	printf("Built \"%s\", len %zu\n", str, strlen(str));
	printf("sb: element_size = %zu, len = %zu, capacity = %zu\n",
		sb->element_size, sb->len, sb->capacity);
	printf("Raw: ");
	for (size_t i = 0; i < sb->len; ++i)
		printf("%02X ", str[i]);
	printf("\n");
}

int	main(int argc, char **argv)
{
	t_sb	sb;

	sb_init(&sb);

	// sb_append(&sb, "test");
	// sb_append(&sb, "/");
	// sb_append(&sb, "file.txt");
	// sb_null_terminate(&sb);
	// dump_sb(&sb);

	// sb_append_many(&sb, "I", " ", "love", " ", "boobs");
	// sb_null_terminate(&sb);
	// dump_sb(&sb);

	for (int i = 1; i < argc; ++i)
	{
		sb_append(&sb, argv[i]);
		if (i < argc - 1) sb_append(&sb, " ");
	}
	sb_null_terminate(&sb);
	dump_sb(&sb);

	sb_free(&sb);
	return (0);
}
