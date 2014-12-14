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


static PyObject *pixstone_get_version(PyObject *self, PyObject *args) {
    return PyUnicode_FromUnicode((const Py_UNICODE *)PIXSTONE_VER_STR, wcslen(PIXSTONE_VER_STR));
}

static PyMethodDef pixStone_Methods[] = {
    { "pixStoneGetVersion"     ,  pixstone_get_version                    ,  METH_VARARGS ,  "pixStoneGetVersion()" }, 
    { "pstBlur"                ,  pixstone_blurs                          ,  METH_VARARGS ,  "pstBlur(PIX, method, radius)" }, 
    { "pstGradient"            ,  pixstone_gradient                       ,  METH_VARARGS ,  "pstGradient(PIX, type, x1, y1, x2, y2, rgb1, rgb2, repeat)" }, 
    { "pstEffect"              ,  pixstone_fx                             ,  METH_VARARGS ,  "pstEffect(PIX, effect, param" }, 
    { "pstDistord"             ,  pixstone_distord                        ,  METH_VARARGS ,  "pstDistord(PIX, filter, param1, param2" }, 
    { "pstLensFlare"           ,  pixstone_lens_flare                     ,  METH_VARARGS ,  "pstLensFlare(PIX, x, y)" }, 
    { "pstSoftGlow"            ,  pixstone_soft_glow                      ,  METH_VARARGS ,  "pstSoftGlow(PIX, radius, brightness, contrast)" }, 
    { "pstBlind"               ,  pixstone_blind                          ,  METH_VARARGS ,  "pstBlind(PIX, direct, width, opacity, rgb)" }, 
    { "pstTileReflection"      ,  pixstone_tile_reflection                ,  METH_VARARGS ,  "pstTileReflection(PIX, angle, squareSize, curvature)" }, 
    { NULL                     ,  NULL                                    ,  0            ,  NULL }
};

static struct PyModuleDef pixStone_Module = {
    PyModuleDef_HEAD_INIT, 
    "pixStone", 
    "Module pixStone is a component of tcax - developed by milkyjing", 
    -1, 
    pixStone_Methods
};

PyMODINIT_FUNC PyInit_pixStone() {
    return PyModule_Create(&pixStone_Module);
}

