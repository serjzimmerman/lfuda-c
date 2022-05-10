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

entry_t *hash_entry_init(void *index, local_node_t local) {
    entry_t *entry = calloc_checked(1, sizeof(entry_t));
    assert(index);

    entry->index = index;
    entry->local = local;

    return entry;
}

//============================================================================================================

#define LFUDA_FREQ_COEF 1

size_t lfuda_get_next_key(lfuda_t cache_, local_node_data_t local_data) {
    struct lfuda_s *cache = (struct lfuda_s *)cache_;
    size_t new_key = LFUDA_FREQ_COEF * local_data.frequency + cache->age;
    return new_key;
}

//============================================================================================================

freq_node_t lfuda_get_next_freq(freq_node_t *freq_node, size_t new_key) {
    // TODO: ajlekcahdp4 - implement
}

//============================================================================================================

lfuda_t lfuda_init(cache_init_t init) {
    struct lfuda_s *lfuda = calloc_checked(1, sizeof(struct lfuda_s));
    base_cache_init(&lfuda->base, init);
    lfuda->rbtree = rb_tree_init(freq_node_cmp);
    return lfuda;
}

//============================================================================================================

void lfuda_free(lfuda_t cache) {
    struct lfuda_s *lfuda = (struct lfuda_s *)cache;
    hashtab_free(lfuda->base.table);
    /* TODO: ajlekcahdp4 - add free of cache list, red black tree, etc.*/
}

//============================================================================================================

// Temporary define (needs to be replaced)
#define LFUDA_INITIAL_FREQ 1

void *lfuda_get(lfuda_t cache_, void *index) {
    struct base_cache_s *cache = (struct base_cache_s *)cache_;

    assert(cache);

    entry_t key = {0};
    key.index = index;

    entry_t *found = hashtab_lookup(cache->table, &key);

    if (found) {
        // increment cache hits
        cache->hits += 1;

        // get data and root node of local node
        local_node_data_t local_data = local_node_get_fam(found->local);
        freq_node_t root_node = dl_node_get_data(found->local);
        // increment frequency
        local_data.frequency += 1;

        // remove local node from this local list and move to the local list
        // with another key (not just incremented)
        dl_list_remove(freq_node_get_local(root_node), found->local);

        size_t new_key = lfuda_get_next_key(cache, local_data);         // NOT IMPLEMENTED YET
        freq_node_t new_freq = lfuda_get_next_freq(root_node, new_key); // NOT IMPLEMENTED YET

        if (new_freq) {
            dl_list_push_front(freq_node_get_local(new_freq), found->local);
        } else {
            new_freq = freq_node_init(new_key);
            dl_list_insert_after(cache->freq_list, root_node, new_freq);
            dl_list_push_front(freq_node_get_local(new_freq), found->local);
        }

        local_node_set_fam(found->local, local_data);

        // return cached page
        return local_node_get_fam(found->local).cached;
    }

    void *page = cache->slow_get(index);
    local_node_t toinsert = NULL;

    if (cache->curr_top < cache->size) {
        char *curr_data_ptr = cache->cached_data + cache->data_size * cache->curr_top;
        memcpy(curr_data_ptr, page, cache->data_size);

        // intialize local_data with current info
        local_node_data_t local_data;
        local_data.cached = curr_data_ptr;
        local_data.frequency = LFUDA_INITIAL_FREQ; // initial frequency/weight

        freq_node_t first_freq = dl_list_get_first(cache->freq_list);

        // check if first_freq exist and key of first freq is equal to intial freq
        if (!first_freq || freq_node_get_key(first_freq) != LFUDA_INITIAL_FREQ) {
            first_freq = freq_node_init(LFUDA_INITIAL_FREQ);
            dl_list_push_front(cache->freq_list, first_freq);
        }

        toinsert = local_node_init(first_freq, local_data);
        dl_list_push_front(freq_node_get_local(first_freq), toinsert);
    } else {
        // TODO: ajlekcahdp4 - implement cache evicting and insertion
    }

    hashtab_insert(&cache->table, hash_entry_init(index, toinsert));
    return page;
}
