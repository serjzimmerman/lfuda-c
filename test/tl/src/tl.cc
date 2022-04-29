#include "dllist.h"
#include <array>
#include <cstdio>
#include <gtest/gtest.h>

struct ListDeleter {
    void operator()(dl_list_t list) {
        dl_list_free(list, free);
    }
};

// Initialize dl_node_t with ptr to an allocated int with value
template <typename T> dl_node_t NodeInitWithValue(const T value) {
    dl_node_t node = dl_node_init(NULL);
    int *ptr = static_cast<T *>(calloc(1, sizeof(int)));
    *ptr = value;
    dl_node_set_data(node, static_cast<void *>(ptr));
    return node;
}

// All data is assumed to be allocated on heap with calloc or malloc and freed with free()
// Use ints for tests
class List {
    std::unique_ptr<std::remove_pointer<dl_list_t>::type, ListDeleter> list;

  public:
    List() : list(dl_list_init(), ListDeleter{}) {
    }

    List(std::initializer_list<int> ilist) : List() {
        for (auto v : ilist) {
            dl_list_push_back(list.get(), NodeInitWithValue(v));
        }
    }

    operator dl_list_t() {
        return list.get();
    }
};

template <typename T> bool NodeEqValue(dl_node_t node, T value) {
    return (*static_cast<T *>(dl_node_get_data(node)) == value);
}

template <typename T> void ASSERT_NODE_EQ(dl_node_t node, T value) {
    ASSERT_EQ(NodeEqValue(node, value), true);
}

// Compare list with a std::array of ints moving in the forward direction
template <typename T, std::size_t N> void ASSERT_LIST_EQ_FORW(dl_list_t list, std::array<T, N> array) {
    dl_node_t node = dl_list_get_first(list);
    std::size_t i = 0;

    while (node && i < array.size()) {
        ASSERT_NODE_EQ(node, array.at(i++));
        node = dl_node_get_next(node);
    }

    ASSERT_EQ(node, nullptr);
    ASSERT_EQ(i, array.size());
}

// Compare list with a std::array of ints moving in the backward direction
template <typename T, std::size_t N> void ASSERT_LIST_EQ_BACK(dl_list_t list, std::array<T, N> array) {
    dl_node_t node = dl_list_get_last(list);
    std::size_t i = array.size() - 1;

    while (node) {
        ASSERT_NODE_EQ(node, array.at(i));
        node = dl_node_get_prev(node);
        if (i == 0) {
            break;
        }
        i = i - 1;
    }

    ASSERT_EQ(node, nullptr);
    ASSERT_EQ(i, 0);
}

template <typename T, std::size_t N> void ASSERT_LIST_EQ(dl_list_t list, std::array<T, N> array) {
    ASSERT_LIST_EQ_FORW(list, array);
    ASSERT_LIST_EQ_BACK(list, array);
}

TEST(TestList, Test1) {
    List list{};

    dl_list_push_back(list, NodeInitWithValue(1));
    dl_list_push_front(list, NodeInitWithValue(2));
    dl_list_push_back(list, NodeInitWithValue(3));

    ASSERT_LIST_EQ(list, std::array<int, 3>{
                             {2, 1, 3}
    });
}

TEST(TestList, Test2) {
    List list{};
    dl_node_t node1 = NodeInitWithValue(2);

    dl_list_push_back(list, NodeInitWithValue(1));
    dl_list_push_back(list, node1);
    dl_list_push_back(list, NodeInitWithValue(3));

    ASSERT_LIST_EQ(list, std::array<int, 3>{
                             {1, 2, 3}
    });

    dl_node_t node2 = NodeInitWithValue(4);
    dl_list_push_back(list, node2);
    dl_list_push_back(list, NodeInitWithValue(5));
    dl_node_free(dl_list_remove(list, node1), free);

    ASSERT_LIST_EQ(list, std::array<int, 4>{
                             {1, 3, 4, 5}
    });

    dl_node_free(dl_list_pop_front(list), free);
    dl_node_free(dl_list_pop_back(list), free);

    ASSERT_LIST_EQ(list, std::array<int, 2>{
                             {3, 4}
    });

    dl_node_free(dl_list_remove(list, node2), free);

    ASSERT_LIST_EQ(list, std::array<int, 1>{{3}});

    dl_node_free(dl_list_pop_back(list), free);
    ASSERT_TRUE(dl_list_is_empty(list));
}

TEST(TestList, Test3) {
    List list{};
    dl_node_t node1 = NodeInitWithValue(1);

    dl_list_push_back(list, node1);
    dl_list_push_back(list, NodeInitWithValue(3));
    dl_list_insert_after(list, node1, NodeInitWithValue(2));

    ASSERT_LIST_EQ(list, std::array<int, 3>{
                             {1, 2, 3}
    });

    dl_list_insert_after(list, dl_list_get_first(list), NodeInitWithValue(4));

    ASSERT_LIST_EQ(list, std::array<int, 4>{
                             {1, 4, 2, 3}
    });

    dl_list_push_back(list, dl_list_pop_front(list));

    ASSERT_LIST_EQ(list, std::array<int, 4>{
                             {4, 2, 3, 1}
    });

    dl_list_push_front(list, dl_list_pop_back(list));

    ASSERT_LIST_EQ(list, std::array<int, 4>{
                             {1, 4, 2, 3}
    });
}

TEST(TestList, Test4) {
    List list{};
    dl_list_push_front(list, NodeInitWithValue(1));

    dl_node_t node1 = dl_list_pop_back(list);
    ASSERT_NODE_EQ(node1, 1);

    dl_list_push_back(list, node1);
    dl_list_push_back(list, NodeInitWithValue(2));
    dl_list_push_back(list, NodeInitWithValue(3));

    ASSERT_LIST_EQ(list, std::array<int, 3>{
                             {1, 2, 3}
    });

    dl_list_insert_after(list, dl_list_get_last(list), NodeInitWithValue(4));

    ASSERT_LIST_EQ(list, std::array<int, 4>{
                             {1, 2, 3, 4}
    });
}

TEST(TestList, Test5) {
    List list{1, 2, 3, 4, 5, 6, 7, 8, 9};

    ASSERT_LIST_EQ(list, std::array<int, 9>{
                             {1, 2, 3, 4, 5, 6, 7, 8, 9}
    });

    dl_node_t node1 = dl_list_get_first(list);
    while (!NodeEqValue(node1, 5)) {
        node1 = dl_node_get_next(node1);
    }

    dl_list_remove(list, node1);

    ASSERT_LIST_EQ(list, std::array<int, 8>{
                             {1, 2, 3, 4, 6, 7, 8, 9}
    });
}

TEST(TestList, Test6) {
    int a = 0x0DED;
    dl_node_t node = dl_node_init_fam(NULL, sizeof(int), static_cast<void *>(&a));
    void *data = dl_node_get_fam(node);
    ASSERT_EQ(*static_cast<int *>(data), 0x0DED);
    dl_node_free(node, NULL);
}

// Run all tests
int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}