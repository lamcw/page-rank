/* includes 2 data structure that utilizes 2 different structs
 * 
 * 	1. table (2-d array)
 * 	2. 1-d array
 *
 * 1. table
 * 	row - keyword
 * 	column - associated url with that keyword
 *
 * 	for example:
 * 		------------------------------------------
 * 		|mars   | url12(1) | url31(2) |          |
 * 		|design | url11(1) | url31(2) | url32(1) |
 * 		-----------------------------------------
 *
 * 	- url is sorted by alphabetical order.
 * 	- number in parenthesis is "url count", i.e. how many urls in the table
 * 	  are the same
 *
 * 2. 1-d array
 * 	merges every column in the table into a single 1-d array with no
 * 	repeated elements, sorted by url count
 * 	
 * 	for example:
 * 	arr = url31(2) url12(1) url11(1) url32(1)
 */

#ifndef URL_TABLE_H
#define URL_TABLE_H

typedef struct _url *url_t;
typedef struct _url_table *urltable_t;

urltable_t new_table(int);
void insert_many(urltable_t, int, char **, int);
void show_table(urltable_t);
void free_table(urltable_t);
void set_count(urltable_t t);
url_t *table_to_arr(urltable_t, int *);
url_t *partition_arr(url_t *, int, int, int *);
int in_arr(url_t *, int, char *);
char *get_arr_url(url_t);
void print_arr(url_t *, int);
void free_table_arr(url_t *, int);

#endif
