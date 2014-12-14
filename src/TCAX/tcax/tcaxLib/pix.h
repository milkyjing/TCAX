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

#ifndef TCAXLIB_PIX_H
#define TCAXLIB_PIX_H
#pragma once

#include "std.h"


#define MATH_PI 3.14159265358979323846264338327950288

#define MAKE8B8B8B(r,g,b) ((unsigned long)((unsigned char)(r) | ((unsigned short)((unsigned char)(g)) << 8) | ((unsigned long)((unsigned char)(b)) << 16)))
#define MAKERGB MAKE8B8B8B


/**
 * TCAX_Pix structure used to store pixel info of text(s) / image(s).
 */
typedef struct _tcax_pix {
    double initX;    /**< left most */
    double initY;    /**< upper most */
    int width;     /**< width of the PIX */
    int height;    /**< height of the PIX */
    int size;    /**< size = height * (width * 4) */
    unsigned char *buf;    /**< pixel values of the PIX */
} TCAX_Pix, *TCAX_pPix;

/**
 * TCAX_Points structure is an auto increase list.
 */
typedef struct _tcax_points {
    double *xBuf;
    double *yBuf;
    unsigned char *aBuf;
    int count;       /**< number of points */
    int capacity;    /**< maximun points that the buffer can contain */
} TCAX_Points, *TCAX_pPoints;

/**
 * TCAX_PyPix structure that has the following structure ((initX, initY), (width, height), (a1, a2, a3, ...)).
 */
typedef PyObject *TCAX_PyPix;

/**
 * TCAX_PyBigPix structure that has the following structure [(PIX, offset_x, offset_y, layer), (PIX, offset_x, offset_y, layer), ...].
 */
typedef PyObject *TCAX_PyBigPix;

/**
 * TCAX_PyPoints structure a tuple of tuples (points).
 * ((x1, y1, a1), (x2, y2, a2), ...)
 */
typedef PyObject *TCAX_PyPoints;


/* Inhibit C++ name-mangling for tcaxLib functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */
    
/**
 * Clip a number into 0 to 255 inclusive
 */
extern unsigned char CLIP_0_255(int a);

/**
 * pPoints->xBuf and pPoints->yBuf need to be allocated before being passed to the function, and it will be reallocated if more space is needed
 * pPoints->count and pPoints->capacity should also be specified before being passed to the function, and their value may be modified
 */
extern void tcaxlib_points_append(TCAX_pPoints pPoints, double x, double y, unsigned char a);

/**
 * Convert TCAX PIX to TCAX PY PIX.
 * @param pPix pointer to TCAX_Pix structure
 * @param delPix indicates whether to delete PIX before returning of the function
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_convert_pix(const TCAX_pPix pPix, int delPix);

/**
 * Convert TCAX PY PIX to TCAX PIX. 
 * Remark: in the following functions we will just use PIX, it may stand for TCAX PIX or TCAX PY PIX, in case that there is no ambiguity.
 *
 * @param pyPix the TCAX_PyPix structure
 * @param pPix pointer to TCAX_Pix structure that is going to hold the converted data
 * @return const TCAX_pPix
 */
extern const TCAX_pPix tcaxlib_convert_py_pix(const TCAX_PyPix pyPix, TCAX_pPix pPix);

/**
 * Get points from TCAX PY PIX.
 * @param self reserved
 * @param args (PIX)
 * @return TCAX_PyPoints
 */
extern TCAX_PyPoints tcaxlib_pix_points(PyObject *self, PyObject *args);

/**
 * Create a blank PIX.
 * @param self reserved
 * @param args (width, height, rgba)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_create_blank_pix(PyObject *self, PyObject *args);

/**
 * Resample TCAX PIX. 
 * Remark: the target width and height should be specified in the target TCAX_Pix structure before it is sent to the function. 
 * Other variables will be ignored. Note that the `buf' of the target TCAX_Pix structure should not be allocated before it is 
 * sent to the function, otherwise will cause memory leak.
 *
 * @param pPixSrc pointer to source TCAX_Pix structure
 * @param pPixDst pointer to target TCAX_Pix structure
 * @return TCAX_pPix
 */
extern const TCAX_pPix tcaxlib_resample_pix(const TCAX_pPix pPixSrc, TCAX_pPix pPixDst);

/**
 * supports fractional width and height
 */
extern const TCAX_pPix tcaxlib_resample_pix_ex(const TCAX_pPix pPixSrc, TCAX_pPix pPixDst, double width, double height);

/**
 * Resample TCAX PY PIX.
 * @param self reserved
 * @param args (PIX, width, height)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_resample_py_pix(PyObject *self, PyObject *args);

/**
 * supports fractional width and height
 */
extern TCAX_PyPix tcaxlib_resample_py_pix_ex(PyObject *self, PyObject *args);

/**
 * Applay a texture to PIX.
 * @param self reserved
 * @param args (PIX, Texture)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_apply_pix_texture(PyObject *self, PyObject *args);

/**
 * Applay a mask to PIX.
 * @param self reserved
 * @param args (PIX, Mask)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_apply_pix_mask(PyObject *self, PyObject *args);

/**
 * Replace the alpha channel of the source PIX with the alpha channel of the destination PIX. 
 * Remark: the transparent pixels of the source PIX will stay unchanged.
 *
 * @param self reserved
 * @param args (PIX, PIX_alpha)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_pix_replace_alpha(PyObject *self, PyObject *args);

/**
 * Enlarge a PIX. 
 * Remark: the PIX is put in the middle of the enlarged one, and the extra space is filled with 0. 
 * The target width and height should be specified in the target TCAX_Pix structure before it is sent to the function. 
 * And target width and height should be both larger than (or equal to) source ones. 
 * Other variables will be ignored. Note that the `buf' of the target TCAX_Pix structure should be empty before it is sent to the function, 
 * otherwise will cause memory leaking.
 *
 * @param pPixSrc pointer to source TCAX_Pix structure
 * @param pPixDst pointer to target TCAX_Pix structure
 * @return TCAX_pPix
 */
extern const TCAX_pPix tcaxlib_enlarge_pix(const TCAX_pPix pPixSrc, TCAX_pPix pPixDst);

/**
 * Enlarge a TCAX PY PIX.
 * @param self reserved
 * @param args (PIX, x, y)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_enlarge_py_pix(PyObject *self, PyObject *args);

/**
 * Crop a TCAX PY PIX.
 * @param self reserved
 * @param args (PIX, left, top, right, bottom)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_crop_py_pix(PyObject *self, PyObject *args);

/**
 * Strip blank border of a TCAX PY PIX.
 * @param self reserved
 * @param args (PIX)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_py_pix_strip_blank(PyObject *self, PyObject *args);

/**
 * Applay Gauss IIR blur to a PIX.
 * @param self reserved
 * @param args (PIX, radius)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_apply_gauss_blur(PyObject *self, PyObject *args);

/**
 * Combine two PIXs, i.e., overlay one PIX upon the other.
 * Remark: during overlaying, the float of the input PIXs' initX and initY will 
 * be discarded and the result PIX's initX and initY will be integers, 
 * so, use BilinearFilter to remove the float of the input PIXs if necessary.
 *
 * @param self reserved
 * @param args (back, overlay)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_combine_two_pixs(PyObject *self, PyObject *args);

/**
 * Change the color of a PIX by multiplying factors to each channel of RGBA.
 * @param self reserved
 * @param args (PIX, r_f, g_f, b_f, a_f)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_pix_color_multiply(PyObject *self, PyObject *args);

/**
 * Change the color of a PIX by shifting values of each channel of RGBA.
 * @param self reserved
 * @param args (PIX, r, g, b, a)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_pix_color_shift(PyObject *self, PyObject *args);

/**
 * Make the color in a PIX with specified rgb value transparent.
 * @param self reserved
 * @param args (PIX, rgb)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_pix_color_transparent(PyObject *self, PyObject *args);

/**
 * Change the value of colors including alpha channels in the PIX with flat rgba value.
 * @param self reserved
 * @param args (PIX, rgba)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_pix_color_flat_rgba(PyObject *self, PyObject *args);

/**
 * Change the value of colors in the PIX with flat rgb value.
 * @param self reserved
 * @param args (PIX, rgb)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_pix_color_flat_rgb(PyObject *self, PyObject *args);

/**
 * Change the value of alpha channels in the PIX with constant alpha value.
 * @param self reserved
 * @param args (PIX, alpha)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_pix_color_flat_alpha(PyObject *self, PyObject *args);

/**
 * Bilinear filter, mainly used in moving a PIX, so that it will look smoother. 
 * Remark: this function is for internal use, it is used to manipulate a TCAX PIX.
 *
 * @see tcaxlib_bilinear_filter()
 * @param pPixSrc pointer to the source TCAX_Pix structure
 * @param offset_x offset of the PIX in the horizontal direction
 * @param offset_y offset of the PIX in the vertical direction
 * @param pPixDst pointer to the target TCAX_Pix structure
 * @return const TCAX_pPix
 */
extern const TCAX_pPix tcaxlib_bilinear_filter_internal(const TCAX_pPix pPixSrc, double offset_x, double offset_y, TCAX_pPix pPixDst);

/**
 * Bilinear filter, mainly used in moving a PIX, so that it will look smoother.
 * @see tcaxlib_bilinear_filter_internal()
 * @param self reserved
 * @param args (PIX, offset_x, offset_y)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_bilinear_filter(PyObject *self, PyObject *args);

/**
 * Scale filter, mainly used in moving a PIX, so that it will look smoother. 
 * Remark: this function is for internal use, it is used to manipulate a TCAX PIX.
 * The difference between scale filter and bilinear filter is that, the scale filter is most fit for
 * complicated pictures such as text, rich colored pictures, while it is bad for simple solid color squares
 *
 * @see tcaxlib_scale_filter()
 * @param pPixSrc pointer to the source TCAX_Pix structure
 * @param offset_x offset of the PIX in the horizontal direction
 * @param offset_y offset of the PIX in the vertical direction
 * @param pPixDst pointer to the target TCAX_Pix structure
 * @return const TCAX_pPix
 */
extern const TCAX_pPix tcaxlib_scale_filter_internal(const TCAX_pPix pPixSrc, double offset_x, double offset_y, TCAX_pPix pPixDst);

/**
 * Scale filter, mainly used in moving a PIX, so that it will look smoother.
 * @see tcaxlib_scale_filter_internal()
 * @param self reserved
 * @param args (PIX, offset_x, offset_y)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_scale_filter(PyObject *self, PyObject *args);

/**
 * Create a TCAX PY BIG PIX structure.
 * @param self reserved
 * @param args ()
 * @return TCAX_PyBigPix
 */
extern TCAX_PyBigPix tcaxlib_init_big_pix(PyObject *self, PyObject *args);

/**
 * Add a PIX to BigPIX.
 * @param self reserved
 * @param args (BIG_PIX, PIX, offset_x, offset_y, layer)
 * @return TCAX_Py_Error_Code
 */
extern TCAX_Py_Error_Code tcaxlib_big_pix_add(PyObject *self, PyObject *args);

/**
 * Convert BigPIX to PIX.
 * @param self reserved
 * @param args (BIG_PIX)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix tcaxlib_convert_big_pix(PyObject *self, PyObject *args);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* TCAXLIB_PIX_H */

