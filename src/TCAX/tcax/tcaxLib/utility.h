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

#ifndef TCAXLIB_UTILITY_H
#define TCAXLIB_UTILITY_H
#pragma once

//#include "pix.h"
#include "text.h"


/**
 * TCAX_String structure is an auto length increase string.
 */
typedef struct _tcax_string {
    wchar_t *buf;
    int len;
    int capacity;
} TCAX_String, *TCAX_pString;

/**
 * TCAX_OutlineState structure is used in tcaxlib_get_text_outline_as_string function().
 */
typedef struct _tcax_outline_string {
    TCAX_String string;    /**< string that contains the drawing commands */
    FT_Vector last;        /**< last point position */
    FT_Pos height;         /**< virtual height of the text, not the real height, used to convert the orientation */
    FT_Vector offset;      /**< translate the position */
} TCAX_OutlineString, *TCAX_pOutlineString;

/**
 * TCAX_PyString structure that is in fact a Python unicode object.
 */
typedef PyObject *TCAX_PyString;

/**
 * TCAX_PyLong structure that is in fact a Python integer object.
 */
typedef PyObject *TCAX_PyLong;

/**
 * TCAX_PyFloat structure that is in fact a Python float object.
 */
typedef PyObject *TCAX_PyFloat;


/* Inhibit C++ name-mangling for tcaxLib functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */
    
/**
 * Get the text outline (which is consisted of contours, lines, bezier arcs) as a string of ASS drawing commands.
 * @param self reserved
 * @param args (pyFont, text, x, y) or (font_file, face_id, font_size, text, x, y)
 * @return TCAX_PyString
 */
extern TCAX_PyString tcaxlib_get_text_outline_as_string(PyObject *self, PyObject *args);

/**
 * Check if a text string is a CJK string. 
 * Remark: here CJK means "Chinese or Japanese or Korea" not "Chinese & Japanese & Korea".
 *
 * @param self reserved
 * @param args (text)
 * @return TCAX_Py_Error_Code
 */
extern TCAX_Py_Error_Code tcaxlib_is_c_or_j_or_k(PyObject *self, PyObject *args);

/**
 * Change to vertical text layout.
 * @param self reserved
 * @param args (text)
 * @return TCAX_PyString
 */
extern TCAX_PyString tcaxlib_vertical_layout_ass(PyObject *self, PyObject *args);

/**
 * Show progress.
 * @param self reserved
 * @param args (total, completed, file_id, file_num)
 * @return TCAX_Py_Error_Code
 */
extern TCAX_Py_Error_Code tcaxlib_show_progress(PyObject *self, PyObject *args);

/**
 * Linear Bezier Curve.
 * @param self reserved
 * @param args (nPoints, xs, ys, xe, ye)
 * @return TCAX_PyPoints
 */
extern TCAX_PyPoints tcaxlib_bezier_curve_linear(PyObject *self, PyObject *args);

/**
 * Quadratic Bezier Curve.
 * @param self reserved
 * @param args (nPoints, xs, ys, xe, ye, xc, yc)
 * @return TCAX_PyPoints
 */
extern TCAX_PyPoints tcaxlib_bezier_curve_quadratic(PyObject *self, PyObject *args);

/**
 * Cubic Bezier Curve.
 * @param self reserved
 * @param args (nPoints, xs, ys, xe, ye, xc1, yc1, xc2, yc2)
 * @return TCAX_PyPoints
 */
extern TCAX_PyPoints tcaxlib_bezier_curve_cubic(PyObject *self, PyObject *args);

/**
 * Nth Order Bezier Curve with random control points.
 * @param self reserved
 * @param args (nPoints, xs, ys, xe, ye, xl1, yl1, xl2, yl2, order)
 * @return TCAX_PyPoints
 */
extern TCAX_PyPoints tcaxlib_bezier_curve_random(PyObject *self, PyObject *args);

/**
 * Calculate the actual font size used in tcaxLib and util.magick modules.
 * @param self reserved
 * @param args (font_file, face_id, font_size)
 * @return TCAX_PyFloat
 */
extern TCAX_PyFloat tcaxlib_get_actual_font_size(PyObject *self, PyObject *args);

/**
 * Calculate a correct font size for cairo from GDI font size.
 * @param self reserved
 * @param args (font_file, face_id, font_size)
 * @return TCAX_PyLong
 */
extern TCAX_PyLong tcaxlib_calc_cairo_font_size(PyObject *self, PyObject *args);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* TCAXLIB_UTILITY_H */

