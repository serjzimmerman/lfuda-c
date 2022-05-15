#include "lfu.h"
#include "basecache.h"
#include "dllist.h"
#include "hashtab.h"

#include "clist.h"
#include <assert.h>
#include <string.h>

#include "memutil.h"

//============================================================================================================

// Basically LFU cache is a base-cache with an implemented policy
struct lfu_s {
    base_cache_t base;
};

lfu_t lfu_init(cache_init_t init) {
    struct lfu_s *lfu = calloc_checked(1, sizeof(struct lfu_s));

    base_cache_init(&lfu->base, init);

    return lfu;
}

//============================================================================================================

// Get next freq node and create one if there is no immediate successor. If freqnode is NULL, then return frequency node
// with key 1, or create one if there are none
static freq_node_t next_freq_node_init(freq_list_t list, freq_node_t freqnode) {
    assert(list);

    if (!freqnode) {
        freq_node_t first_freq = dl_list_get_first(list);
        if (!first_freq || freq_node_get_key(first_freq) != 1) {
            freq_node_t new_freq = freq_node_init(1);
            dl_list_push_front(list, new_freq);
            return new_freq;
        }
        return first_freq;
    }

    freq_node_t next_freq = dl_node_get_next(freqnode);
    size_t nextkey = freq_node_get_key(freqnode) + 1;

    if (next_freq && freq_node_get_key(next_freq) == nextkey) {
        return next_freq;
    }

    next_freq = freq_node_init(nextkey);
    dl_list_insert_after(list, freqnode, next_freq);

    return next_freq;
}

//============================================================================================================

// Case 1. When there is already a node present
static void *lfu_promote(base_cache_t *cache, local_node_t found) {
    assert(cache);
    assert(found);

    // Increment hits counter
    cache->hits++;

    // Get frequency and root node of freq_node_t
    local_node_data_t local_data = local_node_get_data(found);
    freq_node_t root_node = local_data.root_node;
    local_data.frequency++;

    freq_node_data_t freq_data = freq_node_get_data(root_node);
    // Remove node from this list and move to the freq node with incremented key
    local_list_t local_list = freq_data.local_list;
    dl_list_remove(local_list, found);
    freq_node_t next_freq = next_freq_node_init(cache->freq_list, root_node);
    local_data.root_node = next_freq;

    // If frequency node is empty, then remove it
    base_cache_remove_freq_if_empty(cache, root_node);

    dl_list_push_front(freq_node_get_local(next_freq), found);
    local_node_set_data(found, local_data);

    return local_data.cached;
}

//============================================================================================================

static void *lfu_insert_or_replace(base_cache_t *cache, void *index) {
    void *page = (cache->slow_get ? cache->slow_get(index) : NULL);
    local_node_t toinsert = NULL;
    char *curr_data_ptr = NULL;

    // Initialize local_data with corrent info
    local_node_data_t local_data = {};
    // local_data.cached = curr_data_ptr;
    local_data.frequency = 1;
    local_data.index = index;

    // 2.1 In this case cache is not full and we can just insert the node with frequency 1.
    if (cache->curr_top < cache->size) {
        curr_data_ptr = (cache->cached_data + cache->data_size * cache->curr_top++);

        freq_node_t first_freq = next_freq_node_init(cache->freq_list, NULL);
        if (cache->data_size) {
            local_data.cached = curr_data_ptr;
        }

        local_data.root_node = first_freq;
        toinsert = local_node_init(local_data);
        base_cache_insert(cache, first_freq, toinsert, local_data, NULL);
    }

    // 2.2 In this case the cache is full and we decide which entry to invalidate and evict based on LFU strategy
    else {
        freq_node_t first_freq = dl_list_get_first(cache->freq_list);
        local_node_t toevict = dl_list_get_last(freq_node_get_local(first_freq));

        local_node_data_t evicted_data = local_node_get_fam(toevict);
        local_data.cached = evicted_data.cached;
        curr_data_ptr = local_data.cached;

        entry_t *free_entry = base_cache_remove(cache, toevict, &evicted_data.index);

        first_freq = next_freq_node_init(cache->freq_list, NULL);

        local_data.root_node = first_freq;
        toinsert = local_node_init(local_data);
        local_node_free(toevict);

        base_cache_insert(cache, first_freq, toinsert, local_data, free_entry);
    }

    if (cache->data_size) {
        memcpy(curr_data_ptr, page, cache->data_size);
    }

    return page;
}

//============================================================================================================

void *lfu_get(lfu_t cache_, void *index) {
    // In this case strict-aliasing does not apply, because base_cache_t is the first member of lfu_s struct
    base_cache_t *cache = (base_cache_t *)cache_;

    assert(cache);
    assert(index);

    local_node_t found = base_cache_lookup(cache, &index);

    // 1. There is already a cache entry, then we promote it and move futher along the frequency list
    if (found) {
        return lfu_promote(cache, found);
    }

    // 2. If we get here, then the key is not present in the cache. In this case we call slow_get if it is provided and
    // insert the key into the cache, while optionally copying the data. There are 2 subcases here: 2.2 and 2.3
    return lfu_insert_or_replace(cache, index);
}

//============================================================================================================

size_t lfu_get_hits(lfu_t cache_) {
    base_cache_t *cache = (base_cache_t *)cache_;

    assert(cache);

    return cache->hits;
}

//============================================================================================================

void lfu_free(lfu_t cache_) {
    base_cache_t *cache = (base_cache_t *)cache_;

    base_cache_free(cache);

    free(cache);
}