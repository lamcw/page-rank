#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pagerank.h"
#include "url.h"

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

	handle_t coll = parse("url/collection.txt");
	graph_t g = get_graph(coll);
	show_graph(g, 0);
	show_graph(g, 1);

	urll_t l = page_rank(g, coll, atof(argv[1]), atof(argv[2]), atoi(argv[3]));
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
			double d,
			double diff_pr,
			int max_iter)
{
	urll_t li = new_url_list(g, cltn);
	int iter = 0;
	double diff = diff_pr;
	//return li;

	// first term in the formula
	const double fterm = (1 - d) / handle_size(cltn);
	while (iter < max_iter && diff >= diff_pr) {
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
				sum += getwpr(li, j) * weight_in(g, url_to[j], i) * weight_out(g, url_to[j], i);
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

	for (int i = 0; i < size; i++)
		sum += outdegree(g, urls[i]);
	free(urls);

	return deg_pi / sum;
}
