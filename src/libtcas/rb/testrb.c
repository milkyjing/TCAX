#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "rb.h"


typedef void (*node_traverse_func)(const rb_entry);        /* indicates how to print the node */

void print(const rb_entry item) {
    printf("%i\n", *(int *)item);
}

static void _bstree_traverse_inorder(RbNodePtr node, node_traverse_func traverse_f) {
    if (node) {
        _bstree_traverse_inorder(node->link[0], traverse_f);
        traverse_f(node->data);
        _bstree_traverse_inorder(node->link[1], traverse_f);
    }
}

void bstree_traverse_inorder(const RbTreePtr tree) {
    _bstree_traverse_inorder(tree->root, print);
}

int compare(const rb_entry a, const rb_entry b, const void *param) {
    if (*(int *)a < *(int *)b)
        return -1;
    else if (*(int *)a > *(int *)b)
        return 1;
    else
        return 0;
}

void clean(rb_entry item, const void *param) {
    free(item);
}

int is_red (const RbNodePtr node) {
    return node && RB_RED == node->color;
}

int rbtree_validate(const RbNodePtr node) {
    int lh, rh;
    if (!node)
        return 1;
    else {
        RbNodePtr ln, rn;
        ln = node->link[0];
        rn = node->link[1];
        /* Consecutive red links */
        if (is_red(node)) {
            if (is_red(ln) || is_red(rn)) {
                printf("Red violation\n");
                return 0;
            }
        }
        lh = rbtree_validate(ln);
        rh = rbtree_validate(rn);
        /* Invalid binary search tree */
        if ((ln && compare(ln->data, node->data, NULL) >= 0) || (rn && compare(rn->data, node->data, NULL) <= 0)) {
            printf("Binary tree violation\n");
            return 0;
        }
        /* Black height mismatch */
        if (lh != 0 && rh != 0 && lh != rh) {
            printf("Black violation\n");
            return 0;
        }
        /* Only count black links */
        if (lh != 0 && rh != 0)
            return is_red(node) ? lh : lh + 1;
        else
            return 0;
    }
}

int main() {
    RbTreePtr tree;
    int *p;
    int i, count;
    srand((unsigned int)time(NULL));
    tree = rbtree_create(compare, NULL, clean, NULL);
    count = 1000;
    for (i = 0; i < count; i++) {
        p = (int *)malloc(sizeof(int));
        *p = rand() % (count / 10); //rand() * 65535 + rand();
        if (rbtree_insert(tree, p) == -1) {
            rbtree_delete(tree, p);
            free(p);
        }
    }
    if (!rbtree_validate(tree->root)) {
        printf("invalid red-black tree\n");
        return 0;
    }
    printf("%i\n\n", rbtree_count(tree));
    bstree_traverse_inorder(tree);
    rbtree_destroy(tree);
    return 0;
}
