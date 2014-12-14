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

#ifndef TCAXLIB_IMAGE_H
#define TCAXLIB_IMAGE_H
#pragma once

#include "pix.h"
#include "./libpng/png.h"
#pragma comment(lib, "./libpng/zlib.lib")
#pragma comment(lib, "./libpng/libpng15.lib")


#define PNG_SIG_BYTES 8


/* Inhibit C++ name-mangling for tcaxLib functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * Make TCAX PIX from image file (.png).
 * @param self reserved
 * @param args (filename) or (filename, width, height)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_get_pix_from_image(PyObject *self, PyObject *args);

/**
 * Save TCAX PIX to image file (.png).
 * @param self reserved
 * @param args (filename, PIX) or (filename, PIX, width, height)
 * @return TCAX_Py_Error_Code
 */
extern TCAX_Py_Error_Code tcaxlib_save_pix_to_image(PyObject *self, PyObject *args);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* TCAXLIB_IMAGE_H */

