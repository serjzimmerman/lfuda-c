#include "basecache.h"
#include "cache.h"

#include "dllist.h"
#include "hashtab.h"

#include "memutil.h"

#include <assert.h>
#include <string.h>

base_cache_t *base_cache_init(base_cache_t *cache, cache_init_t init) {
    assert(cache);

    assert(init.get);
    assert(init.cmp);
    assert(init.hash);
    assert(init.size);
    assert(init.data_size);

    cache->size = init.size;
    cache->data_size = init.data_size;
    cache->hits = 0;
    cache->slow_get = init.get;

    cache->table = hashtab_init(init.data_size * 2, init.hash, init.cmp, init.free);
    // Disable resize, because this would be bad for perfomance and totally redundant
    hashtab_set_enabled_resize(cache->table, 0);

    cache->freq_list = dl_list_init();
    cache->cached_data = calloc_checked(init.size, init.data_size);

    return cache;
}

local_node_t base_cache_lookup(base_cache_t *cache, void *index) {
    assert(cache);
    assert(cache->table);
    assert(cache->hash);

    dl_node_t check_node = hashtab_lookup(cache->table, index);

    return dl_node_get_data(check_node);
}

// If a node stays in hash table after removing
local_node_t base_cache_remove(base_cache_t *cache, local_node_t node) {
    assert(cache);
    assert(cache->table);
    assert(cache->hash);
#if 1
    hashtab_remove(cache->table, local_node_get_fam(node).cached);
#endif
    // Rebounding of knots
    local_list_t frec_node = local_node_get_fam(node).cached;
    return dl_list_remove(frec_node, node);
}

// Inserting in front due to more comfortable replacemnt for LFU policy
void base_cache_insert(base_cache_t *cache, freq_node_t freqnode, local_node_t toinsert) {
    assert(cache);
    dl_list_push_back(freqnode, toinsert);
}
