#ifndef PARSER_H
#define PARSER_H

typedef struct _handle *handle_t;

handle_t parse(char *);
handle_t parse_url(char *, char *start_tag, char *end_tag);
void free_handle(handle_t);
void print_handle(handle_t);
char *getbuf(handle_t h, int id);
int handle_size(handle_t);

#endif
