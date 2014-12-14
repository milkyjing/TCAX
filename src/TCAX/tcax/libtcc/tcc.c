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

#include "tcc.h"
#include "map.h"
#include "string.h"
#include "../libmap/strmap.h"


#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif    /* _MSC_VER */

#if defined WIN32
/* Convert between unicode big endian and unicode little endian */
static void _libtcc_convert_endian(wchar_t *buffer, int count) {
    int i;
    unsigned char temp;
    unsigned char *buf;
    buf = (unsigned char *)buffer;
    for (i = 0; i < count; i ++) {
        temp = buf[i << 1];
        buf[i << 1] = buf[(i << 1) + 1];
        buf[(i << 1) + 1] = temp;
    }
}

TCC_Error_Code libtcc_read_file_to_unicode(const char *filename, wchar_t **pBuffer, int *pCount) {
    FILE *fp;
    int size, count;
    unsigned char *rawBuffer;
    wchar_t *buffer;
    fp = fopen(filename, "rb");
    if (!fp) return tcc_error_file_cant_open;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    rawBuffer = (unsigned char *)malloc(size + 2);    /* note that `+2' is intending to add a null terminator */
    rawBuffer[size] = 0;
    rawBuffer[size + 1] = 0;
    if (fread(rawBuffer, sizeof(unsigned char), size, fp) != size) {
        free(rawBuffer);
        fclose(fp);
        return tcc_error_file_while_reading;
    }
    fclose(fp);
    if (size >= 2 && 0xFF == rawBuffer[0] && 0xFE == rawBuffer[1]) {    /* unicode little endian */
        count = (size - 2) / 2;        /* reduce count for the BOM-header */
        buffer = (wchar_t *)malloc((count + 1) * sizeof(wchar_t));
        memcpy(buffer, rawBuffer + 2, (count + 1) * sizeof(wchar_t));    /* `+2' is intending to remove the BOM-header */
        free(rawBuffer);
        *pBuffer = buffer;
        *pCount = count;
    } else if (size >= 2 && 0xFE == rawBuffer[0] && 0xFF == rawBuffer[1]) {    /* unicode big endian */
        _libtcc_convert_endian((wchar_t *)rawBuffer, size / 2);
        count = (size - 2) / 2;        /* reduce count for the BOM-header */
        buffer = (wchar_t *)malloc((count + 1) * sizeof(wchar_t));
        memcpy(buffer, rawBuffer + 2, (count + 1) * sizeof(wchar_t));    /* `+2' is intending to remove the BOM-header */
        free(rawBuffer);
        *pBuffer = buffer;
        *pCount = count;
    } else if (size >= 3 && 0xEF == rawBuffer[0] && 0xBB == rawBuffer[1] && 0xBF == rawBuffer[2]) {    /* utf-8 */
        count = MultiByteToWideChar(CP_UTF8, 0, (const char *)(rawBuffer + 3), -1, NULL, 0);    /* `+3' is intending to remove the BOM-header */
        buffer = (wchar_t *)malloc(count * sizeof(wchar_t));
        MultiByteToWideChar(CP_UTF8, 0, (const char *)(rawBuffer + 3), -1, buffer, count);    /* `+3' is intending to remove the BOM-header */
        free(rawBuffer);
        *pBuffer = buffer;
        *pCount = count - 1;    /* reduce count for the null terminator */
    } else {    /* ansi */
        count = MultiByteToWideChar(CP_ACP, 0, (const char *)rawBuffer, -1, NULL, 0);
        buffer = (wchar_t *)malloc(count * sizeof(wchar_t));
        MultiByteToWideChar(CP_ACP, 0, (const char *)rawBuffer, -1, buffer, count);
        free(rawBuffer);
        *pBuffer = buffer;
        *pCount = count - 1;    /* reduce count for the null terminator */
    }
    return tcc_error_success;
}

TCC_Error_Code libtcc_write_tcc_string_to_file(const char *filename, const wchar_t *tccString, int count) {
    FILE *fp;
    int size;
    char *utf8String;
    unsigned char BOM[3];
    fp = fopen(filename, "wb");
    if (!fp) return tcc_error_file_cant_create;
    BOM[0] = 0xEF;
    BOM[1] = 0xBB;
    BOM[2] = 0xBF;
    if (fwrite(BOM, sizeof(unsigned char), 3, fp) != 3) return tcc_error_file_while_writing;
    size = WideCharToMultiByte(CP_UTF8, 0, tccString, count, NULL, 0, NULL, NULL);
    utf8String = (char *)malloc(size);
    WideCharToMultiByte(CP_UTF8, 0, tccString, count, utf8String, size, NULL, NULL);
    if (fwrite(utf8String, sizeof(char), size, fp) != size) {
        free(utf8String);
        fclose(fp);
        return tcc_error_file_while_writing;
    }
    free(utf8String);
    fclose(fp);
    return tcc_error_success;
}
#else
TCC_Error_Code libtcc_read_file_to_unicode(const char *filename, wchar_t **pBuffer, int *pCount) {
    FILE *fp;
    int size, count;
    unsigned char *rawBuffer;
    wchar_t *buffer;
    fp = fopen(filename, "rb");
    if (!fp) return tcc_error_file_cant_open;
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
        return tcc_error_file_while_reading;
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
    } else return tcc_error_file_while_reading;
    return tcc_error_success;
}

TCC_Error_Code libtcc_write_tcc_string_to_file(const char *filename, const wchar_t *tccString, int count) {
    FILE *fp;
    int size;
    char *utf8String;
    unsigned char BOM[3];
    fp = fopen(filename, "wb");
    if (!fp) return tcc_error_file_cant_create;
    BOM[0] = 0xEF;
    BOM[1] = 0xBB;
    BOM[2] = 0xBF;
    if (fwrite(BOM, sizeof(unsigned char), 3, fp) != 3) return tcc_error_file_while_writing;
    setlocale(LC_CTYPE, "");
    size = wcstombs(NULL, tccString, 0) + 1;
    utf8String = (char *)malloc(size * sizeof(char));
    wcstombs(utf8String, tccString, size);
    if (fwrite(utf8String, sizeof(char), size, fp) != size) {
        free(utf8String);
        fclose(fp);
        return tcc_error_file_while_writing;
    }
    free(utf8String);
    fclose(fp);
    return tcc_error_success;
}
#endif  /* WIN32 */

TCC_Error_Code libtcc_write_unicode_to_file(const char *filename, const wchar_t *buffer, int count) {
    FILE *fp;
    wchar_t bom[1];
    fp = fopen(filename, "wb");
    if (!fp) return tcc_error_file_cant_create;
    bom[0] = 0xFEFF;
    if (buffer[0] != bom[0]) {
        if (fwrite(bom, sizeof(wchar_t), 1, fp) != 1) {
            fclose(fp);
            return tcc_error_file_while_writing;
        }
    }
    if (fwrite(buffer, sizeof(wchar_t), count, fp) != count) {
        fclose(fp);
        return tcc_error_file_while_writing;
    }
    fclose(fp);
    return tcc_error_success;
}

TCC_Error_Code libtcc_check_format(const wchar_t *tccString, int count) {
    int i, flag, quote;
    flag = 0;
    quote = 0;
    for (i = 0; i < count; i ++) {
        if (L'\"' == tccString[i]) {
            if (1 == flag) return tcc_error_file_bad_format;
            else if (0 != flag) quote = (0 == quote) ? 1 : 0;
        } else if (L'<' == tccString[i]) {
            if (1 == quote) return tcc_error_file_bad_format;
            else if (0 == flag) flag = 1;
            else return tcc_error_file_bad_format;
        } else if (L'=' == tccString[i]) {
            if (0 == flag || 1 == quote) continue;
            else if (1 == flag) flag = 2;
            else return tcc_error_file_bad_format;
        } else if (L'>' == tccString[i]) {
            if (1 == quote) return tcc_error_file_bad_format;
            else if (2 == flag) flag = 0;
            else return tcc_error_file_bad_format;
        } else if (L'\n' == tccString[i] || L'\r' == tccString[i]) {
            if (0 != flag && 1 != quote) return tcc_error_file_bad_format;
        }
    }
    if (0 != flag || 0 != quote) return tcc_error_file_bad_format;
    return tcc_error_success;
}

/* function libtcc_parse_tcc_string() */
static int _map_compare_string(const String *s1, const String *s2) {
    return string_ignore_case_compare(s1, s2->buffer, s2->count);
}

#define _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN {\
        map_destroy(attrMap);\
        string_clear(&attrValue);\
        libtcc_free_attributes(pAttributes);\
        wprintf(L"TCC Error: can not find the TCC attribute - `%s'\n", string_get_buffer(&attrName));\
        string_clear(&attrName);\
        return tcc_error_file_invalid;\
    }

#define _LIBTCC_PARSE_TCC_STRING_ERROR_MSG2_RETURN {\
        map_destroy(attrMap);\
        string_clear(&attrValue);\
        libtcc_free_attributes(pAttributes);\
        wprintf(L"TCC Error: number of TCC attribute - `%s' can not be less than `%s'\n", string_get_buffer(&attrName), PY_FILE);\
        string_clear(&attrName);\
        return tcc_error_file_invalid;\
    }

TCC_Error_Code libtcc_parse_tcc_string(const wchar_t *tccString, int count, TCC_pAttributes pAttributes) {
    MapPtr attrMap;
    String attrName, attrValue;
    int i, len, indexBegin1, indexEnd1, indexBegin2, indexEnd2, flag, quote;
    memset(pAttributes, 0, sizeof(TCC_Attributes));
    string_init(&attrName, 0);
    string_init(&attrValue, 0);
    map_create(&attrMap, sizeof(String), sizeof(String), 0, (map_key_copy_func)string_copy, (map_value_copy_func)string_copy, (map_key_cleanup_func)string_clear, (map_value_cleanup_func)string_clear, (map_key_compare_func)_map_compare_string, (map_value_compare_func)_map_compare_string);
    flag = 0;
    quote = 0;
    for (i = 0; i < count; i ++) {
        if (L'\"' == tccString[i]) {
            if (0 != flag) quote = (0 == quote) ? 1 : 0;
        } else if (L'<' == tccString[i]) {
            flag = 1;
            indexBegin1 = i + 1;
        } else if (1 == flag && 0 == quote && L'=' == tccString[i]) {
            flag = 2;
            indexEnd1 = i;
            indexBegin2 = i + 1;
        } else if (L'>' == tccString[i]) {
            flag = 0;
            indexEnd2 = i;
            string_assign(&attrName, tccString + indexBegin1, indexEnd1 - indexBegin1);
            string_merge_all_white_spaces(&attrName);
            string_assign(&attrValue, tccString + indexBegin2, indexEnd2 - indexBegin2);
            string_discard_begin_end_white_spaces_and_quote(&attrValue);
            map_push_back(attrMap, &attrName, &attrValue);
        }
    }
    /* Convert attrMap to TCC_Attributes */
    /* tcc file version */
    string_assign(&attrName, TCC_FILE_VERSION, wcslen(TCC_FILE_VERSION));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->tcc_file_version = wcstod(string_get_buffer(&attrValue), NULL);
    /* mode configuration */
    string_assign(&attrName, INHERIT_ASS_HEADER, wcslen(INHERIT_ASS_HEADER));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->inherit_ass_header = (string_ignore_case_compare(&attrValue, L"true", 4) == 0) ? 1 : 0;
    string_assign(&attrName, K_MODE, wcslen(K_MODE));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    if (string_ignore_case_compare(&attrValue, L"word", 4) == 0 || string_ignore_case_compare(&attrValue, L"normal", 6) == 0)
        pAttributes->k_mode = K_MODE_WORD;
    else if (string_ignore_case_compare(&attrValue, L"syl", 3) == 0 || string_ignore_case_compare(&attrValue, L"syllable", 8) == 0)
        pAttributes->k_mode = K_MODE_SYL;
    else if (string_ignore_case_compare(&attrValue, L"adv", 3) == 0 || string_ignore_case_compare(&attrValue, L"advanced", 8) == 0)
        pAttributes->k_mode = K_MODE_ADV;
    else
        pAttributes->k_mode = K_MODE_SYL;
    string_assign(&attrName, TEXT_LAYOUT, wcslen(TEXT_LAYOUT));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    if (string_ignore_case_compare(&attrValue, L"hori", 4) == 0 || string_ignore_case_compare(&attrValue, L"horizontal", 10) == 0)
        pAttributes->text_layout = TEXT_LAYOUT_HORI;
    else if (string_ignore_case_compare(&attrValue, L"vert", 4) == 0 || string_ignore_case_compare(&attrValue, L"vertical", 8) == 0)
        pAttributes->text_layout = TEXT_LAYOUT_VERT;
    else
        pAttributes->text_layout = TEXT_LAYOUT_HORI;
    string_assign(&attrName, TCAXPY_INIT, wcslen(TCAXPY_INIT));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->tcaxpy_init = (string_ignore_case_compare(&attrValue, L"true", 4) == 0) ? 1 : 0;
    string_assign(&attrName, TCAXPY_USER, wcslen(TCAXPY_USER));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->tcaxpy_user = (string_ignore_case_compare(&attrValue, L"true", 4) == 0) ? 1 : 0;
    string_assign(&attrName, TCAXPY_FIN, wcslen(TCAXPY_FIN));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->tcaxpy_fin = (string_ignore_case_compare(&attrValue, L"true", 4) == 0) ? 1 : 0;
    /* py settings */
    /* get py file count */
    pAttributes->py_file_count = 0;
    string_assign(&attrName, PY_FILE, wcslen(PY_FILE));
    while (map_retrieve(attrMap, &attrName, &attrValue)) {
        map_increase_offset(attrMap);
        pAttributes->py_file_count++;
    }
    if (0 == pAttributes->py_file_count) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    map_reset_offset(attrMap);
    /* get py file name */
    pAttributes->py_file = (wchar_t **)malloc(pAttributes->py_file_count * sizeof(wchar_t *));
    i = 0;
    while (map_retrieve(attrMap, &attrName, &attrValue)) {
        string_copy_to_buffer(&attrValue, &pAttributes->py_file[i]);
        map_increase_offset(attrMap);
        i++;
    }
    map_reset_offset(attrMap);
    /* get beginning line id */
    string_assign(&attrName, BEG_LINE, wcslen(BEG_LINE));
    pAttributes->beg_line = (int *)malloc(pAttributes->py_file_count * sizeof(int));
    i = 0;
    while (i < pAttributes->py_file_count && map_retrieve(attrMap, &attrName, &attrValue)) {
        pAttributes->beg_line[i] = wcstol(string_get_buffer(&attrValue), NULL, 10);
        map_increase_offset(attrMap);
        i++;
    }
    if (i != pAttributes->py_file_count) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG2_RETURN;
    map_reset_offset(attrMap);
    /* get end line id */
    string_assign(&attrName, END_LINE, wcslen(END_LINE));
    pAttributes->end_line = (int *)malloc(pAttributes->py_file_count * sizeof(int));
    i = 0;
    while (i < pAttributes->py_file_count && map_retrieve(attrMap, &attrName, &attrValue)) {
        pAttributes->end_line[i] = wcstol(string_get_buffer(&attrValue), NULL, 10);
        map_increase_offset(attrMap);
        i++;
    }
    if (i != pAttributes->py_file_count) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG2_RETURN;
    map_reset_offset(attrMap);
    /* main settings */
    string_assign(&attrName, K_TIMED_ASS_FILE, wcslen(K_TIMED_ASS_FILE));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    string_copy_to_buffer(&attrValue, &pAttributes->k_timed_ass_file);
    string_assign(&attrName, FONT_FILE, wcslen(FONT_FILE));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    string_copy_to_buffer(&attrValue, &pAttributes->font_file);
    string_assign(&attrName, FONT_FACE_ID, wcslen(FONT_FACE_ID));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->font_face_id = wcstol(string_get_buffer(&attrValue), NULL, 10);
    string_assign(&attrName, FONT_SIZE, wcslen(FONT_SIZE));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->font_size = wcstol(string_get_buffer(&attrValue), NULL, 10);
    string_assign(&attrName, FX_WIDTH, wcslen(FX_WIDTH));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->fx_width = wcstol(string_get_buffer(&attrValue), NULL, 10);
    string_assign(&attrName, FX_HEIGHT, wcslen(FX_HEIGHT));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->fx_height = wcstol(string_get_buffer(&attrValue), NULL, 10);
    string_assign(&attrName, FX_FPS, wcslen(FX_FPS));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->fx_fps = wcstod(string_get_buffer(&attrValue), NULL);
    string_assign(&attrName, ALIGNMENT, wcslen(ALIGNMENT));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->alignment = wcstol(string_get_buffer(&attrValue), NULL, 10);
    string_assign(&attrName, X_OFFSET, wcslen(X_OFFSET));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->x_offset = wcstol(string_get_buffer(&attrValue), NULL, 10);
    string_assign(&attrName, Y_OFFSET, wcslen(Y_OFFSET));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->y_offset = wcstol(string_get_buffer(&attrValue), NULL, 10);
    string_assign(&attrName, SPACING, wcslen(SPACING));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->spacing = wcstol(string_get_buffer(&attrValue), NULL, 10);
    string_assign(&attrName, SPACE_SCALE, wcslen(SPACE_SCALE));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->space_scale = wcstod(string_get_buffer(&attrValue), NULL);
    /* style settings */
    string_assign(&attrName, FONT_FACE_NAME, wcslen(FONT_FACE_NAME));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    string_copy_to_buffer(&attrValue, &pAttributes->font_face_name);
    string_assign(&attrName, BORD, wcslen(BORD));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->bord = wcstol(string_get_buffer(&attrValue), NULL, 10);
    string_assign(&attrName, SHAD, wcslen(SHAD));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->shad = wcstol(string_get_buffer(&attrValue), NULL, 10);
    string_assign(&attrName, PRIMARY_COLOR, wcslen(PRIMARY_COLOR));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    len = __min(6, string_get_size(&attrValue));
    memcpy(pAttributes->primary_color, string_get_buffer(&attrValue), len * sizeof(wchar_t));
    pAttributes->primary_color[len] = L'\0';
    string_assign(&attrName, SECONDARY_COLOR, wcslen(SECONDARY_COLOR));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    len = __min(6, string_get_size(&attrValue));
    memcpy(pAttributes->secondary_color, string_get_buffer(&attrValue), len * sizeof(wchar_t));
    pAttributes->secondary_color[len] = L'\0';
    string_assign(&attrName, OUTLINE_COLOR, wcslen(OUTLINE_COLOR));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    len = __min(6, string_get_size(&attrValue));
    memcpy(pAttributes->outline_color, string_get_buffer(&attrValue), len * sizeof(wchar_t));
    pAttributes->outline_color[len] = L'\0';
    string_assign(&attrName, BACK_COLOR, wcslen(BACK_COLOR));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    len = __min(6, string_get_size(&attrValue));
    memcpy(pAttributes->back_color, string_get_buffer(&attrValue), len * sizeof(wchar_t));
    pAttributes->back_color[len] = L'\0';
    string_assign(&attrName, PRIMARY_ALPHA, wcslen(PRIMARY_ALPHA));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->primary_alpha = wcstol(string_get_buffer(&attrValue), NULL, 10);
    string_assign(&attrName, SECONDARY_ALPHA, wcslen(SECONDARY_ALPHA));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->secondary_alpha = wcstol(string_get_buffer(&attrValue), NULL, 10);
    string_assign(&attrName, OUTLINE_ALPHA, wcslen(OUTLINE_ALPHA));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->outline_alpha = wcstol(string_get_buffer(&attrValue), NULL, 10);
    string_assign(&attrName, BACK_ALPHA, wcslen(BACK_ALPHA));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->back_alpha = wcstol(string_get_buffer(&attrValue), NULL, 10);
    /* additional settings */
    string_assign(&attrName, BLUR, wcslen(BLUR));
    if (map_retrieve(attrMap, &attrName, &attrValue) == NULL) _LIBTCC_PARSE_TCC_STRING_ERROR_MSG_RETURN;
    pAttributes->blur = wcstod(string_get_buffer(&attrValue), NULL);
    map_destroy(attrMap);
    string_clear(&attrName);
    string_clear(&attrValue);
    return tcc_error_success;
}

/* new version of parsing */
#define _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN {\
        libmap_destroy(&map);\
        libtcc_free_attributes(pAttributes);\
        wprintf(L"TCC Error: can not find the TCC attribute - `%s'\n", key);\
        return tcc_error_file_invalid;\
    }

#define _LIBTCC_PARSE_TCC_FILE_ERROR_MSG2_RETURN {\
        libmap_destroy(&map);\
        libtcc_free_attributes(pAttributes);\
        wprintf(L"TCC Error: number of TCC attribute - `%s' can not be less than `%s'\n", key, PY_FILE);\
        return tcc_error_file_invalid;\
    }

TCC_Error_Code libtcc_parse_tcc_file(const char *filename, TCC_pAttributes pAttributes) {
    int i, len;
    wchar_t *key;
    wchar_t *value;
    StrMap map;
    memset(pAttributes, 0, sizeof(TCC_Attributes));
    if (libmap_read_map_file(filename, &map) != 0)
        return tcc_error_file_cant_open;
    libmap_parse(&map);
    libmap_polish(&map);
    /* Convert attrMap to TCC_Attributes */
    /* tcc file version */
    key = TCC_FILE_VERSION;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->tcc_file_version = wcstod(value, NULL);
    /* mode configuration */
    key = INHERIT_ASS_HEADER;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->inherit_ass_header = (__wcs_ignore_case_cmp(value, L"true") == 0) ? 1 : 0;
    key = K_MODE;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    if (__wcs_ignore_case_cmp(value, L"word") == 0 || __wcs_ignore_case_cmp(value, L"normal") == 0)
        pAttributes->k_mode = K_MODE_WORD;
    else if (__wcs_ignore_case_cmp(value, L"syl") == 0 || __wcs_ignore_case_cmp(value, L"syllable") == 0)
        pAttributes->k_mode = K_MODE_SYL;
    else if (__wcs_ignore_case_cmp(value, L"adv") == 0 || __wcs_ignore_case_cmp(value, L"advanced") == 0)
        pAttributes->k_mode = K_MODE_ADV;
    else
        pAttributes->k_mode = K_MODE_SYL;
    //
    key = TEXT_LAYOUT;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    if (__wcs_ignore_case_cmp(value, L"hori") == 0 || __wcs_ignore_case_cmp(value, L"horizontal") == 0)
        pAttributes->text_layout = TEXT_LAYOUT_HORI;
    else if (__wcs_ignore_case_cmp(value, L"vert") == 0 || __wcs_ignore_case_cmp(value, L"vertical") == 0)
        pAttributes->text_layout = TEXT_LAYOUT_VERT;
    else
        pAttributes->text_layout = TEXT_LAYOUT_HORI;
    //
    key = TCAXPY_INIT;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->tcaxpy_init = (__wcs_ignore_case_cmp(value, L"true") == 0) ? 1 : 0;
    //
    key = TCAXPY_USER;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->tcaxpy_user = (__wcs_ignore_case_cmp(value, L"true") == 0) ? 1 : 0;
    //
    key = TCAXPY_FIN;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->tcaxpy_fin = (__wcs_ignore_case_cmp(value, L"true") == 0) ? 1 : 0;
    //
    /* py settings */
    /* get py file count */
    pAttributes->py_file_count = 0;
    key = PY_FILE;
    while (tcc_map_retrieve(map.map, &key, &value)) {
        tcc_map_increase_offset(map.map);
        pAttributes->py_file_count++;
    }
    if (0 == pAttributes->py_file_count) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    tcc_map_reset_offset(map.map);
    /* get py file name */
    pAttributes->py_file = (wchar_t **)malloc(pAttributes->py_file_count * sizeof(wchar_t *));
    i = 0;
    while (tcc_map_retrieve(map.map, &key, &value)) {
        len = wcslen(value) + 1;
        pAttributes->py_file[i] = (wchar_t *)malloc(len * sizeof(wchar_t));
        memcpy(pAttributes->py_file[i], value, len * sizeof(wchar_t));
        tcc_map_increase_offset(map.map);
        i++;
    }
    tcc_map_reset_offset(map.map);
    /* get beginning line id */
    key = BEG_LINE;
    pAttributes->beg_line = (int *)malloc(pAttributes->py_file_count * sizeof(int));
    i = 0;
    while (i < pAttributes->py_file_count && tcc_map_retrieve(map.map, &key, &value)) {
        pAttributes->beg_line[i] = wcstol(value, NULL, 10);
        tcc_map_increase_offset(map.map);
        i++;
    }
    if (i != pAttributes->py_file_count) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG2_RETURN;
    tcc_map_reset_offset(map.map);
    /* get end line id */
    key = END_LINE;
    pAttributes->end_line = (int *)malloc(pAttributes->py_file_count * sizeof(int));
    i = 0;
    while (i < pAttributes->py_file_count && tcc_map_retrieve(map.map, &key, &value)) {
        pAttributes->end_line[i] = wcstol(value, NULL, 10);
        tcc_map_increase_offset(map.map);
        i++;
    }
    if (i != pAttributes->py_file_count) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG2_RETURN;
    tcc_map_reset_offset(map.map);
    /* main settings */
    key = K_TIMED_ASS_FILE;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    len = wcslen(value) + 1;
    pAttributes->k_timed_ass_file = (wchar_t *)malloc(len * sizeof(wchar_t));
    memcpy(pAttributes->k_timed_ass_file, value, len * sizeof(wchar_t));
    //
    key = FONT_FILE;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    len = wcslen(value) + 1;
    pAttributes->font_file = (wchar_t *)malloc(len * sizeof(wchar_t));
    memcpy(pAttributes->font_file, value, len * sizeof(wchar_t));
    //
    key = FONT_FACE_ID;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->font_face_id = wcstol(value, NULL, 10);
    //
    key = FONT_SIZE;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->font_size = wcstol(value, NULL, 10);
    //
    key = FX_WIDTH;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->fx_width = wcstol(value, NULL, 10);
    //
    key = FX_HEIGHT;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->fx_height = wcstol(value, NULL, 10);
    //
    key = FX_FPS;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->fx_fps = wcstod(value, NULL);
    //
    key = ALIGNMENT;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->alignment = wcstol(value, NULL, 10);
    //
    key = X_OFFSET;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->x_offset = wcstol(value, NULL, 10);
    //
    key = Y_OFFSET;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->y_offset = wcstol(value, NULL, 10);
    //
    key = SPACING;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->spacing = wcstol(value, NULL, 10);
    //
    key = SPACE_SCALE;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->space_scale = wcstod(value, NULL);
    //
    /* style settings */
    key = FONT_FACE_NAME;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    len = wcslen(value) + 1;
    pAttributes->font_face_name = (wchar_t *)malloc(len * sizeof(wchar_t));
    memcpy(pAttributes->font_face_name, value, len * sizeof(wchar_t));
    //
    key = BORD;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->bord = wcstol(value, NULL, 10);
    //
    key = SHAD;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->shad = wcstol(value, NULL, 10);
    //
    key = PRIMARY_COLOR;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    len = __min(6, wcslen(value));
    memcpy(pAttributes->primary_color, value, len * sizeof(wchar_t));
    pAttributes->primary_color[len] = L'\0';
    //
    key = SECONDARY_COLOR;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    len = __min(6, wcslen(value));
    memcpy(pAttributes->secondary_color, value, len * sizeof(wchar_t));
    pAttributes->secondary_color[len] = L'\0';
    //
    key = OUTLINE_COLOR;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    len = __min(6, wcslen(value));
    memcpy(pAttributes->outline_color, value, len * sizeof(wchar_t));
    pAttributes->outline_color[len] = L'\0';
    //
    key = BACK_COLOR;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    len = __min(6, wcslen(value));
    memcpy(pAttributes->back_color, value, len * sizeof(wchar_t));
    pAttributes->back_color[len] = L'\0';
    //
    key = PRIMARY_ALPHA;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->primary_alpha = wcstol(value, NULL, 10);
    //
    key = SECONDARY_ALPHA;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->secondary_alpha = wcstol(value, NULL, 10);
    //
    key = OUTLINE_ALPHA;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->outline_alpha = wcstol(value, NULL, 10);
    //
    key = BACK_ALPHA;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->back_alpha = wcstol(value, NULL, 10);
    /* additional settings */
    key = BLUR;
    if (tcc_map_retrieve(map.map, &key, &value) == NULL) _LIBTCC_PARSE_TCC_FILE_ERROR_MSG_RETURN;
    pAttributes->blur = wcstod(value, NULL);
    //
    libmap_destroy(&map);
    return tcc_error_success;
}

void libtcc_info_to_tcax(const TCC_pAttributes pAttributes, TCC_pInfoToTcax pInfoToTcax) {
    int count;
    count = wcslen(pAttributes->font_file) + 1;
    pInfoToTcax->font_file = (wchar_t *)malloc(count * sizeof(wchar_t));
    memcpy(pInfoToTcax->font_file, pAttributes->font_file, count * sizeof(wchar_t));
    pInfoToTcax->font_face_id = pAttributes->font_face_id;
    pInfoToTcax->font_size = pAttributes->font_size;
    pInfoToTcax->fx_width = pAttributes->fx_width;
    pInfoToTcax->fx_height = pAttributes->fx_height;
    pInfoToTcax->fx_fps = pAttributes->fx_fps;
    pInfoToTcax->alignment = pAttributes->alignment;
    pInfoToTcax->x_offset = pAttributes->x_offset;
    pInfoToTcax->y_offset = pAttributes->y_offset;
    pInfoToTcax->spacing = pAttributes->spacing;
    pInfoToTcax->space_scale = pAttributes->space_scale;
    count = wcslen(pAttributes->font_face_name) + 1;
    pInfoToTcax->font_face_name = (wchar_t *)malloc(count * sizeof(wchar_t));
    memcpy(pInfoToTcax->font_face_name, pAttributes->font_face_name, count * sizeof(wchar_t));
    pInfoToTcax->bord = pAttributes->bord;
    pInfoToTcax->shad = pAttributes->shad;
    memcpy(pInfoToTcax->primary_color, pAttributes->primary_color, 7 * sizeof(wchar_t));
    memcpy(pInfoToTcax->secondary_color, pAttributes->secondary_color, 7 * sizeof(wchar_t));
    memcpy(pInfoToTcax->outline_color, pAttributes->outline_color, 7 * sizeof(wchar_t));
    memcpy(pInfoToTcax->back_color, pAttributes->back_color, 7 * sizeof(wchar_t));
    pInfoToTcax->primary_alpha = pAttributes->primary_alpha;
    pInfoToTcax->secondary_alpha = pAttributes->secondary_alpha;
    pInfoToTcax->outline_alpha = pAttributes->outline_alpha;
    pInfoToTcax->back_alpha = pAttributes->back_alpha;
    pInfoToTcax->blur = pAttributes->blur;
}

void libtcc_free_attributes(TCC_pAttributes pAttributes) {
    int i;
    for (i = 0; i < pAttributes->py_file_count; i++)
        free(pAttributes->py_file[i]);
    free(pAttributes->py_file);
    free(pAttributes->beg_line);
    free(pAttributes->end_line);
    free(pAttributes->k_timed_ass_file);
    free(pAttributes->font_file);
    free(pAttributes->font_face_name);
}

void libtcc_free_info(TCC_pInfoToTcax pInfoToTcax) {
    free(pInfoToTcax->font_file);
    free(pInfoToTcax->font_face_name);
}

static void _convert_attribute_to_string(String *s, const String *name, const String *value) {
    string_clear(s);
    string_append(s, L"< ", 2);
    string_append(s, string_get_buffer(name), string_get_size(name));
    string_append(s, L" = ", 3);
    string_append(s, string_get_buffer(value), string_get_size(value));
    string_append(s, L" >", 2);
}

void libtcc_convert_attributes_to_tcc_string(const TCC_pAttributes pAttributes, wchar_t **pTccString, int *pCount) {
    int i;
    String tccString, attrName, attrValue, attrString;
    string_init(&tccString, 0);
    string_init(&attrName, 0);
    string_init(&attrValue, 0);
    string_init(&attrString, 0);
    /* tcc file version */
    string_append(&tccString, L"# tcc file version\r\n\r\n", 22);
    string_assign(&attrName, TCC_FILE_VERSION, wcslen(TCC_FILE_VERSION));
    string_assign_from_double(&attrValue, pAttributes->tcc_file_version);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    /* mode configuration */
    string_append(&tccString, L"\r\n\r\n# mode configuration\r\n\r\n", 28);
    string_assign(&attrName, INHERIT_ASS_HEADER, wcslen(INHERIT_ASS_HEADER));
    if (1 == pAttributes->inherit_ass_header)
        string_assign(&attrValue, L"true", 4);
    else
        string_assign(&attrValue, L"false", 5);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    string_assign(&attrName, K_MODE, wcslen(K_MODE));
    if (K_MODE_WORD == pAttributes->k_mode)
        string_assign(&attrValue, L"normal", 6);
    else if (K_MODE_SYL == pAttributes->k_mode)
        string_assign(&attrValue, L"syllable", 8);
    else if (K_MODE_ADV == pAttributes->k_mode)
        string_assign(&attrValue, L"advanced", 8);
    else
        string_assign(&attrValue, L"syllable", 8);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    string_assign(&attrName, TEXT_LAYOUT, wcslen(TEXT_LAYOUT));
    if (TEXT_LAYOUT_HORI == pAttributes->text_layout)
        string_assign(&attrValue, L"horizontal", 10);
    else if (TEXT_LAYOUT_VERT == pAttributes->text_layout)
        string_assign(&attrValue, L"vertical", 8);
    else
        string_assign(&attrValue, L"horizontal", 10);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    string_assign(&attrName, TCAXPY_INIT, wcslen(TCAXPY_INIT));
    if (1 == pAttributes->tcaxpy_init)
        string_assign(&attrValue, L"true", 4);
    else
        string_assign(&attrValue, L"false", 5);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    string_assign(&attrName, TCAXPY_USER, wcslen(TCAXPY_USER));
    if (1 == pAttributes->tcaxpy_user)
        string_assign(&attrValue, L"true", 4);
    else
        string_assign(&attrValue, L"false", 5);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    string_assign(&attrName, TCAXPY_FIN, wcslen(TCAXPY_FIN));
    if (1 == pAttributes->tcaxpy_fin)
        string_assign(&attrValue, L"true", 4);
    else
        string_assign(&attrValue, L"false", 5);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    /* py settings */
    string_append(&tccString, L"\r\n\r\n# py settings\r\n\r\n", 21);
    for (i = 0; i < pAttributes->py_file_count; i ++) {
        string_assign(&attrName, PY_FILE, wcslen(PY_FILE));
        string_assign(&attrValue, pAttributes->py_file[i], wcslen(pAttributes->py_file[i]));
        _convert_attribute_to_string(&attrString, &attrName, &attrValue);
        string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
        string_append(&tccString, L"    ", 4);
        string_assign(&attrName, BEG_LINE, wcslen(BEG_LINE));
        string_assign_from_int(&attrValue, pAttributes->beg_line[i]);
        _convert_attribute_to_string(&attrString, &attrName, &attrValue);
        string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
        string_append(&tccString, L"    ", 4);
        string_assign(&attrName, END_LINE, wcslen(END_LINE));
        string_assign_from_int(&attrValue, pAttributes->end_line[i]);
        _convert_attribute_to_string(&attrString, &attrName, &attrValue);
        string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
        string_append(&tccString, L"\r\n", 2);
    }
    /* main settings */
    string_append(&tccString, L"\r\n\r\n# main settings\r\n\r\n", 23);
    string_assign(&attrName, K_TIMED_ASS_FILE, wcslen(K_TIMED_ASS_FILE));
    string_assign(&attrValue, pAttributes->k_timed_ass_file, wcslen(pAttributes->k_timed_ass_file));
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    string_assign(&attrName, FONT_FILE, wcslen(FONT_FILE));
    string_assign(&attrValue, pAttributes->font_file, wcslen(pAttributes->font_file));
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"    ", 4);
    string_assign(&attrName, FONT_FACE_ID, wcslen(FONT_FACE_ID));
    string_assign_from_int(&attrValue, pAttributes->font_face_id);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"    ", 4);
    string_assign(&attrName, FONT_SIZE, wcslen(FONT_SIZE));
    string_assign_from_int(&attrValue, pAttributes->font_size);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    string_assign(&attrName, FX_WIDTH, wcslen(FX_WIDTH));
    string_assign_from_int(&attrValue, pAttributes->fx_width);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"    ", 4);
    string_assign(&attrName, FX_HEIGHT, wcslen(FX_HEIGHT));
    string_assign_from_int(&attrValue, pAttributes->fx_height);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"    ", 4);
    string_assign(&attrName, FX_FPS, wcslen(FX_FPS));
    string_assign_from_double(&attrValue, pAttributes->fx_fps);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    string_assign(&attrName, ALIGNMENT, wcslen(ALIGNMENT));
    string_assign_from_int(&attrValue, pAttributes->alignment);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    string_assign(&attrName, X_OFFSET, wcslen(X_OFFSET));
    string_assign_from_int(&attrValue, pAttributes->x_offset);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"    ", 4);
    string_assign(&attrName, Y_OFFSET, wcslen(Y_OFFSET));
    string_assign_from_int(&attrValue, pAttributes->y_offset);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    string_assign(&attrName, SPACING, wcslen(SPACING));
    string_assign_from_int(&attrValue, pAttributes->spacing);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"    ", 4);
    string_assign(&attrName, SPACE_SCALE, wcslen(SPACE_SCALE));
    string_assign_from_double(&attrValue, pAttributes->space_scale);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    /* style settings */
    string_append(&tccString, L"\r\n\r\n# style settings\r\n\r\n", 24);
    string_assign(&attrName, FONT_FACE_NAME, wcslen(FONT_FACE_NAME));
    if (L'\0' == pAttributes->font_face_name[0])
        string_assign(&attrValue, L"\"\"", 2);
    else
        string_assign(&attrValue, pAttributes->font_face_name, wcslen(pAttributes->font_face_name));
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    string_assign(&attrName, BORD, wcslen(BORD));
    string_assign_from_int(&attrValue, pAttributes->bord);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"    ", 4);
    string_assign(&attrName, SHAD, wcslen(SHAD));
    string_assign_from_int(&attrValue, pAttributes->shad);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    string_assign(&attrName, PRIMARY_COLOR, wcslen(PRIMARY_COLOR));
    string_assign(&attrValue, pAttributes->primary_color, wcslen(pAttributes->primary_color));
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"    ", 4);
    string_assign(&attrName, SECONDARY_COLOR, wcslen(SECONDARY_COLOR));
    string_assign(&attrValue, pAttributes->secondary_color, wcslen(pAttributes->secondary_color));
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"    ", 4);
    string_assign(&attrName, OUTLINE_COLOR, wcslen(OUTLINE_COLOR));
    string_assign(&attrValue, pAttributes->outline_color, wcslen(pAttributes->outline_color));
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"    ", 4);
    string_assign(&attrName, BACK_COLOR, wcslen(BACK_COLOR));
    string_assign(&attrValue, pAttributes->back_color, wcslen(pAttributes->back_color));
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    string_assign(&attrName, PRIMARY_ALPHA, wcslen(PRIMARY_ALPHA));
    string_assign_from_int(&attrValue, pAttributes->primary_alpha);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"    ", 4);
    string_assign(&attrName, SECONDARY_ALPHA, wcslen(SECONDARY_ALPHA));
    string_assign_from_int(&attrValue, pAttributes->secondary_alpha);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"    ", 4);
    string_assign(&attrName, OUTLINE_ALPHA, wcslen(OUTLINE_ALPHA));
    string_assign_from_int(&attrValue, pAttributes->outline_alpha);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"    ", 4);
    string_assign(&attrName, BACK_ALPHA, wcslen(BACK_ALPHA));
    string_assign_from_int(&attrValue, pAttributes->back_alpha);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    /* additional settings */
    string_append(&tccString, L"\r\n\r\n# additional settings\r\n\r\n", 29);
    string_assign(&attrName, BLUR, wcslen(BLUR));
    string_assign_from_double(&attrValue, pAttributes->blur);
    _convert_attribute_to_string(&attrString, &attrName, &attrValue);
    string_append(&tccString, string_get_buffer(&attrString), string_get_size(&attrString));
    string_append(&tccString, L"\r\n", 2);
    /* end */
    string_append(&tccString, L"\r\n\r\n\r\n\r\n\r\n", 10);
    string_copy_to_buffer(&tccString, pTccString);
    *pCount = string_get_size(&tccString);
    string_clear(&attrString);
    string_clear(&attrValue);
    string_clear(&attrName);
    string_clear(&tccString);
}

static void _libtcc_convert_decimal_to_hex(int d, wchar_t **pHex, int *count) {
    int flag;
    String str;
    string_init(&str, 0);
    while (d != 0) {
        flag = d % 16;
        switch (flag) {
        case 10:
            string_append(&str, L"A", 1);
            break;
        case 11:
            string_append(&str, L"B", 1);
            break;
        case 12:
            string_append(&str, L"C", 1);
            break;
        case 13:
            string_append(&str, L"D", 1);
            break;
        case 14:
            string_append(&str, L"E", 1);
            break;
        case 15:
            string_append(&str, L"F", 1);
            break;
        default:
            string_append_int(&str, flag);
            break;
        }
        d /= 16;
    }
    string_reverse(&str);
    string_copy_to_buffer(&str, pHex);
    *count = string_get_size(&str);
    string_clear(&str);
}

static const wchar_t *_dec_to_hex_truncated(int d, wchar_t *hex) {
    if (!hex) return hex;
    if (d <= 0) {
        hex[0] = L'0';
        hex[1] = L'0';
    } else if (d >= 255) {
        hex[0] = L'F';
        hex[1] = L'F';
    } else {
        wchar_t *str;
        int count;
        _libtcc_convert_decimal_to_hex(d, &str, &count);
        if (count < 2) {
            hex[0] = L'0';
            hex[1] = str[0];
        } else {
            hex[0] = str[1];  /* when writing to the file, the higher byte should be writen first */
            hex[1] = str[0];
        }
        free(str);
    }
    hex[2] = L'\0';
    return (const wchar_t *)hex;
}

void libtcc_make_ass_header(const TCC_pAttributes pAttributes, wchar_t **pAssHeader, int *pCount) {
    wchar_t sz[3];
    String header;
    string_init(&header, 0);
    string_append(&header, L"[Script Info]\r\n", 15);
    string_append(&header, L"; This script is generated by TCAX 1.0.0 or later versions\r\n", 60);
    string_append(&header, L"; Welcome to TCAX forum http://tcax.rhacg.com\r\n", 47);
    string_append(&header, L"ScriptType: v4.00+\r\n", 20);
    string_append(&header, L"Collisions:Normal\r\n", 19);
    string_append(&header, L"PlayResX:", 9);
    string_append_int(&header, pAttributes->fx_width);
    string_append(&header, L"\r\n", 2);
    string_append(&header, L"PlayResY:", 9);
    string_append_int(&header, pAttributes->fx_height);
    string_append(&header, L"\r\n", 2);
    string_append(&header, L"Timer:100.0000\r\n\r\n", 18);
    string_append(&header, L"[V4+ Styles]\r\n", 14);
    string_append(&header, L"Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\r\n", 239);
    string_append(&header, L"Style: TCMS,", 12);
    string_append(&header, pAttributes->font_face_name, wcslen(pAttributes->font_face_name));
    string_append(&header, L",", 1);
    string_append_int(&header, pAttributes->font_size);
    string_append(&header, L",&H", 3);
    string_append(&header, _dec_to_hex_truncated(pAttributes->primary_alpha, sz), 2);
    string_append(&header, pAttributes->primary_color, wcslen(pAttributes->primary_color));
    string_append(&header, L",&H", 3);
    string_append(&header, _dec_to_hex_truncated(pAttributes->secondary_alpha, sz), 2);
    string_append(&header, pAttributes->secondary_color, wcslen(pAttributes->secondary_color));
    string_append(&header, L",&H", 3);
    string_append(&header, _dec_to_hex_truncated(pAttributes->outline_alpha, sz), 2);
    string_append(&header, pAttributes->outline_color, wcslen(pAttributes->outline_color));
    string_append(&header, L",&H", 3);
    string_append(&header, _dec_to_hex_truncated(pAttributes->back_alpha, sz), 2);
    string_append(&header, pAttributes->back_color, wcslen(pAttributes->back_color));
    string_append(&header, L",0,0,0,0,100,100,", 17);
    string_append_int(&header, pAttributes->spacing);
    string_append(&header, L",0,0,", 5);
    string_append_int(&header, pAttributes->bord);
    string_append(&header, L",", 1);
    string_append_int(&header, pAttributes->shad);
    string_append(&header, L",5,15,15,10,1\r\n", 15);
    string_append(&header, L"Style: TCPS,Arial,1,&HFFFFFFFF,&HFFFFFFFF,&HFFFFFFFF,&HFFFFFFFF,0,0,0,0,100,100,0,0,0,0,0,7,0,0,0,1\r\n\r\n", 103);
    string_append(&header, L"[Events]\r\n", 10);
    string_append(&header, L"Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n", 81);
    string_copy_to_buffer(&header, pAssHeader);
    *pCount = string_get_size(&header);
    string_clear(&header);
}

static void _libtcc_default_tcc_attributes(TCC_pAttributes pAttributes) {
    int i;
    pAttributes->tcc_file_version = 1.0;
    pAttributes->inherit_ass_header = 0;
    pAttributes->k_mode = K_MODE_SYL;
    pAttributes->text_layout = TEXT_LAYOUT_HORI;
    pAttributes->tcaxpy_init = 0;
    pAttributes->tcaxpy_user = 0;
    pAttributes->tcaxpy_fin = 0;
    pAttributes->py_file_count = 1;
    pAttributes->py_file = (wchar_t **)malloc(pAttributes->py_file_count * sizeof(wchar_t *));
    pAttributes->beg_line = (int *)malloc(pAttributes->py_file_count * sizeof(int));
    pAttributes->end_line = (int *)malloc(pAttributes->py_file_count * sizeof(int));
    for (i = 0; i < pAttributes->py_file_count; i ++) {
        pAttributes->py_file[i] = (wchar_t *)malloc(8 * sizeof(wchar_t));
        memcpy(pAttributes->py_file[i], L"test.py\0", 8 * sizeof(wchar_t));
        pAttributes->beg_line[i] = 1;
        pAttributes->end_line[i] = 0;
    }
    pAttributes->k_timed_ass_file = (wchar_t *)malloc(11 * sizeof(wchar_t));
    memcpy(pAttributes->k_timed_ass_file, L"test_k.ass\0", 11 * sizeof(wchar_t));
    pAttributes->font_file = (wchar_t *)malloc(11 * sizeof(wchar_t));
    memcpy(pAttributes->font_file, L"simhei.ttf\0", 11 * sizeof(wchar_t));
    pAttributes->font_face_id = 1;;
    pAttributes->font_size = 36;
    pAttributes->fx_width = 1280;
    pAttributes->fx_height = 720;
    pAttributes->fx_fps = 23.976;
    pAttributes->alignment = 7;
    pAttributes->x_offset = 30;
    pAttributes->y_offset = 15;
    pAttributes->spacing = 0;
    pAttributes->space_scale = 1.0;
    pAttributes->font_face_name = (wchar_t *)malloc(1 * sizeof(wchar_t));
    memcpy(pAttributes->font_face_name, L"\0", 1 * sizeof(wchar_t));
    pAttributes->bord = 1;
    pAttributes->shad = 0;
    memcpy(pAttributes->primary_color, L"FFFFFF\0", 7 * sizeof(wchar_t));
    memcpy(pAttributes->secondary_color, L"000000\0", 7 * sizeof(wchar_t));
    memcpy(pAttributes->outline_color, L"000000\0", 7 * sizeof(wchar_t));
    memcpy(pAttributes->back_color, L"000000\0", 7 * sizeof(wchar_t));
    pAttributes->primary_alpha = 0;
    pAttributes->secondary_alpha = 255;
    pAttributes->outline_alpha = 0;
    pAttributes->back_alpha = 255;
    pAttributes->blur = 0.0;
}

TCC_Error_Code libtcc_create_default_tcc_file(const char *filename) {
    TCC_Error_Code error;
    TCC_Attributes attributes;
    wchar_t *tccString;
    int count;
    _libtcc_default_tcc_attributes(&attributes);
    libtcc_convert_attributes_to_tcc_string(&attributes, &tccString, &count);
    libtcc_free_attributes(&attributes);
    error = libtcc_write_tcc_string_to_file(filename, tccString, count);
    if (tcc_error_success != error) {
        free(tccString);
        return error;
    }
    free(tccString);
    return tcc_error_success;
}

