/* 
 *  Copyright (C) 2009-2011 milkyjing <milkyjing@gmail.com>
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

#ifndef TCAXLIB_FILE_H
#define TCAXLIB_FILE_H
#pragma once

#include "std.h"
#include "tcasfunc.h"


/**
 * TCAX_AssFile structure
 */
typedef struct _tcax_ass_file {
    FILE *fp;
} TCAX_AssFile, *TCAX_pAssFile;

/**
 * TCAX_TcasFile structure
 */
typedef struct _tcax_tcas_file {
    TCAS_File file;
    TCAS_Header header;
} TCAX_TcasFile, *TCAX_pTcasFile;

/**
 * TCAX_PyAssFile structure that has the following form (TCAX_pAssFile)
 */
typedef PyObject *TCAX_PyAssFile;

/**
 * TCAX_PyTcasFile structure that has the following form (TCAX_pTcasFile)
 */
typedef PyObject *TCAX_PyTcasFile;


/* Inhibit C++ name-mangling for tcaxLib functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * Create an ASS file and returns the handler to the file.
 * @param self reserved
 * @param args (ass_file, ass_header)
 * @return TCAX_PyAssFile;
 */
extern TCAX_PyAssFile tcaxlib_create_ass_file(PyObject *self, PyObject *args);

/**
 * Append to an existing ASS file and returns the handler to the file.
 * @param self reserved
 * @param args (ass_file)
 * @return TCAX_PyAssFile;
 */
extern TCAX_PyAssFile tcaxlib_append_ass_file(PyObject *self, PyObject *args);

/**
 * Write ASS strings to ASS file.
 * @param self reserved
 * @param args (pyAssFile, ASS_BUF)
 * @return TCAX_Py_Error_Code;
 */
extern TCAX_Py_Error_Code tcaxlib_write_ass_file(PyObject *self, PyObject *args);

/**
 * Finalize the ASS file (close the handler to the file and destroy contents assigned to it).
 * @param self reserved
 * @param args (pyAssFile)
 * @return TCAX_Py_Error_Code;
 */
extern TCAX_Py_Error_Code tcaxlib_fin_ass_file(PyObject *self, PyObject *args);

/**
 * Create an TCAS file and returns the handler to the file.
 * @param self reserved
 * @param args (tcas_file, fx_width, fx_height, fx_fps)
 * @return TCAX_PyTcasFile;
 */
extern TCAX_PyTcasFile tcaxlib_create_tcas_file(PyObject *self, PyObject *args);

/**
 * Write TCAS buffer to TCAS file.
 * @param self reserved
 * @param args (pyTcasFile, tcas_buf)
 * @return TCAX_Py_Error_Code;
 */
extern TCAX_Py_Error_Code tcaxlib_write_tcas_file(PyObject *self, PyObject *args);

/**
 * Finalize the TCAS file (close the handler to the file and destroy contents assigned to it).
 * @param self reserved
 * @param args (pyTcasFile)
 * @return TCAX_Py_Error_Code;
 */
extern TCAX_Py_Error_Code tcaxlib_fin_tcas_file(PyObject *self, PyObject *args);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* TCAXLIB_FILE_H */

