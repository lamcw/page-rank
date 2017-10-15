#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "rank.h"

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

	free_ranks(ranks, nrank);
	return 0;
}

static rank_t *init_rank(int size, char **files)
{
	rank_t *ranks = malloc(size * sizeof(rank_t));

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
