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

#ifndef TCAX_TCAX_H
#define TCAX_TCAX_H
#pragma once

#include "./libtcc/tcc.h"
#include "./libsyl/syl.h"
#include "./libtm/tm.h"
#include "./tcaxpy/py.h"
//#include "../../../../../libtcas/trunk/src/alpha/tcas/tcas.h"
#include "../../../../../libtcas/trunk/src/alpha/tcas/hla_z_comp.h"

#pragma comment(lib, "./FreeType/freetype248.lib")

#ifndef WIN32
#include <unistd.h>
#endif  /* WIN32 */

#if defined WIN32
#define __wcs_ignore_case_cmp _wcsicmp
#define __str_ignore_case_cmp _stricmp
#else
#define __wcs_ignore_case_cmp wcscasecmp
#define __str_ignore_case_cmp strcasecmp
#endif  /* WIN32 */


/* Inhibit C++ name-mangling for tcax functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * Get tcax's root directory.
 * @param pDirectory pointer to the address of the directory string
 * @return pointer to directory
 */
extern const char *tcax_get_root_directory(char **pDirectory);

/**
 * Get TCC file's directory (the working directory)
 * @param tccFilename filename of  the target TCC filename
 * @param pTccDirectory pointer to the directory of the folder that contains the target TCC file
 * @return tccDirectory
 */
extern const char *tcax_get_tcc_directory(const char *tccFilename, char **pTccDirectory);

/**
 * Make the output filename from tcc filename
 * @param tccFilename filename of te target TCC file
 * @param pOutFilename pointer to output filename
 * @return outFilename
 */
extern const char *tcax_make_out_filename_from_tcc(const char *tccFilename, char **pOutFilename);

/**
 * Make the output ASS filename.
 * @param outFilename filename of the output files
 * @param pOutAssFilename pointer to the output ASS filename
 * @return outAssFilename
 */
extern const char *tcax_get_out_ass_filename(const char *outFilename, char **pOutAssFilename);

/**
 * Make the output TCAS filename.
 * @param outFilename filename of the output files
 * @param pOutTcasFilename pointer to the output TCAS filename
 * @return outTcasFilename
 */
extern const char *tcax_get_out_tcas_filename(const char *outFilename, char **pOutTcasFilename);

/**
 * Get TCC attributes from TCC file.
 * @param tccFilename filename of the target tcc file
 * @param pAttributes pointer to TCC_Attributes structure that is going to be filled
 * @return int
 */
extern int tcax_get_tcc_attributes(const char *tccFilename, TCC_pAttributes pAttributes);

/**
 * Get SYL line and word SYL file.
 * @param sylFilename filename of the target SYL file
 * @param pLine pointer to SYL_Line structure that is going to be filled
 * @param pWord pointer to SYL_Word structure that is going to be filled
 * @return int
 */
extern int tcax_get_syl_line_and_word(const char *sylFilename, SYL_pLine pLine, SYL_pWord pWord);

/**
 * Initialize the PY_InitData structure.
 * @param tccFilename filename of the target tcc file
 * @param sylFilename filename of the target SYL file
 * @param pAttributes pointer to TCC_Attributes structure that is going to be filled
 * @param pInitData pointer to PY_InitData structure that is going to be filled
 * @return int
 */
extern int tcax_init_tcaxpy_data(const char *tccFilename, const char *outFilename, TCC_pAttributes pAttributes, PY_pInitData pInitData);

/**
 * Write ASS string (unicode) to ASS file (utf-8).
 * @param assfp pointer to FILE structure
 * @param assString the target ASS string (unicode) that is going to be written to file
 * @param count number of characters of the target ASS string
 * @return int
 */
extern int tcax_write_ass_string_to_file(FILE *assfp, const wchar_t *assString, int count);

/**
 * Write TCAS buffer (raw) to TCAS file (compressed).
 * @param pFile pointer to TCAS_File structure
 * @param pHeader pointer to TCAS_Headers structure
 * @param tcasBuffer the target TCAS buffer (raw) that is going to be written to file
 * @param count number of raw chunks of the target TCAS buffer
 * @return int
 */
extern int tcax_write_tcas_buffer_to_file(TCAS_pFile pFile, TCAS_pHeader pHeader, const tcas_unit *tcasBuffer, int count);

/**
 * Get initial x position value of the first text in an SYL line.
 * @param pAttributes pointer to TCC_Attributes structure
 * @param pInfoToTcax pointer to TM_InfoToTcax structure
 * @param iLine the line index which want to get its initial x position
 * @return int
 */
extern int tcax_get_line_init_pos_x(const TCC_pAttributes pAttributes, const PY_pInitData pInitData, int iLine);

/**
 * Get initial y position value of the first text in an SYL line.
 * @param pAttributes pointer to TCC_Attributes structure
 * @param pInfoToTcax pointer to TM_InfoToTcax structure
 * @param iLine the line index which want to get its initial y position
 * @return int
 */
extern int tcax_get_line_init_pos_y(const TCC_pAttributes pAttributes, const PY_pInitData pInitData, int iLine);

/**
 * Free memory occupied by PY_InitData structure.
 * @param pInitData pointer to PY_InitData structure that is going to be freed
 */
extern void tcax_free_tcaxpy_data(PY_pInitData pInitData);

/**
 * Main function.
 * @param tccFilename filename of the target tcc file
 * @param outFilename filename of the output file
 * @return int
 */
extern int tcax_entry(const char *tccFilename, const char *outFilename);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* TCAX_TCAX_H */

