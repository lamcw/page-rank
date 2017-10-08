// parse text files

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "parser.h"

static void add_size(handle_t);
static FILE *open_file(char *, char *);
static handle_t new_handle(void);
static char *str_lower(char *str);
static void rmoccur(char *str, char c);

struct _handle {
	int size;
	int max_size;
	char **buf;
};

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
	handle_t h = malloc(sizeof(struct _handle));
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
		if (strcmp(buf, end_tag) == 0) {
			free(buf);
			// break early to avoid more fscanf
			break;
		}
		if (read_buf) {
			// use space as delimiter
			char *token = strtok(buf, " ");

			while (token != NULL) {
				h->buf[h->size] = malloc(strlen(token) + 1);
				strcpy(h->buf[h->size], token);
				h->size++;
				if (h->size >= h->max_size) add_size(h);
				token = strtok(NULL, " ");
			}
		}
		// start reading next iteration
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
	char **tmp = realloc(h->buf, new_size * sizeof(char *));

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
	assert(h);
	for (int i = 0; i < h->size; i++)
		printf("%s\n", h->buf[i]);
}

char *getbuf(handle_t h, int id)
{
	assert(h);
	return h->buf[id];
}

int handle_size(handle_t h)
{
	assert(h);
	return h->size;
}

static char *str_lower(char *str)
{
	int i = 0;
	while (str[i]) {
		// cast to unsigned char to avoid non-English character that
		// leads to undefined behavior
		str[i] = tolower((unsigned char)str[i]);
		i++;
	}
	return str;
}

static void rmoccur(char *str, char c)
{
	// replace first occurance of @c with null terminating char
	for (char *p = str; *p; ++p)
		if (*p != c) *str++ = *p;
	*str = '\0';
}

void normalise(handle_t h)
{
	assert(h);
	// characters to be removed from a string
	const char *rm = ".,;?";

	for (int i = 0; i < h->size; i++) {
		str_lower(h->buf[i]);
		for (int j = 0; j < (int)strlen(rm); j++)
			rmoccur(h->buf[i], rm[j]);
	}
}
