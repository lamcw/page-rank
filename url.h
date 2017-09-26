#ifndef URL_H
#define URL_H

#include "graph.h"
#include "parser.h"

// url struct
typedef struct _url *url_t;
// url list struct
typedef struct _urll *urll_t;

urll_t new_url_list(graph_t g, handle_t cltn);
double getwpr(urll_t list, int id);
void setwpr(urll_t list, int id, double wpr);
void free_list(urll_t list);

#endif
