#ifndef PARSER_H
#define PARSER_H

typedef struct _handle *handle_t;
typedef struct _handle {
	int size;
	int max_size;
	char **buf;
} handle;

handle_t parse(char *);
handle_t parse_url(char *, char *start_tag, char *end_tag);
void free_handle(handle_t);
void print_handle(handle_t);
int handle_size(handle_t);

#endif
