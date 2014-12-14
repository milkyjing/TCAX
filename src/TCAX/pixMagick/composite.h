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

#ifndef PIXMAGICK_COMPOSITE_H
#define PIXMAGICK_COMPOSITE_H
#pragma once

#include "convertion.h"


/* Inhibit C++ name-mangling for pixMagick functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * Composite two iamges.
 * @param self reserved
 * @param args (image, compose, composite_image, x_offset, y_offset)
 * @return PIXM_PyImage
 */
extern PIXM_PyImage pixmagick_composite_image(PyObject *self, PyObject *args);


#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif  /* PIXMAGICK_COMPOSITE_H */

