/*
 *  tcas.h -- interface of the 'libtcas' library
 *
 *  Copyright (C) 2011 milkyjing <milkyjing@gmail.com>
 *
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty.  In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 *
 *  milkyjing
 *
 */

#include "../tcas/tcas.h"
#include "../tcas/tcas.c"

int main() {
    TCAS_pFile pFile;
    TCAS_pHeader pHeader;
    TCAS_pRawChunk pRawChunk;
    pFile = (TCAS_pFile)malloc(sizeof(TCAS_File));
    memset(pFile, 0, sizeof(TCAS_File));
    pFile->minTime = TCAS_INIT_MIN_TIME;
    pHeader = (TCAS_pHeader)malloc(sizeof(TCAS_Header));
    pRawChunk = (TCAS_pRawChunk)malloc(4 * sizeof(TCAS_RawChunk));
    pRawChunk[0].startTime = 0;
    pRawChunk[0].endTime = 100;
    pRawChunk[0].frameType = 0;
    pRawChunk[0].layer = 5;
    pRawChunk[0].posX = 100;
    pRawChunk[0].posY = 100;
    pRawChunk[0].r = (tcas_byte)1;
    pRawChunk[0].g = (tcas_byte)2;
    pRawChunk[0].b = (tcas_byte)3;
    pRawChunk[0].a = (tcas_byte)4;
    pFile->minTime = __min(pFile->minTime, pRawChunk[0].startTime);
    pFile->maxTime = __max(pFile->maxTime, pRawChunk[0].endTime);
    pFile->chunks ++;
    pRawChunk[1].startTime = 200;
    pRawChunk[1].endTime = 300;
    pRawChunk[1].frameType = 0;
    pRawChunk[1].layer = 1;
    pRawChunk[1].posX = 200;
    pRawChunk[1].posY = 100;
    pRawChunk[1].r = (tcas_byte)5;
    pRawChunk[1].g = (tcas_byte)6;
    pRawChunk[1].b = (tcas_byte)7;
    pRawChunk[1].a = (tcas_byte)8;
    pFile->minTime = __min(pFile->minTime, pRawChunk[1].startTime);
    pFile->maxTime = __max(pFile->maxTime, pRawChunk[1].endTime);
    pFile->chunks ++;
    pRawChunk[2].startTime = 400;
    pRawChunk[2].endTime = 500;
    pRawChunk[2].frameType = 0;
    pRawChunk[2].layer = 3;
    pRawChunk[2].posX = 100;
    pRawChunk[2].posY = 200;
    pRawChunk[2].r = (tcas_byte)1;
    pRawChunk[2].g = (tcas_byte)2;
    pRawChunk[2].b = (tcas_byte)3;
    pRawChunk[2].a = (tcas_byte)4;
    pFile->minTime = __min(pFile->minTime, pRawChunk[2].startTime);
    pFile->maxTime = __max(pFile->maxTime, pRawChunk[2].endTime);
    pFile->chunks ++;
    pRawChunk[3].startTime = 400;
    pRawChunk[3].endTime = 500;
    pRawChunk[3].frameType = 0;
    pRawChunk[3].layer = 3;
    pRawChunk[3].posX = 200;
    pRawChunk[3].posY = 200;
    pRawChunk[3].r = (tcas_byte)1;
    pRawChunk[3].g = (tcas_byte)2;
    pRawChunk[3].b = (tcas_byte)3;
    pRawChunk[3].a = (tcas_byte)4;
    pFile->minTime = __min(pFile->minTime, pRawChunk[3].startTime);
    pFile->maxTime = __max(pFile->maxTime, pRawChunk[3].endTime);
    pFile->chunks ++;
    libtcas_open_file(pFile, "test_raw.tcas", tcas_file_create_new);
    libtcas_set_file_position_indicator(pFile, tcas_fpi_header);
    libtcas_write_raw_chunks(pFile, pRawChunk, 4);
    libtcas_set_header(pHeader, TCAS_FILE_TYPE_RAW, 0, 640, 480, pFile->minTime, pFile->maxTime, pFile->chunks, 1, 1);
    libtcas_write_header(pFile, pHeader, tcas_false);
    libtcas_close_file(pFile);
    free(pRawChunk);
    free(pHeader);
    free(pFile);
    return 0;
}







