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

#ifndef TCVIDEO_FRAME_H
#define TCVIDEO_FRAME_H
#pragma once

#include "std.h"
#include "ffms.h"


/* Inhibit C++ name-mangling for tcVideo functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * Initialize the FFMpegSource library.
 * @param args ()
 * @return 0;
 */
extern TCAX_Py_Error_Code tcvideo_init(PyObject *self, PyObject *args);

/**
 * Initialize the FFMpegSource library.
 * @param args ()
 * @return 0;
 */
extern TCAX_Py_Error_Code tcvideo_fin(PyObject *self, PyObject *args);

/**
 * Open a video file.
 * @param self reserved
 * @param args (filename)
 * @return videoSource (unsigned long);
 */
extern PyObject *tcvideo_open_file(PyObject *self, PyObject *args);

/**
 * Close a video file, release the videoSource
 * @param self reserved
 * @param args (videoSource)
 * @return 0;
 */
extern TCAX_Py_Error_Code tcvideo_close_file(PyObject *self, PyObject *args);

/**
 * Get frame count, width, height information
 * @param self reserved
 * @param args (videoSource)
 * @return (frameCount, width, height, scaledWidth, scaledHeight);
 */
extern PyObject *tcvideo_get_properties(PyObject *self, PyObject *args);

/**
 * Set the video resolution
 * @param self reserved
 * @param args (videoSource, width, height)
 * @return 0
 */
extern TCAX_Py_Error_Code tcvideo_set_resolution(PyObject *self, PyObject *args);

/**
 * Retrieve the frame according to the frame number.
 * @param self reserved
 * @param args (videoSource, frameNumber)
 * @return PIX;
 */
extern PyObject *tcvideo_get_frame_by_number(PyObject *self, PyObject *args);

/**
 * Retrieve the frame according to the time.
 * @param self reserved
 * @param args (videoSource, frameTime)
 * @return PIX;
 */
extern PyObject *tcvideo_get_frame_by_time(PyObject *self, PyObject *args);

extern PyObject *tcvideo_get_color_of_frame(PyObject *self, PyObject *args);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* TCVIDEO_FRAME_H */
