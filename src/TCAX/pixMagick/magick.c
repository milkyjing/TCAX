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


PIXM_PyLong pixmagick_get_precision(PyObject *self, PyObject *args) {
    return PyLong_FromLong(GetMagickPrecision());
}

PIXM_PyLong pixmagick_is_instantiated(PyObject *self, PyObject *args) {
    return PyLong_FromLong(IsMagickInstantiated());
}

PIXM_Py_Error_Code pixmagick_genesis(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    char *path;
    int establish_signal_handlers;
    if (0 == PyTuple_GET_SIZE(args)) {
        MagickCoreGenesis(NULL, MagickFalse);
        return PyLong_FromLong(0);
    } else if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "pixMagickGenesis error, too less parameters - `(path, establish_signal_handlers)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyUnicode_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "pixMagickGenesis error, the 1st param should be a unicode string - `path'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "pixMagickGenesis error, the 2nd param should be an integer - `establish_signal_handlers'\n");
        return NULL;
    }
    path = PyBytes_AsString(PyUnicode_AsMBCSString(pyArg1));
    establish_signal_handlers = (int)PyLong_AsLong(pyArg2);
    MagickCoreGenesis(path, (MagickBooleanType)establish_signal_handlers);
    return PyLong_FromLong(0);
}

PIXM_Py_Error_Code pixmagick_terminus(PyObject *self, PyObject *args) {
    MagickCoreTerminus();
    return PyLong_FromLong(0);
}

PIXM_PyLong pixmagick_set_precision(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    int precision;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "pixMagickSetPrecision error, too less parameters - `(precision)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "pixMagickSetPrecision error, the 1st param should be an integer - `precision'\n");
        return NULL;
    }
    precision = (int)PyLong_AsLong(pyArg1);
    return PyLong_FromLong(SetMagickPrecision(precision));
}

