#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "url.h"
#include "graph.h"
#include "parser.h"

static urll_t page_rank(graph_t, handle_t, double, double, int);
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

	handle_t cltn = parse("collection.txt");
	graph_t g = get_graph(cltn);

	urll_t l = page_rank(g, cltn, atof(argv[1]), atof(argv[2]), atoi(argv[3]));
	output(l, "pagerankList.txt");
	free_list(l);
	free_handle(cltn);
	free_graph(g);
	return 0;
}

static graph_t get_graph(handle_t collection)
{
	graph_t g = new_graph();

	// add url first then add links to make the graph looks the same to
	// the one in the examples
	//
	// this for loop is originally not needed but added to make debugging
	// easier
	for (int i = 0; i < handle_size(collection); i++)
		add_edge(g, getbuf(collection, i), getbuf(collection, i));

	for (int i = 0; i < handle_size(collection); i++) {
		// stores file path and file name
		// e.g. url1234.txt
		char *fname = malloc(strlen(getbuf(collection, i)) + 4);
		if (fname == NULL) {
			perror("malloc failed");
			exit(EXIT_FAILURE);
		}
		sprintf(fname, "%s.txt", getbuf(collection, i));
		
		// parse url?.txt
		handle_t hd = parse_url(fname, "#start Section-1", "#end Section-1");
		// for each link in url?.txt
		// add edge from this url to that link
		for (int j = 0; j < handle_size(hd); j++)
			add_edge(g, getbuf(collection, i), getbuf(hd, j));

		// free memory used in this for iteration
		free(fname);
		free_handle(hd);
	}

	return g;
}

static urll_t page_rank(const graph_t g,
			const handle_t cltn,
			const double d,
			const double diff_pr,
			const int max_iter)
{
	urll_t li = new_url_list(g, cltn);
	int iter = 0;
	double diff = diff_pr;

	// first term in the formula
	const double fterm = (1 - d) / handle_size(cltn);
	while (iter < max_iter && diff >= diff_pr) {
		diff = 0;
		iter++;
		// stores new wpr value
		double *wpr_list = calloc(handle_size(cltn), sizeof(double));

		for (int i = 0; i < handle_size(cltn); i++) {
			int size = 0;
			// M(pi)
			int *url_to = nodes_to(g, i, &size);
			// sum(PR(pj;t) * Win * Wout
			double sum = 0;
			for (int j = 0; j < size; j++)
				sum += getwpr(li, url_to[j]) *
					weight_in(g, url_to[j], i) *
					weight_out(g, url_to[j], i);
			// sum weight
			wpr_list[i] = fterm + d * sum;
			free(url_to);
		}

		for (int i = 0; i < handle_size(cltn); i++) {
			diff += fabs(wpr_list[i] - getwpr(li, i));
			setwpr(li, i, wpr_list[i]);
		}
		free(wpr_list);
	}
	return li;
}

static double weight_in(graph_t g, int pj, int pi)
{
	double deg_pi = indegree(g, pi);
	double sum = 0;
	int size = 0;
	int *urls = nodes_from(g, pj, &size);

	for (int i = 0; i < size; i++)
		sum += indegree(g, urls[i]);
	free(urls);

	return deg_pi / sum;
}

static double weight_out(graph_t g, int pj, int pi)
{
	double deg_pi = outdegree(g, pi);
	deg_pi = deg_pi == 0 ? 0.5 : deg_pi;
	double sum = 0;
	int size = 0;
	int *urls = nodes_from(g, pj, &size);

	for (int i = 0; i < size; i++) {
		double deg = outdegree(g, urls[i]);
		sum += deg == 0 ? 0.5 : deg;
	}
	free(urls);

	return deg_pi / sum;
}
