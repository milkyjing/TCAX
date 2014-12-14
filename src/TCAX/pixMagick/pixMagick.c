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

#include "magick.h"
#include "convertion.h"
#include "constitute.h"
#include "composite.h"
#include "test.h"


static PyObject *pixmagick_get_version(PyObject *self, PyObject *args) {
    return PyUnicode_FromUnicode((const Py_UNICODE *)PIXMAGICK_VER_STR, wcslen(PIXMAGICK_VER_STR));
}

static PyMethodDef pixMagick_Methods[] = {
    { "pixMagickGetVersion"         ,  pixmagick_get_version                   ,  METH_VARARGS ,  "pixMagickGetVersion()" }, 
    /* in magick.h */
    { "pixMagickGetPrecision"       ,  pixmagick_get_precision                 ,  METH_VARARGS ,  "pixMagickGetPrecision()" }, 
    { "pixMagickIsInstantiated"     ,  pixmagick_is_instantiated               ,  METH_VARARGS ,  "pixMagickIsInstantiated()" }, 
    { "pixMagickGenesis"            ,  pixmagick_genesis                       ,  METH_VARARGS ,  "pixMagickGenesis() or pixMagickGenesis(path, establish_signal_handlers)" }, 
    { "pixMagickTerminus"           ,  pixmagick_terminus                      ,  METH_VARARGS ,  "pixMagickTerminus()" }, 
    { "pixMagickSetPrecision"       ,  pixmagick_set_precision                 ,  METH_VARARGS ,  "pixMagickSetPrecision(precision)" }, 
    /* in convertion.h */
    { "pmgToImage"                  ,  pixmagick_convert_py_pix                ,  METH_VARARGS ,  "pmgToImage(PIX)" }, 
    { "pmgToPix"                    ,  pixmagick_convert_py_image              ,  METH_VARARGS ,  "pmgToPix(image)" }, 
    /* in constitute.h */
    { "pmgRead"                     ,  pixmagick_read_image                    ,  METH_VARARGS ,  "pmgRead(filename)" }, 
    { "pmgFree"                     ,  pixmagick_destroy_image                 ,  METH_VARARGS ,  "pmgFree(image)" }, 
    /* in composite.h */
    { "pmgComposite"                ,  pixmagick_composite_image               ,  METH_VARARGS ,  "pmgComposite(image, compose, composite_image, x_offset, y_offset)" }, 
    /* in test.h */
    { "pmgTest"                     ,  pixmagick_test                          ,  METH_VARARGS ,  "pmgTest(filename)" }, 
    { NULL                          ,  NULL                                    ,  0            ,  NULL }
};

static struct PyModuleDef pixMagick_Module = {
    PyModuleDef_HEAD_INIT, 
    "pixMagick", 
    "Module pixMagick is a component of tcax - developed by milkyjing", 
    -1, 
    pixMagick_Methods
};

PyMODINIT_FUNC PyInit_pixMagick() {
    return PyModule_Create(&pixMagick_Module);
}

