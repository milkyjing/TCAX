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

#include "channels.h"

#pragma comment(lib, "bass.lib")


TCAX_Py_Error_Code tcaudio_init(PyObject *self, PyObject *args) {
    DWORD freq;
    if (PyTuple_GET_SIZE(args) == 0)
        freq = 44100;
    else {
        PyObject *pyArg1;
        pyArg1 = PyTuple_GET_ITEM(args, 0);
        if (!PyLong_Check(pyArg1) && !PyFloat_Check(pyArg1)) {
            PyErr_SetString(PyExc_RuntimeError, "tcAudioInit error, the 1st param should be an integer - `frequency'\n");
            return NULL;
        }
        freq = PyLong_AsUnsignedLong(pyArg1);
    }
    if (!BASS_Init(0, freq, 0, NULL, NULL)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioInit error, failed to initialize the BASS!\n");
        return NULL;
    }
    return PyLong_FromLong(0);
}

TCAX_Py_Error_Code tcaudio_fin(PyObject *self, PyObject *args) {
    if (!BASS_Free()) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioFin error, failed to finalize the BASS!\n");
        return NULL;
    }
    return PyLong_FromLong(0);
}

PyObject *tcaudio_open_file(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    char *path;
    DWORD channel;
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyUnicode_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioOpen error, the 1st param should be a unicode string - `path'\n");
        return NULL;
    }
    path = PyBytes_AsString(PyUnicode_AsMBCSString(pyArg1));
    channel = BASS_StreamCreateFile(FALSE, path, 0, 0, BASS_STREAM_DECODE);
    if (!channel) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioOpen error, failed to open the file!\n");
        return NULL;
    }
    return PyLong_FromUnsignedLong(channel);
}

PyObject *tcaudio_get_duration(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    DWORD channel;
    QWORD len;
    double dur;
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioGetDuration error, the 1st param should be an integer - `channel'\n");
        return NULL;
    }
    channel = PyLong_AsUnsignedLong(pyArg1);
    len = BASS_ChannelGetLength(channel, BASS_POS_BYTE); // the length in bytes
    dur = BASS_ChannelBytes2Seconds(channel, len); // the length in seconds
    return PyFloat_FromDouble(dur);
}

PyObject *tcaudio_get_frequency(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    DWORD channel;
    float frequency;
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioGetFreq error, the 1st param should be an integer - `channel'\n");
        return NULL;
    }
    channel = PyLong_AsUnsignedLong(pyArg1);
    if (!BASS_ChannelGetAttribute(channel, BASS_ATTRIB_FREQ, &frequency)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioGetFreq error, failed to get the frequency of the audio file!\n");
        return NULL;
    }
    return PyFloat_FromDouble(frequency);
}

PyObject *tcaudio_get_fft(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    DWORD channel, length;
    int i, count;
    float *buf;
    PyObject *ret;
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioGetFFT error, the 1st param should be an integer - `channel'\n");
        return NULL;
    }
    channel = PyLong_AsUnsignedLong(pyArg1);
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioGetFFT error, the 2nd param should be an integer - `length'\n");
        return NULL;
    }
    length = PyLong_AsUnsignedLong(pyArg2);
    switch (length) {
    case 256:
        length = BASS_DATA_FFT256;
        count = 128;
        break;
    case 512:
        length = BASS_DATA_FFT512;
        count = 256;
        break;
    case 1024:
        length = BASS_DATA_FFT1024;
        count = 512;
        break;
    case 2048:
        length = BASS_DATA_FFT2048;
        count = 1024;
        break;
    case 4096:
        length = BASS_DATA_FFT4096;
        count = 2048;
        break;
    case 8192:
        length = BASS_DATA_FFT8192;
        count = 4096;
        break;
    case 16384:
        length = BASS_DATA_FFT16384;
        count = 8192;
        break;
    default:
        length = BASS_DATA_FFT2048;
        count = 1024;
        break;
    }
    buf = (float *)malloc(count * sizeof(float));
    if (BASS_ChannelGetData(channel, buf, length) == -1) {    // get the FFT data
        PyErr_SetString(PyExc_RuntimeError, "tcAudioGetFFT error, failed to retrieve the data\n");
        return NULL;
    }
    ret = PyTuple_New(count);
    for (i = 0; i < count; i++)
        PyTuple_SetItem(ret, i, PyFloat_FromDouble(buf[i]));
    free(buf);
    return ret;
}

PyObject *tcaudio_get_data(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    DWORD channel, length;
    BASS_CHANNELINFO ci;
    int i, count;
    float *buf;
    PyObject *ret;
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioGetData error, the 1st param should be an integer - `channel'\n");
        return NULL;
    }
    channel = PyLong_AsUnsignedLong(pyArg1);
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioGetData error, the 2nd param should be an integer - `length'\n");
        return NULL;
    }
    length = PyLong_AsUnsignedLong(pyArg2);
    BASS_ChannelGetInfo(channel, &ci);   // get number of channels
    count = length * ci.chans;
    buf = (float *)malloc(count * sizeof(float));
    if (BASS_ChannelGetData(channel, buf, (count * sizeof(float)) | BASS_DATA_FLOAT) == -1) {    // get the sample data
        PyErr_SetString(PyExc_RuntimeError, "tcAudioGetData error, failed to retrieve the data\n");
        return NULL;
    }
    ret = PyTuple_New(count);
    for (i = 0; i < count; i++)
        PyTuple_SetItem(ret, i, PyFloat_FromDouble(buf[i]));
    free(buf);
    return ret;
}

PyObject *tcaudio_bytes_to_seconds(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    DWORD channel;
    QWORD bytes;
    double seconds;
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioBytes2Sec error, the 1st param should be an integer - `channel'\n");
        return NULL;
    }
    channel = PyLong_AsUnsignedLong(pyArg1);
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioBytes2Sec error, the 2nd param should be an integer - `bytes'\n");
        return NULL;
    }
    bytes = PyLong_AsUnsignedLongLong(pyArg2);
    seconds = BASS_ChannelBytes2Seconds(channel, bytes);
    if (seconds < 0) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioBytes2Sec error, failed to convert bytes to seconds!\n");
        return NULL;
    }
    return PyFloat_FromDouble(seconds);
}

PyObject *tcaudio_seconds_to_bytes(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    DWORD channel;
    double seconds;
    QWORD bytes;
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioSec2Bytes error, the 1st param should be an integer - `channel'\n");
        return NULL;
    }
    channel = PyLong_AsUnsignedLong(pyArg1);
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioSec2Bytes error, the 2nd param should be a float - `seconds'\n");
        return NULL;
    }
    seconds = PyFloat_AsDouble(pyArg2);
    bytes = BASS_ChannelSeconds2Bytes(channel, seconds);
    if (-1 == bytes) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioSec2Bytes error, failed to convert seconds to bytes!\n");
        return NULL;
    }
    return PyLong_FromUnsignedLongLong(bytes);
}

PyObject *tcaudio_get_position(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    DWORD channel;
    QWORD pos;
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioGetPos error, the 1st param should be an integer - `channel'\n");
        return NULL;
    }
    channel = PyLong_AsUnsignedLong(pyArg1);
    pos = BASS_ChannelGetPosition(channel, BASS_POS_BYTE);
    if (-1 == pos) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioGetPos error, failed to get the position!\n");
        return NULL;
    }
    return PyLong_FromUnsignedLongLong(pos);
}

PyObject *tcaudio_set_position(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    DWORD channel;
    QWORD pos;
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioSetPos error, the 1st param should be an integer - `channel'\n");
        return NULL;
    }
    channel = PyLong_AsUnsignedLong(pyArg1);
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioSetPos error, the 2nd param should be an integer - `pos'\n");
        return NULL;
    }
    pos = PyLong_AsUnsignedLongLong(pyArg2);
    if (!BASS_ChannelSetPosition(channel, pos, BASS_POS_BYTE)) {
        PyErr_SetString(PyExc_RuntimeError, "tcAudioSetPos error, failed to set the position!\n");
        return NULL;
    }
    return PyLong_FromLong(0);
}

