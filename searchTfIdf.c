#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "parser.h"
#include "invindex.h"

double tf(char *, handle_t);
double idf(char *, handle_t);
double tfidf(char *, handle_t, handle_t);

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [search_terms]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	handle_t cltn = parse("url/collection.txt");
	invindex_t ind = read_index("invertedIndex.txt");

	free_index(ind);
	free_handle(cltn);
	return 0;
}

double tf(char *word, handle_t page)
{
	int count = 0;;
	for (int i = 0; i < handle_size(page); i++)
		if (strcmp(word, getbuf(page, i)) == 0)
			count++;
	return (double) count / (double) handle_size(page);
}

double idf(char *word, handle_t cltn)
{
	int count = 0;
	for (int i = 0; i < handle_size(cltn); i++) {
		char *fname = malloc(strlen(getbuf(cltn, i)) + 20);

		if (fname == NULL) {
			perror("malloc failed");
			exit(EXIT_FAILURE);
		}

		sprintf(fname, "url/%s.txt", getbuf(cltn, i));

		handle_t page = parse_url(fname, "#start Section-1", "#end Section-1");

		for (int j = 0; j < handle_size(page); j++)
			if (strcmp(word, getbuf(page, i)) == 0) {
				count++;
				break;
			}

		free_handle(page);
		free(fname);
	}
	assert(count > 0);
	return log10((double) handle_size(cltn) / (double) abs(count));
}

double tfidf(char *word, handle_t page, handle_t cltn)
{
	return tf(word, page) * idf(word, cltn);
}
