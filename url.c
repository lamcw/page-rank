// url operations

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "url.h"

// internal definition of url
typedef struct _url {
	// url string
	char *url;
	// number of out going links
	int out_degree;
	int inlinks_size;
	// contains outgoing links to this url
	int *inlinks;
	// weighted pagerank value
	double wpr;
} url;

// takes in graph and collection to generate a list of url_t
urll_t new_url_list(graph_t g, handle_t cltn)
{
	urll_t url_li = malloc(sizeof(urll));
	if (url_li == NULL) exit(EXIT_FAILURE);

	url_li->size = cltn->size;
	url_li->li = malloc(url_li->size * sizeof(url_t));

	// init url_t struct
	for (int i = 0; i < url_li->size; i++) {
		url_li->li[i] = malloc(sizeof(url));
		// assign to var to make it more readable
		url_t u = url_li->li[i];

		u->url = malloc(strlen(cltn->buf[i]) + 1);
		strcpy(u->url, cltn->buf[i]);

		u->out_degree = outdegree(g, i);
		u->inlinks_size = 0;
		u->inlinks = edge_to(g, i, &u->inlinks_size);
		u->wpr = (double)1 / cltn->size;
	}

	return url_li;
}

// update weighted page rank
void setwpr(urll_t list, int id, int wpr)
{
	list->li[id]->wpr = wpr;
}

double getwpr(urll_t list, int id)
{
	return list->li[id]->wpr;
}

void free_list(urll_t list)
{
	for (int i = 0; i < list->size; i++) {
		free(list->li[i]->url);
		free(list->li[i]->inlinks);
		free(list->li[i]);
	}
	free(list->li);
	free(list);
}
