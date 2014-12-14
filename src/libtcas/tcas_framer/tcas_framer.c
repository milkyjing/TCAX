#define TCAS_FRAMER_INTERNAL

#include "tcas_framer.h"

#include "../tcas/tcas.h"
#include "../tcas/hla_z_comp.h"

#include <string.h>

#include "../tcas/vector.c"
#include "../tcas/rb.c"
#include "../tcas/tcas.c"
#include "../tcas/hla_z_comp.c"


typedef struct _tcas_st {
    TCAS_File file;
    TCAS_Header header;
    TCAS_IndexStreamsPtr indexStreams;
    TCAS_ChunksCache chunksCache;
    TCAS_pCompactedIndex pParsedIndexs;    /* for parsed tcas file */
    tcas_u32 fpsNumerator;
    tcas_u32 fpsDenominator;
    tcas_u32 minFrame;
    tcas_u32 maxFrame;
    tcas_u16 width;
    tcas_u16 height;
} TcasSt, *TcasPtr;


TcasPtr tcas_open(const char *filename, char *szErr) {
    TcasPtr pTcas;
    TCAS_pOrderedIndex pOrderedIndexs;
    pTcas = (TcasPtr)malloc(sizeof(TcasSt));
    if (libtcas_open_file(&pTcas->file, filename, tcas_file_open_existing) != tcas_error_success) {
        if (szErr)
            strcpy(szErr, "Error: cannot open the TCAS file.");
        return NULL;
    }
    if (libtcas_read_header(&pTcas->file, &pTcas->header, 0) != tcas_error_success) {
        libtcas_close_file(&pTcas->file);
        if (szErr)
            strcpy(szErr, "Error: cannot read the TCAS file.");
        return NULL;
    }
    pTcas->fpsNumerator = pTcas->header.fpsNumerator;
    pTcas->fpsDenominator = pTcas->header.fpsDenominator;
    if (TCAS_FILE_TYPE_COMPRESSED == GETHI16B(pTcas->header.flag)) {
        if (0 == pTcas->header.chunks)
            libtcas_get_min_max_time_and_chunks(&pTcas->file, &pTcas->header.minTime, &pTcas->header.maxTime, &pTcas->header.chunks);
        if (libtcas_create_ordered_index(&pTcas->file, &pTcas->header, pTcas->fpsNumerator, pTcas->fpsDenominator, &pOrderedIndexs, NULL) != tcas_error_success) {
            libtcas_close_file(&pTcas->file);
            if (szErr)
                strcpy(szErr, "Error: cannot parse the TCAS file, type1.");
            return NULL;
        }
        pTcas->indexStreams = libtcas_linearize_ordered_indexs(pOrderedIndexs, pTcas->header.chunks, NULL);
        free(pOrderedIndexs);
        libtcas_init_chunks_cache(&pTcas->chunksCache, vector_get_count(pTcas->indexStreams));
        pTcas->pParsedIndexs = NULL;
    } else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(pTcas->header.flag)) {
        if (0 == pTcas->header.chunks)
            libtcas_get_min_max_time_and_chunks_z(&pTcas->file, &pTcas->header.minTime, &pTcas->header.maxTime, &pTcas->header.chunks);
        if (libtcas_create_ordered_index_z(&pTcas->file, &pTcas->header, pTcas->fpsNumerator, pTcas->fpsDenominator, &pOrderedIndexs, NULL) != tcas_error_success) {
            libtcas_close_file(&pTcas->file);
            if (szErr)
                strcpy(szErr, "Error: cannot parse the TCAS file, type2.");
            return NULL;
        }
        pTcas->indexStreams = libtcas_linearize_ordered_indexs(pOrderedIndexs, pTcas->header.chunks, NULL);
        free(pOrderedIndexs);
        libtcas_init_chunks_cache(&pTcas->chunksCache, vector_get_count(pTcas->indexStreams));
        pTcas->pParsedIndexs = NULL;
    } else if (TCAS_FILE_TYPE_PARSED == GETHI16B(pTcas->header.flag)) {
        if (0 == pTcas->header.chunks)
            libtcas_get_min_max_time_and_chunks(&pTcas->file, &pTcas->header.minTime, &pTcas->header.maxTime, &pTcas->header.chunks);
        if (libtcas_index_parsed_tcas_file(&pTcas->file, &pTcas->header, pTcas->fpsNumerator, pTcas->fpsDenominator, &pTcas->pParsedIndexs, NULL) != tcas_error_success) {
            libtcas_close_file(&pTcas->file);
            if (szErr)
                strcpy(szErr, "Error: cannot parse the TCAS file, type3.");
            return NULL;
        }
        pTcas->indexStreams = NULL;
        pTcas->chunksCache.streamCount = 0;
    } else if (TCAS_FILE_TYPE_PARSED_Z == GETHI16B(pTcas->header.flag)) {
        if (0 == pTcas->header.chunks)
            libtcas_get_min_max_time_and_chunks_z(&pTcas->file, &pTcas->header.minTime, &pTcas->header.maxTime, &pTcas->header.chunks);
        if (libtcas_index_parsed_tcas_file_z(&pTcas->file, &pTcas->header, pTcas->fpsNumerator, pTcas->fpsDenominator, &pTcas->pParsedIndexs, NULL) != tcas_error_success) {
            libtcas_close_file(&pTcas->file);
            if (szErr)
                strcpy(szErr, "Error: cannot parse the TCAS file, type4.");
            return NULL;
        }
        pTcas->indexStreams = NULL;
        pTcas->chunksCache.streamCount = 0;
    } else {
        libtcas_close_file(&pTcas->file);
        if (szErr)
            strcpy(szErr, "Error: does not support this TCAS file type yet.");
        return NULL;
    }
    pTcas->minFrame = (tcas_u32)((tcas_s64)pTcas->header.minTime * pTcas->fpsNumerator / (pTcas->fpsDenominator * 1000)) + 1;
    pTcas->maxFrame = (tcas_u32)((tcas_s64)pTcas->header.maxTime * pTcas->fpsNumerator / (pTcas->fpsDenominator * 1000)) + 1;
    pTcas->width = GETPOSX(pTcas->header.resolution);
    pTcas->height = GETPOSY(pTcas->header.resolution);
    return pTcas;
}

void tcas_close(TcasPtr pTcas) {
    if (pTcas->indexStreams)
        vector_destroy(pTcas->indexStreams);
    pTcas->indexStreams = NULL;
    if (pTcas->chunksCache.streamCount > 0)
        libtcas_free_chunks_cache(&pTcas->chunksCache);
    if (pTcas->pParsedIndexs)
        free(pTcas->pParsedIndexs);
    libtcas_close_file(&pTcas->file);
    free(pTcas);
}

__forceinline int tcas_get_width(const TcasPtr pTcas) {
    return pTcas->width;
}

__forceinline int tcas_get_height(const TcasPtr pTcas) {
    return pTcas->height;
}

__forceinline long tcas_get_min_frame(const TcasPtr pTcas) {
    return pTcas->minFrame;
}

__forceinline long tcas_get_max_frame(const TcasPtr pTcas) {
    return pTcas->maxFrame;
}

__forceinline double tcas_get_fps(const TcasPtr pTcas) {
    return pTcas->fpsNumerator / (double)pTcas->fpsDenominator;
}

unsigned char *tcas_get_frame(const TcasPtr pTcas, long n) {
    unsigned char *buf;
    switch (GETHI16B(pTcas->header.flag)) {
    case TCAS_FILE_TYPE_COMPRESSED:
        libtcas_create_frame_with_linearized_index(&pTcas->file, &pTcas->header, pTcas->indexStreams, n, &pTcas->chunksCache, pTcas->width, pTcas->height, &buf);
        break;
    case TCAS_FILE_TYPE_COMPRESSED_Z:
        libtcas_create_frame_with_linearized_index_z(&pTcas->file, &pTcas->header, pTcas->indexStreams, n, &pTcas->chunksCache, pTcas->width, pTcas->height, &buf);
        break;
    case TCAS_FILE_TYPE_PARSED:
        libtcas_create_frame_with_parsed_index(&pTcas->file, &pTcas->header, pTcas->pParsedIndexs, pTcas->header.chunks, n, pTcas->width, pTcas->height, &buf);
        break;
    case TCAS_FILE_TYPE_PARSED_Z:
        libtcas_create_frame_with_parsed_index_z(&pTcas->file, &pTcas->header, pTcas->pParsedIndexs, pTcas->header.chunks, n, pTcas->width, pTcas->height, &buf);
        break;
    }
    return buf;
}

