#ifndef COUNTER_H
#define COUNTER_H

#include "hashtab.h"
#include <stddef.h>

struct counter_s;

struct counter_s *counter_init();
void counter_item_add(struct counter_s *counter, char *key);
unsigned counter_item_get_count(struct counter_s *counter, char *key);
void counter_free(struct counter_s *counter, int free_table);
hashtab_t counter_get_hashtable(struct counter_s *counter);

#endif