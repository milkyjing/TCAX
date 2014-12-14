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

#include "rb.h"


RbTreePtr rbtree_create(rb_compare_func compare_f, rb_copy_func copy_f, rb_cleanup_func clean_f, void *param) {
    RbTreePtr tree;
    tree = (RbTreePtr)malloc(sizeof(RbTree));
    tree->root = NULL;
    tree->compare_f = compare_f;
    tree->param = param;
    tree->count = 0;
    tree->copy_f = copy_f;
    tree->clean_f = clean_f;
    return tree;
}

rb_entry_ptr rbtree_find(const RbTreePtr tree, const rb_entry_ptr key) {
    int res;
    RbNodePtr p;
    p = tree->root;
    while (p) {
        res = tree->compare_f(key, p->data, tree->param);
        if (res < 0)
            p = p->link[0];
        else if (res > 0)
            p = p->link[1];
        else {    /* |cmp == 0| */
            if (tree->copy_f)
                return tree->copy_f(p->data, tree->param);        /* we will create the buffer in the function and return the pointer */
            else
                return p->data;
        }
    }
    return NULL;
}

rb_entry_ptr *rbtree_probe(RbTreePtr tree, const rb_entry_ptr item) {
    RbNodePtr pa[RB_MAX_HEIGHT];        /* Nodes on stack. */
    unsigned char da[RB_MAX_HEIGHT];    /* Directions moved from stack nodes. */
    int k, res;                         /* Stack height. Compare result */
    RbNodePtr p;                        /* Traverses tree looking for insertion point. */
    RbNodePtr n;                        /* Newly inserted node. */
    pa[0] = (RbNodePtr) &tree->root;
    da[0] = 0;
    k = 1;
    for (p = tree->root; p != NULL; p = p->link[da[k - 1]]) {
        res = tree->compare_f(item, p->data, tree->param);
        if (0 == res)
            return NULL;    // &p->data;  duplicate key
        pa[k] = p;
        da[k++] = res > 0;
    }
    n = pa[k - 1]->link[da[k - 1]] = (RbNodePtr)malloc(sizeof(RbNode));
    if (tree->copy_f)
        n->data = tree->copy_f(item, tree->param);
    else
        n->data = item;
    n->link[0] = n->link[1] = NULL;
    n->color = RB_RED;
    tree->count++;
    while (k >= 3 && RB_RED == pa[k - 1]->color) {
        if (0 == da[k - 2]) {
            RbNodePtr y;
            y = pa[k - 2]->link[1];
            if (y && RB_RED == y->color) {
                pa[k - 1]->color = y->color = RB_BLACK;
                pa[k - 2]->color = RB_RED;
                k -= 2;
            } else {
                RbNodePtr x;
                if (0 == da[k - 1])
                    y = pa[k - 1];
                else {
                    x = pa[k - 1];
                    y = x->link[1];
                    x->link[1] = y->link[0];
                    y->link[0] = x;
                    pa[k - 2]->link[0] = y;
                }
                x = pa[k - 2];
                x->color = RB_RED;
                y->color = RB_BLACK;
                x->link[0] = y->link[1];
                y->link[1] = x;
                pa[k - 3]->link[da[k - 3]] = y;
                break;
            }
        } else {
            RbNodePtr y;
            y = pa[k - 2]->link[0];
            if (y && RB_RED == y->color) {
                pa[k - 1]->color = y->color = RB_BLACK;
                pa[k - 2]->color = RB_RED;
                k -= 2;
            } else {
                RbNodePtr x;
                if (1 == da[k - 1])
                    y = pa[k - 1];
                else {
                    x = pa[k - 1];
                    y = x->link[0];
                    x->link[0] = y->link[1];
                    y->link[1] = x;
                    pa[k - 2]->link[1] = y;
                }
                x = pa[k - 2];
                x->color = RB_RED;
                y->color = RB_BLACK;
                x->link[1] = y->link[0];
                y->link[0] = x;
                pa[k - 3]->link[da[k - 3]] = y;
                break;
            }
        }
    }
    tree->root->color = RB_BLACK;
    return &n->data;
}

int rbtree_insert(RbTreePtr tree, const rb_entry_ptr item) {
    rb_entry_ptr *p = rbtree_probe(tree, item);
    return (p == NULL) ? -1 : 0;
}

int rbtree_delete(RbTreePtr tree, const rb_entry_ptr key) {
    RbNodePtr pa[RB_MAX_HEIGHT];        /* Nodes on stack. */
    unsigned char da[RB_MAX_HEIGHT];    /* Directions moved from stack nodes. */
    int k, res;                             /* Stack height. Result of comparison between |item| and |p|. */
    RbNodePtr p;    /* The node to delete, or a node part way to it. */
    rb_entry_ptr item;
    k = 0;
    p = (RbNodePtr) &tree->root;
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
        RbColor t;
        RbNodePtr r = p->link[1];
        if (!r->link[0]) {
            r->link[0] = p->link[0];
            t = (RbColor)r->color;
            r->color = p->color;
            p->color = t;
            pa[k - 1]->link[da[k - 1]] = r;
            da[k] = 1;
            pa[k++] = r;
        } else {
            RbNodePtr s;
            int j = k++;
            while (1) {
                da[k] = 0;
                pa[k++] = r;
                s = r->link[0];
                if (!s->link[0])
                    break;
                r = s;
            }
            da[j] = 1;
            pa[j] = s;
            pa[j - 1]->link[da[j - 1]] = s;
            s->link[0] = p->link[0];
            r->link[0] = s->link[1];
            s->link[1] = p->link[1];
            t = (RbColor)s->color;
            s->color = p->color;
            p->color = t;
        }
    }
    if (RB_BLACK == p->color) {
        while (1) {
            RbNodePtr x = pa[k - 1]->link[da[k - 1]];
            if (x && RB_RED == x->color) {
                x->color = RB_BLACK;
                break;
            }
            if (k < 2)
                break;
            if (0 == da[k - 1]) {
                RbNodePtr w = pa[k - 1]->link[1];
                if (RB_RED == w->color) {
                    w->color = RB_BLACK;
                    pa[k - 1]->color = RB_RED;
                    pa[k - 1]->link[1] = w->link[0];
                    w->link[0] = pa[k - 1];
                    pa[k - 2]->link[da[k - 2]] = w;
                    pa[k] = pa[k - 1];
                    da[k] = 0;
                    pa[k - 1] = w;
                    k++;
                    w = pa[k - 1]->link[1];
                }
                if ((!w->link[0] || RB_BLACK == w->link[0]->color) && (!w->link[1] || RB_BLACK == w->link[1]->color))
                    w->color = RB_RED;
                else {
                    if (!w->link[1] || RB_BLACK == w->link[1]->color) {
                        RbNodePtr y = w->link[0];
                        y->color = RB_BLACK;
                        w->color = RB_RED;
                        w->link[0] = y->link[1];
                        y->link[1] = w;
                        w = pa[k - 1]->link[1] = y;
                    }
                    w->color = pa[k - 1]->color;
                    pa[k - 1]->color = RB_BLACK;
                    w->link[1]->color = RB_BLACK;
                    pa[k - 1]->link[1] = w->link[0];
                    w->link[0] = pa[k - 1];
                    pa[k - 2]->link[da[k - 2]] = w;
                    break;
                }
            } else {
                RbNodePtr w = pa[k - 1]->link[0];
                if (RB_RED == w->color) {
                    w->color = RB_BLACK;
                    pa[k - 1]->color = RB_RED;
                    pa[k - 1]->link[0] = w->link[1];
                    w->link[1] = pa[k - 1];
                    pa[k - 2]->link[da[k - 2]] = w;
                    pa[k] = pa[k - 1];
                    da[k] = 1;
                    pa[k - 1] = w;
                    k++;
                    w = pa[k - 1]->link[0];
                }
                if ((!w->link[0] || RB_BLACK == w->link[0]->color) && (!w->link[1] || RB_BLACK == w->link[1]->color))
                    w->color = RB_RED;
                else {
                    if (!w->link[0] || RB_BLACK == w->link[0]->color) {
                        RbNodePtr y = w->link[1];
                        y->color = RB_BLACK;
                        w->color = RB_RED;
                        w->link[1] = y->link[0];
                        y->link[0] = w;
                        w = pa[k - 1]->link[0] = y;
                    }
                    w->color = pa[k - 1]->color;
                    pa[k - 1]->color = RB_BLACK;
                    w->link[0]->color = RB_BLACK;
                    pa[k - 1]->link[0] = w->link[1];
                    w->link[1] = pa[k - 1];
                    pa[k - 2]->link[da[k - 2]] = w;
                    break;
                }
            }
            k--;
        }
    }
    if (tree->clean_f)
        tree->clean_f(item, tree->param);
    free(p);
    tree->count--;
    return 0;
}

rb_entry_ptr rbtree_min(const RbTreePtr tree) {
    RbNodePtr p;
    p = tree->root;
    if (!p)
        return NULL;
    while (p && p->link[0])
        p = p->link[0];
    return p->data;
}

rb_entry_ptr rbtree_max(const RbTreePtr tree) {
    RbNodePtr p;
    p = tree->root;
    if (!p)
        return NULL;
    while (p && p->link[1])
        p = p->link[1];
    return p->data;
}

void rbtree_destroy(RbTreePtr tree) {
    RbNodePtr p, q;
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

