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

#include "constitute.h"


PIXM_PyImage pixmagick_read_image(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    char *filename;
    ImageInfo *pInfo;
    ExceptionInfo *exception;
    Image *pImage;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "pmgRead error, too less parameters - `(filename)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyUnicode_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "pmgRead error, the 1st param should be a unicode string - `filename'\n");
        return NULL;
    }
    filename = PyBytes_AsString(PyUnicode_AsMBCSString(pyArg1));
    pInfo = CloneImageInfo(NULL);
    exception = AcquireExceptionInfo();
    strcpy(pInfo->filename, filename);
    pImage = ReadImage(pInfo, exception);
    if (!pImage) {
        DestroyImageInfo(pInfo);
        DestroyExceptionInfo(exception);
        PyErr_SetString(PyExc_RuntimeError, "pmgRead error, failed to open the image\n");
        return NULL;
    }
    DestroyImageInfo(pInfo);
    DestroyExceptionInfo(exception);
    return PyLong_FromUnsignedLong((unsigned long)pImage);  /* real content are still kept in memory, not a real leak, but will be deallocated by pmgToPix function */
}

PIXM_Py_Error_Code pixmagick_destroy_image(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    Image *pImage;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "pmgFree error, too less parameters - `(image)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "pmgFree error, the 1st param should be an integer - `image'\n");
        return NULL;
    }
    pImage = (Image *)PyLong_AsUnsignedLong(pyArg1);
    if (!IsImageObject(pImage)) {
        PyErr_SetString(PyExc_RuntimeError, "pmgFree error, invalid iamge object\n");
        return NULL;
    } else
        DestroyImage(pImage);
    return PyLong_FromLong(0);
}

