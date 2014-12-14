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

#ifndef LIBSYL_SYL_H
#define LIBSYL_SYL_H
#pragma once

#if defined WIN32
#include <windows.h>
#else
#include <locale.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <memory.h>

#define LIBSYL_VERSION 0x00006000    /**< High word of LIBSYL_VERSION indicates major libsyl version, and low word indicates minor version */
#define LIBSYL_VER_STR L"0.6"

#define SYL_VERSION 0x10000000
#define SYL_VER_STR L"1.0"


/**
 * SYL_Error_Code enumeration defines some common errors.
 */
typedef enum _syl_error_code {
    syl_error_success = 0,    /**< indicates a successful process */
    syl_error_null_pointer,  /**< receive a null pointer which is not desired */
    syl_error_file_not_found,  /**< can not find the target SYL file */
    syl_error_file_cant_open,   /**< can not open the target SYL file */
    syl_error_file_cant_create,  /**< can not create the target SYL file */
    syl_error_file_while_reading,  /**< some error occurred while reading a SYL file */
    syl_error_file_while_writing,    /**< some error occurred while writing to a SYL file */
    syl_error_file_bad_format,    /**< the target SYL file's bad-formated */
    syl_error_file_invalid    /**< the target SYL file's invalid */
} SYL_Error_Code;

/**
 * SYL_Line structure
 */
typedef struct _syl_line {
    wchar_t  *header;            /**< header of the input SYL file */
    int       lines;             /**< number of lines of the input SYL file */
    wchar_t **line1D;            /**< store karaoke lines of the input SYL file */
    int      *begTime1D;         /**< beginning time of karaoke lines */
    int      *endTime1D;         /**< end time of karaoke lines */
} SYL_Line, *SYL_pLine;

/**
 * SYL_Word structure
 */
typedef struct _syl_word {
    int       lines;             /**< number of lines of the input SYL file */
    int       words;             /**< total number of words of the input SYL file */
    int      *kCount1D;          /**< number of k codes in a karaoke line, the same as number of words */
    int      *kTime2D;           /**< the input karaoke values */
    int      *kTimeDiff2D;       /**< the sum of all the k-time values up to the specified word in a karaoke line */
    wchar_t **word2D;            /**< words in a karaoke line, pointer to string */
} SYL_Word, *SYL_pWord;

/**
 * SYL_Syllable structure
 */
typedef struct _syl_syllable {
    int       lines;             /**< number of lines of the input SYL file */
    int       syls;              /**< total number of texts (syllables) of the input SYL file */
    int      *sylCount1D;        /**< number of texts (syllables) in a karaoke line */
    int      *sylTime2D;         /**< the derived syllable-time based from k-time, the value is obtained by a special algorithm that ensures every text (syllable) has its very own time */
    int      *sylTimeDiff2D;     /**< the sum of all the syl-time values up to the specified text in a karaoke line */
    wchar_t  *syl2D;             /**< texts in a karaoke line, pointer to wchar_t */
} SYL_Syllable, *SYL_pSyllable;

/**
 * SYL_Adv structure
 */
typedef struct _syl_adv {
    int       lines;             /**< number of lines of the input SYL file */
    int       advs;              /**< total number of adv words of the input SYL file */
    int      *advCount1D;        /**< the number of adv (re-coupled) texts in a karaoke line, adv word is derived by a special algorithm that ensures the word holding <= 2 syllables */
    int      *advTime2D;         /**< the derived adv-time based from syl-time, the value is obtained by a special algorithm that ensures every two (or less) texts has its very own time */
    int      *advTimeDiff2D;     /**< the sum of all the adv-time values up to the specified adv word in a karaoke line */
    wchar_t **adv2D;             /**< adv words in a karaoke line, pointer to string */
} SYL_Adv, *SYL_pAdv;

/**
 * SYL_InfoToTcax structure
 */
typedef struct _syl_info_to_tcax {
    /* SYL_Line */
    /* wchar_t  *header; */
    int       lines;             /**< number of lines of the input SYL file */
    wchar_t **line1D;            /**< store karaoke lines of the input SYL file */
    int      *begTime1D;         /**< beginning time of karaoke lines */
    int      *endTime1D;         /**< end time of karaoke lines */
    /* can be one of SYL_Word/SYL_Syllable/SYL_Adv */
    int       count;             /**< total number of words/syls/advs of the input SYL file */
    int      *count1D;           /**< number of k codes/syllables/adv words in a karaoke line */
    int      *time2D;            /**< k-time values */
    int      *timeDiff2D;        /**< the sum of all the k-time values up to the specified word/syl/adv in a karaoke line */
    wchar_t **text2D;            /**< word/syl/adv */
} SYL_InfoToTcax, *SYL_pInfoToTcax;


/* Inhibit C++ name-mangling for libsyl functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * Read a plain text file and convert to a unicode (little endian) string. 
 * Remark: this function will automatically add an L"\r\n" to the end of the string. 
 * The string is null terminated.
 *
 * @param filename the filename of the target SYL file
 * @param pBuffer a pointer to a block of memory that is going to hold the SYL string (unicode little endian)
 * @param pCount the address of an int variable that is going to hold number of characters in the SYL string
 * @return SYL_Error_Code
 */
extern SYL_Error_Code libsyl_read_file_to_unicode(const char *filename, wchar_t **pBuffer, int *pCount);

/**
 * Create the simplest SYL string.
 * @param pBuffer a pointer to a block of memory that is going to hold the SYL string (unicode little endian)
 * @param pCount the address of an int variable that is going to hold number of characters in the SYL string
 */
extern void libsyl_create_simplest_syl_string(wchar_t **pBuffer, int *pCount);

/**
 * Parse SYL string and convert to SYL_Line.
 * @param sylString the target SYL string which is going to be parsed
 * @param count number of characters in the SYL string
 * @param pLine pointer to the SYL_Line structure that is going to be filled with the parsed data
 * @return SYL_Error_Code
 */
extern SYL_Error_Code libsyl_parse_syl_string(const wchar_t *sylString, int count, SYL_pLine pLine);

/**
 * Check general format of SYL line. 
 * Remark: refer to the SYL File Format Specification to see the details about what is a good SYL line format. 
 * Note that this function should be used immediately after function libsyl_read_file_to_unicode().
 *
 * @param pLine pointer to SYL_Line structure that is going to be checked
 * @param pi pointer to an int value which is going to hold the index of the line that encountered an error
 * @param pj pointer to an int value which is going to hold the index of the text in the line that encountered an error
 * @return SYL_Error_Code
 */
extern SYL_Error_Code libsyl_check_line_format(const SYL_pLine pLine, int *pi, int *pj);

/**
 * Add k-tags {\k0} to white spaces in the SYL string.
 * @param pLine pointer to the SYL_Line structure whose SYL lines are going to be added with {\k0} tags
 */
extern void libsyl_add_k0_to_white_spaces(SYL_pLine pLine);

/**
 * Delete the first k-tags, which are not followed by texts from the SYL line. 
 * Remark: this function should be put after libsyl_add_k0_to_white_spaces function, 
 * since it will remove the sub-dialogue information from the line
 *
 * @param pLine pointer to the SYL_Line structure whose SYL lines are going to be reformed
 */
extern void libsyl_del_first_null_k(SYL_pLine pLine);

/**
 * Parse SYL_Line and convert to SYL_Word.
 * @param pLine pointer to SYL_Line structure that is going to be parsed
 * @param pWord pointer to SYL_Word structure that is going to be filled with the parsed data
 */
extern void libsyl_parse_line(const SYL_pLine pLine, SYL_pWord pWord);

/**
 * Parse SYL_Word and convert to SYL_Syllable.
 * @param pWord pointer to SYL_Word structure that is going to be parsed
 * @param pSyllable pointer to SYL_Syllable structure that is going to be filled with the parsed data
 */
extern void libsyl_parse_word_to_syllable(const SYL_pWord pWord, SYL_pSyllable pSyllable);

/**
 * Check if a string contains only white spaces.
 * @param str the target string
 * @param count number of characters in the string
 * @return int
 */
extern int libsyl_is_white_spaces(const wchar_t *str, int count);

/**
 * Parse SYL_Word and convert to SYL_Adv.
 * @param pWord pointer to SYL_Word structure that is going to be parsed
 * @param pAdv pointer to SYL_Adv structure that is going to be filled with the parsed data
 */
extern void libsyl_parse_word_to_adv(const SYL_pWord pWord, SYL_pAdv pAdv);

/**
 * Copy SYL_Line and SYL_Word to SYL_InfoToTcx structure.
 * @param pLine pointer to SYL_Line structure 
 * @param pWord pointer to SYL_Word structure
 * @param pInfoToTcax pointer to SYL_InfoToTcax structure that is going to be filled with the copied data
 */
extern void libsyl_info_to_tcax_with_word(const SYL_pLine pLine, const SYL_pWord pWord, SYL_pInfoToTcax pInfoToTcax);

/**
 * Copy SYL_Line and SYL_Syllable to SYL_InfoToTcx structure.
 * @param pLine pointer to SYL_Line structure 
 * @param pSyllable pointer to SYL_Syllable structure
 * @param pInfoToTcax pointer to SYL_InfoToTcax structure that is going to be filled with the copied data
 */
extern void libsyl_info_to_tcax_with_syl(const SYL_pLine pLine, const SYL_pSyllable pSyllable, SYL_pInfoToTcax pInfoToTcax);

/**
 * Copy SYL_Line and SYL_Adv to SYL_InfoToTcx structure.
 * @param pLine pointer to SYL_Line structure 
 * @param pAdv pointer to SYL_Adv structure
 * @param pInfoToTcax pointer to SYL_InfoToTcax structure that is going to be filled with the copied data
 */
extern void libsyl_info_to_tcax_with_adv(const SYL_pLine pLine, const SYL_pAdv pAdv, SYL_pInfoToTcax pInfoToTcax);

/**
 * Free memory occupied by SYL_Line structure.
 * @param pLine the address of SYL_Line structure that is going to be freed
 */
extern void libsyl_free_line(SYL_pLine pLine);

/**
 * Free memory occupied by SYL_Word structure.
 * @param pWord the address of SYL_Word structure that is going to be freed
 */
extern void libsyl_free_word(SYL_pWord pWord);

/**
 * Free memory occupied by SYL_Syllable structure.
 * @param pSyllable the address of SYL_Syllable structure that is going to be freed
 */
extern void libsyl_free_syllable(SYL_pSyllable pSyllable);

/**
 * Free memory occupied by SYL_Adv structure.
 * @param pAdv the address of SYL_Adv structure that is going to be freed
 */
extern void libsyl_free_adv(SYL_pAdv pAdv);

/**
 * Free memory occupied by SYL_InfoToTcax structure.
 * @param pInfoToTcax the address of SYL_InfoToTcax structure that is going to be freed
 */
extern void libsyl_free_info(SYL_pInfoToTcax pInfoToTcax);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* LIBSYL_SYL_H */

