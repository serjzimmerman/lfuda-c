//----------------------------------------------------------------------------------------------------------//
//                                                                                                          //
//                                          hash table realization                                          //
//                                                                                                          //
//----------------------------------------------------------------------------------------------------------//
//                                        Authors:                                                          //
//                                            Zimmerman Sergey                                              //
//                                            Romanov Alexander                                             //
//                                            Gerasemenko Danil                                             //
//----------------------------------------------------------------------------------------------------------//

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "memutil.h"

#include "dllist.h"
#include "hashtab.h"

typedef struct {
    dl_node_t node;
#ifdef HASHTAB_USE_N_OPTIMIZATION
    size_t n; // Number of nodes in bucket
#endif
} buckets_t;

struct hashtab_s {
    // One big ass doubly linked lists that stores all of the entries
    dl_list_t list;

    // Utility functions that have to be provided by the user
    hash_func_t hash;
    entry_cmp_func_t cmp;
    entry_free_func_t free;

    // Hash table stats
    size_t size;
    size_t inserts;
    size_t buckets_used;
    size_t collisions;

    // Critical load factor
    float load_factor;

    // Array of buckets that stores the pointers to the first node of the list with the hash corresponding to the index
    // This array is stored as a flexible array member
    buckets_t array[];
};

//============================================================================================

#define DEFAULT_LOAD_FACTOR 0.7f
hashtab_t hashtab_init(size_t initial_size, hash_func_t hash, entry_cmp_func_t cmp, entry_free_func_t freefunc) {
    assert(initial_size);
    assert(hash);
    assert(cmp);

    struct hashtab_s *table = calloc_checked(1, sizeof(struct hashtab_s) + initial_size * sizeof(buckets_t));
    table->list = dl_list_init();

    table->size = initial_size;
    table->hash = hash;
    table->cmp = cmp;
    table->free = freefunc;
    table->load_factor = DEFAULT_LOAD_FACTOR;

    return table;
}

//============================================================================================

void hashtab_set_load_factor(hashtab_t table_, float load_factor) {
    struct hashtab_s *table = table_;
    // Assert that the new load factor is in a reasonable range
    assert(load_factor <= 1.0f && load_factor >= 0.0f);
    table->load_factor = load_factor;
}

//============================================================================================

void hashtab_free(hashtab_t table_) {
    struct hashtab_s *table = (struct hashtab_s *)table_;
    dl_list_free(table->list, table->free);
    free(table);
}

//============================================================================================

hashtab_stat_t hashtab_get_stat(hashtab_t table_) {
    struct hashtab_s *table = (struct hashtab_s *)table_;
    hashtab_stat_t stat;

    stat.size = table->size;
    stat.inserts = table->inserts;
    stat.used = table->buckets_used;
    stat.collisions = table->collisions;

    return stat;
}

//============================================================================================

static void hashtab_insert_impl(hashtab_t *table_, dl_node_t node) {
    struct hashtab_s *table = *(struct hashtab_s **)table_;

    assert(table);

    unsigned long hash = table->hash(dl_node_get_data(node)) % table->size;
    table->inserts++;

    if (table->array[hash].node == NULL) {    // If there were no nodes in bucket,
        dl_list_push_back(table->list, node); // Insert in the end of the list
        table->array[hash].node = node;
        table->buckets_used++;
    } else {
        dl_list_insert_after(table->list, table->array[hash].node, node); // If there are a number of nodes in bucket,
        table->collisions++;                                              // insert after top of the sublist
    }
#ifdef HASHTAB_USE_N_OPTIMIZATION
    table->array[hash].n++; // Increment number of nodes in bucket
#endif
}

#define ENCR_MULTIPLIER 2
void hashtab_insert(hashtab_t *table_, void *entry) {
    struct hashtab_s *table = *(struct hashtab_s **)table_;

    assert(table);
    assert(entry);

    if ((float)table->inserts > table->load_factor * (float)table->size) { // Resize if many insertions done
        // This is a funny bug actually, because if initial size is one, then 2 * 1 - 1 is also 1 ;)
        // So this is necessary to handle this edge case
        size_t new_size = ENCR_MULTIPLIER * table->size - 1;
        if (table->size == 1) {
            new_size = ENCR_MULTIPLIER * table->size;
        }

        table = hashtab_resize(table, new_size);
        // Note that new size is (old_size * MULT - 1) to avoid sizes that are powers of MULT
        *table_ = table;
    }

    dl_node_t node = dl_node_init(entry); // Create new node
    hashtab_insert_impl(table_, node);
}

//============================================================================================

void *hashtab_lookup(hashtab_t table_, const void *key) {
    struct hashtab_s *table = (struct hashtab_s *)table_;

    assert(table);
    assert(key);

    unsigned long hash = table->hash(key) % table->size;
    dl_node_t find = table->array[hash].node;

    if (!find) {
        return NULL;
    }

#ifdef HASHTAB_USE_N_OPTIMIZATION // Using number of nodes in bucket
    size_t capacity = table->array[hash].n;
    for (size_t i = 0; i < capacity; i++) {
        if (table->cmp(dl_node_get_data(find), key) == 0) {
            return dl_node_get_data(find);
        }
        find = dl_node_get_next(find);
    }

#else // Using hash
    unsigned long temphash = hash;
    while (temphash == hash) {
        if (table->cmp(dl_node_get_data(find), key) == 0) {
            return dl_node_get_data(find);
        }
        if (!(find = dl_node_get_next(find))) {
            break;
        }
        temphash = table->hash(dl_node_get_data(find)) % table->size; // Update hash
    }
#endif

    return NULL;
}

//============================================================================================

#ifdef HASHTAB_USE_N_OPTIMIZATION
static inline void *hashtab_remove_use_n_impl(hashtab_t table_, void *key) {
    struct hashtab_s *table = (struct hashtab_s *)table_;

    assert(table_);
    assert(key);

    unsigned long hash = table->hash(key) % table->size;
    dl_list_t find = table->array[hash].node;

    if (!find) {
        return NULL;
    }

    size_t capacity = table->array[hash].n;
    for (size_t i = 0; i < capacity; i++) {
        if (table->cmp(dl_node_get_data(find), key) == 0) {
            if (capacity > 1) {
                table->collisions--; // Decrement number of collisions if there were many nodes in bucket
            } else {
                table->array[hash].node = NULL; // If only one - clear the bucket
            }
            table->inserts--;
            table->array[hash].n--;

            void *result = dl_node_get_data(dl_list_remove(table->list, find));
            dl_node_free(find, NULL);
            return result;
        }
        find = dl_node_get_next(find);
    }

    return NULL;
}
#else

static inline void *hashtab_remove_impl_no_n_impl(hashtab_t table_, void *key) {
    struct hashtab_s *table = (struct hashtab_s *)table_;

    assert(table_);
    assert(key);

    unsigned long hash = table->hash(key) % table->size;
    dl_list_t find = table->array[hash].node;

    // Return value (pointer to the data)
    void *result = NULL;

    if (!find) {
        return NULL;
    }

    unsigned long temphash = hash;
    dl_node_t next = dl_node_get_next(find);

    // If next is NULL, then we remove the entry from the bucket. The same applies if the next bucket has a different
    // hash
    if ((table->cmp(dl_node_get_data(find), key) == 0)) {
        if (!next || (next && (table->hash(next) % table->size != hash))) {
            table->array[hash].node = NULL;
            table->buckets_used--;
        } else {
            // In this case there are more nodes after the first and we happily move the bucket pointer futher along
            table->array[hash].node = next;
            table->collisions--;
        }

        // In any case the counter gets decremented
        table->inserts--;

        goto hashtab_remove_exit;
    }

    find = dl_node_get_next(find);
    if (!find) {
        return NULL;
    }

    // From now on we can assume that there are some previous nodes with the same hash
    while (temphash == hash) {
        if (table->cmp(dl_node_get_data(find), key) == 0) {
            table->collisions--;
            goto hashtab_remove_exit;
        }

        if (!(find = dl_node_get_next(find))) {
            break;
        }

        temphash = table->hash(dl_node_get_data(find)) % table->size; // update hash
    }

    return NULL;

// Handle all exiting from function. For whatever reason I get a compiler error when I try to declare a variable right
// after a marker. This code is here to plug up some memory leaks
hashtab_remove_exit:
    result = dl_node_get_data(dl_list_remove(table->list, find));
    dl_node_free(find, NULL);
    return result;
}
#endif

void *hashtab_remove(hashtab_t table_, void *key) {
#ifdef HASHTAB_USE_N_OPTIMIZATION // Using number of nodes in bucket
    return hashtab_remove_use_n_impl(table_, key);
#else // Using hash
    return hashtab_remove_impl_no_n_impl(table_, key);
#endif
}

// Resize the table by moving nodes from the old table's list to a newly allocated one and return the handle.
hashtab_t hashtab_resize(hashtab_t table_, size_t newsize) {
    struct hashtab_s *table = (struct hashtab_s *)table_;

    assert(table);
    assert(newsize);

    // Creating a new hash table
    struct hashtab_s *new_table = hashtab_init(newsize, table->hash, table->cmp, table->free);

    // Creating node for passing through the old list
    while (!dl_list_is_empty(table->list)) {
        dl_node_t node = dl_list_pop_front(table->list);
        hashtab_insert_impl((hashtab_t *)&new_table, node); // Move the node to the new table
    }

    //  Free old hash table
    hashtab_free(table);
    return new_table;
}
