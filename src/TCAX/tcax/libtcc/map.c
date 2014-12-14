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

#include "map.h"


void map_create(MapPtr *pmptr, unsigned long keySize, unsigned long valueSize, unsigned long elements, map_key_copy_func copy_key, map_value_copy_func copy_value, map_key_cleanup_func clean_key, map_value_cleanup_func clean_value, map_key_compare_func compare_key, map_value_compare_func compare_value) {
    MapPtr mptr;
    mptr = (MapPtr)malloc(sizeof(Map));
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
        mptr->keyBuf = (map_key_ptr)malloc(size);
        memset(mptr->keyBuf, 0, size);
        size = mptr->capacity * mptr->valueSize;
        mptr->valueBuf = (map_value_ptr)malloc(size);
        memset(mptr->valueBuf, 0, size);
    } else {
        mptr->keyBuf = NULL;
        mptr->valueBuf = NULL;
    }
    *pmptr = mptr;
}

unsigned long map_get_size(const MapPtr mptr) {
    return mptr->count;
}

int map_get_offset(const MapPtr mptr) {
    return mptr->offset;
}

void map_set_offset(MapPtr mptr, int offset) {
    mptr->offset = offset;
}

void map_reset_offset(MapPtr mptr) {
    mptr->offset = 0;
}

void map_increase_offset(MapPtr mptr) {
    mptr->offset++;
}

int map_assign_to_key(MapPtr mptr, const map_key_ptr key, const map_value_ptr value) {
    unsigned long i;
    int offCount;
    map_key_ptr keyTemp;
    map_value_ptr valueTemp;
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

int map_assign_at(MapPtr mptr, unsigned long i, const map_key_ptr key, const map_value_ptr value) {
    map_key_ptr keyTemp;
    map_value_ptr valueTemp;
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

const map_value_ptr map_retrieve(MapPtr mptr, const map_key_ptr key, map_value_ptr value) {
    unsigned long i;
    int offCount;
    map_key_ptr keyTemp;
    map_value_ptr valueTemp;
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

void map_push_back(MapPtr mptr, const map_key_ptr key, const map_value_ptr value) {
    map_key_ptr keyTemp;
    map_value_ptr valueTemp;
    if (mptr->count == mptr->capacity) {
        mptr->capacity += mptr->capacity / 2 + 1;
        mptr->keyBuf = (map_key_ptr)realloc(mptr->keyBuf, mptr->capacity * mptr->keySize);
        mptr->valueBuf = (map_value_ptr)realloc(mptr->valueBuf, mptr->capacity * mptr->valueSize);
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

int map_pop_back(MapPtr mptr) {
    if (0 == mptr->count) return -1;
    if (mptr->clean_key)
        mptr->clean_key((unsigned char *)mptr->keyBuf + (mptr->count - 1) * mptr->keySize);
    if (mptr->clean_value)
        mptr->clean_value((unsigned char *)mptr->valueBuf + (mptr->count - 1) * mptr->valueSize);
    mptr->count--;
    return 0;
}

void map_clear(MapPtr mptr) {
    unsigned long i;
    if (mptr->clean_key) {
        for (i = 0; i < mptr->count; i++)
            mptr->clean_key((unsigned char *)mptr->keyBuf + i * mptr->keySize);  /* the parameter passed to clean_f is the address of the ith element */
    }
    free(mptr->keyBuf);
    mptr->keyBuf = NULL;
    if (mptr->clean_value) {
        for (i = 0; i < mptr->count; i++)
            mptr->clean_value((unsigned char *)mptr->valueBuf + i * mptr->valueSize);  /* the parameter passed to clean_f is the address of the ith element */
    }
    free(mptr->valueBuf);
    mptr->valueBuf = NULL;
    mptr->capacity = 0;
    mptr->count = 0;
    mptr->offset = 0;
    /* other data members should stay unchanged */
}

void map_destroy(MapPtr mptr) {
    map_clear(mptr);
    free(mptr);
}

