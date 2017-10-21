/* Scaled-footrule optimal aggregation for partial list
 * 	- define bipartite graph where vertices on each side represents urls
 * 	  and posssible positions, edges represents the sum of scaled-footrule
 * 	  distance
 *
 * 	   url        pos
 * 	 -----------------  
 * 	  url1 \-------1
 *              \     /
 * 	  url2 --\-----2
 *                \ /
 * 	  url3     \   3
 *               /  \
 * 	  url4       \ 4
 *                    \
 * 	  url5         5
 * 	  	etc...
 *
 * 	- time complexity O(n!) if uses brute force to generate all possible
 * 	  permutation, where n is the number of possible position
 * 	- Smarter way: recast the problem as a minimum cost bipartite matching
 * 	  problem, which we can use Hungarian Algorithm to solve
 * 	- time complexity: O(n^3)
 *
 * Hungarian Algorithm
 * 	- solve the "assignment problem"
 * 	- convert the above bipartite graph into a n x n matrix
 * 	- row = urls, col = positions
 * 	- steps:
 * 		1. subtract row minima
 *	 	2. subtract column minima
 * 		3. find possible assignment. If can't, cover all zeros
 * 		   with a minimum number of lines
 * 		4. adjust matrix (create additional zeros)
 * 		5. repeat step 3-4 until an assignment is possible
 * 		6. assign urls to positions (optimal assignment)
 * 	- this algorithm is implemented in minsfd()
 *
 * Website reference
 * 	- http://www.hungarianalgorithm.com/hungarianalgorithm.php (basic steps)
 * 	- http://cbom.atozmath.com/CBOM/Assignment.aspx?q=hm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#include "rank.h"

#define ISROW 1
#define ISCOL 0

#ifndef DUMP_ERR
#define DUMP_ERR(ptr, str)				\
	if (ptr == NULL) {				\
		perror(str);				\
		exit(EXIT_FAILURE);			\
	}
#endif

// macro to mitigate errors when comparing float / double
#ifndef FEQUAL
#define FEQUAL(a ,b) fabs(a - b) < 0.00000000000001
#endif

typedef struct {
	int row;
	int col;
} tuple_t;

static double **cost_matrix(rank_t, rank_t *, int);
static double row_lowest(double **, int, int);
static double col_lowest(double **, int, int);
static void subtract_lowest(double **, int);
static void adjust_matrix(double **, unsigned char **, int);
static void mark(unsigned char **, int, int, int);
static int row_assigned(tuple_t *, int, int);
static int is_elm_assigned(tuple_t *, int, int, int);
static int cover_zeros(double **, unsigned char **, tuple_t *,int, int);
static void reset_cover(unsigned char **, int);
static void mark_selected(unsigned char **, int, int, int);
static int has_unique_zero(double **, unsigned char **, int, int, int);
static int done_assign(double **, unsigned char **, tuple_t *, int);
static void assign_tuple(tuple_t *, int *, int, int);
static void pick_zero(double **, unsigned char **, int, int *, int *);
static tuple_t *assign(double **, double **, int, int *);
static char **pos_to_arr(tuple_t *, int, rank_t);
static void free_matrix(double **, int);

// scaled-footrule distance for 1 item
double sfd(double p, rank_t r, char *item, double c_size)
{
	double p_agg_rank = p / c_size;
	double pos = pos_in_rank(r, item);
	if (pos != -1)
		// if found in rank
		return fabs((pos + 1) / (double) rank_size(r) - p_agg_rank);
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

// create a n x n cost matrix
static double **cost_matrix(rank_t merged, rank_t *ranks, int nrank)
{
	// init matrix with 0
	const int n = rank_size(merged);
	double **cost = calloc(n, sizeof(double *));
	for (int i = 0; i < n; i++)
		cost[i] = calloc(n, sizeof(double));

	// row - url
	// column - position
	for (int i = 0; i < n; i++)
		for (int j = 1; j <= n; j++)
			cost[i][j - 1] = sfdsum(j, ranks, get_rank_item(merged, i),    \
					nrank, n);
	return cost;
}

// find the smallest element in @row
static double row_lowest(double **cost, const int size, int row)
{
	double min = -1;
	for (int i = 0; i < size; i++)
		if (min == -1 || min > cost[row][i])
			min = cost[row][i];
	return min;
}

// find the smallest element in @row
static double col_lowest(double **cost, const int size, int col)
{
	double min = -1;
	for (int i = 0; i < size; i++)
		if (min == -1 || min > cost[i][col])
			min = cost[i][col];
	return min;
}

// perform row and column reduction on cost matrix
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

// Find the smallest element (call it k) that is not covered by a line
// Subtract k from all uncovered elements, and add k to all elements that
// are covered twice.
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
				// subtract k from uncovered elements
				cost[i][j] -= min;
			else if (line[i][j] == 2)
				// add k to all elements covered twice
				cost[i][j] += min;
		}
	}
}

// mark row / column to be selected
static void mark(unsigned char **m, int size, int k, int isrow)
{
	for (int i = 0; i < size; i++)
		switch (isrow) {
			case ISROW:
				m[k][i]++;
				break;
			case ISCOL:
				m[i][k]++;
				break;
		}
}

// check if row has assignment
static int row_assigned(tuple_t *assigned, int nassigned, int row)
{
	for (int i = 0; i < nassigned; i++)
		if (assigned[i].row == row)
			return 1;
	return 0;
}

static int is_elm_assigned(tuple_t *assigned, int n, int row, int col)
{
	for (int i = 0; i < n; i++)
		if (assigned[i].row == row && assigned[i].col == col)
			return 1;
	return 0;
}

// step 3 cover all zeros in cost matrix with minimum number of lines
static int cover_zeros(double **cost, unsigned char **line, tuple_t *assigned,
			int nassigned, int size)
{
	unsigned char *tick_row = calloc(size, sizeof(unsigned char));
	unsigned char *tick_col = calloc(size, sizeof(unsigned char));

	// Mark all rows having no assignments
	for (int i = 0; i < size; i++)
		if (!row_assigned(assigned, nassigned, i))
			tick_row[i] = 1;

	int ticked = 1;
	while (ticked) {
		ticked = 0;
		// Mark all (unmarked) columns having zeros in newly marked
		// row(s)
		for (int i = 0; i < size; i++) {
			if (tick_row[i] == 1) {
				for (int j = 0; j < size; j++) {
					if (FEQUAL(cost[i][j], 0) && !tick_col[j]) {
						tick_col[j] = 1;
						ticked++;
					}
				}
			}
		}
		// Mark all rows having assignments in newly marked columns
		for (int j = 0; j < size; j++) {
			if (tick_col[j] == 1) {
				for (int i = 0; i < size; i++) {
					if (FEQUAL(cost[i][j], 0) && !tick_row[i]
					    && is_elm_assigned(assigned, nassigned, i, j))
					{
						tick_row[i] = 1;
						ticked++;
					}
				}
			}
		}
	}
	// draw lines through all marked columns and unmarked rows
	int l = 0;
	for (int i = 0; i < size; i++) {
		if (!tick_row[i]) {mark(line, size, i, ISROW); l++;}
		if (tick_col[i]) {mark(line, size, i, ISCOL); l++;}
	}
	free(tick_row);
	free(tick_col);
	return l;
}

// reset all the lines to zero
static void reset_cover(unsigned char **line, int size)
{
	for (int i = 0; i < size; i++)
		for (int j = 0; j < size; j++)
			line[i][j] = 0;
}

// mark @row and @col to be selected
static void mark_selected(unsigned char **selected, int size, int row, int col)
{
	mark(selected, size, row, ISROW);
	mark(selected, size, col, ISCOL);
}

// if matrix has unqiue zero along row / column @k, return the position of
// that zero else reutrn -1
static int has_unique_zero(double **cost, unsigned char **selected, int k,
		int size, int isrow)
{
	int count = 0;
	int first_zero = -1;

	for (int i = 0; i < size; i++) {
		if (isrow) {
			if (FEQUAL(cost[k][i], 0) && !selected[k][i]) {
				if (first_zero == -1) first_zero = i;
				count++;
			}
		} else {
			if (FEQUAL(cost[i][k], 0) && !selected[i][k]) {
				if (first_zero == -1) first_zero = i;
				count++;
			}
		}
	}

	return count == 1 ? first_zero : -1;
}


// check if matrix has a unique zero, that is, there is only 1 zero along a
// particular row / column
static int done_assign(double **m, unsigned char **selected, tuple_t *assigned,
		       int size)
{
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			if (FEQUAL(m[i][j], 0)
			    && !(is_elm_assigned(assigned, size, i, j)
			         || selected[i][j])) {
				return 0;
			}
		}
	}	
	return 1;
}

static void assign_tuple(tuple_t *p, int *index, int row, int col)
{
	p[*index].row = row;
	p[*index].col = col;
	(*index)++;
}

// pick a zero in matrix arbitrarily
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

// step 5
// find unique zeros in each row and column and assign url to positions
static tuple_t *assign(double **orig, double **cost, int size, int *nassigned)
{
	*nassigned = 0;
	// array to store url and position pair
	tuple_t *p = calloc(size, sizeof(tuple_t));
	// stores selected elements in @cost
	unsigned char **selected = calloc(size, sizeof(unsigned char *));
	DUMP_ERR(p, "calloc failed");
	DUMP_ERR(selected, "calloc failed");
	for (int i = 0; i < size; i++) {
		selected[i] = calloc(size, sizeof(unsigned char));
		DUMP_ERR(selected[i], "calloc failed");
	}

	while (!done_assign(cost, selected, p, size)) { 
		int picked = 0;
		// row scanning
		for (int i = 0; i < size; i++) {
			int pos = has_unique_zero(cost, selected, i, size, 1);
			if (pos >= 0) {
				// mark this row to be selected
				mark_selected(selected, size, i, pos);
				// push url and position into @p array
				assign_tuple(p, nassigned, i, pos);
				picked++;
			}
		}
		// column scanning
		for (int j = 0; j < size; j++) {
			int pos = has_unique_zero(cost, selected, j, size, 0);
			if (pos >= 0) {
				// mark column as selected
				mark_selected(selected, size, pos, j);
				assign_tuple(p, nassigned, pos, j);
				picked++;
			}
		}

		// heuristic: when the above cannot choose any zero to assign
		// just pick one arbitrarily
		if (!picked) {
			int i = 0, j = 0;
			pick_zero(cost, selected, size, &i, &j);
			mark_selected(selected, size, i, j);
			assign_tuple(p, nassigned, i, j);
		}
	}
	for (int i = 0; i < size; i++)
		free(selected[i]);
	free(selected);
	return p;
}

// convert the tuple array to char array
static char **pos_to_arr(tuple_t *pos, int size, rank_t merged)
{
	char **arr = malloc(size * sizeof(char *));
	for (int i = 0; i < size; i++) {
		arr[pos[i].col] = malloc(strlen(get_rank_item(merged, pos[i].col)) + 1);
		DUMP_ERR(arr[pos[i].col], "malloc failed");
		strcpy(arr[pos[i].col], get_rank_item(merged, pos[i].row));
	}
	return arr;
}

// uses Hungarian assignment algorithm to find the minimum scaled-footrule
// distance
char **minsfd(rank_t merged, rank_t *ranks, int nrank, double *minsfd)
{
	*minsfd = 0;
	// cadinality of the set of nodes to be ranked
	const int c_size = rank_size(merged);
	double **orig = cost_matrix(merged, ranks, nrank);
	double **cost = cost_matrix(merged, ranks, nrank);
	unsigned char **covered = calloc(c_size, sizeof(unsigned char *));
	DUMP_ERR(covered, "calloc failed");
	for (int i = 0; i < c_size; i++) {
		covered[i] = calloc(c_size, sizeof(unsigned char));
		DUMP_ERR(covered[i], "calloc failed");
	}

	int nassigned = 0;
	// step 1 & 2
	subtract_lowest(cost, c_size);
	// step 3
	tuple_t *pos = assign(orig, cost, c_size, &nassigned);
	while (nassigned < c_size) {
		// step 3
		cover_zeros(cost, covered, pos, nassigned, c_size);
		// step 4
		adjust_matrix(cost, covered, c_size);
		pos = assign(orig, cost, c_size, &nassigned);
		reset_cover(covered, c_size);
	}

	for (int i = 0; i < c_size; i++)
		*minsfd += orig[pos[i].row][pos[i].col];

	// step 5
	char **url = pos_to_arr(pos, c_size, merged);

	// free eveything
	for (int i = 0; i < c_size; i++)
		free(covered[i]);
	free(covered);
	free(pos);
	free_matrix(cost, c_size);
	free_matrix(orig, c_size);
	return url;
}

static void free_matrix(double **m, int size)
{
	for (int i = 0; i < size; i++)
		free(m[i]);
	free(m);
}
