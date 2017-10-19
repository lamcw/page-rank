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

typedef char *item_t;

typedef struct _tuple {
	int url;
	int p;
} tuple_t;

// wrapper around an array of char *
// with some extra attributes
struct rank {
	item_t *rank;		// basically an array of char *
	int size;		// current size of the @rank
	int max_size;		// size allocated for @rank
};

// helper functions signature
static void add_size(rank_t);
static double **cost_matrix(rank_t, rank_t *, int);
static double row_lowest(double **, int, int);
static double col_lowest(double **, int, int);
static void subtract_lowest(double **, int);
static int all_zero(int *, int);
static void show_matrix(double **, int);
static void init_zero_count(double **, int *, int *, int);
static int cover_zeros(double **, unsigned char **, int);
static void adjust_matrix(double **, unsigned char **, int);
static void mark_selected(unsigned char **, int, int, int);
static int has_unique_zero(double **, unsigned char **, int, int, int);
static int matrix_has_unqiue_zero(double **, unsigned char **, int, int *, int *);
static void insert_to_position(tuple_t *, int *, int, int);
static void reset_cover(unsigned char **, int);
static void free_matrix(double **, int);

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

// calculate the sum of scaled-footrule distance for a given url in a given
// position in final rank
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
		for (int j = 1; j <= n; j++)
			cost[i][j - 1] = sfdsum(j, ranks, merged->rank[i],    \
					nrank, n);
	return cost;
}

static double row_lowest(double **cost, const int size, int row)
{
	double min = -1;
	for (int i = 0; i < size; i++)
		if (min == -1 || min > cost[row][i])
			min = cost[row][i];
	return min;
}

static double col_lowest(double **cost, const int size, int col)
{
	double min = -1;
	for (int i = 0; i < size; i++)
		if (min == -1 || min > cost[i][col])
			min = cost[i][col];
	return min;
}

static void subtract_lowest(double **cost, const int size)
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

static int all_zero(int *arr, const int n)
{
	for (int i = 0; i < n; i++)
		if (arr[i] != 0)
			return 0;
	return 1;
}

static void show_matrix(double **m, const int size)
{
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++)
			printf("%f ", m[i][j]);
		puts("");
	}
}

// find number of zeros in each column and row
static void init_zero_count(double **m, int *row, int *col, const int size)
{
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			if (m[i][j] == 0) {
				row[i]++;
				col[j]++;
			}
		}
	}
}

static int cover_zeros(double **cost, unsigned char **line, const int size)
{
	// number of zeros in row / column
	int *zeros_row = calloc(size, sizeof(int));
	int *zeros_col = calloc(size, sizeof(int));
	int line_required = 0;

	init_zero_count(cost, zeros_row, zeros_col, size);

	while (!all_zero(zeros_row, size) && !all_zero(zeros_col, size)) {
		// index of row with max no. of 0s
		int maxrow = 0;
		// index of column with max no. of 0s
		int maxcol = 0;
		// maximum number of 0s in one row/column
		int max = -1;
		unsigned char max_iscol = 0;

		// search for row / column with max no. of 0s
		for (int i = 0; i < size; i++) {
			if (max == -1 || zeros_row[i] > max) {
				max = zeros_row[i];
				maxrow = i;
				max_iscol = 0;
			}
			if (zeros_col[i] > max) {
				max = zeros_col[i];
				maxcol = i;
				max_iscol = 1;
			}
		}

		// reset the 0 count for row / column with max no. of 0s
		if (max_iscol)
			zeros_col[maxcol] = 0;
		else
			zeros_row[maxrow] = 0;

		// reduce 0 count
		//
		//           0 count
		// 0 1 2 3 -> 1
		// 0 4 0 5 -> 2
		// 0 6 7 8 -> 1
		// 0 0 0 9 -> 3
		// | | | |
		// v v v v
		// 4 1 2 0
		//
		// count in column 0 is reset to 0
		//
		// | 1 2 3 -> 0
		// | 4 0 5 -> 1
		// | 6 7 8 -> 0
		// | 0 0 9 -> 2
		// | | | |
		// v v v v
		// 0 1 2 0
		if (max_iscol) {
			for (int i = 0; i < size; i++)
				if (cost[i][maxcol] == 0)
					zeros_row[i]--;
		} else {
			for (int i = 0; i < size; i++)
				if (cost[maxrow][i] == 0)
					zeros_col[i]--;
		}
		for (int i = 0; i < size; i++)
			if (max_iscol)
				line[i][maxcol]++;
			else
				line[maxrow][i]++;
		line_required++;
	}

	free(zeros_col);
	free(zeros_row);
	return line_required;
}

static void adjust_matrix(double **cost, unsigned char **line, int size)
{
	double min = -1;

	// find smallest uncovered element in matrix
	for (int i = 0; i < size; i++)
		for (int j = 0; j < size; j++)
			if (line[i][j] == 0 && (min == -1 || min > cost[i][j]))
				min = cost[i][j];

	// subtract @min from all uncovered elements
	// add @min to all elements that are covered twice
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			if (line[i][j] == 0)
				cost[i][j] -= min;
			else if (line[i][j] == 2)
				cost[i][j] += min;
		}
	}
}

static void reset_cover(unsigned char **line, int size)
{
	// reset everything to zero
	for (int i = 0; i < size; i++)
		for (int j = 0; j < size; j++)
			line[i][j] = 0;
}

static void mark_selected(unsigned char **selected, int size, int row, int col)
{
	for (int i = 0; i < size; i++) {
		selected[row][i] = 1;
		selected[i][col] = 1;
	}
}

static int has_unique_zero(double **cost, unsigned char **selected, int k,
			   int size, int isrow)
{
	int count = 0;
	int first_zero = -1;

	for (int i = 0; i < size; i++) {
		if (isrow) {
			if (cost[k][i] == 0 && !selected[k][i]) {
				if (first_zero == -1) first_zero = i;
				count++;
			}
		} else {
			if (cost[i][k] == 0 && !selected[i][k]) {
				if (first_zero == -1) first_zero = i;
				count++;
			}
		}
	}

	return count == 1 ? first_zero : -1;
}

static void pick_zero(double **cost, unsigned char **selected, int size,
		int *row, int *col)
{
	*row = *col = 0;
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			if (cost[i][j] == 0 && !selected[i][j]) {
				*row = i;
				*col = j;
				break;
			}
		}
	}
}

static int matrix_has_unqiue_zero(double **m, unsigned char **selected,
		int size, int *row, int *col)
{
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			if (has_unique_zero(m, selected, i, size, 1) != -1
				&& has_unique_zero(m, selected, j, size, 0) != -1) {
				*row = i;
				*col = j;
				return 1;
			}
		}
	}	
	return 0;
}

static void insert_to_position(tuple_t *p, int *index, int row, int col)
{
	p[*index].url = row;
	p[*index].p = col;
	(*index)++;
}

static tuple_t *set_minsfd(double **orig, double **cost, int size, double *sfd)
{
	int index = 0;
	tuple_t *p = calloc(size, sizeof(tuple_t));
	unsigned char **selected = calloc(size, sizeof(unsigned char *));
	for (int i = 0; i < size; i++)
		selected[i] = calloc(size, sizeof(unsigned char));

	// row scanning
	for (int i = 0; i < size; i++) {
		int pos = has_unique_zero(cost, selected, i, size, 1);
		if (pos >= 0) {
			mark_selected(selected, size, i, pos);
			insert_to_position(p, &index, i, pos);
		}
	}
	// column scanning
	for (int j = 0; j < size; j++) {
		int pos = has_unique_zero(cost, selected, j, size, 0);
		if (pos >= 0) {
			mark_selected(selected, size, pos, j);
			insert_to_position(p, &index, pos, j);
		}
	}

	if (index < size) {
		int i = 0, j = 0;
		pick_zero(cost, selected, size, &i, &j);
		mark_selected(selected, size, i, j);
		p[index].url = i;
		p[index].p = j;
		index++;
		while (index < size && matrix_has_unqiue_zero(cost, selected, size, &i, &j)) {
			mark_selected(selected, size, i, j);
			insert_to_position(p, &index, i, j);
		}
	}

	for (int i = 0; i < size; i++) {
		*sfd += orig[p[i].url][p[i].p];
		free(selected[i]);
	}
	free(selected);
	return p;
}

static char **pos_to_parr(tuple_t *pos, int size, rank_t merged)
{
	char **arr = malloc(size * sizeof(char *));
	for (int i = 0; i < size; i++) {
		arr[pos[i].p] = malloc(strlen(merged->rank[pos[i].url]) + 1);
		strcpy(arr[pos[i].p], merged->rank[pos[i].url]);
	}
	return arr;
}

// permute position and find the minimum scaled-footrule distance permutation
// currently uses Hungarian assignment algorithm
char **minsfd(rank_t merged, rank_t *ranks, int nrank, double *minsfd)
{
	*minsfd = 0;
	// cadinality of the set of nodes to be ranked
	const int c_size = merged->size;
	double **orig = cost_matrix(merged, ranks, nrank);
	double **cost = cost_matrix(merged, ranks, nrank);
	unsigned char **covered = calloc(c_size, sizeof(unsigned char *));

	for (int i = 0; i < c_size; i++)
		covered[i] = calloc(c_size, sizeof(unsigned char));

	subtract_lowest(cost, c_size);
	while (cover_zeros(cost, covered, c_size) < c_size) {
		show_matrix(cost, c_size);
		adjust_matrix(cost, covered, c_size);
		reset_cover(covered, c_size);
	}

	tuple_t *pos = set_minsfd(orig, cost, c_size, minsfd);
	char **p = pos_to_parr(pos, c_size, merged);

	for (int i = 0; i < c_size; i++)
		free(covered[i]);
	free(covered);
	free(pos);
	free_matrix(cost, c_size);
	free_matrix(orig, c_size);
	return p;
}

static void free_matrix(double **m, int size)
{
	for (int i = 0; i < size; i++)
		free(m[i]);
	free(m);
}
