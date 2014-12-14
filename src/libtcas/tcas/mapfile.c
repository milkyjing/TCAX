/*
 *  Copyright (C) 2011 milkyjing <milkyjing@gmail.com>
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

#include "mapfile.h"


#if defined WIN32
unsigned long *libtcas_mapfile(const char *filename, HANDLE *pMapFileHandle, unsigned long **pMapBuf) {
    HANDLE hFile;
    HANDLE hMapFile;
    unsigned long *buf;
    hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile) return NULL;
    hMapFile = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    CloseHandle(hFile);
    if (!hMapFile) return NULL;
    buf = (unsigned long *)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
    *pMapFileHandle = hMapFile;
    if (pMapBuf)
        *pMapBuf = buf;
    return buf;
}

void libtcas_unmapfile(HANDLE hMapFile, unsigned long *mapBuf) {
    UnmapViewOfFile(mapBuf);
    CloseHandle(hMapFile);
}
#else
unsigned long *libtcas_mapfile(const char *filename, unsigned long *pSize, unsigned long **pMapBuf) {
    int fd;
    struct stat st;
    unsigned long *buf;
    if ((fd = open(filename, O_RDONLY)) == -1) return NULL;
    fstat(fd, &st);
    if ((buf = (unsigned long *)mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        close(fd);
        return NULL;
    }
    *pSize = st.st_size;
    if (pMapBuf)
        *pMapBuf = buf;
    return buf;
}

void libtcas_unmapfile(unsigned long *mapBuf, unsigned long size) {
    munmap(mapBuf, size);
}
#endif  /* WIN32 */

