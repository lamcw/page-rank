#ifndef PAGERANK_H
#define PAGERANK_H

#include "graph.h"
#include "parser.h"

typedef struct _url *url_t;
typedef struct _url {
	// url string
	char *url;
	// number of out going links
	int out_degree;
	// weighted pagerank value
	double wpr;
} url;

#endif
