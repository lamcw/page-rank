#ifndef URL_H
#define URL_H

#include "graph.h"
#include "parser.h"

// url struct
typedef struct _url *url_t;
// url list struct
typedef struct _urll *urll_t;

urll_t new_url_list(graph_t, handle_t);
double getwpr(urll_t list, int id);
void setwpr(urll_t, int, double);
void output(urll_t, char *);
int *get_outlinks(urll_t, int);
int *get_inlinks(urll_t, int);
void free_list(urll_t);
void show_list(urll_t);

#endif
