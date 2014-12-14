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

#ifndef PIXMAGICK_CONVERTION_H
#define PIXMAGICK_CONVERTION_H
#pragma once

#include "../tcax/tcaxLib/pix.h"
#include "std.h"

/**
 * PIXM_PyImage structure is a python version of ImageMagick's Image *.
 * The information contained in the py image is only the address, no more than an unsigned long variable, 
 * the real content is stored in the module, and is only reachable for the following function calls from 
 * pixMagick lib, not for direct access by python script, such as retrieving infomation from the py image.
 * And the memory should be cleaned by pixmagick_destroy_image function, once the function being called, 
 * the real content of py image in the memory are freed, and could no longer be accessed.
 */
typedef PyObject *PIXM_PyImage;


/* Inhibit C++ name-mangling for pixMagick functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * Convert a TCAX_PyPix object to PIXM_PyImage object.
 * @param self reserved
 * @param args (pix)
 * @return PIXM_PyImage
 */
extern PIXM_PyImage pixmagick_convert_py_pix(PyObject *self, PyObject *args);

/**
 * Convert a PIXM_PyImage object to TCAX_PyPix object.
 * @param self reserved
 * @param args (image)
 * @return PIXM_PyImage
 */
extern TCAX_PyPix pixmagick_convert_py_image(PyObject *self, PyObject *args);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif  /* PIXMAGICK_CONVERTION_H */

