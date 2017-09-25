// parse text files

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parser.h"

static void add_size(handle_t);
static FILE *open_file(char *, char *);
static handle_t new_handle(void);

// function wrapper around fopen
static FILE *open_file(char *path, char *mode)
{
	FILE *fp;
	fp = fopen(path, mode);

	if (fp == NULL) {
		perror("Failed to open file");
		exit(EXIT_FAILURE);
	}

	return fp;
}

static handle_t new_handle(void)
{
	handle_t h = (handle_t)malloc(sizeof(handle));
	assert(h);
	h->buf = NULL;
	h->size = h->max_size = 0;

	return h;
}

handle_t parse(char *path)
{
	FILE *fp = open_file(path, "r");

	handle_t h = new_handle();
	// malloc buffer
	add_size(h);

	while (fscanf(fp, "%ms", &(h->buf[h->size])) != EOF) {
		h->size++;
		if (h->size >= h->max_size) add_size(h);
	}

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
		if (strcmp(buf, end_tag) == 0) {
			free(buf);
			// break early to avoid more fscanf
			break;
		}
		if (read_buf) {
			// use space as delimiter
			char *token = strtok(buf, " ");

			while (token != NULL) {
				h->buf[h->size] = (char *)malloc(strlen(token) + 1);
				strcpy(h->buf[h->size], token);
				h->size++;
				if (h->size >= h->max_size) add_size(h);
				token = strtok(NULL, " ");
			}
		}
		// stop reading next iteration
		if (strcmp(buf, start_tag) == 0) read_buf = !read_buf;
		free(buf);
	}

	fclose(fp);
	return h;

}

// doubles buf size
static void add_size(handle_t h)
{
	assert(h);
	// doubles the original size
	int new_size = h->size == 0 ? 1 : 2 * h->size;
	char **tmp = (char **)realloc(h->buf, new_size * sizeof(char *));

	if (tmp) {
		h->buf = tmp;
		h->max_size = new_size;
	} else {
		perror("realloc failed");
		exit(EXIT_FAILURE);
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

int handle_size(handle_t h)
{
	return h->size;
}
