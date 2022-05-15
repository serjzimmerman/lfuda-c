#include "rbtree.h"

#include <limits.h>
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

static void rb_tree_free_no_data_impl(rb_node_t *root) {
    assert(root);

    if (root->right) {
        rb_tree_free_no_data_impl(root->right);
    }

    if (root->left) {
        rb_tree_free_no_data_impl(root->left);
    }

    free(root);
}

static void rb_tree_free_data_impl(rb_node_t *root, rb_free_func_t data_free) {
    assert(root);
    assert(data_free);

    if (root->right) {
        rb_tree_free_data_impl(root->right, data_free);
    }

    if (root->left) {
        rb_tree_free_data_impl(root->left, data_free);
    }

    data_free(root->data);
    free(root);
}

void rb_tree_free(rb_tree_t tree_, rb_free_func_t data_free) {
    struct rb_tree_s *tree = (struct rb_tree_s *)tree_;

    if (!tree->root) {
        free(tree);
        return;
    }

    if (data_free) {
        rb_tree_free_data_impl(tree->root, data_free);
    } else {
        rb_tree_free_no_data_impl(tree->root);
    }

    free(tree);
}

// If node is NULL then it is considered black
static inline enum node_color_e get_node_color(rb_node_t *node) {
    return (node ? node->color : COLOR_BLACK);
}

// Check wheter the node is a left child
static inline int is_left_child(rb_node_t *node) {
    assert(node);
    assert(node->parent);

    return (node == node->parent->left ? 1 : 0);
}

// Check wheter the node is a right child
static inline int is_right_child(rb_node_t *node) {
    assert(node);
    assert(node->parent);

    return (node == node->parent->right ? 1 : 0);
}

static inline rb_node_t *get_sibling(rb_node_t *node) {
    assert(node);
    assert(node->parent);

    if (is_left_child(node)) {
        return node->parent->right;
    } else {
        return node->parent->left;
    }
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
    } else if (is_left_child(root)) {
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
    } else if (is_right_child(root)) {
        root->parent->right = lchild;
    } else {
        root->parent->left = lchild;
    }

    lchild->right = root;
    root->parent = lchild;
}

static void rotate_to_parent(struct rb_tree_s *tree, rb_node_t *node) {
    assert(tree);
    assert(node);

    if (is_left_child(node)) {
        right_rotate(tree, node->parent);
    } else {
        left_rotate(tree, node->parent);
    }
}

static void replace_node(rb_node_t *root, rb_node_t *child, struct rb_tree_s *tree) {
    assert(root);
    assert(tree);

    if (child) {
        child->parent = root->parent;
    }

    // If root is the root of the tree, then set tree->root to the child
    if (!root->parent) {
        tree->root = child;
        return;
    }

    if (is_left_child(root)) {
        root->parent->left = child;
    } else {
        root->parent->right = child;
    }
}

static rb_node_t *get_uncle(rb_node_t *node) {
    assert(node);
    assert(node->parent);
    assert(node->parent->parent);

    if (is_right_child(node->parent)) {
        return node->parent->parent->left;
    } else {
        return node->parent->parent->right;
    }
}

static int is_linear(rb_node_t *node) {
    assert(node);
    assert(node->parent);
    assert(node->parent->parent);

    return (is_left_child(node) && is_left_child(node->parent)) ||
           (is_right_child(node) && is_right_child(node->parent));
}

// https://www.youtube.com/watch?v=KRWm1uhqMNI
// This is an incredibly good explanation of the algorithm
static void recolor_after_insert(struct rb_tree_s *tree, rb_node_t *node) {
    assert(tree);
    assert(node);

    while (get_node_color(node->parent) == COLOR_RED) {
        if (node == tree->root || (node->parent && node->parent->color == COLOR_BLACK)) {
            break;
        }

        rb_node_t *uncle = get_uncle(node);
        if (get_node_color(uncle) == COLOR_RED) {
            node->parent->color = COLOR_BLACK;
            uncle->color = COLOR_BLACK;
            node->parent->parent->color = COLOR_RED;
            node = node->parent->parent;
        } else {
            if (!is_linear(node)) {
                rb_node_t *old = node->parent;
                rotate_to_parent(tree, node);
                node = old;
            }
            node->parent->color = COLOR_BLACK;
            node->parent->parent->color = COLOR_RED;
            rotate_to_parent(tree, node->parent);
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
}

const void *rb_tree_lookup(rb_tree_t tree_, void *key) {
    struct rb_tree_s *tree = (struct rb_tree_s *)tree_;

    assert(tree);
    assert(key);

    rb_node_t *found = rb_tree_lookup_impl(tree->root, key, tree->cmp);
    return (found ? found->data : NULL);
}

static rb_node_t *rb_tree_get_max_impl(rb_node_t *root) {
    while (root->right) {
        root = root->right;
    }
    return root;
}

static rb_node_t *rb_tree_get_min_impl(rb_node_t *root) {
    while (root->left) {
        root = root->left;
    }
    return root;
}

static inline void swap_data(rb_node_t *a, rb_node_t *b) {
    void *temp = a->data;
    a->data = b->data;
    b->data = temp;
}

// This function swaps todelete node data with a leaf node that can then be safely pruned after red-black violation
// fix-up
static rb_node_t *rb_tree_remove_impl(rb_node_t *root, struct rb_tree_s *tree, void *todelete) {
    assert(tree);
    assert(todelete);

    rb_node_t *node = rb_tree_lookup_impl(root, todelete, tree->cmp);

    // Trying to delete a node that is not present in the tree
    if (!node) {
        return NULL;
    }

    // Result is the node that will be deleted and passed back from the function
    rb_node_t *found = node;

    // Case 1: The node has 0 children
    if (!found->left && !found->right) {
        return found;
    }

    // Case 2 and 3: The node has a single child or 2 children
    if (found->right) {
        rb_node_t *successor = rb_tree_get_min_impl(found->right);
        swap_data(found, successor);
        rb_tree_remove_impl(found->right, tree, successor->data);
    } else {
        rb_node_t *predecessor = rb_tree_get_max_impl(found->left);
        swap_data(found, predecessor);
        rb_tree_remove_impl(found->left, tree, predecessor->data);
    }
}

static inline void prune_leaf(struct rb_tree_s *tree, rb_node_t *toprune) {
    assert(tree);
    assert(toprune);

    if (!toprune->parent) {
        tree->root = NULL;
        return;
    }

    if (is_left_child(toprune)) {
        toprune->parent->left = NULL;
    } else {
        toprune->parent->right = NULL;
    }
}

static void recolor_after_remove(struct rb_tree_s *tree, rb_node_t *leaf) {
    while (get_node_color(leaf) != COLOR_RED) {
        if (!leaf->parent) {
            break;
        }

        rb_node_t *sibling = get_sibling(leaf);
        if (get_node_color(sibling) == COLOR_RED) {
            leaf->parent->color = COLOR_RED;
            sibling->color = COLOR_BLACK;
            rotate_to_parent(tree, sibling);
            continue;
        }

        // Nephew is the right child of a sibling
        // Niece is the left child of a sibling
        rb_node_t *nephew = (sibling ? sibling->right : NULL);
        if (get_node_color(nephew) == COLOR_RED) {
            sibling->color = leaf->parent->color;
            leaf->parent->color = COLOR_BLACK;
            nephew->color = COLOR_BLACK;
            rotate_to_parent(tree, sibling);
            leaf = tree->root;
            break;
        }

        rb_node_t *niece = (sibling ? sibling->left : NULL);
        if (get_node_color(niece) == COLOR_RED) {
            niece->color = COLOR_BLACK;
            sibling->color = COLOR_RED;
            rotate_to_parent(tree, niece);
        } else {
            if (sibling) {
                sibling->color = COLOR_RED;
            }
            leaf = leaf->parent;
        }
    }

    leaf->color = COLOR_BLACK;
}

void *rb_tree_remove(rb_tree_t tree_, void *toremove) {
    struct rb_tree_s *tree = (struct rb_tree_s *)tree_;

    // 1. Run regular bst removal algorithm, which moves toremove to a leaf node
    rb_node_t *leaf = rb_tree_remove_impl(tree->root, tree, toremove);
    if (!leaf) {
        return NULL;
    }

    void *result = leaf->data;

    // 2. Fix red-black violations
    recolor_after_remove(tree, leaf);

    // 3. Prune the leaf
    prune_leaf(tree, leaf);

    // 4. Free the leaf
    free(leaf);

    return result;
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
static rb_tree_valid_t validate_subtree_rb(rb_node_t *root) {
    // Check Rule@3
    if (!root) {
        return valid_inits(1, 1);
    }

    // Check Rule@4
    if (get_node_color(root) == COLOR_RED &&
        (get_node_color(root->left) == COLOR_RED || get_node_color(root->right) == COLOR_RED)) {
        WARNING("Property 4 violated: two consecutive red nodes\n");
        return valid_inits(0, 0);
    }

    // Check Rule@5

    rb_tree_valid_t right = validate_subtree_rb(root->right);
    rb_tree_valid_t left = validate_subtree_rb(root->left);

    if (!right.valid || !left.valid || (right.blacks != left.blacks)) {
        if (right.blacks != left.blacks) {
            WARNING("Property 5 violated: black height does not match: %lu != %lu\n", right.blacks, left.blacks);
        }
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

    rb_tree_valid_t valid = validate_subtree_rb(tree->root);

    return valid.valid;
}

static inline int iabs(const int value) {
    return (value > 0 ? value : -value);
}

const void *rb_tree_closest_left_impl(rb_node_t *root, void *key, rb_cmp_func_t cmp) {
    assert(root);
    assert(key);

    void *closest_data = NULL;
    int closest_diff = INT_MAX;

    while (root) {
        int current_diff = cmp(root->data, key);
        if (current_diff <= 0) {
            if (iabs(current_diff) < closest_diff) {
                closest_diff = iabs(current_diff);
                closest_data = root->data;
            }
            root = root->right;
        } else if (current_diff > 0) {
            root = root->left;
        } else {
            break;
        }
    }

    return closest_data;
}

const void *rb_tree_closest_left(rb_tree_t tree_, void *key) {
    struct rb_tree_s *tree = (struct rb_tree_s *)tree_;

    assert(tree);

    return (tree->root ? rb_tree_closest_left_impl(tree->root, key, tree->cmp) : NULL);
}

const void *rb_tree_closest_right_impl(rb_node_t *root, void *key, rb_cmp_func_t cmp) {
    assert(root);
    assert(key);

    void *closest_data = NULL;
    int closest_diff = INT_MAX;

    while (root) {
        int current_diff = cmp(root->data, key);
        if (current_diff < 0) {
            root = root->right;
        } else if (current_diff >= 0) {
            if (iabs(current_diff) < closest_diff) {
                closest_diff = iabs(current_diff);
                closest_data = root->data;
            }
            root = root->left;
        } else {
            break;
        }
    }

    return closest_data;
}

const void *rb_tree_closest_right(rb_tree_t tree_, void *key) {
    struct rb_tree_s *tree = (struct rb_tree_s *)tree_;

    assert(tree);

    return (tree->root ? rb_tree_closest_right_impl(tree->root, key, tree->cmp) : NULL);
}