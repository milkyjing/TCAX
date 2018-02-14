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

#ifndef LIBTM_TM_H
#define LIBTM_TM_H
#pragma once

#include "../libsyl/syl.h"
#include "../FreeType/ft2build.h"
#include FT_FREETYPE_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_IDS_H
#pragma comment(lib, "./FreeType/freetype29.lib")

#define LIBTM_VERSION 0x00006000    /**< High word of LIBTM_VERSION indicates major libtm version, and low word indicates minor version */
#define LIBTM_VER_STR L"0.6"

#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#define __min(a,b)  (((a) < (b)) ? (a) : (b))


/**
 * TM_Error_Code enumeration defines some common errors.
 */
typedef enum _tm_error_code {
    tm_error_success = 0,      /**< indicates a successful process */
    tm_error_null_pointer,     /**< receive a null pointer which is not desired */
    tm_error_file_not_found,   /**< can not find the target SYL file */
    tm_error_file_cant_open,   /**< can not open the target SYL file */
    tm_error_init_fail,        /**< failed to initialize the TM_Font structure */
    tm_error_invalid_text      /**< find an invalid text in SYL syllables */
} TM_Error_Code;

/**
 * TM_Font structure
 */
typedef struct _tm_font {
    char      *filename;    /**< font file name */
    FT_Library library;     /**< FT library */
    FT_Face    face;        /**< FT face */
    int        id;          /**< font face id */
    int        size;        /**< font size */
    wchar_t   *name;        /**< font face name */
    int        ascender;    /**< ascender of the font */
    int        descender;   /**< descender of the font */
    double     spaceScale;  /**< white space (including L' ' and L'　') scale */
} TM_Font, *TM_pFont;

/**
 * TM_TextHori structure - horizontal text metrics
 */
typedef struct _tm_text_hori {
    int  lines;             /**< number of lines of the input SYL file */
    int  texts;             /**< total number of texts (syllables) of the input SYL file */
    int *textCount1D;       /**< number of texts (syllables) in a karaoke line */
    int  spacing;           /**< spacing between texts */
    int *width2D;           /**< text width */
    int *height2D;          /**< text height */
    int *kerning2D;         /**< text kerning pair */
    int *advance2D;         /**< text advance */
    int *advanceDiff2D;     /**< the sum of text advances in an SYL line up to the specified text */
    int *length1D;          /**< the total length of texts in an SYL line */
    int *initX2D;           /**< the initial x-axis position of the left-most and upper-most of a text */
    int *initY2D;           /**< the initial y-axis position of the left-most and upper-most of a text */
    int *bearingY2D;        /**< text bearingY */
} TM_TextHori, *TM_pTextHori;

/**
 * TM_WordHori structure - horizontal word metrics
 */
typedef struct _tm_word_hori {
    int  lines;             /**< number of lines of the input SYL file */
    int  words;             /**< total number of words (`\k' tags) of the input SYL file */
    int *wordCount1D;       /**< number of words (`\k' tags) in a karaoke line */
    int  spacing;           /**< spacing between texts */
    int *width2D;           /**< word width */
    int *height2D;          /**< word height */
    int *kerning2D;         /**< word kerning pair */
    int *advance2D;         /**< word advance */
    int *advanceDiff2D;     /**< the sum of word advances in an SYL line up to the specified word */
    int *length1D;          /**< the total length of words in an SYL line */
    int *initX2D;           /**< the initial x-axis position of the left-most and upper-most of a word */
    int *initY2D;           /**< the initial y-axis position of the left-most and upper-most of a word */
    int *bearingY2D;        /**< word bearingY */
} TM_WordHori, *TM_pWordHori;

/**
 * TM_AdvHori structure - horizontal adv word metrics
 */
typedef struct _tm_adv_hori {
    int  lines;             /**< number of lines of the input SYL file */
    int  advs;              /**< total number of adv words of the input SYL file */
    int *advCount1D;        /**< number of adv words in a karaoke line */
    int  spacing;           /**< spacing between texts */
    int *width2D;           /**< adv word width */
    int *height2D;          /**< adv word height */
    int *kerning2D;         /**< adv word kerning pair */
    int *advance2D;         /**< adv word advance */
    int *advanceDiff2D;     /**< the sum of adv word advances in an SYL line up to the specified adv word */
    int *length1D;          /**< the total length of adv words in an SYL line */
    int *initX2D;           /**< the initial x-axis position of the left-most and upper-most of an adv word */
    int *initY2D;           /**< the initial y-axis position of the left-most and upper-most of an adv word */
    int *bearingY2D;        /**< adv word bearingY */
} TM_AdvHori, *TM_pAdvHori;

/**
 * TM_TextVert structure - vertical text metrics
 */
typedef struct _tm_text_vert {
    int  lines;             /**< number of lines of the input SYL file */
    int  texts;             /**< total number of texts (syllables) of the input SYL file */
    int *textCount1D;       /**< number of texts (syllables) in a karaoke line */
    int  spacing;           /**< spacing between texts */
    int *width2D;           /**< text width */
    int *height2D;          /**< text height */
    /* int *kerning2D; */         /**< text kerning pair (do not support vertical layout yet) */
    int *advance2D;         /**< text advance */
    int *advanceDiff2D;     /**< the sum of text advances in an SYL line up to the specified text */
    int *length1D;          /**< the total length of texts in an SYL line */
    int *initX2D;           /**< the initial x-axis position of the left-most and upper-most of a text */
    int *initY2D;           /**< the initial y-axis position of the left-most and upper-most of a text */
    int *bearingX2D;        /**< text bearingX */
} TM_TextVert, *TM_pTextVert;

/**
 * TM_WordVert structure - vertical word metrics
 */
typedef struct _tm_word_vert {
    int  lines;             /**< number of lines of the input SYL file */
    int  words;             /**< total number of words (`\k' tags) of the input SYL file */
    int *wordCount1D;       /**< number of words (`\k' tags) in a karaoke line */
    int  spacing;           /**< spacing between texts */
    int *width2D;           /**< word width */
    int *height2D;          /**< word height */
    /* int *kerning2D; */         /**< word kerning pair (do not support vertical layout yet) */
    int *advance2D;         /**< word advance */
    int *advanceDiff2D;     /**< the sum of word advances in an SYL line up to the specified word */
    int *length1D;          /**< the total length of words in an SYL line */
    int *initX2D;           /**< the initial x-axis position of the left-most and upper-most of a word */
    int *initY2D;           /**< the initial y-axis position of the left-most and upper-most of a word */
    int *bearingX2D;        /**< word bearingX */
} TM_WordVert, *TM_pWordVert;

/**
 * TM_AdvVert structure - vertical adv word metrics
 */
typedef struct _tm_adv_vert {
    int  lines;             /**< number of lines of the input SYL file */
    int  advs;              /**< total number of adv words of the input SYL file */
    int *advCount1D;        /**< number of adv words in a karaoke line */
    int  spacing;           /**< spacing between texts */
    int *width2D;           /**< adv word width */
    int *height2D;          /**< adv word height */
    /* int *kerning2D; */         /**< adv word kerning pair (do not support vertical layout yet) */
    int *advance2D;         /**< adv word advance */
    int *advanceDiff2D;     /**< the sum of adv word advances in an SYL line up to the specified adv word */
    int *length1D;          /**< the total length of adv words in an SYL line */
    int *initX2D;           /**< the initial x-axis position of the left-most and upper-most of an adv word */
    int *initY2D;           /**< the initial y-axis position of the left-most and upper-most of an adv word */
    int *bearingX2D;        /**< adv word bearingX */
} TM_AdvVert, *TM_pAdvVert;

/**
 * TM_InfoToTcaxHori structure
 */
typedef struct _tm_info_to_tcax_hori {
    /* TM_Font */
    int  ascender;          /**< ascender of the font */
    int  descender;         /**< descender of the font */
    /* can be one of TM_TextHori/TM_WordHori/TM_AdvHori */
    int  lines;             /**< number of lines of the input SYL file */
    int  count;             /**< total number of texts (syllables) of the input SYL file */
    int *count1D;           /**< number of texts (syllables)/words/advs in a karaoke line */
    /* int  spacing; */           /**< spacing between texts */
    int *width2D;           /**< text width */
    int *height2D;          /**< text height */
    int *kerning2D;         /**< text kerning pair */
    int *advance2D;         /**< text advance */
    int *advanceDiff2D;     /**< the sum of text advances in an SYL line up to the specified text */
    int *length1D;          /**< the total length of texts in an SYL line */
    int *initX2D;           /**< the initial x-axis position of the left-most and upper-most of a text */
    int *initY2D;           /**< the initial y-axis position of the left-most and upper-most of a text */
    int *bearingY2D;        /**< text bearingY */
} TM_InfoToTcaxHori, *TM_pInfoToTcaxHori;

/**
 * TM_InfoToTcaxVert structure
 */
typedef struct _tm_info_to_tcax_vert {
    /* TM_Font */
    int  fontSize;          /**< size of the font */
    /* can be one of TM_TextVert/TM_WordVert/TM_AdvVert */
    int  lines;             /**< number of lines of the input SYL file */
    int  count;             /**< total number of texts (syllables) of the input SYL file */
    int *count1D;           /**< number of texts (syllables)/words/advs in a karaoke line */
    /* int  spacing; */           /**< spacing between texts */
    int *width2D;           /**< text width */
    int *height2D;          /**< text height */
    int *advance2D;         /**< text advance */
    int *advanceDiff2D;     /**< the sum of text advances in an SYL line up to the specified text */
    int *length1D;          /**< the total length of texts in an SYL line */
    int *initX2D;           /**< the initial x-axis position of the left-most and upper-most of a text */
    int *initY2D;           /**< the initial y-axis position of the left-most and upper-most of a text */
    int *bearingX2D;        /**< text bearingX */
} TM_InfoToTcaxVert, *TM_pInfoToTcaxVert;


/* Inhibit C++ name-mangling for libtm functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * Initialize the TM_Font structure.
 * @param pFont pointer to TM_Font structure which is going to be initialized
 * @param filename font file name
 * @param id font face id
 * @param size font size
 * @param spaceScale white space (including L' ' and L'　') scale
 * @return TM_Error_Code
 */
extern TM_Error_Code libtm_init_font(TM_pFont pFont, const char *filename, int id, int size, double spaceScale);

/**
 * Fill the TM_TextHori structure.
 * @param pFont pointer to TM_Font structure
 * @param pSyllable pointer to SYL_Syllable structure
 * @param spacing spacing between texts
 * @param pTextHori pointer to TM_TextHori structure that is going to be filled
 * @return TM_Error_Code
 */
extern TM_Error_Code libtm_fill_tm_text_hori(const TM_pFont pFont, const SYL_pSyllable pSyllable, int spacing, TM_pTextHori pTextHori);

/**
 * Fill the TM_TextVert structure.
 * @param pFont pointer to TM_Font structure
 * @param pSyllable pointer to SYL_Syllable structure
 * @param spacing spacing between texts
 * @param pTextVert pointer to TM_TextVert structure that is going to be filled
 * @return TM_Error_Code
 */
extern TM_Error_Code libtm_fill_tm_text_vert(const TM_pFont pFont, const SYL_pSyllable pSyllable, int spacing, TM_pTextVert pTextVert);

/**
 * Fill the TM_WordHori structure.
 * @param pFont pointer to TM_Font structure
 * @param pWord pointer to SYL_Word structure
 * @param pTextHori pointer to TM_TextHori structure which had already been filled
 * @param pWordHori pointer to TM_WordHori structure that is going to be filled
 */
extern void libtm_fill_tm_word_hori(const TM_pFont pFont, const SYL_pWord pWord, const TM_pTextHori pTextHori, TM_pWordHori pWordHori);

/**
 * Fill the TM_WordVert structure.
 * @param pFont pointer to TM_Font structure
 * @param pWord pointer to SYL_Word structure
 * @param pTextVert pointer to TM_TextVert structure which had already been filled
 * @param pWordVert pointer to TM_WordVert structure that is going to be filled
 */
extern void libtm_fill_tm_word_vert(const TM_pFont pFont, const SYL_pWord pWord, const TM_pTextVert pTextVert, TM_pWordVert pWordVert);

/**
 * Fill the TM_AdvHori structure.
 * @param pFont pointer to TM_Font structure
 * @param pAdv pointer to SYL_Adv structure
 * @param pTextHori pointer to TM_TextHori structure which had already been filled
 * @param pAdvHori pointer to TM_AdvHori structure that is going to be filled
 */
extern void libtm_fill_tm_adv_hori(const TM_pFont pFont, const SYL_pAdv pAdv, const TM_pTextHori pTextHori, TM_pAdvHori pAdvHori);

/**
 * Fill the TM_AdvVert structure.
 * @param pFont pointer to TM_Font structure
 * @param pAdv pointer to SYL_Adv structure
 * @param pTextVert pointer to TM_TextVert structure which had already been filled
 * @param pAdvVert pointer to TM_AdvVert structure that is going to be filled
 */
extern void libtm_fill_tm_adv_vert(const TM_pFont pFont, const SYL_pAdv pAdv, const TM_pTextVert pTextVert, TM_pAdvVert pAdvVert);

/**
 * Copy TM_Font and TM_TextHori to TM_InfoToTcaxHori structure.
 * @param pFont pointer to TM_Font structure
 * @param pTextHori pointer to TM_TextHori structure
 * @param pInfoToTcax pointer to TM_InfoToTcaxHori structure that is going to be filled with the copied data
 */
extern void libtm_info_to_tcax_with_text_hori(const TM_pFont pFont, const TM_pTextHori pTextHori, TM_pInfoToTcaxHori pInfoToTcax);

/**
 * Copy TM_Font and TM_TextVert to TM_InfoToTcaxVert structure.
 * @param pFont pointer to TM_Font structure
 * @param pTextVert pointer to TM_TextVert structure
 * @param pInfoToTcax pointer to TM_InfoToTcaxVert structure that is going to be filled with the copied data
 */
extern void libtm_info_to_tcax_with_text_vert(const TM_pFont pFont, const TM_pTextVert pTextVert, TM_pInfoToTcaxVert pInfoToTcax);

/**
 * Copy TM_Font and TM_TextHori to TM_InfoToTcaxHori structure.
 * @param pFont pointer to TM_Font structure
 * @param pWordHori pointer to TM_WordHori structure
 * @param pInfoToTcax pointer to TM_InfoToTcaxHori structure that is going to be filled with the copied data
 */
extern void libtm_info_to_tcax_with_word_hori(const TM_pFont pFont, const TM_pWordHori pWordHori, TM_pInfoToTcaxHori pInfoToTcax);

/**
 * Copy TM_Font and TM_TextVert to TM_InfoToTcaxVert structure.
 * @param pFont pointer to TM_Font structure
 * @param pWordVert pointer to TM_WordVert structure
 * @param pInfoToTcax pointer to TM_InfoToTcaxVert structure that is going to be filled with the copied data
 */
extern void libtm_info_to_tcax_with_word_vert(const TM_pFont pFont, const TM_pWordVert pWordVert, TM_pInfoToTcaxVert pInfoToTcax);

/**
 * Copy TM_Font and TM_TextHori to TM_InfoToTcaxHori structure.
 * @param pFont pointer to TM_Font structure
 * @param pAdvHori pointer to TM_AdvHori structure
 * @param pInfoToTcax pointer to TM_InfoToTcaxHori structure that is going to be filled with the copied data
 */
extern void libtm_info_to_tcax_with_adv_hori(const TM_pFont pFont, const TM_pAdvHori pAdvHori, TM_pInfoToTcaxHori pInfoToTcax);

/**
 * Copy TM_Font and TM_TextVert to TM_InfoToTcaxVert structure.
 * @param pFont pointer to TM_Font structure
 * @param pAdvVert pointer to TM_AdvVert structure
 * @param pInfoToTcax pointer to TM_InfoToTcaxVert structure that is going to be filled with the copied data
 */
extern void libtm_info_to_tcax_with_adv_vert(const TM_pFont pFont, const TM_pAdvVert pAdvVert, TM_pInfoToTcaxVert pInfoToTcax);

/**
 * Free memory occupied by the TM_Font structure.
 * @param pFont pointer to TM_Font structure which is going to be freed
 */
extern void libtm_free_font(TM_pFont pFont);

/**
 * Free memory occupied by the TM_TextHori structure.
 * @param pTextHori pointer to TM_TextHori structure which is going to be freed
 */
extern void libtm_free_tm_text_hori(TM_pTextHori pTextHori);

/**
 * Free memory occupied by the TM_TextVert structure.
 * @param pTextVert pointer to TM_TextVert structure which is going to be freed
 */
extern void libtm_free_tm_text_vert(TM_pTextVert pTextVert);

/**
 * Free memory occupied by the TM_WordHori structure.
 * @param pWordHori pointer to TM_WordHori structure which is going to be freed
 */
extern void libtm_free_tm_word_hori(TM_pWordHori pWordHori);

/**
 * Free memory occupied by the TM_WordVert structure.
 * @param pWordVert pointer to TM_WordVert structure which is going to be freed
 */
extern void libtm_free_tm_word_vert(TM_pWordVert pWordVert);

/**
 * Free memory occupied by the TM_AdvHori structure.
 * @param pAdvHori pointer to TM_AdvHori structure which is going to be freed
 */
extern void libtm_free_tm_adv_hori(TM_pAdvHori pAdvHori);

/**
 * Free memory occupied by the TM_AdvVert structure.
 * @param pAdvVert pointer to TM_AdvVert structure which is going to be freed
 */
extern void libtm_free_tm_adv_vert(TM_pAdvVert pAdvVert);

/**
 * Free memory occupied by the TM_InfoToTcaxHori structure.
 * @param pInfoToTcax pointer to TM_InfoToTcaxHori structure which is going to be freed
 */
extern void libtm_free_info_hori(TM_pInfoToTcaxHori pInfoToTcax);

/**
 * Free memory occupied by the TM_InfoToTcaxVert structure.
 * @param pInfoToTcax pointer to TM_InfoToTcaxVert structure which is going to be freed
 */
extern void libtm_free_info_vert(TM_pInfoToTcaxVert pInfoToTcax);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* LIBTM_TM_H */

