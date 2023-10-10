#include <stdio.h>
#include <string.h>

#include "../da.h"

typedef struct s_response
{
	int	a;
	int	b;
}	t_response;

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
	sb_dump(&sb);

	printf("\n");

	da_init(&da, sizeof(t_response));
	da_append_many(&da, &r1, &r2, &r3);
	da_dump(&da);
	for (size_t i = 0; i < da.len; i++)
	{
		t_response *r = da_get(&da, i);
		printf("a = %d, b = %d\n", r->a, r->b);
	}
	printf("\n");

	da_pop(&da);
	da_dump(&da);

	sb_free(&sb);
	da_free(&da);
	return (0);
}
