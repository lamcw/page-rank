#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pagerank.h"
#include "url.h"

static urll_t page_rank(graph_t, handle_t, int, int, int);
static graph_t get_graph(handle_t);
static double weight_out(graph_t g, int pj, int pi);
static double weight_in(graph_t g, int pj, int pi);

int main(int argc, char **argv)
{
	if (argc != 4) {
		fprintf(stderr,
			"Usage: %s [d] [diffPR] [maxIterations]\n", argv[0]);
		return EXIT_FAILURE;
	}

	handle_t coll = parse("url/collection.txt");
	graph_t g = get_graph(coll);

	urll_t l = page_rank(g, coll, atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
	free_list(l);
	free_handle(coll);
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
		for (int j = 0; j < handle_size(hd); j++) {
			add_edge(g, collection->buf[i], hd->buf[j]);
		}

		// free memory used in this for iteration
		free(fname);
		free_handle(hd);
	}

	return g;
}

static urll_t page_rank(graph_t g,
			handle_t cltn,
			int d,
			int diff_pr,
			int max_iter)
{
	urll_t li = new_url_list(g, cltn);
	int iter = 0;
	int diff = diff_pr;
	return li;
	while (iter < max_iter && diff >= diff_pr) {
		int pr = (1 - d) / cltn->size; // + d * sum(PR(pj;t)*Win*Wout
		// sum weight
		printf("%d\n", pr);
		weight_in(g, 1, 1);
		weight_out(g, 1, 1);
	}
	return li;
}

static double weight_in(graph_t g, int pj, int pi)
{
	return 0.0;
}

static double weight_out(graph_t g, int pj, int pi)
{
	return 0.0;
}
