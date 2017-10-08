#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "invindex.h"
#include "urltable.h"

// pagerank struct
typedef struct _pr {
	char *url;
	double pr;
} pr_t;

static int count_lines(FILE *f);
static pr_t *parse_pr(char *path, int *size);
static void free_pr(pr_t *arr, int size);

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [search_terms]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const int nquery = argc - 1;
	char **query = &argv[1];

	invindex_t in = read_index("invertedIndex.txt");
	urltable_t t = new_table(nquery);

	for (int i = 0; i < nquery; i++) {
		int row_size = 0;
		char **urls = url_for(in, query[i], &row_size);
		// batch insert url
		insert_many(t, i, urls, row_size);
	}
	set_count(t);

	int urlsize = 0;
	// merge table rows into one array
	url_t *arr = table_to_arr(t, &urlsize);
	int pr_size = 0;
	pr_t *pr = parse_pr("pagerankList.txt", &pr_size);

	free_pr(pr, pr_size);
	free_table_arr(arr, urlsize);
	free_table(t);
	free_index(in);
	return 0;
}

// count number of lines in file
static int count_lines(FILE *f)
{
	int lines = 0;
	char ch;
	while ((ch = fgetc(f)) != EOF)
		if (ch == '\n')
			lines++;
	// reset file pointer back to the start
	fseek(f, 0, SEEK_SET);
	return lines;
}

static pr_t *parse_pr(char *path, int *size)
{
	FILE *fp;
	fp = fopen(path, "r");

	if (fp == NULL) {
		perror("Failed to open file");
		exit(EXIT_FAILURE);
	}

	*size = count_lines(fp);
	pr_t *arr = malloc((*size + 1) * sizeof(pr_t));

	if (arr == NULL) {
		perror("malloc failed");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < *size; i++) {
		arr[i].url = NULL;
		arr[i].pr = 0;
	}

	int tmp = 0;
	int i = 0;
	while (fscanf(fp, "%m[^,],%d,%lf\n", &(arr[i].url), &tmp, &(arr[i].pr)) != EOF)
		i++;

	// in the end arr has one empty element at the back of the array
	// i can choose to realloc the array here but it seems pretty
	// unnecessary to do so

	fclose(fp);
	return arr;
}

static void free_pr(pr_t *arr, int size)
{
	// is i < size because the last url in struct is NULL
	for (int i = 0; i < size; i++) {
		free(arr[i].url);
	}
	free(arr);
}
