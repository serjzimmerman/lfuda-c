#ifndef LFUDA_CLIST_H
#define LFUDA_CLIST_H

// Cache list utility function header

#include "cache.h"
#include "dllist.h"
#include "hashtab.h"
#include <assert.h>
#include <string.h>

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

typedef struct {
    size_t frequency;
    void *cached;
} local_node_data_t;

// Utility functions for working with frequency and local lists

// Init local node with pointer to a root node and fam data with pointer to cached data
static inline local_node_t local_node_init(freq_node_t root_node, local_node_data_t fam) {
    assert(root_node);
    freq_node_t node = dl_node_init_fam(NULL, sizeof(local_node_data_t), &fam);
    dl_node_set_data(node, root_node);
    return node;
}

// Initialize frequency node with key
static inline freq_node_t freq_node_init(size_t key) {
    freq_node_t node = dl_node_init_fam(NULL, sizeof(size_t), &key);
    dl_node_set_data(node, dl_list_init());
    return node;
}

// Get frequency node key
static inline size_t freq_node_get_key(freq_node_t node_) {
    size_t key = 0;
    assert(node_);
    memcpy(&key, dl_node_get_fam(node_), sizeof(size_t));
    return key;
}

// Get local list pointed to by the frequency node
static inline local_list_t freq_node_get_local(freq_node_t node_) {
    assert(node_);
    return dl_node_get_data(node_);
}

// Set local list pointed to by the frequency node. local can be equal to NULL
static inline void freq_node_set_local(freq_node_t node_, local_list_t local) {
    assert(node_);
    dl_node_set_data(node_, local);
}

// Get local node data
static inline local_node_data_t local_node_get_fam(local_node_t node_) {
    local_node_data_t result = {0};
    assert(node_);
    memcpy(&result, dl_node_get_fam(node_), sizeof(local_node_data_t));
    return result;
}

// Set local node data
static inline void local_node_set_fam(local_node_t node_, local_node_data_t fam_) {
    assert(node_);
    void *fam = dl_node_get_fam(node_);
    assert(fam);
    memcpy(fam, &fam_, sizeof(local_node_data_t));
}

#ifdef __cplusplus
}
#endif

#endif