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

#include "pix.h"


unsigned char CLIP_0_255(int a) {
    if (a < 0) return (unsigned char)0;
    else if (a > 255) return (unsigned char)255;
    else return (unsigned char)a;
}

void tcaxlib_points_append(TCAX_pPoints pPoints, double x, double y, unsigned char a) {
    double *xBuf, *yBuf;
    unsigned char *aBuf;
    int count, capacity;
    xBuf = pPoints->xBuf;
    yBuf = pPoints->yBuf;
    aBuf = pPoints->aBuf;
    count = pPoints->count;
    capacity = pPoints->capacity;
    if (count + 1 > capacity) {  /* we need more space */
        capacity += capacity / 2 + 1;
        xBuf = (double *)realloc(xBuf, capacity * sizeof(double));
        yBuf = (double *)realloc(yBuf, capacity * sizeof(double));
        aBuf = (unsigned char *)realloc(aBuf, capacity * sizeof(unsigned char));
        pPoints->xBuf = xBuf;
        pPoints->yBuf = yBuf;
        pPoints->aBuf = aBuf;
        pPoints->capacity = capacity;
    }
    xBuf[count] = x;
    yBuf[count] = y;
    aBuf[count] = a;
    count++;
    pPoints->count = count;
}

TCAX_PyPix tcaxlib_convert_pix(const TCAX_pPix pPix, int delPix) {
    int i;
    TCAX_PyPix pyPix;
    PyObject *pyTemp;
    pyPix = PyTuple_New(3);
    pyTemp = PyTuple_New(2);
    PyTuple_SET_ITEM(pyTemp, 0, PyFloat_FromDouble(pPix->initX));
    PyTuple_SET_ITEM(pyTemp, 1, PyFloat_FromDouble(pPix->initY));
    PyTuple_SET_ITEM(pyPix, 0, pyTemp);
    pyTemp = PyTuple_New(2);
    PyTuple_SET_ITEM(pyTemp, 0, PyLong_FromLong(pPix->width));
    PyTuple_SET_ITEM(pyTemp, 1, PyLong_FromLong(pPix->height));
    PyTuple_SET_ITEM(pyPix, 1, pyTemp);
    pPix->size = pPix->height * (pPix->width << 2);
    pyTemp = PyTuple_New(pPix->size);
    for (i = 0; i < pPix->size; i++)
        PyTuple_SET_ITEM(pyTemp, i, PyLong_FromLong(pPix->buf[i]));
    PyTuple_SET_ITEM(pyPix, 2, pyTemp);
    if (delPix) free(pPix->buf);
    return pyPix;
}

const TCAX_pPix tcaxlib_convert_py_pix(const TCAX_PyPix pyPix, TCAX_pPix pPix) {
    int i;
    PyObject *pos;
    PyObject *size;
    PyObject *buf;
    pos = PyTuple_GET_ITEM(pyPix, 0);
    size = PyTuple_GET_ITEM(pyPix, 1);
    buf = PyTuple_GET_ITEM(pyPix, 2);
    pPix->initX = PyFloat_AsDouble(PyTuple_GET_ITEM(pos, 0));
    pPix->initY = PyFloat_AsDouble(PyTuple_GET_ITEM(pos, 1));
    pPix->width = (int)PyLong_AsLong(PyTuple_GET_ITEM(size, 0));
    pPix->height = (int)PyLong_AsLong(PyTuple_GET_ITEM(size, 1));
    pPix->size = pPix->height * (pPix->width << 2);
    pPix->buf = (unsigned char *)malloc(pPix->size * sizeof(unsigned char));
    for (i = 0; i < pPix->size; i++)
        pPix->buf[i] = (unsigned char)PyLong_AsLong(PyTuple_GET_ITEM(buf, i));
    return (const TCAX_pPix)pPix;
}

TCAX_PyPoints tcaxlib_pix_points(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    TCAX_PyPix pyPix;
    TCAX_Pix pix;
    int i, w, h;
    TCAX_Points points;
    TCAX_PyPoints pyPoints;
    PyObject *pyPointTemp;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "PixPoints error, too less parameters - `(PIX)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "PixPoints error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyPix = (TCAX_PyPix)pyArg1;
    tcaxlib_convert_py_pix(pyPix, &pix);
    points.count = 0;
    points.capacity = 100;
    points.xBuf = (double *)malloc(points.capacity * sizeof(double));
    points.yBuf = (double *)malloc(points.capacity * sizeof(double));
    points.aBuf = (unsigned char *)malloc(points.capacity * sizeof(unsigned char));
    for (h = 0; h < pix.height; h++) {
        for (w = 0; w < pix.width; w++) {
            unsigned char a = pix.buf[((h * pix.width + w) << 2) + 3];
            if (0 != a)
                tcaxlib_points_append(&points, w, h, a);
        }
    }
    free(pix.buf);
    pyPoints = PyTuple_New(points.count);
    for (i = 0; i < points.count; i++) {
        pyPointTemp = PyTuple_New(3);
        PyTuple_SET_ITEM(pyPointTemp, 0, PyLong_FromLong((long)points.xBuf[i]));
        PyTuple_SET_ITEM(pyPointTemp, 1, PyLong_FromLong((long)points.yBuf[i]));
        PyTuple_SET_ITEM(pyPointTemp, 2, PyLong_FromLong((long)points.aBuf[i]));
        PyTuple_SET_ITEM(pyPoints, i, pyPointTemp);
    }
    free(points.xBuf);
    free(points.yBuf);
    free(points.aBuf);
    return pyPoints;
}

TCAX_PyPix tcaxlib_create_blank_pix(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3;
    unsigned long rgba;
    TCAX_Pix pix;
    int i;
    if (PyTuple_GET_SIZE(args) < 3) {
        PyErr_SetString(PyExc_RuntimeError, "BlankPix error, too less parameters - `(width, height, rgba)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1) && !PyFloat_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "BlankPix error, the 1st param should be an integer - `width'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "BlankPix error, the 2nd param should be an integer - `height'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "BlankPix error, the 3rd param should be an integer - `rgba'\n");
        return NULL;
    }
    pix.initX = 0;
    pix.initY = 0;
    pix.width = (int)PyLong_AsLong(pyArg1);
    pix.height = (int)PyLong_AsLong(pyArg2);
    rgba = (unsigned long)PyLong_AsUnsignedLong(pyArg3);
    pix.size = pix.height * (pix.width << 2);
    pix.buf = (unsigned char *)malloc(pix.size * sizeof(unsigned char));
    for (i = 0; i < pix.size; i += 4)
        *((unsigned long *)&pix.buf[i]) = rgba;
    return tcaxlib_convert_pix(&pix, 1);
}

/* resample image */
static double _tcaxlib_filter_MitchellNetravali(double x, double b, double c) {
    double p0, p2, p3, q0, q1, q2, q3;
    p0 = (   6 -  2 * b          ) / 6.0;
    p2 = ( -18 + 12 * b +  6 * c ) / 6.0;
    p3 = (  12 -  9 * b -  6 * c ) / 6.0;
    q0 = (        8 * b + 24 * c ) / 6.0;
    q1 = (     - 12 * b - 48 * c ) / 6.0;
    q2 = (        6 * b + 30 * c ) / 6.0;
    q3 = (     -      b -  6 * c ) / 6.0;
    if (x < 0) x = -x;    /* x = fabs(x) */
    if (x < 1)
        return p0 + (p2 + p3 * x) * x * x;
    else if (x < 2)
        return q0 + (q1 + (q2 + q3 * x) * x) * x;
    else return 0;
}

static double _tcaxlib_filter_cubic(double x, double a) {
    if (x < 0) x = -x;    /* x = fabs(x) */
    if (x < 1) return (a + 2) * x * x * x - (a + 3) * x * x + 1;
    else if (x < 2) return a * x * x * x - 5 * a * x * x + 8 * a * x - 4 * a;
    else return 0;
}

static double _tcaxlib_filter_BSpline(double x) {
    if (x < -2) return 0;
    else if (x < -1) return (2 + x) * (2 + x) * (2 + x) / 6.0;
    else if (x < 0) return (4 + (-6 - 3 * x) * x * x) / 6.0;
    else if (x < 1) return (4 + (-6 + 3 * x) * x * x) / 6.0;
    else if (x < 2) return (2 - x) * (2 - x) * (2 - x) / 6.0;
    else return 0;
}

static void _tcaxlib_resample_rgba(const unsigned char *src, int width, int height, unsigned char *dst, int targetWidth, int targetHeight) {
    int h, w, m, n, index;
    double fx, fy;
    int ix, iy, xx, yy;
    double xScale, yScale, r1, r2;
    double rr, gg, bb, aa;
    if (targetWidth == width && targetHeight == height)
        memcpy(dst, src, height * (width << 2) * sizeof(unsigned char));
    else {
        xScale = targetWidth / (double)width;
        yScale = targetHeight / (double)height;
        for (h = 0; h < targetHeight; h++) {
            fy = h / yScale;
            iy = (int)fy;
            for (w = 0; w < targetWidth; w++) {
                fx = w / xScale;
                ix = (int)fx;
                rr = 0;
                gg = 0;
                bb = 0;
                aa = 0;
                for (m = 0; m < 4; m++) {
                    yy = iy + m - 1;
                    r1 = _tcaxlib_filter_cubic(yy - fy, -0.5);    /* can use _tcaxlib_filter_MitchellNetravali (best quality but slowest) or _tcaxlib_filter_BSpline (smoothest) */
                    if (yy < 0) yy = 0;
                    if (yy >= height) yy = height - 1;
                    for (n = 0; n < 4; n++) {
                        xx = ix + n - 1;
                        r2 = r1 * _tcaxlib_filter_cubic(xx - fx, -0.5);    /* can use _tcaxlib_filter_MitchellNetravali (best quality but slowest) or _tcaxlib_filter_BSpline (smoothest) */
                        if (xx < 0) xx = 0;
                        if (xx >= width) xx = width - 1;
                        index = (yy * width + xx) << 2;
                        rr += src[index] * r2;
                        gg += src[index + 1] * r2;
                        bb += src[index + 2] * r2;
                        aa += src[index + 3] * r2;
                    }
                }
                index = (h * targetWidth + w) << 2;
                dst[index] = CLIP_0_255((int)rr);
                dst[index + 1] = CLIP_0_255((int)gg);
                dst[index + 2] = CLIP_0_255((int)bb);
                dst[index + 3] = CLIP_0_255((int)aa);
            }
        }
    }
}

const TCAX_pPix tcaxlib_resample_pix(const TCAX_pPix pPixSrc, TCAX_pPix pPixDst) {
    if (0 == pPixDst->width) pPixDst->width = pPixDst->height * pPixSrc->width / pPixSrc->height;    /* keep aspect ratio */
    else if (0 == pPixDst->height) pPixDst->height = pPixDst->width * pPixSrc->height / pPixSrc->width;    /* keep aspect ratio */
    pPixDst->initX = pPixSrc->initX - (pPixDst->width - pPixSrc->width) / 2;
    pPixDst->initY = pPixSrc->initY - (pPixDst->height - pPixSrc->height) / 2;
    pPixDst->size = pPixDst->height * (pPixDst->width << 2);
    pPixDst->buf = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    _tcaxlib_resample_rgba(pPixSrc->buf, pPixSrc->width, pPixSrc->height, pPixDst->buf, pPixDst->width, pPixDst->height);
    return (const TCAX_pPix)pPixDst;
}

const TCAX_pPix tcaxlib_resample_pix_ex(const TCAX_pPix pPixSrc, TCAX_pPix pPixDst, double width, double height) {
    const int m = 4;
    double f_w, f_h, f_mw, f_mh;
    int i_mw, i_mh, pad_w, pad_h;
    int sf_mw, sf_mh, sf_msize;
    unsigned char *sBuf, *bufm;
    int i, index1, index2, size;
    f_w = width;
    f_h = height;
    if (0 == f_w) f_w = f_h * pPixSrc->width / (double)pPixSrc->height;    /* keep aspect ratio */
    else if (0 == f_h) f_h = f_w * pPixSrc->height / (double)pPixSrc->width;    /* keep aspect ratio */
    f_mw = m * f_w;
    f_mh = m * f_h;
    i_mw = (int)f_mw;
    i_mh = (int)f_mh;
    pad_w = m - i_mw % m;
    if (m == pad_w)
        pad_w = 0;
    pad_h = m - i_mh % m;
    if (m == pad_h)
        pad_h = 0;
    sf_mw = i_mw + pad_w;
    sf_mh = i_mh + pad_h;
    sf_msize = sf_mh * (sf_mw << 2);
    sBuf = (unsigned char *)malloc(sf_msize * sizeof(unsigned char));
    memset(sBuf, 0, sf_msize * sizeof(unsigned char));      /* now we've got the surface to draw */
    bufm = (unsigned char *)malloc(i_mh * (i_mw << 2) * sizeof(unsigned char));   /* magnitude the source to m * m */
    _tcaxlib_resample_rgba(pPixSrc->buf, pPixSrc->width, pPixSrc->height, bufm, i_mw, i_mh);
    size = (i_mw << 2) * sizeof(unsigned char);
    for (i = 0; i < i_mh; i++) {
        index1 = i * (sf_mw << 2) * sizeof(unsigned char);
        index2 = i * (i_mw << 2) * sizeof(unsigned char);
        memcpy(sBuf + index1, bufm + index2, size);
    }
    free(bufm);
    pPixDst->initX = pPixSrc->initX - (f_w - pPixSrc->width) / 2;
    pPixDst->initY = pPixSrc->initY - (f_h - pPixSrc->height) / 2;
    pPixDst->width = sf_mw / m;
    pPixDst->height = sf_mh / m;
    pPixDst->size = pPixDst->height * (pPixDst->width << 2);
    pPixDst->buf = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    _tcaxlib_resample_rgba(sBuf, sf_mw, sf_mh, pPixDst->buf, pPixDst->width, pPixDst->height);
    free(sBuf);
    return (const TCAX_pPix)pPixDst;
}

TCAX_PyPix tcaxlib_resample_py_pix(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3;
    TCAX_PyPix pyPix;
    TCAX_Pix pixSrc;
    TCAX_Pix pixDst;
    int width, height;
    if (PyTuple_GET_SIZE(args) < 3) {
        PyErr_SetString(PyExc_RuntimeError, "PixResize error, too less parameters - `(PIX, width, height)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "PixResize error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "PixResize error, the 2nd param should be an integer - `width'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "PixResize error, the 3rd param should be an integer - `height'\n");
        return NULL;
    }
    pyPix = (TCAX_PyPix)pyArg1;
    width = (int)PyLong_AsLong(pyArg2);
    height = (int)PyLong_AsLong(pyArg3);
    if (width < 0 || height < 0 || (0 == width && 0 == height)) {
        PyErr_SetString(PyExc_RuntimeError, "PixResize error, invalid width or height value\n");
        return NULL;
    }
    tcaxlib_convert_py_pix(pyPix, &pixSrc);
    pixDst.width = width;
    pixDst.height = height;
    tcaxlib_resample_pix(&pixSrc, &pixDst);
    free(pixSrc.buf);
    return tcaxlib_convert_pix(&pixDst, 1);
}

TCAX_PyPix tcaxlib_resample_py_pix_ex(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3;
    TCAX_PyPix pyPix;
    TCAX_Pix pixSrc;
    TCAX_Pix pixDst;
    double width, height;
    if (PyTuple_GET_SIZE(args) < 3) {
        PyErr_SetString(PyExc_RuntimeError, "PixResizeF error, too less parameters - `(PIX, width, height)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "PixResizeF error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "PixResizeF error, the 2nd param should be a float - `width'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "PixResizeF error, the 3rd param should be a float - `height'\n");
        return NULL;
    }
    pyPix = (TCAX_PyPix)pyArg1;
    width = PyFloat_AsDouble(pyArg2);
    height = PyFloat_AsDouble(pyArg3);
    if (width < 0 || height < 0 || (0 == width && 0 == height)) {
        PyErr_SetString(PyExc_RuntimeError, "PixResizeF error, invalid width or height value\n");
        return NULL;
    }
    tcaxlib_convert_py_pix(pyPix, &pixSrc);
    tcaxlib_resample_pix_ex(&pixSrc, &pixDst, width, height);
    free(pixSrc.buf);
    return tcaxlib_convert_pix(&pixDst, 1);
}

TCAX_PyPix tcaxlib_apply_pix_texture(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    TCAX_Pix pix;
    TCAX_Pix textureSrc;
    TCAX_Pix texture;
    int i, alpha;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "PixTexture error, too less parameters - `(PIX, texture)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "PixTexture error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyTuple_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "PixTexture error, the 2nd param should be a tuple - `texture'\n");
        return NULL;
    }
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg1, &pix);
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg2, &textureSrc);
    texture.width = pix.width;
    texture.height = pix.height;
    tcaxlib_resample_pix(&textureSrc, &texture);
    free(textureSrc.buf);
    for (i = 0; i < pix.size; i += 4) {
        if ((alpha = texture.buf[i + 3]) != 0) {
            pix.buf[i]     = texture.buf[i]     * alpha / 255 + pix.buf[i]     * (255 - alpha) / 255;
            pix.buf[i + 1] = texture.buf[i + 1] * alpha / 255 + pix.buf[i + 1] * (255 - alpha) / 255;
            pix.buf[i + 2] = texture.buf[i + 2] * alpha / 255 + pix.buf[i + 2] * (255 - alpha) / 255;
        }
    }
    free(texture.buf);
    return tcaxlib_convert_pix(&pix, 1);
}

TCAX_PyPix tcaxlib_apply_pix_mask(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    TCAX_Pix pix;
    TCAX_Pix mask;
    int w, h, offset_x, offset_y, index1, index2;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "PixMask error, too less parameters - `(PIX, mask)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "PixMask error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyTuple_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "PixMask error, the 2nd param should be a tuple - `mask'\n");
        return NULL;
    }
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg1, &pix);
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg2, &mask);
    offset_x = (int)(pix.initX - mask.initX);
    offset_y = (int)(pix.initY - mask.initY);
    for (h = 0; h < pix.height; h++) {
        for (w = 0; w < pix.width; w++) {
            index1 = h * (pix.width << 2) + (w << 2) + 3;
            index2 = (((h + offset_y) * mask.width + w + offset_x) << 2) + 3;
            if (!(0 <= offset_y + h && offset_y + h < mask.height && 0 <= offset_x + w && offset_x + w < mask.width && 0 != mask.buf[index2]))
                pix.buf[index1] = 0;
            else
                pix.buf[index1] = pix.buf[index1] * mask.buf[index2] / 255;
        }
    }
    free(mask.buf);
    return tcaxlib_convert_pix(&pix, 1);
}

TCAX_PyPix tcaxlib_pix_replace_alpha(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    TCAX_Pix pix;
    TCAX_Pix alphaSrc;
    TCAX_Pix alpha;
    int i;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "PixReplaceAlpha error, too less parameters - `(PIX, PIX_alpha)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "PixReplaceAlpha error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyTuple_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "PixReplaceAlpha error, the 2nd param should be a tuple - `PIX_alpha'\n");
        return NULL;
    }
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg1, &pix);
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg2, &alphaSrc);
    alpha.width = pix.width;
    alpha.height = pix.height;
    tcaxlib_resample_pix(&alphaSrc, &alpha);
    free(alphaSrc.buf);
    for (i = 0; i < pix.size; i += 4) {
        if (0 != pix.buf[i + 3])
            pix.buf[i + 3] = alpha.buf[i + 3];
    }
    free(alpha.buf);
    return tcaxlib_convert_pix(&pix, 1);
}


const TCAX_pPix tcaxlib_enlarge_pix(const TCAX_pPix pPixSrc, TCAX_pPix pPixDst) {
    int w, h, offset_x, offset_y, srcIndex, dstIndex;
    offset_x = (pPixDst->width - pPixSrc->width) / 2;
    offset_y = (pPixDst->height - pPixSrc->height) / 2;
    pPixDst->initX = pPixSrc->initX - offset_x;
    pPixDst->initY = pPixSrc->initY - offset_y;
    pPixDst->size = pPixDst->height * (pPixDst->width << 2);
    pPixDst->buf = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    memset(pPixDst->buf, 0, pPixDst->size * sizeof(unsigned char));
    for (w = 0; w < pPixSrc->width; w++) {
        for (h = 0; h < pPixSrc->height; h++) {
            dstIndex = ((h + offset_y) * pPixDst->width + w + offset_x) << 2;
            srcIndex = (h * pPixSrc->width + w) << 2;
            memcpy(pPixDst->buf + dstIndex, pPixSrc->buf + srcIndex, 4 * sizeof(unsigned char));
        }
    }
    return (const TCAX_pPix)pPixDst;
}

TCAX_PyPix tcaxlib_enlarge_py_pix(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3;
    TCAX_PyPix pyPix;
    TCAX_Pix pixSrc;
    TCAX_Pix pixDst;
    int x, y;
    if (PyTuple_GET_SIZE(args) < 3) {
        PyErr_SetString(PyExc_RuntimeError, "PixEnlarge error, too less parameters - `(PIX, x, y)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "PixEnlarge error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "PixEnlarge error, the 2nd param should be an integer - `x'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "PixEnlarge error, the 3rd param should be an integer - `y'\n");
        return NULL;
    }
    pyPix = (TCAX_PyPix)pyArg1;
    x = (int)PyLong_AsLong(pyArg2);
    y = (int)PyLong_AsLong(pyArg3);
    tcaxlib_convert_py_pix(pyPix, &pixSrc);
    pixDst.width = pixSrc.width + x;
    pixDst.height = pixSrc.height + y;
    tcaxlib_enlarge_pix(&pixSrc, &pixDst);
    free(pixSrc.buf);
    return tcaxlib_convert_pix(&pixDst, 1);
}

TCAX_PyPix tcaxlib_crop_py_pix(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5;
    TCAX_PyPix pyPix;
    TCAX_Pix pixSrc;
    TCAX_Pix pixDst;
    int left, top, right, bottom;
    int h, w, srcIndex, dstIndex;
    if (PyTuple_GET_SIZE(args) < 5) {
        PyErr_SetString(PyExc_RuntimeError, "PixCrop error, too less parameters - `(PIX, left, top, right, bottom)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "PixCrop error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "PixCrop error, the 2nd param should be an integer - `left'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "PixCrop error, the 3rd param should be an integer - `top'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "PixCrop error, the 4th param should be an integer - `right'\n");
        return NULL;
    }
    pyArg5 = PyTuple_GET_ITEM(args, 4);
    if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
        PyErr_SetString(PyExc_RuntimeError, "PixCrop error, the 5th param should be an integer - `bottom'\n");
        return NULL;
    }
    pyPix = (TCAX_PyPix)pyArg1;
    left = (int)PyLong_AsLong(pyArg2);
    top = (int)PyLong_AsLong(pyArg3);
    right = (int)PyLong_AsLong(pyArg4);
    bottom = (int)PyLong_AsLong(pyArg5);
    tcaxlib_convert_py_pix(pyPix, &pixSrc);
    if (right <= 0) right += pixSrc.width;
    if (bottom <= 0) bottom += pixSrc.height;
    if (left < 0 || top < 0 || right > pixSrc.width || bottom > pixSrc.height || left >= right || top >= bottom) {
        free(pixSrc.buf);
        PyErr_SetString(PyExc_RuntimeError, "PixCrop error, invalid value(s) in `left, top, right, bottom'\n");
        return NULL;
    }
    pixDst.initX = pixSrc.initX + left;
    pixDst.initY = pixSrc.initY + top;
    pixDst.width = right - left;
    pixDst.height = bottom - top;
    pixDst.size = pixDst.height * (pixDst.width << 2);
    pixDst.buf = (unsigned char *)malloc(pixDst.size * sizeof(unsigned char));
    for (h = 0; h < pixDst.height; h++) {
        for (w = 0; w < pixDst.width; w++) {
            dstIndex = (h * pixDst.width + w) << 2;
            srcIndex = ((h + top) * pixSrc.width + w + left) << 2;
            memcpy(pixDst.buf + dstIndex, pixSrc.buf + srcIndex, 4 * sizeof(unsigned char));
        }
    }
    free(pixSrc.buf);
    return tcaxlib_convert_pix(&pixDst, 1);
}

TCAX_PyPix tcaxlib_py_pix_strip_blank(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    TCAX_PyPix pyPix;
    TCAX_Pix pixSrc;
    TCAX_Pix pixDst;
    int left, top, right, bottom;
    int h, w, srcIndex, dstIndex;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "PixStrip error, too less parameters - `(PIX)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "PixStrip error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyPix = (TCAX_PyPix)pyArg1;
    tcaxlib_convert_py_pix(pyPix, &pixSrc);
    left = pixSrc.width;
    right = 0;
    top = pixSrc.height;
    bottom = 0;
    for (h = 0; h < pixSrc.height; h++) {
        for (w = 0; w < pixSrc.width; w++) {
            srcIndex = (h * pixSrc.width + w) << 2;
            if (pixSrc.buf[srcIndex + 3] != 0) {
                left = __min(left, w);
                right = __max(right, w + 1);
                top = __min(top, h);
                bottom = __max(bottom, h + 1);
            }
        }
    }
    pixDst.initX = pixSrc.initX + left;
    pixDst.initY = pixSrc.initY + top;
    pixDst.width = right - left;
    pixDst.height = bottom - top;
    if (pixDst.width <= 0 || pixDst.height <= 0) {
        free(pixSrc.buf);
        Py_INCREF(pyPix);
        return pyPix;
    }
    pixDst.size = pixDst.height * (pixDst.width << 2);
    pixDst.buf = (unsigned char *)malloc(pixDst.size * sizeof(unsigned char));
    for (h = 0; h < pixDst.height; h++) {
        for (w = 0; w < pixDst.width; w++) {
            dstIndex = (h * pixDst.width + w) << 2;
            srcIndex = ((h + top) * pixSrc.width + w + left) << 2;
            memcpy(pixDst.buf + dstIndex, pixSrc.buf + srcIndex, 4 * sizeof(unsigned char));
        }
    }
    free(pixSrc.buf);
    return tcaxlib_convert_pix(&pixDst, 1);
}

/* Gauss IIR blur */
/* Convert from separated to premultiplied alpha, on a single scan line. */
static void _tcaxlib_multiply_alpha(unsigned char *buf, int length) {
    int i, index;
    double a;
    for (i = 0; i < length; i++) {
        index = i << 2;
        a = buf[index + 3] / 255.0;
        buf[index] = (unsigned char)(buf[index] * a);
        buf[index + 1] = (unsigned char)(buf[index + 1] * a);
        buf[index + 2] = (unsigned char)(buf[index + 2] * a);
    }
}

/* Convert from premultiplied to separated alpha, on a single scan line. */
static void _tcaxlib_separate_alpha(unsigned char *buf, int length) {
    int i, index;
    unsigned char alpha;
    double a;
    for (i = 0; i < length; i++) {
        index = i << 2;
        alpha = buf[index + 3];
        if (0 != alpha && 255 != alpha) {
            a = 255 / (double)alpha;
            buf[index] = __min(255, (int)(buf[index] * a));
            buf[index + 1] = __min(255, (int)(buf[index + 1] * a));
            buf[index + 2] = __min(255, (int)(buf[index + 2] * a));
        }
    }
}

static void _tcaxlib_gauss_iir_find_constants(double *n_p, double *n_m, double *d_p, double *d_m, double *bd_p, double *bd_m, double std_dev) {
    int i;
    double constants[8];
    double div;
    double sum_n_p, sum_n_m, sum_d_p, a, b;
    /* The constants used in the implemenation of a casual sequence using a 4th order approximation of the gaussian operator */
    div = sqrt(2 * MATH_PI) * std_dev;
    constants[0] = -1.783 / std_dev;
    constants[1] = -1.723 / std_dev;
    constants[2] = 0.6318 / std_dev;
    constants[3] = 1.997  / std_dev;
    constants[4] = 1.6803 / div;
    constants[5] = 3.735 / div;
    constants[6] = -0.6803 / div;
    constants[7] = -0.2598 / div;
    n_p[0] = constants[4] + constants[6];
    n_p[1] = exp(constants[1]) * (constants[7] * sin(constants[3]) - (constants[6] + 2 * constants[4]) * cos(constants[3])) + exp(constants[0]) * (constants[5] * sin(constants[2]) - (2 * constants[6] + constants[4]) * cos(constants[2]));
    n_p[2] = 2 * exp(constants[0] + constants[1]) * ((constants[4] + constants[6]) * cos(constants[3]) * cos(constants[2]) - constants[5] * cos(constants[3]) * sin(constants[2]) - constants[7] * cos(constants[2]) * sin(constants[3])) + constants[6] * exp(2 * constants[0]) + constants[4] * exp(2 * constants[1]);
    n_p[3] = exp(constants[1] + 2 * constants[0]) * (constants[7] * sin(constants[3]) - constants[6] * cos(constants[3])) + exp(constants[0] + 2 * constants[1]) * (constants[5] * sin(constants[2]) - constants[4] * cos(constants[2]));
    n_p[4] = 0;
    d_p[0] = 0;
    d_p[1] = -2 * exp(constants[1]) * cos(constants[3]) - 2 * exp(constants[0]) * cos(constants[2]);
    d_p[2] = 4 * cos(constants[3]) * cos(constants[2]) * exp(constants[0] + constants[1]) + exp(2 * constants[1]) + exp(2 * constants[0]);
    d_p[3] = -2 * cos(constants[2]) * exp(constants[0] + 2 * constants[1]) - 2 * cos(constants[3]) * exp(constants[1] + 2 * constants[0]);
    d_p[4] = exp(2 * constants[0] + 2 * constants[1]);
    for (i = 0; i < 5; i++)
        d_m[i] = d_p[i];
    n_m[0] = 0;
    for (i = 1; i < 5; i++)
        n_m[i] = n_p[i] - d_p[i] * n_p[0];
    sum_n_p = 0;
    sum_n_m = 0;
    sum_d_p = 0;
    for (i = 0; i < 5; i++) {
        sum_n_p += n_p[i];
        sum_n_m += n_m[i];
        sum_d_p += d_p[i];
    }
    a = sum_n_p / (1 + sum_d_p);
    b = sum_n_m / (1 + sum_d_p);
    for (i = 0; i < 5; i++) {
        bd_p[i] = d_p[i] * a;
        bd_m[i] = d_m[i] * b;
    }
}

static void _tcaxlib_gauss_iir_blur(unsigned char *img, int width, int height, int vertRadius, int horiRadius) {
    int i, j, b, x, y, vert, hori, size, terms;
    double n_p[5], n_m[5], d_p[5], d_m[5], bd_p[5], bd_m[5];
    double std_dev;
    int initial_p[4], initial_m[4];
    unsigned char *src;
    unsigned char *sp_p, *sp_m, *p;
    double *val_p, *val_m;
    double *vp, *vm, *vpptr, *vmptr, *pTmp_p, *pTmp_m;
    /* First the vertical pass */
    vert = vertRadius + 1;
    std_dev = sqrt(-(vert * vert) / (2 * log(1 / 255.0)));
    /* derive the constants for calculating the gaussian from the std dev */
    _tcaxlib_gauss_iir_find_constants(n_p, n_m, d_p, d_m, bd_p, bd_m, std_dev);
    size = height << 2;
    val_p = (double *)malloc(size * sizeof(double));
    val_m = (double *)malloc(size * sizeof(double));
    src = (unsigned char *)malloc(size * sizeof(unsigned char));
    for (x = 0; x < width; x++) {
        memset(val_p, 0, size * sizeof(double));
        memset(val_m, 0, size * sizeof(double));
        /* get col */
        for (y = 0; y < height; y++)
            memcpy(&src[y << 2], &img[(y * width + x) << 2], sizeof(unsigned char) << 2);
        _tcaxlib_multiply_alpha(src, height);
        sp_p = src;
        sp_m = src + (height - 1) * 4;
        vp = val_p;
        vm = val_m + (height - 1) * 4;
        /* Set up the first vals */
        for (i = 0; i < 4; i++) {
            initial_p[i] = sp_p[i];
            initial_m[i] = sp_m[i];
        }
        for (y = 0; y < height; y++) {
            terms = __min(y, 4);
            for (b = 0; b < 4; b++) {
                vpptr = vp + b;
                vmptr = vm + b;
                for (i = 0; i <= terms; i++) {
                    *vpptr += n_p[i] * sp_p[(-i * 4) + b] - d_p[i] * vp[(-i * 4) + b];
                    *vmptr += n_m[i] * sp_m[(i * 4) + b] - d_m[i] * vm[(i * 4) + b];
                }
                for (j = i ; j <= 4 ; j++) {
                    *vpptr += (n_p[j] - bd_p[j]) * initial_p[b];
                    *vmptr += (n_m[j] - bd_m[j]) * initial_m[b];
                }
            }
            sp_p += 4;
            sp_m -= 4;
            vp += 4;
            vm -= 4;
        }
        pTmp_p = val_p;
        pTmp_m = val_m;
        for (i = 0; i < height; i++) {
            p = &img[(i * width + x) << 2];
            for (b = 0; b < 4; b++) {
                p[b] = CLIP_0_255((int)(*pTmp_p + *pTmp_m));
                pTmp_p++;
                pTmp_m++;
            }
        }
    }
    /* Now the horizontal pass */
    hori = horiRadius + 1;
    std_dev = sqrt(-(hori * hori) / (2 * log(1 / 255.0)));
    /*derive the constants for calculating the gaussian from the std dev */
    _tcaxlib_gauss_iir_find_constants(n_p, n_m, d_p, d_m, bd_p, bd_m, std_dev);
    free(val_p);
    free(val_m);
    free(src);
    size = width << 2;
    val_p = (double *)malloc(size * sizeof(double));
    val_m = (double *)malloc(size * sizeof(double));
    src = (unsigned char *)malloc(size * sizeof(unsigned char));
    for (y = 0; y < height; y++) {
        memset(val_p, 0, size * sizeof(double));
        memset(val_m, 0, size * sizeof(double));
        /* get row */
        memcpy(src, &img[y * (width << 2)], size * sizeof(unsigned char));
        sp_p = src;
        sp_m = src + (width - 1) * 4;
        vp = val_p;
        vm = val_m + (width - 1) * 4;
        /* Set up the first vals */
        for (i = 0; i < 4; i++) {
            initial_p[i] = sp_p[i];
            initial_m[i] = sp_m[i];
        }
        for (x = 0; x < width; x++) {
            terms = __min(x, 4);
            for (b = 0; b < 4; b++) {
                vpptr = vp + b;
                vmptr = vm + b;
                for (i = 0; i <= terms; i++) {
                    *vpptr += n_p[i] * sp_p[(-i * 4) + b] - d_p[i] * vp[(-i * 4) + b];
                    *vmptr += n_m[i] * sp_m[(i * 4) + b] - d_m[i] * vm[(i * 4) + b];
                }
                for (j = i; j <= 4; j++) {
                    *vpptr += (n_p[j] - bd_p[j]) * initial_p[b];
                    *vmptr += (n_m[j] - bd_m[j]) * initial_m[b];
                }
            }
            sp_p += 4;
            sp_m -= 4;
            vp += 4;
            vm -= 4;
        }
        pTmp_p = val_p;
        pTmp_m = val_m;
        for (i = 0; i < width; i++) {
            p = &img[(y * width + i) << 2];
            for (b = 0; b < 4; b++) {
                p[b] = CLIP_0_255((int)(*pTmp_p + *pTmp_m));
                pTmp_p++;
                pTmp_m++;
            }
        }
        _tcaxlib_separate_alpha(&img[y * (width << 2)], width);
    }
    free(val_p);
    free(val_m);
    free(src);
}

TCAX_PyPix tcaxlib_apply_gauss_blur(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    TCAX_PyPix pyPix;
    TCAX_Pix pixSrc;
    TCAX_Pix pixDst;
    int radius;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "PixBlur error, too less parameters - `(PIX, radius)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "PixBlur error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "PixBlur error, the 2nd param should be an integer - `radius'\n");
        return NULL;
    }
    pyPix = (TCAX_PyPix)pyArg1;
    radius = (int)PyLong_AsLong(pyArg2);
    tcaxlib_convert_py_pix(pyPix, &pixSrc);
    pixDst.width = pixSrc.width + 2 * radius;
    pixDst.height = pixSrc.height + 2 * radius;
    tcaxlib_enlarge_pix(&pixSrc, &pixDst);
    free(pixSrc.buf);
    _tcaxlib_gauss_iir_blur(pixDst.buf, pixDst.width, pixDst.height, radius, radius);
    return tcaxlib_convert_pix(&pixDst, 1);
}

/**
 * PIX blender, 0 - video, 1 - back, 2 - overlay
 * R_t = R1 * A1 / 255 + R0 * (255 - A1) / 255
 * R = R2 * A2 / 255 + R_t * (255 - A2) / 255
 * R = Rx * Ax / 255 + R0 * (255 - Ax) / 255
 * result:
 * Ax = 255 - (255 - A1) * (255 - A2) / 255
 * Rx = (R2 * A2 + R1 * A1 * (255 - A2) / 255) / Ax
 */
TCAX_PyPix tcaxlib_combine_two_pixs(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    int i, h, w, dstIndex, srcIndex, left, top, right, bottom, offset_x1, offset_y1, offset_x2, offset_y2;
    unsigned char *overlayBuf;
    unsigned char R1, R2, R;
    unsigned char G1, G2, G;
    unsigned char B1, B2, B;
    unsigned char A1, A2, A;
    TCAX_Pix backPix;
    TCAX_Pix overlayPix;
    TCAX_Pix blendedPix;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "CombinePixs error, too less parameters - `(back, overlay)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "CombinePixs error, the 1st param should be a tuple - `back'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyTuple_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "CombinePixs error, the 2nd param should be a tuple - `overlay'\n");
        return NULL;
    }
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg1, &backPix);
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg2, &overlayPix);
    left = __min((int)backPix.initX, (int)overlayPix.initX);
    top = __min((int)backPix.initY, (int)overlayPix.initY);
    right = __max((int)backPix.initX + backPix.width, (int)overlayPix.initX + overlayPix.width);
    bottom = __max((int)backPix.initY + backPix.height, (int)overlayPix.initY + overlayPix.height);
    blendedPix.width = right - left;
    blendedPix.height = bottom - top;
    blendedPix.size = blendedPix.height * (blendedPix.width << 2);
    blendedPix.buf = (unsigned char *)malloc(blendedPix.size * sizeof(unsigned char));
    memset(blendedPix.buf, 0, blendedPix.size * sizeof(unsigned char));
    offset_x1 = (int)backPix.initX - left;
    offset_y1 = (int)backPix.initY - top;
    for (h = 0; h < backPix.height; h++) {
        for (w = 0; w < backPix.width; w++) {
            dstIndex = ((h + offset_y1) * blendedPix.width + w + offset_x1) << 2;
            srcIndex = (h * backPix.width + w) << 2;
            memcpy(blendedPix.buf + dstIndex, backPix.buf + srcIndex, 4 * sizeof(unsigned char));
        }
    }
    offset_x2 = (int)overlayPix.initX - left;
    offset_y2 = (int)overlayPix.initY - top;
    overlayBuf = (unsigned char *)malloc(blendedPix.size * sizeof(unsigned char));
    memset(overlayBuf, 0, blendedPix.size * sizeof(unsigned char));
    for (h = 0; h < overlayPix.height; h++) {
        for (w = 0; w < overlayPix.width; w++) {
            dstIndex = ((h + offset_y2) * blendedPix.width + w + offset_x2) << 2;
            srcIndex = (h * overlayPix.width + w) << 2;
            memcpy(overlayBuf + dstIndex, overlayPix.buf + srcIndex, 4 * sizeof(unsigned char));
        }
    }
    blendedPix.initX = left;
    blendedPix.initY = top;
    for (i = 0; i < blendedPix.size; i += 4) {
        R1 = blendedPix.buf[i];
        G1 = blendedPix.buf[i + 1];
        B1 = blendedPix.buf[i + 2];
        A1 = blendedPix.buf[i + 3];
        R2 = overlayBuf[i];
        G2 = overlayBuf[i + 1];
        B2 = overlayBuf[i + 2];
        A2 = overlayBuf[i + 3];
        A = 255 - (255 - A1) * (255 - A2) / 255;
        if (0 != A) {
            R = (R2 * A2 + R1 * A1 * (255 - A2) / 255) / A;
            G = (G2 * A2 + G1 * A1 * (255 - A2) / 255) / A;
            B = (B2 * A2 + B1 * A1 * (255 - A2) / 255) / A;
            blendedPix.buf[i]     = R;
            blendedPix.buf[i + 1] = G;
            blendedPix.buf[i + 2] = B;
            blendedPix.buf[i + 3] = A;
        }
    }
    free(backPix.buf);
    free(overlayPix.buf);
    free(overlayBuf);
    return tcaxlib_convert_pix(&blendedPix, 1);
}

TCAX_PyPix tcaxlib_pix_color_multiply(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5;
    TCAX_Pix pix;
    double r_f, g_f, b_f, a_f;
    int i;
    if (PyTuple_GET_SIZE(args) < 5) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorMul error, too less parameters - `(PIX, r_f, g_f, b_f, a_f)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorMul error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorMul error, the 2nd param should be a float - `r_f'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorMul error, the 3rd param should be a float - `g_f'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorMul error, the 4th param should be a float - `b_f'\n");
        return NULL;
    }
    pyArg5 = PyTuple_GET_ITEM(args, 4);
    if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorMul error, the 5th param should be a float - `a_f'\n");
        return NULL;
    }
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg1, &pix);
    r_f = PyFloat_AsDouble(pyArg2);
    g_f = PyFloat_AsDouble(pyArg3);
    b_f = PyFloat_AsDouble(pyArg4);
    a_f = PyFloat_AsDouble(pyArg5);
    for (i = 0; i < pix.size; i += 4) {
        if (0 != pix.buf[i + 3]) {
            pix.buf[i] = CLIP_0_255((int)(pix.buf[i] * r_f + 0.5));
            pix.buf[i + 1] = CLIP_0_255((int)(pix.buf[i + 1] * g_f + 0.5));
            pix.buf[i + 2] = CLIP_0_255((int)(pix.buf[i + 2] * b_f + 0.5));
            pix.buf[i + 3] = CLIP_0_255((int)(pix.buf[i + 3] * a_f + 0.5));
            if (0 == pix.buf[i + 3])
                pix.buf[i + 3] = 1;  /* transparent DIP will not be written to the tcas file */
        }
    }
    return tcaxlib_convert_pix(&pix, 1);
}

TCAX_PyPix tcaxlib_pix_color_shift(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5;
    TCAX_Pix pix;
    int r, g, b, a;
    int i;
    if (PyTuple_GET_SIZE(args) < 5) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorShift error, too less parameters - `(PIX, r, g, b, a)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorShift error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorShift error, the 2nd param should be an integer - `r'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorShift error, the 3rd param should be an integer - `g'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorShift error, the 4th param should be an integer - `b'\n");
        return NULL;
    }
    pyArg5 = PyTuple_GET_ITEM(args, 4);
    if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorShift error, the 5th param should be an integer - `a'\n");
        return NULL;
    }
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg1, &pix);
    r = (int)PyLong_AsLong(pyArg2);
    g = (int)PyLong_AsLong(pyArg3);
    b = (int)PyLong_AsLong(pyArg4);
    a = (int)PyLong_AsLong(pyArg5);
    for (i = 0; i < pix.size; i += 4) {
        if (0 != pix.buf[i + 3]) {
            pix.buf[i] = CLIP_0_255(pix.buf[i] + r);
            pix.buf[i + 1] = CLIP_0_255(pix.buf[i + 1] + g);
            pix.buf[i + 2] = CLIP_0_255(pix.buf[i + 2] + b);
            pix.buf[i + 3] = CLIP_0_255(pix.buf[i + 3] + a);
            if (0 == pix.buf[i + 3])
                pix.buf[i + 3] = 1;  /* transparent DIP will not be written to the tcas file */
        }
    }
    return tcaxlib_convert_pix(&pix, 1);
}

TCAX_PyPix tcaxlib_pix_color_transparent(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    TCAX_Pix pix;
    unsigned long rgb;
    int i;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorTrans error, too less parameters - `(PIX, rgb)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorTrans error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorTrans error, the 2nd param should be an integer - `rgb'\n");
        return NULL;
    }
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg1, &pix);
    rgb = (unsigned long)PyLong_AsUnsignedLong(pyArg2);
    for (i = 0; i < pix.size; i += 4) {
        if ((0x00FFFFFF & (*((unsigned long *)&pix.buf[i]))) == rgb)
            pix.buf[i + 3] = 0;
    }
    return tcaxlib_convert_pix(&pix, 1);
}

TCAX_PyPix tcaxlib_pix_color_flat_rgba(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    TCAX_Pix pix;
    unsigned long rgba;
    int i;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorRGBA error, too less parameters - `(PIX, rgba)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorRGBA error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorRGBA error, the 2nd param should be an integer - `rgba'\n");
        return NULL;
    }
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg1, &pix);
    rgba = (unsigned long)PyLong_AsUnsignedLong(pyArg2);
    for (i = 0; i < pix.size; i += 4) {
        if (0 != pix.buf[i + 3])
            (*((unsigned long *)&pix.buf[i])) = rgba;
    }
    return tcaxlib_convert_pix(&pix, 1);
}

TCAX_PyPix tcaxlib_pix_color_flat_rgb(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    TCAX_Pix pix;
    unsigned long rgb;
    int i;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorRGB error, too less parameters - `(PIX, rgb)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorRGB error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorRGB error, the 2nd param should be an integer - `rgb'\n");
        return NULL;
    }
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg1, &pix);
    rgb = (unsigned long)PyLong_AsUnsignedLong(pyArg2);
    for (i = 0; i < pix.size; i += 4) {
        if (0 != pix.buf[i + 3])
            memcpy(pix.buf + i, &rgb, 3 * sizeof(unsigned char));
    }
    return tcaxlib_convert_pix(&pix, 1);
}

TCAX_PyPix tcaxlib_pix_color_flat_alpha(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    TCAX_Pix pix;
    unsigned char alpha;
    int i;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorA error, too less parameters - `(PIX, alpha)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorA error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "PixColorA error, the 2nd param should be an integer - `alpha'\n");
        return NULL;
    }
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg1, &pix);
    alpha = (unsigned char)PyLong_AsLong(pyArg2);
    for (i = 0; i < pix.size; i += 4) {
        if (0 != pix.buf[i + 3])
            pix.buf[i + 3] = alpha;
    }
    return tcaxlib_convert_pix(&pix, 1);
}

const TCAX_pPix tcaxlib_bilinear_filter_internal(const TCAX_pPix pPixSrc, double offset_x, double offset_y, TCAX_pPix pPixDst) {
    int h, w, index, index1, index2, index3, index4;
    double initPosX, initPosY;
    double u_r, v_r, u_o, v_o;
    unsigned char *buf1, *buf2, *buf3, *buf4;    /* top-left, top-right, bottom-left, bottom-right */
    initPosX = pPixSrc->initX + offset_x;
    initPosY = pPixSrc->initY + offset_y;
    pPixDst->initX = pPixSrc->initX;    /* when computing initX we need to use float, but when come to write them to TCAS file we will use integer instead */
    pPixDst->initY = pPixSrc->initY;
    pPixDst->width = pPixSrc->width + 1;
    pPixDst->height = pPixSrc->height + 1;
    pPixDst->size = pPixDst->height * (pPixDst->width << 2);
    buf1 = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    buf2 = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    buf3 = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    buf4 = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    memset(buf1, 0, pPixDst->size * sizeof(unsigned char));
    memset(buf2, 0, pPixDst->size * sizeof(unsigned char));
    memset(buf3, 0, pPixDst->size * sizeof(unsigned char));
    memset(buf4, 0, pPixDst->size * sizeof(unsigned char));
    u_r = initPosX - (int)initPosX;    /* factor for the right */
    if (u_r < 0) u_r++;
    v_r = initPosY - (int)initPosY;    /* factor for the bottom */
    if (v_r < 0) v_r++;
    u_o = 1 - u_r;    /* factor for the left */
    v_o = 1 - v_r;    /* factor for the top */
    for (h = 0; h < pPixSrc->height; h++) {
        for (w = 0; w < pPixSrc->width; w++) {
            index = (h * pPixSrc->width + w) << 2;
            index1 = (h * pPixDst->width + w) << 2;
            index2 = (h * pPixDst->width + w + 1) << 2;
            index3 = ((h + 1) * pPixDst->width + w) << 2;
            index4 = ((h + 1) * pPixDst->width + w + 1) << 2;
            memcpy(buf1 + index1, pPixSrc->buf + index, 4 * sizeof(unsigned char));
            memcpy(buf2 + index2, pPixSrc->buf + index, 4 * sizeof(unsigned char));
            memcpy(buf3 + index3, pPixSrc->buf + index, 4 * sizeof(unsigned char));
            memcpy(buf4 + index4, pPixSrc->buf + index, 4 * sizeof(unsigned char));
        }
    }
    pPixDst->buf = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    for (h = 0; h < pPixDst->height; h++) {
        for (w = 0; w < pPixDst->width; w++) {
            index = (h * pPixDst->width + w) << 2;
            pPixDst->buf[index]     = CLIP_0_255((int)((buf1[index]     * u_o + buf2[index]     * u_r) * v_o + (buf3[index]     * u_o + buf4[index]     * u_r) * v_r));
            pPixDst->buf[index + 1] = CLIP_0_255((int)((buf1[index + 1] * u_o + buf2[index + 1] * u_r) * v_o + (buf3[index + 1] * u_o + buf4[index + 1] * u_r) * v_r));
            pPixDst->buf[index + 2] = CLIP_0_255((int)((buf1[index + 2] * u_o + buf2[index + 2] * u_r) * v_o + (buf3[index + 2] * u_o + buf4[index + 2] * u_r) * v_r));
            pPixDst->buf[index + 3] = CLIP_0_255((int)((buf1[index + 3] * u_o + buf2[index + 3] * u_r) * v_o + (buf3[index + 3] * u_o + buf4[index + 3] * u_r) * v_r));
        }
    }
    free(buf1);
    free(buf2);
    free(buf3);
    free(buf4);
    return (const TCAX_pPix)pPixDst;
}

TCAX_PyPix tcaxlib_bilinear_filter(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3;
    TCAX_Pix pixSrc;
    TCAX_Pix pixDst;
    double offset_x, offset_y;
    if (PyTuple_GET_SIZE(args) < 3) {
        PyErr_SetString(PyExc_RuntimeError, "BilinearFilter error, too less parameters - `(PIX, offset_x, offset_y)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "BilinearFilter error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "BilinearFilter error, the 2nd param should be a float - `offset_x'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "BilinearFilter error, the 3rd param should be a float - `offset_y'\n");
        return NULL;
    }
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg1, &pixSrc);
    offset_x = PyFloat_AsDouble(pyArg2);
    offset_y = PyFloat_AsDouble(pyArg3);
    tcaxlib_bilinear_filter_internal(&pixSrc, offset_x, offset_y, &pixDst);
    free(pixSrc.buf);
    return tcaxlib_convert_pix(&pixDst, 1);
}

const TCAX_pPix tcaxlib_scale_filter_internal(const TCAX_pPix pPixSrc, double offset_x, double offset_y, TCAX_pPix pPixDst) {
    const int m = 4;
    int i_w, i_h, i_mw, i_mh;
    double f_dx, f_dy, f_mdx, f_mdy;
    int i_mdx, i_mdy;
    int pad_dx_i, pad_dy_i, pad_dx, pad_dy;
    int sf_mw, sf_mh, sf_msize;
    unsigned char *sBuf, *bufm;
    int i, index1, index2, size;
    i_w = pPixSrc->width;
    i_h = pPixSrc->height;
    i_mw = m * i_w;
    i_mh = m * i_h;
    f_dx = pPixSrc->initX + offset_x;
    f_dy = pPixSrc->initY + offset_y;
    f_mdx = m * f_dx;
    f_mdy = m * f_dy;
    i_mdx = (int)f_mdx;
    i_mdy = (int)f_mdy;
    pad_dx_i = i_mdx % m;
    pad_dx = 0;
    if (0 != pad_dx_i)
        pad_dx = m;
    pad_dy_i = i_mdy % m;
    pad_dy = 0;
    if (0 != pad_dy_i)
        pad_dy = m;
    sf_mw = i_mw + pad_dx;
    sf_mh = i_mh + pad_dy;
    sf_msize = sf_mh * (sf_mw << 2);
    sBuf = (unsigned char *)malloc(sf_msize * sizeof(unsigned char));
    memset(sBuf, 0, sf_msize * sizeof(unsigned char));      /* now we've got the surface to draw */
    bufm = (unsigned char *)malloc(i_mh * (i_mw << 2) * sizeof(unsigned char));   /* magnitude the source to m * m */
    _tcaxlib_resample_rgba(pPixSrc->buf, i_w, i_h, bufm, i_mw, i_mh);
    size = (i_mw << 2) * sizeof(unsigned char);
    for (i = 0; i < i_mh; i++) {
        index1 = (((i + pad_dy_i) * sf_mw + pad_dx_i) << 2) * sizeof(unsigned char);
        index2 = i * (i_mw << 2) * sizeof(unsigned char);
        memcpy(sBuf + index1, bufm + index2, size);
    }
    free(bufm);
    pPixDst->initX = pPixSrc->initX;
    pPixDst->initY = pPixSrc->initY;
    pPixDst->width = sf_mw / m;
    pPixDst->height = sf_mh / m;
    pPixDst->size = pPixDst->height * (pPixDst->width << 2);
    pPixDst->buf = (unsigned char *)malloc(pPixDst->size * sizeof(unsigned char));
    _tcaxlib_resample_rgba(sBuf, sf_mw, sf_mh, pPixDst->buf, pPixDst->width, pPixDst->height);
    free(sBuf);
    return (const TCAX_pPix)pPixDst;
}

TCAX_PyPix tcaxlib_scale_filter(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3;
    TCAX_Pix pixSrc;
    TCAX_Pix pixDst;
    double offset_x, offset_y;
    if (PyTuple_GET_SIZE(args) < 3) {
        PyErr_SetString(PyExc_RuntimeError, "ScaleFilter error, too less parameters - `(PIX, offset_x, offset_y)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "ScaleFilter error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "ScaleFilter error, the 2nd param should be a float - `offset_x'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "ScaleFilter error, the 3rd param should be a float - `offset_y'\n");
        return NULL;
    }
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg1, &pixSrc);
    offset_x = PyFloat_AsDouble(pyArg2);
    offset_y = PyFloat_AsDouble(pyArg3);
    tcaxlib_scale_filter_internal(&pixSrc, offset_x, offset_y, &pixDst);
    free(pixSrc.buf);
    return tcaxlib_convert_pix(&pixDst, 1);
}

TCAX_PyBigPix tcaxlib_init_big_pix(PyObject *self, PyObject *args) {
    return (TCAX_PyBigPix)PyList_New(0);
}

TCAX_Py_Error_Code tcaxlib_big_pix_add(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5;
    PyObject *pyBigPixUnit;
    if (PyTuple_GET_SIZE(args) < 5) {
        PyErr_SetString(PyExc_RuntimeError, "BigPixAdd error, too less parameters - `(BIG_PIX, PIX, offset_x, offset_y, layer)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyList_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "BigPixAdd error, the 1st param should be a list - `BIG_PIX'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyTuple_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "BigPixAdd error, the 2nd param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "BigPixAdd error, the 3rd param should be a float - `offset_x'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "BigPixAdd error, the 4th param should be a float - `offset_y'\n");
        return NULL;
    }
    pyArg5 = PyTuple_GET_ITEM(args, 4);
    if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
        PyErr_SetString(PyExc_RuntimeError, "BigPixAdd error, the 5th param should be an integer - `layer'\n");
        return NULL;
    }
    pyBigPixUnit = PyTuple_New(4);
    Py_INCREF(pyArg2);
    Py_INCREF(pyArg3);
    Py_INCREF(pyArg4);
    Py_INCREF(pyArg5);
    PyTuple_SET_ITEM(pyBigPixUnit, 0, pyArg2);
    PyTuple_SET_ITEM(pyBigPixUnit, 1, pyArg3);
    PyTuple_SET_ITEM(pyBigPixUnit, 2, pyArg4);
    PyTuple_SET_ITEM(pyBigPixUnit, 3, pyArg5);
    PyList_Append((TCAX_PyBigPix)pyArg1, pyBigPixUnit);
    Py_CLEAR(pyBigPixUnit);
    return PyLong_FromLong(0);
}

TCAX_PyPix tcaxlib_convert_big_pix(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    TCAX_Pix pix;
    int i, j, h, w, srcIndex, dstIndex, count;
    PyObject *pyBigPixUnit;
    TCAX_pPix pPixSrc;
    double *pOffsetX, *pOffsetY;
    int *pLayer;
    TCAX_Pix pixTemp;
    double fTemp1, fTemp2;
    int iTemp;
    int initPosX, initPosY, left, top, right, bottom, offset_x, offset_y;
    unsigned char r0, g0, b0, a0, r, g, b, a, A;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "ConvertBigPix error, too less parameters - `(BIG_PIX)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyList_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "ConvertBigPix error, the 1st param should be a list - `BIG_PIX'\n");
        return NULL;
    }
    count = PyList_GET_SIZE((TCAX_PyBigPix)pyArg1);
    pPixSrc = (TCAX_pPix)malloc(count * sizeof(TCAX_Pix));
    pOffsetX = (double *)malloc(count * sizeof(double));
    pOffsetY = (double *)malloc(count * sizeof(double));
    pLayer = (int *)malloc(count * sizeof(int));
    for (i = 0; i < count; i++) {
        pyBigPixUnit = PyList_GET_ITEM((TCAX_PyBigPix)pyArg1, i);
        tcaxlib_convert_py_pix((const TCAX_PyPix)PyTuple_GET_ITEM(pyBigPixUnit, 0), &pPixSrc[i]);
        pOffsetX[i] = PyFloat_AsDouble(PyTuple_GET_ITEM(pyBigPixUnit, 1));
        pOffsetY[i] = PyFloat_AsDouble(PyTuple_GET_ITEM(pyBigPixUnit, 2));
        pLayer[i] = (int)PyLong_AsLong(PyTuple_GET_ITEM(pyBigPixUnit, 3));
    }
    for (i = 0; i < count; i++) {
        for (j = 0; j < count - i - 1; j++) {
            if (pLayer[j] > pLayer[j + 1]) {
                pixTemp = pPixSrc[j + 1];
                fTemp1 = pOffsetX[j + 1];
                fTemp2 = pOffsetY[j + 1];
                iTemp = pLayer[j + 1];
                pPixSrc[j + 1] = pPixSrc[j];
                pOffsetX[j + 1] = pOffsetX[j];
                pOffsetY[j + 1] = pOffsetY[j];
                pLayer[j + 1] = pLayer[j];
                pPixSrc[j] = pixTemp;
                pOffsetX[j] = fTemp1;
                pOffsetY[j] = fTemp2;
                pLayer[j] = iTemp;
            }
        }
    }
    initPosX = (int)(pOffsetX[0] + pPixSrc[0].initX);
    initPosY = (int)(pOffsetY[0] + pPixSrc[0].initY);
    left = initPosX;
    top = initPosY;
    right = initPosX + pPixSrc[0].width;
    bottom = initPosY + pPixSrc[0].height;
    pix.initX = pOffsetX[0] + pPixSrc[0].initX;
    pix.initY = pOffsetY[0] + pPixSrc[0].initY;
    for (i = 1; i < count; i++) {
        initPosX = (int)(pOffsetX[i] + pPixSrc[i].initX);
        if (initPosX < left) {
            left = initPosX;
            pix.initX = pOffsetX[i] + pPixSrc[i].initX;
        }
        initPosY = (int)(pOffsetY[i] + pPixSrc[i].initY);
        if (initPosY < top) {
            top = initPosY;
            pix.initY = pOffsetY[i] + pPixSrc[i].initY;
        }
        right = __max(right, initPosX + pPixSrc[i].width);
        bottom = __max(bottom, initPosY + pPixSrc[i].height);
    }
    pix.width = right - left;
    pix.height = bottom - top;
    pix.size = pix.height * (pix.width << 2);
    pix.buf = (unsigned char *)malloc(pix.size * sizeof(unsigned char));
    memset(pix.buf, 0, pix.size * sizeof(unsigned char));
    for (i = 0; i < count; i++) {
        initPosX = (int)(pOffsetX[i] + pPixSrc[i].initX);
        initPosY = (int)(pOffsetY[i] + pPixSrc[i].initY);
        offset_x = initPosX - left;
        offset_y = initPosY - top;
        for (h = 0; h < pPixSrc[i].height; h++) {
            for (w = 0; w < pPixSrc[i].width; w++) {
                dstIndex = ((h + offset_y) * pix.width + w + offset_x) << 2;
                srcIndex = (h * pPixSrc[i].width + w) << 2;
                r = pPixSrc[i].buf[srcIndex];
                g = pPixSrc[i].buf[srcIndex + 1];
                b = pPixSrc[i].buf[srcIndex + 2];
                a = pPixSrc[i].buf[srcIndex + 3];
                r0 = pix.buf[dstIndex];
                g0 = pix.buf[dstIndex + 1];
                b0 = pix.buf[dstIndex + 2];
                a0 = pix.buf[dstIndex + 3];
                A = 255 - (255 - a) * (255 - a0) / 255;
                if (0 != A) {
                    pix.buf[dstIndex]     = (r * a + r0 * a0 * (255 - a) / 255) / A;
                    pix.buf[dstIndex + 1] = (g * a + g0 * a0 * (255 - a) / 255) / A;
                    pix.buf[dstIndex + 2] = (b * a + b0 * a0 * (255 - a) / 255) / A;
                    pix.buf[dstIndex + 3] =  A;
                }
            }
        }
        free(pPixSrc[i].buf);
    }
    free(pPixSrc);
    free(pOffsetX);
    free(pOffsetY);
    free(pLayer);
    return tcaxlib_convert_pix(&pix, 1);
}

