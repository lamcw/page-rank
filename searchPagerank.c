#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "invindex.h"
#include "urltable.h"

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
		insert_many(t, i, urls, row_size);
	}
	set_count(t);

	int size = 0;
	url_t *arr = table_to_arr(t, &size);
	print_arr(arr, size);

	/* show_table(t); */
	free_table_arr(arr, size);
	free_table(t);
	free_index(in);
	return 0;
}
