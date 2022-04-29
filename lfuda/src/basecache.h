#ifndef LFUDA_BASECACHE_H
#define LFUDA_BASECACHE_H

#include "cache.h"
#include "dllist.h"
#include "hashtab.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif

// Refer to http://dhruvbird.com/lfu.pdf for more information
typedef dl_list_t freq_list_t;
typedef dl_node_t freq_node_t;

typedef dl_list_t local_list_t;
typedef dl_node_t local_node_t;

freq_node_t freq_node_init(size_t key);
void freq_node_free(freq_node_t node_);

size_t freq_node_get_key(freq_node_t node_);

local_node_t local_node_init(void *data);

// This definition should be moved to a header file private to the implementation of derived LFU and LFUDA classes
struct base_cache_s {
    hashtab_t table;
    freq_list_t freq_list;
    // This is a sort of virtual function that will be overriden in derived classes
    cache_get_priority_func_t priority_func;
    // For the time being this cache will support only entries of fixed size, which is fine at the moment
};

// Accepts ptr to a base_cache member in derived classes and returns it for compatibility
base_cache_t *base_cache_init(base_cache_t *cache, size_t size, size_t data_size, cache_init_t init);
void base_cache_free(base_cache_t *cache);

void *base_cache_get(base_cache_t *cache, void *index);

#ifdef __cplusplus
}
#endif

#endif