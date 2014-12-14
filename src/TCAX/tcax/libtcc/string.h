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

#ifndef STRING_H
#define STRING_H
#pragma once

#include <memory.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif    /* _MSC_VER */

/* NULL terminated string */
typedef struct _string {
    wchar_t *buffer;
    unsigned long capacity;
    unsigned long count;    /* do not include the null terminator, and max count = capacity - 1 */
} String;

#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

extern void string_init(String *s, unsigned long capacity);

extern unsigned long string_get_size(const String *s);

extern const wchar_t *string_get_buffer(const String *s);

extern void string_assign(String *s, const wchar_t *wcs, unsigned long count);

extern void string_assign_from_int(String *s, int i);

extern void string_assign_from_double(String *s, double f);

extern void string_append(String *s, const wchar_t *wcs, unsigned long count);

extern void string_append_int(String *s, int i);

extern void string_append_double(String *s, double f);

extern void string_push_back(String *s, wchar_t w);

extern void string_reverse(String *s);

extern void string_clear(String *s);

extern void string_copy(String *dst, const String *src);

extern void string_copy_to_buffer(const String *s, wchar_t **pBuf);

extern void string_to_lower_case(String *s);

extern void string_to_upper_case(String *s);

extern int string_ignore_case_compare(const String *s, const wchar_t *wcs, unsigned long count);

/* merge all white spaces to a single half white space between two non white space characters in the string, here white space includes half or full white space and tab */
extern void string_merge_all_white_spaces(String *s);

/* discard begin and end white spaces in the string, here white space includes half or full white space and tab */
extern void string_discard_begin_end_white_spaces_and_quote(String *s);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* STRING_H */
