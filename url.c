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
	// weighted pagerank value
	double wpr;
} url;

urll_t new_url_list(graph_t g, handle_t cltn)
{
	urll_t url_li = malloc(sizeof(urll));
	if (url_li == NULL) exit(EXIT_FAILURE);

	url_li->size = cltn->size;
	url_li->li = malloc(url_li->size * sizeof(url_t));

	// init url_t struct
	for (int i = 0; i < url_li->size; i++) {
		url_li->li[i] = malloc(sizeof(url));
		url_li->li[i]->url = malloc(strlen(cltn->buf[i]) + 1);
		strcpy(url_li->li[i]->url, cltn->buf[i]);
		url_li->li[i]->out_degree = outdegree(g, i);
		url_li->li[i]->wpr = (double)1 / cltn->size;
	}

	return url_li;
}

void free_list(urll_t list)
{
	for (int i = 0; i < list->size; i++) {
		free(list->li[i]->url);
		free(list->li[i]);
	}
	free(list->li);
	free(list);
}
