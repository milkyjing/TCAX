/*
 * Copyright (C) 2011 milkyjing <milkyjing@gmail.com>
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

#ifndef TCCMAP_H
#define TCCMAP_H
#pragma once

#include <memory.h>
#include <stdlib.h>

typedef void *tcc_map_key_ptr;      /**< key entry */
typedef void *tcc_map_value_ptr;    /**< value entry */

/* copy functions */
typedef void (*tcc_map_key_copy_func)(tcc_map_key_ptr, const tcc_map_key_ptr);
typedef void (*tcc_map_value_copy_func)(tcc_map_value_ptr, const tcc_map_value_ptr);

/* cleanup functions */
typedef void (*tcc_map_key_cleanup_func)(tcc_map_key_ptr);
typedef void (*tcc_map_value_cleanup_func)(tcc_map_value_ptr);

/* compare functions */
typedef int (*tcc_map_key_compare_func)(const tcc_map_key_ptr, const tcc_map_key_ptr);
typedef int (*tcc_map_value_compare_func)(const tcc_map_value_ptr, const tcc_map_value_ptr);

/* one key may have multiple values, specified by offset */
typedef struct _tcc_map {
    tcc_map_key_ptr keyBuf;
    tcc_map_value_ptr valueBuf;
    unsigned long keySize;
    unsigned long valueSize;
    unsigned long capacity;   /**< can be enlarged dynamically */
    unsigned long count;  /**< number of elements (values) */
    tcc_map_key_copy_func copy_key;
    tcc_map_value_copy_func copy_value;
    tcc_map_key_cleanup_func clean_key;
    tcc_map_value_cleanup_func clean_value;
    tcc_map_key_compare_func compare_key;
    tcc_map_value_compare_func compare_value;
    int offset;    /**< a key may be assigned with multiple values, so offset is used to specify which value to retrieve */
} TccMap, *TccMapPtr;


#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

extern void tcc_map_create(TccMapPtr *pmptr, unsigned long keySize, unsigned long valueSize, unsigned long elements, tcc_map_key_copy_func copy_key, tcc_map_value_copy_func copy_value, tcc_map_key_cleanup_func clean_key, tcc_map_value_cleanup_func clean_value, tcc_map_key_compare_func compare_key, tcc_map_value_compare_func compare_value);

extern unsigned long tcc_map_get_size(const TccMapPtr mptr);

extern int tcc_map_get_offset(const TccMapPtr mptr);

extern void tcc_map_set_offset(TccMapPtr mptr, int offset);

extern void tcc_map_reset_offset(TccMapPtr mptr);

extern void tcc_map_increase_offset(TccMapPtr mptr);

extern void tcc_map_decrease_offset(TccMapPtr mptr);

/* assign the key (first one) with a new value, the key must be existed */
extern int tcc_map_assign_to_key(TccMapPtr mptr, const tcc_map_key_ptr key, const tcc_map_value_ptr value);

/* replace the key and value at position i */
extern int tcc_map_assign_at(TccMapPtr mptr, unsigned long i, const tcc_map_key_ptr key, const tcc_map_value_ptr value);

/* you can retrieve n times to get all the values of a duplicate key, offset will automatically increase, value can be NULL */
extern const tcc_map_value_ptr tcc_map_retrieve(TccMapPtr mptr, const tcc_map_key_ptr key, tcc_map_value_ptr value);

/* Sort the pairs by keys, this is to improve the performance of the lookup process, compare function must be specified, not complete yet */
extern void tcc_map_sort(TccMapPtr mptr);

extern void tcc_map_push_back(TccMapPtr mptr, const tcc_map_key_ptr key, const tcc_map_value_ptr value);

extern int tcc_map_pop_back(TccMapPtr mptr);

/* will only clear the elements in the map but will not free the memory occupied by the map itself */
extern void tcc_map_clear(TccMapPtr mptr);

extern void tcc_map_destroy(TccMapPtr mptr);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* TCCMAP_H */
