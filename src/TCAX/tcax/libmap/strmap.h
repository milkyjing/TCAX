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

#ifndef STRMAP_H
#define STRMAP_H
#pragma once

#include "tccmap.h"

#include <stdio.h>
#include <wchar.h>

#if defined WIN32
#include <windows.h>
#else
#include <locale.h>
#endif  /* WIN32 */



#define SCAN_STAT_INIT 0            /**< before the '<' mark */
#define SCAN_STAT_BEFORE_KEY 1      /**< between the '<' mark and the very first non-space character */
#define SCAN_STAT_KEY 2             /**< parsing the key */
#define SCAN_STAT_AFTER_KEY 3       /**< between the last non-space character and the '=' mark */
#define SCAN_STAT_BEFORE_VALUE 4    /**< between the '=' mark and the very first non-space character */
#define SCAN_STAT_VALUE 5           /**< parsing the value */
#define SCAN_STAT_AFTER_VALUE 6     /**< between the last non-space character and the '>' mark */
#define SCAN_STAT_FIN 7             /**< after the '>' mark */

#define QUOTE_FLAG_NONE 0           /**< no quote mark */
#define QUOTE_FLAG_QUOTED 1         /**< the content is quoted */
#define QUOTE_FLAG_ENCLOSED 0       /**< end of the quoted content */


typedef struct _str_map {
    wchar_t *buffer;    /**< buffer of the file, in wide char mode */
    int count;          /**< characters of buffer including the NULL terminator, and maybe the BOM */
    TccMapPtr map;      /**< the tcc map */
} StrMap, *StrMapPtr;


/* Inhibit C++ name-mangling for libmap functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * Read a map (plain text) file and convert to a utf-8 string.
 * @param filename the filename of the target map file
 * @param mptr a pointer to StrMap struct
 * @return 0 on success, -1 on failure
 */
extern int libmap_read_map_file(const char *filename, StrMapPtr mptr);

/**
 * Parse the buffer to map
 * @param mptr a pointer to StrMap struct
 */
extern void libmap_parse(StrMapPtr mptr);

/**
 * Polish the keys and values of the map, merge white spaces into a single space, reduce the "" to "
 * @param mptr a pointer to StrMap struct
 */
extern void libmap_polish(StrMapPtr mptr);

/**
 * Destroy the map
 * @param mptr a pointer to StrMap struct
 */
extern void libmap_destroy(StrMapPtr mptr);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* STRMAP_H */

