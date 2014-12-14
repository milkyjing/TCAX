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

#ifndef TCAXPY_PY_H
#define TCAXPY_PY_H
#pragma once

#include <Python.h>

#if defined WIN32
#include <windows.h>
#else
#include <locale.h>
#endif  /* WIN32 */

#define TCAXPY_VERSION 0x00006000    /**< High word of TCAXPY_VERSION indicates major tcaxpy version, and low word indicates minor version */
#define TCAXPY_VER_STR L"0.6"

/* tcaxPy.py macros */
#define TCAXPY_PY_LIB_FOLDER "Lib"
#define TCAXPY_PY_MODULE_NAME "tcaxPy"
#define TCAXPY_PY_INIT_DATA "tcaxPy_InitData"
#define TCAXPY_PY_FUNC_INIT "tcaxPy_Init"
#define TCAXPY_PY_FUNC_USER "tcaxPy_User"
#define TCAXPY_PY_FUNC_MAIN "tcaxPy_Main"
#define TCAXPY_PY_FUNC_FIN "tcaxPy_Fin"

/**
 * PY_Error_Code enumeration defines some common errors.
 */
typedef enum _py_error_code {
    py_error_success = 0,           /**< indicates a successful process */
    py_error_null_pointer,          /**< receive a null pointer which is not desired */
    py_error_file_not_found,        /**< can not find the target file */
    py_error_file_cant_open,        /**< can not open the target file */
    py_error_file_cant_create,      /**< can not create the target file */
    py_error_file_while_reading,    /**< some error occurred while reading a py file */
    py_error_file_while_writing,    /**< some error occurred while writing to a py file */
    py_error_file_invalid_script,   /**< the py script is invalid (contains some errors) */
    py_error_file_modified,         /**< indicate whether a py script has been modified */
    py_error_init_fail,             /**< failed to initialize the Python Interpreter */
    py_error_exec_fail,             /**< failed to execute a certain script or function, tcaxpy_exec_py_script, tcaxpy_script_func_init, tcaxpy_script_func_user, tcaxpy_script_func_fin  */
    py_error_fin_fail               /**< failed to finalize the Python Interpreter */
} PY_Error_Code;

/**
 * PY_TcaxPy structure
 */
typedef struct _py_tcaxpy {
    PyObject *pyBaseModule;    /**< pointer to tcaxPy.py */
    PyObject *pyUserModule;    /**< pointer to user tcax py script */
    PyObject *pyInitFunc;      /**< pointer to function tcaxPy_Init() of user tcax py script */
    PyObject *pyUserFunc;      /**< pointer to function tcaxPy_User() of user tcax py script */
    PyObject *pyMainFunc;      /**< pointer to function tcaxPy_Main() of user tcax py script */
    PyObject *pyFinFunc;       /**< pointer to function tcaxPy_Fin() of user tcax py script */
} PY_TcaxPy, *PY_pTcaxPy;

/**
 * PY_TccData structure
 */
typedef struct _py_tcc_data {
    /**< main settings */
    /* wchar_t *k_timed_ass_file */
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
} PY_TccData, *PY_pTccData;

/**
 * PY_SylData structure
 */
typedef struct _py_syl_data {
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
} PY_SylData, *PY_pSylData;

/**
 * PY_TmHoriData structure
 */
typedef struct _py_tm_hori_data {
    /* TM_Font */
    int  ascender;          /**< ascender of the font */
    int  descender;         /**< descender of the font */
    /* can be one of TM_TextHori/TM_WordHori /TM_AdvHori */
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
} PY_TmHoriData, *PY_pTmHoriData;

/**
 * PY_TmVertData structure
 */
typedef struct _py_tm_vert_data {
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
} PY_TmVertData, *PY_pTmVertData;

/**
 * PY_InitData structure
 */
typedef struct _py_init_data {
    char *directory;              /**< root directory of the program */
    wchar_t *outFilename;         /**< filename of the output file */
    wchar_t *assHeader;           /**< ASS header */
    PY_TccData tccData;           /**< tcc info */
    PY_SylData sylData;           /**< syl info */
    PY_TmHoriData tmHoriData;     /**< horizontal tm info */
    PY_TmVertData tmVertData;     /**< vertical tm info */
} PY_InitData, *PY_pInitData;


/* Inhibit C++ name-mangling for tcaxpy functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * Convert a file (ansi/unicode/unicode big endian) to utf-8 encoded. 
 * Remark: this function is system specific, ie., you need different implementation on Linux and Windows.
 *
 * @param filename filename of the target file that is going to be converted
 * @return PY_Error_Code
 */
extern PY_Error_Code tcaxpy_convert_file_to_utf8(const char *filename);

/**
 * Initialize the Python Interpreter.
 * @return PY_Error_Code
 */
extern PY_Error_Code tcaxpy_init_python();

/**
 * Finalize the Python Interpreter.
 * @return PY_Error_Code
 */
extern PY_Error_Code tcaxpy_fin_python();

/**
 * Execute a py script.
 * @param filename filename of the target file that is going to be executed
 * @return PY_Error_Code
 */
extern PY_Error_Code tcaxpy_exec_py_script(const char *filename);

/* tcax py token file functions */

/**
 * Make a tcax py token filename from tcax py filename.
 * @param filename the filename of the target py file that is going to have a new token
 * @param pTokenFilename pointer to the address of a block of memory that is going to hold the token filename
 * @return tokenFilename
 */
extern const char *tcaxpy_make_py_token_filename(const char *filename, char **pTokenFilename);

/**
 * Create a tcax py token file.
 * Remark: this function is system specific, only availabel on Windows.
 *
 * @param filename the filename of the target py token file that is going to be created
 * @return PY_Error_Code
 */
extern PY_Error_Code tcaxpy_create_py_token(const char *filename);

/**
 * Synchronize a tcax py token file with its host py file.
 * Remark: this function is system specific, only availabel on Windows.
 *
 * @param filename the filename of the target py file
 * @param tokenFilename the filename of the target py token file, if tokenFilename is NULL or the token file specified by tokenFilename is not existed, then the token file will be created
 * @return PY_Error_Code
 */
extern PY_Error_Code tcaxpy_synchronize_py_token(const char *filename, const char *tokenFilename);

/**
 * Check to see if a tcax py script has been modified.
 * Remark: this function is system specific, only availabel on Windows.
 *
 * @param filename the filename of the target py file
 * @param tokenFilename the filename of the target py token file, if tokenFilename is NULL or the token file specified by tokenFilename is not existed, then the token file will be created, and the function returns py_error_file_modified
 * @return PY_Error_Code
 */
extern PY_Error_Code tcaxpy_is_py_modified(const char *filename, const char *tokenFilename);

/* main functions */

/**
 * Create a user defined tcax py script template.
 * Remark: this function is system specific, ie., you need different implementation on Linux and Windows.
 *
 * @param filename filename of the target py file
 * @return PY_Error_Code
 */
extern PY_Error_Code tcaxpy_create_py_template(const char *filename);

/* main functions */

/**
 * Initialize the base py module (tcaxPy.py).
 * @param pTcaxPy pointer to the PY_TcaxPy structure which is going to be initialized
 * @param pInitData pointer to PY_InitData structure which is used to initialize the tcaxPy module
 * @return PY_Error_Code
 */
extern PY_Error_Code tcaxpy_init_base_py_module(PY_pTcaxPy pTcaxPy, const PY_pInitData pInitData);

/**
 * Initialize the user tcax py module.
 * @param pTcaxPy pointer to the PY_TcaxPy structure which is going to be initialized
 * @param directory the root directory of the program
 * @param userPyFilename the filename of the user tcax py script
 * @param use_tcaxpy_init_func indicate if to use the function tcaxPy_Init() in the user tcax py script
 * @param use_tcaxpy_user_func indicate if to use the function tcaxPy_User() in the user tcax py script
 * @param use_tcaxpy_fin_func indicate if to use the function tcaxPy_Fin() in the user tcax py script
 * @return PY_Error_Code
 */
extern PY_Error_Code tcaxpy_init_user_py_module(PY_pTcaxPy pTcaxPy, const char *directory, const char *userPyFilename, int use_tcaxpy_init_func, int use_tcaxpy_user_func, int use_tcaxpy_fin_func);

/**
 * The interface of tcax to the function tcaxPy_Init in user tcax py script.
 * @param pTcaxPy pointer to PY_TcaxPy structure
 * @return PY_Error_Code
 */
extern PY_Error_Code tcaxpy_script_func_init(const PY_pTcaxPy pTcaxPy);

/**
 * The interface of tcax to the function tcaxPy_User in user tcax py script.
 * @param pTcaxPy pointer to PY_TcaxPy structure
 * @return PY_Error_Code
 */
extern PY_Error_Code tcaxpy_script_func_user(const PY_pTcaxPy pTcaxPy);

/**
 * The interface of tcax to the function tcaxPy_Main in user tcax py script.
 * @param pTcaxPy pointer to PY_TcaxPy structure
 * @param iLine SYL line index
 * @param iText text index in the SYL line
 * @param nTexts number of texts in the SYL line
 * @param start the beginning time of the SYL line
 * @param end the end time of the SYL line
 * @param timeDiff sum of karaoke time values up to the text
 * @param time karaoke time of the text
 * @param x position x of the text
 * @param y position y of the text
 * @param advance advance of the text
 * @param text the text string
 * @return void **
 */
extern void **tcaxpy_script_func_main(const PY_pTcaxPy pTcaxPy, int iLine, int iText, int nTexts, int start, int end, int timeDiff, int time, int x, int y, int advance, const wchar_t *text);

/**
 * The interface of tcax to the function tcaxPy_Fin in user tcax py script.
 * @param pTcaxPy pointer to PY_TcaxPy structure
 * @return PY_Error_Code
 */
extern PY_Error_Code tcaxpy_script_func_fin(const PY_pTcaxPy pTcaxPy);

/**
 * Finalize PY_TcaxPy structure.
 * @param pTcaxPy pointer to PY_TcaxPy structure that is going to be finalized
 */
extern void tcaxpy_fin_tcaxpy(PY_pTcaxPy pTcaxPy);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* TCAXPY_PY_H */

