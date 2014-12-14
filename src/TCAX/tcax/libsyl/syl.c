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

#include "syl.h"

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif    /* _MSC_VER */

#if defined WIN32
/* Convert between unicode big endian and unicode little endian */
static void _libsyl_convert_endian(wchar_t *buffer, int count) {
    int i;
    unsigned char temp;
    unsigned char *buf;
    buf = (unsigned char *)buffer;
    for (i = 0; i < count; i++) {
        temp = buf[i << 1];
        buf[i << 1] = buf[(i << 1) + 1];
        buf[(i << 1) + 1] = temp;
    }
}

SYL_Error_Code libsyl_read_file_to_unicode(const char *filename, wchar_t **pBuffer, int *pCount) {
    FILE *fp;
    int size, count;
    unsigned char *rawBuffer;
    wchar_t *buffer;
    fp = fopen(filename, "rb");
    if (!fp) return syl_error_file_cant_open;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    rawBuffer = (unsigned char *)malloc(size + 6);    /* note that `+6' is intending to add an L"\r\n" and a null terminator */
    rawBuffer[size] = '\r';
    rawBuffer[size + 1] = 0;
    rawBuffer[size + 2] = '\n';
    rawBuffer[size + 3] = 0;
    rawBuffer[size + 4] = '\0';
    rawBuffer[size + 5] = 0;
    if (fread(rawBuffer, sizeof(unsigned char), size, fp) != size) {
        free(rawBuffer);
        fclose(fp);
        return syl_error_file_while_reading;
    }
    fclose(fp);
    if (size >= 2 && 0xFF == rawBuffer[0] && 0xFE == rawBuffer[1]) {    /* unicode little endian */
        count = (size - 2) / 2 + 2;        /* `-2' is intending to reduce count for the BOM-header, `+2' is intending to add count for the L"\r\n" */
        buffer = (wchar_t *)malloc((count + 1) * sizeof(wchar_t));    /* `+1' is intending to add the null terminator */
        memcpy(buffer, rawBuffer + 2, (count + 1) * sizeof(wchar_t));    /* `+2' is intending to remove the BOM-header */
        free(rawBuffer);
        *pBuffer = buffer;
        *pCount = count;
    } else if (size >= 2 && 0xFE == rawBuffer[0] && 0xFF == rawBuffer[1]) {    /* unicode big endian */
        _libsyl_convert_endian((wchar_t *)rawBuffer, size / 2);
        count = (size - 2) / 2 + 2;        /* `-2' is intending to reduce count for the BOM-header, `+2' is intending to add count for the L"\r\n" */
        buffer = (wchar_t *)malloc((count + 1) * sizeof(wchar_t));    /* `+1' is intending to add the null terminator */
        memcpy(buffer, rawBuffer + 2, (count + 1) * sizeof(wchar_t));    /* `+2' is intending to remove the BOM-header */
        free(rawBuffer);
        *pBuffer = buffer;
        *pCount = count;
    } else if (size >= 3 && 0xEF == rawBuffer[0] && 0xBB == rawBuffer[1] && 0xBF == rawBuffer[2]) {    /* utf-8 */
        count = MultiByteToWideChar(CP_UTF8, 0, (const char *)(rawBuffer + 3), size - 3, NULL, 0);    /* `+3' is intending to remove the BOM-header, count is not including the null terminator */
        buffer = (wchar_t *)malloc((count + 3) * sizeof(wchar_t));    /* `+3' is intending to add L"\r\n" and null terminator */
        MultiByteToWideChar(CP_UTF8, 0, (const char *)(rawBuffer + 3), size - 3, buffer, count);    /* `+3' is intending to remove the BOM-header */
        free(rawBuffer);
        buffer[count] = L'\r';
        buffer[count + 1] = L'\n';
        buffer[count + 2] = L'\0';
        *pBuffer = buffer;
        *pCount = count + 2;    /* add count for L"\r\n" */
    } else {    /* ansi */
        count = MultiByteToWideChar(CP_ACP, 0, (const char *)rawBuffer, size, NULL, 0);    /* not including the null terminator */
        buffer = (wchar_t *)malloc((count + 3) * sizeof(wchar_t));    /* `+3' is intending to add L"\r\n" and null terminator */
        MultiByteToWideChar(CP_ACP, 0, (const char *)rawBuffer, size, buffer, count);
        free(rawBuffer);
        buffer[count] = L'\r';
        buffer[count + 1] = L'\n';
        buffer[count + 2] = L'\0';
        *pBuffer = buffer;
        *pCount = count + 2;    /* add count for L"\r\n" */
    }
    return syl_error_success;
}
#else
SYL_Error_Code libsyl_read_file_to_unicode(const char *filename, wchar_t **pBuffer, int *pCount) {
    FILE *fp;
    int size, count;
    unsigned char *rawBuffer;
    wchar_t *buffer;
    fp = fopen(filename, "rb");
    if (!fp) return syl_error_file_cant_open;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    rawBuffer = (unsigned char *)malloc(size + 3);    /* note that `+6' is intending to add an L"\r\n" and a null terminator */
    rawBuffer[size] = '\r';
    rawBuffer[size + 1] = '\n';
    rawBuffer[size + 2] = '\0';
    if (fread(rawBuffer, sizeof(unsigned char), size, fp) != size) {
        free(rawBuffer);
        fclose(fp);
        return syl_error_file_while_reading;
    }
    fclose(fp);
    if (size >= 3 && 0xEF == rawBuffer[0] && 0xBB == rawBuffer[1] && 0xBF == rawBuffer[2]) {    /* utf-8 */
        setlocale(LC_CTYPE, "");
        count = mbstowcs(0, (const char *)(rawBuffer + 3), 0) + 1;    /* `+3' is intending to remove the BOM-header, count is not including the null terminator */
        buffer = (wchar_t *)malloc(count * sizeof(wchar_t));
        mbstowcs(buffer, (const char *)(rawBuffer + 3), count);
        free(rawBuffer);
        *pBuffer = buffer;
        *pCount = count;
    } else return syl_error_file_while_reading;
    return syl_error_success;
}
#endif  /* WIN32 */

void libsyl_create_simplest_syl_string(wchar_t **pBuffer, int *pCount) {
    int count;
    wchar_t *buffer;
    const wchar_t *sylString = L"Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n\
Dialogue: 0,0:00:00.00,0:00:00.00,TCMS,NTP,0000,0000,0000,,{\\k0}\r\n";
    count = wcslen(sylString);
    buffer = (wchar_t *)malloc((count + 1) * sizeof(wchar_t));
    memcpy(buffer, sylString, (count + 1) * sizeof(wchar_t));
    *pBuffer = buffer;
    *pCount = count;
}

/* a time string: "0:01:10.63" */
static int _libsyl_get_time_from_time_string(const wchar_t *timeString, int count) {
    int time, len;
    wchar_t sz[8];
    const wchar_t *str, *str2;
    time = 0;
    memset(sz, 0, 8 * sizeof(wchar_t));
    str = wcsstr(timeString, L":");
    if (!str) return -1;
    len = str - timeString;
    memcpy(sz, timeString, len * sizeof(wchar_t));
    time += wcstol(sz, NULL, 10) * 60 * 60 * 100;
    memset(sz, 0, 8 * sizeof(wchar_t));
    str += 1;
    str2 = wcsstr(str, L":");
    if (!str2) return -1;
    len = str2 - str;
    memcpy(sz, str, len * sizeof(wchar_t));
    time += wcstol(sz, NULL, 10) * 60 * 100;
    memset(sz, 0, 8 * sizeof(wchar_t));
    str += len + 1;
    str2 = wcsstr(str, L".");
    if (!str2) return -1;
    len = str2 - str;
    memcpy(sz, str, len * sizeof(wchar_t));
    time += wcstol(sz, NULL, 10) * 100;
    memset(sz, 0, 8 * sizeof(wchar_t));
    str += len + 1;
    len = timeString + count - str;
    if (len < 0) return -1;
    memcpy(sz, str, len * sizeof(wchar_t));
    time += wcstol(sz, NULL, 10);
    return time;
}

static int _libsyl_line_to_skip(const wchar_t *sylString, int index1, int index2) {
    if (index2 - index1 > 50) {
        int i;
        for (i = index1; i < index2; i++)
            if (L'\\' == sylString[i]) return 0;
    }
    return 1;
}

SYL_Error_Code libsyl_parse_syl_string(const wchar_t *sylString, int count, SYL_pLine pLine) {
    const wchar_t *indicator = L"Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text";
    const wchar_t *str;
    int i, index1, index2, len, indicatorLen, headerLen, lineCount;
    indicatorLen = wcslen(indicator);
    str = wcsstr(sylString, indicator);
    if (!str) return syl_error_file_bad_format;
    /* get SYL header */
    headerLen = str + indicatorLen - sylString;
    pLine->header = (wchar_t *)malloc((headerLen + 3) * sizeof(wchar_t));    /* `+3' is intending to add L"\r\n\0" to the end of header string*/
    memcpy(pLine->header, sylString, headerLen * sizeof(wchar_t));
    pLine->header[headerLen] = L'\r';
    pLine->header[headerLen + 1] = L'\n';
    pLine->header[headerLen + 2] = L'\0';
    /* get SYL lines */
    count -= headerLen;
    sylString = str + indicatorLen;
    lineCount = 0;
    for (i = 0; i < count; i++) {
        if (L'\r' == sylString[i] && L'\n' == sylString[i + 1]) lineCount++;
    }
    pLine->line1D = (wchar_t **)malloc(lineCount * sizeof(wchar_t *));
    lineCount = 0;
    for (i = 0; i < count; i++) {
        if (L'\r' == sylString[i] && L'\n' == sylString[i + 1]) {
            if (0 == i) index1 = i + 2;
            else {
                index2 = i;
                if (!_libsyl_line_to_skip(sylString, index1, index2)) {
                    pLine->line1D[lineCount] = (wchar_t *)malloc((index2 - index1 + 1) * sizeof(wchar_t));
                    memcpy(pLine->line1D[lineCount], sylString + index1, (index2 - index1) * sizeof(wchar_t));
                    pLine->line1D[lineCount][index2 - index1] = L'\0';
                    lineCount++;
                }
                index1 = index2 + 2;
            }
        }
    }
    pLine->lines = lineCount;
    pLine->begTime1D = (int *)malloc(pLine->lines * sizeof(int));
    pLine->endTime1D = (int *)malloc(pLine->lines * sizeof(int));
    for (i = 0; i < pLine->lines; i++) {
        str = wcsstr(pLine->line1D[i], L",") + 1;
        len = wcsstr(str, L",") - str;
        pLine->begTime1D[i] = _libsyl_get_time_from_time_string(str, len);
        if (pLine->begTime1D[i] < 0) {
            free(pLine->header);
            free(pLine->line1D);
            free(pLine->begTime1D);
            return syl_error_file_invalid;
        }
        str += len + 1;
        len = wcsstr(str, L",") - str;
        pLine->endTime1D[i] = _libsyl_get_time_from_time_string(str, len);
        if (pLine->endTime1D[i] < 0) {
            free(pLine->header);
            free(pLine->line1D);
            free(pLine->begTime1D);
            free(pLine->endTime1D);
            return syl_error_file_invalid;
        }
    }
    return syl_error_success;
}

SYL_Error_Code libsyl_check_line_format(const SYL_pLine pLine, int *pi, int *pj) {
    int i, j, lineIndex, kIndex, prevLineIndex, prevKIndex, len, flag;
    flag = 0;
    lineIndex = 1;
    for (i = 0; i < pLine->lines; i++) {
        if (wcsstr(pLine->line1D[i], L",,") == NULL) {
            *pi = lineIndex;
            *pj = 0;
            return syl_error_file_bad_format;
        }
        if (0 == flag) prevLineIndex = lineIndex;    /* the previous line is ended with a `}' */
        kIndex = 1;
        len = wcslen(pLine->line1D[i]);
        for (j = 0; j < len; j++) {
            if (L'{' == pLine->line1D[i][j]) {
                if (0 != flag || L'\\' != pLine->line1D[i][j + 1]) {
                    if (2 == flag) {    /* the previous line is not ended with a `}' */
                        *pi = prevLineIndex;
                        *pj = prevKIndex;
                    } else {
                        *pi = lineIndex;
                        *pj = kIndex;
                    }
                    return syl_error_file_bad_format;
                }
                flag = 1;
            } else if (L'\\' == pLine->line1D[i][j]) {
                if (L'k' == pLine->line1D[i][j + 1] || L'K' == pLine->line1D[i][j + 1]) {
                    if (1 != flag) {
                        *pi = lineIndex;
                        *pj = kIndex;
                        return syl_error_file_bad_format;
                    }
                    flag = 2;
                    prevKIndex = kIndex;
                    kIndex++;
                }
            } else if (L'}' == pLine->line1D[i][j]) {
                if (2 != flag) {
                    *pi = lineIndex;
                    *pj = kIndex;
                    return syl_error_file_bad_format;
                }
                flag = 0;
            }
        }
        lineIndex++;
    }
    if (0 != flag) {    /* equals to 2, the previous line is not ended with a `}' */
        *pi = prevLineIndex;
        *pj = prevKIndex;
        return syl_error_file_bad_format;
    }
    return syl_error_success;
}

void libsyl_add_k0_to_white_spaces(SYL_pLine pLine) {
    int i, j, index, len1, len2, count, spaces;
    const wchar_t *str;
    wchar_t *tempSylLine;
    for (i = 0; i < pLine->lines; i++) {
        spaces = 0;
        str = wcsstr(pLine->line1D[i], L",,") + 2;
        len1 = str - pLine->line1D[i];
        len2 = wcslen(str);
        for (j = 0; j < len2; j++) {
            if (L' ' == str[j] || L'　' == str[j]) spaces++;
        }
        count = len1 + len2 + spaces * 5;
        tempSylLine = (wchar_t *)malloc((count + 1) * sizeof(wchar_t));
        memcpy(tempSylLine, pLine->line1D[i], len1 * sizeof(wchar_t));
        index = len1;
        for (j = 0; j < len2; j++) {
            if (L' ' != str[j] && L'　' != str[j]) tempSylLine[index] = str[j];
            else {
                if (L'}' != str[j - 1]) {
                    memcpy(tempSylLine + index, L"{\\k0}", 5 * sizeof(wchar_t));
                    index += 5;
                }
                tempSylLine[index] = str[j];
            }
            index++;
        }
        tempSylLine[index] = L'\0';
        free(pLine->line1D[i]);
        pLine->line1D[i] = tempSylLine;
    }
}

void libsyl_del_first_null_k(SYL_pLine pLine) {
    int i, len;
    wchar_t *str, *str1, *str2;
    wchar_t sz[32];
    for (i = 0; i < pLine->lines; i++) {
        str = pLine->line1D[i];
        str1 = wcsstr(str, L"{");    /* for a valid SYL line, there must be at least one k-tag */
        str2 = wcsstr(str, L"}");
        if (L'{' == str2[1]) {    /* the first k-tag is a NULL k-tag */
            len = str2 - str1 - 3;
            memcpy(sz, str1 + 3, len * sizeof(wchar_t));
            sz[len] = L'\0';
            pLine->begTime1D[i] += wcstol(sz, NULL, 10);    /* add the k-time of the first NULL k-tag to the begin time */
            len = wcslen(str) - (str2 - str + 1) + 1;
            pLine->line1D[i] = (wchar_t *)malloc(len * sizeof(wchar_t));
            memcpy(pLine->line1D[i], str2 + 1, len * sizeof(wchar_t));
            free(str);
        }
    }
}

void libsyl_parse_line(const SYL_pLine pLine, SYL_pWord pWord) {
    int i, j, len, count, index;
    const wchar_t *str;
    wchar_t *sz;
    pWord->lines = pLine->lines;
    pWord->kCount1D = (int *)malloc(pWord->lines * sizeof(int));
    memset(pWord->kCount1D, 0, pWord->lines * sizeof(int));
    count = 0;
    for (i = 0; i < pLine->lines; i++) {
        len = wcslen(pLine->line1D[i]);
        for (j = 0; j < len; j++) {
            if (L'}' == pLine->line1D[i][j]) pWord->kCount1D[i]++;
        }
        count += pWord->kCount1D[i];
    }
    pWord->words = count;
    pWord->kTime2D = (int *)malloc(pWord->words * sizeof(int));
    count = 0;
    for (i = 0; i < pWord->lines; i++) {
        str = pLine->line1D[i];
        for (j = 0; j < pWord->kCount1D[i]; j++) {
            str = wcsstr(str, L"{") + 3;
            len = wcsstr(str, L"}") - str;
            sz = (wchar_t *)malloc((len + 1) * sizeof(wchar_t));
            memcpy(sz, str, len * sizeof(wchar_t));
            sz[len] = L'\0';
            pWord->kTime2D[count++] = wcstol(sz, NULL, 10);
            free(sz);
        }
    }
    pWord->kTimeDiff2D = (int *)malloc(pWord->words * sizeof(int));
    memset(pWord->kTimeDiff2D, 0, pWord->words * sizeof(int));
    count = 0;
    index = 0;
    for (i = 0; i < pWord->lines; i++) {
        pWord->kTimeDiff2D[index] = 0;
        count++;
        for (j = 1; j < pWord->kCount1D[i]; j++) {
            pWord->kTimeDiff2D[count] = pWord->kTimeDiff2D[count - 1] + pWord->kTime2D[count - 1];
            count++;
        }
        index += pWord->kCount1D[i];
    }
    pWord->word2D = (wchar_t **)malloc(pWord->words * sizeof(wchar_t *));
    count = 0;
    for (i = 0; i < pWord->lines; i++) {
        str = pLine->line1D[i];
        for (j = 0; j < pWord->kCount1D[i]; j++) {
            str = wcsstr(str, L"}") + 1;
            if (pWord->kCount1D[i] - 1 == j)
                len = pLine->line1D[i] + wcslen(pLine->line1D[i]) - str;
            else
                len = wcsstr(str, L"{") - str;
            pWord->word2D[count] = (wchar_t *)malloc((len + 1) * sizeof(wchar_t));
            memcpy(pWord->word2D[count], str, len * sizeof(wchar_t));
            pWord->word2D[count][len] = L'\0';
            count++;
        }
    }
}

void libsyl_parse_word_to_syllable(const SYL_pWord pWord, SYL_pSyllable pSyllable) {
    int i, j, k, len, count, index, spaces;
    int *kSylCount2D;    /* number of syllables in a word */
    pSyllable->lines = pWord->lines;
    count = 0;
    for (i = 0; i < pWord->words; i++) {
        len = wcslen(pWord->word2D[i]);
        count += (0 == len) ? 1 : len;
    }
    pSyllable->syls = count;
    pSyllable->sylCount1D = (int *)malloc(pSyllable->lines * sizeof(int));
    memset(pSyllable->sylCount1D, 0, pSyllable->lines * sizeof(int));
    kSylCount2D = (int *)malloc(pWord->words * sizeof(int));
    memset(kSylCount2D, 0, pWord->words * sizeof(int));
    count = 0;
    for (i = 0; i < pWord->lines; i++) {
        for (j = 0; j < pWord->kCount1D[i]; j++) {
            len = wcslen(pWord->word2D[count]);
            kSylCount2D[count] = (0 == len) ? 1 : len;
            pSyllable->sylCount1D[i] += kSylCount2D[count];
            count++;
        }
    }
    pSyllable->syl2D = (wchar_t *)malloc(pSyllable->syls * sizeof(wchar_t));
    count = 0;
    index = 0;
    for (i = 0; i < pWord->lines; i++) {
        for (j = 0; j < pWord->kCount1D[i]; j++) {
            for (k = 0; k < kSylCount2D[count]; k++) {
                pSyllable->syl2D[index] = pWord->word2D[count][k];
                index++;
            }
            count++;
        }
    }
    pSyllable->sylTime2D = (int *)malloc(pSyllable->syls * sizeof(int));
    count = 0;
    index = 0;
    for (i = 0; i < pWord->lines; i++) {
        for (j = 0; j < pWord->kCount1D[i]; j++) {
            spaces = 0;
            for (k = 0; k < kSylCount2D[count]; k++) {
                if (L' ' == pSyllable->syl2D[index] || L'　' == pSyllable->syl2D[index]) {
                    pSyllable->sylTime2D[index] = 0;
                    spaces++;
                }
                index++;
            }
            index -= kSylCount2D[count];
            if (spaces == kSylCount2D[count]) {
                for (k = 0; k < kSylCount2D[count]; k++) {
                    pSyllable->sylTime2D[index] = pWord->kTime2D[count] / kSylCount2D[count];
                    if (k < pWord->kTime2D[count] % kSylCount2D[count]) pSyllable->sylTime2D[index]++;
                    index++;
                }
            } else {
                for (k = 0; k < kSylCount2D[count]; k++) {
                    if (L' ' == pSyllable->syl2D[index] || L'　' == pSyllable->syl2D[index]) {
                        index++;
                        continue;
                    }
                    pSyllable->sylTime2D[index] = pWord->kTime2D[count] / (kSylCount2D[count] - spaces);
                    if (k < pWord->kTime2D[count] % (kSylCount2D[count] - spaces)) pSyllable->sylTime2D[index]++;
                    index++;
                }
            }
            count++;
        }
    }
    free(kSylCount2D);
    pSyllable->sylTimeDiff2D = (int *)malloc(pSyllable->syls * sizeof(int));
    memset(pSyllable->sylTimeDiff2D, 0, pSyllable->syls * sizeof(int));
    count = 0;
    index = 0;
    for (i = 0; i < pSyllable->lines; i++) {
        pSyllable->sylTimeDiff2D[index] = 0;
        count++;
        for (j = 1; j < pSyllable->sylCount1D[i]; j++) {
            pSyllable->sylTimeDiff2D[count] = pSyllable->sylTimeDiff2D[count - 1] + pSyllable->sylTime2D[count - 1];
            count++;
        }
        index += pSyllable->sylCount1D[i];
    }
}

int libsyl_is_white_spaces(const wchar_t *str, int count) {
    int i;
    for (i = 0; i < count; i++) {
        if (L' ' != str[i] && L'　' != str[i]) return 0;    /* 0 means that the characters in the string are not all white spaces */
    }
    return 1;    /* 1 means that the characters in the string are all white spaces */
}

void libsyl_parse_word_to_adv(const SYL_pWord pWord, SYL_pAdv pAdv) {
    int i, j, k, len, count, index, spaces;
    int *kAdvCount2D;    /* number of adv words in a word */
    pAdv->lines = pWord->lines;
    count = 0;
    for (i = 0; i < pWord->words; i++) {
        len = wcslen(pWord->word2D[i]);
        while (len > 2) {
            count++;
            len -= 2;
        }
        count++;
    }
    pAdv->advs = count;
    pAdv->advCount1D = (int *)malloc(pAdv->lines * sizeof(int));
    memset(pAdv->advCount1D, 0, pAdv->lines * sizeof(int));
    kAdvCount2D = (int *)malloc(pWord->words * sizeof(int));
    memset(kAdvCount2D, 0, pWord->words * sizeof(int));
    count = 0;
    for (i = 0; i < pWord->lines; i++) {
        for (j = 0; j < pWord->kCount1D[i]; j++) {
            len = wcslen(pWord->word2D[count]);
            while (len > 2) {
                kAdvCount2D[count]++;
                len -= 2;
            }
            kAdvCount2D[count]++;
            pAdv->advCount1D[i] += kAdvCount2D[count];
            count++;
        }
    }
    pAdv->adv2D = (wchar_t **)malloc(pAdv->advs * sizeof(wchar_t *));
    count = 0;
    index = 0;
    for (i = 0; i < pWord->lines; i++) {
        for (j = 0; j < pWord->kCount1D[i]; j++) {
            len = 2;
            for (k = 0; k < kAdvCount2D[count]; k++) {
                if (kAdvCount2D[count] - 1 == k)
                    len = wcslen(pWord->word2D[count]) - 2 * k;
                pAdv->adv2D[index] = (wchar_t *)malloc((len + 1) * sizeof(wchar_t));
                memcpy(pAdv->adv2D[index], pWord->word2D[count] + 2 * k, len * sizeof(wchar_t));
                pAdv->adv2D[index][len] = L'\0';
                index++;
            }
            count++;
        }
    }
    pAdv->advTime2D = (int *)malloc(pAdv->advs * sizeof(int));
    count = 0;
    index = 0;
    for (i = 0; i < pWord->lines; i++) {
        for (j = 0; j < pWord->kCount1D[i]; j++) {
            spaces = 0;
            for (k = 0; k < kAdvCount2D[count]; k++) {
                if (libsyl_is_white_spaces(pAdv->adv2D[index], wcslen(pAdv->adv2D[index])) == 0) {
                    pAdv->advTime2D[index] = 0;
                    spaces++;
                }
                index++;
            }
            index -= kAdvCount2D[count];
            if (spaces == kAdvCount2D[count]) {
                for (k = 0; k < kAdvCount2D[count]; k++) {
                    pAdv->advTime2D[index] = pWord->kTime2D[count] / kAdvCount2D[count];
                    if (k < pWord->kTime2D[count] % kAdvCount2D[count]) pAdv->advTime2D[index]++;
                    index++;
                }
            } else {
                for (k = 0; k < kAdvCount2D[count]; k++) {
                    if (libsyl_is_white_spaces(pAdv->adv2D[index], wcslen(pAdv->adv2D[index])) == 0) {
                        index++;
                        continue;
                    }
                    pAdv->advTime2D[index] = pWord->kTime2D[count] / (kAdvCount2D[count] - spaces);
                    if (k < pWord->kTime2D[count] % (kAdvCount2D[count] - spaces)) pAdv->advTime2D[index]++;
                    index++;
                }
            }
            count++;
        }
    }
    free(kAdvCount2D);
    pAdv->advTimeDiff2D = (int *)malloc(pAdv->advs * sizeof(int));
    memset(pAdv->advTimeDiff2D, 0, pAdv->advs * sizeof(int));
    count = 0;
    index = 0;
    for (i = 0; i < pAdv->lines; i++) {
        pAdv->advTimeDiff2D[index] = 0;
        count++;
        for (j = 1; j < pAdv->advCount1D[i]; j++) {
            pAdv->advTimeDiff2D[count] = pAdv->advTimeDiff2D[count - 1] + pAdv->advTime2D[count - 1];
            count++;
        }
        index += pAdv->advCount1D[i];
    }
}

static void _libsyl_copy_line_to_info(const SYL_pLine pLine, SYL_pInfoToTcax pInfoToTcax) {
    int i, count;
    pInfoToTcax->lines = pLine->lines;
    pInfoToTcax->line1D = (wchar_t **)malloc(pInfoToTcax->lines * sizeof(wchar_t *));
    for (i = 0; i < pLine->lines; i++) {
        count = wcslen(pLine->line1D[i]) + 1;
        pInfoToTcax->line1D[i] = (wchar_t *)malloc(count * sizeof(wchar_t));
        memcpy(pInfoToTcax->line1D[i], pLine->line1D[i], count * sizeof(wchar_t));
    }
    pInfoToTcax->begTime1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->begTime1D, pLine->begTime1D, pLine->lines * sizeof(int));
    pInfoToTcax->endTime1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->endTime1D, pLine->endTime1D, pLine->lines * sizeof(int));
}

void libsyl_info_to_tcax_with_word(const SYL_pLine pLine, const SYL_pWord pWord, SYL_pInfoToTcax pInfoToTcax) {
    int i, count;
    _libsyl_copy_line_to_info(pLine, pInfoToTcax);
    pInfoToTcax->count = pWord->words;
    pInfoToTcax->count1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->count1D, pWord->kCount1D, pWord->lines * sizeof(int));
    pInfoToTcax->time2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->time2D, pWord->kTime2D, pWord->words * sizeof(int));
    pInfoToTcax->timeDiff2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->timeDiff2D, pWord->kTimeDiff2D, pWord->words * sizeof(int));
    pInfoToTcax->text2D = (wchar_t **)malloc(pInfoToTcax->count * sizeof(wchar_t *));
    for (i = 0; i < pWord->words; i++) {
        count = wcslen(pWord->word2D[i]) + 1;
        pInfoToTcax->text2D[i] = (wchar_t *)malloc(count * sizeof(wchar_t));
        memcpy(pInfoToTcax->text2D[i], pWord->word2D[i], count * sizeof(wchar_t));
    }
}

void libsyl_info_to_tcax_with_syl(const SYL_pLine pLine, const SYL_pSyllable pSyllable, SYL_pInfoToTcax pInfoToTcax) {
    int i;
    _libsyl_copy_line_to_info(pLine, pInfoToTcax);
    pInfoToTcax->count = pSyllable->syls;
    pInfoToTcax->count1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->count1D, pSyllable->sylCount1D, pSyllable->lines * sizeof(int));
    pInfoToTcax->time2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->time2D, pSyllable->sylTime2D, pSyllable->syls * sizeof(int));
    pInfoToTcax->timeDiff2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->timeDiff2D, pSyllable->sylTimeDiff2D, pSyllable->syls * sizeof(int));
    pInfoToTcax->text2D = (wchar_t **)malloc(pInfoToTcax->count * sizeof(wchar_t *));
    for (i = 0; i < pSyllable->syls; i++) {
        pInfoToTcax->text2D[i] = (wchar_t *)malloc(2 * sizeof(wchar_t));
        pInfoToTcax->text2D[i][0] = pSyllable->syl2D[i];
        pInfoToTcax->text2D[i][1] = L'\0';
    }
}

void libsyl_info_to_tcax_with_adv(const SYL_pLine pLine, const SYL_pAdv pAdv, SYL_pInfoToTcax pInfoToTcax) {
    int i, count;
    _libsyl_copy_line_to_info(pLine, pInfoToTcax);
    pInfoToTcax->count = pAdv->advs;
    pInfoToTcax->count1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->count1D, pAdv->advCount1D, pAdv->lines * sizeof(int));
    pInfoToTcax->time2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->time2D, pAdv->advTime2D, pAdv->advs * sizeof(int));
    pInfoToTcax->timeDiff2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->timeDiff2D, pAdv->advTimeDiff2D, pAdv->advs * sizeof(int));
    pInfoToTcax->text2D = (wchar_t **)malloc(pInfoToTcax->count * sizeof(wchar_t *));
    for (i = 0; i < pAdv->advs; i++) {
        count = wcslen(pAdv->adv2D[i]) + 1;
        pInfoToTcax->text2D[i] = (wchar_t *)malloc(count * sizeof(wchar_t));
        memcpy(pInfoToTcax->text2D[i], pAdv->adv2D[i], count * sizeof(wchar_t));
    }
}

void libsyl_free_line(SYL_pLine pLine) {
    int i;
    for (i = 0; i < pLine->lines; i++)
        free(pLine->line1D[i]);
    free(pLine->line1D);
    free(pLine->begTime1D);
    free(pLine->endTime1D);
    free(pLine->header);
}

void libsyl_free_word(SYL_pWord pWord) {
    int i;
    for (i = 0; i < pWord->words; i++)
        free(pWord->word2D[i]);
    free(pWord->word2D);
    free(pWord->kCount1D);
    free(pWord->kTime2D);
    free(pWord->kTimeDiff2D);
}

void libsyl_free_syllable(SYL_pSyllable pSyllable) {
    free(pSyllable->syl2D);
    free(pSyllable->sylCount1D);
    free(pSyllable->sylTime2D);
    free(pSyllable->sylTimeDiff2D);
}

void libsyl_free_adv(SYL_pAdv pAdv) {
    int i;
    for (i = 0; i < pAdv->advs; i++)
        free(pAdv->adv2D[i]);
    free(pAdv->adv2D);
    free(pAdv->advCount1D);
    free(pAdv->advTime2D);
    free(pAdv->advTimeDiff2D);
}

void libsyl_free_info(SYL_pInfoToTcax pInfoToTcax) {
    int i;
    for (i = 0; i < pInfoToTcax->lines; i++)
        free(pInfoToTcax->line1D[i]);
    free(pInfoToTcax->line1D);
    free(pInfoToTcax->begTime1D);
    free(pInfoToTcax->endTime1D);
    for (i = 0; i < pInfoToTcax->count; i++)
        free(pInfoToTcax->text2D[i]);
    free(pInfoToTcax->text2D);
    free(pInfoToTcax->count1D);
    free(pInfoToTcax->time2D);
    free(pInfoToTcax->timeDiff2D);
}

