#ifndef RANK_H
#define RANK_H

typedef struct rank *rank_t;

rank_t new_rank(int);
int rank_size(rank_t);
void insert_rank(rank_t, char *);
void sort_rank(rank_t);
void print_rank(rank_t);
int max_size(int, rank_t *);
int in_rank(rank_t, char *);
rank_t merge_ranks(rank_t *, int);
void free_rank(rank_t);
#endif
