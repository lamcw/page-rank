#include <stdio.h>
#include <stdlib.h>

#include "invindex.h"

struct _invurl {
	char *word;
	char **urls;
};

struct _invindex {
	invurl_t *url;
	int size;
};
