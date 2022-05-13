#ifndef LFUDA_LFUDA_CACHE_H
#define LFUDA_LFUDA_CACHE_H

#include "cache.h"
#include "dllist.h"
#include "hashtab.h"

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

typedef void *lfuda_t;

// initialize cache
lfuda_t lfuda_init(cache_init_t init);

// free cache
void lfuda_free(lfuda_t lfuda);

// get page by index
void *lfuda_get(lfuda_t cache_, void *index);

// Get current hits in lfuda
size_t lfuda_get_hits(lfuda_t cache_);

#ifdef __cplusplus
}
#endif

#endif