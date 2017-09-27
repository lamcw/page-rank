#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "invindex.h"

static void get_ii(handle_t);

int main(void)
{
	handle_t cltn = parse("url/collection.txt");

	get_ii(cltn);
	free_handle(cltn);
}

static void get_ii(handle_t h)
{
	for (int i = 0; i < handle_size(h); i++) {
		char *fname = malloc(strlen(getbuf(h, i)) + 20);
		sprintf(fname, "url/%s.txt", getbuf(h, i));

		handle_t hd = parse_url(fname, "#start Section-2", "#end Section-2");
		normalise(hd);
		free_handle(hd);
		free(fname);
	}
}
