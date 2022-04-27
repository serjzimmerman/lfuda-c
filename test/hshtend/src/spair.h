#ifndef PAIR_H
#define PAIR_H

typedef unsigned pair_val_t;
typedef void *spair_t;

struct spair_s {
    void *key;
    pair_val_t value;
};

unsigned long hash_djb2(const char *str);
unsigned long spair_hash_djb2(const void *pair);

spair_t spair_init(const char *key, pair_val_t value);
void spair_free(spair_t pair);

void spair_set_value(spair_t pair, pair_val_t value);
pair_val_t spair_get_value(spair_t pair);
const char *spair_get_key(spair_t pair);

int spair_cmp(spair_t a, spair_t b);

#endif