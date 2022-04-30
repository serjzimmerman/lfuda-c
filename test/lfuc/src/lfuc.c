#include <stdio.h>

#include "dllist.h"
#include "error.h"

#include "lfu.h"

typedef struct {
    int value;
} index_t;

unsigned long index_hash(index_t **a) {
    return (unsigned long)((*a)->value);
}

int index_cmp(index_t **a, index_t **b) {
    return ((*a)->value) - ((*b)->value);
}

void *get_page(index_t *index) {
    static int a = 5;
    return &a;
}

int main() {
    cache_init_t init = {
        .hash = index_hash,
        .cmp = index_cmp,
        .get = get_page,
        .size = 128,
        .data_size = sizeof(int),
    };
    lfu_t lfu = lfu_init(init);

    index_t page1 = {.value = 1};
    printf("%d", *(int *)lfu_get(lfu, &page1));

    printf("%d", *(int *)lfu_get(lfu, &page1));
}