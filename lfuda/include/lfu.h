#ifndef LFUDA_LFU_CACHE_H
#define LFUDA_LFU_CACHE_H

#include "cache.h"
#include "dllist.h"
#include "hashtab.h"

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

typedef void *lfu_t;

// Init LFU cache from initalizer struct
lfu_t lfu_init(cache_init_t init);

// Free cache
void lfu_free(lfu_t cache_);

// Get data from either cache or slow_get function with index, for which appropritate function have been provided by the
// user
void *lfu_get(lfu_t cache_, void *index);

size_t lfu_get_hits(lfu_t cache_);

#ifdef __cplusplus
}
#endif

#endif