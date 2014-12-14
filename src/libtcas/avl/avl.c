/*
 *  Copyright (C) 2011 milkyjing <milkyjing@gmail.com>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * milkyjing
 *
 */

#include <stdlib.h>
#include <memory.h>

#include "avl.h"


AvlTreePtr avltree_create(avl_compare_func compare_f, avl_copy_func copy_f, avl_cleanup_func clean_f, void *param) {
    AvlTreePtr tree;
    tree = (AvlTreePtr)malloc(sizeof(AvlTree));
    tree->root = NULL;
    tree->compare_f = compare_f;
    tree->param = param;
    tree->count = 0;
    tree->copy_f = copy_f;
    tree->clean_f = clean_f;
    return tree;
}

avl_entry avltree_find(const AvlTreePtr tree, const avl_entry key) {
    int res;
    AvlNodePtr p;
    p = tree->root;
    while (p) {
        res = tree->compare_f(key, p->data, tree->param);
        if (res < 0)
            p = p->link[0];
        else if (res > 0)
            p = p->link[1];
        else {    /* res == 0 */
            if (tree->copy_f)
                return tree->copy_f(p->data, tree->param);
            else
                return p->data;
        }
    }
    return NULL;
}

avl_entry *avltree_probe(AvlTreePtr tree, const avl_entry item) {
    AvlNodePtr y, z;    /* Top node to update balance factor, and parent. */
    AvlNodePtr p, q;    /* Iterator, and parent. */
    AvlNodePtr n;       /* Newly inserted node. */
    AvlNodePtr w;       /* New root of rebalanced subtree. */
    int dir, k, res;                        /* Direction to descend. Number of cached results. compare result */
    unsigned char da[AVL_MAX_HEIGHT];       /* Cached comparison results. */
    z = (AvlNodePtr) &tree->root;
    y = tree->root;
    dir = 0, k = 0;
    for (q = z, p = y; p != NULL; q = p, p = p->link[dir]) {
        res = tree->compare_f(item, p->data, tree->param);
        if (0 == res)
            return NULL;    // &p->data;  duplicate key
        if (p->balance != 0)
            z = q, y = p, k = 0;
        da[k++] = dir = (res > 0);
    }
    n = q->link[dir] = (AvlNodePtr)malloc(sizeof(AvlNode));
    tree->count++;
    if (tree->copy_f)
        n->data = tree->copy_f(item, tree->param);
    else
        n->data = item;         /* copy data */
    n->link[0] = n->link[1] = NULL;
    n->balance = 0;
    if (!y)
        return &n->data;
    for (p = y, k = 0; p != n; p = p->link[da[k]], k++) {
        if (0 == da[k])
            p->balance--;
        else
            p->balance++;
    }
    if (-2 == y->balance) {
        AvlNodePtr x;
        x = y->link[0];
        if (-1 == x->balance) {
            w = x;
            y->link[0] = x->link[1];
            x->link[1] = y;
            x->balance = y->balance = 0;
        } else {
            w = x->link[1];
            x->link[1] = w->link[0];
            w->link[0] = x;
            y->link[0] = w->link[1];
            w->link[1] = y;
            if (-1 == w->balance)
                x->balance = 0, y->balance = +1;
            else if (0 == w->balance)
                x->balance = y->balance = 0;
            else        /* |w->balance == +1| */
                x->balance = -1, y->balance = 0;
            w->balance = 0;
        }
    } else if (+2 == y->balance) {
        AvlNodePtr x = y->link[1];
        if (+1 == x->balance) {
            w = x;
            y->link[1] = x->link[0];
            x->link[0] = y;
            x->balance = y->balance = 0;
        } else {
            w = x->link[0];
            x->link[0] = w->link[1];
            w->link[1] = x;
            y->link[1] = w->link[0];
            w->link[0] = y;
            if (+1 == w->balance)
                x->balance = 0, y->balance = -1;
            else if (0 == w->balance)
                x->balance = y->balance = 0;
            else        /* |w->balance == -1| */
                x->balance = +1, y->balance = 0;
            w->balance = 0;
        }
    } else
        return &n->data;
    z->link[y != z->link[0]] = w;
    return &n->data;
}

int avltree_insert(AvlTreePtr tree, const avl_entry item) {
    avl_entry *p = avltree_probe(tree, item);
    return (p == NULL) ? -1 : 0;
}

int avltree_delete(AvlTreePtr tree, const avl_entry key) {
    AvlNodePtr pa[AVL_MAX_HEIGHT]; /* Stack of Nodes. */
    unsigned char da[AVL_MAX_HEIGHT];    /* |link[]| indexes. */
    int k, res;                               /* Stack pointer. Result of comparison between |item| and |p|. */
    AvlNodePtr p;   /* Traverses tree to find node to delete. */
    avl_entry item;
    k = 0;
    p = (AvlNodePtr) &tree->root;
    for (res = -1; res != 0; res = tree->compare_f(key, p->data, tree->param)) {
        int dir = res > 0;
        pa[k] = p;
        da[k++] = dir;
        p = p->link[dir];
        if (!p)
            return -1;
    }
    item = p->data;
    if (!p->link[1])
        pa[k - 1]->link[da[k - 1]] = p->link[0];
    else {
        AvlNodePtr r = p->link[1];
        if (!r->link[0]) {
            r->link[0] = p->link[0];
            r->balance = p->balance;
            pa[k - 1]->link[da[k - 1]] = r;
            da[k] = 1;
            pa[k++] = r;
        } else {
            AvlNodePtr s;
            int j = k++;
            while (1) {
                da[k] = 0;
                pa[k++] = r;
                s = r->link[0];
                if (!s->link[0])
                    break;
                r = s;
            }
            s->link[0] = p->link[0];
            r->link[0] = s->link[1];
            s->link[1] = p->link[1];
            s->balance = p->balance;
            pa[j - 1]->link[da[j - 1]] = s;
            da[j] = 1;
            pa[j] = s;
        }
    }
    if (tree->clean_f)
        tree->clean_f(item, tree->param);
    free(p);
    while (--k > 0) {
        AvlNodePtr y = pa[k];
        if (0 == da[k]) {
            y->balance++;
            if (+1 == y->balance)
                break;
            else if (+2 == y->balance) {
                AvlNodePtr x = y->link[1];
                if (-1 == x->balance) {
                    AvlNodePtr w;
                    w = x->link[0];
                    x->link[0] = w->link[1];
                    w->link[1] = x;
                    y->link[1] = w->link[0];
                    w->link[0] = y;
                    if (+1 == w->balance)
                        x->balance = 0, y->balance = -1;
                    else if (0 == w->balance)
                        x->balance = y->balance = 0;
                    else    /* |w->balance == -1| */
                        x->balance = +1, y->balance = 0;
                    w->balance = 0;
                    pa[k - 1]->link[da[k - 1]] = w;
                } else {
                    y->link[1] = x->link[0];
                    x->link[0] = y;
                    pa[k - 1]->link[da[k - 1]] = x;
                    if (0 == x->balance) {
                        x->balance = -1;
                        y->balance = +1;
                        break;
                    } else
                        x->balance = y->balance = 0;
                }
            }
        } else {
            y->balance--;
            if (-1 == y->balance)
                break;
            else if (-2 == y->balance) {
                AvlNodePtr x = y->link[0];
                if (+1 == x->balance) {
                    AvlNodePtr w;
                    w = x->link[1];
                    x->link[1] = w->link[0];
                    w->link[0] = x;
                    y->link[0] = w->link[1];
                    w->link[1] = y;
                    if (-1 == w->balance)
                        x->balance = 0, y->balance = +1;
                    else if (0 == w->balance)
                        x->balance = y->balance = 0;
                    else    /* |w->balance == +1| */
                        x->balance = -1, y->balance = 0;
                    w->balance = 0;
                    pa[k - 1]->link[da[k - 1]] = w;
                } else {
                    y->link[0] = x->link[1];
                    x->link[1] = y;
                    pa[k - 1]->link[da[k - 1]] = x;
                    if (0 == x->balance) {
                        x->balance = +1;
                        y->balance = -1;
                        break;
                    } else
                        x->balance = y->balance = 0;
                }
            }
        }
    }
    tree->count--;
    return 0;
}

void avltree_destroy(AvlTreePtr tree) {
    AvlNodePtr p, q;
    for (p = tree->root; p != NULL; p = q) {
        if (!p->link[0]) {
            q = p->link[1];
            if (tree->clean_f)
                tree->clean_f(p->data, tree->param);
            free(p);
        } else {
            q = p->link[0];
            p->link[0] = q->link[1];
            q->link[1] = p;
        }
    }
    free(tree);
}


