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

lfu_t lfu_init(cache_init_t init);
void lfu_free(lfu_t);

void *lfu_get(lfu_t cache_, void *index);

#ifdef __cplusplus
}
#endif

#endif