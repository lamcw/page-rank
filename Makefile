CC=gcc
CFLAGS= -Wall -Werror -g -std=c11

all: pagerank inverted

pagerank: pagerank.c parser.o graph.o url.o

inverted: inverted.c parser.o invindex.o

parser.o: parser.c parser.h

graph.o: graph.c graph.h

url.o: url.c url.h

invindex.o: invindex.c invindex.h

clean:
	rm -f *.o pagerank inverted *.dSYM
