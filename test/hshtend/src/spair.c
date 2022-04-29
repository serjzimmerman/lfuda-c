#include "spair.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* djb2 hash function http://www.cse.yorku.ca/~oz/hash.html */
unsigned long hash_djb2(const char *str) {
    unsigned long hash = 5381;
    int c = 0;

    assert(str);

    while ((c = *(str++))) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

unsigned long spair_hash_djb2(const void *pair) {
    return hash_djb2(((struct spair_s *)pair)->key);
}

spair_t spair_init(const char *key, pair_val_t value) {
    assert(key);

    if (!key) {
        return NULL;
    }

    struct spair_s *pair = calloc(1, sizeof(struct spair_s));

    if (!pair) {
        return NULL;
    }

    size_t len = strlen(key);
    pair->key = calloc(1, (len + 1) * sizeof(char));

    if (!pair->key) {
        free(pair);
        return NULL;
    }

    memcpy(pair->key, key, len);
    pair->value = value;

    return (spair_t)pair;
}

void spair_free(spair_t pair) {
    assert(pair);

    free(((struct spair_s *)pair)->key);
    free(pair);
}

void spair_set_value(spair_t pair, pair_val_t value) {
    assert(pair);

    ((struct spair_s *)pair)->value = value;
}

pair_val_t spair_get_value(spair_t pair) {
    assert(pair);

    return ((struct spair_s *)pair)->value;
}

const char *spair_get_key(spair_t pair) {
    assert(pair);

    return ((struct spair_s *)pair)->key;
}

int spair_cmp(spair_t a, spair_t b) {
    return strcmp(((struct spair_s *)a)->key, ((struct spair_s *)b)->key);
}