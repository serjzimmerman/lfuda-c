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

static unsigned long entry_hash(const void *entry) {
    return static_cast<const entry_t *>(entry)->a;
}

static unsigned long entry_hash_mod2(const void *entry) {
    return static_cast<const entry_t *>(entry)->a % 2;
}

static int entry_cmp(const void *a, const void *b) {
    const entry_t *first = static_cast<const entry_t *>(a), *second = static_cast<const entry_t *>(b);
    return first->a - second->a;
}

TEST(TestHashTab, TestInsert) {
    hashtab_t table = hashtab_init(1, entry_hash_mod2, entry_cmp, free);

    entry_t *insert1 = entry_init(1);
    hashtab_insert(&table, insert1);

    entry_t key{1};
    entry_t *entry1 = static_cast<entry_t *>(hashtab_lookup(table, &key));
    ASSERT_EQ(entry1->a, 1);

    entry_t *entry2 = static_cast<entry_t *>(hashtab_remove(table, &key));
    ASSERT_EQ(entry2->a, 1);

    hashtab_insert(&table, entry_init(3));
    hashtab_insert(&table, insert1);

    entry_t key2{3};
    entry_t *entry3 = static_cast<entry_t *>(hashtab_remove(table, &key2));
    ASSERT_EQ(entry3->a, 3);

    free(entry3);

    hashtab_free(table);
}

TEST(TastHashTab, TestRemove) {
    hashtab_t table = hashtab_init(1, entry_hash, entry_cmp, free);

    constexpr int testlen = 20;
    constexpr int notinserted = 235;

    for (int i = 0; i < testlen; i++) {
        entry_t *insert = entry_init(i);
        hashtab_insert(&table, insert);
    }

    for (int i = 0; i < testlen; i++) {
        entry_t key{i};
        entry_t *entry = static_cast<entry_t *>(hashtab_lookup(table, &key));
        ASSERT_EQ(entry->a, i);
    }

    entry_t key{notinserted};
    entry_t *entry = static_cast<entry_t *>(hashtab_lookup(table, &key));
    ASSERT_EQ(entry, nullptr);

    hashtab_free(table);
}

TEST(TestHashTab, TestResize) {
    hashtab_t table = hashtab_init(1, entry_hash, entry_cmp, free);

    constexpr int testlen = 10;
    for (int i = 0; i < testlen; i++) {
        entry_t *insert = entry_init(i);
        hashtab_insert(&table, insert);
    }

    for (int i = 0; i < testlen; i++) {
        entry_t key{i};
        entry_t *entry = static_cast<entry_t *>(hashtab_lookup(table, &key));
        ASSERT_EQ(entry->a, i);
    }

    hashtab_free(table);
}

TEST(TestHashTab, TestStat) {
    hashtab_t table = hashtab_init(1, entry_hash, entry_cmp, free);

    constexpr int testlen = 50;
    for (int i = 0; i < testlen; i++) {
        entry_t *insert = entry_init(i);
        hashtab_insert(&table, insert);
    }
    hashtab_stat_t stat = hashtab_get_stat(table);

    for (int i = 0; i < testlen; i++) {
        entry_t key{i};
        entry_t *entry = static_cast<entry_t *>(hashtab_remove(table, &key));
        EXPECT_NE(entry, nullptr);
        ASSERT_EQ(entry->a, i);
        stat = hashtab_get_stat(table);
    }

    stat = hashtab_get_stat(table);

    ASSERT_EQ(stat.collisions, 0);
    ASSERT_EQ(stat.inserts, 0);
    ASSERT_EQ(stat.used, 0);

    hashtab_free(table);
}

// Run all tests
int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}