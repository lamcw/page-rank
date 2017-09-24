#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "pagerank.h"

int main(int argc, char **argv)
{
	if (argc != 4) {
		fprintf(stderr,
			"Usage: %s [d] [diffPR] [maxIterations]\n", argv[0]);
		return EXIT_FAILURE;
	}

	handle_t coll = parse("url/collection.txt");
	print_handle(coll);
	graph_t g = get_graph(coll);
	page_rank(g, atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
	free_graph(g);
	free_handle(coll);
	return 0;
}

graph_t get_graph(handle_t collection)
{
	graph_t g = new_graph();

	for (int i = 0; i < handle_size(collection); i++) {
		char *fname = malloc(strlen(collection->buf[i]) + 20);
		if (fname == NULL) exit(1);
		sprintf(fname, "url/%s.txt", collection->buf[i]);
		handle_t hd = parse_url(fname, "#start Section-1", "#end Section-1");
		for (int j = 0; j < handle_size(hd); j++)
			add_edge(g, collection->buf[i], hd->buf[j]);
		free(fname);
		free_handle(hd);
	}
	show_graph(g, 0);
	return g;
}
url_t *page_rank(graph_t g, int d, int diff_pr, int max_iter)
{
	url_t *list = NULL;
	return list;
}
