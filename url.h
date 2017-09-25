#ifndef URL_H
#define URL_H

#include "graph.h"
#include "parser.h"

// url struct
typedef struct _url *url_t;

// url struct LIST
typedef struct _urll {
	int size;
	url_t *li;
} urll;
typedef urll *urll_t;

urll_t new_url_list(graph_t g, handle_t cltn);
void free_list(urll_t list);

#endif
