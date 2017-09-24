#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "graph.h"
#include "parser.h"

void page_rank(int, int, int);
handle_t get_collection(char *path);
graph_t get_graph(char *path);

int main(int argc, char **argv)
{
	if (argc != 4) {
		fprintf(stderr,
			"Usage: %s [d] [diffPR] [maxIterations]\n", argv[0]);
		return EXIT_FAILURE;
	}

	page_rank(atoi(argv[1]), atoi(argv[1]), atoi(argv[1]));

	handle_t h = parse_url("url/url22.txt", "#start Section-1", "#end Section-1");
	print_handle(h);
	free_handle(h);

	handle_t hh = parse("url/collection.txt");
	print_handle(hh);
	free_handle(hh);

	graph_t gr = new_graph();
	srand(time(NULL));
	int size = 100000;
	for (int i = 0; i < size; i++) {
		char str1[10];
		char str2[10];
		sprintf(str1, "v%d", rand() % size);
		sprintf(str2, "v%d", rand() % size);
		add_edge(gr, str1, str2);
	}
	show_graph(gr,0);
	free_graph(gr);
	return 0;
}

void page_rank(int d, int diff_pr, int max_iter)
{
}
