#include "basecache.h"
#include "cache.h"

#include "dllist.h"
#include "hashtab.h"

#include "memutil.h"

#include <assert.h>
#include <string.h>

freq_node_t freq_node_init(size_t key) {
    freq_node_t node = dl_node_init_fam(NULL, sizeof(size_t), &key);
    return node;
}

size_t freq_node_get_key(freq_node_t node_) {
    size_t key = 0;
    memcpy(&key, dl_node_get_fam(node_), sizeof(size_t));
    return key;
}

local_node_t local_node_init(void *data) {
    freq_node_t node = dl_node_init_fam(NULL, sizeof(void *), &data);
    return node;
}

void *local_node_get_data(local_node_t node_) {
    void *result = NULL;
    memcpy(&result, dl_node_get_fam(node_), sizeof(void *));
}

base_cache_t *base_cache_init(base_cache_t *cache, size_t size, size_t data_size, cache_init_t init) {
    assert(cache);
    assert(size);
    assert(data_size);

    assert(init.cmp);
    assert(init.hash);

    cache->table = hashtab_init(size * 2, init.hash, init.cmp, init.free);
    cache->freq_list = dl_list_init();

    return cache;
}