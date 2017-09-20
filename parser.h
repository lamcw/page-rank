#ifndef PARSER_H
#define PARSER_H

typedef struct handle *handle_t;

handle_t parse(char *);
void free_handle(handle_t);
void print_handle(handle_t);

#endif
