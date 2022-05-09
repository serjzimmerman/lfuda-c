#include "rbtree.h"

#include <stdio.h>

#include "error.h"
#include "memutil.h"

#include <assert.h>

enum node_color_e {
    COLOR_RED = 0,
    COLOR_BLACK = 1,
};

typedef struct rb_node_s {
    struct rb_node_s *left, *right, *parent;
    enum node_color_e color;
    void *data;
} rb_node_t;

struct rb_tree_s {
    rb_cmp_func_t cmp;
    rb_node_t *root;
};

rb_node_t *rb_node_init(enum node_color_e color, void *data) {
    rb_node_t *node = calloc_checked(1, sizeof(rb_node_t));
    node->color = color;
    node->data = data;
    return node;
}

rb_tree_t rb_tree_init(rb_cmp_func_t cmp) {
    struct rb_tree_s *tree = calloc_checked(1, sizeof(struct rb_tree_s));
    tree->cmp = cmp;
    return tree;
}

static rb_node_t *rb_tree_bst_insert(struct rb_tree_s *tree, void *toinsert, rb_cmp_func_t cmp) {
    assert(tree);
    assert(toinsert);
    assert(cmp);

    rb_node_t *prev = NULL, *curr = tree->root, *node;

    while (curr) {
        prev = curr;

        int cmp_result = cmp(curr->data, toinsert);

        if (cmp_result < 0) {
            curr = curr->right;
        } else if (cmp_result > 0) {
            curr = curr->left;
        } else {
            return NULL;
        }
    }

    node = rb_node_init(COLOR_RED, toinsert);
    node->parent = prev;

    if (!prev) {
        node->color = COLOR_BLACK;
        tree->root = node;
    } else if (cmp(prev->data, toinsert) > 0) {
        prev->left = node;
    } else {
        prev->right = node;
    }

    return node;
}

// If node is NULL then it is considered black
static inline enum node_color_e get_node_color(rb_node_t *node) {
    return (node ? node->color : COLOR_BLACK);
}

// Perform left rotate operation on the node
static void left_rotate(struct rb_tree_s *tree, rb_node_t *node) {
    assert(tree);
    assert(node);
    assert(node->right);

    rb_node_t *root = node, *rchild = node->right;

    root->right = rchild->left;

    if (rchild->left) {
        rchild->left->parent = root;
    }

    rchild->parent = root->parent;

    if (!root->parent) {
        tree->root = rchild;
    } else if (root->parent->left == root) {
        root->parent->left = rchild;
    } else {
        root->parent->right = rchild;
    }

    rchild->left = root;
    root->parent = rchild;
}

// Perform right rotate operation on the node
static void right_rotate(struct rb_tree_s *tree, rb_node_t *node) {
    assert(tree);
    assert(node);
    assert(node->left);

    rb_node_t *root = node, *lchild = node->left;

    root->left = lchild->right;

    if (lchild->right) {
        lchild->right->parent = root;
    }

    lchild->parent = root->parent;

    if (!root->parent) {
        tree->root = lchild;
    } else if (root->parent->right == root) {
        root->parent->right = lchild;
    } else {
        root->parent->left = lchild;
    }

    lchild->right = root;
    root->parent = lchild;
}

static void recolor_after_insert(struct rb_tree_s *tree, rb_node_t *node) {
    assert(tree);
    assert(node);

    while (get_node_color(node->parent) == COLOR_RED) {
        if (node->parent == node->parent->parent->right) {
            rb_node_t *uncle = node->parent->parent->left;

            if (get_node_color(uncle) == COLOR_RED) {
                node->parent->color = COLOR_BLACK;
                uncle->color = COLOR_BLACK;
                node->parent->parent->color = COLOR_RED;
                node = node->parent->parent;
            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    right_rotate(tree, node);
                }
                node->parent->color = COLOR_BLACK;
                node->parent->parent->color = COLOR_RED;
                left_rotate(tree, node->parent->parent);
            }
        } else {
            rb_node_t *uncle = node->parent->parent->right;

            if (get_node_color(uncle) == COLOR_RED) {
                node->parent->color = COLOR_BLACK;
                uncle->color = COLOR_BLACK;
                node->parent->parent->color = COLOR_RED;
                node = node->parent->parent;
            } else {
                if (node == node->parent->right) {
                    node = node->parent;
                    left_rotate(tree, node);
                }
                node->parent->color = COLOR_BLACK;
                node->parent->parent->color = COLOR_RED;
                right_rotate(tree, node->parent->parent);
            }
        }

        if (node == tree->root) {
            break;
        }
    }

    // For safety's sake set root's color to black
    tree->root->color = COLOR_BLACK;
}

void rb_tree_insert(rb_tree_t tree_, void *toinsert) {
    struct rb_tree_s *tree = (struct rb_tree_s *)tree_;

    assert(tree);

    rb_node_t *node = rb_tree_bst_insert(tree, toinsert, tree->cmp);

    if (!node || !node->parent || !node->parent->parent) {
        return;
    }

    // Fix any red-black violations
    recolor_after_insert(tree, node);
}

static rb_node_t *rb_tree_lookup_impl(rb_node_t *root, void *key, rb_cmp_func_t cmp) {
    assert(cmp);
    assert(key);
// Recursive approach to lookup
#if 0
    int cmp_result;

    if (!root || !(cmp_result = cmp(root->data, key))) {
        return root;
    }

    // Then root->data is less than key, recursively search right branch of the tree
    if (cmp_result < 0) {
        return rb_tree_lookup_impl(root->right, key, cmp);
    }

    return rb_tree_lookup_impl(root->left, key, cmp);
#else
    if (!root) {
        return NULL;
    }

    int cmp_result = cmp(root->data, key);
    while (cmp_result) {
        if (cmp_result < 0) {
            root = root->right;
        } else {
            root = root->left;
        }
        if (!root) {
            break;
        }
        cmp_result = cmp(root->data, key);
    }

    return root;
#endif
}

void *rb_tree_lookup(rb_tree_t tree_, void *key) {
    struct rb_tree_s *tree = (struct rb_tree_s *)tree_;

    assert(tree);
    assert(key);

    rb_node_t *found = rb_tree_lookup_impl(tree->root, key, tree->cmp);
    return (found ? found->data : NULL);
}

// Global variable to keep track of current index for dumping
static unsigned current_index_ = 0;
static const char *const color_names[] = {"red", "black"};

void rb_tree_dump_impl(rb_node_t *root, FILE *fp, rb_stringify_func_t stringify) {
    unsigned current_index = current_index_;

    if (!root) {
        fprintf(fp, "\tnode_%u [label = \"%s\", color = \"%s\"];\n", current_index_++, "NIL", color_names[COLOR_BLACK]);
        return;
    }

    const char *s = stringify(root->data);
    assert(root->color < (sizeof(color_names) / sizeof(color_names[0])));
    fprintf(fp, "\tnode_%u [label = \"%s\", color = \"%s\"];\n", current_index_++, s, color_names[root->color]);

    fprintf(fp, "\tnode_%u -> node_%u;\n", current_index, current_index + 1);
    rb_tree_dump_impl(root->left, fp, stringify);

    fprintf(fp, "\tnode_%u -> node_%u;\n", current_index, current_index_);
    rb_tree_dump_impl(root->right, fp, stringify);
}

void rb_tree_dump(rb_tree_t tree_, FILE *fp, rb_stringify_func_t stringify) {
    struct rb_tree_s *tree = (struct rb_tree_s *)tree_;

    fprintf(fp, "digraph RedBlackTree {\n");

    current_index_ = 0;
    rb_tree_dump_impl(tree->root, fp, stringify);

    fprintf(fp, "}\n");
}

typedef struct {
    size_t blacks;
    int valid;
} rb_tree_valid_t;

static inline rb_tree_valid_t valid_inits(size_t blacks, int valid) {
    rb_tree_valid_t result = {0};
    result.blacks = blacks;
    result.valid = valid;
    return result;
}

// https://cs.kangwon.ac.kr/~leeck/file_processing/red_black_tree.pdf
static rb_tree_valid_t validate_subtree(rb_node_t *root) {
    // Check Rule@3
    if (!root) {
        return valid_inits(1, 1);
    }

    // Check Rule@4
    if (get_node_color(root) == COLOR_RED &&
        (get_node_color(root->left) == COLOR_RED || get_node_color(root->right) == COLOR_RED)) {
        return valid_inits(0, 0);
    }

    // Check Rule@5

    rb_tree_valid_t right = validate_subtree(root->right);
    rb_tree_valid_t left = validate_subtree(root->left);

    if (!right.valid || !left.valid || (right.blacks != left.blacks)) {
        return valid_inits(0, 0);
    } else {
        return valid_inits(right.blacks + (get_node_color(root) == COLOR_BLACK ? 1 : 0), 1);
    }
}

// Validate that all red-black tree properties are followed
int rb_tree_is_valid(rb_tree_t tree_) {
    struct rb_tree_s *tree = (struct rb_tree_s *)tree_;

    assert(tree);

    if (!tree->root) {
        return 1;
    }

    // Check Rule@2
    if (get_node_color(tree->root) != COLOR_BLACK) {
        return 0;
    }

    rb_tree_valid_t valid = validate_subtree(tree->root);

    return valid.valid;
}