#ifndef INVINDEX_H
#define INVINDEX_H

/* Inverted index can be implemented with set (and map) but I do not have time
 * to implement the whole data structure. Therefor simple array is used here
 */

typedef struct _invurl *invurl_t;
typedef struct _invindex *invindex_t;

invindex_t newindex(void);
invindex_t read_index(char *);
void add_entry(invindex_t, char *, char *);
void show_index(invindex_t ind);
void output_index(invindex_t ind, char*);
void free_index(invindex_t ind);

#endif
