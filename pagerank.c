#include <stdio.h>
#include <stdlib.h>

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

	graph_t gr = new_graph();
	add_edge(gr, "v1", "v2");
	add_edge(gr, "v2", "v3");
	add_edge(gr, "v2", "v1");
	show_graph(gr, SHOW_INDENT);
	free_graph(gr);
	return 0;
}

void page_rank(int d, int diff_pr, int max_iter)
{
}
