// url operations

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "url.h"

// internal definition of url
struct _url {
	char *url;	// url string
	int out_degree;	// number of out going links
	int in_degree;	// number of incoming links
	int *inlinks;	// contains incoming links to this url
	int *outlinks;	// contains outgoing links to url(s)
	double wpr;	// weighted pagerank value
};

// url struct LIST
struct _urll {
	int size;
	url_t *li;
};

// takes in graph and collection to generate a list of url_t
urll_t new_url_list(graph_t g, handle_t cltn)
{
	urll_t url_li = malloc(sizeof(struct _urll));
	if (url_li == NULL) exit(EXIT_FAILURE);

	url_li->size = handle_size(cltn);
	url_li->li = malloc(url_li->size * sizeof(url_t));

	// init url_t struct
	for (int i = 0; i < url_li->size; i++) {
		url_li->li[i] = malloc(sizeof(struct _url));
		// assign to var to make it more readable
		url_t u = url_li->li[i];

		u->url = malloc(strlen(getbuf(cltn, i)) + 1);
		strcpy(u->url, getbuf(cltn, i));

		u->out_degree = outdegree(g, get_vertex_id(g, u->url));
		u->in_degree = indegree(g, get_vertex_id(g, u->url));
		/* u->outlinks = nodes_to(g, i, &u->out_degree); */
		/* u->inlinks = nodes_from(g, i, &u->in_degree); */
		u->wpr = (double)1 / handle_size(cltn);
	}

	return url_li;
}

// update weighted page rank
void setwpr(urll_t list, int id, double wpr)
{
	list->li[id]->wpr = wpr;
}

double getwpr(urll_t list, int id)
{
	return list->li[id]->wpr;
}

/* int *get_outlinks(urll_t list, int id) */
/* { */
/* 	return list->li[id]->outlinks; */
/* } */

/* int *get_inlinks(urll_t list, int id) */
/* { */
/* 	return list->li[id]->inlinks; */
/* } */

void free_list(urll_t list)
{
	for (int i = 0; i < list->size; i++) {
		free(list->li[i]->url);
		/* free(list->li[i]->inlinks); */
		/* free(list->li[i]->outlinks); */
		free(list->li[i]);
	}
	free(list->li);
	free(list);
}

int _cmp_wpr(const void *a, const void *b)
{
	url_t *ia = (url_t *)a;
	url_t *ib = (url_t *)b;
	return (*ia)->wpr < (*ib)->wpr;
}

void output(urll_t list, char *path)
{
	FILE *fp = fopen(path, "w");

	qsort(list->li, list->size, sizeof(url_t), _cmp_wpr);

	for (int i = 0; i < list->size; i++)
		fprintf(fp, "%s, %d, %.7f\n", 
				list->li[i]->url,
				list->li[i]->out_degree,
				list->li[i]->wpr);
	fclose(fp);
}

void show_list(urll_t list)
{
	for (int i = 0; i < list->size; i++)
		printf("%s %d %.7f\n", list->li[i]->url, list->li[i]->out_degree, list->li[i]->wpr);
}
