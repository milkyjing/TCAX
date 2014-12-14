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

#ifndef PIXSTONE_EFFECTS_H
#define PIXSTONE_EFFECTS_H
#pragma once

#include "../tcax/tcaxLib/pix.h"
#include "./ImageStone/ImageStone.h"

#define PIXSTONE_VERSION 0x00005000    /**< High word of PIXSTONE_VERSION indicates major pixStone version, and low word indicates minor version */
#define PIXSTONE_VER_STR L"0.5"


/**
 * Apply blur effect to TCAX PY PIX with several kinds of blurs. 
 * Remark: methods (0 - 5), Gauss, Box, Zoom, Radial, Motion_Right, Motion_Left
 *
 * @param self reserved
 * @param args (PIX, method, radius)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix pixstone_blurs(PyObject *self, PyObject *args);

/**
 * Apply gradient effect to TCAX PY PIX with several kinds of gradient types. 
 * Remark: types (0 - 4), BiLine ConicalASym ConicalSym Rect Radial, 
 * repeat, 0 - REPEAT_NONE, 1 - REPEAT_SAWTOOTH, 2 - REPEAT_TRIANGULAR
 *
 * @param self reserved
 * @param args (PIX, type, x1, y1, x2, y2, rgb1, rgb2, repeat)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix pixstone_gradient(PyObject *self, PyObject *args);

/**
 * Apply simple effect to TCAX PY PIX with several kinds of effects. 
 * Remark: effects available, 
 * 1.FCPixelAutoColorEnhance 2.FCPixelAutoContrast 3.FCPixelBrightness 
 * 4.FCPixelContrast 5.FCPixelDetectEdges 6.FCPixelEmboss 
 * 7.FCPixelFlip 8.FCPixelGamma 9.FCPixelGrayscale 
 * 10.FCPixelHalftoneM3 11.FCPixelIllusion 12.FCPixelInvert 
 * 13.FCPixelLens 14.FCPixelMedianFilter 15.FCPixelMirror 
 * 16.FCPixelMosaic 17.FCPixelNoisify 18.FCPixelOilPaint 
 * 19.FCPixelPosterize 20.FCPixelRotate 21.FCPixelSharp 
 * 22.FCPixelShift 23.FCPixelSmoothEdge 24.FCPixelSolarize 
 * 25.FCPixelSplash 26.FCPixelThreshold 27.FCPixelVideo
 *
 * @param self reserved
 * @param args (PIX, effect, param)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix pixstone_fx(PyObject *self, PyObject *args);

/**
 * Apply distortion effect to TCAX PY PIX with several kinds of filters. 
 * Remark: filters available, 
 * 1.FCPixelCylinder 2.FCPixelFractalTrace 3.FCPixelLens 
 * 4.FCPixelRibbon 5.FCPixelRipple 6.FCPixelWave 7.FCPixelWhirlPinch
 *
 * @param self reserved
 * @param args (PIX, filter, param1, param2)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix pixstone_distord(PyObject *self, PyObject *args);

/**
 * Apply LensFlare effect to TCAX PY PIX.
 * @param self reserved
 * @param args (PIX, x, y)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix pixstone_lens_flare(PyObject *self, PyObject *args);

/**
 * Apply soft glow effect to TCAX PY PIX.
 * @param self reserved
 * @param args (PIX, radius, brightness, contrast)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix pixstone_soft_glow(PyObject *self, PyObject *args);

/**
 * Apply blind effect to TCAX PY PIX.
 * @param self reserved
 * @param args (PIX, direct, width, opacity, rgb)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix pixstone_blind(PyObject *self, PyObject *args);

/**
 * Apply tile reflection effect to TCAX PY PIX.
 * @param self reserved
 * @param args (PIX, angle, squareSize, curvature)
 * @return TCAX_PyPix
 */
extern TCAX_PyPix pixstone_tile_reflection(PyObject *self, PyObject *args);

#endif  /* PIXSTONE_EFFECTS_H */

