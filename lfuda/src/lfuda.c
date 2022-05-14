//----------------------------------------------------------------------------------------------------------//
//                                                                                                          //
//                                         LFU-DA cache relization                                          //
//                                                                                                          //
//----------------------------------------------------------------------------------------------------------//
//                                         Authors:                                                         //
//                                             Romanov Alexander                                            //
//----------------------------------------------------------------------------------------------------------//

#include "lfuda.h"
#include "basecache.h"
#include "clist.h"
#include "rbtree.h"

#include "memutil.h"
#include <assert.h>

struct lfuda_s {
    base_cache_t base;
    rb_tree_t rbtree;
    size_t age;
};

//============================================================================================================

// compare 2 freq nodes by checking keys
int freq_node_cmp(freq_node_t node1_, freq_node_t node2_) {
    size_t key1 = freq_node_get_key(node1_);
    size_t key2 = freq_node_get_key(node2_);
    return (int)key1 - (int)key2;
}

//============================================================================================================

// Get cache structure and local_data of the local node.
// Return next key of the cached element

#define LFUDA_FREQ_COEF 1

static size_t lfuda_get_next_key(lfuda_t cache_, local_node_t localnode) {
    local_node_data_t local_data = local_node_get_fam(localnode);
    struct lfuda_s *cache = (struct lfuda_s *)cache_;
    size_t new_key = LFUDA_FREQ_COEF * local_data.frequency + cache->age;
    return new_key;
}

//============================================================================================================

static freq_node_t lfuda_next_freq_node_init(lfuda_t cache_, local_node_t localnode) {
    assert(cache_);
    assert(localnode);

    struct lfuda_s *lfuda = (struct lfuda_s *)cache_;
    // In this case strict-aliasing does not apply, because base_cache_t is the first member of lfuda_s struct
    base_cache_t *basecache = (struct base_cache_s *)cache_;

    size_t nextkey = lfuda_get_next_key(cache_, localnode);

    freq_node_t testnewfreq = freq_node_init(nextkey);
    freq_node_t newfreq = rb_tree_lookup(lfuda->rbtree, testnewfreq);

    // if a node with our new key already exists then we return this node and free testnewfreq
    // else we insert testnewnode in freq list and return testnewnode
    if (newfreq) {
        // remove testnewnode because we already have freq node with our key
        // dl_list_free function used to remove local list of that freq node
        dl_node_free(testnewfreq, NULL);
        return newfreq;
    }

    // insert testnewfreq in red black tree
    rb_tree_insert(lfuda->rbtree, testnewfreq);
    // insert testnewfreq in freq list
    freq_node_t insertafter = rb_tree_closest_left(lfuda->rbtree, testnewfreq);
    dl_list_insert_after(basecache->freq_list, insertafter, testnewfreq);

    return testnewfreq;
}

//============================================================================================================

lfuda_t lfuda_init(cache_init_t init) {
    struct lfuda_s *lfuda = calloc_checked(1, sizeof(struct lfuda_s));
    base_cache_init(&lfuda->base, init);
    lfuda->rbtree = rb_tree_init(freq_node_cmp);
    lfuda->age = 1;
    return lfuda;
}

//============================================================================================================

void lfuda_free(lfuda_t cache_) {
    struct lfuda_s *lfuda = (struct lfuda_s *)cache_;
    // In this case strict-aliasing does not apply, because base_cache_t is the first member of lfuda_s struct
    struct base_cache_s *basecache = (struct base_cache_s *)cache_;

    base_cache_free(basecache);

    rb_tree_free(lfuda->rbtree, NULL);

    free(lfuda);
}

//============================================================================================================

// Temporary define (needs to be replaced)
#define LFUDA_INITIAL_FREQ 1

freq_node_t lfuda_new_freq_node_init(lfuda_t cache_) {
    struct lfuda_s *lfuda = (struct lfuda_s *)cache_;
    // In this case strict-aliasing does not apply, because base_cache_t is the first member of lfuda_s struct
    struct base_cache_s *basecache = (struct base_cache_s *)cache_;

    assert(lfuda);

    // get first node of the frequency list
    freq_node_t first_freq = dl_list_get_first(basecache->freq_list);

    // check if first_freq exist and key of first freq is equal to intial freq
    if (!first_freq || freq_node_get_key(first_freq) != LFUDA_INITIAL_FREQ) {
        // create new freq node and insert it in a red-black tree
        freq_node_t new_freq = freq_node_init(LFUDA_INITIAL_FREQ);
        rb_tree_insert(lfuda->rbtree, new_freq);
        dl_list_push_front(basecache->freq_list, new_freq);
        return new_freq;
    }
    return first_freq;
}

//============================================================================================================

static void *lfuda_get_case_found_impl(struct lfuda_s *lfuda, local_node_t found) {
    struct base_cache_s *basecache = &lfuda->base;

    // Increment cache hits
    basecache->hits += 1;

    // Get data and root node of local node
    local_node_data_t local_data = local_node_get_fam(found);
    freq_node_t root_node = local_node_get_freq_node(found);

    // Increment frequency of the found cache entry
    local_data.frequency += 1;

    // Remove local node from this local list and move to the local list
    // with another key (not just incremented)
    local_list_t local_list = freq_node_get_local(root_node);
    dl_list_remove(local_list, found);

    // Remove root node from red-black tree and from freq_list if old local list is now empty
    if (dl_list_is_empty(local_list)) {
        rb_tree_remove(lfuda->rbtree, root_node);
        dl_list_free(local_list, NULL);
        dl_list_remove(basecache->freq_list, root_node);
    }

    // Find next freq node(or create it)
    freq_node_t next_freq = lfuda_next_freq_node_init(lfuda, found);
    dl_list_push_front(freq_node_get_local(next_freq), found);
    // Insert a pointer to next_freq node into the found node
    local_node_set_freq_node(found, next_freq);
    // Set updated information of the local node into it
    local_node_set_fam(found, local_data);
    // Return cached page
    return local_node_get_fam(found).cached;
}

//============================================================================================================

static void *lfuda_get_case_is_not_full_impl(struct lfuda_s *lfuda, void *index) {
    struct base_cache_s *basecache = &lfuda->base;

    void *page = (basecache->slow_get ? basecache->slow_get(index) : NULL);
    local_node_t toinsert = NULL;
    char *curr_data_ptr = NULL;

    // Intialize local_data with current information
    local_node_data_t local_data = {};
    local_data.frequency = LFUDA_INITIAL_FREQ;
    local_data.index = index;

    // Set current data pointer
    curr_data_ptr = basecache->cached_data + basecache->data_size * basecache->curr_top;
    // Increment curr_top
    basecache->curr_top += 1;

    freq_node_t first_freq = lfuda_new_freq_node_init(lfuda);
    if (basecache->data_size) {
        local_data.cached = curr_data_ptr;
    }

    toinsert = local_node_init(local_data);

    local_node_data_t data_to_insert = {};
    data_to_insert.cached = curr_data_ptr;
    data_to_insert.root_node = first_freq;
    data_to_insert.index = index;
    data_to_insert.frequency = LFUDA_INITIAL_FREQ;

    base_cache_insert(basecache, first_freq, toinsert, data_to_insert, NULL);

    if (basecache->data_size) {
        memcpy(curr_data_ptr, page, basecache->data_size);
    }

    return page;
}

//============================================================================================================

static void *lfuda_get_case_full_impl(struct lfuda_s *lfuda, void *index) {
    struct base_cache_s *basecache = &lfuda->base;

    void *page = (basecache->slow_get ? basecache->slow_get(index) : NULL);
    local_node_t toinsert = NULL;
    char *curr_data_ptr = NULL;

    // Intialize local_data with current information
    local_node_data_t local_data = {};
    local_data.frequency = LFUDA_INITIAL_FREQ;
    local_data.index = index;

    // Get first node of frequency list
    //(according to the LFU-DA policy we must evict entry with lowest freq)
    freq_node_t first_freq = dl_list_get_first(basecache->freq_list);
    // According to the LFU-DA policy we must evict least recently used entry in
    // the list of nodes with the same freq
    local_node_t toevict = dl_list_get_last(freq_node_get_local(first_freq));

    local_node_data_t evicted_data = local_node_get_fam(toevict);
    lfuda->age = evicted_data.frequency;
    local_data.cached = evicted_data.cached;
    curr_data_ptr = local_data.cached;

    entry_t *free_entry = base_cache_remove(basecache, toevict, &evicted_data.index);

    first_freq = lfuda_new_freq_node_init(lfuda);

    toinsert = local_node_init(local_data);
    local_node_data_t data_to_insert = {};
    data_to_insert.cached = curr_data_ptr;
    data_to_insert.root_node = first_freq;
    data_to_insert.index = index;
    data_to_insert.frequency = LFUDA_INITIAL_FREQ;

    base_cache_insert(basecache, first_freq, toinsert, data_to_insert, free_entry);

    if (basecache->data_size) {
        memcpy(curr_data_ptr, page, basecache->data_size);
    }

    return page;
}

//============================================================================================================

void *lfuda_get(lfuda_t cache_, void *index) {
    struct lfuda_s *lfuda = (struct lfuda_s *)cache_;
    // In this case strict-aliasing does not apply, because base_cache_t is the first member of lfuda_s struct
    struct base_cache_s *basecache = (struct base_cache_s *)cache_;

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