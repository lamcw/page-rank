#include <stdio.h>
#include <stdlib.h>

#include "graph.h"
#include "parser.h"

void page_rank(int, int, int);

int main(int argc, char **argv)
{
	if (argc != 4) {
		fprintf(stderr,
			"Usage: %s [d] [diffPR] [maxIterations]\n", argv[0]);
		return EXIT_FAILURE;
	}

	//int d = atoi(argv[1]);
	//int diff_pr = atoi(argv[2]);
	//int max_iter = atoi(argv[3]);

	handle_t h = parse("url/collection.txt");
	print_handle(h);
	free_handle(h);
	return 0;
}

void page_rank(int d, int diff_pr, int max_iter)
{
}
