// graph.h ... Interface to Graph of strings
// Written by John Shepherd, September 2015
// Modified by Thomas Lam for COMP2521 assignment 2

#ifndef GRAPH_H
#define GRAPH_H

// show_graph mode constants
#define SHOW_INDENT 0
#define SHOW_MTRX 1

typedef struct _graph *graph_t;
// use uchar to save memory when representing an "edge"
// uchar == 1: has edge
// uchar == 0: no edge
typedef unsigned char uchar;
typedef struct _graph {
	// @nv - number of vertices
	// @ne - number of edges
	// @max_v - maximum number of vertices (not used in this adt)
	// @max_e - maximum number of edges
	int nv;
	int ne;
	int max_v;
	int max_e;
	// @vertex - vertex name
	// @edges - adj matrix
	char **vertex;
	uchar **edges;
} graph;

// Function signatures

graph_t new_graph(void);
void free_graph(graph_t);
int add_edge(graph_t ,char *, char *);
int nvertices(graph_t);
int outdegree(graph_t, int);
int indegree(graph_t, int);
int *nodes_to(graph_t, int, int *);
int *nodes_from(graph_t, int, int *);
char *id_to_name(graph_t, int);
int is_connected(graph_t, char *, char *);
void show_graph(graph_t, int);

#endif
