#include "dllist.h"
#include <array>
#include <cstdio>
#include <gtest/gtest.h>
#include <hashtab.h>

struct entry_t {
    int a;

    entry_t(int value) : a(value) {
    }
};

static entry_t *entry_init(int value) {
    entry_t *ptr = static_cast<entry_t *>(calloc(1, sizeof(entry_t)));
    ptr->a = value;
    return ptr;
}

static unsigned long entry_hash(void *entry) {
    return static_cast<entry_t *>(entry)->a;
}

static unsigned long entry_hash_mod2(void *entry) {
    return static_cast<entry_t *>(entry)->a % 2;
}

static int entry_cmp(void *a, void *b) {
    entry_t *first = static_cast<entry_t *>(a), *second = static_cast<entry_t *>(b);
    return first->a - second->a;
}

TEST(TestHashTab, TestInsert) {
    hashtab_t table = hashtab_init(128, entry_hash_mod2, entry_cmp, free);

    entry_t *insert1 = entry_init(1);
    hashtab_insert(&table, insert1);

    entry_t key{1};
    entry_t *entry1 = static_cast<entry_t *>(hashtab_lookup(table, &key));
    ASSERT_EQ(entry1->a, 1);

    entry_t *entry2 = static_cast<entry_t *>(hashtab_remove(table, &key));
    ASSERT_EQ(entry2->a, 1);

    hashtab_insert(&table, entry_init(3));
    hashtab_insert(&table, insert1);

    entry_t *entry3 = static_cast<entry_t *>(hashtab_remove(table, &key));
}

// Run all tests
int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}