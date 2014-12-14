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

#include "frame.h"

#pragma comment(lib, "ffms2.lib")


TCAX_Py_Error_Code tcvideo_init(PyObject *self, PyObject *args) {
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    FFMS_Init(0, 0);
    return PyLong_FromLong(0);
}

TCAX_Py_Error_Code tcvideo_fin(PyObject *self, PyObject *args) {
    CoUninitialize();
    return PyLong_FromLong(0);
}

PyObject *tcvideo_open_file(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    char *sourcefile;
    char errmsg[1024];
    FFMS_ErrorInfo errinfo;
    FFMS_Index *index;
    int trackno;
    FFMS_VideoSource *videosource;
    const FFMS_Frame *propframe;
    int pixfmts[2];
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoOpen error, too less parameters - `(filename)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyUnicode_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoOpen error, the 1st param should be a unicode string - `filename'\n");
        return NULL;
    }
    sourcefile = PyBytes_AsString(PyUnicode_AsMBCSString(pyArg1));/* Index the source file. Note that this example does not index any audio tracks. */
    errinfo.Buffer      = errmsg;
    errinfo.BufferSize  = sizeof(errmsg);
    errinfo.ErrorType   = FFMS_ERROR_SUCCESS;
    errinfo.SubType     = FFMS_ERROR_SUCCESS;
    index = FFMS_MakeIndex(sourcefile, 0, 0, NULL, NULL, FFMS_IEH_ABORT, NULL, NULL, &errinfo);
    if (!index) {
        /* handle error (print errinfo.Buffer somewhere) */
        PyErr_SetString(PyExc_RuntimeError, "tcVideoOpen error, cannot open the file.\n");
        return NULL;
    }
    /* Retrieve the track number of the first video track */
    trackno = FFMS_GetFirstTrackOfType(index, FFMS_TYPE_VIDEO, &errinfo);
    if (trackno < 0) {
        /* no video tracks found in the file, this is bad and you should handle it */
        /* (print the errmsg somewhere) */
        PyErr_SetString(PyExc_RuntimeError, "tcVideoOpen error, cannot open the file.\n");
        return NULL;
    }
    /* We now have enough information to create the video source object */
    videosource = FFMS_CreateVideoSource(sourcefile, trackno, index, 1, FFMS_SEEK_NORMAL, &errinfo);
    if (!videosource) {
        /* handle error (you should know what to do by now) */
        PyErr_SetString(PyExc_RuntimeError, "tcVideoOpen error, cannot open the file.\n");
        return NULL;
    }
    /* Since the index is copied into the video source object upon its creation,
    we can and should now destroy the index object. */
    FFMS_DestroyIndex(index);
    propframe = FFMS_GetFrame(videosource, 0, &errinfo);
	pixfmts[0] = FFMS_GetPixFmt("rgba");
	pixfmts[1] = -1;
    FFMS_SetOutputFormatV2(videosource, pixfmts, propframe->EncodedWidth, propframe->EncodedHeight, FFMS_RESIZER_BICUBIC, &errinfo);
    return PyLong_FromUnsignedLong((unsigned long)videosource);
}

TCAX_Py_Error_Code tcvideo_close_file(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    FFMS_VideoSource *videosource;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoClose error, too less parameters - `(videoSource)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoClose error, the 1st param should be an integer - `videoSource'\n");
        return NULL;
    }
    videosource = (FFMS_VideoSource *)PyLong_AsUnsignedLong(pyArg1);
    FFMS_DestroyVideoSource(videosource);
    return PyLong_FromLong(0);
}

PyObject *tcvideo_get_properties(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    FFMS_VideoSource *videosource;
    const FFMS_VideoProperties *videoprops;
    char errmsg[1024];
    FFMS_ErrorInfo errinfo;
    const FFMS_Frame *propframe;
    PyObject *props;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoGetProps error, too less parameters - `(videoSource)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoGetProps error, the 1st param should be an integer - `videoSource'\n");
        return NULL;
    }
    videosource = (FFMS_VideoSource *)PyLong_AsUnsignedLong(pyArg1);
    videoprops = FFMS_GetVideoProperties(videosource);
    errinfo.Buffer      = errmsg;
    errinfo.BufferSize  = sizeof(errmsg);
    errinfo.ErrorType   = FFMS_ERROR_SUCCESS;
    errinfo.SubType     = FFMS_ERROR_SUCCESS;
    propframe = FFMS_GetFrame(videosource, 0, &errinfo);
    props = PyTuple_New(5);
    PyTuple_SET_ITEM(props, 0, PyLong_FromLong(videoprops->NumFrames));
    PyTuple_SET_ITEM(props, 1, PyLong_FromLong(propframe->EncodedWidth));
    PyTuple_SET_ITEM(props, 2, PyLong_FromLong(propframe->EncodedHeight));
    PyTuple_SET_ITEM(props, 3, PyLong_FromLong(propframe->ScaledWidth));
    PyTuple_SET_ITEM(props, 4, PyLong_FromLong(propframe->ScaledHeight));
    return props;
}

TCAX_Py_Error_Code tcvideo_set_resolution(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3;
    FFMS_VideoSource *videosource;
    int width, height;
    int pixfmts[2];
    char errmsg[1024];
    FFMS_ErrorInfo errinfo;
    if (PyTuple_GET_SIZE(args) < 3) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoSetRes error, too less parameters - `(videoSource, width, height)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoSetRes error, the 1st param should be an integer - `videoSource'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyFloat_Check(pyArg2) && !PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoSetRes error, the 2nd param should be an integer - `width'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyFloat_Check(pyArg3) && !PyLong_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoSetRes error, the 3rd param should be an integer - `height'\n");
        return NULL;
    }
    videosource = (FFMS_VideoSource *)PyLong_AsUnsignedLong(pyArg1);
    width = (int)PyLong_AsLong(pyArg2);
    height = (int)PyLong_AsLong(pyArg3);
    errinfo.Buffer      = errmsg;
    errinfo.BufferSize  = sizeof(errmsg);
    errinfo.ErrorType   = FFMS_ERROR_SUCCESS;
    errinfo.SubType     = FFMS_ERROR_SUCCESS;
	pixfmts[0] = FFMS_GetPixFmt("rgba");
	pixfmts[1] = -1;
    FFMS_SetOutputFormatV2(videosource, pixfmts, width, height, FFMS_RESIZER_BICUBIC, &errinfo);
    return PyLong_FromLong(0);
}

PyObject *tcvideo_get_frame_by_number(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    FFMS_VideoSource *videosource;
    int frameNo;
    char errmsg[1024];
    FFMS_ErrorInfo errinfo;
    const FFMS_Frame *frame;
    int i, w, h, width, height, size;
    PyObject *pix;
    PyObject *pix_pos;
    PyObject *pix_res;
    PyObject *pix_buf;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoGetFrame error, too less parameters - `(videoSource, frameNumber)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoGetFrame error, the 1st param should be an integer - `videoSource'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoGetFrame error, the 2nd param should be an integer - `frameNumber'\n");
        return NULL;
    }
    videosource = (FFMS_VideoSource *)PyLong_AsUnsignedLong(pyArg1);
    frameNo = (int)PyLong_AsLong(pyArg2);
    errinfo.Buffer      = errmsg;
    errinfo.BufferSize  = sizeof(errmsg);
    errinfo.ErrorType   = FFMS_ERROR_SUCCESS;
    errinfo.SubType     = FFMS_ERROR_SUCCESS;
    frame = FFMS_GetFrame(videosource, frameNo, &errinfo);
    width = frame->ScaledWidth;
    height = frame->ScaledHeight;
    size = height * (width << 2);
    pix_buf = PyTuple_New(size);
    for (h = 0; h < height; h++) {
        for (w = 0; w < width; w++) {
            i = (h * width + w) << 2;
            PyTuple_SET_ITEM(pix_buf, i, PyLong_FromLong(frame->Data[0][i++]));
            PyTuple_SET_ITEM(pix_buf, i, PyLong_FromLong(frame->Data[0][i++]));
            PyTuple_SET_ITEM(pix_buf, i, PyLong_FromLong(frame->Data[0][i++]));
            PyTuple_SET_ITEM(pix_buf, i, PyLong_FromLong(frame->Data[0][i++]));
        }
    }
    pix_pos = PyTuple_New(2);
    PyTuple_SET_ITEM(pix_pos, 0, PyLong_FromLong(0));
    PyTuple_SET_ITEM(pix_pos, 1, PyLong_FromLong(0));
    pix_res = PyTuple_New(2);
    PyTuple_SET_ITEM(pix_res, 0, PyLong_FromLong(width));
    PyTuple_SET_ITEM(pix_res, 1, PyLong_FromLong(height));
    pix = PyTuple_New(3);
    PyTuple_SET_ITEM(pix, 0, pix_pos);
    PyTuple_SET_ITEM(pix, 1, pix_res);
    PyTuple_SET_ITEM(pix, 2, pix_buf);
    return pix;
}

PyObject *tcvideo_get_frame_by_time(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    FFMS_VideoSource *videosource;
    double frameTime;
    char errmsg[1024];
    FFMS_ErrorInfo errinfo;
    const FFMS_Frame *frame;
    int i, w, h, width, height, size;
    PyObject *pix;
    PyObject *pix_pos;
    PyObject *pix_res;
    PyObject *pix_buf;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoGetFrameByTime error, too less parameters - `(videoSource, frameTime)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoGetFrameByTime error, the 1st param should be an integer - `videoSource'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyFloat_Check(pyArg2) && !PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoGetFrameByTime error, the 2nd param should be a float - `frameTime'\n");
        return NULL;
    }
    videosource = (FFMS_VideoSource *)PyLong_AsUnsignedLong(pyArg1);
    frameTime = PyFloat_AsDouble(pyArg2);
    errinfo.Buffer      = errmsg;
    errinfo.BufferSize  = sizeof(errmsg);
    errinfo.ErrorType   = FFMS_ERROR_SUCCESS;
    errinfo.SubType     = FFMS_ERROR_SUCCESS;
    frame = FFMS_GetFrameByTime(videosource, frameTime, &errinfo);
    width = frame->ScaledWidth;
    height = frame->ScaledHeight;
    size = height * (width << 2);
    pix_buf = PyTuple_New(size);
    for (h = 0; h < height; h++) {
        for (w = 0; w < width; w++) {
            i = (h * width + w) << 2;
            PyTuple_SET_ITEM(pix_buf, i, PyLong_FromLong(frame->Data[0][i++]));
            PyTuple_SET_ITEM(pix_buf, i, PyLong_FromLong(frame->Data[0][i++]));
            PyTuple_SET_ITEM(pix_buf, i, PyLong_FromLong(frame->Data[0][i++]));
            PyTuple_SET_ITEM(pix_buf, i, PyLong_FromLong(frame->Data[0][i++]));
        }
    }
    pix_pos = PyTuple_New(2);
    PyTuple_SET_ITEM(pix_pos, 0, PyLong_FromLong(0));
    PyTuple_SET_ITEM(pix_pos, 1, PyLong_FromLong(0));
    pix_res = PyTuple_New(2);
    PyTuple_SET_ITEM(pix_res, 0, PyLong_FromLong(width));
    PyTuple_SET_ITEM(pix_res, 1, PyLong_FromLong(height));
    pix = PyTuple_New(3);
    PyTuple_SET_ITEM(pix, 0, pix_pos);
    PyTuple_SET_ITEM(pix, 1, pix_res);
    PyTuple_SET_ITEM(pix, 2, pix_buf);
    return pix;
}

PyObject *tcvideo_get_color_of_frame(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    FFMS_VideoSource *videosource;
    int frameNo;
    char errmsg[1024];
    FFMS_ErrorInfo errinfo;
    const FFMS_Frame *frame;
    int i, w, h, width, height;
    int r, g, b, a, rr, gg, bb, aa;
    PyObject *color;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoGetFrameColor error, too less parameters - `(videoSource, frameNumber)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoGetFrameColor error, the 1st param should be an integer - `videoSource'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "tcVideoGetFrameColor error, the 2nd param should be an integer - `frameNumber'\n");
        return NULL;
    }
    videosource = (FFMS_VideoSource *)PyLong_AsUnsignedLong(pyArg1);
    frameNo = (int)PyLong_AsLong(pyArg2);
    errinfo.Buffer      = errmsg;
    errinfo.BufferSize  = sizeof(errmsg);
    errinfo.ErrorType   = FFMS_ERROR_SUCCESS;
    errinfo.SubType     = FFMS_ERROR_SUCCESS;
    frame = FFMS_GetFrame(videosource, frameNo, &errinfo);
    width = frame->ScaledWidth;
    height = frame->ScaledHeight;
    r = g = b = a = 0;
    rr = gg = bb = aa = 0;
    for (h = 0; h < height; h++) {
        for (w = 0; w < width; w++) {
            i = (h * width + w) << 2;
            rr += frame->Data[0][i++];
            gg += frame->Data[0][i++];
            bb += frame->Data[0][i++];
            aa += frame->Data[0][i++];
        }
        r += rr / width;
        g += gg / width;
        b += bb / width;
        a += aa / width;
    }
    r /= height;
    g /= height;
    b /= height;
    a /= height;
    color = PyTuple_New(4);
    PyTuple_SET_ITEM(color, 0, PyLong_FromLong((unsigned char)r));
    PyTuple_SET_ITEM(color, 1, PyLong_FromLong((unsigned char)g));
    PyTuple_SET_ITEM(color, 2, PyLong_FromLong((unsigned char)b));
    PyTuple_SET_ITEM(color, 3, PyLong_FromLong((unsigned char)a));
    return color;
}

