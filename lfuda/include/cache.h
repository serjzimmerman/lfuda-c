#ifndef LFUDA_CACHE_H
#define LFUDA_CACHE_H

#include "dllist.h"
#include "hashtab.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif

struct base_cache_s;
typedef struct base_cache_s base_cache_t;

typedef size_t (*cache_get_priority_func_t)(size_t freq, base_cache_t *cache);

typedef struct {
    hash_func_t hash;
    entry_cmp_func_t cmp;
    // Optional free function
    entry_free_func_t free;
} cache_init_t;

#ifdef __cplusplus
}
#endif

#endif