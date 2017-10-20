#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "rank.h"
#include "hungarian.h"

static rank_t *init_rank(int, char **);
static void free_ranks(rank_t *, int);

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [ranks]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const int nrank = argc - 1;
	rank_t *ranks = init_rank(nrank, &argv[1]);
	// merge the above ranks into one set of nodes to be ranked
	// i.e. C 
	rank_t merged = merge_ranks(ranks, nrank);

	// W
	double sfd = 0;
	// Position array
	// i.e. P
	char **P = minsfd(merged, ranks, nrank, &sfd);
	printf("%.6f\n", sfd);
	for (int i = 0; i < rank_size(merged); i++) {
		printf("%s\n", P[i]);
		free(P[i]);
	}

	free(P);
	free_rank(merged);
	free_ranks(ranks, nrank);
	return 0;
}

static rank_t *init_rank(int size, char **files)
{
	// array of ranks
	rank_t *ranks = malloc(size * sizeof(rank_t));

	// parse all the rank text files and insert ranks
	for (int i = 0; i < size; i++) {
		handle_t rank_handle = parse(files[i]);
		ranks[i] = new_rank(handle_size(rank_handle));
		for (int j = 0; j < handle_size(rank_handle); j++)
			insert_rank(ranks[i], getbuf(rank_handle, j));
		free_handle(rank_handle);
	}

	return ranks;
}

static void free_ranks(rank_t *ranks, int nrank)
{
	for (int i = 0; i < nrank; i++)
		free_rank(ranks[i]);
	free(ranks);
}
