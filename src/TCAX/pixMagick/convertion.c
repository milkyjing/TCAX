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

#include "convertion.h"
#include "../tcax/tcaxLib/pix.c"


PIXM_PyImage pixmagick_convert_py_pix(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    TCAX_Pix pix;
    Image *pImage;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "pmgToImage error, too less parameters - `(pix)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "pmgToImage error, the 1st param should be a tuple - `PIX'\n");
        return NULL;
    }
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg1, &pix);
    pImage = ConstituteImage(pix.width, pix.height, "RGBA", CharPixel, pix.buf, NULL);
    free(pix.buf);
    return PyLong_FromUnsignedLong((unsigned long)pImage);  /* real content are still kept in memory, not a real leak, but will be deallocated by pmgToPix function */
}

TCAX_PyPix pixmagick_convert_py_image(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    Image *pImage;
    TCAX_Pix pix;
    PixelPacket *buf;
    int i, index, count;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "pmgToPix error, too less parameters - `(image)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "pmgToPix error, the 1st param should be an integer - `img'\n");
        return NULL;
    }
    pImage = (Image *)PyLong_AsUnsignedLong(pyArg1);
    if (!IsImageObject(pImage)) {
        PyErr_SetString(PyExc_RuntimeError, "pmgToPix error, the parameter specified by `img' is not a valid image\n");
        return NULL;
    }
    pix.initX = 0;
    pix.initY = 0;
    pix.width = pImage->columns;
    pix.height = pImage->rows;
    pix.size = pix.height * (pix.width << 2);
    pix.buf = (unsigned char *)malloc(pix.size * sizeof(unsigned char));
    buf = GetAuthenticPixels(pImage, 0, 0, pImage->columns, pImage->rows, NULL);
    count = pImage->columns * pImage->rows;
    index = 0;
    for (i = 0; i < count; i++) {
        pix.buf[index++] = buf[i].red;
        pix.buf[index++] = buf[i].green;
        pix.buf[index++] = buf[i].blue;
        pix.buf[index++] = 255 - buf[i].opacity;
    }
    return tcaxlib_convert_pix(&pix, 1);
}

