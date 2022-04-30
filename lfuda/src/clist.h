#ifndef LFUDA_CLIST_H
#define LFUDA_CLIST_H

// Cache list utility function header

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

// Frequency(more accurately "key") node stores its key and another list with elements that have identical weights
typedef dl_list_t freq_list_t;
typedef dl_node_t freq_node_t;

// Local nodes all have the same key and store the pointer to its parent frequency node and it's own frequency as well
// as a pointer to cached data
typedef dl_list_t local_list_t;
typedef dl_node_t local_node_t;

freq_node_t freq_node_init(size_t key);
void freq_node_free(freq_node_t node_);

size_t freq_node_get_key(freq_node_t node_);

typedef struct {
    size_t frequency;
    void *cached;
} local_node_data_t;

// Init local node with pointer to a root node and fam data with pointer to cached data
local_node_t local_node_init(freq_node_t root_node, local_node_data_t fam);

// Utility functions for working with frequency and local lists
// Initialize frequency node with key
freq_node_t freq_node_init(size_t key);

// Get frequency node key
size_t freq_node_get_key(freq_node_t node_);

// Get local list pointed to by the frequency node
local_list_t freq_node_get_local(freq_node_t node_);

// Set local list pointed to by the frequency node
void freq_node_set_local(freq_node_t node_, local_list_t local);

// Initialize local node with local_node_data
local_node_t local_node_init(freq_node_t root_node, local_node_data_t fam);

// Get local node data
local_node_data_t local_node_get_fam(local_node_t node_);

// Set local node data
void local_node_set_fam(local_node_t node_, local_node_data_t fam_);

#ifdef __cplusplus
}
#endif

#endif