/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <gerasimenko.dv@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet some day, and you think this stuff is
 * worth it, you can buy us a beer in return.
 * ----------------------------------------------------------------------------
 */

#include "lfuda.h"
#include "basecache.h"
#include "clist.h"
#include "rbtree.h"

#include "memutil.h"
#include <assert.h>

//============================================================================================================
struct lfuda_s {
    base_cache_t base;
    rb_tree_t rbtree;
    size_t age;
};

typedef struct {
    size_t key;
    freq_node_t freq_node;
} rb_entry_t;

//============================================================================================================

rb_entry_t *rb_entry_init(size_t key, freq_node_t freq_node) {
    rb_entry_t *entry = calloc_checked(1, sizeof(rb_entry_t));

    entry->freq_node = freq_node;
    entry->key = key;

    return entry;
}

//============================================================================================================

// Reinterpret rb_entry_t as size_t
int rb_entry_cmp(void *node1_, void *node2_) {
    assert(node1_);
    assert(node2_);
    size_t key1 = *(size_t *)node1_, key2 = *(size_t *)node2_;
    return (int)key1 - (int)key2;
}

//============================================================================================================

// Get cache structure and local_data of the local node.
// Returns next key of the localnode

static size_t lfuda_get_next_key(struct lfuda_s *cache, local_node_t localnode) {
    assert(cache);
    local_node_data_t local_data = local_node_get_data(localnode);
    return local_data.frequency + cache->age;
}

//============================================================================================================

// Remove freq node from the freq list and from the red black tree if local list of this freq node is empty

static inline void lfuda_remove_freq_if_empty(struct lfuda_s *lfuda, freq_node_t root_node) {
    assert(lfuda);
    assert(root_node);

    local_list_t local_list = freq_node_get_local(root_node);
    int freq_key = freq_node_get_key(root_node);

    if (dl_list_is_empty(local_list)) {
        rb_entry_t *entry = rb_tree_remove(lfuda->rbtree, &freq_key);
        free(entry);
        local_list_free(local_list);
        freq_node_free(dl_list_remove(lfuda->base.freq_list, root_node));
    }
}

//============================================================================================================

// Remove local node from the cache

static entry_t *lfuda_remove(base_cache_t *cache, local_node_t node, void **index) {
    assert(cache);
    assert(node);

    // Index should correspond to the local node
    entry_t *free_entry = hashtab_remove(cache->table, index);

    freq_node_t freq_node = local_node_get_freq_node(node);
    local_list_t local_list = freq_node_get_local(freq_node);
    dl_list_remove(local_list, node);

    // If list becomes empty, then free it and remove it
    lfuda_remove_freq_if_empty(cache, freq_node);

    return free_entry;
}

//============================================================================================================

// Returns the freq node to insert localnode into

static freq_node_t lfuda_next_freq_node_init(struct lfuda_s *lfuda, local_node_t localnode) {
    assert(lfuda);
    assert(localnode);

    // In this case strict-aliasing does not apply, because base_cache_t is the first member of lfuda_s struct
    base_cache_t *basecache = &lfuda->base;

    size_t nextkey = lfuda_get_next_key(lfuda, localnode);
    rb_entry_t *next_freq = rb_tree_closest_left(lfuda->rbtree, &nextkey);

    if (!next_freq) {
        freq_node_t new_freq_node = freq_node_init(nextkey);
        rb_tree_insert(lfuda->rbtree, rb_entry_init(nextkey, new_freq_node));
        dl_list_push_front(basecache->freq_list, new_freq_node);
        return new_freq_node;
    }

    freq_node_t next_freq_node = next_freq->freq_node;
    if (next_freq->key == nextkey) {
        return next_freq_node;
    }

    freq_node_t new_freq_node = freq_node_init(nextkey);

    rb_tree_insert(lfuda->rbtree, rb_entry_init(nextkey, new_freq_node));
    dl_list_insert_after(basecache->freq_list, next_freq_node, new_freq_node);

    return new_freq_node;
}

//============================================================================================================

lfuda_t lfuda_init(cache_init_t init) {
    struct lfuda_s *lfuda = calloc_checked(1, sizeof(struct lfuda_s));

    base_cache_init(&lfuda->base, init);

    lfuda->rbtree = rb_tree_init(RBTREE_CMP_F(rb_entry_cmp));
    lfuda->age = 0;

    return lfuda;
}

//============================================================================================================

void lfuda_free(lfuda_t cache_) {
    struct lfuda_s *lfuda = (struct lfuda_s *)cache_;

    base_cache_free(&lfuda->base);

    rb_tree_free(lfuda->rbtree, free);

    free(lfuda);
}

//============================================================================================================

// Returns the freq node for inserting new local node

freq_node_t lfuda_first_freq_node_init(struct lfuda_s *lfuda) {
    assert(lfuda);
    struct base_cache_s *basecache = &lfuda->base;

    // Get first node of the frequency list
    freq_node_t first_freq = dl_list_get_first(basecache->freq_list);

    // When a new object is added, its key should be set to cache's age
    if (!first_freq || freq_node_get_key(first_freq) != lfuda->age) {
        // create new freq node and insert it in a red-black tree
        freq_node_t new_freq_node = freq_node_init(lfuda->age);
        rb_tree_insert(lfuda->rbtree, rb_entry_init(lfuda->age, new_freq_node));
        dl_list_push_front(basecache->freq_list, new_freq_node);
        return new_freq_node;
    }

    return first_freq;
}

//============================================================================================================

static void *lfuda_get_case_found_impl(struct lfuda_s *lfuda, local_node_t found) {
    struct base_cache_s *basecache = &lfuda->base;

    // Increment cache hits
    basecache->hits += 1;

    // Get data and root node of local node
    local_node_data_t local_data = local_node_get_data(found);
    freq_node_t root_node = local_node_get_freq_node(found);

    // Increment frequency of the found cache entry
    local_data.frequency += 1;

    // Remove local node from this local list and move to the local list
    // with another key (not just incremented)
    local_list_t local_list = freq_node_get_local(root_node);
    dl_list_remove(local_list, found);
    lfuda_remove_freq_if_empty(lfuda, root_node);
    // Find next freq node (or create it)
    freq_node_t next_freq = lfuda_next_freq_node_init(lfuda, found);
    dl_list_push_front(freq_node_get_local(next_freq), found);

    local_data.root_node = next_freq;
    local_node_set_data(found, local_data);

    return local_data.cached;
}

//============================================================================================================

static void *lfuda_get_case_is_not_full_impl(struct lfuda_s *lfuda, void *index) {
    struct base_cache_s *basecache = &lfuda->base;

    void *page = (basecache->slow_get ? basecache->slow_get(index) : NULL);
    local_node_t toinsert = NULL;
    char *curr_data_ptr = NULL;

    // Set current data pointer
    curr_data_ptr = basecache->cached_data + basecache->data_size * basecache->curr_top;
    // Increment curr_top
    basecache->curr_top += 1;

    // Intialize local_data with current information
    local_node_data_t local_data = {0};
    local_data.frequency = 1;
    local_data.index = index;
    local_data.cached = curr_data_ptr;

    freq_node_t first_freq = lfuda_first_freq_node_init(lfuda);
    if (basecache->data_size) {
        local_data.cached = curr_data_ptr;
    }

    toinsert = local_node_init(local_data);
    local_data.root_node = first_freq;

    base_cache_insert(basecache, first_freq, toinsert, local_data, NULL);

    if (basecache->data_size) {
        memcpy(curr_data_ptr, page, basecache->data_size);
    }

    return page;
}

//============================================================================================================

static void *lfuda_get_case_full_impl(struct lfuda_s *lfuda, void *index) {
    struct base_cache_s *basecache = &lfuda->base;

    void *page = (basecache->slow_get ? basecache->slow_get(index) : NULL);
    char *curr_data_ptr = NULL;

    // Intialize local_data with current information
    local_node_data_t local_data = {0};
    local_data.frequency = 1;
    local_data.index = index;

    // Get first node of frequency list
    // (according to the LFU-DA policy we must evict entry with lowest freq)
    freq_node_t first_freq = dl_list_get_first(basecache->freq_list);

    // According to the LFU-DA policy we must evict least recently used entry in
    // the list of nodes with the same freq
    local_node_t toevict = dl_list_get_last(freq_node_get_local(first_freq));
    local_node_data_t evicted_data = local_node_get_data(toevict);

    lfuda->age = freq_node_get_key(evicted_data.root_node);
    curr_data_ptr = local_data.cached = evicted_data.cached;

    entry_t *free_entry = lfuda_remove(basecache, toevict, &local_data.index);
    freq_node_t next_freq = lfuda_first_freq_node_init(lfuda);

    local_data.root_node = next_freq;
    local_data.cached = evicted_data.cached;

    base_cache_insert(basecache, next_freq, toevict, local_data, free_entry);

    if (basecache->data_size) {
        memcpy(curr_data_ptr, page, basecache->data_size);
    }

    return page;
}

//============================================================================================================

void *lfuda_get(lfuda_t cache_, void *index) {
    struct lfuda_s *lfuda = (struct lfuda_s *)cache_;
    // In this case strict-aliasing does not apply, because base_cache_t is the first member of lfuda_s struct
    struct base_cache_s *basecache = &lfuda->base;

    assert(lfuda);
    assert(index);

    local_node_t found = base_cache_lookup(basecache, &index);

    // 1. There is already a cache entry, then we promote it and move futher along the frequency list
    if (found) {
        return lfuda_get_case_found_impl(cache_, found);
    }

    // If we get here, then the key is not present in the cache. In this case we call slow_get if it is
    // provided and insert the key into the cache, while optionally copying the data.

    // 2. In this case cache is not full and we can just insert the node with initial frequency
    if (basecache->curr_top < basecache->size) {
        return lfuda_get_case_is_not_full_impl(lfuda, index);
    }
    // 3. In this case the cache is already full and we need to evict some entry from
    // cache according to the LFU-DA policy
    else {
        return lfuda_get_case_full_impl(lfuda, index);
    }
}

//============================================================================================================

size_t lfuda_get_hits(lfuda_t cache_) {
    // In this case strict-aliasing does not apply, because base_cache_t is the first member of lfuda_s struct
    base_cache_t *cache = (base_cache_t *)cache_;

    assert(cache);

    return cache->hits;
}