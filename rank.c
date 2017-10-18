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
double sfdsum(int p, rank_t *ranks, char *item, int nrank, int c_size)
{
	double sum = 0;

	for (int i = 0; i < nrank; i++)
		sum += sfd(p, ranks[i], item, c_size);

	return sum;
}

static double **cost_matrix(rank_t merged, rank_t *ranks, int nrank)
{
	// init matrix with 0
	const int n = merged->size;
	double **cost = calloc(n, sizeof(double *));
	for (int i = 0; i < n; i++)
		cost[i] = calloc(n, sizeof(double));

	// row - url
	// column - position
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			cost[i][j] += sfdsum(j + 1, ranks, merged->rank[i],    \
					     nrank, n);
	return cost;
}

static double row_lowest(double **cost, int size, int row)
{
	double min = -1;
	for (int i = 0; i < size; i++)
		if (min == -1 || min > cost[row][i])
			min = cost[row][i];
	return min;
}

static double col_lowest(double **cost, int size, int col)
{
	double min = -1;
	for (int i = 0; i < size; i++)
		if (min == -1 || min > cost[i][col])
			min = cost[i][col];
	return min;
}

static void subtract_lowest(double **cost, int size)
{
	// subtract row minima
	for (int i = 0; i < size; i++) {
		double sub = row_lowest(cost, size, i);
		for (int j = 0; j < size; j++)
			cost[i][j] -= sub;
	}
	// subtract column minima
	for (int i = 0; i < size; i++) {
		double sub = col_lowest(cost, size, i);
		for (int j = 0; j < size; j++)
			cost[j][i] -= sub;
	}
}

// permute position and find the minimum scaled-footrule distance permutation
// currently uses Hungarian assignment algorithm
int *minsfd(rank_t merged, rank_t *ranks, int nrank, double *minsfd)
{
	// cadinality of the set of nodes to be ranked
	const int c_size = merged->size;
	int *P = calloc(c_size, sizeof(int));
	double **cost = cost_matrix(merged, ranks, nrank);

	subtract_lowest(cost, c_size);

	return P;
}
