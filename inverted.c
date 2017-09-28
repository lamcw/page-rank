#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "invindex.h"

static invindex_t get_invindex(handle_t);

int main(void)
{
	handle_t cltn = parse("url/collection.txt");

	invindex_t index = get_invindex(cltn);
	output_index(index, "invertedIndex.txt");
	//show_index(index);
	free_index(index);
	free_handle(cltn);
}

static invindex_t get_invindex(handle_t h)
{
	invindex_t index = newindex();

	for (int i = 0; i < handle_size(h); i++) {
		char *fname = malloc(strlen(getbuf(h, i)) + 20);
		sprintf(fname, "url/%s.txt", getbuf(h, i));

		handle_t hd = parse_url(fname, "#start Section-2", "#end Section-2");
		normalise(hd);
		for (int j = 0; j < handle_size(hd); j++) {
			add_entry(index, getbuf(hd, j), getbuf(h, i));
		}
		free_handle(hd);
		free(fname);
	}

	return index;
}
