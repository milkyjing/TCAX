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

#ifndef TCAXLIB_TCASFUNC_H
#define TCAXLIB_TCASFUNC_H
#pragma once

#include "pix.h"
//#include "../../../../../../libtcas/trunk/src/alpha/tcas/tcas.h"
//#include "../../../../../../libtcas/trunk/src/alpha/tcas/hla_z_comp.h"
#include "../../../../../../libtcas/trunk/src/alpha/tcas/hla_stream_parser.h"

#define _MAKE16B16B(x, y) ((unsigned long)(((unsigned short)(x)) | (((unsigned long)(y)) << 16)))
#define _MAKELTP _MAKE16B16B


/* Inhibit C++ name-mangling for tcaxLib functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * Append PIX to TCAS list (TCAS_BUF).
 * @param self reserved
 * @param args (TCAS_BUF, pix, start, end, offsetX, offsetY, layer)
 * @return TCAX_Py_Error_Code
 */
extern TCAX_Py_Error_Code tcaxlib_tcas_list_append_pix(PyObject *self, PyObject *args);

/**
 * Append a pair of key frame PIXs to TCAS list (TCAS_BUF).
 * @param self reserved
 * @param args (TCAS_BUF, pix_start, pix_end, start, end, offsetX, offsetY, type, layer)
 * @return TCAX_Py_Error_Code
 */
extern TCAX_Py_Error_Code tcaxlib_tcas_list_append_key_pixs(PyObject *self, PyObject *args);

extern void tcaxlib_convert_tcas_list_to_buf(PyObject *tcasList, tcas_unit **pTcasBuf, tcas_u32 *pCount);

/**
 * Parse non-linear TCAS_BUF to linear TCAS_BUF.
 * @param self reserved
 * @param args (TCAS_BUF, width, height, fps, layer)
 * @return TCAS_BUF
 */
extern PyObject *tcaxlib_tcas_list_parse(PyObject *self, PyObject *args);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* TCAXLIB_TCASFUNC_H */

