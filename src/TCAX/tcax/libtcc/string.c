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

#include "string.h"


void string_init(String *s, unsigned long capacity) {
    s->capacity = capacity + 1;  /* plus one intend to hold the NULL terminator */
    s->count = 0;
    if (capacity > 0) {
        s->buffer = (wchar_t *)malloc(s->capacity * sizeof(wchar_t));
        memset(s->buffer, 0, s->capacity * sizeof(wchar_t));
    } else
        s->buffer = NULL;
}

unsigned long string_get_size(const String *s) {
    return s->count;
}

const wchar_t *string_get_buffer(const String *s) {
    return (const wchar_t *)s->buffer;
}

void string_assign(String *s, const wchar_t *wcs, unsigned long count) {
    unsigned long prevCapacity;
    prevCapacity = s->capacity;
    s->count = count;
    while (s->count + 1 > s->capacity)
        s->capacity += s->capacity / 2 + 1;
    if (s->capacity != prevCapacity) {
        free(s->buffer);
        s->buffer = (wchar_t *)malloc(s->capacity * sizeof(wchar_t));
    }
    memcpy(s->buffer, wcs, s->count * sizeof(wchar_t));
    memset(s->buffer + s->count, 0, (s->capacity - s->count) * sizeof(wchar_t));
}

void string_assign_from_int(String *s, int i) {
    unsigned long len;
    wchar_t buf[34];
    len = swprintf(buf, 33, L"%i", i);
    string_assign(s, buf, len);
}

void string_assign_from_double(String *s, double f) {
    int len;
    wchar_t buf[66];
    len = swprintf(buf, 65, L"%.3f", f);
    string_assign(s, buf, len);
}

void string_append(String *s, const wchar_t *wcs, unsigned long count) {
    unsigned long prevCount, prevCapacity;
    prevCount = s->count;
    prevCapacity = s->capacity;
    s->count += count;
    while (s->count + 1 > s->capacity)
        s->capacity += s->capacity / 2 + 1;
    if (s->capacity != prevCapacity) {
        s->buffer = (wchar_t *)realloc(s->buffer, s->capacity * sizeof(wchar_t));
        memset(s->buffer + prevCapacity, 0, (s->capacity - prevCapacity) * sizeof(wchar_t));
    }
    memcpy(s->buffer + prevCount, wcs, count * sizeof(wchar_t));
}

void string_append_int(String *s, int i) {
    unsigned long len;
    wchar_t buf[34];
    len = swprintf(buf, 33, L"%i", i);
    string_append(s, buf, len);
}

void string_append_double(String *s, double f) {
    int len;
    wchar_t buf[66];
    len = swprintf(buf, 65, L"%.3f", f);
    string_append(s, buf, len);
}

void string_push_back(String *s, wchar_t w) {
    unsigned long prevCapacity;
    prevCapacity = s->capacity;
    if (s->count + 2 > s->capacity) {
        s->capacity += s->capacity / 2 + 1;
        s->buffer = (wchar_t *)realloc(s->buffer, s->capacity * sizeof(wchar_t));
        memset(s->buffer + prevCapacity, 0, (s->capacity - prevCapacity) * sizeof(wchar_t));
    }
    s->buffer[s->count++] = w;
}

void string_reverse(String *s) {
    unsigned long i;
    wchar_t wc;
    for (i = 0; i < s->count; i++) {
        wc = s->buffer[i];
        s->buffer[i] = s->buffer[s->count - i - 1];
        s->buffer[s->count - i - 1] = wc;
    }
}

void string_clear(String *s) {
    free(s->buffer);
    s->buffer = NULL;
    s->capacity = 0;
    s->count = 0;
}

void string_copy(String *dst, const String *src) {
    dst->capacity = src->capacity;
    dst->count = src->count;
    dst->buffer = (wchar_t *)malloc(dst->capacity * sizeof(wchar_t));
    memcpy(dst->buffer, src->buffer, src->capacity * sizeof(wchar_t));
}

void string_copy_to_buffer(const String *s, wchar_t **pBuf) {
    wchar_t *buf;
    buf = (wchar_t *)malloc((s->count + 1) * sizeof(wchar_t));
    memcpy(buf, s->buffer, (s->count + 1) * sizeof(wchar_t));
    *pBuf = buf;
}

void string_to_lower_case(String *s) {
    unsigned long i;
    for (i = 0; i < s->count; i++) {
        if (iswupper(s->buffer[i]))
            s->buffer[i] = towlower(s->buffer[i]);
    }
}

void string_to_upper_case(String *s) {
    unsigned long i;
    for (i = 0; i < s->count; i++) {
        if (iswlower(s->buffer[i]))
            s->buffer[i] = towupper(s->buffer[i]);
    }
}

int string_ignore_case_compare(const String *s, const wchar_t *wcs, unsigned long count) {
    unsigned long i;
    if (count != s->count) return -1;  /* not equal */
    for (i = 0; i < s->count; i++) {
        if (towupper(s->buffer[i]) != towupper(wcs[i])) return 1;  /* not equal */
    }
    return 0;  /* equal */
}

/* merge all white spaces to a single half white space between two non white space characters in the string, here white space includes half or full white space and tab */
void string_merge_all_white_spaces(String *s) {
    unsigned long i, count, flag;
    wchar_t temp;
    flag = 0;
    count = 0;
    for (i = 0; i < s->count; i++) {
        temp = s->buffer[i];
        if (L' ' == temp || L'　' == temp || L'	' == temp) {
            if (0 == flag && 0 != count) {
                flag = 1;
                temp = L' ';
            } else continue;
        }
        if (L' ' != temp) flag = 0;
        s->buffer[count++] = temp;
    }
    if (L' ' == s->buffer[count - 1]) count--;
    memset(s->buffer + count, 0, (s->count - count) * sizeof(wchar_t));
    s->count = count;
}

/* discard begin and end white spaces in the string, here white space includes half or full white space and tab */
void string_discard_begin_end_white_spaces_and_quote(String *s) {
    unsigned long i, count1, count2, count;
    count1 = 0;
    count2 = 0;
    for (i = 0; i < s->count; i++) {
        if (L' ' != s->buffer[i] && L'　' != s->buffer[i] && L'	' != s->buffer[i]) break;
        count1++;
    }
    if (L'\"' == s->buffer[i]) count1++;
    for (i = s->count - 1; i >= 0; i--) {
        if (L' ' != s->buffer[i] && L'　' != s->buffer[i] && L'	' != s->buffer[i]) break;
        count2++;
    }
    if (L'\"' == s->buffer[i]) count2++;
    count = s->count - count1 - count2;
    for (i = 0; i < count; i++)
        s->buffer[i] = s->buffer[i + count1];
    memset(s->buffer + count, 0, (s->count - count) * sizeof(wchar_t));
    s->count = count;
}

