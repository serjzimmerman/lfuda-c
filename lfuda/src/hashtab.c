//----------------------------------------------------------------------------------------------------------//
//                                                                                                          //
//                                          hash table realization                                          //
//                                                                                                          //
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
    size_t n;
#endif
} buckets_t;

struct hashtab_s {
    size_t size;
    size_t inserts;
    size_t buckets_used;
    size_t collisions;

    hash_func_t hash;
    entry_cmp_func_t cmp;
    entry_free_func_t free;
    dl_list_t list;
    float load_factor;

    buckets_t array[];
};

//============================================================================================
hashtab_t hashtab_init(size_t initial_size, hash_func_t hash, entry_cmp_func_t cmp, entry_free_func_t freefunc) {
    assert(initial_size);
    assert(hash);
    assert(cmp);

    struct hashtab_s *table = calloc_checked(1, sizeof(struct hashtab_s) + initial_size * sizeof(buckets_t));

    table->size = initial_size;
    table->hash = hash;
    table->cmp = cmp;
    table->free = freefunc;

    table->list = dl_list_init();

    return table;
}

//============================================================================================
void hashtab_free(hashtab_t table_) {
    struct hashtab_s *table = (struct hashtab_s *)table_;
    dl_list_free(table->list, table->free);
    free(table);
}

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
#define CRITICAL_FACTOR 0.7f

void hashtab_insert(hashtab_t *table_, void *entry) {
    dl_node_t node = NULL;
    struct hashtab_s *table = NULL;
    unsigned long hash = 0;

    assert(table_);
    assert(entry);

    table = *table_;

    if ((float)table->inserts / table->size > CRITICAL_FACTOR) {
        table = hashtab_resize(table, 2 * table->size);
        *table_ = table;
    }

    hash = table->hash(entry) % table->size;
    node = dl_node_init(entry);

    table->inserts += 1;

    if (table->array[hash].node == NULL) {
        dl_list_push_back(table->list, node);
        table->array[hash].node = node;
        table->buckets_used += 1;
    } else {
        dl_list_insert_after(table->list, table->array[hash].node, node);
        table->collisions += 1;
    }
#ifdef HASHTAB_USE_N_OPTIMIZATION
    table->array[hash].n += 1;
#endif
}

//============================================================================================
void *hashtab_lookup(hashtab_t table_, void *key) {
    struct hashtab_s *table = (struct hashtab_s *)table_;
    dl_node_t find = NULL;
    unsigned long hash = 0;

    assert(table);
    assert(key);

    hash = table->hash(key) % table->size;
    find = table->array[hash].node;
    if (!find)
        return NULL;

#ifdef HASHTAB_USE_N_OPTIMIZATION
    size_t capacity = table->array[hash].n;
    for (size_t i = 0; i < capacity; i++) {
        if (table->cmp(dl_node_get_data(find), key) == 0)
            return find;
        find = dl_node_get_next(find);
    }
#else
    unsigned long temphash = hash;
    while (temphash == hash) {
        if (table->cmp(dl_node_get_data(find), key) == 0)
            return find;
        find = dl_node_get_next(find);
        if (!find)
            return NULL;
        temphash = table->hash(dl_node_get_data(find)) % table->size;
    }
#endif
    return NULL;
}

#if 0
//============================================================================================
void *hashtab_remove(hashtab_t table_, void *key) {
    struct hashtab_s *table = NULL;
    dl_list_t find = NULL;
    int hash = 0;
#ifdef HASHTAB_USE_N_OPTIMIZATION
    int capacity = 0;
#else
    int temphash = 0;
#endif
    assert(table_);
    assert(key);

    hash = table->hash(key) % table->size;
    find = table->array[hash].node;
    if (!find)
        return NULL;

#ifdef HASHTAB_USE_N_OPTIMIZATION
    capacity = table->array[hash].n;
    for (int i = 0; i < capacity; i++) {
        if (table->cmp(dl_node_get_data(find), key) == 0)
            return dl_list_remove(table->list, find);
        find = dl_node_get_next(find);
    }
#else
    temphash = hash;
    while (temphash == hash) {
        if (table->cmp(dl_node_get_data(find), key) == 0)
            return dl_list_remove(table->list, find);
        find = dl_node_get_next(find);
        if (!find)
            return NULL;
        temphash = table->hash(dl_node_get_data(find)) % table->size;
    }
#endif
    return NULL;
}
#endif