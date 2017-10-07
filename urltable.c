#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "urltable.h"

// macro for dumping error messages
#ifndef DUMP_ERR
#define DUMP_ERR(ptr, str)				\
	if (ptr == NULL) {				\
		perror(str);				\
		exit(EXIT_FAILURE);			\
	}
#endif

struct _url {
	char *url;
	int count;	// url count
};

struct _url_table {
	url_t **table;
	int row;
	int *col_size;
};

static int count_url(urltable_t, char *);
static int get_nurl(urltable_t);
static int in_arr(url_t *, int, char *);

urltable_t new_table(int row)
{
	urltable_t t = malloc(sizeof(struct _url_table));
	DUMP_ERR(t, "malloc failed");

	t->row = row;
	t->col_size = malloc(row * sizeof(int));
	t->table = malloc(row * sizeof(url_t));

	for (int i = 0; i < row; i++) {
		t->col_size[i] = 0;
		t->table[i] = NULL;
	}

	return t;
}

// batch insert urls into table
void insert_many(urltable_t t, int row, char **url, int arr_size)
{
	assert(t);

	t->col_size[row] = arr_size;
	// malloc row
	t->table[row] = malloc(arr_size * sizeof(url_t));
	DUMP_ERR(t->table[row], "malloc failed");

	for (int i = 0; i < arr_size; i++) {
		// malloc column struct
		t->table[row][i] = malloc(sizeof(struct _url));
		DUMP_ERR(t->table[row][i], "malloc failed");

		// malloc url in table
		t->table[row][i]->url = malloc(strlen(url[i]) + 1);
		DUMP_ERR(t->table[row][i]->url, "malloc failed");

		// copy to table
		strcpy(t->table[row][i]->url, url[i]);
		t->table[row][i]->count = 0;
	}
}

void show_table(urltable_t t)
{
	assert(t);

	for (int i = 0; i < t->row; i++) {
		printf("%d: ", i);
		for (int j = 0; j < t->col_size[i]; j++) {
			printf("%s(%d) ", t->table[i][j]->url, t->table[i][j]->count);
		}
		putchar('\n');
	}
}

void free_table(urltable_t t)
{
	assert(t);

	for (int i = 0; i < t->row; i++) {
		for (int j = 0; j < t->col_size[i]; j++) {
			free(t->table[i][j]->url);
			free(t->table[i][j]);
		}
		free(t->table[i]);
	}
	free(t->table);
	free(t->col_size);
	free(t);
}

int _cmp(const void *a, const void *b)
{
	url_t *ia = (url_t *)a;
	url_t *ib = (url_t *)b;
	return strcmp((*ia)->url, (*ib)->url);
}

static int count_url(urltable_t t, char *url)
{
	assert(t);

	int count = 0;

	// make a dummy struct
	url_t key = malloc(sizeof(struct _url));
	DUMP_ERR(key, "malloc failed");
	key->url = malloc(strlen(url) + 1);
	DUMP_ERR(key->url, "malloc failed");
	strcpy(key->url, url);

	for (int i = 0; i < t->row; i++) {
		// pass the dummy struct to bsearch
		url_t *result = bsearch(&key, t->table[i], t->col_size[i],	\
					sizeof(url), _cmp);

		if (result)
			count++;
	}

	free(key->url);
	free(key);
	return count;
}

// find url count
void set_count(urltable_t t)
{
	assert(t);

	for (int i = 0; i < t->row; i++) {
		for (int j = 0; j < t->col_size[i]; j++) {
			url_t u = t->table[i][j];
			u->count = count_url(t, u->url);
		}
	}
}

static int get_nurl(urltable_t t)
{
	int count = 0;
	for (int i = 0; i < t->row; i++)
		count += t->col_size[i];
	return count;
}

static int in_arr(url_t *arr, int size, char *key)
{
	for (int i = 0; i < size; i++) {
		if (strcmp(arr[i]->url, key) == 0)
			return 1;
	}
	return 0;
}

int _count_cmp(const void *a, const void *b)
{
	// sort in descending order
	return (*(url_t *)b)->count - (*(url_t *)a)->count;
}

// merge all rows into one array
// 	- without duplicates
// 	- sorted by url count
url_t *table_to_arr(urltable_t t, int *size)
{
	*size = 0;
	assert(t);

	url_t *arr = malloc(get_nurl(t) * sizeof(url_t));
	DUMP_ERR(arr, "malloc failed");

	for (int i = 0; i < t->row; i++) {
		for (int j = 0; j < t->col_size[i]; j++) {
			url_t item = t->table[i][j];
			if (!in_arr(arr, *size, item->url)) {
				// make a copy
				arr[*size] = malloc(sizeof(struct _url));
				arr[*size]->url = malloc(strlen(item->url) + 1);
				strcpy(arr[*size]->url, item->url);
				arr[*size]->count = item->count;
				(*size)++;
			}
		}
	}

	// reduce size
	url_t *tmp = realloc(arr, *size * sizeof(url_t));
	DUMP_ERR(tmp, "realloc failed");
	arr = tmp;

	// sort arr by count
	qsort(arr, *size, sizeof(url_t), _count_cmp);
	return arr;
}

void print_arr(url_t *arr, int size)
{
	for (int i = 0; i < size; i++)
		printf("%s(%d) ", arr[i]->url, arr[i]->count);
}

void free_table_arr(url_t *arr, int size)
{
	for (int i = 0; i < size; i++) {
		free(arr[i]->url);
		free(arr[i]);
	}
	free(arr);
}
