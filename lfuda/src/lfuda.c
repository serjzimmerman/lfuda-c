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

freq_node_t lfuda_get_next_freq(lfuda_t cache_, size_t new_key) {
    struct lfuda_s *lfuda = (struct lfuda_s *)cache_;
    freq_node_t test_freq = freq_node_init(new_key);
    freq_node_t freq = rb_tree_lookup(lfuda->rbtree, test_freq);
    free(test_freq); // not so good free, needs to be replaced
    return freq;
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
    struct lfuda_s *lfuda = (struct lfuda_s *)cache_;
    struct base_cache_s *basecache = (struct base_cache_s *)cache_;

    assert(lfuda);

    entry_t key = {0};
    key.index = index;

    entry_t *found = hashtab_lookup(basecache->table, &key);

    if (found) {
        // increment cache hits
        basecache->hits += 1;

        // get data and root node of local node
        local_node_data_t local_data = local_node_get_fam(found->local);
        freq_node_t root_node = dl_node_get_data(found->local);
        // increment frequency
        local_data.frequency += 1;

        // remove local node from this local list and move to the local list
        // with another key (not just incremented)
        dl_list_remove(freq_node_get_local(root_node), found->local);

        // remove root node from red-black tree if local list now is empty
        if (dl_list_is_empty(freq_node_get_local(root_node))) {
            rb_tree_remove(lfuda->rbtree, root_node);
        }

        // find if there are local list with new_key
        size_t new_key = lfuda_get_next_key(lfuda, local_data);
        freq_node_t new_freq = lfuda_get_next_freq(root_node, new_key);

        if (new_freq) {
            dl_list_push_front(freq_node_get_local(new_freq), found->local);
        } else {
            // create new node and insert it in red-black tree
            new_freq = freq_node_init(new_key);
            rb_tree_insert(lfuda->rbtree, new_freq);
            dl_list_insert_after(basecache->freq_list, root_node, new_freq);
            dl_list_push_front(freq_node_get_local(new_freq), found->local);
        }

        local_node_set_fam(found->local, local_data);

        // return cached page
        return local_node_get_fam(found->local).cached;
    }

    void *page = basecache->slow_get(index);
    local_node_t toinsert = NULL;

    if (basecache->curr_top < basecache->size) {
        char *curr_data_ptr = basecache->cached_data + basecache->data_size * basecache->curr_top;
        memcpy(curr_data_ptr, page, basecache->data_size);

        // intialize local_data with current info
        local_node_data_t local_data;
        local_data.cached = curr_data_ptr;
        local_data.frequency = LFUDA_INITIAL_FREQ; // initial frequency/weight

        freq_node_t first_freq = dl_list_get_first(basecache->freq_list);

        // check if first_freq exist and key of first freq is equal to intial freq
        if (!first_freq || freq_node_get_key(first_freq) != LFUDA_INITIAL_FREQ) {
            // create new freq node and insert it in a red-black tree
            first_freq = freq_node_init(LFUDA_INITIAL_FREQ);
            rb_tree_insert(lfuda->rbtree, first_freq);
            dl_list_push_front(basecache->freq_list, first_freq);
        }

        toinsert = local_node_init(first_freq, local_data);
        dl_list_push_front(freq_node_get_local(first_freq), toinsert);
    } else {
        // TODO: ajlekcahdp4 - implement cache evicting and insertion
    }

    hashtab_insert(&basecache->table, hash_entry_init(index, toinsert));
    return page;
}
