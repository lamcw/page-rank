#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "rank.h"

// macro for dumping error messages
#ifndef DUMP_ERR
#define DUMP_ERR(ptr, str)				\
	if (ptr == NULL) {				\
		perror(str);				\
		exit(EXIT_FAILURE);			\
	}
#endif

typedef char *item_t;

struct rank {
	item_t *rank;
	int size;
	int max_size;
};

static void add_size(rank_t r);

// malloc a rank
rank_t new_rank(const int size)
{
	rank_t new = malloc(sizeof(struct rank));
	DUMP_ERR(new, "malloc failed");

	new->size = 0;
	new->max_size = size;
	new->rank = malloc(size * sizeof(item_t));
	DUMP_ERR(new->rank, "malloc failed");

	return new;
}

// doubles the size of rank
static void add_size(rank_t r)
{
	int new_size = 2 * r->max_size;
	item_t *tmp = realloc(r->rank, new_size * sizeof(item_t));
	DUMP_ERR(tmp, "realloc failed");
	r->rank = tmp;
	r->max_size = new_size;
}

// insert an item into rank
void insert_rank(rank_t r, char *item)
{
	assert(r && item);
	assert(r->size <= r->max_size);

	r->rank[r->size]= malloc(strlen(item) + 1);
	DUMP_ERR(r->rank[r->size], "malloc failed");
	strcpy(r->rank[r->size], item);
	r->size++;

	if (r->size >= r->max_size)
		add_size(r);
	assert(r->size <= r->max_size);
}

// return the largest size within @ranks
int max_size(int n, rank_t *ranks)
{
	int max = 0;
	for (int i = 0; i < n; i++)
		if (ranks[i]->size > max)
			max = ranks[i]->size;
	return max;
}

int rank_size(rank_t r)
{
	assert(r);
	return r->size;
}

void print_rank(rank_t r)
{
	assert(r);
	for (int i = 0; i < r->size; i++)
		puts(r->rank[i]);
}

static int _item_cmp(const void *a, const void *b)
{
	return strcmp(*(item_t *)a, *(item_t *)b);
}

void sort_rank(rank_t r)
{
	assert(r);
	qsort(r->rank, r->size, sizeof(item_t), _item_cmp);
}

void free_rank(rank_t r)
{
	assert(r);
	for (int i = 0; i < r->max_size; i++)
		free(r->rank[i]);
	free(r->rank);
	free(r);
}

int in_rank(rank_t r, char *item)
{
	assert(r);
	for (int i = 0; i < r->size; i++)
		if (strcmp(r->rank[i], item) == 0)
			return 1;
	return 0;
}

rank_t merge_ranks(rank_t *ranks, int nranks)
{
	rank_t merged = new_rank(max_size(nranks, ranks));
	
	for (int i = 0; i < nranks; i++) {
		for (int j = 0; j < ranks[i]->size; j++) {
			if (!in_rank(merged, ranks[i]->rank[j]))
				insert_rank(merged, ranks[i]->rank[j]);
		}
	}

	sort_rank(merged);
	return merged;
}
