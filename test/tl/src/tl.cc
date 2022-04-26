#include "dllist.h"
#include <array>
#include <cstdio>
#include <gtest/gtest.h>

// SetUp and TearDown calling constructor and destructor of dl_list
// All data is assumed to be allocated on heap with calloc or malloc and freed with free()
// Use ints for tests
class TestList : public ::testing::Test {
protected:
  void SetUp() {
    list = dl_list_init();
  }

  void TearDown() {
    dl_list_free(list, free);
    list = NULL;
  }

  dl_list_t list;
};

// Template function to compare list to a std::array of integers
// If list differs from the array, then exit the test is failed
template <std::size_t N> void ASSERT_LIST_EQ(dl_list_t list, std::array<int, N> array) {
  dl_node_t node = dl_list_get_first(list);
  std::size_t i = 0;

  while (node && i < array.size()) {
    ASSERT_EQ(*(int *)dl_node_get_data(node), array[i]);
    node = dl_node_get_next(node);
    i++;
  }

  ASSERT_EQ(node, nullptr);
  ASSERT_EQ(i, array.size());
}

// Initialize dl_node_t with ptr to an allocated int with value
dl_node_t NodeInitWithValue(int value) {
  dl_node_t node = dl_node_init(NULL);
  int *ptr = (int *)calloc(1, sizeof(int));
  *ptr = value;
  dl_node_set_data(node, (void *)ptr);
  return node;
}

TEST_F(TestList, TestPush) {
  dl_list_push_back(list, NodeInitWithValue(1));
  dl_list_push_front(list, NodeInitWithValue(2));
  dl_list_push_back(list, NodeInitWithValue(3));

  ASSERT_LIST_EQ(list, std::array<int, 3>{
                           {2, 1, 3}
  });
}

TEST_F(TestList, TestPop) {
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

// Run all tests
int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}