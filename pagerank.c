#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pagerank.h"

static url_t *page_rank(graph_t, int, int, int);
static graph_t get_graph(handle_t);

int main(int argc, char **argv)
{
	if (argc != 4) {
		fprintf(stderr,
			"Usage: %s [d] [diffPR] [maxIterations]\n", argv[0]);
		return EXIT_FAILURE;
	}

	handle_t coll = parse("url/collection.txt");
	graph_t g = get_graph(coll);
	free_handle(coll);

	page_rank(g, atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
	free_graph(g);
	return 0;
}

static graph_t get_graph(handle_t collection)
{
	graph_t g = new_graph();

	for (int i = 0; i < handle_size(collection); i++) {
		// stores file path and file name
		// e.g. url1234.txt
		char *fname = malloc(strlen(collection->buf[i]) + 20);
		
		if (fname == NULL) {
			perror("malloc failed");
			exit(EXIT_FAILURE);
		}

		sprintf(fname, "url/%s.txt", collection->buf[i]);
		
		// parse url?.txt
		handle_t hd = parse_url(fname, "#start Section-1", "#end Section-1");
		// for each link in url?.txt
		// add edge from this url to that link
		for (int j = 0; j < handle_size(hd); j++)
			add_edge(g, collection->buf[i], hd->buf[j]);

		// free memory used in this for iteration
		free(fname);
		free_handle(hd);
	}

	return g;
}

static url_t *page_rank(graph_t g, int d, int diff_pr, int max_iter)
{
	url_t *list = NULL;
	return list;
}
