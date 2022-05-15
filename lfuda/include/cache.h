#ifndef LFUDA_CACHE_H
#define LFUDA_CACHE_H

#include "dllist.h"
#include "hashtab.h"

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

typedef void *(*cache_get_page_t)(void *index);

// Initializer struct for cache
typedef struct {
    cache_get_page_t get;
    // Functions for hashing and comparing indexes
    hash_func_t hash;
    entry_cmp_func_t cmp;
    // Optional free function
    entry_free_func_t free;
    size_t size, data_size;
} cache_init_t;

#define CACHE_HASH_F(func) ((hash_func_t)(func))
#define CACHE_CMP_F(func)  ((entry_cmp_func_t)(func))
#define CACHE_GET_F(func)  ((cache_get_page_t)(func))

#ifdef __cplusplus
}
#endif

#endif