#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "graph.h"

#ifndef DUMP_ERR
#define DUMP_ERR(ptr, str) \
	if (ptr == NULL) {\
		perror(str);\
		exit(EXIT_FAILURE);\
	}
#endif

// use uchar to save memory when representing an "edge"
// uchar == 1: has edge
// uchar == 0: no edge
typedef unsigned char uchar;
typedef struct graph {
	// @nv - number of vertices
	// @ne - number of edges
	// note: nv should be equal to ne
	int nv;
	int ne;
	// @vertex - vertex name
	// @edges - adj matrix
	char **vertex;
	uchar **edges;
} graph;

static int get_vertex_id(graph_t , char *);
static int add_vertex(graph_t, char *);
static void add_mtrx_size(graph_t g);

// create empty graph
graph_t new_graph(void)
{
	graph_t new = malloc(sizeof(graph));
	assert(new);
	
	new->nv = 0;
	new->ne = 0;
	new->vertex = NULL;
	new->edges = NULL;

	return new;
}

// free graph memory
void free_graph(graph_t g)
{
	if (g == NULL) return;
	
	for (int i = 0; i < g->nv; i++)
		free(g->vertex[i]);

	for (int i = 0; i < g->ne; i++)
		free(g->edges[i]);

	free(g->vertex);
	free(g->edges);
	free(g);
}

// add g->edges' row and column by 1
static void add_mtrx_size(graph_t g)
{
	assert(g);

	uchar **tmp = realloc(g->edges, (g->ne + 1) * sizeof(uchar *));
	DUMP_ERR(tmp, "realloc failed");

	g->edges = tmp;
	g->edges[g->ne] = NULL;
	for (int i = 0; i < g->ne + 1; i++) {
		uchar *tmp = realloc(g->edges[i], (g->ne + 1) * sizeof(uchar));
		DUMP_ERR(tmp, "realloc failed");

		g->edges[i] = tmp;
		// init last column with 0
		g->edges[i][g->ne] = 0;
		// init last row with 0s
		if (i == g->ne) {
			for (int j = 0; j < g->ne + 1; j++)
				g->edges[i][j] = 0;
		}
	}
	g->ne++;
}

int add_edge(graph_t g, char *src, char *dest)
{
	assert(g);
	
	int v = get_vertex_id(g, src);
	if (v < 0) {
		add_mtrx_size(g);
		v = add_vertex(g, src);
	}

	int w = get_vertex_id(g, dest);
	if (w < 0) {
		add_mtrx_size(g);
		w = add_vertex(g, dest);
	}

	g->edges[v][w] = 1;
	return 1;
}

// check if 2 vertices are connected directly
int is_connected(graph_t g, char *src, char *dest)
{
	assert(g);
	int v = get_vertex_id(g, src);
	int w = get_vertex_id(g, dest);

	if (v < 0 || w < 0)
		return 0;
	else
		return g->edges[v][w];
}

int get_n_vertices(graph_t g)
{
	assert(g);
	return g->nv;
}

void show_graph(graph_t g, int mode)
{
	assert(g);
	if (g->nv == 0)
		fprintf(stderr, "graph is empty\n");
	else {
		printf("graph has %d vertices:\n", g->nv);
		for (int i = 0; i < g->nv; i++) {
			if (mode == SHOW_MTRX) {
				for (int j = 0; j < g->nv; j++)
					printf("%d", g->edges[i][j]);
				putchar('\n');
			} else if (mode == SHOW_INDENT) {
				printf("Vertex: %s\n", g->vertex[i]);
				printf("connects to\n");
				for (int j = 0; j < g->nv; j++) {
					if (g->edges[i][j])
						printf("   %s\n", g->vertex[j]);
				}
			}
		}
	}
}

static int get_vertex_id(graph_t g, char *name)
{
	for (int i = 0; i < g->nv; i++)
		if (strcmp(name, g->vertex[i]) == 0) return i;
	return -1;
}

static int add_vertex(graph_t g, char *name)
{
	assert(strlen(name) > 0);
	char **tmp = realloc(g->vertex, (g->nv + 1) * sizeof(char *));

	DUMP_ERR(tmp, "malloc failed");

	g->vertex = tmp;
	// use malloc + strcpy instead of strdup since strdup
	// is not in C standard library
	g->vertex[g->nv] = malloc(strlen(name) + 1);
	strcpy(g->vertex[g->nv], name);
	g->nv++;

	return g->nv - 1;
}
