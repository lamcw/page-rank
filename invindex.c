#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "invindex.h"

// macro for dumping error messages
#ifndef DUMP_ERR
#define DUMP_ERR(ptr, str)				\
	if (ptr == NULL) {				\
		perror(str);				\
		exit(EXIT_FAILURE);			\
	}
#endif

// default array size for invindex_t tokens
#define DEFAULT_SIZE 30

struct _invurl {
	char *word;		// stores token, eg, "design"/"mars"
	char **urls;		// stores url
	int count;		// stores number of appearence of word
	int maxurl;		// maximum number of urls
};

// wrapper of an array of invurl_t
struct _invindex {
	invurl_t *tokens;	// stores an array of tokens
	int size;		// size of the above array
	int max_size;		// maximum size of *tokens
};

static void add_url(invurl_t tok, char *url);
static void sort_url(invurl_t u);
static void sort_tok(invindex_t ind);
static void add_urls_size(invurl_t u);
static void add_tokens_size(invindex_t ind);

/*
 * newindex - create an invindex adt
 * invindex_t is typedef'd in invindex.h
 * The struct wraps an array of invurl_t and the @size and @max_size of
 * the array
 */
invindex_t newindex(void) {
	invindex_t new = malloc(sizeof(struct _invindex));
	DUMP_ERR(new, "malloc failed");

	new->size = 0;
	new->max_size = DEFAULT_SIZE;
	new->tokens = malloc(new->max_size * sizeof(invurl_t));
	DUMP_ERR(new->tokens, "malloc failed");

	for (int i = 0; i < new->max_size; i++) {
		// init structs
		new->tokens[i] = malloc(sizeof(struct _invurl));
		new->tokens[i]->word = NULL;
		new->tokens[i]->urls = NULL;
		new->tokens[i]->count = 0;
		new->tokens[i]->maxurl = 0;
		add_urls_size(new->tokens[i]);
	}

	return new;
}

//  internal string comparison function
int _url_cmp(const void *a, const void *b)
{
	return strcmp(*(char **)a, *(char **)b);
}

// wrapper for qsort
// sorts only u->urls
static void sort_url(invurl_t u)
{
	qsort(u->urls, u->count, sizeof(char *), _url_cmp);
}

int _tok_cmp(const void *a, const void *b)
{
	invurl_t *ia = (invurl_t *)a;
	invurl_t *ib = (invurl_t *)b;
	return strcmp((*ia)->word, (*ib)->word);
}

// wrapper for qsort
// sorts only invindex_t tokens
static void sort_tok(invindex_t ind)
{
	qsort(ind->tokens, ind->size, sizeof(invurl_t), _tok_cmp);
}

/* add_urls_size - realloc url array
 * @u: an invurl_t struct that stores url array to be realloc'd
 *
 * This function increase the size of the array by a factor of 1.25
 * and update the maximum size of the array and pointer values respectively
 */
static void add_urls_size(invurl_t u)
{
	assert(u);

	const int new_size = u->count == 0 ? 5 : (double)u->count * 1.25;
	char **tmp = realloc(u->urls, new_size * sizeof(char *));
	DUMP_ERR(tmp, "realloc failed");

	u->urls = tmp;
	u->maxurl = new_size;
	for (int i = u->count; i < u->maxurl; i++)
		u->urls[i] = NULL;
}

static void add_url(invurl_t tok, char *url)
{
	assert(tok && url);

	// search for duplicated urls
	char **result = bsearch(&url, tok->urls, tok->count, \
				sizeof(char *), _url_cmp);
	if (!result) {
		tok->urls[tok->count] = malloc(strlen(url) + 1);
		strcpy(tok->urls[tok->count], url);
		sort_url(tok);
		tok->count++;
		if (tok->count >= tok->maxurl) add_urls_size(tok);
	}
}

void add_entry(invindex_t ind, char *word, char *url)
{
	assert(ind);
	assert(word && url);

	// create an target struct
	invurl_t key = malloc(sizeof(struct _invurl));
	key->word = malloc(strlen(word) + 1);
	strcpy(key->word, word);
	// check if word is in invindex
	invurl_t *tok = bsearch(&key, ind->tokens, ind->size, \
				sizeof(invurl_t), _tok_cmp);
	free(key->word);
	free(key);

	// found
	if (tok) {
		add_url(*tok, url);
		sort_tok(ind);
	} else {
		// add to a new token
		ind->tokens[ind->size]->word = malloc(strlen(word) + 1);
		strcpy(ind->tokens[ind->size]->word, word);
		add_url(ind->tokens[ind->size], url);
		sort_tok(ind);
		ind->size++;
		if (ind->size >= ind->max_size) add_tokens_size(ind);
	}
}

// increase invindex_t->tokens' size by a quarter
static void add_tokens_size(invindex_t ind)
{
	const int new_size = (double)ind->size * 1.25;
	invurl_t *tmp = realloc(ind->tokens, new_size * sizeof(invurl_t));
	DUMP_ERR(tmp, "realloc failed");

	ind->tokens = tmp;
	ind->max_size = new_size;

	for (int i = ind->size; i < new_size; i++) {
		ind->tokens[i] = malloc(sizeof(struct _invurl));
		ind->tokens[i]->word = NULL;
		ind->tokens[i]->urls = NULL;
		ind->tokens[i]->count = 0;
		ind->tokens[i]->maxurl = 0;
		add_urls_size(ind->tokens[i]);
	}
}

// print the structure of invindex_t
// for debugging
void show_index(invindex_t ind)
{
	assert(ind);
	for (int i = 0; i < ind->size; i++) {
		printf("%s: ", ind->tokens[i]->word);
		for (int j = 0; j < ind->tokens[i]->count; j++)
			printf("%s ", ind->tokens[i]->urls[j]);
		putchar('\n');
	}
}

// free all memory
void free_index(invindex_t ind)
{
	assert(ind);
	for (int i = 0; i < ind->max_size; i++) {
		free(ind->tokens[i]->word);
		for (int j = 0; j < ind->tokens[i]->maxurl; j++)
			free(ind->tokens[i]->urls[j]);
		free(ind->tokens[i]->urls);
		free(ind->tokens[i]);
	}
	free(ind->tokens);
	free(ind);
}

void output_index(invindex_t ind, char *path)
{
	assert(ind);
	FILE *fp = fopen(path, "w");

	for (int i = 0; i < ind->size; i++) {
		fprintf(fp, "%s ", ind->tokens[i]->word);
		for (int j = 0; j < ind->tokens[i]->count; j++)
			fprintf(fp, "%s ", ind->tokens[i]->urls[j]);
		fputc('\n', fp);
	}
	fclose(fp);
}
