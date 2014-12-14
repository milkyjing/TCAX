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

#ifndef LIBTCC_TCC_H
#define LIBTCC_TCC_H
#pragma once

#if defined WIN32
#include <windows.h>
#else
#include <locale.h>
#endif  /* WIN32 */

#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>

#if defined WIN32
#define __wcs_ignore_case_cmp _wcsicmp
#define __str_ignore_case_cmp _stricmp
#else
#define __wcs_ignore_case_cmp wcscasecmp
#define __str_ignore_case_cmp strcasecmp
#endif  /* WIN32 */

#define LIBTCC_VERSION 0x00006000    /**< High word of LIBTCC_VERSION indicates major libtcc version, and low word indicates minor version */
#define LIBTCC_VER_STR L"0.6"

#define TCC_VERSION 0x10000000
#define TCC_VER_STR L"1.0"

#define K_MODE_NORMAL 1
#define K_MODE_SYLLABLE 2
#define K_MODE_ADVANCED 3
#define K_MODE_WORD K_MODE_NORMAL
#define K_MODE_SYL  K_MODE_SYLLABLE
#define K_MODE_ADV  K_MODE_ADVANCED

#define TEXT_LAYOUT_HORI 1
#define TEXT_LAYOUT_VERT 2

/* TCC attribute macros */
/**< tcc file version */
#define TCC_FILE_VERSION    L"tcc file version"
/**< mode configuration */
#define INHERIT_ASS_HEADER  L"inherit ass header"
#define K_MODE              L"k mode"
#define TEXT_LAYOUT         L"text layout"
#define TCAXPY_INIT         L"tcaxpy init"
#define TCAXPY_USER         L"tcaxpy user"
#define TCAXPY_FIN          L"tcaxpy fin"
/**< py settings */
#define PY_FILE             L"py file"
#define BEG_LINE            L"beg line"
#define END_LINE            L"end line"
/**< main settings */
#define K_TIMED_ASS_FILE    L"k-timed ass file"
#define FONT_FILE           L"font file"
#define FONT_FACE_ID        L"font face id"
#define FONT_SIZE           L"font size"
#define FX_WIDTH            L"fx width"
#define FX_HEIGHT           L"fx height"
#define FX_FPS              L"fx fps"
#define ALIGNMENT           L"alignment"
#define X_OFFSET            L"x offset"
#define Y_OFFSET            L"y offset"
#define SPACING             L"spacing"
#define SPACE_SCALE         L"space scale"
/**< style settings */
#define FONT_FACE_NAME      L"font face name"
#define BORD                L"bord"
#define SHAD                L"shad"
#define PRIMARY_COLOR       L"primary color"
#define SECONDARY_COLOR     L"secondary color"
#define OUTLINE_COLOR       L"outline color"
#define BACK_COLOR          L"back color"
#define PRIMARY_ALPHA       L"primary alpha"
#define SECONDARY_ALPHA     L"secondary alpha"
#define OUTLINE_ALPHA       L"outline alpha"
#define BACK_ALPHA          L"back alpha"
/**< additional settings */
#define BLUR                L"blur"

#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#define __min(a,b)  (((a) < (b)) ? (a) : (b))


/**
 * TCC_Error_Code enumeration defines some common errors.
 */
typedef enum _tcc_error_code {
    tcc_error_success = 0,    /**< indicates a successful process */
    tcc_error_null_pointer,  /**< receive a null pointer which is not desired */
    tcc_error_file_not_found,  /**< can not find the target TCC file */
    tcc_error_file_cant_open,   /**< can not open the target TCC file */
    tcc_error_file_cant_create,  /**< can not create the target TCC file */
    tcc_error_file_while_reading,  /**< some error occurred while reading a TCC file */
    tcc_error_file_while_writing,    /**< some error occurred while writing to a TCC file */
    tcc_error_file_bad_format,    /**< the target TCC file's bad-formated */
    tcc_error_file_invalid    /**< the target TCC file's invalid */
} TCC_Error_Code;

/**
 * TCC_Attributes contains all attributes listed in TCC file
 */
typedef struct _tcc_attributes {
    /**< tcc file version */
    double tcc_file_version;
    /**< mode configuration */
    int inherit_ass_header;
    int k_mode;
    int text_layout;
    int tcaxpy_init;
    int tcaxpy_user;
    int tcaxpy_fin;
    /**< py settings */
    int py_file_count;    /**< this is not in TCC file */
    wchar_t **py_file;
    int *beg_line;
    int *end_line;
    /**< main settings */
    wchar_t *k_timed_ass_file;
    wchar_t *font_file;
    int font_face_id;
    int font_size;
    int fx_width;
    int fx_height;
    double fx_fps;
    int alignment;
    int x_offset;
    int y_offset;
    int spacing;
    double space_scale;
    /**< style settings */
    wchar_t *font_face_name;
    int bord;
    int shad;
    wchar_t primary_color[7];
    wchar_t secondary_color[7];
    wchar_t outline_color[7];
    wchar_t back_color[7];
    int primary_alpha;
    int secondary_alpha;
    int outline_alpha;
    int back_alpha;
    /**< additional settings */
    double blur;
} TCC_Attributes, *TCC_pAttributes;

/**
 * TCC_InfoToTcax contains all attributes that will pass to tcax module
 */
typedef struct _tcc_info_to_tcax {
    /**< main settings */
    /* wchar_t *k_timed_ass_file; */
    wchar_t *font_file;
    int font_face_id;
    int font_size;
    int fx_width;
    int fx_height;
    double fx_fps;
    int alignment;
    int x_offset;
    int y_offset;
    int spacing;
    double space_scale;
    /**< style settings */
    wchar_t *font_face_name;        /* can be set directly in the TCC file, or, if not set, can be assigned from libtm by tcax */
    int bord;
    int shad;
    wchar_t primary_color[7];
    wchar_t secondary_color[7];
    wchar_t outline_color[7];
    wchar_t back_color[7];
    int primary_alpha;
    int secondary_alpha;
    int outline_alpha;
    int back_alpha;
    /**< additional settings */
    double blur;
} TCC_InfoToTcax, *TCC_pInfoToTcax;


/* Inhibit C++ name-mangling for libtcc functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * Read a plain text file and convert to a unicode (little endian) string.
 * @param filename the filename of the target TCC file
 * @param pBuffer a pointer to a block of memory that is going to hold the TCC string (unicode little endian)
 * @param pCount the address of an int variable that is going to hold number of characters in the TCC string
 * @return TCC_Error_Code
 */
extern TCC_Error_Code libtcc_read_file_to_unicode(const char *filename, wchar_t **pBuffer, int *pCount);

/**
 * Write a TCC string (unicode little endian) to a utf-8 encoded plain text file.
 * @param filename the filename of the target TCC file
 * @param tccString the address of a block of memory that holds the TCC string
 * @param count number of characters in the TCC string
 * @return TCC_Error_Code
 */
extern TCC_Error_Code libtcc_write_tcc_string_to_file(const char *filename, const wchar_t *tccString, int count);

/**
 * Write a unicode (little endian) string to a plain text file.
 * @param filename the filename of the target TCC file
 * @param buffer the address of a block of memory that is going to write
 * @param count number of characters in the TCC string
 * @return TCC_Error_Code
 */
extern TCC_Error_Code libtcc_write_unicode_to_file(const char *filename, const wchar_t *buffer, int count);

/**
 * Check general format of TCC file. 
 * Remark: refer to the TCC File Format Specification to see what is a valid TCC file.
 *
 * @param tccString the target TCC string which is going to be checked
 * @param count number of characters in the TCC string
 * @return TCC_Error_Code
 */
extern TCC_Error_Code libtcc_check_format(const wchar_t *tccString, int count);

/**
 * Parse TCC string and convert to TCC_Attributes
 * @param tccString the target TCC string which is going to be parsed
 * @param count number of characters in the TCC string
 * @param pAttributes pointer to the TCC_Attributes structure that is going to hold the value of attributes
 * @return TCC_Error_Code
 */
extern TCC_Error_Code libtcc_parse_tcc_string(const wchar_t *tccString, int count, TCC_pAttributes pAttributes);

/**
 * Parse a TCC file and convert to TCC_Attributes
 * Remark: this function is a newer version, and a replacement of libtcc_read_file_to_unicode, libtcc_check_format, libtcc_parse_tcc_string functions.
 *
 * @param filename the filename of the target TCC file
 * @param pAttributes pointer to the TCC_Attributes structure that is going to hold the value of attributes
 * @return TCC_Error_Code
 */
extern TCC_Error_Code libtcc_parse_tcc_file(const char *filename, TCC_pAttributes pAttributes);

/**
 * Copy TCC attributes to TCC_InfoToTcax structure.
 * @param pAttributes pointer to the TCC_Attributes structure that holds the value of attributes
 * @param pInfoToTcax pointer to TCC_InfoToTcax structure that is going to hold the info which will be passed to tcax
 */
extern void libtcc_info_to_tcax(const TCC_pAttributes pAttributes, TCC_pInfoToTcax pInfoToTcax);

/**
 * Free memory that is occupied by TCC_Attributes structure
 * @param pAttributes pointer to TCC_Attributes structure which is going to be freed
 */
extern void libtcc_free_attributes(TCC_pAttributes pAttributes);

/**
 * Free memory that is occupied by TCC_InfoToTcax structure
 * @param pInfoToTcax pointer to TCC_InfoToTcax structure which is going to be freed
 */
extern void libtcc_free_info(TCC_pInfoToTcax pInfoToTcax);

/**
 * Convert TCC_Attributes structure to a TCC string.
 * @param pAttributes pointer to TCC_Attributes structure that is going to be converted
 * @param pTccString pointer to the address of a TCC string that is going to hold the converted data
 * @param pCount the address of an int variable that is going to hold number of characters in the TCC string
 */
extern void libtcc_convert_attributes_to_tcc_string(const TCC_pAttributes pAttributes, wchar_t **pTccString, int *pCount);

/**
 * Make an ASS header using TCC Attributes.
 * @param pAttributes pointer to TCC_Attributes structure
 * @param pAssHeader pointer to the address of a ASS Header string that is going to hold the converted data
 * @param pCount the address of an int variable that is going to hold number of characters in the ASS Header string
 */
extern void libtcc_make_ass_header(const TCC_pAttributes pAttributes, wchar_t **pAssHeader, int *pCount);

/**
 * Create a default TCC file.
 * @param filename filename of the target TCC file
 * @return TCC_Error_Code
 */
extern TCC_Error_Code libtcc_create_default_tcc_file(const char *filename);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* LIBTCC_TCC_H */

