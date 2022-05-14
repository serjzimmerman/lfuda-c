#ifndef LFUDA_CLIST_H
#define LFUDA_CLIST_H

// Cache list utility function header

#include "cache.h"
#include "dllist.h"
#include "hashtab.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "memutil.h"
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
    void *index;
    freq_node_t root_node;
} local_node_data_t;

// Utility functions for working with frequency and local lists

// Init local node with pointer to a root node and fam data with pointer to cached data
static inline local_node_t local_node_init(local_node_data_t data) {
    local_node_data_t *data_ptr = calloc_checked(1, sizeof(local_node_data_t));
    freq_node_t node = dl_node_init(data_ptr);

    *data_ptr = data;

    return node;
}

typedef struct {
    local_list_t local_list;
    size_t key;
} freq_node_data_t;

// Initialize frequency node with key
static inline freq_node_t freq_node_init(size_t key) {
    freq_node_data_t *data = calloc_checked(1, sizeof(freq_node_data_t));
    freq_node_t node = dl_node_init(data);

    data->local_list = dl_list_init();
    data->key = key;

    return node;
}

// Get frequency node key
static inline size_t freq_node_get_key(freq_node_t node_) {
    assert(node_);
    freq_node_data_t *data = (freq_node_data_t *)dl_node_get_data(node_);
    assert(data);
    return data->key;
}

// Get local list pointed to by the frequency node
static inline local_list_t freq_node_get_local(freq_node_t node_) {
    assert(node_);
    freq_node_data_t *data = (freq_node_data_t *)dl_node_get_data(node_);
    return data->local_list;
}

// Set local list pointed to by the frequency node. local can be equal to NULL
static inline void freq_node_set_local(freq_node_t node_, local_list_t local) {
    assert(node_);
    freq_node_data_t *data = (freq_node_data_t *)dl_node_get_data(node_);
    assert(data);
    data->local_list = local;
}

// Get local node data
static inline local_node_data_t local_node_get_fam(local_node_t node_) {
    assert(node_);
    local_node_data_t *data_ptr = (local_node_data_t *)dl_node_get_data(node_);
    assert(data_ptr);
    return *data_ptr;
}

// Set local node data
static inline void local_node_set_fam(local_node_t node_, local_node_data_t data) {
    assert(node_);
    local_node_data_t *data_ptr = (local_node_data_t *)dl_node_get_data(node_);
    assert(data_ptr);

    data_ptr->cached = data.cached;
    data_ptr->frequency = data.frequency;
    data_ptr->index = data.index;
}

static inline freq_node_t local_node_get_freq_node(local_node_t node_) {
    assert(node_);
    local_node_data_t *data = (local_node_data_t *)dl_node_get_data(node_);
    assert(data);
    return data->root_node;
}

static inline void local_node_set_freq_node(local_node_t node_, freq_node_t freqnode) {
    assert(node_);
    local_node_data_t *data = (local_node_data_t *)dl_node_get_data(node_);
    assert(data);
    data->root_node = freqnode;
}

static void local_node_free_data(void *data_) {
    local_node_data_t *data = (local_node_data_t *)data_;
    assert(data);
    free(data);
}

static void local_list_free(local_list_t list_) {
    assert(list_);
    dl_list_free(list_, local_node_free_data);
}

static void freq_node_free(freq_node_t node_) {
    assert(node_);
    freq_node_data_t *data = (freq_node_data_t *)dl_node_get_data(node_);
    free(data);
    free(node_);
}

static void freq_node_free_data(void *data_) {
    freq_node_data_t *data = (freq_node_data_t *)data_;
    assert(data);

    assert(data->local_list);
    local_list_free(data->local_list);

    free(data);
}

static void local_node_free(local_node_t node_) {
    assert(node_);
    local_node_data_t *data = (local_node_data_t *)dl_node_get_data(node_);
    free(data);
    free(node_);
}

static void freq_list_free(freq_list_t list_) {
    assert(list_);
    dl_list_free(list_, freq_node_free_data);
}

// Prefer to use this functions over any others

static inline local_node_data_t local_node_get_data(local_node_t node_) {
    assert(node_);
    local_node_data_t *data = (local_node_data_t *)dl_node_get_data(node_);
    assert(data);
    return *data;
}

static inline void local_node_set_data(local_node_t node_, local_node_data_t newdata) {
    assert(node_);
    local_node_data_t *data = (local_node_data_t *)dl_node_get_data(node_);
    assert(data);
    *data = newdata;
}

static inline freq_node_data_t freq_node_get_data(freq_node_t node_) {
    assert(node_);
    freq_node_data_t *data = (freq_node_data_t *)dl_node_get_data(node_);
    assert(data);
    return *data;
}

static inline void freq_node_set_data(freq_node_t node_, freq_node_data_t newdata) {
    assert(node_);
    freq_node_data_t *data = (freq_node_data_t *)dl_node_get_data(node_);
    assert(data);
    *data = newdata;
}

#endif