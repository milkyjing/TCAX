/*
 * Map file implementation for both Windows and Linux/Unix
 * Copyright (C) 2011 milkyjing <milkyjing@gmail.com>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * milkyjing
 *
 */

#ifndef LIBTCAS_MAPFILE_H
#define LIBTCAS_MAPFILE_H
#pragma once

#if defined WIN32
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif  /* WIN32 */


/* Inhibit C++ name-mangling for libtcas functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

#if defined WIN32
extern unsigned long *libtcas_mapfile(const char *filename, HANDLE *pMapFileHandle, unsigned long **pMapBuf);

extern void libtcas_unmapfile(HANDLE hMapFile, unsigned long *mapBuf);
#else
extern unsigned long *libtcas_mapfile(const char *filename, unsigned long *pSize, unsigned long **pMapBuf);

extern void libtcas_unmapfile(unsigned long *mapBuf, unsigned long size);
#endif  /* WIN32 */

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* LIBTCAS_MAPFILE_H */

