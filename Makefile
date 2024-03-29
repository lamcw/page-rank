CC=gcc
CFLAGS= -Wall -Werror -g -std=c11 -lm

all: pagerank inverted searchPagerank searchTfIdf

searchTfIdf: searchTfIdf.c invindex.o parser.o urltable.o

searchPagerank: searchPagerank.c invindex.o urltable.o

pagerank: pagerank.c parser.o graph.o url.o

inverted: inverted.c parser.o invindex.o

parser.o: parser.c parser.h

graph.o: graph.c graph.h

url.o: url.c url.h

invindex.o: invindex.c invindex.h

urltable.o: urltable.c urltable.h

clean:
	rm -f *.o pagerank inverted searchPagerank searchTfIdf *.dSYM
