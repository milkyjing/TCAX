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

#include "tcasfunc.h"
#include "../../../../../../libtcas/trunk/src/alpha/tcas/vector.c"
#include "../../../../../../libtcas/trunk/src/alpha/tcas/rb.c"
#include "../../../../../../libtcas/trunk/src/alpha/tcas/tcas.c"
#include "../../../../../../libtcas/trunk/src/alpha/tcas/hla_z_comp.c"
#include "../../../../../../libtcas/trunk/src/alpha/tcas/hla_stream_parser.c"


TCAX_Py_Error_Code tcaxlib_tcas_list_append_pix(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5, *pyArg6, *pyArg7;
    int start, end;
    double diffX, diffY;
    unsigned long layer_type_pair;
    PyObject *tcasList;
    TCAX_Pix pix;
    int w, h, index;
    unsigned long color;
    int alpha;
    int posX, posY, initPosX, initPosY;
    PyObject *pixDip;   /* store a dynamic isolated pixel of PIX temporary */
    if (PyTuple_GET_SIZE(args) < 7) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_main error, too less parameters - `(TCAS_BUF, PIX, Start, End, DiffX, DiffY, Layer)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyList_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_main error, the 1st param should be a list - `TCAS_BUF'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyTuple_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_main error, the 2nd param should be a tuple - `PIX'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_main error, the 3rd param should be an integer - `start'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_main error, the 4th param should be an integer - `end'\n");
        return NULL;
    }
    pyArg5 = PyTuple_GET_ITEM(args, 4);
    if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_main error, the 5th param should be a float - `initX'\n");
        return NULL;
    }
    pyArg6 = PyTuple_GET_ITEM(args, 5);
    if (!PyLong_Check(pyArg6) && !PyFloat_Check(pyArg6)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_main error, the 6th param should be a float - `initY'\n");
        return NULL;
    }
    pyArg7 = PyTuple_GET_ITEM(args, 6);
    if (!PyLong_Check(pyArg7) && !PyFloat_Check(pyArg7)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_main error, the 7th param should be an integer - `layer'\n");
        return NULL;
    }
    start = (int)PyLong_AsLong(pyArg3);
    end = (int)PyLong_AsLong(pyArg4);
    if (start > end) return Py_BuildValue("i", -1);
    tcasList = pyArg1;    /* note: we needn't use Py_CLEAR(tcasList) or Py_INCREF(tcasList) */
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg2, &pix);
    diffX = PyFloat_AsDouble(pyArg5);
    diffY = PyFloat_AsDouble(pyArg6);
    //layer = (0x0000000F && (unsigned long)PyLong_AsLong(pyArg7));
    layer_type_pair = (unsigned long)PyLong_AsUnsignedLong(pyArg7);
    initPosX = (int)floor(diffX + pix.initX);    /* Note: no we don't use (int)(fDiffX + fInitX + 0.5) */
    initPosY = (int)floor(diffY + pix.initY);
    for (h = 0; h < pix.height; h++) {
        posY = initPosY + h;
        for (w = 0; w < pix.width; w++) {
            posX = initPosX + w;
            index = (h * pix.width + w) << 2;
            color = MAKERGB(pix.buf[index], pix.buf[index + 1], pix.buf[index + 2]);
            alpha = pix.buf[index + 3];
            if (0 != alpha) {
                pixDip = PyTuple_New(7);
                PyTuple_SET_ITEM(pixDip, 0, PyLong_FromLong(start));
                PyTuple_SET_ITEM(pixDip, 1, PyLong_FromLong(end));
                PyTuple_SET_ITEM(pixDip, 2, PyLong_FromUnsignedLong(layer_type_pair));
                PyTuple_SET_ITEM(pixDip, 3, PyLong_FromLong(posX));
                PyTuple_SET_ITEM(pixDip, 4, PyLong_FromLong(posY));
                PyTuple_SET_ITEM(pixDip, 5, PyLong_FromLong(color));
                PyTuple_SET_ITEM(pixDip, 6, PyLong_FromLong(alpha));
                PyList_Append(tcasList, pixDip);
                Py_CLEAR(pixDip);
            }
        }
    }
    free(pix.buf);
    return PyLong_FromLong(0);
}

TCAX_Py_Error_Code tcaxlib_tcas_list_append_key_pixs(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5, *pyArg6, *pyArg7, *pyArg8, *pyArg9;
    int start, end;
    double diffX, diffY;
    unsigned long layer_type_pair;
    PyObject *tcasList;
    TCAX_Pix pix_start;
    TCAX_Pix pix_end;
    int w, h, index;
    unsigned long color;
    int alpha;
    int posX, posY, initPosX, initPosY;
    PyObject *pixDip;   /* store a dynamic isolated pixel of PIX temporary */
    if (PyTuple_GET_SIZE(args) < 9) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_keyframe error, too less parameters - `(TCAS_BUF, pix_start, pix_end, start, end, offsetX, offsetY, type, layer)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyList_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_keyframe error, the 1st param should be a list - `TCAS_BUF'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyTuple_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_keyframe error, the 2nd param should be a tuple - `PIX_start'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyTuple_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_keyframe error, the 3rd param should be a tuple - `PIX_end'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_keyframe error, the 4th param should be an integer - `start'\n");
        return NULL;
    }
    pyArg5 = PyTuple_GET_ITEM(args, 4);
    if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_keyframe error, the 5th param should be an integer - `end'\n");
        return NULL;
    }
    pyArg6 = PyTuple_GET_ITEM(args, 5);
    if (!PyLong_Check(pyArg6) && !PyFloat_Check(pyArg6)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_keyframe error, the 6th param should be a float - `initX'\n");
        return NULL;
    }
    pyArg7 = PyTuple_GET_ITEM(args, 6);
    if (!PyLong_Check(pyArg7) && !PyFloat_Check(pyArg7)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_keyframe error, the 7th param should be a float - `initY'\n");
        return NULL;
    }
    pyArg8 = PyTuple_GET_ITEM(args, 7);
    if (!PyLong_Check(pyArg8) && !PyFloat_Check(pyArg8)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_keyframe error, the 8th param should be an integer - `type'\n");
        return NULL;
    }
    pyArg9 = PyTuple_GET_ITEM(args, 8);
    if (!PyLong_Check(pyArg9) && !PyFloat_Check(pyArg9)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_keyframe error, the 9th param should be an integer - `layer'\n");
        return NULL;
    }
    start = (int)PyLong_AsLong(pyArg4);
    end = (int)PyLong_AsLong(pyArg5);
    if (start > end) return Py_BuildValue("i", -1);
    tcasList = pyArg1;    /* note: we needn't use Py_CLEAR(tcasList) or Py_INCREF(tcasList) */
    // pix start
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg2, &pix_start);
    diffX = PyFloat_AsDouble(pyArg6);
    diffY = PyFloat_AsDouble(pyArg7);
    layer_type_pair = _MAKELTP(PyLong_AsLong(pyArg9), MAKERAWTP(PyLong_AsLong(pyArg8), 0));
    initPosX = (int)floor(diffX + pix_start.initX);    /* Note: no we don't use (int)(fDiffX + fInitX + 0.5) */
    initPosY = (int)floor(diffY + pix_start.initY);
    for (h = 0; h < pix_start.height; h++) {
        posY = initPosY + h;
        for (w = 0; w < pix_start.width; w++) {
            posX = initPosX + w;
            index = (h * pix_start.width + w) << 2;
            color = MAKERGB(pix_start.buf[index], pix_start.buf[index + 1], pix_start.buf[index + 2]);
            alpha = pix_start.buf[index + 3];
            if (0 != alpha) {
                pixDip = PyTuple_New(7);
                PyTuple_SET_ITEM(pixDip, 0, PyLong_FromLong(start));
                PyTuple_SET_ITEM(pixDip, 1, PyLong_FromLong(start));
                PyTuple_SET_ITEM(pixDip, 2, PyLong_FromUnsignedLong(layer_type_pair));
                PyTuple_SET_ITEM(pixDip, 3, PyLong_FromLong(posX));
                PyTuple_SET_ITEM(pixDip, 4, PyLong_FromLong(posY));
                PyTuple_SET_ITEM(pixDip, 5, PyLong_FromLong(color));
                PyTuple_SET_ITEM(pixDip, 6, PyLong_FromLong(alpha));
                PyList_Append(tcasList, pixDip);
                Py_CLEAR(pixDip);
            }
        }
    }
    free(pix_start.buf);
    // pix end
    tcaxlib_convert_py_pix((const TCAX_PyPix)pyArg3, &pix_end);
    layer_type_pair = _MAKELTP(PyLong_AsLong(pyArg9), MAKERAWTP(PyLong_AsLong(pyArg8), 1));
    initPosX = (int)floor(diffX + pix_end.initX);    /* Note: no we don't use (int)(fDiffX + fInitX + 0.5) */
    initPosY = (int)floor(diffY + pix_end.initY);
    for (h = 0; h < pix_end.height; h++) {
        posY = initPosY + h;
        for (w = 0; w < pix_end.width; w++) {
            posX = initPosX + w;
            index = (h * pix_end.width + w) << 2;
            color = MAKERGB(pix_end.buf[index], pix_end.buf[index + 1], pix_end.buf[index + 2]);
            alpha = pix_end.buf[index + 3];
            if (0 != alpha) {
                pixDip = PyTuple_New(7);
                PyTuple_SET_ITEM(pixDip, 0, PyLong_FromLong(end));
                PyTuple_SET_ITEM(pixDip, 1, PyLong_FromLong(end));
                PyTuple_SET_ITEM(pixDip, 2, PyLong_FromUnsignedLong(layer_type_pair));
                PyTuple_SET_ITEM(pixDip, 3, PyLong_FromLong(posX));
                PyTuple_SET_ITEM(pixDip, 4, PyLong_FromLong(posY));
                PyTuple_SET_ITEM(pixDip, 5, PyLong_FromLong(color));
                PyTuple_SET_ITEM(pixDip, 6, PyLong_FromLong(alpha));
                PyList_Append(tcasList, pixDip);
                Py_CLEAR(pixDip);
            }
        }
    }
    free(pix_end.buf);
    return PyLong_FromLong(0);
}

void tcaxlib_convert_tcas_list_to_buf(PyObject *tcasList, tcas_unit **pTcasBuf, tcas_u32 *pCount) {
    int i, count;
    PyObject *pyTcasItem;
    tcas_s16 x, y;
    tcas_u32 rgb;
    tcas_byte alpha;
    tcas_unit *tcasBuf;
    count = PyList_GET_SIZE(tcasList);
    /* tcas buffer from user tcax py script: (Start, End, Layer_Type_Pair, PosX, PosY, RGB, Alpha) */
    /* a raw tcas chunk defined in TCC file format specification: (Start, End, Layer_Type_Pair, Pos, RGBA) */
    tcasBuf = (tcas_unit *)malloc(count * 5 * sizeof(tcas_unit));
    for (i = 0; i < count; i++) {
        pyTcasItem = PyList_GET_ITEM(tcasList, i);
        x = (tcas_s16)PyLong_AsLong(PyTuple_GET_ITEM(pyTcasItem, 3));
        y = (tcas_s16)PyLong_AsLong(PyTuple_GET_ITEM(pyTcasItem, 4));
        rgb = (tcas_u32)PyLong_AsUnsignedLong(PyTuple_GET_ITEM(pyTcasItem, 5));
        alpha = (tcas_byte)PyLong_AsLong(PyTuple_GET_ITEM(pyTcasItem, 6));
        tcasBuf[5 * i + 0] = (tcas_s32)PyLong_AsLong(PyTuple_GET_ITEM(pyTcasItem, 0));    /* Start */
        tcasBuf[5 * i + 1] = (tcas_s32)PyLong_AsLong(PyTuple_GET_ITEM(pyTcasItem, 1));    /* End */
        tcasBuf[5 * i + 2] = (tcas_unit)PyLong_AsUnsignedLong(PyTuple_GET_ITEM(pyTcasItem, 2));    /* Layer_Type_Pair */
        tcasBuf[5 * i + 3] = MAKEPOS(x, y);
        tcasBuf[5 * i + 4] = (rgb | ((tcas_u32)alpha << 24));
    }
    *pTcasBuf = tcasBuf;
    *pCount = count;
}

static PyObject *_tcaxlib_convert_chunks_vector_to_tcas_list(VectorPtr chunksVector, int layer) {
    PyObject *tcasList;
    PyObject *pixDip;
    TCAS_Chunk chunk;
    int i, j, count, num;
    unsigned long pos, rgba;
    tcasList = PyList_New(0);
    count = vector_get_count(chunksVector);
    for (i = 0; i < count; i++) {
        vector_retrieve(chunksVector, i, &chunk);
        num = GETCOUNT(chunk.cltp);
        for (j = 0; j < num; j++) {
            pixDip = PyTuple_New(7);
            PyTuple_SET_ITEM(pixDip, 0, PyLong_FromLong(chunk.startTime));
            PyTuple_SET_ITEM(pixDip, 1, PyLong_FromLong(chunk.endTime));
            PyTuple_SET_ITEM(pixDip, 2, PyLong_FromUnsignedLong(layer));
            pos = chunk.pos_and_color[j << 1];
            rgba = chunk.pos_and_color[(j << 1) + 1];
            PyTuple_SET_ITEM(pixDip, 3, PyLong_FromLong(GETPOSX(pos)));
            PyTuple_SET_ITEM(pixDip, 4, PyLong_FromLong(GETPOSY(pos)));
            PyTuple_SET_ITEM(pixDip, 5, PyLong_FromLong(0x00FFFFFF & rgba));
            PyTuple_SET_ITEM(pixDip, 6, PyLong_FromLong(GETA(rgba)));
            PyList_Append(tcasList, pixDip);
            Py_CLEAR(pixDip);
        }
        free(chunk.pos_and_color);
    }
    return tcasList;
}

static void _tcaxlib_get_fps_n_d(double fps, tcas_u32 *pFpsNumerator, tcas_u32 *pFpsDenominator) {
    tcas_u32 fpsNumerator, fpsDenominator, x, y, t;
    fpsDenominator = 100000;
    fpsNumerator = (tcas_u32)(fps * fpsDenominator);
    x = fpsNumerator;
    y = fpsDenominator;
    while (y) {   // find gcd
        t = x % y;
        x = y;
        y = t;
    }
    fpsNumerator /= x;
    fpsDenominator /= x;
    *pFpsNumerator = fpsNumerator;
    *pFpsDenominator = fpsDenominator;
}

PyObject *tcaxlib_tcas_list_parse(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5;
    tcas_u16 width, height;
    double fps;
    int layer;
    tcas_u32 fpsNumerator, fpsDenominator;
    tcas_unit *tcasBuf;
    tcas_unit *compTcasBuf;
    tcas_u32 count, chunks, units;
    TCAS_StreamParser parser;
    VectorPtr chunksVector;
    PyObject *retTcasBuf;
    if (PyTuple_GET_SIZE(args) < 5) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_parse error, too less parameters - `(TCAS_BUF, width, height, fps, layer)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyList_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_parse error, the 1st param should be a list - `TCAS_BUF'\n");
        return NULL;
    }
    if (PyList_GET_SIZE(pyArg1) == 0) {
        PyErr_SetString(PyExc_RuntimeWarning, "tcas_parse warning, empty list `TCAS_BUF'\n");
        return PyLong_FromLong(-1);
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_parse error, the 2nd param should be an integer - `width'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_parse error, the 3rd param should be an integer - `height'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_parse error, the 4th param should be a float - `fps'\n");
        return NULL;
    }
    pyArg5 = PyTuple_GET_ITEM(args, 4);
    if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
        PyErr_SetString(PyExc_RuntimeError, "tcas_parse error, the 5th param should be an integer - `layer'\n");
        return NULL;
    }
    tcaxlib_convert_tcas_list_to_buf(pyArg1, &tcasBuf, &count);
    libtcas_compress_raw_chunks((const TCAS_pRawChunk)tcasBuf, count, TCAS_FALSE, &compTcasBuf, &chunks, &units);
    free(tcasBuf);
    width = (tcas_u16)PyLong_AsLong(pyArg2);
    height = (tcas_u16)PyLong_AsLong(pyArg3);
    fps = PyFloat_AsDouble(pyArg4);
    layer = (int)PyLong_AsLong(pyArg5);
    _tcaxlib_get_fps_n_d(fps, &fpsNumerator, &fpsDenominator);
    vector_create(&chunksVector, sizeof(TCAS_Chunk), 0, NULL, NULL);
    libtcas_stream_parser_init(compTcasBuf, chunks, fpsNumerator, fpsDenominator, width, height, &parser);
    libtcas_stream_parser_parse(&parser, chunksVector);
    libtcas_stream_parser_fin(&parser);
    free(compTcasBuf);
    retTcasBuf = _tcaxlib_convert_chunks_vector_to_tcas_list(chunksVector, layer);
    vector_destroy(chunksVector);
    return retTcasBuf;
}
