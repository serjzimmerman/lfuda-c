#include <array>
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

#include "rbtree.h"

#include "memutil.h"

struct RBTreeDeleter {
    void operator()(rb_tree_t tree) {
        rb_tree_free(tree, free);
    }
};

template <typename T> class RBTree {
    std::unique_ptr<std::remove_pointer<rb_tree_t>::type, RBTreeDeleter> tree;

    static int Cmp(const void *a, const void *b) {
        return *static_cast<const T *>(a) - *static_cast<const T *>(b);
    }

  public:
    RBTree() : tree(rb_tree_init(&Cmp), RBTreeDeleter{}) {
    }

    operator rb_tree_t() {
        return tree.get();
    }

    void Insert(T val) {
        T *newval = static_cast<T *>(calloc_checked(1, sizeof(T)));
        *newval = val;
        rb_tree_insert(tree.get(), newval);
    }

    T Remove(T val) {
        T *result = static_cast<T *>(rb_tree_remove(tree.get(), &val)), temp{};

        if (result) {
            temp = *result;
        }

        free(result);
        return temp;
    }
};

template <typename T> void EXPECT_RB_TREE_VALID(RBTree<T> &tree) {
    EXPECT_TRUE(rb_tree_is_valid(tree));
}

template <typename T> const char *Stringify(const void *a);

template <> const char *Stringify<int>(const void *a) {
    static char buf[1024];
    snprintf(buf, 1024, "%d", *static_cast<const int *>(a));
    return buf;
}

TEST(TestRBTree, Test1) {
    RBTree<int> tree{};

    for (int i = 0; i < (1 << 10); ++i) {
        tree.Insert(i);
    }

    EXPECT_RB_TREE_VALID(tree);

    for (int i = 0; i < (1 << 9); ++i) {
        tree.Remove(i);
    }

    EXPECT_RB_TREE_VALID(tree);
}

TEST(TestRBTree, Test2) {
    RBTree<int> tree{};

    constexpr int modulo = (1 << 20);

    std::srand(0x0DED);
    for (int i = 0; i < modulo; ++i) {
        int val = std::rand() % modulo;
        if (!rb_tree_lookup(tree, &val)) {
            tree.Insert(val);
        }
    }

    EXPECT_RB_TREE_VALID(tree);

    for (int i = 0; i < modulo >> 2; ++i) {
        if (rb_tree_lookup(tree, &i)) {
            tree.Remove(i);
        }
    }

    EXPECT_RB_TREE_VALID(tree);
}

TEST(TestRBTree, Test3) {
    RBTree<int> tree{};

    for (int i = 0; i < 128; ++i) {
        tree.Insert(i);
    }

    int test_close{16};

    EXPECT_RB_TREE_VALID(tree);

    EXPECT_EQ(*static_cast<const int *>(rb_tree_closest_right(tree, &test_close)), 16);
    EXPECT_EQ(*static_cast<const int *>(rb_tree_closest_right(tree, &test_close)), 16);

    tree.Remove(16);

    EXPECT_EQ(*static_cast<const int *>(rb_tree_closest_left(tree, &test_close)), 15);
    EXPECT_EQ(*static_cast<const int *>(rb_tree_closest_right(tree, &test_close)), 17);

    EXPECT_RB_TREE_VALID(tree);
}

TEST(TestRBTree, Test4) {
    RBTree<int> tree{};

    tree.Insert(1);
    int lookup = 1;
    EXPECT_TRUE(rb_tree_lookup(tree, &lookup) != nullptr);

    tree.Remove(1);

    EXPECT_TRUE(rb_tree_lookup(tree, &lookup) == nullptr);
}

// Run all tests
int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}