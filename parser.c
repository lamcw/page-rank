#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parser.h"

struct handle {
	int size;
	char **buf;
} handle;

static void add_size(handle_t);

handle_t parse(char *path)
{
	FILE *fp;
	fp = fopen(path, "r");

	if (fp == NULL) {
		fprintf(stderr, "No such file: %s\n", path);
		exit(1);
	}

	handle_t h = (handle_t)malloc(sizeof(struct handle));
	assert(h);
	h->buf = NULL;
	h->size = 0;
	// malloc buffer
	add_size(h);

	while (fscanf(fp, "%ms", &(h->buf[h->size - 1])) != EOF)
		add_size(h);

	// reduce size
	h->size--;
	fclose(fp);
	return h;
}

static void add_size(handle_t h)
{
	assert(h);
	char **tmp = (char **)realloc(h->buf, (h->size + 1) * sizeof(char *));

	if (tmp) {
		h->buf = tmp;
		h->size++;
	} else {
		fprintf(stderr, "Not enough memory\n");
		exit(1);
	}
}

void free_handle(handle_t h)
{
	assert(h);
	for (int i = 0; i < h->size; i++)
		free(h->buf[i]);
	free(h->buf);
	free(h);
}

void print_handle(handle_t h)
{
	for (int i = 0; i < h->size; i++)
		printf("%s\n", h->buf[i]);
}
