#ifndef LFUDA_BASECACHE_H
#define LFUDA_BASECACHE_H

#include "cache.h"
#include "dllist.h"
#include "hashtab.h"

#include "clist.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif

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

    hash_func_t hash;
    entry_cmp_func_t cmp;
    // For the time being this cache will support only entries of fixed size, which is fine at the moment
    char *cached_data;
};

typedef struct {
    void *index;
    local_node_t local;
} entry_t;

// Accepts ptr to a base_cache member in derived classes and returns it
base_cache_t *base_cache_init(base_cache_t *cache, cache_init_t init);
void base_cache_free(base_cache_t *cache);

void *base_cache_get(base_cache_t *cache, void *index);

#ifdef __cplusplus
}
#endif

#endif