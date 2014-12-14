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

#include "tcax.h"

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif    /* _MSC_VER */


#if defined WIN32
static void _tcax_sz_unicode_to_ansi(const wchar_t *uni, char **ansi) {
    int size;
    char *sz;
    size = WideCharToMultiByte(CP_ACP, 0, uni, -1, NULL, 0, NULL, NULL);
    sz = (char *)malloc(size * sizeof(char));
    WideCharToMultiByte(CP_ACP, 0, uni, -1, sz, size, NULL, NULL);
    *ansi = sz;
}

static void _tcax_sz_ansi_to_unicode(const char *ansi, wchar_t **uni) {
    int count;
    wchar_t *sz;
    count = MultiByteToWideChar(CP_ACP, 0, ansi, -1, NULL, 0);
    sz = (wchar_t *)malloc(count * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, ansi, -1, sz, count);
    *uni = sz;
}
#else
static void _tcax_sz_unicode_to_ansi(const wchar_t *uni, char **ansi) {
    int size;
    char *sz;
    setlocale(LC_CTYPE, "");
    size = wcstombs(NULL, uni, 0) + 1;
    sz = (char *)malloc(size * sizeof(char));
    wcstombs(sz, uni, size);
    *ansi = sz;
}

static void _tcax_sz_ansi_to_unicode(const char *ansi, wchar_t **uni) {
    int count;
    wchar_t *sz;
    setlocale(LC_CTYPE, "");
    count = mbstowcs(NULL, ansi, 0) + 1;
    sz = (wchar_t *)malloc(count * sizeof(wchar_t));
    mbstowcs(sz, ansi, count);
    *uni = sz;
}
#endif  /* WIN32 */

#if defined WIN32
const char *tcax_get_root_directory(char **pDirectory) {
    int i, len, indicator, count;
    char szDirectory[MAX_PATH];
    char *directory;
    len = GetModuleFileNameA(NULL, szDirectory, MAX_PATH);
    for (i = len - 1; i >= 0; i --) {
        if ('\\' == szDirectory[i] || '/' == szDirectory[i]) {
            indicator = i;
            break;
        }
    }
    count = indicator;
    directory = (char *)malloc((count + 1) * sizeof(char));
    memcpy(directory, szDirectory, count * sizeof(char));
    directory[count] = '\0';
    *pDirectory = directory;
    return (const char *)directory;
}
#else
const char *tcax_get_root_directory(char **pDirectory) {
    int count;
    char szDirectory[MAX_PATH];
    char *directory;
    getcwd(szDirectory, MAX_PATH);
    count = strlen(szDirectory) + 1;
    directory = (char *)malloc(count * sizeof(char));
    memcpy(directory, szDirectory, count * sizeof(char));
    *pDirectory = directory;
    return (const char *)directory;
}
#endif  /* WIN32 */

#if defined WIN32
const char *tcax_get_tcc_directory(const char *tccFilename, char **pTccDirectory) {
    int i, len, count, indicator;
    char *fullTccFilename;
    char *tccDirectory;
    len = strlen(tccFilename);
    count = __max(len, 4095) + 1;
    fullTccFilename = (char *)malloc(count * sizeof(char));
    if (strstr(tccFilename, ":") == NULL)
        GetFullPathNameA(tccFilename, count, fullTccFilename, NULL);
    else
        memcpy(fullTccFilename, tccFilename, (len + 1) * sizeof(char));
    len = strlen(fullTccFilename);
    for (i = len - 1; i >= 0; i --) {
        if ('\\' == fullTccFilename[i] || '/' == fullTccFilename[i]) {
            indicator = i;
            break;
        }
    }
    count = indicator;
    tccDirectory = (char *)malloc((count + 1) * sizeof(char));
    memcpy(tccDirectory, fullTccFilename, count * sizeof(char));
    tccDirectory[count] = '\0';
    free(fullTccFilename);
    *pTccDirectory = tccDirectory;
    return (const char *)tccDirectory;
}
#else
const char *tcax_get_tcc_directory(const char *tccFilename, char **pTccDirectory) {
    int i, len, count, indicator;
    char *fullTccFilename;
    char *tccDirectory;
    len = strlen(tccFilename);
    count = __max(len, 4095) + 1;
    fullTccFilename = (char *)malloc(count * sizeof(char));
    if ('/' == tccFilename[0]) {
        int size;
        getcwd(fullTccFilename, count);
        size = strlen(fullTccFilename);
        memcpy(fullTccFilename + size, tccFilename, (len + 1) * sizeof(char));
    }
    else
        memcpy(fullTccFilename, tccFilename, (len + 1) * sizeof(char));
    len = strlen(fullTccFilename);
    for (i = len - 1; i >= 0; i --) {
        if ('\\' == fullTccFilename[i] || '/' == fullTccFilename[i]) {
            indicator = i;
            break;
        }
    }
    count = indicator;
    tccDirectory = (char *)malloc((count + 1) * sizeof(char));
    memcpy(tccDirectory, fullTccFilename, count * sizeof(char));
    tccDirectory[count] = '\0';
    free(fullTccFilename);
    *pTccDirectory = tccDirectory;
    return (const char *)tccDirectory;
}
#endif  /* WIN32 */

const char *tcax_make_out_filename_from_tcc(const char *tccFilename, char **pOutFilename) {
    int count;
    char *outFilename;
    count = strlen(tccFilename);
    if (__str_ignore_case_cmp(tccFilename + count - 4, ".tcc") == 0) {
        count -= 4;
        outFilename = (char *)malloc((count + 1) * sizeof(char));
        memcpy(outFilename, tccFilename, count * sizeof(char));
        outFilename[count] = '\0';
    } else {
        outFilename = (char *)malloc((count + 1) * sizeof(char));
        memcpy(outFilename, tccFilename, (count + 1) * sizeof(char));
    }
    *pOutFilename = outFilename;
    return (const char *)outFilename;
}

const char *tcax_get_out_ass_filename(const char *outFilename, char **pOutAssFilename) {
    int count;
    char *assFilename;
    count = strlen(outFilename);
    assFilename = (char *)malloc((count + 5) * sizeof(char));
    memcpy(assFilename, outFilename, count * sizeof(char));
    memcpy(assFilename + count, ".ass\0", 5 * sizeof(char));
    *pOutAssFilename = assFilename;
    return (const char *)assFilename;
}

const char *tcax_get_out_tcas_filename(const char *outFilename, char **pOutTcasFilename) {
    int count;
    char *tcasFilename;
    count = strlen(outFilename);
    tcasFilename = (char *)malloc((count + 6) * sizeof(char));
    memcpy(tcasFilename, outFilename, count * sizeof(char));
    memcpy(tcasFilename + count, ".tcas\0", 6 * sizeof(char));
    *pOutTcasFilename = tcasFilename;
    return (const char *)tcasFilename;
}

static void _tcax_make_full_path_of_syl_file(const wchar_t *tccDirectory, TCC_pAttributes pAttributes) {
    if (L'\0' != pAttributes->k_timed_ass_file[0] && wcsstr(pAttributes->k_timed_ass_file, L":") == NULL) {
        int dir_len, len, count;
        wchar_t *tempString;
        dir_len = wcslen(tccDirectory);
        tempString = pAttributes->k_timed_ass_file;
        len = wcslen(tempString);
        count = dir_len + 1 + len + 1;
        pAttributes->k_timed_ass_file = (wchar_t *)malloc(count * sizeof(wchar_t));
        memcpy(pAttributes->k_timed_ass_file, tccDirectory, dir_len * sizeof(wchar_t));
        pAttributes->k_timed_ass_file[dir_len] = L'\\';
        memcpy(pAttributes->k_timed_ass_file + dir_len + 1, tempString, (len + 1) * sizeof(wchar_t));
        free(tempString);
    }
}

static void _tcax_make_full_path_of_font_file(const wchar_t *tccDirectory, TCC_pAttributes pAttributes) {
    if (wcsstr(pAttributes->font_file, L":") == NULL) {
        int dir_len, len, count;
        wchar_t *tempString;
        dir_len = wcslen(tccDirectory);
        tempString = pAttributes->font_file;
        len = wcslen(tempString);
        count = dir_len + 1 + len + 1;
        pAttributes->font_file = (wchar_t *)malloc(count * sizeof(wchar_t));
        memcpy(pAttributes->font_file, tccDirectory, dir_len * sizeof(wchar_t));
        pAttributes->font_file[dir_len] = L'\\';
        memcpy(pAttributes->font_file + dir_len + 1, tempString, (len + 1) * sizeof(wchar_t));
        if (GetFileAttributesW(pAttributes->font_file) == INVALID_FILE_ATTRIBUTES) {
            int lenWinDir;
            wchar_t szWinDir[MAX_PATH];
            wchar_t *font_file = pAttributes->font_file;
            //free(pAttributes->font_file);
            GetWindowsDirectoryW(szWinDir, MAX_PATH);
            lenWinDir = wcslen(szWinDir);
            count = lenWinDir + 7 + len + 1;
            pAttributes->font_file = (wchar_t *)malloc(count * sizeof(wchar_t));
            memcpy(pAttributes->font_file, szWinDir, lenWinDir * sizeof(wchar_t));
            memcpy(pAttributes->font_file + lenWinDir, L"\\Fonts\\", 7 * sizeof(wchar_t));
            memcpy(pAttributes->font_file + lenWinDir + 7, tempString, (len + 1) * sizeof(wchar_t));
            /* copy the font file to the current working directory */
            CopyFileW(pAttributes->font_file, font_file, 0);
            free(font_file);
        }
        free(tempString);
    }
}

static void _tcax_make_full_path_of_py_file(const wchar_t *tccDirectory, TCC_pAttributes pAttributes) {
    int i, lenTccDir, lenRootDir, len, count;
    char *ansiRootDirectory;
    wchar_t *rootDirectory;
    wchar_t *tempString;
    wchar_t *str;
    tcax_get_root_directory(&ansiRootDirectory);
    _tcax_sz_ansi_to_unicode(ansiRootDirectory, &rootDirectory);
    free(ansiRootDirectory);
    lenRootDir = wcslen(rootDirectory);
    lenTccDir = wcslen(tccDirectory);
    for (i = 0; i < pAttributes->py_file_count; i ++) {
        str = wcsstr(pAttributes->py_file[i], L"!\\");    /* !\ or !/ indicates that the py script is in the folder that is under the TCAX's root directory */
        if (!str) str = wcsstr(pAttributes->py_file[i], L"!/");
        if (str) {    /* if is in the folder which is under the TCAX's root directory */
            len = wcslen(str);
            count = lenRootDir + 1 + len - 2 + 1;
            tempString = pAttributes->py_file[i];
            pAttributes->py_file[i] = (wchar_t *)malloc(count * sizeof(wchar_t));
            memcpy(pAttributes->py_file[i], rootDirectory, lenRootDir * sizeof(wchar_t));
            pAttributes->py_file[i][lenRootDir] = L'\\';
            memcpy(pAttributes->py_file[i] + lenRootDir + 1, str + 2, (len - 2 + 1) * sizeof(wchar_t));
            free(tempString);
        } else if ((str = wcsstr(pAttributes->py_file[i], L"!")) != NULL) {    /* use built-in tcaxPy scripts */
            int j, len1, len2, offset;
            const wchar_t *scriptsFolder = L"scripts";
            wchar_t prefix[10];
            memset(prefix, 0, 10 * sizeof(wchar_t));
            str += 1;    /* to skip the exclamation mark */
            for (j = 0; j < 9; j++) {    /* to reserve a room for the NULL terminator */
                prefix[j] = towupper(str[j]);
                if (L'_' == prefix[j]) {
                    prefix[j] = L'\0';
                    break;
                }
            }
            len1 = wcslen(scriptsFolder);
            len2 = wcslen(prefix);
            len = wcslen(str);
            count = lenRootDir + 1 + len1 + 1 + len2 + 1 + len + 1;
            tempString = pAttributes->py_file[i];
            pAttributes->py_file[i] = (wchar_t *)malloc(count * sizeof(wchar_t));
            offset = 0;
            memcpy(pAttributes->py_file[i], rootDirectory, lenRootDir * sizeof(wchar_t));
            offset += lenRootDir;
            pAttributes->py_file[i][offset] = L'\\';
            offset += 1;
            memcpy(pAttributes->py_file[i] + offset, scriptsFolder, len1 * sizeof(wchar_t));
            offset += len1;
            pAttributes->py_file[i][offset] = L'\\';
            offset += 1;
            memcpy(pAttributes->py_file[i] + offset, prefix, len2 * sizeof(wchar_t));
            offset += len2;
            pAttributes->py_file[i][offset] = L'\\';
            offset += 1;
            memcpy(pAttributes->py_file[i] + offset, str, len * sizeof(wchar_t));
            offset += len;
            pAttributes->py_file[i][offset] = L'\0';
            free(tempString);
        } else if (wcsstr(pAttributes->py_file[i], L":") == NULL) {    /* is not using absolute path */
            tempString = pAttributes->py_file[i];
            len = wcslen(tempString);
            count = lenTccDir + 1 + len + 1;
            pAttributes->py_file[i] = (wchar_t *)malloc(count * sizeof(wchar_t));
            memcpy(pAttributes->py_file[i], tccDirectory, lenTccDir * sizeof(wchar_t));
            pAttributes->py_file[i][lenTccDir] = L'\\';
            memcpy(pAttributes->py_file[i] + lenTccDir + 1, tempString, (len + 1) * sizeof(wchar_t));
            free(tempString);
        }
    }
    free(rootDirectory);
}

int tcax_get_tcc_attributes(const char *tccFilename, TCC_pAttributes pAttributes) {
    /*int count;
    wchar_t *tccString;*/
    char *ansiTccDirectory;
    wchar_t *tccDirectory;
    /*if (libtcc_read_file_to_unicode(tccFilename, &tccString, &count) != tcc_error_success) {
        printf("Fatal Error: failed to read the TCC file.\n");
        return -1;
    }
    if (libtcc_check_format(tccString, count) != tcc_error_success) {
        free(tccString);
        printf("Fatal Error: bad formated TCC file.\n");
        return -1;
    }
    if (libtcc_parse_tcc_string(tccString, count, pAttributes) != tcc_error_success) {
        free(tccString);
        printf("Fatal Error: uncompleted TCC file.\n");
        return -1;
    }
    free(tccString);*/
    if (libtcc_parse_tcc_file(tccFilename, pAttributes) != tcc_error_success) {
        printf("Fatal Error: failed to read the TCC file or it is incomplete.\n");
        return -1;
    }
    /* ensure the integrity of TCC attributes */
    tcax_get_tcc_directory(tccFilename, &ansiTccDirectory);
    _tcax_sz_ansi_to_unicode(ansiTccDirectory, &tccDirectory);
    free(ansiTccDirectory);
    _tcax_make_full_path_of_syl_file(tccDirectory, pAttributes);
    _tcax_make_full_path_of_font_file(tccDirectory, pAttributes);
    _tcax_make_full_path_of_py_file(tccDirectory, pAttributes);
    free(tccDirectory);
    return 0;
}

int tcax_get_syl_line_and_word(const char *sylFilename, SYL_pLine pLine, SYL_pWord pWord) {
    int i, j, count;
    wchar_t *sylString;
    if (L'\0' == sylFilename[0])
        libsyl_create_simplest_syl_string(&sylString, &count);
    else {
        if (libsyl_read_file_to_unicode(sylFilename, &sylString, &count) != syl_error_success) {
            printf("Fatal Error: failed to read the SYL file.\n");
            printf("Tips: check if the name specified in the TCC file is the same with the ASS file exited.\n");
            return -1;
        }
    }
    if (libsyl_parse_syl_string(sylString, count, pLine) != syl_error_success) {
        free(sylString);
        printf("Fatal Error: bad formated SYL file.\n");
        printf("Tips: make sure that the k-timed ASS file contains the correct header.\n");
        printf("Tips: `Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text'\n");
        return -1;
    }
    free(sylString);
    if (libsyl_check_line_format(pLine, &i, &j) != syl_error_success) {
        printf("Fatal Error: invalid SYL line, Line index = %i ; K index = %i\n", i, j);
        printf("Tips: every SYL line should have at least one `\\k' tag.\n");
        printf("Tips: check if the k tag is correct.\n");
        return -1;
    }
    libsyl_add_k0_to_white_spaces(pLine);
    libsyl_del_first_null_k(pLine);
    libsyl_parse_line(pLine, pWord);
    return 0;
}

int tcax_init_tcaxpy_data(const char *tccFilename, const char *outFilename, TCC_pAttributes pAttributes, PY_pInitData pInitData) {
    int count;
    SYL_Line sylLine;
    SYL_Word sylWord;
    SYL_Syllable sylSyllable;
    TM_Font tmFont;
    TM_TextHori tmTextHori;
    TM_TextVert tmTextVert;
    char *outAssFilename;
    char *tempStr;
    wchar_t *tempWcs;
    if (tcax_get_tcc_attributes(tccFilename, pAttributes) != 0) return -1;
    tcax_get_out_ass_filename(outFilename, &outAssFilename);
    _tcax_sz_unicode_to_ansi(pAttributes->k_timed_ass_file, &tempStr);
    if (__str_ignore_case_cmp(outAssFilename, tempStr) == 0) {    /* the output ASS filename cannot be the same as the input k-timed ASS filename */
        free(outAssFilename);
        free(tempStr);
        libtcc_free_attributes(pAttributes);
        printf("Fatal Error: the output filename cannot be the same as the input k-timed ASS filename!\n");
        return -1;
    }
    free(outAssFilename);
    if (tcax_get_syl_line_and_word(tempStr, &sylLine, &sylWord) != 0) {
        free(tempStr);
        libtcc_free_attributes(pAttributes);
        return -1;
    }
    free(tempStr);
    libsyl_parse_word_to_syllable(&sylWord, &sylSyllable);
    pAttributes->font_face_id -= 1;    /* in TCC file font_face_id starts from 1, in FreeType it starts from 0 */
    _tcax_sz_unicode_to_ansi(pAttributes->font_file, &tempStr);
    if (libtm_init_font(&tmFont, tempStr, pAttributes->font_face_id, pAttributes->font_size, pAttributes->space_scale) != tm_error_success) {
        free(tempStr);
        libtcc_free_attributes(pAttributes);
        libsyl_free_line(&sylLine);
        libsyl_free_word(&sylWord);
        libsyl_free_syllable(&sylSyllable);
        printf("Fatal Error: failed to initialize the font.\n");
        printf("Tips: check if the name specified in the TCC file is the same with the font file exited.\n");
        printf("Tips: `font face id' should be 1 to TTF files.\n");
        return -1;
    }
    free(tempStr);
    if (L'\0' == pAttributes->font_face_name[0]) {
        free(pAttributes->font_face_name);
        count = wcslen(tmFont.name) + 1;
        pAttributes->font_face_name = (wchar_t *)malloc(count * sizeof(wchar_t));
        memcpy(pAttributes->font_face_name, tmFont.name, count * sizeof(wchar_t));
    }
    if (libtm_fill_tm_text_hori(&tmFont, &sylSyllable, pAttributes->spacing, &tmTextHori) != tm_error_success) {
        libtcc_free_attributes(pAttributes);
        libsyl_free_line(&sylLine);
        libsyl_free_word(&sylWord);
        libsyl_free_syllable(&sylSyllable);
        libtm_free_font(&tmFont);
        printf("Fatal Error: invalid character in SYL file.\n");
        printf("Tips: change a correct font which contains the character.\n");
        printf("Tips: delete the character.\n");
        return -1;
    }
    if (libtm_fill_tm_text_vert(&tmFont, &sylSyllable, pAttributes->spacing, &tmTextVert) != tm_error_success) {
        libtcc_free_attributes(pAttributes);
        libsyl_free_line(&sylLine);
        libsyl_free_word(&sylWord);
        libsyl_free_syllable(&sylSyllable);
        libtm_free_font(&tmFont);
        libtm_free_tm_text_hori(&tmTextHori);
        printf("Fatal Error: invalid character in SYL file.\n");
        printf("Tips: change a correct font which contains the character.\n");
        printf("Tips: delete the character.\n");
        return -1;
    }
    libtcc_info_to_tcax(pAttributes, (TCC_pInfoToTcax) &pInitData->tccData);
    if (K_MODE_WORD == pAttributes->k_mode) {
        TM_WordHori tmWordHori;
        TM_WordVert tmWordVert;
        libsyl_info_to_tcax_with_word(&sylLine, &sylWord, (SYL_pInfoToTcax) &pInitData->sylData);
        libtm_fill_tm_word_hori(&tmFont, &sylWord, &tmTextHori, &tmWordHori);
        libtm_info_to_tcax_with_word_hori(&tmFont, &tmWordHori, (TM_pInfoToTcaxHori) &pInitData->tmHoriData);
        libtm_free_tm_word_hori(&tmWordHori);
        libtm_fill_tm_word_vert(&tmFont, &sylWord, &tmTextVert, &tmWordVert);
        libtm_info_to_tcax_with_word_vert(&tmFont, &tmWordVert, (TM_pInfoToTcaxVert) &pInitData->tmVertData);
        libtm_free_tm_word_vert(&tmWordVert);
    } else if (K_MODE_SYL == pAttributes->k_mode) {
        libsyl_info_to_tcax_with_syl(&sylLine, &sylSyllable, (SYL_pInfoToTcax) &pInitData->sylData);
        libtm_info_to_tcax_with_text_hori(&tmFont, &tmTextHori, (TM_pInfoToTcaxHori) &pInitData->tmHoriData);
        libtm_info_to_tcax_with_text_vert(&tmFont, &tmTextVert, (TM_pInfoToTcaxVert) &pInitData->tmVertData);
    } else if (K_MODE_ADV == pAttributes->k_mode) {
        SYL_Adv sylAdv;
        TM_AdvHori tmAdvHori;
        TM_AdvVert tmAdvVert;
        libsyl_parse_word_to_adv(&sylWord, &sylAdv);
        libsyl_info_to_tcax_with_adv(&sylLine, &sylAdv, (SYL_pInfoToTcax) &pInitData->sylData);
        libtm_fill_tm_adv_hori(&tmFont, &sylAdv, &tmTextHori, &tmAdvHori);
        libtm_info_to_tcax_with_adv_hori(&tmFont, &tmAdvHori, (TM_pInfoToTcaxHori) &pInitData->tmHoriData);
        libtm_free_tm_adv_hori(&tmAdvHori);
        libtm_fill_tm_adv_vert(&tmFont, &sylAdv, &tmTextVert, &tmAdvVert);
        libtm_info_to_tcax_with_adv_vert(&tmFont, &tmAdvVert, (TM_pInfoToTcaxVert) &pInitData->tmVertData);
        libtm_free_tm_adv_vert(&tmAdvVert);
        libsyl_free_adv(&sylAdv);
    }
    libsyl_free_word(&sylWord);
    libsyl_free_syllable(&sylSyllable);
    libtm_free_font(&tmFont);
    libtm_free_tm_text_hori(&tmTextHori);
    libtm_free_tm_text_vert(&tmTextVert);
    if (pAttributes->inherit_ass_header) {
        count = wcslen(sylLine.header) + 1;
        pInitData->assHeader = (wchar_t *)malloc(count * sizeof(wchar_t));
        memcpy(pInitData->assHeader, sylLine.header, count * sizeof(wchar_t));
    } else libtcc_make_ass_header(pAttributes, &pInitData->assHeader, &count);
    libsyl_free_line(&sylLine);
    tcax_get_root_directory(&pInitData->directory);
    _tcax_sz_ansi_to_unicode(outFilename, &tempWcs);
    count = wcslen(tempWcs) + 1;
    pInitData->outFilename = (wchar_t *)malloc(count * sizeof(wchar_t));
    memcpy(pInitData->outFilename, tempWcs, count * sizeof(wchar_t));
    free(tempWcs);
    return 0;
}

void tcax_free_tcaxpy_data(PY_pInitData pInitData) {
    free(pInitData->assHeader);
    free(pInitData->directory);
    free(pInitData->outFilename);
    libtcc_free_info((TCC_pInfoToTcax) &pInitData->tccData);
    libsyl_free_info((SYL_pInfoToTcax) &pInitData->sylData);
    libtm_free_info_hori((TM_pInfoToTcaxHori) &pInitData->tmHoriData);
    libtm_free_info_vert((TM_pInfoToTcaxVert) &pInitData->tmVertData);
}

int tcax_write_ass_string_to_file(FILE *assfp, const wchar_t *assString, int count) {
    int size;
    char *utf8String;
#if defined WIN32
    size = WideCharToMultiByte(CP_UTF8, 0, assString, count, NULL, 0, NULL, NULL);
    utf8String = (char *)malloc(size);
    WideCharToMultiByte(CP_UTF8, 0, assString, count, utf8String, size, NULL, NULL);
#else
    setlocale(LC_CTYPE, "");
    size = wcstombs(NULL, assString, 0);
    utf8String = (char *)malloc(size * sizeof(char));
    wcstombs(utf8String, assString, size);
#endif  /* WIN32 */
    if (fwrite(utf8String, sizeof(char), size, assfp) != size) {
        free(utf8String);
        return -1;
    }
    free(utf8String);
    return 0;
}

/*
int tcax_write_tcas_buffer_to_file(TCAS_pFile pFile, TCAS_pHeader pHeader, const tcas_unit *tcasBuffer, int count) {
    tcas_unit *buf;
    tcas_u32 chunks, units;
    libtcas_get_raw_chunks_min_max_time((const TCAS_pRawChunk)tcasBuffer, count, pHeader);
    libtcas_compress_raw_chunks((const TCAS_pRawChunk)tcasBuffer, count, TCAS_FALSE, &buf, &chunks, &units);
    pHeader->chunks += chunks;
    if (libtcas_write(pFile, buf, units) != tcas_error_success) {
        free(buf);
        return -1;
    }
    free(buf);
    return 0;
}
*/

int tcax_write_tcas_buffer_to_file(TCAS_pFile pFile, TCAS_pHeader pHeader, const tcas_unit *tcasBuffer, int count) {
    tcas_unit *buf;
    tcas_u32 chunks, units;
    TCAS_pChunk pChunks;
    libtcas_get_raw_chunks_min_max_time((const TCAS_pRawChunk)tcasBuffer, count, pHeader);
    libtcas_compress_raw_chunks((const TCAS_pRawChunk)tcasBuffer, count, TCAS_FALSE, &buf, &chunks, &units);
    pHeader->chunks += chunks;
    libtcas_compress_chunks_z(buf, chunks, &pChunks);
    free(buf);
    libtcas_write_chunks_z(pFile, pChunks, chunks, TCAS_TRUE);
    free(pChunks);
    return 0;
}

int tcax_get_line_init_pos_x(const TCC_pAttributes pAttributes, const PY_pInitData pInitData, int iLine) {
    int lineInitPosX;
    if (TEXT_LAYOUT_HORI == pAttributes->text_layout) {
        switch (pAttributes->alignment) {
        case 1:
        case 4:
        case 7:
            lineInitPosX = pAttributes->x_offset;
            break;
        case 2:
        case 5:
        case 8:
            lineInitPosX = (int)((pAttributes->fx_width - pInitData->tmHoriData.length1D[iLine]) / 2.0 + 0.5) + pAttributes->x_offset;
            break;
        case 3:
        case 6:
        case 9:
            lineInitPosX = pAttributes->fx_width - pInitData->tmHoriData.length1D[iLine] - pAttributes->x_offset;
            break;
        default:    /* same as an1 */
            lineInitPosX = pAttributes->x_offset;
            break;
        }
    } else {
        switch (pAttributes->alignment) {
        case 1:
        case 4:
        case 7:
            lineInitPosX = pAttributes->x_offset;
            break;
        case 2:
        case 5:
        case 8:
            lineInitPosX = (int)(pAttributes->fx_width / 2.0 + 0.5) + pAttributes->x_offset;
            break;
        case 3:
        case 6:
        case 9:
            lineInitPosX = pAttributes->fx_width - pAttributes->font_size - pAttributes->x_offset;
            break;
        default:    /* same as an9 */
            lineInitPosX = pAttributes->fx_width - pAttributes->font_size - pAttributes->x_offset;
            break;
        }
    }
    return lineInitPosX;
}

int tcax_get_line_init_pos_y(const TCC_pAttributes pAttributes, const PY_pInitData pInitData, int iLine) {
    int lineInitPosY;
    if (TEXT_LAYOUT_HORI == pAttributes->text_layout) {
        switch (pAttributes->alignment) {
        case 1:
        case 2:
        case 3:
            lineInitPosY = pAttributes->fx_height - pAttributes->y_offset - pAttributes->font_size;
            break;
        case 4:
        case 5:
        case 6:
            lineInitPosY = (int)(pAttributes->fx_height / 2.0 + 0.5) - pAttributes->y_offset;
            break;
        case 7:
        case 8:
        case 9:
            lineInitPosY = pAttributes->y_offset;
            break;
        default:    /* same as an1 */
            lineInitPosY = pAttributes->fx_height - pAttributes->y_offset - pAttributes->font_size;
            break;
        }
    } else {
        switch (pAttributes->alignment) {
        case 1:
        case 2:
        case 3:
            lineInitPosY = pAttributes->fx_height - pInitData->tmVertData.length1D[iLine] - pAttributes->y_offset;
            break;
        case 4:
        case 5:
        case 6:
            lineInitPosY = (int)((pAttributes->fx_height - pInitData->tmVertData.length1D[iLine]) / 2.0 + 0.5) - pAttributes->y_offset;
            break;
        case 7:
        case 8:
        case 9:
            lineInitPosY = pAttributes->y_offset;
            break;
        default:    /* same as an9 */
            lineInitPosY = pAttributes->y_offset;
            break;
        }
    }
    return lineInitPosY;
}

static void _tcax_get_numerator_and_denominator(double fps, unsigned long *pNumerator, unsigned long *pDenominator) {
    unsigned long numerator, denominator, x, y, t;
    denominator = 100000;
    numerator   = (unsigned long)(fps * denominator);
    x = numerator;
    y = denominator;
    while (y) {   // find gcd
        t = x % y;
        x = y;
        y = t;
    }
    numerator /= x;
    denominator /= x;
    *pNumerator = numerator;
    *pDenominator = denominator;
}

static int _tcax_is_text_c_or_j_or_k(const wchar_t *text) {
    int i, count;
    count = wcslen(text);
    for (i = 0; i < count; i ++) {
        if (text[0] >= 0x3000 && text[1] <= 0x9FFF) return 1;
    }
    return 0;
}

static void _tcax_adjust_vertical_metrics(PY_pInitData pInitData) {
    int i, j, k, index;
    int *pKerning;
    pKerning = (int *)malloc(pInitData->sylData.count * sizeof(int));
    memset(pKerning, 0, pInitData->sylData.count * sizeof(int));
    k = 0;
    for (i = 0; i < pInitData->sylData.count; i ++) {
        if (!_tcax_is_text_c_or_j_or_k(pInitData->sylData.text2D[i])) {
            pInitData->tmVertData.advance2D[i] = pInitData->tmHoriData.advance2D[i];
            if (i - 1 == k)    /* if the two horizontal ones are close to each other then they should have kerning */
                pKerning[i] = pInitData->tmHoriData.kerning2D[i];
            k = i;
        }
    }
    index = 0;
    for (i = 0; i < pInitData->sylData.lines; i ++) {
        k = index;
        pInitData->tmVertData.advanceDiff2D[k] = 0;
        index ++;
        for (j = 1; j < pInitData->sylData.count1D[i]; j ++) {
            pInitData->tmVertData.advanceDiff2D[index] = pInitData->tmVertData.advanceDiff2D[index - 1] + pInitData->tmVertData.advance2D[index - 1] + pKerning[index] + pInitData->tccData.spacing;
            index ++;
        }
    }
    free(pKerning);
}

static int _tcax_use_user_func(const PY_pTcaxPy pTcaxPy, const PY_pInitData pInitData, const TCC_pAttributes pAttributes) {
    int i;
    char *tempStr;
    for (i = 0; i < pAttributes->py_file_count; i ++) {
        if (pAttributes->beg_line[i] <= 0)
            pAttributes->beg_line[i] += pInitData->sylData.lines;
        pAttributes->beg_line[i] -= 1;    /* in TCC file beg_line starts from 1, in tcax it starts from 0 */
        if (pAttributes->end_line[i] <= 0)
            pAttributes->end_line[i] += pInitData->sylData.lines;
        if (pAttributes->beg_line[i] >= pAttributes->end_line[i])
            printf("Warning: the %i(th) `beg line' is greater than `end line'\n", i + 1);
    }
    for (i = 0; i < pAttributes->py_file_count; i ++) {
        _tcax_sz_unicode_to_ansi(pAttributes->py_file[i], &tempStr);
        if (tcaxpy_init_user_py_module(pTcaxPy, pInitData->directory, tempStr, pAttributes->tcaxpy_init, pAttributes->tcaxpy_user, pAttributes->tcaxpy_fin) != py_error_success) {
            free(tempStr);
            printf("Fatal Error: failed to initialize the %i(th) user tcax py script.\n", i + 1);
            printf("Tips: check if the name specified in the TCC file is the same with the py file exited.\n");
            printf("Tips: if so, there may be some errors in the py script.\n");
            return -1;
        }
        free(tempStr);
        if (pAttributes->tcaxpy_init) {
            if (tcaxpy_script_func_init(pTcaxPy) != py_error_success) {
                printf("Fatal Error: failed to execute the tcaxPy_Init() function in the %i(th) user tcax py script.\n", i + 1);
                return -1;
            }
        }
        if (tcaxpy_script_func_user(pTcaxPy) != py_error_success) {
            printf("Fatal Error: failed to execute the tcaxPy_User() function in the %i(th) user tcax py script.\n", i + 1);
            return -1;
        }
        if (pAttributes->tcaxpy_fin) {
            if (tcaxpy_script_func_fin(pTcaxPy) != py_error_success) {
                printf("Fatal Error: failed to execute the tcaxPy_Fin() function in the %i(th) user tcax py script.\n", i + 1);
                return -1;
            }
        }
    }
    return 0;
}

static int _tcax_use_main_func(const PY_pTcaxPy pTcaxPy, const PY_pInitData pInitData, const TCC_pAttributes pAttributes) {
    int i;
    unsigned char BOM[3];
    char *assFilename;
    char *tcasFilename;
    char *ansiOutFilename;
    char *tempStr;
    FILE *assfp;
    TCAS_File tcasFile;
    TCAS_Header tcasHeader;
    /* Show progress */
    HANDLE hStdout;
    COORD coord;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    /* */
    _tcax_sz_unicode_to_ansi(pInitData->outFilename, &ansiOutFilename);
    assfp = fopen(tcax_get_out_ass_filename(ansiOutFilename, &assFilename), "wb");
    if (!assfp) {
        free(ansiOutFilename);
        free(assFilename);
        printf("Fatal Error: failed to create the output ASS file.\n");
        return -1;
    }
    free(assFilename);
    if (libtcas_open_file(&tcasFile, tcax_get_out_tcas_filename(ansiOutFilename, &tcasFilename), tcas_file_create_new) != tcas_error_success) {
        free(ansiOutFilename);
        free(tcasFilename);
        fclose(assfp);
        printf("Fatal Error: failed to create the output TCAS file.\n");
        return -1;
    }
    free(ansiOutFilename);
    free(tcasFilename);
    BOM[0] = 0xEF;
    BOM[1] = 0xBB;
    BOM[2] = 0xBF;
    fwrite(BOM, sizeof(unsigned char), 3, assfp);
    tcax_write_ass_string_to_file(assfp, pInitData->assHeader, wcslen(pInitData->assHeader));
    _tcax_get_numerator_and_denominator(pAttributes->fx_fps, &tcasHeader.fpsNumerator, &tcasHeader.fpsDenominator);
    //libtcas_set_header(&tcasHeader, TCAS_FILE_TYPE_COMPRESSED, TCAS_KEY_FRAMING_ENABLED, pAttributes->fx_width, pAttributes->fx_height, TCAS_INIT_MIN_TIME, TCAS_INIT_MAX_TIME, 0, tcasHeader.fpsNumerator, tcasHeader.fpsDenominator);
    libtcas_set_header(&tcasHeader, TCAS_FILE_TYPE_COMPRESSED_Z, TCAS_KEY_FRAMING_ENABLED, pAttributes->fx_width, pAttributes->fx_height, TCAS_INIT_MIN_TIME, TCAS_INIT_MAX_TIME, 0, tcasHeader.fpsNumerator, tcasHeader.fpsDenominator);
    libtcas_write_header(&tcasFile, &tcasHeader, TCAS_FALSE);
    for (i = 0; i < pAttributes->py_file_count; i ++) {
        if (pAttributes->beg_line[i] <= 0)
            pAttributes->beg_line[i] += pInitData->sylData.lines;
        pAttributes->beg_line[i] -= 1;    /* in TCC file beg_line starts from 1, in tcax it starts from 0 */
        if (pAttributes->end_line[i] <= 0)
            pAttributes->end_line[i] += pInitData->sylData.lines;
        if (pAttributes->beg_line[i] >= pAttributes->end_line[i])
            printf("Warning: the %i(th) `beg line' is greater than `end line'\n", i + 1);
    }
    for (i = 0; i < pAttributes->py_file_count; i ++) {
        int index, offset, total;
        int iLine, iText, nTexts, start, end, timeDiff, time, ix, iy, x, y, advance;
        const wchar_t *text;
        void **pReturnedBuf;
        _tcax_sz_unicode_to_ansi(pAttributes->py_file[i], &tempStr);
        if (tcaxpy_init_user_py_module(pTcaxPy, pInitData->directory, tempStr, pAttributes->tcaxpy_init, pAttributes->tcaxpy_user, pAttributes->tcaxpy_fin) != py_error_success) {
            free(tempStr);
            fclose(assfp);
            libtcas_close_file(&tcasFile);
            printf("Fatal Error: failed to initialize the %i(th) user tcax py script.\n", i + 1);
            printf("Tips: check if the name specified in the TCC file is the same with the py file exited.\n");
            printf("Tips: if so, there may be some errors in the py script.\n");
            return -1;
        }
        free(tempStr);
        if (pAttributes->tcaxpy_init) {
            if (tcaxpy_script_func_init(pTcaxPy) != py_error_success) {
                fclose(assfp);
                libtcas_close_file(&tcasFile);
                printf("Fatal Error: failed to execute the tcaxPy_Init() function in the %i(th) user tcax py script.\n", i + 1);
                return -1;
            }
        }
        /* get the current cursor position */
        hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(hStdout, &csbi);
        coord.X = 0;
        coord.Y = csbi.dwCursorPosition.Y + 1;
        /* */
        if (0 == pAttributes->beg_line[i] && pInitData->sylData.lines == pAttributes->end_line[i]) {
            offset = 0;
            total = pInitData->sylData.count;
        } else {
            offset = 0;
            for (iLine = 0; iLine < pAttributes->beg_line[i]; iLine ++)
                offset += pInitData->sylData.count1D[iLine];
            total = 0;
            for (iLine = pAttributes->beg_line[i]; iLine < pAttributes->end_line[i]; iLine ++)
                total += pInitData->sylData.count1D[iLine];
        }
        index = 0;
        for (iLine = 0; iLine < pInitData->sylData.lines; iLine ++) {
            ix = tcax_get_line_init_pos_x(pAttributes, pInitData, iLine);
            iy = tcax_get_line_init_pos_y(pAttributes, pInitData, iLine);
            nTexts = pInitData->sylData.count1D[iLine];
            for (iText = 0; iText < nTexts; iText ++) {
                text = pInitData->sylData.text2D[index];
                if (iLine >= pAttributes->beg_line[i] && iLine < pAttributes->end_line[i] && !libsyl_is_white_spaces(text, wcslen(text))) {
                    start = pInitData->sylData.begTime1D[iLine];
                    end = pInitData->sylData.endTime1D[iLine];
                    timeDiff = pInitData->sylData.timeDiff2D[index];
                    time = pInitData->sylData.time2D[index];
                    if (TEXT_LAYOUT_HORI == pAttributes->text_layout) {
                        advance = pInitData->tmHoriData.advance2D[index];
                        x = ix + pInitData->tmHoriData.advanceDiff2D[index] + (int)(advance / 2.0 + 0.5);
                        y = iy + (int)(pAttributes->font_size / 2.0 + 0.5);
                    } else {
                        advance = pInitData->tmVertData.advance2D[index];
                        x = ix + (int)(pAttributes->font_size / 2.0 + 0.5);
                        y = iy + pInitData->tmVertData.advanceDiff2D[index] + (int)(advance / 2.0 + 0.5);
                    }
                    pReturnedBuf = tcaxpy_script_func_main(pTcaxPy, iLine, iText, nTexts, start, end, timeDiff, time, x, y, advance, text);
                    if (!pReturnedBuf) {
                        fclose(assfp);
                        libtcas_write_header(&tcasFile, &tcasHeader, 0);
                        libtcas_close_file(&tcasFile);
                        printf("Fatal Error: failed to execute the tcaxPy_Main() function in the %i(th) user tcax py script.\n", i + 1);
                        return -1;
                    }
                    if ((int)pReturnedBuf[0] > 0) {
                        tcax_write_ass_string_to_file(assfp, (const wchar_t *)pReturnedBuf[1], (int)pReturnedBuf[0]);
                        free(pReturnedBuf[1]);
                    }
                    if ((int)pReturnedBuf[2] > 0) {
                        tcax_write_tcas_buffer_to_file(&tcasFile, &tcasHeader, (const tcas_unit *)pReturnedBuf[3], (int)pReturnedBuf[2]);
                        free(pReturnedBuf[3]);
                    }
                    free(pReturnedBuf);
                    SetConsoleCursorPosition(hStdout, coord);
                    printf("Executing script %i of %i, progress: %.2f%%\n", i + 1, pAttributes->py_file_count, 100 * (index - offset) / (double)total);
                    //printf("Progress: %.2f%c\n", 100 * (i + (index - offset) / (double)total) / pAttributes->py_file_count, '%');
                }
                index ++;
            }
        }
        if (pAttributes->tcaxpy_fin) {
            if (tcaxpy_script_func_fin(pTcaxPy) != py_error_success) {
                fclose(assfp);
                libtcas_close_file(&tcasFile);
                printf("Fatal Error: failed to execute the tcaxPy_Fin() function in the %i(th) user tcax py script.\n", i + 1);
                return -1;
            }
        }
        SetConsoleCursorPosition(hStdout, coord);
        printf("Executing script %i of %i, progress: %.2f%%\n", i + 1, pAttributes->py_file_count, 100.0);
    }
    fclose(assfp);
    libtcas_write_header(&tcasFile, &tcasHeader, TCAS_FALSE);    /* update the header */
    libtcas_close_file(&tcasFile);
    return 0;
}

int tcax_entry(const char *tccFilename, const char *outFilename) {
    TCC_Attributes tccAttributes;
    PY_InitData InitData;
    PY_TcaxPy TcaxPy;
    memset(&TcaxPy, 0, sizeof(PY_TcaxPy));
    if (tcax_init_tcaxpy_data(tccFilename, outFilename, &tccAttributes, &InitData) != 0) return -1;
    _tcax_adjust_vertical_metrics(&InitData);
    if (tcaxpy_init_base_py_module(&TcaxPy, &InitData) != py_error_success) {
        libtcc_free_attributes(&tccAttributes);
        tcax_free_tcaxpy_data(&InitData);
        printf("Fatal Error: failed to initialize the base tcaxPy module.\n");
        printf("Tips: reinstall TCAX may fix the problem.\n");
        return -1;
    }
    if (tccAttributes.tcaxpy_user) {
        if (_tcax_use_user_func(&TcaxPy, &InitData, &tccAttributes) != 0) {
            libtcc_free_attributes(&tccAttributes);
            tcax_free_tcaxpy_data(&InitData);
            tcaxpy_fin_tcaxpy(&TcaxPy);
            return -1;
        }
    } else {
        if (_tcax_use_main_func(&TcaxPy, &InitData, &tccAttributes) != 0) {
            libtcc_free_attributes(&tccAttributes);
            tcax_free_tcaxpy_data(&InitData);
            tcaxpy_fin_tcaxpy(&TcaxPy);
            return -1;
        }
    }
    libtcc_free_attributes(&tccAttributes);
    tcax_free_tcaxpy_data(&InitData);
    tcaxpy_fin_tcaxpy(&TcaxPy);
    return 0;
}

