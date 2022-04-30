#include "basecache.h"
#include "cache.h"

#include "dllist.h"
#include "hashtab.h"

#include "memutil.h"

#include <assert.h>
#include <string.h>

base_cache_t *base_cache_init(base_cache_t *cache, cache_init_t init) {
    assert(cache);

    // That's ridiculous
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