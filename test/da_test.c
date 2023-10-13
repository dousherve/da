#include <stdio.h>
#include <string.h>

#include "../da.h"
#include "../dap.h"
#include "../sb.h"

typedef struct s_response
{
	int	a;
	int	b;
}	t_response;

int	main(int argc, char **argv)
{
	t_sb		*sb, *sb2;
	t_da		*da;
	t_dap		*dap;

	t_response	r1 = {1, 2}, r2 = {3, 4}, r3 = {5, 6};
	t_response	*mr1, *mr2;

	char	*popped;

	sb = sb_init();

	for (int i = 1; i < argc - 1; ++i)
	{
		sb_append(sb, argv[i]);
		if (i < argc - 2) sb_append(sb, " ");
	}
	sb_dump(sb);

	popped = sb_pop(sb);
	printf("\nPopped: \"%s\"\n\n", popped);
	free(popped);
	sb_dump(sb);
	sb_append(sb, argv[argc - 1]);
	printf("\nReplaced with: \"%s\"\n\n", argv[argc - 1]);
	sb_dump(sb);

	printf("\n\n==== sb2 ====\n\n");
	sb2 = sb_init();
	char *s = malloc(5);
	strncpy(s, "abcd", 5);
	sb_append_ptr(sb2, s);
	sb_dump(sb2);
	printf("\nAppending ' ' and 2 chars\n\n");
	sb_append(sb2, " ");
	sb_append_n(sb2, s, 2);
	sb_dump(sb2);

	printf("\n\n==== da ====\n\n");

	da = da_init(sizeof(t_response));
	da_append_many(da, &r1, &r2, &r3);
	da_dump(da);
	for (size_t i = 0; i < da->len; i++)
	{
		t_response *r = da_get(da, i);
		printf("a = %d, b = %d\n", r->a, r->b);
	}

	da_pop(da);
	printf("\n");
	da_dump(da);

	printf("\n\n==== dap ====\n\n");

	dap = dap_init();
	mr1 = malloc(sizeof(t_response));
	mr2 = malloc(sizeof(t_response));
	mr1->a = 4;
	mr1->b = 2;
	mr2->a = 69;
	mr2->b = 420;
	dap_append_many(dap, mr1, mr2);
	for (size_t i = 0; i < dap->len; i++)
	{
		t_response *r = dap_get(dap, i);
		printf("mr%zu: a = %d, b = %d\n", i + 1, r->a, r->b);
	}
	printf("\n");
	dap_pop_free(dap);
	for (size_t i = 0; i < dap->len; i++)
	{
		t_response *r = dap_get(dap, i);
		printf("mr%zu: a = %d, b = %d\n", i + 1, r->a, r->b);
	}

	printf("\nTest end. Freeing memory.\n");

	sb_free(sb);
	sb_free(sb2);
	da_free(da);
	dap_free(dap);
	return (0);
}
