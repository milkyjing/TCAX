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

#include "tccmap.h"


void tcc_map_create(TccMapPtr *pmptr, unsigned long keySize, unsigned long valueSize, unsigned long elements, tcc_map_key_copy_func copy_key, tcc_map_value_copy_func copy_value, tcc_map_key_cleanup_func clean_key, tcc_map_value_cleanup_func clean_value, tcc_map_key_compare_func compare_key, tcc_map_value_compare_func compare_value) {
    TccMapPtr mptr;
    mptr = (TccMapPtr)malloc(sizeof(TccMap));
    mptr->keySize = keySize;
    mptr->valueSize = valueSize;
    mptr->capacity = elements;
    mptr->count = elements;
    mptr->copy_key = copy_key;
    mptr->copy_value = copy_value;
    mptr->clean_key = clean_key;
    mptr->clean_value = clean_value;
    mptr->compare_key = compare_key;
    mptr->compare_value = compare_value;
    mptr->offset = 0;
    if (elements > 0) {
        unsigned long size;
        size = mptr->capacity * mptr->keySize;
        mptr->keyBuf = (tcc_map_key_ptr)malloc(size);
        memset(mptr->keyBuf, 0, size);
        size = mptr->capacity * mptr->valueSize;
        mptr->valueBuf = (tcc_map_value_ptr)malloc(size);
        memset(mptr->valueBuf, 0, size);
    } else {
        mptr->keyBuf = NULL;
        mptr->valueBuf = NULL;
    }
    *pmptr = mptr;
}

unsigned long tcc_map_get_size(const TccMapPtr mptr) {
    return mptr->count;
}

int tcc_map_get_offset(const TccMapPtr mptr) {
    return mptr->offset;
}

void tcc_map_set_offset(TccMapPtr mptr, int offset) {
    mptr->offset = offset;
}

void tcc_map_reset_offset(TccMapPtr mptr) {
    mptr->offset = 0;
}

void tcc_map_increase_offset(TccMapPtr mptr) {
    mptr->offset++;
}

void tcc_map_decrease_offset(TccMapPtr mptr) {
    mptr->offset--;
}

int tcc_map_assign_to_key(TccMapPtr mptr, const tcc_map_key_ptr key, const tcc_map_value_ptr value) {
    unsigned long i;
    int offCount;
    tcc_map_key_ptr keyTemp;
    tcc_map_value_ptr valueTemp;
    offCount = 0;
    if (mptr->compare_key) {
        for (i = 0; i < mptr->count; i++) {
            keyTemp = (unsigned char *)mptr->keyBuf + i * mptr->keySize;
            if (mptr->compare_key(keyTemp, key) == 0) {
                if (offCount == mptr->offset) break;
                else offCount++;
            }
        }
    } else {
        for (i = 0; i < mptr->count; i++) {
            keyTemp = (unsigned char *)mptr->keyBuf + i * mptr->keySize;
            if (memcmp(keyTemp, key, mptr->keySize) == 0) {
                if (offCount == mptr->offset) break;
                else offCount++;
            }
        }
    }
    if (i == mptr->count) return -1;  /* can not find the key */
    valueTemp = (unsigned char *)mptr->valueBuf + i * mptr->valueSize;  /* the value to replace */
    if (mptr->clean_value)
        mptr->clean_value(valueTemp);
    if (mptr->copy_value)
        mptr->copy_value(valueTemp, value);
    else
        memcpy(valueTemp, value, mptr->valueSize);
    return 0;
}

int tcc_map_assign_at(TccMapPtr mptr, unsigned long i, const tcc_map_key_ptr key, const tcc_map_value_ptr value) {
    tcc_map_key_ptr keyTemp;
    tcc_map_value_ptr valueTemp;
    if (i >= mptr->count) return -1;
    keyTemp = (unsigned char *)mptr->keyBuf + i * mptr->keySize;
    if (mptr->clean_key)
        mptr->clean_key(keyTemp);
    if (mptr->copy_key)
        mptr->copy_key(keyTemp, key);
    else
        memcpy(keyTemp, key, mptr->keySize);
    valueTemp = (unsigned char *)mptr->valueBuf + i * mptr->valueSize;  /* the value to replace */
    if (mptr->clean_value)
        mptr->clean_value(valueTemp);
    if (mptr->copy_value)
        mptr->copy_value(valueTemp, value);
    else
        memcpy(valueTemp, value, mptr->valueSize);
    return 0;
}

const tcc_map_value_ptr tcc_map_retrieve(TccMapPtr mptr, const tcc_map_key_ptr key, tcc_map_value_ptr value) {
    unsigned long i;
    int offCount;
    tcc_map_key_ptr keyTemp;
    tcc_map_value_ptr valueTemp;
    offCount = 0;
    if (mptr->compare_key) {
        for (i = 0; i < mptr->count; i++) {
            keyTemp = (unsigned char *)mptr->keyBuf + i * mptr->keySize;
            if (mptr->compare_key(keyTemp, key) == 0) {
                if (offCount == mptr->offset) break;
                else offCount++;
            }
        }
    } else {
        for (i = 0; i < mptr->count; i++) {
            keyTemp = (unsigned char *)mptr->keyBuf + i * mptr->keySize;
            if (memcmp(keyTemp, key, mptr->keySize) == 0) {
                if (offCount == mptr->offset) break;
                else offCount++;
            }
        }
    }
    if (i == mptr->count) return NULL;
    valueTemp = (unsigned char *)mptr->valueBuf + i * mptr->valueSize;
    if (value) {
        if (mptr->copy_value)
            mptr->copy_value(value, valueTemp);
        else
            memcpy(value, valueTemp, mptr->valueSize);
    }
    return valueTemp;
}

void tcc_map_sort(TccMapPtr mptr) {
    unsigned long i, j;
    tcc_map_key_ptr pk1, pk2, pkt;
    tcc_map_value_ptr pv1, pv2, pvt;
    if (!(mptr->compare_key && mptr->compare_value))
        return;
    pkt = (tcc_map_key_ptr)malloc(mptr->keySize);    /* use as temp */
    pvt = (tcc_map_value_ptr)malloc(mptr->valueSize);   /* use as temp */
    for (i = 0; i < mptr->count; i++) {
        for (j = 0; j < mptr->count - i - 1; j++) {
            pk1 = (unsigned char *)mptr->keyBuf + j * mptr->keySize;
            pk2 = (unsigned char *)pk1 + mptr->keySize;
            if (mptr->compare_key(pk1, pk2) > 0) {
                memcpy(pkt, pk1, mptr->keySize);
                memcpy(pk1, pk2, mptr->keySize);
                memcpy(pk2, pkt, mptr->keySize);
                pv1 = (unsigned char *)mptr->valueBuf + j * mptr->valueSize;
                pv2 = (unsigned char *)pv1 + mptr->valueSize;
                memcpy(pvt, pv1, mptr->valueSize);
                memcpy(pv1, pv2, mptr->valueSize);
                memcpy(pv2, pvt, mptr->valueSize);
            }
        }
    }
    free(pkt);
    free(pvt);
}

void tcc_map_push_back(TccMapPtr mptr, const tcc_map_key_ptr key, const tcc_map_value_ptr value) {
    tcc_map_key_ptr keyTemp;
    tcc_map_value_ptr valueTemp;
    if (mptr->count == mptr->capacity) {
        mptr->capacity += mptr->capacity / 2 + 1;
        mptr->keyBuf = (tcc_map_key_ptr)realloc(mptr->keyBuf, mptr->capacity * mptr->keySize);
        mptr->valueBuf = (tcc_map_value_ptr)realloc(mptr->valueBuf, mptr->capacity * mptr->valueSize);
    }
    keyTemp = (unsigned char *)mptr->keyBuf + mptr->count * mptr->keySize;
    if (mptr->copy_key)
        mptr->copy_key(keyTemp, key);
    else
        memcpy(keyTemp, key, mptr->keySize);
    valueTemp = (unsigned char *)mptr->valueBuf + mptr->count * mptr->valueSize;
    if (mptr->copy_value)
        mptr->copy_value(valueTemp, value);
    else
        memcpy(valueTemp, value, mptr->valueSize);
    mptr->count++;
}

int tcc_map_pop_back(TccMapPtr mptr) {
    if (0 == mptr->count) return -1;
    if (mptr->clean_key)
        mptr->clean_key((unsigned char *)mptr->keyBuf + (mptr->count - 1) * mptr->keySize);
    if (mptr->clean_value)
        mptr->clean_value((unsigned char *)mptr->valueBuf + (mptr->count - 1) * mptr->valueSize);
    mptr->count--;
    return 0;
}

void tcc_map_clear(TccMapPtr mptr) {
    unsigned long i;
    if (mptr->clean_key) {
        for (i = 0; i < mptr->count; i++)
            mptr->clean_key((unsigned char *)mptr->keyBuf + i * mptr->keySize);  /* the parameter passed to clean_key is the address of the ith element */
    }
    free(mptr->keyBuf);
    mptr->keyBuf = NULL;
    if (mptr->clean_value) {
        for (i = 0; i < mptr->count; i++)
            mptr->clean_value((unsigned char *)mptr->valueBuf + i * mptr->valueSize);  /* the parameter passed to clean_value is the address of the ith element */
    }
    free(mptr->valueBuf);
    mptr->valueBuf = NULL;
    mptr->capacity = 0;
    mptr->count = 0;
    mptr->offset = 0;
    /* other data members should stay unchanged */
}

void tcc_map_destroy(TccMapPtr mptr) {
    tcc_map_clear(mptr);
    free(mptr);
}

