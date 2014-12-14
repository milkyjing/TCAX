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

#ifndef PIXMAGICK_STD_H
#define PIXMAGICK_STD_H
#pragma once

#include <Python.h>
#include <Windows.h>
#include <magick/MagickCore.h>
#pragma comment(lib, "./ImageMagick/CORE_RL_magick_.lib")

#define PIXMAGICK_VERSION 0x00003000    /**< High word of PIXMAGICK_VERSION indicates major pixMagick version, and low word indicates minor version */
#define PIXMAGICK_VER_STR L"0.3"


/**
 * PIXM_Py_Error_Code structure that is in fact a Python long object.
 */
typedef PyObject *PIXM_Py_Error_Code;

/**
 * PIXM_PyLong structure that is in fact a Python long object.
 */
typedef PyObject *PIXM_PyLong;


#endif    /* PIXMAGICK_STD_H */

