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


static PyMethodDef tcAudio_Methods[] = {
    { "tcAudioInit"           ,  tcaudio_init                            ,  METH_VARARGS ,  "tcAudioInit() or tcAudioInit(frequency)" },
    { "tcAudioFin"            ,  tcaudio_fin                             ,  METH_VARARGS ,  "tcAudioFin()" },
    { "tcAudioOpen"           ,  tcaudio_open_file                       ,  METH_VARARGS ,  "tcAudioOpen(filename)" },
    { "tcAudioGetDuration"    ,  tcaudio_get_duration                    ,  METH_VARARGS ,  "tcAudioGetDuration(channel)" },
    { "tcAudioGetFreq"        ,  tcaudio_get_frequency                   ,  METH_VARARGS ,  "tcAudioGetFreq(channel)" },
    { "tcAudioGetFFT"         ,  tcaudio_get_fft                         ,  METH_VARARGS ,  "tcAudioGetFFT(channel, length)" },
    { "tcAudioGetData"        ,  tcaudio_get_data                        ,  METH_VARARGS ,  "tcAudioGetData(channel, length)" },
    { "tcAudioBytes2Sec"      ,  tcaudio_bytes_to_seconds                ,  METH_VARARGS ,  "tcAudioBytes2Sec(channel, bytes)" },
    { "tcAudioSec2Bytes"      ,  tcaudio_seconds_to_bytes                ,  METH_VARARGS ,  "tcAudioSec2Bytes(channel, seconds)" },
    { "tcAudioGetPos"         ,  tcaudio_get_position                    ,  METH_VARARGS ,  "tcAudioGetPos(channel)" },
    { "tcAudioSetPos"         ,  tcaudio_set_position                    ,  METH_VARARGS ,  "tcAudioSetPos(channel, pos)" },
    { NULL                    ,  NULL                                    ,  0            ,  NULL }
};

static struct PyModuleDef tcAudio_Module = {
    PyModuleDef_HEAD_INIT, 
    "tcAudio", 
    "Module tcAudio is a component of tcax - developed by milkyjing", 
    -1, 
    tcAudio_Methods
};

PyMODINIT_FUNC PyInit_tcAudio() {
    return PyModule_Create(&tcAudio_Module);
}

