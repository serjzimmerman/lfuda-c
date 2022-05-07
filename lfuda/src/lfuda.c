#include "lfuda.h"
#include "basecache.h"
#include "clist.h"

#include "memutil.h"
#include <assert.h>

struct lfuda_s {
    base_cache_t base;
};

lfuda_t lfuda_init(cache_init_t init) {
    struct lfuda_s *lfuda = calloc_checked(1, sizeof(struct lfuda_s));
    base_cache_init(&lfuda->base, init);
    return lfuda;
}

void lfuda_free(lfuda_t cache) {
    struct lfuda_s *lfuda = (struct lfuda_s *)cache;
    hashtab_free(lfuda->base.table);
    /* TODO: ajlekcahdp4 - add free of cache list, red black tree, etc.*/
}

void *lfuda_get(lfuda_t cache_, void *index) {
    /* TODO: ajlekcahdp4 - implement lfuda get page */
}