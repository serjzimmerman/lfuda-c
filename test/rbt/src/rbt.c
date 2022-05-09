#include <stdio.h>

#include "dllist.h"
#include "error.h"

#include "rbtree.h"

#include "memutil.h"

int intcmp(void *a, void *b) {
    return *(int *)a - *(int *)b;
}

int *int_init_with_value(int a) {
    int *result = calloc_checked(1, sizeof(int));
    *result = a;
    return result;
}

const char *int_stringify(void *a) {
    static char buf[128];
    sprintf(buf, "%d", *(int *)a);
    return buf;
}

int main() {
    rb_tree_t tree = rb_tree_init(intcmp);

    for (int i = 0; i < 256; ++i) {
        int *key = int_init_with_value(rand() % (1 << 16));

        int *found = rb_tree_lookup(tree, key);
        if (!found) {
            rb_tree_insert(tree, key);
        } else {
            printf("%d\n", *found);
        }
    }

    FILE *fp = fopen("test.dot", "w");
    rb_tree_dump(tree, fp, int_stringify);

    int valid = rb_tree_is_valid(tree);

    fclose(fp);

    if (!valid) {
        ERROR("Tree is invalid\n");
    }
}