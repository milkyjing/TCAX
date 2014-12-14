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
    tcas_u32 chunks;
    TCAS_pFile pFile;
    TCAS_pHeader pHeader;
    TCAS_pRawChunk pRawChunk;
    TCAS_pChunk pChunk;
    pFile = (TCAS_pFile)malloc(sizeof(TCAS_File));
    pHeader = (TCAS_pHeader)malloc(sizeof(TCAS_Header));
    pRawChunk = (TCAS_pRawChunk)malloc(4 * sizeof(TCAS_RawChunk));
    libtcas_open_file(pFile, "test_raw.tcas", tcas_file_open_existing);
    libtcas_read_header(pFile, pHeader, tcas_false);
    libtcas_read_raw_chunks(pFile, pRawChunk, 4);
    libtcas_close_file(pFile);
    memset(pFile, 0, sizeof(TCAS_File));
    libtcas_open_file(pFile, "test.tcas", tcas_file_create_new);
    pFile->minTime = pHeader->minTime;
    pFile->maxTime = pHeader->maxTime;
    libtcas_set_file_position_indicator(pFile, tcas_fpi_header);
    libtcas_convert_raw_chunks(pRawChunk, 4, &pChunk, &chunks);
    pFile->chunks += chunks;
    libtcas_write_chunks(pFile, pChunk, chunks, tcas_true);
    libtcas_set_header(pHeader, TCAS_FILE_TYPE_COMPRESSED, 0, GETPOSX(pHeader->resolution), GETPOSY(pHeader->resolution), pFile->minTime, pFile->maxTime, pFile->chunks, pHeader->fpsNumerator, pHeader->fpsDenominator);
    libtcas_write_header(pFile, pHeader, tcas_false);
    libtcas_close_file(pFile);
    free(pRawChunk);
    free(pHeader);
    free(pFile);
    return 0;
}



