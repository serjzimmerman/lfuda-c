#include "basecache.h"
#include "cache.h"

#include "dllist.h"
#include "hashtab.h"

#include "memutil.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

base_cache_t *base_cache_init(base_cache_t *cache, cache_init_t init) {
    assert(cache);

    // All this should not be either NULL or 0
    assert(init.cmp);
    assert(init.hash);
    assert(init.size);

    // Either there should be a get function and non-zero data_size, or all must be NULL
    assert((!init.get && !init.data_size) || (init.get && init.data_size));

    cache->size = init.size;
    cache->data_size = init.data_size;
    cache->hits = 0;
    cache->slow_get = init.get;
    cache->cached_data = NULL;

    cache->table = hashtab_init(init.size * 2, init.hash, init.cmp, free);
    // Disable resize, because this would be bad for perfomance and totally redundant
    hashtab_set_enabled_resize(cache->table, 0);

    cache->freq_list = dl_list_init();

    // If data_size == 0, then no data will get copied
    if (init.data_size) {
        cache->cached_data = calloc_checked(init.size, init.data_size);
    }

    return cache;
}

// Data type that is stored in the hash table
typedef struct {
    void *index;
    local_node_t local;
} entry_t;

entry_t *entry_init(void *index, local_node_t local) {
    entry_t *entry = calloc_checked(1, sizeof(entry_t));
    assert(index);

    entry->index = index;
    entry->local = local;

    return entry;
}

local_node_t base_cache_lookup(base_cache_t *cache, void **index) {
    assert(cache);
    assert(index);

    entry_t *found = hashtab_lookup(cache->table, index);

    local_node_t *result = (found ? found->local : NULL);

    return result;
}

local_node_t base_cache_remove(base_cache_t *cache, local_node_t node, void **index) {
    assert(cache);
    assert(node);

    // Index should correspond to the local node
    free(hashtab_remove(cache->table, index));

    freq_node_t freq_node = local_node_get_freq_node(node);
    local_list_t local_list = freq_node_get_local(freq_node);
    local_node_t result = dl_list_remove(local_list, node);

    // If list becomes empty, then free it and remove it
    if (dl_list_is_empty(local_list)) {
        dl_list_free(local_list, NULL);
        dl_list_remove(cache->freq_list, freq_node);
    }

    return result;
}

void base_cache_insert(base_cache_t *cache, freq_node_t freqnode, local_node_t toinsert, void *index) {
    assert(cache);
    assert(freqnode);
    assert(toinsert);

    // 1. Set the root of toinsert to freqnode
    local_node_set_freq_node(toinsert, freqnode);

    // 2. Insert the node the the local list
    dl_list_push_front(freq_node_get_local(freqnode), toinsert);

    // 3. Create an entry and insert it into the hash table
    entry_t *entry = entry_init(index, toinsert);

    hashtab_insert(&cache->table, entry);
}