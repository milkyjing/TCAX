/*
 *  Copyright (C) 2012 milkyjing <milkyjing@gmail.com>
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

#include "hla_stream_parser.h"


TCAS_Error_Code libtcas_stream_parser_init(const tcas_unit *tcasBuf, tcas_u32 chunks, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, tcas_u16 width, tcas_u16 height, TCAS_pStreamParser pParser) {
    tcas_s32 minTime, maxTime;
    TCAS_pOrderedIndex pOrderedIndexs;
    libtcas_get_chunk_stream_min_max_time(tcasBuf, chunks, &minTime, &maxTime);
    libtcas_create_ordered_index_from_stream(tcasBuf, chunks, 0, 0, fpsNumerator, fpsDenominator, &pOrderedIndexs, NULL, NULL, NULL);
    pParser->indexStreams = libtcas_linearize_ordered_indexs_deprecated(pOrderedIndexs, chunks, NULL);    // because TCAS_BUF is usually small, so we use old version to have better performance
    free(pOrderedIndexs);
    libtcas_init_chunks_cache(&pParser->chunksCache, vector_get_count(pParser->indexStreams));
    pParser->tcasBuf = tcasBuf;
    pParser->chunks = chunks;
    pParser->fpsNumerator = fpsNumerator;
    pParser->fpsDenominator = fpsDenominator;
    pParser->width = width;
    pParser->height = height;
    pParser->minFrame = libtcas_MulDiv(minTime, fpsNumerator, fpsDenominator * 1000) + 1;
    pParser->maxFrame = libtcas_MulDiv(maxTime, fpsNumerator, fpsDenominator * 1000) + 1;
    return tcas_error_success;
}

int libtcas_stream_parser_parse(TCAS_pStreamParser pParser, VectorPtr chunksVector) {
    TCAS_Chunk chunk;
    tcas_byte *rgba;
    tcas_s32 n;
    tcas_u32 size;
    size = pParser->width * pParser->height * sizeof(tcas_u32);
    rgba = (tcas_byte *)malloc(size);
    for (n = pParser->minFrame; n < pParser->maxFrame; n++) {
        memset(rgba, 0, size);
        libtcas_fill_frame_from_stream_with_linearized_index(pParser->tcasBuf, pParser->indexStreams, n, &pParser->chunksCache, rgba, pParser->width, pParser->height);
        chunk.startTime = libtcas_MulDiv(n, pParser->fpsDenominator * 1000, pParser->fpsNumerator);
        chunk.endTime = libtcas_MulDiv(n + 1, pParser->fpsDenominator * 1000, pParser->fpsNumerator);
        chunk.cltp = 0;
        libtcas_convert_rgba_to_chunk(rgba, pParser->width, pParser->height, TCAS_TRUE, &chunk);
        if (GETCOUNT(chunk.cltp) > 0)
            vector_push_back(chunksVector, &chunk);
    }
    free(rgba);
    return 0;
}

void libtcas_stream_parser_fin(TCAS_pStreamParser pParser) {
    if (pParser->indexStreams)
        vector_destroy(pParser->indexStreams);
    pParser->indexStreams = NULL;
    if (pParser->chunksCache.streamCount > 0)
        libtcas_free_chunks_cache(&pParser->chunksCache);
}

