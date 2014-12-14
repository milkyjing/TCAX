/*
 *  Copyright (C) 2011 milkyjing <milkyjing@gmail.com>
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

#include "hla_framer.h"


TCAS_Error_Code libtcas_framer_init(const char *filename, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, tcas_u16 width, tcas_u16 height, TCAS_pFramer pFramer) {
    TCAS_Error_Code error;
    TCAS_pOrderedIndex pOrderedIndexs;
    error = libtcas_open_file(&pFramer->file, filename, tcas_file_open_existing);
    if (tcas_error_success != error) {
        printf("Error: can not open the TCAS file.\n");
        return error;
    }
    error = libtcas_read_header(&pFramer->file, &pFramer->header, 0);
    if (tcas_error_success != error) {
        libtcas_close_file(&pFramer->file);
        printf("Error: can not read the TCAS file.\n");
        return error;
    }
    if ((tcas_u32)(-1) == fpsNumerator && 1 == fpsDenominator) {
        fpsNumerator = pFramer->header.fpsNumerator;
        fpsDenominator = pFramer->header.fpsDenominator;
    }
    if (TCAS_FILE_TYPE_COMPRESSED == GETHI16B(pFramer->header.flag)) {
        if (0 == pFramer->header.chunks)
            libtcas_get_min_max_time_and_chunks(&pFramer->file, &pFramer->header.minTime, &pFramer->header.maxTime, &pFramer->header.chunks);
        error = libtcas_create_ordered_index(&pFramer->file, &pFramer->header, fpsNumerator, fpsDenominator, &pOrderedIndexs, NULL);
        if (tcas_error_success != error) {
            libtcas_close_file(&pFramer->file);
            printf("Error: can not parse the TCAS file, type1.\n");
            return error;
        }
        pFramer->indexStreams = libtcas_linearize_ordered_indexs(pOrderedIndexs, pFramer->header.chunks, NULL);
        free(pOrderedIndexs);
        libtcas_init_chunks_cache(&pFramer->chunksCache, vector_get_count(pFramer->indexStreams));
        pFramer->pParsedIndexs = NULL;
    } else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(pFramer->header.flag)) {
        if (0 == pFramer->header.chunks)
            libtcas_get_min_max_time_and_chunks_z(&pFramer->file, &pFramer->header.minTime, &pFramer->header.maxTime, &pFramer->header.chunks);
        error = libtcas_create_ordered_index_z(&pFramer->file, &pFramer->header, fpsNumerator, fpsDenominator, &pOrderedIndexs, NULL);
        if (tcas_error_success != error) {
            libtcas_close_file(&pFramer->file);
            printf("Error: can not parse the TCAS file, type2.\n");
            return error;
        }
        pFramer->indexStreams = libtcas_linearize_ordered_indexs(pOrderedIndexs, pFramer->header.chunks, NULL);
        free(pOrderedIndexs);
        libtcas_init_chunks_cache(&pFramer->chunksCache, vector_get_count(pFramer->indexStreams));
        pFramer->pParsedIndexs = NULL;
    } else if (TCAS_FILE_TYPE_PARSED == GETHI16B(pFramer->header.flag)) {
        if (0 == pFramer->header.chunks)
            libtcas_get_min_max_time_and_chunks(&pFramer->file, &pFramer->header.minTime, &pFramer->header.maxTime, &pFramer->header.chunks);
        error = libtcas_index_parsed_tcas_file(&pFramer->file, &pFramer->header, fpsNumerator, fpsDenominator, &pFramer->pParsedIndexs, NULL);
        if (tcas_error_success != error) {
            libtcas_close_file(&pFramer->file);
            printf("Error: can not parse the TCAS file, type3.\n");
            return error;
        }
        pFramer->indexStreams = NULL;
        pFramer->chunksCache.streamCount = 0;
    } else if (TCAS_FILE_TYPE_PARSED_Z == GETHI16B(pFramer->header.flag)) {
        if (0 == pFramer->header.chunks)
            libtcas_get_min_max_time_and_chunks_z(&pFramer->file, &pFramer->header.minTime, &pFramer->header.maxTime, &pFramer->header.chunks);
        error = libtcas_index_parsed_tcas_file_z(&pFramer->file, &pFramer->header, fpsNumerator, fpsDenominator, &pFramer->pParsedIndexs, NULL);
        if (tcas_error_success != error) {
            libtcas_close_file(&pFramer->file);
            printf("Error: can not parse the TCAS file, type3.\n");
            return error;
        }
        pFramer->indexStreams = NULL;
        pFramer->chunksCache.streamCount = 0;
    } else {
        libtcas_close_file(&pFramer->file);
        printf("Error: tcasfilter does no support this TCAS file type yet.\n");
        return tcas_error_file_type_not_support;
    }
    pFramer->width = width;
    pFramer->height = height;
    pFramer->minFrame = libtcas_MulDiv(pFramer->header.minTime, fpsNumerator, fpsDenominator * 1000) + 1;
    pFramer->maxFrame = libtcas_MulDiv(pFramer->header.maxTime, fpsNumerator, fpsDenominator * 1000) + 1;
    return tcas_error_success;
}

int libtcas_framer_get(TCAS_pFramer pFramer, tcas_s32 n, tcas_byte **pBuf) {
    switch (GETHI16B(pFramer->header.flag)) {
    case TCAS_FILE_TYPE_COMPRESSED:
        libtcas_create_frame_with_linearized_index(&pFramer->file, &pFramer->header, pFramer->indexStreams, n, &pFramer->chunksCache, pFramer->width, pFramer->height, pBuf);
        break;
    case TCAS_FILE_TYPE_COMPRESSED_Z:
        libtcas_create_frame_with_linearized_index_z(&pFramer->file, &pFramer->header, pFramer->indexStreams, n, &pFramer->chunksCache, pFramer->width, pFramer->height, pBuf);
        break;
    case TCAS_FILE_TYPE_PARSED:
        libtcas_create_frame_with_parsed_index(&pFramer->file, &pFramer->header, pFramer->pParsedIndexs, pFramer->header.chunks, n, pFramer->width, pFramer->height, pBuf);
        break;
    case TCAS_FILE_TYPE_PARSED_Z:
        libtcas_create_frame_with_parsed_index_z(&pFramer->file, &pFramer->header, pFramer->pParsedIndexs, pFramer->header.chunks, n, pFramer->width, pFramer->height, pBuf);
        break;
    }
    return 0;
}

void libtcas_framer_fin(TCAS_pFramer pFramer) {
    if (pFramer->indexStreams)
        vector_destroy(pFramer->indexStreams);
    pFramer->indexStreams = NULL;
    if (pFramer->chunksCache.streamCount > 0)
        libtcas_free_chunks_cache(&pFramer->chunksCache);
    if (pFramer->pParsedIndexs)
        free(pFramer->pParsedIndexs);
    libtcas_close_file(&pFramer->file);
}

