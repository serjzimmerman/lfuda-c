#ifndef LFUDA_BASECACHE_H
#define LFUDA_BASECACHE_H

#include "cache.h"
#include "dllist.h"
#include "hashtab.h"

#include "clist.h"
#include <stddef.h>

// Base cache types private to the library files
struct base_cache_s;
typedef struct base_cache_s base_cache_t;

// Refer to http://dhruvbird.com/lfu.pdf for more information

// This definition should be moved to a header file private to the implementation of derived LFU and LFUDA classes
struct base_cache_s {
    cache_get_page_t slow_get;
    hashtab_t table;
    freq_list_t freq_list;

    size_t size;
    size_t data_size;

    size_t hits;
    size_t curr_top;

    // For the time being this cache will support only entries of fixed size, which is fine at the moment
    char *cached_data;
};

// Accepts ptr to a base_cache member in derived classes and returns it
base_cache_t *base_cache_init(base_cache_t *cache, cache_init_t init);
void base_cache_free(base_cache_t *cache);

// Gets local node with index
local_node_t base_cache_lookup(base_cache_t *cache, void **index);

// Removes local node from cache and returns it
local_node_t base_cache_remove(base_cache_t *cache, local_node_t node, void *index);

// Inserts toinsert at freqnode (at head)
void base_cache_insert(base_cache_t *cache, freq_node_t freqnode, local_node_t toinsert, void *index);

#endif