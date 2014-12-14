#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "avl.h"


typedef void (*node_traverse_func)(const avl_entry);        /* indicates how to print the node */

void print(const avl_entry item) {
    printf("%i\n", *(int *)item);
}

static void _bstree_traverse_inorder(AvlNodePtr node, node_traverse_func traverse_f) {
    if (node) {
        _bstree_traverse_inorder(node->link[0], traverse_f);
        traverse_f(node->data);
        _bstree_traverse_inorder(node->link[1], traverse_f);
    }
}

void bstree_traverse_inorder(const AvlTreePtr tree) {
    _bstree_traverse_inorder(tree->root, print);
}

int compare(const avl_entry a, const avl_entry b, const void *param) {
    if (*(int *)a < *(int *)b)
        return -1;
    else if (*(int *)a > *(int *)b)
        return 1;
    else
        return 0;
}

void clean(avl_entry item, const void *param) {
    free(item);
}

int main() {
    AvlTreePtr tree;
    int *p;
    int i, count;
    srand((unsigned int)time(NULL));
    tree = avltree_create(compare, NULL, clean, NULL);
    count = 1000;
    for (i = 0; i < count; i++) {
        p = (int *)malloc(sizeof(int));
        *p = rand() % (count / 10); //rand() * 65535 + rand();
        if (avltree_insert(tree, p) == -1) {
            avltree_delete(tree, p);
            free(p);
        }
    }
    printf("%i\n\n", avltree_count(tree));
    bstree_traverse_inorder(tree);
    avltree_destroy(tree);
    return 0;
}
