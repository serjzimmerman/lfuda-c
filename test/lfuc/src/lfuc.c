#include <stdio.h>

#include "dllist.h"
#include "error.h"

#include "dump.h"
#include "lfu.h"
#include "memutil.h"

#include <assert.h>

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
    UNUSED_PARAMETER(index);
    static int a = 5;
    return &a;
}

void print_elem(void *index, FILE *file) {
    fprintf(file, "%d", *((int *)index));
}

int main() {
    size_t m = 0, n = 0;

    int res = scanf("%lu %lu", &m, &n);
    if (res != 2) {
        ERROR("Invalid input\n");
    }

    cache_init_t init = {
        .hash = CACHE_HASH_F(index_hash),
        .cmp = CACHE_CMP_F(index_cmp),
        .get = CACHE_GET_F(get_page),
        .size = m,
        .data_size = sizeof(index_t),
    };

    lfu_t lfu = lfu_init(init);

    index_t *array = calloc_checked(n, sizeof(index_t));
    for (size_t i = 0; i < n; ++i) {
        index_t *index = &array[i];
        if (scanf("%d", &index->value) != 1) {
            ERROR("Invalid input\n");
        }
        lfu_get(lfu, index);
    }

#ifdef DUMP
    output_t output = {0};
    output.file = fopen("dump.dot", "w");
    if (!output.file) {
        ERROR("Could not open a file\n");
    }
    assert(output.file);
    output.print = print_elem;

    dump_cache(lfu, output);
    fclose(output.file);
    printf("%lu\n", lfu_get_hits(lfu));
#endif

    lfu_free(lfu);
    free(array);
}