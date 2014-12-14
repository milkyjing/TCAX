/* 
 *  Copyright (C) 2011 milkyjing <milkyjing@gmail.com>
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifndef MAP_H
#define MAP_H
#pragma once

#include <memory.h>
#include <stdlib.h>

typedef void *map_key_ptr;
typedef void *map_value_ptr;

typedef void (*map_key_copy_func)(map_key_ptr, const map_key_ptr);
typedef void (*map_value_copy_func)(map_value_ptr, const map_value_ptr);
typedef void (*map_key_cleanup_func)(map_key_ptr);
typedef void (*map_value_cleanup_func)(map_value_ptr);
typedef int (*map_key_compare_func)(const map_key_ptr, const map_key_ptr);
typedef int (*map_value_compare_func)(const map_value_ptr, const map_value_ptr);

/* one key may have multiple values, specified by offset */
typedef struct _map {
    map_key_ptr keyBuf;
    map_value_ptr valueBuf;
    unsigned long keySize;
    unsigned long valueSize;
    unsigned long capacity;
    unsigned long count;  /* number of elements (values) */
    map_key_copy_func copy_key;
    map_value_copy_func copy_value;
    map_key_cleanup_func clean_key;
    map_value_cleanup_func clean_value;
    map_key_compare_func compare_key;
    map_value_compare_func compare_value;
    int offset;    /* a key may be assigned with multiple values, so offset is used to specify which value to retrieve */
} Map, *MapPtr;

#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

extern void map_create(MapPtr *pmptr, unsigned long keySize, unsigned long valueSize, unsigned long elements, map_key_copy_func copy_key, map_value_copy_func copy_value, map_key_cleanup_func clean_key, map_value_cleanup_func clean_value, map_key_compare_func compare_key, map_value_compare_func compare_value);

extern unsigned long map_get_size(const MapPtr mptr);

extern int map_get_offset(const MapPtr mptr);

extern void map_set_offset(MapPtr mptr, int offset);

extern void map_reset_offset(MapPtr mptr);

extern void map_increase_offset(MapPtr mptr);

/* assign the key (first one) with a new value, the key must be existed */
extern int map_assign_to_key(MapPtr mptr, const map_key_ptr key, const map_value_ptr value);

/* replace the key and value at position i */
extern int map_assign_at(MapPtr mptr, unsigned long i, const map_key_ptr key, const map_value_ptr value);

/* you can retrieve n times to get all the values of a duplicate key, offset will automatically increase, value can be NULL */
extern const map_value_ptr map_retrieve(MapPtr mptr, const map_key_ptr key, map_value_ptr value);

extern void map_push_back(MapPtr mptr, const map_key_ptr key, const map_value_ptr value);

extern int map_pop_back(MapPtr mptr);

extern void map_clear(MapPtr mptr);

extern void map_destroy(MapPtr mptr);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* MAP_H */
