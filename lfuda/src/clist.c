#include "basecache.h"
#include "cache.h"

#include "memutil.h"

#include <assert.h>
#include <string.h>

// These functions are basically one-liners but improve readability, so they stay
freq_node_t freq_node_init(size_t key) {
    freq_node_t node = dl_node_init_fam(NULL, sizeof(size_t), &key);
    dl_node_set_data(node, dl_list_init());
    return node;
}

size_t freq_node_get_key(freq_node_t node_) {
    size_t key = 0;
    memcpy(&key, dl_node_get_fam(node_), sizeof(size_t));
    return key;
}

local_list_t freq_node_get_local(freq_node_t node_) {
    return dl_node_get_data(node_);
}

void freq_node_set_local(freq_node_t node_, local_list_t local) {
    dl_node_set_data(node_, local);
}

local_node_t local_node_init(freq_node_t root_node, local_node_data_t fam) {
    freq_node_t node = dl_node_init_fam(NULL, sizeof(local_node_data_t), &fam);
    dl_node_set_data(node, root_node);
    return node;
}

local_node_data_t local_node_get_fam(local_node_t node_) {
    local_node_data_t result = {0};
    memcpy(&result, dl_node_get_fam(node_), sizeof(local_node_data_t));
    return result;
}

void local_node_set_fam(local_node_t node_, local_node_data_t fam_) {
    void *fam = dl_node_get_fam(node_);
    assert(fam);
    memcpy(fam, &fam_, sizeof(local_node_data_t));
}