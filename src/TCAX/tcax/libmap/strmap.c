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

#include "strmap.h"


#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif    /* _MSC_VER */

#if defined WIN32
/* Convert between unicode big endian and unicode little endian */
static void _libmap_convert_endian(wchar_t *buffer, int count) {
    int i;
    for (i = 0; i < count; i++)
        buffer[i] = ((0x00FF & buffer[i]) << 8) | ((0xFF00 & buffer[i]) >> 8);
}

static int _libmap_read_file_to_unicode(const char *filename, wchar_t **pBuffer, int *pCount) {
    FILE *fp;
    int len;    /* length of the file */
    unsigned char *rawBuffer;    /* raw buffer of the file */
    fp = fopen(filename, "rb");
    if (!fp) return -1;
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);    /* length of the file */
    fseek(fp, 0, SEEK_SET);
    rawBuffer = (unsigned char *)malloc(len + 2);    /* note that `+2' is intending to add a null terminator */
    rawBuffer[len] = 0;
    rawBuffer[len + 1] = 0;
    if (fread(rawBuffer, sizeof(unsigned char), len, fp) != len) {
        free(rawBuffer);
        fclose(fp);
        return -1;
    }
    fclose(fp);
    if (len >= 2 && 0xFF == rawBuffer[0] && 0xFE == rawBuffer[1]) {    /* unicode little endian */
        *pBuffer = (wchar_t *)rawBuffer;
        *pCount = (len + 2) / 2;    /* the BOM header and the NULL terminator are included */
    } else if (len >= 2 && 0xFE == rawBuffer[0] && 0xFF == rawBuffer[1]) {    /* unicode big endian */
        _libmap_convert_endian((wchar_t *)rawBuffer, len / 2);
        *pBuffer = (wchar_t *)rawBuffer;
        *pCount = (len + 2) / 2;    /* the BOM header and the NULL terminator are included */
    } else if (len >= 3 && 0xEF == rawBuffer[0] && 0xBB == rawBuffer[1] && 0xBF == rawBuffer[2]) {    /* utf-8 */
        wchar_t *buffer;
        int count;
        count = MultiByteToWideChar(CP_UTF8, 0, (const char *)rawBuffer, -1, NULL, 0);
        buffer = (wchar_t *)malloc(count * sizeof(wchar_t));
        MultiByteToWideChar(CP_UTF8, 0, (const char *)rawBuffer, -1, buffer, count);
        free(rawBuffer);
        *pBuffer = buffer;
        *pCount = count;    /* the BOM header and the NULL terminator are included */
    } else {    /* ansi */
        wchar_t *buffer;
        int count;
        count = MultiByteToWideChar(CP_ACP, 0, (const char *)rawBuffer, -1, NULL, 0);
        buffer = (wchar_t *)malloc(count * sizeof(wchar_t));
        MultiByteToWideChar(CP_ACP, 0, (const char *)rawBuffer, -1, buffer, count);
        free(rawBuffer);
        *pBuffer = buffer;
        *pCount = count;    /* the NULL terminator is included */
    }
    return 0;
}
#else
static int _libmap_read_file_to_unicode(const char *filename, wchar_t **pBuffer, int *pCount) {
    FILE *fp;
    int len;    /* length of the file */
    unsigned char *rawBuffer;    /* raw buffer of the file */
    fp = fopen(filename, "rb");
    if (!fp) return -1;
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);    /* length of the file */
    fseek(fp, 0, SEEK_SET);
    rawBuffer = (unsigned char *)malloc(len + 2);    /* note that `+2' is intending to add a null terminator */
    rawBuffer[len] = 0;
    rawBuffer[len + 1] = 0;
    if (fread(rawBuffer, sizeof(unsigned char), len, fp) != len) {
        free(rawBuffer);
        fclose(fp);
        return -1;
    }
    fclose(fp);
    if (len >= 3 && 0xEF == rawBuffer[0] && 0xBB == rawBuffer[1] && 0xBF == rawBuffer[2]) {    /* utf-8 */
        wchar_t *buffer;
        int count;
        count = mbstowcs(0, (const char *)(rawBuffer + 3), 0) + 1;
        buffer = (wchar_t *)malloc(count * sizeof(wchar_t));
        mbstowcs(buffer, (const char *)(rawBuffer + 3), count);
        free(rawBuffer);
        *pBuffer = buffer;
        *pCount = count;    /* the BOM header and the NULL terminator are included */setlocale(LC_CTYPE, "");
    } else return -1;
    return 0;
}
#endif    /* WIN32 */

int libmap_read_map_file(const char *filename, StrMapPtr mptr) {
    return _libmap_read_file_to_unicode(filename, &mptr->buffer, &mptr->count);
}

/* add a pair to the map, and return the next start position */
static int _libmap_add_pair(StrMapPtr mptr, int startPos) {
    int i, len, stat, flag, pos;
    wchar_t *str, *key, *value;
    len = mptr->count - startPos;
    str = mptr->buffer + startPos;
    stat = SCAN_STAT_INIT;
    flag = QUOTE_FLAG_NONE;
    for (i = 0; i < len; i++) {
        switch (stat) {
        /* will skip all the characters until it meets the '<' mark */
        case SCAN_STAT_INIT:
            if (L'<' == str[i])
                stat = SCAN_STAT_BEFORE_KEY;
            break;
        /* will skip all the spaces */
        case SCAN_STAT_BEFORE_KEY:
            if (L'<' == str[i] || L'=' == str[i] || L'>' == str[i] || L'\r' == str[i] || L'\n' == str[i])      /* we regard this element invalid */
                return startPos + i;
            else if (L' ' != str[i]) {    /* when we meet the first non-space character, the parsing phase begins */
                stat = SCAN_STAT_KEY;
                key = str + i;
                if (L'\"' == str[i])     /* if we are to use the quote mark */
                    flag = QUOTE_FLAG_QUOTED;
            }
            break;
        /* retrieve the raw key */
        case SCAN_STAT_KEY:
            if (QUOTE_FLAG_QUOTED == flag) {        /* be careful to use the quote mark, if you forget to enclose it, the behavior is likely to be undefined */
                if (L'\"' == str[i]) {
                    if (L'\"' == str[i + 1])
                        i++;
                    else {
                        stat = SCAN_STAT_AFTER_KEY;
                        flag = QUOTE_FLAG_ENCLOSED;
                        str[i] = L'\0';     /* Note that, the left " will be kept as an indication during the polishing phase, while the right " is discarded */
                    }
                }
            } else {
                if (L'<' == str[i] || L'\"' == str[i] || L'>' == str[i] || L'\r' == str[i] || L'\n' == str[i])      /* we regard this element invalid */
                    return startPos + i;
                else if (L'=' == str[i]) {
                    stat = SCAN_STAT_BEFORE_VALUE;
                    pos = 1;
                    while (L' ' == str[i - pos])    /* discard the right space characters */
                        pos++;
                    str[i - pos + 1] = L'\0';
                }
            }
            break;
        /* should be all white spaces */
        case SCAN_STAT_AFTER_KEY:
            if (L'=' == str[i])
                stat = SCAN_STAT_BEFORE_VALUE;
            else if (L' ' != str[i])
                return startPos + i;
            break;
        case SCAN_STAT_BEFORE_VALUE:
            if (L'>' == str[i]) {    /* the value is empty */
                stat = SCAN_STAT_FIN;
                value = str + i;
                str[i] = L'\0';
            } else if (L'<' == str[i] || L'=' == str[i] || L'\r' == str[i] || L'\n' == str[i])      /* we regard this element invalid */
                return startPos + i;
            else if (L' ' != str[i]) {    /* when we meet the first non-space character, the parsing phase begins */
                stat = SCAN_STAT_VALUE;
                value = str + i;
                if (L'\"' == str[i]) {     /* if we are to use the quote mark */
                    flag = QUOTE_FLAG_QUOTED;
                    value++;
                }
            }
            break;
        /* retrieve the raw value */
        case SCAN_STAT_VALUE:
            if (QUOTE_FLAG_QUOTED == flag) {        /* be careful to use the quote mark, if you forget to enclose it, the behavior is likely to be undefined */
                if (L'\"' == str[i]) {
                    if (L'\"' == str[i + 1])
                        i++;
                    else {
                        stat = SCAN_STAT_AFTER_VALUE;
                        flag = QUOTE_FLAG_ENCLOSED;
                        str[i] = L'\0';
                    }
                }
            } else {
                if (L'<' == str[i] || L'\"' == str[i] || L'=' == str[i] || L'\r' == str[i] || L'\n' == str[i])      /* we regard this element invalid */
                    return startPos + i;
                else if (L'>' == str[i]) {
                    stat = SCAN_STAT_FIN;
                    pos = 1;
                    while (L' ' == str[i - pos])    /* discard the right space characters */
                        pos++;
                    str[i - pos + 1] = L'\0';
                }
            }
            break;
        /* should be all white spaces */
        case SCAN_STAT_AFTER_VALUE:
            if (L'>' == str[i])
                stat = SCAN_STAT_FIN;
            else if (L' ' != str[i])
                return startPos + i;
            break;
        /* finish, the end of the whole process */
        case SCAN_STAT_FIN:
            //wprintf(L"key = `%s'\n", key);
            //wprintf(L"value = `%s'\n\n", value);
            tcc_map_push_back(mptr->map, &key, &value);
            return startPos + i;
        }
    }
    return -1;    /* no more elements to retrieve */
}

static int _libmap_string_compare(const wchar_t **dst, const wchar_t **src) {
    return wcscmp(*dst, *src);
}

void libmap_parse(StrMapPtr mptr) {
    int startPos;
    tcc_map_create(&mptr->map, sizeof(wchar_t *), sizeof(wchar_t *), 0, NULL, NULL, NULL, NULL, (tcc_map_key_compare_func)_libmap_string_compare, (tcc_map_value_compare_func)_libmap_string_compare);    /* all the strings are only low copy, the real content are stored in the buffer */
    startPos = 0;
    do
        startPos = _libmap_add_pair(mptr, startPos);
    while (-1 != startPos);
}

static void _libmap_polish_key(wchar_t *key) {
    int i, skip, count;
    //wprintf(L"key = `%s'\n", key);
    count = wcslen(key) + 1;    /* include the NULL terminator */
    if (L'\"' == key[0]) {      /* quoted key, note that only the left quote has been kept, while the right quote was discarded */
        skip = 1;
        for (i = 1; i < count; i++) {
            if (L'\"' == key[i]) {      /* for a quoted key, only the double "" will be merged into a single ", while the spaces will be kept */
                key[i - skip] = L'\"';
                skip++;
                i++;
            } else
                key[i - skip] = key[i];
        }
    } else {
        skip = 0;
        for (i = 0; i < count; i++) {
            if (L'\"' == key[i]) {
                key[i - skip] = L'\"';
                skip++;
                i++;
            } else if (L' ' == key[i]) {
                key[i - skip] = L' ';
                while (L' ' == key[i + 1]) {
                    skip++;
                    i++;
                }
            } else
                key[i - skip] = key[i];
        }
    }
    //wprintf(L"key = `%s'\n", key);
}

static void _libmap_polish_value(wchar_t *value) {
    int i, skip, count;
    //wprintf(L"value = `%s'\n", value);
    count = wcslen(value) + 1;    /* include the NULL terminator */
    skip = 0;
    for (i = 0; i < count; i++) {
        if (L'\"' == value[i]) {
            value[i - skip] = L'\"';
            skip++;
            i++;
        } else
            value[i - skip] = value[i];
    }
    //wprintf(L"value = `%s'\n", value);
}

void libmap_polish(StrMapPtr mptr) {
    unsigned long i;
    for (i = 0; i < mptr->map->count; i++) {
        _libmap_polish_key(*(wchar_t **)((unsigned char *)mptr->map->keyBuf + i * mptr->map->keySize));
        _libmap_polish_value(*(wchar_t **)((unsigned char *)mptr->map->valueBuf + i * mptr->map->valueSize));
    }
}

void libmap_destroy(StrMapPtr mptr) {
    tcc_map_destroy(mptr->map);
    free(mptr->buffer);
}

