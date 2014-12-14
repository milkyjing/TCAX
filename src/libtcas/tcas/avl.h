/*
 * AVL Tree C Implementation, borrowed the mind of libavl
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

#ifndef __AVL_TREE_H__
#define __AVL_TREE_H__
#pragma once

#include <stddef.h>


#define AVL_MAX_HEIGHT 32       /* large enough, since the AVL tree is well balanced */

typedef void *avl_entry_ptr;        /* type of the data stored in the node */

typedef struct _avl_node {
    struct _avl_node *link[2];  /* to avoid the symmetric code, and should be placed as the first data member for convinent memory access reason */
    avl_entry_ptr data;
    signed char balance;
} AvlNode, *AvlNodePtr;

typedef int (*avl_compare_func)(const avl_entry_ptr, const avl_entry_ptr, const void *);     /* this function is very important, and should be specifed */
typedef avl_entry_ptr (*avl_copy_func)(const avl_entry_ptr, const void *);
typedef void (*avl_cleanup_func)(avl_entry_ptr, const void *);    /* the parameter passed to clean_f is the address of the ith element */

typedef struct _avl_tree {
    AvlNodePtr root;          /* Tree's root. */
    avl_compare_func compare_f;   /* Comparison function. */
    void *param;            /* extra param for comparison */
    int count;                   /* Number of items in tree. */
    avl_copy_func copy_f;            /* copy function, can be NULL */
    avl_cleanup_func clean_f;        /* cleanup function, can be NULL */
} AvlTree, *AvlTreePtr;

#define avltree_count(tree) ((AvlTreePtr)(tree)->count)


#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

extern AvlTreePtr avltree_create(avl_compare_func compare_f, avl_copy_func copy_f, avl_cleanup_func clean_f, void *param);

extern avl_entry_ptr avltree_find(const AvlTreePtr tree, const avl_entry_ptr key);

extern avl_entry_ptr *avltree_probe(AvlTreePtr tree, const avl_entry_ptr item);

extern int avltree_insert(AvlTreePtr tree, const avl_entry_ptr item);

extern int avltree_delete(AvlTreePtr tree, const avl_entry_ptr key);

extern void avltree_destroy(AvlTreePtr tree);

#ifdef __cplusplus
}
#endif

#endif /* __AVL_TREE_H__ */

