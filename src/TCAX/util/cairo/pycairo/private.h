/* -*- mode: C; c-basic-offset: 2 -*-
 *
 * Copyright © 2003,2010 James Henstridge, Steven Chaplin
 *
 * This file is part of pycairo.
 *
 * Pycairo is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3 as published
 * by the Free Software Foundation.
 *
 * Pycairo is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with pycairo. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PYCAIRO_PRIVATE_H_
#define _PYCAIRO_PRIVATE_H_

#ifdef _PYCAIRO_H_
#  error "don't include pycairo.h and pycairo-private.h together"
#endif

#define _INSIDE_PYCAIRO_
#include <Python.h>

#include "py3cairo.h"


extern PyObject *CairoError;

extern PyTypeObject PycairoContext_Type;
PyObject *PycairoContext_FromContext (cairo_t *ctx, PyTypeObject *type,
				      PyObject *base);

extern PyTypeObject PycairoFontFace_Type;
extern PyTypeObject PycairoToyFontFace_Type;
PyObject *PycairoFontFace_FromFontFace (cairo_font_face_t *font_face);

extern PyTypeObject PycairoFontOptions_Type;
PyObject *PycairoFontOptions_FromFontOptions (
                                          cairo_font_options_t *font_options);

extern PyTypeObject PycairoMatrix_Type;
PyObject *PycairoMatrix_FromMatrix (const cairo_matrix_t *matrix);

extern PyTypeObject PycairoPath_Type;
PyObject *PycairoPath_FromPath (cairo_path_t *path);

extern PyTypeObject PycairoPathiter_Type;

extern PyTypeObject PycairoPattern_Type;
extern PyTypeObject PycairoSolidPattern_Type;
extern PyTypeObject PycairoSurfacePattern_Type;
extern PyTypeObject PycairoGradient_Type;
extern PyTypeObject PycairoLinearGradient_Type;
extern PyTypeObject PycairoRadialGradient_Type;
PyObject *PycairoPattern_FromPattern (cairo_pattern_t *pattern,
				      PyObject *base);

extern PyTypeObject PycairoScaledFont_Type;
PyObject *PycairoScaledFont_FromScaledFont (cairo_scaled_font_t *scaled_font);

extern PyTypeObject PycairoSurface_Type;
extern PyTypeObject PycairoImageSurface_Type;

#if CAIRO_HAS_PDF_SURFACE
extern PyTypeObject PycairoPDFSurface_Type;
#endif

#if CAIRO_HAS_PS_SURFACE
extern PyTypeObject PycairoPSSurface_Type;
#endif

#if CAIRO_HAS_SVG_SURFACE
extern PyTypeObject PycairoSVGSurface_Type;
#endif

#if CAIRO_HAS_WIN32_SURFACE
extern PyTypeObject PycairoWin32Surface_Type;
extern PyTypeObject PycairoWin32PrintingSurface_Type;
#endif

#if CAIRO_HAS_XCB_SURFACE
extern PyTypeObject PycairoXCBSurface_Type;
#ifdef HAVE_XPYB
#  include <xpyb.h>
extern xpyb_CAPI_t *xpyb_CAPI;
extern PyObject *xpybVISUALTYPE_type;
#endif
#endif

#if CAIRO_HAS_XLIB_SURFACE
extern PyTypeObject PycairoXlibSurface_Type;
#endif

PyObject *PycairoSurface_FromSurface (cairo_surface_t *surface,
                                      PyObject *base);

int Pycairo_Check_Status (cairo_status_t status);

/* error checking macros */
#define RETURN_NULL_IF_CAIRO_ERROR(status)    \
  do {					      \
    if (status != CAIRO_STATUS_SUCCESS) {     \
      Pycairo_Check_Status (status);	      \
      return NULL;			      \
    }					      \
  } while (0)

#define RETURN_NULL_IF_CAIRO_CONTEXT_ERROR(ctx)    \
  do {						   \
    cairo_status_t status = cairo_status (ctx);	   \
    if (status != CAIRO_STATUS_SUCCESS) {	   \
      Pycairo_Check_Status (status);		   \
      return NULL;				   \
    }						   \
  } while (0)

#define RETURN_NULL_IF_CAIRO_PATTERN_ERROR(pattern)             \
  do {								\
    cairo_status_t status = cairo_pattern_status (pattern);	\
    if (status != CAIRO_STATUS_SUCCESS) {			\
      Pycairo_Check_Status (status);				\
      return NULL;						\
    }								\
  } while (0)

#define RETURN_NULL_IF_CAIRO_SURFACE_ERROR(surface)	        \
  do {								\
    cairo_status_t status = cairo_surface_status (surface);	\
    if (status != CAIRO_STATUS_SUCCESS) {			\
      Pycairo_Check_Status (status);				\
      return NULL;						\
    }								\
  } while (0)

#define RETURN_NULL_IF_CAIRO_SCALED_FONT_ERROR(sc_font)             \
  do {								    \
    cairo_status_t status = cairo_scaled_font_status (sc_font);	    \
    if (status != CAIRO_STATUS_SUCCESS) {			    \
      Pycairo_Check_Status (status);				    \
      return NULL;						    \
    }								    \
  } while (0)

#define RETURN_NULL_IF_CAIRO_FONT_OPTIONS_ERROR(fo)	        \
  do {								\
    cairo_status_t status = cairo_font_options_status (fo);	\
    if (status != CAIRO_STATUS_SUCCESS) {			\
      Pycairo_Check_Status (status);				\
      return NULL;						\
    }								\
  } while (0)


#endif /* _PYCAIRO_PRIVATE_H_ */
