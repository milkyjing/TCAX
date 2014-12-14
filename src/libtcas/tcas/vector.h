/*
 * Dynamic Allocated Array-Based Vector C Implementation
 * For The Teco Project
 * Copyright (C) 2010 milkyjing <milkyjing@gmail.com>
 * Auguest 11th, 2010
 *
 * Modified for The RIXE Project at June 14th, 2011 by milkyjing
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

#ifndef VECTOR_H
#define VECTOR_H
#pragma once

#include <memory.h>
#include <stdlib.h>

typedef void *vector_entry_ptr;

typedef void (*vector_copy_func)(vector_entry_ptr, const vector_entry_ptr);
typedef void (*vector_cleanup_func)(vector_entry_ptr);    /* the parameter passed to clean_f is the address of the ith element */

typedef struct _vector {
    vector_entry_ptr datap;
    unsigned long elementSize;
    unsigned long capacity;
    unsigned long count;
    vector_copy_func copy_f;
    vector_cleanup_func clean_f;
} Vector, *VectorPtr;

typedef int (*vector_compare_func)(const vector_entry_ptr, const vector_entry_ptr);  /* (src, key), `> 0' - left larger, `= 0' equal, `< 0', left smaller */
typedef int (*vector_compare_interval_func)(const vector_entry_ptr, const vector_entry_ptr, const vector_entry_ptr);  /* (start, end, key), the last parameter is key, `> 0' - key on the left, `= 0' in the interval, `< 0', key on the right */
typedef void (*vector_traverse_func)(const vector_entry_ptr);

#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

extern void vector_create(VectorPtr *pvptr, unsigned long elementSize, unsigned long elements, vector_copy_func copy_f, vector_cleanup_func clean_f);

extern unsigned long vector_get_count(const VectorPtr vptr);

extern const vector_entry_ptr vector_get_buf(const VectorPtr vptr);

extern int vector_assign(VectorPtr vptr, unsigned long i, const vector_entry_ptr element);

/* element can be NULL, if so, we will not make a copy to element. the return value is the address of the element retrieved */
extern const vector_entry_ptr vector_retrieve(const VectorPtr vptr, unsigned long i, vector_entry_ptr element);

extern void vector_push_back(VectorPtr vptr, const vector_entry_ptr element);

extern int vector_pop_back(VectorPtr vptr);

extern void vector_clear(VectorPtr vptr);

extern void vector_destroy(VectorPtr vptr);

/* UTILITY FUNCTIONS */

extern void vector_traverse(const VectorPtr vptr, vector_traverse_func traverse_func);

extern void vector_bulbsort(VectorPtr vptr, vector_compare_func compare_func);

/* s should always be 0, and l should always be vptr->count - 1, note that both s and l should be signed rather than unsigned */
extern void vector_quicksort(VectorPtr vptr, long s, long l, vector_compare_func compare_func);

/* if element = NULL, it won't copy the result to it, just return the index, which somehow increase performance */
extern long vector_sequential_search(const VectorPtr vptr, const vector_entry_ptr key, vector_compare_func compare_func, vector_entry_ptr element);

/* if element = NULL, it won't copy the result to it, just return the index, which somehow increase performance */
extern long vector_binary_search(const VectorPtr vptr, const vector_entry_ptr key, vector_compare_func compare_func, vector_entry_ptr element);

/* returns the left side element (start) */
extern long vector_binary_search_interval(const VectorPtr vptr, const vector_entry_ptr key, vector_compare_interval_func compare_interval_func);

#ifdef __cplusplus
}
#endif

#endif    /* VECTOR_H */

