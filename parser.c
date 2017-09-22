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
static FILE *open_file(char *, char *);
static handle_t new_handle(void);

// function wrapper around fopen
static FILE *open_file(char *path, char *mode)
{
	FILE *fp;
	fp = fopen(path, mode);

	if (fp == NULL) {
		fprintf(stderr, "No such file: %s\n", path);
		exit(1);
	}

	return fp;
}

static handle_t new_handle(void)
{
	handle_t h = (handle_t)malloc(sizeof(struct handle));
	assert(h);
	h->buf = NULL;
	h->size = 0;

	return h;
}

handle_t parse(char *path)
{
	FILE *fp = open_file(path, "r");

	handle_t h = new_handle();
	// malloc buffer
	add_size(h);

	while (fscanf(fp, "%ms", &(h->buf[h->size - 1])) != EOF)
		add_size(h);

	// reduce size
	h->size--;
	fclose(fp);
	return h;
}

handle_t parse_url(char *path, char *start_tag, char *end_tag)
{
	FILE *fp = open_file(path, "r");

	handle_t h = new_handle();
	// malloc buffer
	add_size(h);

	char *buf;
	int read_buf= 0;

	// %m[^\n]\n reads whole line until '\n' (not including \n)
	while (fscanf(fp, "%m[^\n]\n", &buf) != EOF) {
		// start reading next iteration
		if (strcmp(buf, end_tag) == 0) read_buf = !read_buf;
		if (read_buf) {
			// use space as delimiter
			char *token = strtok(buf, " ");

			while (token != NULL) {
				h->buf[h->size - 1] = (char *)malloc(strlen(token) + 1);
				strcpy(h->buf[h->size - 1], token);
				add_size(h);
				token = strtok(NULL, " ");
			}
		}
		// stop reading next iteration
		if (strcmp(buf, start_tag) == 0) read_buf = !read_buf;
		free(buf);
	}

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
