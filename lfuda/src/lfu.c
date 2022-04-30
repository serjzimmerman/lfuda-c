#include "lfu.h"
#include "basecache.h"
#include "dllist.h"
#include "hashtab.h"

#include "clist.h"
#include <assert.h>
#include <string.h>

#include "memutil.h"

struct lfu_s {
    base_cache_t base;
};

lfu_t lfu_init(cache_init_t init) {
    struct lfu_s *lfu = calloc_checked(1, sizeof(struct lfu_s));
    base_cache_init(&lfu->base, init);
    return lfu;
}

void lfu_free(lfu_t cache) {
    struct lfu_s *lfu = (struct lfu_s *)cache;
    hashtab_free(lfu->base.table);
}

entry_t *hash_entry_init(void *index, local_node_t local) {
    entry_t *entry = calloc_checked(1, sizeof(entry_t));
    assert(index);

    entry->index = index;
    entry->local = local;

    return entry;
}

// This function is kinda horrendous, will deal with it later
void *lfu_get(lfu_t cache_, void *index) {
    struct base_cache_s *cache = (struct base_cache_s *)cache_;

    assert(cache);

    entry_t key = {0};
    key.index = index;

    entry_t *found = hashtab_lookup(cache->table, &key);

    if (found) {
        // Increment hits counter
        cache->hits++;

        // Get frequency and root node of freq_node_t
        local_node_data_t local_data = local_node_get_fam(found->local);
        freq_node_t root_node = dl_node_get_data(found->local);

        local_data.frequency++;
        // Remove node from this list and move to the freq node with incremented key
        dl_list_remove(freq_node_get_local(root_node), found->local);

        freq_node_t next_freq = dl_node_get_next(root_node);
        size_t next_key = local_data.frequency;

        if (next_freq && freq_node_get_key(next_freq) == next_key) {
            dl_list_push_front(freq_node_get_local(next_freq), found->local);
        } else {
            freq_node_t new_freq = freq_node_init(next_key);
            dl_list_insert_after(cache->freq_list, root_node, new_freq);
            dl_list_push_front(freq_node_get_local(new_freq), found->local);
        }

        local_node_set_fam(found->local, local_data);
        return local_node_get_fam(found->local).cached;
    }

    void *page = cache->slow_get(index);
    local_node_t toinsert = NULL;

    if (cache->curr_top < cache->size) {
        char *curr_data_ptr = (cache->cached_data + cache->data_size * cache->curr_top);
        memcpy(curr_data_ptr, page, cache->data_size);

        // Initialize local_data with corrent info
        local_node_data_t local_data;
        local_data.cached = curr_data_ptr;
        local_data.frequency = 1;

        freq_node_t first_node = dl_list_get_first(cache->freq_list);
        if (!first_node || freq_node_get_key(first_node) != 1) {
            first_node = freq_node_init(1);
            dl_list_push_front(cache->freq_list, first_node);
        }

        toinsert = local_node_init(first_node, local_data);
        dl_list_push_front(freq_node_get_local(first_node), toinsert);
    } else {
        // TODO: implement
    }

    hashtab_insert(&cache->table, hash_entry_init(index, toinsert));
    return page;
}