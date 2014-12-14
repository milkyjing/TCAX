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

#include "queue.h"


void queue_create(QueuePtr *pqptr, unsigned long elementSize, unsigned long elements, queue_copy_func copy_f, queue_cleanup_func clean_f) {
    QueuePtr qptr;
    qptr = (QueuePtr)malloc(sizeof(Queue));
    qptr->elementSize = elementSize;
    qptr->head_position = 0;
    qptr->capacity = (elements < 1) ? 1 : elements;
    qptr->count = 0;
    qptr->copy_f = copy_f;    /* if copy_f = NULL, default copy function will be applied */
    qptr->clean_f = clean_f;  /* if clean_f = NULL, default cleanup function will be applied */
    qptr->datap = (queue_entry_ptr)malloc(qptr->capacity * qptr->elementSize);
    //memset(qptr->datap, 0, qptr->capacity * qptr->elementSize);
    *pqptr = qptr;
}

unsigned long queue_get_count(const QueuePtr qptr) {
    return qptr->count;
}

const queue_entry_ptr queue_get_buf(const QueuePtr qptr) {
    return (const queue_entry_ptr)qptr->datap;
}

const queue_entry_ptr queue_retrieve_front(const QueuePtr qptr, queue_entry_ptr element) {
    queue_entry_ptr oldElem;
    if (0 == qptr->count) return NULL;
    oldElem = (unsigned char *)qptr->datap + qptr->head_position * qptr->elementSize;
    if (element) {  /* make a copy */
        if (qptr->copy_f)
            qptr->copy_f(element, oldElem);
        else
            memcpy(element, oldElem, qptr->elementSize);
    }
    return (const queue_entry_ptr)oldElem;
}

int queue_push_rear(QueuePtr qptr, const queue_entry_ptr element) {
    unsigned long rear_position;
    queue_entry_ptr oldElem;
    if (qptr->count == qptr->capacity) return -1;    /* queue is full */
    rear_position = (qptr->head_position + qptr->count) % qptr->capacity;
    oldElem = (unsigned char *)qptr->datap + rear_position * qptr->elementSize;
    if (qptr->copy_f)
        qptr->copy_f(oldElem, element);
    else
        memcpy(oldElem, element, qptr->elementSize);
    qptr->count++;
    return 0;
}

int queue_pop_front(QueuePtr qptr, queue_entry_ptr element) {
    queue_entry_ptr oldElem;
    if (0 == qptr->count) return -1;    /* queue is empty */
    oldElem = (unsigned char *)qptr->datap + qptr->head_position * qptr->elementSize;
    if (element) {  /* make a copy */
        if (qptr->copy_f)
            qptr->copy_f(element, oldElem);
        else
            memcpy(element, oldElem, qptr->elementSize);
    }
    if (qptr->clean_f)
        qptr->clean_f(oldElem);
    qptr->head_position++;
    if (qptr->head_position == qptr->capacity)
        qptr->head_position = 0;
    qptr->count--;
    return 0;
}

void queue_clear(QueuePtr qptr) {
    if (qptr->clean_f) {
        unsigned long i, index;
        for (i = 0; i < qptr->count; i++) {
            index = (qptr->head_position + i) % qptr->capacity;
            qptr->clean_f((unsigned char *)qptr->datap + index * qptr->elementSize);  /* the parameter passed to clean_f is the address of the ith element */
        }
    }
    /*free(qptr->datap);     should not be freed */
    qptr->datap = NULL;
    qptr->head_position = 0;
    qptr->count = 0;
    /* other data members should stay unchanged */
}

void queue_destroy(QueuePtr qptr) {
    queue_clear(qptr);
    free(qptr->datap);
    free(qptr);
}

/* UTILITY FUNCTIONS */

void queue_traverse(const QueuePtr qptr, queue_traverse_func traverse_func) {
    unsigned long i, index;
    for (i = 0; i < qptr->count; i++) {
        index = (qptr->head_position + i) % qptr->capacity;
        traverse_func((unsigned char *)qptr->datap + index * qptr->elementSize);
    }
}

