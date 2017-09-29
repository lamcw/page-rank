#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "invindex.h"

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [search_terms]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	invindex_t in = read_index("invertedIndex.txt");
	show_index(in);
	free_index(in);

	return 0;
}
