#include <stdio.h>

#include "dllist.h"
#include "error.h"

#include "dump.h"
#include "lfuda.h"
#include "memutil.h"

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

void print_data(index_t *index, FILE *file) {
    fprintf(file, "%d", *(int *)index);
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

    lfuda_t lfu = lfuda_init(init);
    output_t output = {};
    output.print = print_data;

    index_t *array = calloc_checked(n, sizeof(index_t));
    for (size_t i = 0; i < n; ++i) {
        static char buf[128];
        snprintf(buf, 128, "dump%d.dot", i);
        output.file = fopen(buf, "w");
        index_t *index = &array[i];
        scanf("%d", index);
        lfuda_get(lfu, index);
        dump_cache(lfu, output);
        fclose(output.file);
    }

    printf("%lu\n", lfuda_get_hits(lfu));

    lfuda_free(lfu);
    free(array);
}