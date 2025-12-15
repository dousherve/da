#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "da.h"

static const size_t DA_MIN_CAP = 4;
static const size_t DA_SHRINK_DIV = 4;

da_err_t	da_init(da_t *da, size_t init_cap, da_destructor_fn destroy)
{
	if (da == NULL)
		return (DA_ERR_NULL);

	*da = (da_t) {0};

	if (init_cap < DA_MIN_CAP)
		init_cap = DA_MIN_CAP;

	da->data = malloc(init_cap * sizeof(*da->data));
	if (da->data == NULL)
		return (DA_ERR_OOM);

	da->capacity = init_cap;
	da->destroy = destroy;

	return (DA_OK);
}

void	da_free(da_t *da)
{
	size_t	i;

	if (da == NULL)
		return;

	if (da->destroy)
		for (i = 0; i < da->size; ++i)
			da->destroy(da->data[i]);

	free(da->data);
	*da = (da_t) {0};
}

static da_err_t	da_ensure_capacity(da_t *da, size_t min_capacity)
{
	void	**new_data;
	size_t	new_capacity;

	if (min_capacity < DA_MIN_CAP)
		min_capacity = DA_MIN_CAP;

	if (da->capacity >= min_capacity)
		return (DA_OK);

	if (da->capacity < DA_MIN_CAP)
		new_capacity = DA_MIN_CAP;
	else
		new_capacity = da->capacity;

	while (new_capacity < min_capacity)
	{
		if (new_capacity > SIZE_MAX / 2)
			return (DA_ERR_OOM);
		new_capacity *= 2;
	}

	if (new_capacity > SIZE_MAX / sizeof(*da->data))
		return (DA_ERR_OOM);
	new_data = realloc(da->data, new_capacity * sizeof(*da->data));
	if (new_data == NULL)
		return (DA_ERR_OOM);

	da->data = new_data;
	da->capacity = new_capacity;

	return (DA_OK);
}

static void	da_try_shrink(da_t *da)
{
	void	**new_data;
	size_t	new_capacity;

	if (da->capacity > DA_MIN_CAP && da->size <= da->capacity / DA_SHRINK_DIV)
	{
		new_capacity = da->capacity / 2;
		if (new_capacity < DA_MIN_CAP)
			new_capacity = DA_MIN_CAP;

		new_data = realloc(da->data, new_capacity * sizeof(*da->data));
		if (new_data)
		{
			da->data = new_data;
			da->capacity = new_capacity;
		}
	}
}

/* Must call `da_init` first. Otherwise, behavior is undefined. */
da_err_t	da_push(da_t *da, void *elem)
{
	da_err_t	err;

	if (da == NULL)
		return (DA_ERR_NULL);

	err = da_ensure_capacity(da, da->size + 1);
	if (err != DA_OK)
		return (err);

	da->data[da->size++] = elem;
	return (DA_OK);
}

da_err_t	da_pop(da_t *da, void **out)
{
	if (da == NULL || out == NULL)
		return (DA_ERR_NULL);
	if (da->size == 0)
		return (DA_ERR_EMPTY);

	*out = da->data[--da->size];
	da_try_shrink(da);

	return (DA_OK);
}

da_err_t	da_insert(da_t *da, size_t index, void *elem)
{
	da_err_t	err;

	if (da == NULL)
		return (DA_ERR_NULL);
	if (index > da->size)
		return (DA_ERR_OOB);

	err = da_ensure_capacity(da, da->size + 1);
	if (err != DA_OK)
		return (err);

	if (index < da->size)
		memmove(
				da->data + index + 1,
				da->data + index,
				(da->size - index) * sizeof(*da->data)
		);

	da->data[index] = elem;
	++da->size;

	return (DA_OK);
}

da_err_t	da_remove(da_t *da, size_t index, void **out)
{
	if (da == NULL || out == NULL)
		return (DA_ERR_NULL);
	if (da->size == 0)
		return (DA_ERR_EMPTY);
	if (index >= da->size)
		return (DA_ERR_OOB);

	*out = da->data[index];

	if (index < da->size - 1)
		memmove(
				da->data + index,
				da->data + index + 1,
				(da->size - index - 1) * sizeof(*da->data)
		);

	--da->size;
	da_try_shrink(da);

	return (DA_OK);
}

da_err_t	da_get(const da_t *da, size_t index, void **out)
{
	if (da == NULL || out == NULL)
		return (DA_ERR_NULL);
	if (index >= da->size)
		return (DA_ERR_OOB);

	*out = da->data[index];

	return (DA_OK);
}

da_err_t	da_set(da_t *da, size_t index, void *elem)
{
	if (da == NULL)
		return (DA_ERR_NULL);
	if (index >= da->size)
		return (DA_ERR_OOB);

	if (da->destroy && da->data[index])
		da->destroy(da->data[index]);

	da->data[index] = elem;

	return (DA_OK);
}

size_t	da_size(const da_t *da)
{
	if (da == NULL)
		return (0);

	return (da->size);
}

size_t	da_capacity(const da_t *da)
{
	if (da == NULL)
		return (0);

	return (da->capacity);
}

