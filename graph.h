// graph.h ... Interface to Graph of strings
// Written by John Shepherd, September 2015

#ifndef GRAPH_H
#define GRAPH_H

// show_graph mode constants
#define SHOW_INDENT 0
#define SHOW_MTX 1

typedef struct graph *graph_t;

// Function signatures

graph_t new_graph(int);
void free_graph(graph_t);
int add_edge(graph_t ,char *, char *);
int get_n_vertices(graph_t);
int is_connected(graph_t, char *, char *);
void show_graph(graph_t, int);

#endif
