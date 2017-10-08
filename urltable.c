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
	int *col_size;	// array to store column size of each row
};

static int count_url(urltable_t, char *);
static int get_nurl(urltable_t);

// malloc an urltable_t
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

// print table to stdout
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

	// search for @key in each row
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
			// if count is set, skip
			if (u->count == 0)
				u->count = count_url(t, u->url);
		}
	}
}

// count number of urls in the table
static int get_nurl(urltable_t t)
{
	int count = 0;
	for (int i = 0; i < t->row; i++)
		count += t->col_size[i];
	return count;
}

// check if @key is in @arr
int in_arr(url_t *arr, int size, char *key)
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

	int old_size = get_nurl(t);
	url_t *arr = malloc(old_size * sizeof(url_t));
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

	// reduce size of arr if @size is smaller than @old_size
	if (*size < old_size) {
		url_t *tmp = realloc(arr, *size * sizeof(url_t));
		DUMP_ERR(tmp, "realloc failed");
		arr = tmp;
	}

	// sort arr by count
	qsort(arr, *size, sizeof(url_t), _count_cmp);
	return arr;
}

// return a subarray from url_t array where count == urlcount
url_t *partition_arr(url_t *arr, int size, int urlcount, int *sub_arr_size)
{
	int start = -1;
	int end = -1;

	int i = 0;
	while (i < size) {
		// array starts here
		if (arr[i]->count == urlcount && start == -1)
			start = i;
		// array ends here
		if (arr[i]->count != urlcount && start >= 0 && end == -1) {
			end = i;
			break;
		}
		i++;
	}

	// count not found
	if (start == -1)
		return NULL;
	// index overruns (i.e. copy till the end of array)
	if (end == -1)
		end = size;

	*sub_arr_size = end - start;
	url_t *ret = malloc(*sub_arr_size * sizeof(url_t));
	memcpy(ret, &arr[start], *sub_arr_size * sizeof(url_t));
	return ret;
}

void print_arr(url_t *arr, int size)
{
	for (int i = 0; i < size; i++)
		printf("%s(%d) ", arr[i]->url, arr[i]->count);
	putchar('\n');
}

void free_table_arr(url_t *arr, int size)
{
	for (int i = 0; i < size; i++) {
		free(arr[i]->url);
		free(arr[i]);
	}
	free(arr);
}
