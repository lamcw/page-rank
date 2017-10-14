#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>

#include "parser.h"
#include "invindex.h"
#include "urltable.h"

#ifndef DUMP_ERR
#define DUMP_ERR(ptr, str)				\
	if (ptr == NULL) {				\
		perror(str);				\
		exit(EXIT_FAILURE);			\
	}
#endif

typedef struct {
	char *url;
	double tfidf;
} tfidf_t;

static double tf(char *, handle_t);
static double idf(char *, handle_t);
static double tfidf(char *, handle_t, handle_t);
static tfidf_t *copy_url(url_t *, int);
static void print_tfidf(tfidf_t *, int);
static void sortby_tfidf(tfidf_t *, int, char **, handle_t);
static void free_tfidf_arr(tfidf_t *, int);
static char *str_lower(char *str);

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [search_terms]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int nquery = argc - 1;
	char **query = &argv[1];

	// normalise words
	for (int i = 0; i < nquery; i++)
		str_lower(query[i]);

	// init data structures
	handle_t cltn = parse("collection.txt");
	invindex_t ind = read_index("invertedIndex.txt");
	urltable_t t = new_table(nquery);

	// find associated url for each keyword
	// insert those urls into table
	for (int i = 0; i < nquery; i++) {
		int row_size = 0;
		char **urls = url_for(ind, query[i], &row_size);
		if (urls)
			insert_many(t, i, urls, row_size);
		else
			nquery--;
	}
	// count number of repeated urls in table
	set_count(t);

	int urlsize = 0;
	// take union of urls 
	url_t *url = table_to_arr(t, &urlsize);

	for (int i = nquery; i > 0; i--) {
		int subarr_size = 0;
		// disect array into partition based on "url count"
		url_t *subarr = partition_arr(url, urlsize, i, &subarr_size);
		tfidf_t *tfidf = copy_url(subarr, subarr_size);
		sortby_tfidf(tfidf, subarr_size, query, cltn);
		print_tfidf(tfidf, subarr_size);
		free_tfidf_arr(tfidf, subarr_size);
		free(subarr);
	}

	// release memory
	free_table_arr(url, urlsize);
	free_index(ind);
	free_table(t);
	free_handle(cltn);
	return 0;
}

static tfidf_t *copy_url(url_t *url, int size)
{
	tfidf_t *arr = malloc(size * sizeof(tfidf_t));
	DUMP_ERR(arr, "malloc failed");

	for (int i = 0; i < size; i++) {
		arr[i].url = malloc(strlen(get_arr_url(url[i])) + 1);
		DUMP_ERR(arr, "malloc failed");
		strcpy(arr[i].url, get_arr_url(url[i]));
		arr[i].tfidf = 0;
	}

	return arr;
}

static void print_tfidf(tfidf_t *tfidf, int size)
{
	static int count = 0;
	for (int i = 0; i < size && count < 30; i++) {
		printf("%s %.6f\n", tfidf[i].url, tfidf[i].tfidf);
		count++;
	}
}

int _tfidf_cmp(const void *a, const void *b)
{
	return (*(tfidf_t *)b).tfidf > (*(tfidf_t *)a).tfidf;
}

static void sortby_tfidf(tfidf_t *arr, int size, char **query, handle_t cltn)
{
	for (int i = 0; i < size; i++) {
		// open url source
		char *fname = malloc(strlen(arr[i].url) + 4);
		sprintf(fname, "%s.txt", arr[i].url);
		handle_t page = parse_url(fname, "#start Section-2", "#end Section-2");

		// for each search term
		// calculate the summation of tfidf for this url
		for (int j = 0; query[j] != NULL; j++) {
			arr[i].tfidf += tfidf(query[j], page, cltn);
		}

		free(fname);
		free_handle(page);
	}
	// sort by tfidf in descending order
	qsort(arr, size, sizeof(tfidf_t), _tfidf_cmp);
}

static double tf(char *word, handle_t page)
{
	int count = 0;;
	for (int i = 0; i < handle_size(page); i++) {
		if (strcmp(word, getbuf(page, i)) == 0)
			count++;
	}
	return (double) count / (double) handle_size(page);
}

// given a corpus, find its idf with @word
static double idf(char *word, handle_t cltn)
{
	int count = 0;
	for (int i = 0; i < handle_size(cltn); i++) {
		char *fname = malloc(strlen(getbuf(cltn, i)) + 4);
		DUMP_ERR(fname, "malloc failed");
		sprintf(fname, "%s.txt", getbuf(cltn, i));

		handle_t page = parse_url(fname, "#start Section-2", "#end Section-2");
		normalise(page);

		for (int j = 0; j < handle_size(page); j++) {
			if (strcmp(word, getbuf(page, j)) == 0) {
				count++;
				break;
			}
		}
		free(fname);
		free_handle(page);
	}
	assert(count > 0);
	return log10((double) handle_size(cltn) / (double) abs(count));
}

static double tfidf(char *word, handle_t page, handle_t cltn)
{
	normalise(page);
	return tf(word, page) * idf(word, cltn);
}

static void free_tfidf_arr(tfidf_t *arr, int size)
{
	for (int i = 0; i < size; i++)
		free(arr[i].url);
	free(arr);
}

static char *str_lower(char *str)
{
	int i = 0;
	while (str[i]) {
		str[i] = tolower((unsigned char)str[i]);
		i++;
	}
	return str;
}
