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

#include "composite.h"


PIXM_PyImage pixmagick_composite_image(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5;
    Image *pImage;
    CompositeOperator compose;
    Image *pCompImage;
    int offset_x, offset_y;
    if (PyTuple_GET_SIZE(args) < 5) {
        PyErr_SetString(PyExc_RuntimeError, "pmgComposite error, too less parameters - `(image, compose, composite_image, x_offset, y_offset)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    pyArg5 = PyTuple_GET_ITEM(args, 4);
    pImage = (Image *)PyLong_AsUnsignedLong(pyArg1);
    compose = (CompositeOperator)PyLong_AsLong(pyArg2);
    pCompImage = (Image *)PyLong_AsUnsignedLong(pyArg3);
    offset_x = PyLong_AsLong(pyArg4);
    offset_y = PyLong_AsLong(pyArg5);
    CompositeImage(pImage, compose, pCompImage, offset_x, offset_y);
    return PyLong_FromUnsignedLong((unsigned long)pImage);  /* real content are still kept in memory, not a real leak, but will be deallocated by pmgToPix function */
}

