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
	t_sb	sb, sb2;
	t_da	da;
	t_response r1 = {1, 2}, r2 = {3, 4}, r3 = {5, 6};
	char	*popped;

	sb_init(&sb);

	for (int i = 1; i < argc - 1; ++i)
	{
		sb_append(&sb, argv[i]);
		if (i < argc - 2) sb_append(&sb, " ");
	}
	sb_dump(&sb);

	popped = sb_pop(&sb);
	printf("\nPopped: \"%s\"\n\n", popped);
	free(popped);
	sb_dump(&sb);
	sb_append(&sb, argv[argc - 1]);
	printf("\nReplaced with: \"%s\"\n\n", argv[argc - 1]);
	sb_dump(&sb);

	printf("\n\n==== sb2 ====\n\n");
	sb_init(&sb2);
	char *s = malloc(5);
	strncpy(s, "abcd", 5);
	sb_append_ptr(&sb2, s);
	sb_dump(&sb2);
	printf("\nAppending ' ' and 2 chars\n\n");
	sb_append(&sb2, " ");
	sb_append_n(&sb2, s, 2);
	sb_dump(&sb2);

	printf("\n\n==== da ====\n\n");

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
	sb_free(&sb2);
	da_free(&da);
	return (0);
}
