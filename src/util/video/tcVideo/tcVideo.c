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


static PyMethodDef tcVideo_Methods[] = {
    { "tcVideoInit"            ,  tcvideo_init                            ,  METH_VARARGS ,  "tcVideoInit()" },
    { "tcVideoFin"             ,  tcvideo_fin                             ,  METH_VARARGS ,  "tcVideoFin()" },
    { "tcVideoOpen"            ,  tcvideo_open_file                       ,  METH_VARARGS ,  "tcVideoOpen(filename)" },
    { "tcVideoClose"           ,  tcvideo_close_file                      ,  METH_VARARGS ,  "tcVideoClose(videoSource)" },
    { "tcVideoGetProps"        ,  tcvideo_get_properties                  ,  METH_VARARGS ,  "tcVideoGetProps(videoSource)" },
    { "tcVideoSetRes"          ,  tcvideo_set_resolution                  ,  METH_VARARGS ,  "tcVideoSetRes(videoSource, width, height)" },
    { "tcVideoGetFrame"        ,  tcvideo_get_frame_by_number             ,  METH_VARARGS ,  "tcVideoGetFrame(videoSource, frameNumber)" },
    { "tcVideoGetFrameByTime"  ,  tcvideo_get_frame_by_time               ,  METH_VARARGS ,  "tcVideoGetFrameByTime(videoSource, frameTime)" },
    { "tcVideoGetFrameColor"   ,  tcvideo_get_color_of_frame              ,  METH_VARARGS ,  "tcVideoGetFrameColor(videoSource, frameNumber)" },
    { NULL                     ,  NULL                                    ,  0            ,  NULL }
};

static struct PyModuleDef tcVideo_Module = {
    PyModuleDef_HEAD_INIT, 
    "tcVideo", 
    "Module tcVideo is a component of tcax - developed by milkyjing", 
    -1, 
    tcVideo_Methods
};

PyMODINIT_FUNC PyInit_tcVideo() {
    return PyModule_Create(&tcVideo_Module);
}

