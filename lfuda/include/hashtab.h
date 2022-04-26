#ifndef LFUDA_HASHTAB_H
#define LFUDA_HASHTAB_H

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

// This is a draft of possible hash table interface

typedef void *hashtab_t;

// Type for hashing function that accepts arbitrary object and must be defined by the user
// Where details of the implementation are known
typedef unsigned long (*hash_func_t)(void *);
// Comparator function
typedef int (*entry_cmp_func_t)(void *, void *);
typedef void (*entry_free_func_t)(void *);

hashtab_t hashtab_init(size_t initial_size, hash_func_t hash, entry_cmp_func_t cmp, entry_free_func_t freefunc);
void hashtab_free(hashtab_t table_);

// Intermediate struct to be returned from get_stat function
typedef struct {
    size_t size;       // Max number of buckets
    size_t used;       // Number of used buckets
    size_t collisions; // Number of collisions
    size_t inserts;    // Total number of elements in the hash table, including collisions
} hashtab_stat_t;

hashtab_stat_t hashtab_get_stat(hashtab_t table_);

// Main hash table accessor functions

// Function that inserts entry into the table assuming it is not already present
// It accepts pointer to a pointer to a handle, because it may be necessary to resize the table, which would invalidate
// previous handle
void hashtab_insert(hashtab_t *table_, void *entry);

// Lookup whether the entry is alredy present in the table
// Returns pointer to the entry, NULL if is absent
void *hashtab_lookup(hashtab_t table_, void *key);

// Resize the table to accomodate max newsize buckets
hashtab_t hashtab_resize(hashtab_t table_, size_t newsize);

// Remove key from the table an return entry by pointer, NULL if it's absent from the table
void *hashtab_remove(hashtab_t table_, void *key);

#ifdef __cplusplus
}
#endif

#endif