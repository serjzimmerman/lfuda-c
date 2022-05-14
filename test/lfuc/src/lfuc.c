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
    static int a = 5;
    return &a;
}

void print_elem(void *index, FILE *file) {
    fprintf(file, "%d", *((int *)index));
}

int main() {
    size_t m = 0, n = 0;

    int res = scanf("%lu %lu", &m, &n);
    if (!res) {
        ERROR("Invalid input\n");
    }

    cache_init_t init = {
        .hash = index_hash,
        .cmp = index_cmp,
        .get = get_page,
        .size = m,
        .data_size = sizeof(index_t),
    };

    lfu_t lfu = lfu_init(init);

    index_t *array = calloc_checked(n, sizeof(index_t));
    for (int i = 0; i < n; ++i) {
        index_t *index = &array[i];
        scanf("%d", index);
        lfu_get(lfu, index);
    }

    output_t output = {};
    output.file = fopen("dump.dot", "w");
    assert(output.file);
    output.print = print_elem;

    dump_cache(lfu, output);
    fclose(output.file);
    printf("%lu\n", lfu_get_hits(lfu));

    lfu_free(lfu);
    free(array);
}