#include <stdio.h>

#include "dllist.h"
#include "error.h"

const char *array[] = {"Red", "Green", "Blue"};

int main() {
  dl_list_t list = dl_list_init();
  dl_list_push_back(list, dl_node_init((void *)array[0]));
  dl_list_push_front(list, dl_node_init((void *)array[1]));
  dl_list_push_back(list, dl_node_init((void *)array[2]));

  while (!dl_list_is_empty(list)) {
    dl_node_t node = dl_list_pop_front(list);
    printf("%s\n", (const char *)dl_node_get_data(node));
    dl_node_free(node);
  }

  dl_list_free(list, NULL);
}