/*
 * Dynamic Allocated Array-Based Circular Queue C Implementation
 * For The libtcas Project
 * Copyright (C) 2011 milkyjing <milkyjing@gmail.com>
 * October 7th, 2011
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

#ifndef QUEUE_H
#define QUEUE_H
#pragma once

#include <memory.h>
#include <stdlib.h>

typedef void *queue_entry_ptr;

typedef void (*queue_copy_func)(queue_entry_ptr, const queue_entry_ptr);
typedef void (*queue_cleanup_func)(queue_entry_ptr);    /* the parameter passed to clean_f is the address of the ith element */

typedef struct _queue {
    queue_entry_ptr datap;
    unsigned long elementSize;
    int head_position;    /* rear_position can be calculated by head_position + count */
    unsigned long capacity;    /* maximun elements that the queue can hold */
    unsigned long count;    /* elements */
    queue_copy_func copy_f;
    queue_cleanup_func clean_f;
} Queue, *QueuePtr;

typedef void (*queue_traverse_func)(const queue_entry_ptr);

#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

extern void queue_create(QueuePtr *pqptr, unsigned long elementSize, unsigned long elements, queue_copy_func copy_f, queue_cleanup_func clean_f);

extern unsigned long queue_get_count(const QueuePtr qptr);

extern const queue_entry_ptr queue_get_buf(const QueuePtr qptr);

/* element can be NULL, if so, we will not make a copy to element. the return value is the address of the element retrieved */
extern const queue_entry_ptr queue_retrieve_front(const QueuePtr qptr, queue_entry_ptr element);

extern int queue_push_rear(QueuePtr qptr, const queue_entry_ptr element);

extern int queue_pop_front(QueuePtr qptr, queue_entry_ptr element);

extern void queue_clear(QueuePtr qptr);

extern void queue_destroy(QueuePtr qptr);

/* UTILITY FUNCTIONS */

extern void queue_traverse(const QueuePtr qptr, queue_traverse_func traverse_func);

#ifdef __cplusplus
}
#endif

#endif    /* QUEUE_H */

