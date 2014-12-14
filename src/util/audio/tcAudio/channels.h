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

#ifndef TCAUDIO_CHANNELS_H
#define TCAUDIO_CHANNELS_H
#pragma once

#include "std.h"
#include "bass.h"


/* Inhibit C++ name-mangling for tcAudio functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * Initialize the audio device.
 * Remark: the sampling rate is 44100 Hz
 *
 * @param self reserved
 * @param args () or (frequency)
 * @return 0;
 */
extern TCAX_Py_Error_Code tcaudio_init(PyObject *self, PyObject *args);

/**
 * Finalize the audio device.
 * @param self reserved
 * @param args ()
 * @return 0;
 */
extern TCAX_Py_Error_Code tcaudio_fin(PyObject *self, PyObject *args);

/**
 * Open an audio file.
 * @param self reserved
 * @param args (filename)
 * @return channel (DWORD);
 */
extern PyObject *tcaudio_open_file(PyObject *self, PyObject *args);

/**
 * Retrieve the duration in seconds of the opened audio.
 * @param self reserved
 * @param args (channel)
 * @return duration (double);
 */
extern PyObject *tcaudio_get_duration(PyObject *self, PyObject *args);

/**
 * Retrieve the frequency of the opened audio.
 * @param self reserved
 * @param args (channel)
 * @return frequency (float);
 */
extern PyObject *tcaudio_get_frequency(PyObject *self, PyObject *args);

/**
 * Retrieve the FFT representation of the audio data.
 * @param self reserved
 * @param args (channel, length)
 * @return data (float array);
 */
extern PyObject *tcaudio_get_fft(PyObject *self, PyObject *args);

/**
 * Retrieve sample data of the audio.
 * @param self reserved
 * @param args (channel, length)
 * @return data (float array);
 */
extern PyObject *tcaudio_get_data(PyObject *self, PyObject *args);

/**
 * Translates a byte position into time (seconds), based on a channel's format.
 * @param self reserved
 * @param args (channel, bytes)
 * @return seconds (double);
 */
extern PyObject *tcaudio_bytes_to_seconds(PyObject *self, PyObject *args);

/**
 * Translates a time (seconds) position into bytes, based on a channel's format.
 * @param self reserved
 * @param args (channel, seconds)
 * @return bytes (QWORD);
 */
extern PyObject *tcaudio_seconds_to_bytes(PyObject *self, PyObject *args);

/**
 * Retrieves the playback position.
 * @param self reserved
 * @param args (channel)
 * @return pos (QWORD);
 */
extern PyObject *tcaudio_get_position(PyObject *self, PyObject *args);

/**
 * Sets the playback position.
 * @param self reserved
 * @param args (channel, pos)
 * @return 0;
 */
extern PyObject *tcaudio_set_position(PyObject *self, PyObject *args);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* TCAUDIO_CHANNELS_H */
