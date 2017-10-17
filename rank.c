#include <stdlib.h>
#include <stdio.h>
#include <math.h>
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

// macro for swapping 2 integers
// does not use extra memory
#ifndef SWAP
#define SWAP(x, y) { x = x + y; y = x - y; x = x - y; }
#endif

typedef char *item_t;

// wrapper around an array of char *
// with some extra attributes
struct rank {
	item_t *rank;		// basically an array of char *
	int size;		// current size of the @rank
	int max_size;		// size allocated for @rank
};

static void add_size(rank_t);

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
	for (int i = r->size; i < r->max_size; i++)
		r->rank[i] = NULL;
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
	// debug checks
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

// merge an array of ranks into 1 rank
// remove deuplicates and sorted
// items are unique
rank_t merge_ranks(rank_t *ranks, int nranks)
{
	rank_t merged = new_rank(max_size(nranks, ranks));
	
	for (int i = 0; i < nranks; i++) {
		for (int j = 0; j < ranks[i]->size; j++) {
			if (pos_in_rank(merged, ranks[i]->rank[j]) == -1)
				insert_rank(merged, ranks[i]->rank[j]);
		}
	}

	sort_rank(merged);
	return merged;
}

char *get_rank_item(rank_t r, int i)
{
	assert(r);
	return r->rank[i];
}

// find position of @item in rank @r
int pos_in_rank(rank_t r, char *item)
{
	assert(r);
	for (int i = 0; i < r->size; i++)
		if (strcmp(r->rank[i], item) == 0)
			return i;
	return -1;
}

// scaled-footrule distance for 1 item
double sfd(double p, rank_t r, char *item, double c_size)
{
	double p_agg_rank = p / c_size;
	double pos = pos_in_rank(r, item);
	if (pos != -1)
		// if found in rank
		return fabs((pos + 1) / (double) r->size - p_agg_rank);
	else
		return 0;
}

// calculate the sum of scaled-footrule distance for a given 'P' arrangment
double sfdsum(int *P, rank_t merged, rank_t *ranks, int nrank)
{
	double sum = 0;

	for (int i = 0; i < nrank; i++) {
		for (int j = 0; j < merged->size; j++)
			sum += sfd(P[j], ranks[i], merged->rank[j], merged->size);
	}

	return sum;
}

// permute position and find the minimum scaled-footrule distance permutation
// currently using brute force approach
int *minsfd(rank_t merged, rank_t *ranks, int nrank, double *minsfd)
{
	const int c_size = merged->size;
	int *P = malloc(c_size * sizeof(int));
	int *ret = malloc(c_size * sizeof(int));
	// helper array for permutation
	// controls index bounderies for i
	int *ctl = calloc(c_size, sizeof(int));
	DUMP_ERR(P, "malloc failed");
	DUMP_ERR(ret, "malloc failed");
	DUMP_ERR(ctl, "calloc failed");

	// init position array
	for (int i = 1; i <= c_size; i++)
		P[i - 1] = i;

	// first permutation
	memcpy(ret, P, c_size * sizeof(int));
	*minsfd = sfdsum(P, merged, ranks, nrank);

	int i = 1;
	// permute P (Heap's algorithm)
	// https://en.wikipedia.org/wiki/Heap%27s_algorithm
	while (i < c_size) {
		if (ctl[i] < i) {
			int j = (i % 2) * ctl[i];
			SWAP(P[i], P[j]);
			// do calculations here
			double val = sfdsum(P, merged, ranks, nrank);
			if (*minsfd > val) {
				*minsfd = val;
				memcpy(ret, P, c_size * sizeof(int));
			}
			ctl[i]++;
			i = 1;
		} else {
			ctl[i] = 0;
			i++;
		}
	}

	free(ctl);
	free(P);
	return ret;
}
