#include <stdio.h>
#include <string.h>

#include "../da.h"

typedef struct s_response
{
	int	a;
	int	b;
}	t_response;

void dump_da(t_da *da)
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

void dump_sb(t_sb *sb)
{
	char	*str;

	str = sb_cstr(sb);
	printf("Built \"%s\", len %zu\n", str, strlen(str));
	dump_da(sb);
}

int	main(int argc, char **argv)
{
	t_sb	sb;
	t_da	da;
	t_response r1 = {1, 2}, r2 = {3, 4}, r3 = {5, 6};

	sb_init(&sb);

	for (int i = 1; i < argc; ++i)
	{
		sb_append(&sb, argv[i]);
		if (i < argc - 1) sb_append(&sb, " ");
	}
	sb_null_terminate(&sb);
	dump_sb(&sb);

	printf("\n");

	da_init(&da, sizeof(t_response));
	da_append_many(&da, &r1, &r2, &r3);
	dump_da(&da);
	for (size_t i = 0; i < da.len; i++)
	{
		t_response *r = da_get(&da, i);
		printf("a = %d, b = %d\n", r->a, r->b);
	}
	
	da_pop(&da);
	dump_da(&da);

	sb_free(&sb);
	da_free(&da);
	return (0);
}
