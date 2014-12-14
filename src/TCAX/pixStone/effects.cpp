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

#include "effects.h"
#include "../tcax/tcaxLib/pix.c"


/************************* Blur ******************************/
static void _pixstone_blurs(const TCAX_pPix pPixSrc, int method, int radius, TCAX_pPix pPixDst) {
    int offset = radius * 2;
    pPixDst->initX = pPixSrc->initX - offset;
    pPixDst->initY = pPixSrc->initY - offset;
    pPixDst->width = pPixSrc->width + offset * 2;
    pPixDst->height = pPixSrc->height + offset * 2;
    pPixDst->size = pPixDst->height * (pPixDst->width << 2);
    FCObjImage img(pPixDst->width, pPixDst->height, 32);  /* create a temp image object */
    unsigned char *pBits = img.GetMemStart();
    memset(pBits, 0, pPixDst->size * sizeof(unsigned char));
    int w, h, i1, i2;
    for (h = 0; h < pPixSrc->height; h++) {
        for (w = 0; w < pPixSrc->width; w++) {
            i1 = ((h + offset) * pPixDst->width + w + offset) << 2;
            i2 = (h * pPixSrc->width + w) << 2;
            pBits[i1 + 0] = pPixSrc->buf[i2 + 0];
            pBits[i1 + 1] = pPixSrc->buf[i2 + 1];
            pBits[i1 + 2] = pPixSrc->buf[i2 + 2];
            pBits[i1 + 3] = pPixSrc->buf[i2 + 3];
        }
    }
    FCSinglePixelProcessBase *pEffect = NULL;
    switch (method) {  /* choose blur type */
    case 1:
        pEffect = new FCPixelBlur_Box(radius, TRUE);
        break;
    case 2:
        pEffect = new FCPixelBlur_Zoom(radius);
        break;
    case 3:
        pEffect = new FCPixelBlur_Radial(radius);
        break;
    case 4:
        pEffect = new FCPixelBlur_Motion(radius, DIRECT_RIGHT);
        break;
    case 5:
        pEffect = new FCPixelBlur_Motion(radius, DIRECT_LEFT);
        break;
    default:
        pEffect = new FCPixelBlur_Gauss_IIR(radius, radius);
        break;
    }
    img.SinglePixelProcessProc(*pEffect);  /* process the blur */
    delete pEffect;
    pPixDst->buf = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    for (h = 0; h < pPixDst->height; h++) {
        for (w = 0; w < pPixDst->width; w++) {
            i1 = (h * pPixDst->width + w) << 2;
            pPixDst->buf[i1 + 0] = pBits[i1 + 0];
            pPixDst->buf[i1 + 1] = pBits[i1 + 1];
            pPixDst->buf[i1 + 2] = pBits[i1 + 2];
            pPixDst->buf[i1 + 3] = pBits[i1 + 3];
        }
    }
}

TCAX_PyPix pixstone_blurs(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3;
    TCAX_PyPix pyPix;
    TCAX_Pix pixSrc;
    TCAX_Pix pixDst;
    int method, radius;
    if (PyTuple_GET_SIZE(args) < 3) {
        PyErr_SetString(PyExc_RuntimeError, "pstBlur error, too less parameters - `(PIX, method, radius)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "pstBlur error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "pstBlur error, the 2nd param should be an integer - `method'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "pstBlur error, the 3rd param should be an integer - `radius'\n");
        return NULL;
    }
    pyPix = (TCAX_PyPix)pyArg1;
    method = (int)PyLong_AsLong(pyArg2);
    radius = (int)PyLong_AsLong(pyArg3);
    tcaxlib_convert_py_pix(pyPix, &pixSrc);
    _pixstone_blurs(&pixSrc, method, radius, &pixDst);
    free(pixSrc.buf);
    return tcaxlib_convert_pix(&pixDst, 1);
}

/************************* Gradient ******************************/
static void _pixstone_gradient(const TCAX_pPix pPixSrc, int type, int x1, int y1, int x2, int y2, unsigned long rgb1, unsigned long rgb2, int repeat, TCAX_pPix pPixDst) {
    pPixDst->initX = pPixSrc->initX;
    pPixDst->initY = pPixSrc->initY;
    pPixDst->width = pPixSrc->width;
    pPixDst->height = pPixSrc->height;
    pPixDst->size = pPixSrc->size;
    FCObjImage img(pPixDst->width, pPixDst->height, 32);
    unsigned char *pBits = img.GetMemStart();
    memset(pBits, 0, pPixDst->size * sizeof(unsigned char));
    int w, h, index;
    for (h = 0; h < pPixSrc->height; h++) {
        for (w = 0; w < pPixSrc->width; w++) {
            index = (h * pPixSrc->width + w) << 2;
            pBits[index + 0] = pPixSrc->buf[index + 0];
            pBits[index + 1] = pPixSrc->buf[index + 1];
            pBits[index + 2] = pPixSrc->buf[index + 2];
            pBits[index + 3] = pPixSrc->buf[index + 3];
        }
    }
    POINT ptStart = {x1, y1};
    POINT ptEnd = {x2, y2};
    RECT rc = {x1, y1, x2, y2};
    RGBQUAD crStart = PCL_RGBA((BYTE)(rgb1 >> 16), (BYTE)(rgb1 >> 8), (BYTE)rgb1);
    RGBQUAD crEnd = PCL_RGBA((BYTE)(rgb2 >> 16), (BYTE)(rgb2 >> 8), (BYTE)rgb2);
    FCSinglePixelProcessBase *pEffect = NULL;
    switch (type) {
    case 1:
        pEffect = new FCPixelGradientConicalASym(ptStart, ptEnd, crStart, crEnd, (REPEAT_MODE)repeat);
        break;
    case 2:
        pEffect = new FCPixelGradientConicalSym(ptStart, ptEnd, crStart, crEnd, (REPEAT_MODE)repeat);
        break;
    case 3:
        pEffect = new FCPixelGradientRect(rc, crStart, crEnd, (REPEAT_MODE)repeat);
        break;
    case 4:
        pEffect = new FCPixelGradientRadial(rc, crStart, crEnd, (REPEAT_MODE)repeat);
        break;
    default:
        pEffect = new FCPixelGradientBiLine(ptStart, ptEnd, crStart, crEnd, (REPEAT_MODE)repeat);
        break;
    }
    img.SinglePixelProcessProc(*pEffect);
    delete pEffect;
    pPixDst->buf = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    for (h = 0; h < pPixDst->height; h++) {
        for (w = 0; w < pPixDst->width; w++) {
            index = (h * pPixDst->width + w) << 2;
            pPixDst->buf[index + 0] = pBits[index + 0];
            pPixDst->buf[index + 1] = pBits[index + 1];
            pPixDst->buf[index + 2] = pBits[index + 2];
            pPixDst->buf[index + 3] = pBits[index + 3];
        }
    }
}

TCAX_PyPix pixstone_gradient(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5, *pyArg6, *pyArg7, *pyArg8, *pyArg9;
    TCAX_PyPix pyPix;
    TCAX_Pix pixSrc;
    TCAX_Pix pixDst;
    int type, repeat;
    int x1, y1, x2, y2;
    unsigned long rgb1, rgb2;
    if (PyTuple_GET_SIZE(args) < 9) {
        PyErr_SetString(PyExc_RuntimeError, "pstGradient error, too less parameters - `(PIX, type, x1, y1, x2, y2, rgb1, rgb2, repeat)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "pstGradient error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "pstGradient error, the 2nd param should be an integer - `type'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "pstGradient error, the 3rd param should be an integer - `x1'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "pstGradient error, the 4th param should be an integer - `y1'\n");
        return NULL;
    }
    pyArg5 = PyTuple_GET_ITEM(args, 4);
    if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
        PyErr_SetString(PyExc_RuntimeError, "pstGradient error, the 5th param should be an integer - `x2'\n");
        return NULL;
    }
    pyArg6 = PyTuple_GET_ITEM(args, 5);
    if (!PyLong_Check(pyArg6) && !PyFloat_Check(pyArg6)) {
        PyErr_SetString(PyExc_RuntimeError, "pstGradient error, the 6th param should be an integer - `y2'\n");
        return NULL;
    }
    pyArg7 = PyTuple_GET_ITEM(args, 6);
    if (!PyLong_Check(pyArg7)) {
        PyErr_SetString(PyExc_RuntimeError, "pstGradient error, the 7th param should be an integer - `rgb1'\n");
        return NULL;
    }
    pyArg8 = PyTuple_GET_ITEM(args, 7);
    if (!PyLong_Check(pyArg8)) {
        PyErr_SetString(PyExc_RuntimeError, "pstGradient error, the 8th param should be an integer - `rgb2'\n");
        return NULL;
    }
    pyArg9 = PyTuple_GET_ITEM(args, 8);
    if (!PyLong_Check(pyArg9)) {
        PyErr_SetString(PyExc_RuntimeError, "pstGradient error, the 9th param should be an integer - `repeat'\n");
        return NULL;
    }
    pyPix = (TCAX_PyPix)pyArg1;
    type = (int)PyLong_AsLong(pyArg2);
    x1 = (int)PyLong_AsLong(pyArg3);
    y1 = (int)PyLong_AsLong(pyArg4);
    x2 = (int)PyLong_AsLong(pyArg5);
    y2 = (int)PyLong_AsLong(pyArg6);
    rgb1 = PyLong_AsUnsignedLong(pyArg7);
    rgb2 = PyLong_AsUnsignedLong(pyArg8);
    repeat = (int)PyLong_AsLong(pyArg9);
    tcaxlib_convert_py_pix(pyPix, &pixSrc);
    _pixstone_gradient(&pixSrc, type, x1, y1, x2, y2, rgb1, rgb2, repeat, &pixDst);
    free(pixSrc.buf);
    return tcaxlib_convert_pix(&pixDst, 1);
}

/************************* FX ******************************/
static void _pixstone_fx(const TCAX_pPix pPixSrc, int effect, double param, TCAX_pPix pPixDst) {
    FCObjImage img(pPixSrc->width, pPixSrc->height, 32);
    unsigned char *pBits = img.GetMemStart();
    memset(pBits, 0, pPixSrc->size * sizeof(unsigned char));
    int w, h, index;
    for (h = 0; h < pPixSrc->height; h++) {
        for (w = 0; w < pPixSrc->width; w++) {
            index = (h * pPixSrc->width + w) << 2;
            pBits[index + 0] = pPixSrc->buf[index + 0];
            pBits[index + 1] = pPixSrc->buf[index + 1];
            pBits[index + 2] = pPixSrc->buf[index + 2];
            pBits[index + 3] = pPixSrc->buf[index + 3];
        }
    }
    FCSinglePixelProcessBase *pEffect = NULL;
    switch (effect) {
    case 1:
        pEffect = new FCPixelAutoColorEnhance;
        break;
    case 2:
        pEffect = new FCPixelAutoContrast;
        break;
    case 3:
        pEffect = new FCPixelBrightness((int)param);
        break;
    case 4:
        pEffect = new FCPixelContrast((int)param);
        break;
    case 5:
        if (param < 1)
            param = 1;
        pEffect = new FCPixelDetectEdges((int)param);
        break;
    case 6:
        pEffect = new FCPixelEmboss((int)param);
        break;
    case 7:
        pEffect = new FCPixelFlip;
        break;
    case 8:
        if (param < 0)
            param = 0;
        pEffect = new FCPixelGamma(param);
        break;
    case 9:
        pEffect = new FCPixelGrayscale;
        break;
    case 10:
        pEffect = new FCPixelHalftoneM3;
        break;
    case 11:
        pEffect = new FCPixelIllusion((int)param);
        break;
    case 12:
        pEffect = new FCPixelInvert;
        break;
    case 13:
        if (param < 1)
            param = 1;
        pEffect = new FCPixelLens(param, TRUE);
        break;
    case 14:
        if (param < 2)
            param = 2;
        pEffect = new FCPixelMedianFilter((int)param);
        break;
    case 15:
        pEffect = new FCPixelMirror;
        break;
    case 16:
        if (param < 1)
            param = 1;
        pEffect = new FCPixelMosaic((int)param);
        break;
    case 17:
        if (param < 0)
            param = 0;
        else if (param > 100)
            param = 100;
        pEffect = new FCPixelNoisify((int)param, FALSE);
        break;
    case 18:
        if (param < 1)
            param = 1;
        pEffect = new FCPixelOilPaint((int)param);
        break;
    case 19:
        if (param < 2)
            param = 2;
        pEffect = new FCPixelPosterize((int)param);
        break;
    case 20:
        if (param < 0)
            param = 0;
        else if (param > 360)
            param = 360;
        pEffect = new FCPixelRotate((int)param);
        break;
    case 21:
        if (param < 1)
            param = 1;
        pEffect = new FCPixelSharp((int)param);
        break;
    case 22:
        pEffect = new FCPixelShift((int)param);
        break;
    case 23:
        pEffect = new FCPixelSmoothEdge((int)param);
        break;
    case 24:
        if (param < 0)
            param = 0;
        else if (param > 255)
            param = 255;
        pEffect = new FCPixelSolarize((int)param);
        break;
    case 25:
        if (param < 3)
            param = 3;
        pEffect = new FCPixelSplash((int)param);
        break;
    case 26:
        if (param < 0)
            param = 0;
        else if (param > 255)
            param = 255;
        pEffect = new FCPixelThreshold((int)param);
        break;
    case 27:
        if (param < 0 || param > 3)
            param = 0;
        pEffect = new FCPixelVideo((FCPixelVideo::VIDEO_TYPE)(int)param);
        break;
    default:
        pPixDst->initX = pPixSrc->initX;
        pPixDst->initY = pPixSrc->initY;
        pPixDst->width = pPixSrc->width;
        pPixDst->height = pPixSrc->height;
        pPixDst->size = pPixSrc->size;
        pPixDst->buf = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
        memcpy(pPixDst->buf, pPixSrc->buf, pPixDst->size * sizeof(unsigned char));
        return;
    }
    img.SinglePixelProcessProc(*pEffect);
    delete pEffect;
    pPixDst->initX = pPixSrc->initX;
    pPixDst->initY = pPixSrc->initY;
    pPixDst->width = img.Width();
    pPixDst->height = img.Height();
    pPixDst->size = pPixDst->height * (pPixDst->width << 2);
    pPixDst->buf = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    pBits = img.GetMemStart();
    for (h = 0; h < pPixDst->height; h++) {
        for (w = 0; w < pPixDst->width; w++) {
            index = (h * pPixDst->width + w) << 2;
            pPixDst->buf[index + 0] = pBits[index + 0];
            pPixDst->buf[index + 1] = pBits[index + 1];
            pPixDst->buf[index + 2] = pBits[index + 2];
            pPixDst->buf[index + 3] = pBits[index + 3];
        }
    }
}

TCAX_PyPix pixstone_fx(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3;
    TCAX_PyPix pyPix;
    TCAX_Pix pixSrc;
    TCAX_Pix pixDst;
    int effect;
    double param;
    if (PyTuple_GET_SIZE(args) < 3) {
        PyErr_SetString(PyExc_RuntimeError, "pstEffect error, too less parameters - `(PIX, effect, param)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "pstEffect error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "pstEffect error, the 2nd param should be an integer - `method'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "pstEffect error, the 3rd param should be a float - `param'\n");
        return NULL;
    }
    pyPix = (TCAX_PyPix)pyArg1;
    effect = (int)PyLong_AsLong(pyArg2);
    param = PyFloat_AsDouble(pyArg3);
    tcaxlib_convert_py_pix(pyPix, &pixSrc);
    _pixstone_fx(&pixSrc, effect, param, &pixDst);
    free(pixSrc.buf);
    return tcaxlib_convert_pix(&pixDst, 1);
}

/************************* Distord ******************************/
static void _pixstone_distord(const TCAX_pPix pPixSrc, int filter, double param1, double param2, TCAX_pPix pPixDst) {
    pPixDst->initX = pPixSrc->initX;
    pPixDst->initY = pPixSrc->initY;
    pPixDst->width = pPixSrc->width;
    pPixDst->height = pPixSrc->height;
    pPixDst->size = pPixSrc->size;
    FCObjImage img(pPixDst->width, pPixDst->height, 32);
    unsigned char *pBits = img.GetMemStart();
    memset(pBits, 0, pPixDst->size * sizeof(unsigned char));
    int w, h, index;
    for (h = 0; h < pPixSrc->height; h++) {
        for (w = 0; w < pPixSrc->width; w++) {
            index = (h * pPixSrc->width + w) << 2;
            pBits[index + 0] = pPixSrc->buf[index + 0];
            pBits[index + 1] = pPixSrc->buf[index + 1];
            pBits[index + 2] = pPixSrc->buf[index + 2];
            pBits[index + 3] = pPixSrc->buf[index + 3];
        }
    }
    FCSinglePixelProcessBase *pEffect = NULL;
    switch (filter)
    {
    case 1:
        pEffect = new FCPixelCylinder;
        break;
    case 2:
        if (param1 < 1)
            param1 = 1;
        pEffect = new FCPixelFractalTrace((int)param1);
        break;
    case 3:
        pEffect = new FCPixelLens(param1, TRUE);
        break;
    case 4:
        pEffect = new FCPixelRibbon((int)param1, (int)param2);
        break;
    case 5:
        pEffect = new FCPixelRipple((int)param1, (int)param2);
        break;
    case 6:
        pEffect = new FCPixelWave((int)param1, (int)param2);
        break;
    case 7:
        if (param2 < -1)
            param2 = -1;
        else if (param2 > 1)
            param2 = 1;
        pEffect = new FCPixelWhirlPinch(param1, param2);
        break;
    default:
        pEffect = new FCPixelWave((int)param1, (int)param2);
        break;
    }
    img.SinglePixelProcessProc(*pEffect);
    delete pEffect;
    pPixDst->buf = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    for (h = 0; h < pPixDst->height; h++) {
        for (w = 0; w < pPixDst->width; w++) {
            index = (h * pPixDst->width + w) << 2;
            pPixDst->buf[index + 0] = pBits[index + 0];
            pPixDst->buf[index + 1] = pBits[index + 1];
            pPixDst->buf[index + 2] = pBits[index + 2];
            pPixDst->buf[index + 3] = pBits[index + 3];
        }
    }
}

TCAX_PyPix pixstone_distord(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4;
    TCAX_PyPix pyPix;
    TCAX_Pix pixSrc;
    TCAX_Pix pixDst;
    int filter;
    double param1, param2;
    if (PyTuple_GET_SIZE(args) < 4) {
        PyErr_SetString(PyExc_RuntimeError, "pstDistord error, too less parameters - `(PIX, filter, param1, param2)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "pstDistord error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "pstDistord error, the 2nd param should be an integer - `filter'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "pstDistord error, the 3rd param should be an integer - `param1'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "pstDistord error, the 4th param should be an integer - `param2'\n");
        return NULL;
    }
    pyPix = (TCAX_PyPix)pyArg1;
    filter = (int)PyLong_AsLong(pyArg2);
    param1 = PyFloat_AsDouble(pyArg3);
    param2 = PyFloat_AsDouble(pyArg4);
    tcaxlib_convert_py_pix(pyPix, &pixSrc);
    _pixstone_distord(&pixSrc, filter, param1, param2, &pixDst);
    free(pixSrc.buf);
    return tcaxlib_convert_pix(&pixDst, 1);
}

/************************* LensFlare ******************************/
static void _pixstone_lens_flare(const TCAX_pPix pPixSrc, int x, int y, TCAX_pPix pPixDst) {
    pPixDst->initX = pPixSrc->initX;
    pPixDst->initY = pPixSrc->initY;
    pPixDst->width = pPixSrc->width;
    pPixDst->height = pPixSrc->height;
    pPixDst->size = pPixSrc->size;
    FCObjImage img(pPixDst->width, pPixDst->height, 32);
    unsigned char *pBits = img.GetMemStart();
    memset(pBits, 0, pPixDst->size * sizeof(unsigned char));
    int w, h, index;
    for (h = 0; h < pPixSrc->height; h++) {
        for (w = 0; w < pPixSrc->width; w++) {
            index = (h * pPixSrc->width + w) << 2;
            pBits[index + 0] = pPixSrc->buf[index + 0];
            pBits[index + 1] = pPixSrc->buf[index + 1];
            pBits[index + 2] = pPixSrc->buf[index + 2];
            pBits[index + 3] = pPixSrc->buf[index + 3];
        }
    }
    POINT pt = {x, y};
    FCPixelLensFlare aCmd(pt);
    img.SinglePixelProcessProc(aCmd);
    pPixDst->buf = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    for (h = 0; h < pPixDst->height; h++) {
        for (w = 0; w < pPixDst->width; w++) {
            index = (h * pPixDst->width + w) << 2;
            pPixDst->buf[index + 0] = pBits[index + 0];
            pPixDst->buf[index + 1] = pBits[index + 1];
            pPixDst->buf[index + 2] = pBits[index + 2];
            pPixDst->buf[index + 3] = pBits[index + 3];
        }
    }
}

TCAX_PyPix pixstone_lens_flare(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3;
    TCAX_PyPix pyPix;
    TCAX_Pix pixSrc;
    TCAX_Pix pixDst;
    int x, y;
    if (PyTuple_GET_SIZE(args) < 3) {
        PyErr_SetString(PyExc_RuntimeError, "pstLensFlare error, too less parameters - `(PIX, x, y)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "pstLensFlare error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "pstLensFlare error, the 2nd param should be an integer - `x'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "pstLensFlare error, the 3rd param should be an integer - `y'\n");
        return NULL;
    }
    pyPix = (TCAX_PyPix)pyArg1;
    x = (int)PyLong_AsLong(pyArg2);
    y = (int)PyLong_AsLong(pyArg3);
    tcaxlib_convert_py_pix(pyPix, &pixSrc);
    _pixstone_lens_flare(&pixSrc, x, y, &pixDst);
    free(pixSrc.buf);
    return tcaxlib_convert_pix(&pixDst, 1);
}

/************************* Soft glow ******************************/
static void _pixstone_soft_glow(const TCAX_pPix pPixSrc, int radius, int brightness, int contrast, TCAX_pPix pPixDst) {
    pPixDst->initX = pPixSrc->initX;
    pPixDst->initY = pPixSrc->initY;
    pPixDst->width = pPixSrc->width;
    pPixDst->height = pPixSrc->height;
    pPixDst->size = pPixSrc->size;
    FCObjImage img(pPixDst->width, pPixDst->height, 32);
    unsigned char *pBits = img.GetMemStart();
    memset(pBits, 0, pPixDst->size * sizeof(unsigned char));
    int w, h, index;
    for (h = 0; h < pPixSrc->height; h++) {
        for (w = 0; w < pPixSrc->width; w++) {
            index = (h * pPixSrc->width + w) << 2;
            pBits[index + 0] = pPixSrc->buf[index + 0];
            pBits[index + 1] = pPixSrc->buf[index + 1];
            pBits[index + 2] = pPixSrc->buf[index + 2];
            pBits[index + 3] = pPixSrc->buf[index + 3];
        }
    }
    FCPixelSoftGlow aCmd(radius, brightness, contrast);
    img.SinglePixelProcessProc(aCmd);
    pPixDst->buf = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    for (h = 0; h < pPixDst->height; h++) {
        for (w = 0; w < pPixDst->width; w++) {
            index = (h * pPixDst->width + w) << 2;
            pPixDst->buf[index + 0] = pBits[index + 0];
            pPixDst->buf[index + 1] = pBits[index + 1];
            pPixDst->buf[index + 2] = pBits[index + 2];
            pPixDst->buf[index + 3] = pBits[index + 3];
        }
    }
}

TCAX_PyPix pixstone_soft_glow(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4;
    TCAX_PyPix pyPix;
    TCAX_Pix pixSrc;
    TCAX_Pix pixDst;
    int radius, brightness, contrast;
    if (PyTuple_GET_SIZE(args) < 4) {
        PyErr_SetString(PyExc_RuntimeError, "pstSoftGlow error, too less parameters - `(PIX, radius, brightness, contrast)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "pstLensFlare error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "pstLensFlare error, the 2nd param should be an integer - `radius'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "pstLensFlare error, the 3rd param should be an integer - `brightness'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "pstLensFlare error, the 4th param should be an integer - `contrast'\n");
        return NULL;
    }
    pyPix = (TCAX_PyPix)pyArg1;
    radius = (int)PyLong_AsLong(pyArg2);
    brightness = (int)PyLong_AsLong(pyArg3);
    contrast = (int)PyLong_AsLong(pyArg4);
    tcaxlib_convert_py_pix(pyPix, &pixSrc);
    _pixstone_soft_glow(&pixSrc, radius, brightness, contrast, &pixDst);
    free(pixSrc.buf);
    return tcaxlib_convert_pix(&pixDst, 1);
}

/************************* Blind ******************************/
static void _pixstone_blind(const TCAX_pPix pPixSrc, int direct, int width, int opacity, unsigned long rgb, TCAX_pPix pPixDst) {
    pPixDst->initX = pPixSrc->initX;
    pPixDst->initY = pPixSrc->initY;
    pPixDst->width = pPixSrc->width;
    pPixDst->height = pPixSrc->height;
    pPixDst->size = pPixSrc->size;
    FCObjImage img(pPixDst->width, pPixDst->height, 32);
    unsigned char *pBits = img.GetMemStart();
    memset(pBits, 0, pPixDst->size * sizeof(unsigned char));
    int w, h, index;
    for (h = 0; h < pPixSrc->height; h++) {
        for (w = 0; w < pPixSrc->width; w++) {
            index = (h * pPixSrc->width + w) << 2;
            pBits[index + 0] = pPixSrc->buf[index + 0];
            pBits[index + 1] = pPixSrc->buf[index + 1];
            pBits[index + 2] = pPixSrc->buf[index + 2];
            pBits[index + 3] = pPixSrc->buf[index + 3];
        }
    }
    RGBQUAD color = PCL_RGBA((BYTE)(rgb >> 16), (BYTE)(rgb >> 8), (BYTE)rgb);
    FCPixelBlinds aCmd((FCPixelBlinds::BLIND_TYPE)direct, width, opacity, color);
    img.SinglePixelProcessProc(aCmd);
    pPixDst->buf = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    for (h = 0; h < pPixDst->height; h++) {
        for (w = 0; w < pPixDst->width; w++) {
            index = (h * pPixDst->width + w) << 2;
            pPixDst->buf[index + 0] = pBits[index + 0];
            pPixDst->buf[index + 1] = pBits[index + 1];
            pPixDst->buf[index + 2] = pBits[index + 2];
            pPixDst->buf[index + 3] = pBits[index + 3];
        }
    }
}

TCAX_PyPix pixstone_blind(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5;
    TCAX_PyPix pyPix;
    TCAX_Pix pixSrc;
    TCAX_Pix pixDst;
    int direct, width, opacity;
    unsigned long rgb;
    if (PyTuple_GET_SIZE(args) < 5) {
        PyErr_SetString(PyExc_RuntimeError, "pstBlind error, too less parameters - `(PIX, direct, width, opacity, rgb)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "pstBlind error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "pstBlind error, the 2nd param should be an integer - `direct'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "pstBlind error, the 3rd param should be an integer - `width'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "pstBlind error, the 4th param should be an integer - `opacity'\n");
        return NULL;
    }
    pyArg5 = PyTuple_GET_ITEM(args, 4);
    if (!PyLong_Check(pyArg5)) {
        PyErr_SetString(PyExc_RuntimeError, "pstBlind error, the 5th param should be an integer - `rgb'\n");
        return NULL;
    }
    pyPix = (TCAX_PyPix)pyArg1;
    direct = (int)PyLong_AsLong(pyArg2);
    width = (int)PyLong_AsLong(pyArg3);
    opacity = (int)PyLong_AsLong(pyArg4);
    rgb = PyLong_AsUnsignedLong(pyArg5);
    tcaxlib_convert_py_pix(pyPix, &pixSrc);
    _pixstone_blind(&pixSrc, direct, width, opacity, rgb, &pixDst);
    free(pixSrc.buf);
    return tcaxlib_convert_pix(&pixDst, 1);
}

/************************* Tile Reflection ******************************/
static void _pixstone_tile_reflection(const TCAX_pPix pPixSrc, int angle, int squareSize, int curvature, TCAX_pPix pPixDst) {
    pPixDst->initX = pPixSrc->initX;
    pPixDst->initY = pPixSrc->initY;
    pPixDst->width = pPixSrc->width;
    pPixDst->height = pPixSrc->height;
    pPixDst->size = pPixSrc->size;
    FCObjImage img(pPixDst->width, pPixDst->height, 32);
    unsigned char *pBits = img.GetMemStart();
    memset(pBits, 0, pPixDst->size * sizeof(unsigned char));
    int w, h, index;
    for (h = 0; h < pPixSrc->height; h++) {
        for (w = 0; w < pPixSrc->width; w++) {
            index = (h * pPixSrc->width + w) << 2;
            pBits[index + 0] = pPixSrc->buf[index + 0];
            pBits[index + 1] = pPixSrc->buf[index + 1];
            pBits[index + 2] = pPixSrc->buf[index + 2];
            pBits[index + 3] = pPixSrc->buf[index + 3];
        }
    }
    FCPixelTileReflection aCmd(angle, squareSize, curvature);
    img.SinglePixelProcessProc(aCmd);
    pPixDst->buf = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    for (h = 0; h < pPixDst->height; h++) {
        for (w = 0; w < pPixDst->width; w++) {
            index = (h * pPixDst->width + w) << 2;
            pPixDst->buf[index + 0] = pBits[index + 0];
            pPixDst->buf[index + 1] = pBits[index + 1];
            pPixDst->buf[index + 2] = pBits[index + 2];
            pPixDst->buf[index + 3] = pBits[index + 3];
        }
    }
}

TCAX_PyPix pixstone_tile_reflection(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4;
    TCAX_PyPix pyPix;
    TCAX_Pix pixSrc;
    TCAX_Pix pixDst;
    int angle, squareSize, curvature;
    if (PyTuple_GET_SIZE(args) < 4) {
        PyErr_SetString(PyExc_RuntimeError, "pstTileReflection error, too less parameters - `(PIX, angle, squareSize, curvature)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "pstTileReflection error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "pstTileReflection error, the 2nd param should be an integer - `angle'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "pstTileReflection error, the 3rd param should be an integer - `squareSize'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "pstTileReflection error, the 4th param should be an integer - `curvature'\n");
        return NULL;
    }
    pyPix = (TCAX_PyPix)pyArg1;
    angle = (int)PyLong_AsLong(pyArg2);
    squareSize = (int)PyLong_AsLong(pyArg3);
    curvature = (int)PyLong_AsLong(pyArg4);
    tcaxlib_convert_py_pix(pyPix, &pixSrc);
    _pixstone_tile_reflection(&pixSrc, angle, squareSize, curvature, &pixDst);
    free(pixSrc.buf);
    return tcaxlib_convert_pix(&pixDst, 1);
}

