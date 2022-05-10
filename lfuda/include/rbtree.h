#ifndef LFUDA_RBTREE_H
#define LFUDA_RBTREE_H

#ifdef __cplusplus
#include <cstdio>
extern "C" {
#else
#include <stdio.h>
#endif

typedef void *rb_tree_t;

typedef int (*rb_cmp_func_t)(const void *, const void *);
typedef const char *(rb_stringify_func_t)(const void *);

typedef void (*rb_free_func_t)(void *);

// Initialize an empty red-black tree and return a handle
rb_tree_t rb_tree_init(rb_cmp_func_t cmp);

// Free the tree, when data_free != NULL call it for all node data
void rb_tree_free(rb_tree_t tree_, rb_free_func_t data_free);

// Lookup an element in the tree, return pointer to data if found, else return NULL
void *rb_tree_lookup(rb_tree_t tree_, void *key);

// Remove key from the tree if it is present and return data, else return NULL
void *rb_tree_remove(rb_tree_t tree_, void *toremove);

// Insert key into the tree
void rb_tree_insert(rb_tree_t tree_, void *toinsert);

// Dump tree to a .dot file format
void rb_tree_dump(rb_tree_t tree_, FILE *fp, rb_stringify_func_t stringify);

// Check whether the tree is a valid Red-Black tree
int rb_tree_is_valid(rb_tree_t tree_);

#ifdef __cplusplus
}
#endif

#endif