#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "graph.h"

// macro for dumping error messages
#ifndef DUMP_ERR
#define DUMP_ERR(ptr, str)				\
	if (ptr == NULL) {				\
		perror(str);				\
		exit(EXIT_FAILURE);			\
	}
#endif

// use uchar to save memory when representing an "edge"
// uchar == 1: has edge
// uchar == 0: no edge
typedef unsigned char uchar;
struct _graph {
	// @nv - number of vertices
	// @ne - number of edges
	// @max_v - maximum number of vertices (not used)
	// @max_e - maximum number of edges
	int nv;
	int ne;
	int max_v;
	int max_e;
	// @vertex - vertex name
	// @edges - adj matrix
	char **vertex;
	uchar **edges;
};

static int get_vertex_id(graph_t , char *);
static int add_vertex(graph_t, char *);
static void add_mtrx_size(graph_t g);

// create empty graph
graph_t new_graph(void)
{
	graph_t new = malloc(sizeof(struct _graph));
	assert(new);
	
	new->nv = new->max_v = 0;
	new->ne = new->max_e = 0;
	new->max_e = 0;
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

	for (int i = 0; i < g->max_e; i++)
		free(g->edges[i]);

	free(g->vertex);
	free(g->edges);
	free(g);
}

// doules g->edges' row and column size
static void add_mtrx_size(graph_t g)
{
	assert(g);

	int new_size = g->ne == 0 ? 1 : 2 * g->ne;
	uchar **tmp = realloc(g->edges, new_size * sizeof(uchar *));
	DUMP_ERR(tmp, "realloc failed");

	g->edges = tmp;
	// init new row ptrs to NULL
	for (int i = g->ne; i < new_size; i++) g->edges[i] = NULL;
	for (int i = 0; i < new_size; i++) {
		uchar *tmp = realloc(g->edges[i], new_size * sizeof(uchar));
		DUMP_ERR(tmp, "realloc failed");

		g->edges[i] = tmp;
		// init new columns with 0
		for (int j = g->ne; j < new_size; j++) g->edges[i][j] = 0;
	}
	// init new rows and columns with 0
	for (int i = g->ne; i < new_size; i++) {
		for (int j = 0; j < g->ne; j++)
			g->edges[i][j] = 0;
	}
	g->max_e = new_size;
}

int add_edge(graph_t g, char *src, char *dest)
{
	assert(g);
	
	int v = get_vertex_id(g, src);
	if (v < 0) {
		if (g->ne >= g->max_e) add_mtrx_size(g);
		g->ne++;
		v = add_vertex(g, src);
	}

	int w = get_vertex_id(g, dest);
	if (w < 0) {
		if (g->ne >= g->max_e) add_mtrx_size(g);
		g->ne++;
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

int nvertices(graph_t g)
{
	assert(g);
	return g->nv;
}

// count number of outgoing links of one node (doesnt count
// self loop)
int outdegree(graph_t g, int id)
{
	assert(g);
	int count = 0;
	for (int i = 0; i < g->ne; i++)
		if (g->edges[id][i] && i != id) count++;
	return count;
}

// count number of incoming links of one node (doesnt count
// self loop)
int indegree(graph_t g, int id)
{
	assert(g);
	int count = 0;
	for (int i = 0; i < g->ne; i++)
		if (g->edges[i][id] && i != id) count++;
	return count;
}

// return a list of node id(s) that has outlink to @id
int *nodes_to(graph_t g, int id, int *size)
{
	assert(g);
	int *list = malloc(g->ne * sizeof(int));
	DUMP_ERR(list, "malloc failed");
	*size = 0;

	for (int i = 0; i < g->ne; i++) {
		if (g->edges[i][id] && i != id)
			list[(*size)++] = i;
	}

	// shrink size
	if (*size) {
		int *tmp = realloc(list, *size * sizeof(int));
		DUMP_ERR(tmp, "realloc failed");
		list = tmp;
	}

	return list;
}

// return a list of node id(s) that has inlinks from @id
int *nodes_from(graph_t g, int id, int *size)
{
	assert(g);
	int *list = malloc(g->ne * sizeof(int));
	DUMP_ERR(list, "malloc failed");
	*size = 0;

	for (int i = 0; i < g->ne; i++) {
		if (g->edges[id][i] && i != id) {
			list[(*size)++] = i;
		}
	}

	// shrink size
	if (*size) {
		int *tmp = realloc(list, *size * sizeof(int));
		DUMP_ERR(tmp, "realloc failed");
		list = tmp;
	}

	return list;
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
						printf("\t%s\n", g->vertex[j]);
				}
			}
		}
	}
}

char *id_to_name(graph_t g, int id)
{
	assert(g);
	return g->vertex[id];
}

static int get_vertex_id(graph_t g, char *name)
{
	assert(g);
	for (int i = 0; i < g->nv; i++)
		if (strcmp(name, g->vertex[i]) == 0) return i;
	return -1;
}

static int add_vertex(graph_t g, char *name)
{
	assert(g);
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
