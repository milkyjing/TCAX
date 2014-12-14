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

#include "vector.h"


void vector_create(VectorPtr *pvptr, unsigned long elementSize, unsigned long elements, vector_copy_func copy_f, vector_cleanup_func clean_f) {
    VectorPtr vptr;
    vptr = (VectorPtr)malloc(sizeof(Vector));
    vptr->elementSize = elementSize;
    vptr->capacity = elements;
    vptr->count = elements;
    vptr->copy_f = copy_f;    /* if copy_f = NULL, default copy function will be applied */
    vptr->clean_f = clean_f;  /* if clean_f = NULL, default cleanup function will be applied */
    if (elements > 0) {
        unsigned long size = vptr->capacity * vptr->elementSize;
        vptr->datap = (vector_entry_ptr)malloc(size);
        memset(vptr->datap, 0, size);
    } else
        vptr->datap = NULL;
    *pvptr = vptr;
}

unsigned long vector_get_count(const VectorPtr vptr) {
    return vptr->count;
}

const vector_entry_ptr vector_get_buf(const VectorPtr vptr) {
    return (const vector_entry_ptr)vptr->datap;
}

int vector_assign(VectorPtr vptr, unsigned long i, const vector_entry_ptr element) {
    vector_entry_ptr oldElem;
    if (i >= vptr->count) return -1;
    oldElem = (unsigned char *)vptr->datap + i * vptr->elementSize;
    if (vptr->clean_f)
        vptr->clean_f(oldElem);
    if (vptr->copy_f)
        vptr->copy_f(oldElem, element);
    else
        memcpy(oldElem, element, vptr->elementSize);
    return 0;
}

const vector_entry_ptr vector_retrieve(const VectorPtr vptr, unsigned long i, vector_entry_ptr element) {
    vector_entry_ptr oldElem;
    if (i >= vptr->count) return NULL;
    oldElem = (unsigned char *)vptr->datap + i * vptr->elementSize;
    if (element) {  /* make a copy */
        if (vptr->copy_f)
            vptr->copy_f(element, oldElem);
        else
            memcpy(element, oldElem, vptr->elementSize);
    }
    return (const vector_entry_ptr)oldElem;
}

void vector_push_back(VectorPtr vptr, const vector_entry_ptr element) {
    vector_entry_ptr oldElem;
    if (vptr->count == vptr->capacity) {
        vptr->capacity += vptr->capacity / 2 + 1;
        vptr->datap = (vector_entry_ptr)realloc(vptr->datap, vptr->capacity * vptr->elementSize);
    }
    oldElem = (unsigned char *)vptr->datap + vptr->count * vptr->elementSize;
    if (vptr->copy_f)
        vptr->copy_f(oldElem, element);
    else
        memcpy(oldElem, element, vptr->elementSize);
    vptr->count++;
}

int vector_pop_back(VectorPtr vptr) {
    vector_entry_ptr oldElem;
    if (0 == vptr->count) return -1;
    oldElem = (unsigned char *)vptr->datap + (vptr->count - 1) * vptr->elementSize;
    if (vptr->clean_f)
        vptr->clean_f(oldElem);
    vptr->count--;
    return 0;
}

void vector_clear(VectorPtr vptr) {
    if (vptr->clean_f) {
        unsigned long i;
        for (i = 0; i < vptr->count; i++)
            vptr->clean_f((unsigned char *)vptr->datap + i * vptr->elementSize);  /* the parameter passed to clean_f is the address of the ith element */
    }
    free(vptr->datap);
    vptr->datap = NULL;
    vptr->capacity = 0;
    vptr->count = 0;
    /* other data members should stay unchanged */
}

void vector_destroy(VectorPtr vptr) {
    vector_clear(vptr);
    free(vptr);
}

/* UTILITY FUNCTIONS */

void vector_traverse(const VectorPtr vptr, vector_traverse_func traverse_func) {
    unsigned long i;
    for (i = 0; i < vptr->count; i++)
        traverse_func((unsigned char *)vptr->datap + i * vptr->elementSize);
}

void vector_bulbsort(VectorPtr vptr, vector_compare_func compare_func) {
    unsigned long i, j;
    vector_entry_ptr p1, p2, pt;
    pt = (vector_entry_ptr)malloc(vptr->elementSize);    /* use as temp */
    for (i = 0; i < vptr->count; i++) {
        for (j = 0; j < vptr->count - i - 1; j++) {
            p1 = (unsigned char *)vptr->datap + j * vptr->elementSize;
            p2 = (unsigned char *)p1 + vptr->elementSize;
            if (compare_func(p1, p2) > 0) {
                memcpy(pt, p1, vptr->elementSize);
                memcpy(p1, p2, vptr->elementSize);
                memcpy(p2, pt, vptr->elementSize);
            }
        }
    }
    free(pt);
}

/* s should always be 0, and l should always be vptr->count - 1, note that both s and l should be signed rather than unsigned */
void vector_quicksort(VectorPtr vptr, long s, long l, vector_compare_func compare_func) {
    long i, j;
    vector_entry_ptr pt;
    i = s;
    j = l;
    pt = (vector_entry_ptr)malloc(vptr->elementSize);    /* use as temp */
    if (s < l) {
        memcpy(pt, (unsigned char *)vptr->datap + s * vptr->elementSize, vptr->elementSize);
        while (i != j) {
            while (j > i && compare_func((unsigned char *)vptr->datap + j * vptr->elementSize, pt) > 0) j--;
            memcpy((unsigned char *)vptr->datap + i * vptr->elementSize, (unsigned char *)vptr->datap + j * vptr->elementSize, vptr->elementSize);
            while (i < j && compare_func((unsigned char *)vptr->datap + i * vptr->elementSize, pt) < 0) i++;
            memcpy((unsigned char *)vptr->datap + j * vptr->elementSize, (unsigned char *)vptr->datap + i * vptr->elementSize, vptr->elementSize);
        }
        memcpy((unsigned char *)vptr->datap + i * vptr->elementSize, pt, vptr->elementSize);
        vector_quicksort(vptr, s, i - 1, compare_func);
        vector_quicksort(vptr, i + 1, l, compare_func);
    }
    free(pt);
}

long vector_sequential_search(const VectorPtr vptr, const vector_entry_ptr key, vector_compare_func compare_func, vector_entry_ptr element) {
    unsigned long i;
    vector_entry_ptr temp;
    for (i = 0; i < vptr->count; i++) {
        temp = (unsigned char *)vptr->datap + i * vptr->elementSize;
        if (compare_func(key, temp) == 0) {
            if (element) {
                if (vptr->copy_f)
                    vptr->copy_f(element, temp);
                else
                    memcpy(element, temp, vptr->elementSize);
            }
            return i;
        }
    }
    return -1;
}

long vector_binary_search(const VectorPtr vptr, const vector_entry_ptr key, vector_compare_func compare_func, vector_entry_ptr element) {
    long bottom, top, mid;
    vector_entry_ptr temp;
    bottom = 0;
    top = vptr->count - 1;
    while (bottom < top) {
        mid = (top + bottom) / 2;
        temp = (unsigned char *)vptr->datap + mid * vptr->elementSize;
        if (compare_func(temp, key) < 0)
            bottom = mid + 1;
        else
            top = mid;
    }
    temp = (unsigned char *)vptr->datap + top * vptr->elementSize;
    if (bottom > top || compare_func(temp, key) != 0) return -1;
    if (element) {
        if (vptr->copy_f)
            vptr->copy_f(element, temp);
        else
            memcpy(element, temp, vptr->elementSize);
    }
    return top;
}

long vector_binary_search_interval(const VectorPtr vptr, const vector_entry_ptr key, vector_compare_interval_func compare_interval_func) {
    long bottom, top, mid;
    vector_entry_ptr start, end;
    bottom = 0;
    top = vptr->count - 2;
    while (bottom < top) {
        mid = (top + bottom) / 2;
        start = (unsigned char *)vptr->datap + mid * vptr->elementSize;
        end = (unsigned char *)start + vptr->elementSize;
        if (compare_interval_func(start, end, key) < 0)
            bottom = mid + 1;
        else
            top = mid;
    }
    start = (unsigned char *)vptr->datap + top * vptr->elementSize;
    end = (unsigned char *)start + vptr->elementSize;
    if (bottom > top || compare_interval_func(start, end, key) != 0) return -1;
    return top;
}


