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
int get_url_id(url_t *, int, char *);
void print_arr(url_t *, int);
void free_table_arr(url_t *, int);

#endif
