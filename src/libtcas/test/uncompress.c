/*
 *  tcas.h -- interface of the 'libtcas' library
 *
 *  Copyright (C) 2009-2010 milkyjing <milkyjing@gmail.com>
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
    tcas_u32 rawChunks;
    TCAS_pFile pFile;
    TCAS_pHeader pHeader;
    TCAS_pRawChunk pRawChunk;
    TCAS_pChunk pChunk;
    pFile = (TCAS_pFile)malloc(sizeof(TCAS_File));
    pHeader = (TCAS_pHeader)malloc(sizeof(TCAS_Header));
    libtcas_open_file(pFile, "test.tcas", tcas_file_open_existing);
    libtcas_read_header(pFile, pHeader, 0);
    pChunk = (TCAS_pChunk)malloc(pHeader->chunks * sizeof(TCAS_Chunk));
    libtcas_read_chunks(pFile, pChunk, pHeader->chunks);
    libtcas_close_file(pFile);
    libtcas_open_file(pFile, "test_uncompressed.tcas", tcas_file_create_new);
    libtcas_convert_chunks(pChunk, pHeader->chunks, &pRawChunk, &rawChunks, 1);
    libtcas_set_header(pHeader, TCAS_FILE_TYPE_RAW, GETLOW16B(pHeader->flag), GETPOSX(pHeader->resolution), GETPOSY(pHeader->resolution), pHeader->minTime, pHeader->maxTime, rawChunks, pHeader->fpsNumerator, pHeader->fpsDenominator);
    libtcas_write_header(pFile, pHeader, 0);
    libtcas_write_raw_chunks(pFile, pRawChunk, rawChunks);
    libtcas_close_file(pFile);
    free(pChunk);
    free(pRawChunk);
    free(pHeader);
    free(pFile);
    return 0;
}

