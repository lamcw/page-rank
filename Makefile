CC=gcc
CFLAGS= -Wall -Werror -g -std=c11
OBJS=parser.o graph.o url.o

pagerank: pagerank.o $(OBJS)
	$(CC) $(CFLAGS) -o pagerank pagerank.c $(OBJS)

parser.o: parser.c parser.h

graph.o: graph.c graph.h

url.o: url.c url.h

clean:
	rm -f *.o pagerank *.dSYM
