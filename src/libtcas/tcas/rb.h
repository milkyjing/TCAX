/*
 * Red-Black Tree C Implementation, borrowed the mind of libavl
 * For The libtcas Project
 * Copyright (C) 2011 milkyjing <milkyjing@gmail.com>
 * October 21th, 2011
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

#ifndef __RED_BLACK_TREE_H__
#define __RED_BLACK_TREE_H__
#pragma once

#include <stddef.h>


#define RB_MAX_HEIGHT 48       /* large enough, since the red-black tree is well balanced */

typedef void *rb_entry_ptr;        /* type of the data stored in the node */

typedef enum _rb_color {
    RB_BLACK,   /* Black. */
    RB_RED      /* Red. */
} RbColor;

typedef struct _rb_node {
    struct _rb_node *link[2];       /* to avoid the symmetric code, and should be placed as the first data member for convinent memory access reason */
    rb_entry_ptr data;                  /* Pointer to data. */
    unsigned char color;            /* Color. */
} RbNode, *RbNodePtr;

typedef int (*rb_compare_func)(const rb_entry_ptr, const rb_entry_ptr, const void *);     /* this function is very important, and should be specifed */
typedef rb_entry_ptr (*rb_copy_func)(const rb_entry_ptr, const void *);
typedef void (*rb_cleanup_func)(rb_entry_ptr, const void *);    /* the parameter passed to clean_f is the address of the ith element */

typedef struct _rb_tree {
    RbNodePtr root;                 /* Tree's root. */
    rb_compare_func compare_f;      /* Comparison function. */
    void *param;                    /* extra param for comparison */
    int count;                      /* Number of items in tree. */
    rb_copy_func copy_f;            /* copy function, can be NULL */
    rb_cleanup_func clean_f;        /* cleanup function, can be NULL */
} RbTree, *RbTreePtr;

#define rbtree_count(tree) ((RbTreePtr)(tree)->count)


#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

extern RbTreePtr rbtree_create(rb_compare_func compare_f, rb_copy_func copy_f, rb_cleanup_func clean_f, void *param);

extern rb_entry_ptr rbtree_find(const RbTreePtr tree, const rb_entry_ptr key);

extern rb_entry_ptr *rbtree_probe(RbTreePtr tree, const rb_entry_ptr item);

extern int rbtree_insert(RbTreePtr tree, const rb_entry_ptr item);

extern int rbtree_delete(RbTreePtr tree, const rb_entry_ptr key);

extern rb_entry_ptr rbtree_min(const RbTreePtr tree);

extern rb_entry_ptr rbtree_max(const RbTreePtr tree);

extern void rbtree_destroy(RbTreePtr tree);

#ifdef __cplusplus
}
#endif

#endif /* __RED_BLACK_TREE_H__ */

