#ifndef LFUDA_DLLIST_H
#define LFUDA_DLLIST_H

#ifdef __cplusplus
extern "C" {
#endif

// Handles to doubly linked list and node
typedef void *dl_list_t;
typedef void *dl_node_t;

// Create and return a handle to an empty list
dl_list_t dl_list_init();

// Free list and if data_free != NULL call data_free() for all node->data
void dl_list_free(dl_list_t list_, void (*data_free)(void *));
int dl_list_is_empty(dl_list_t list_);

// Add node to the head of the list
void dl_list_push_front(dl_list_t list_, dl_node_t node_);
// Add node to the end of the list

void dl_list_push_back(dl_list_t list_, dl_node_t node_);
// Add toinsert after node
void dl_list_insert_after(dl_list_t list_, dl_node_t node_, dl_node_t toinsert_);

// Remove node from list, assuming it is present in the list
// It is important not to call this function with mismatched parameters (Obviously)
dl_node_t dl_list_remove(dl_list_t list_, dl_node_t node_);

// Pop first node of the list and return it
dl_node_t dl_list_pop_front(dl_list_t list_);
dl_node_t dl_list_pop_back(dl_list_t list_);

// Get handle to the node at the head of the list
dl_node_t dl_list_get_first(dl_list_t list_);
// Get handle to the node at the tail of the list
dl_node_t dl_list_get_last(dl_list_t list_);

// Create a list node with data member pointing to data and return the handle to it
dl_node_t dl_node_init(void *data);

// Free node and if data_free != NULL call data_free() for node->data
void dl_node_free(dl_node_t node_, void (*data_free)(void *));

dl_node_t dl_node_get_next(dl_node_t node_);
dl_node_t dl_node_get_prev(dl_node_t node_);

// Getter and setter functions for node->data
void *dl_node_get_data(dl_node_t node_);
void dl_node_set_data(dl_node_t node_, void *data);

#ifdef __cplusplus
}
#endif

#endif