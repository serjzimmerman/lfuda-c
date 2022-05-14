#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "dllist.h"
#include "error.h"

#include "dump.h"
#include "lfu.h"
#include "memutil.h"

#define MAXLEN 128
#define STR(x) #x

int index_cmp_string(char *str1, char *str2) {
    return strcmp(str1, str2);
}

unsigned long index_hash_string(char *str) {
    unsigned key = 5381;
    while (*str != '\0') {
        key = ((key << 5) + key) + *str;
        str++;
    }
    return key;
}

void print_elem_string(void *index, FILE *file) {
    fprintf(file, "%s", (char *)index);
}

int main() {
    size_t m = 0, n = 0;

    int res = scanf("%lu %lu", &m, &n);
    if (!res) {
        ERROR("Invalid input\n");
    }

    cache_init_t init = {
        .hash = index_hash_string,
        .cmp = index_cmp_string,
        .get = NULL,
        .size = m,
        .data_size = 0,
    };

    lfu_t lfu = lfu_init(init);

    char **array = calloc_checked(n, sizeof(char *));
    for (int i = 0; i < n; i++) {
        static char buf[MAXLEN] = {0};
        scanf("%" STR(MAXLEN) "s", buf);
        size_t len = strlen(buf);
        array[i] = calloc_checked(len + 1, sizeof(char));
        memcpy(array[i], buf, len + 1);
        lfu_get(lfu, array[i]);
    }

    output_t output = {};
    output.file = fopen("dump.dot", "w");
    assert(output.file);
    output.print = print_elem_string;

    dump_cache(lfu, output);
    fclose(output.file);
    printf("%lu\n", lfu_get_hits(lfu));

    for (int i = 0; i < n; i++) {
    }

    lfu_free(lfu);
    free(array);
}