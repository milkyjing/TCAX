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

#include "hla_z_comp.h"


tcas_bool libtcas_z_comp_inflate_init(z_stream *pStrmInf) {
    pStrmInf->zalloc = Z_NULL;
    pStrmInf->zfree = Z_NULL;
    pStrmInf->opaque = Z_NULL;
    pStrmInf->avail_in = 0;
    pStrmInf->next_in = Z_NULL;
    if (inflateInit(pStrmInf) == Z_OK)
        return TCAS_TRUE;
    return TCAS_FALSE;
}

tcas_bool libtcas_z_comp_deflate_init(z_stream *pStrmDef) {
    pStrmDef->zalloc = Z_NULL;
    pStrmDef->zfree = Z_NULL;
    pStrmDef->opaque = Z_NULL;
    if (deflateInit(pStrmDef, 1) == Z_OK)
        return TCAS_TRUE;
    return TCAS_FALSE;
}

TCAS_Error_Code libtcas_read_specified_chunk_z(TCAS_pFile pFile, tcas_u64 offset, TCAS_pChunk pChunk) {
    z_stream strmInf;
    tcas_u32 count, size;    /* count of pos_and_color */
    tcas_unit zSize;        /* size of zbuf */
    tcas_byte *zbuf;    /* buffer of zlib compressed pos_and_color */
    tcas_fseek(pFile->fp, offset, SEEK_SET);
    if (fread(pChunk, sizeof(tcas_unit), 3, pFile->fp) != 3) return tcas_error_file_while_reading;    /* startTime endTime cltp takes up 3 tcas_unit */
    if (fread(&zSize, sizeof(tcas_unit), 1, pFile->fp) != 1) return tcas_error_file_while_reading;    /* size takes up 1 tcas_unit */
    zbuf = (tcas_byte *)malloc(zSize);
    if (fread(zbuf, sizeof(tcas_byte), zSize, pFile->fp) != zSize) {
        free(zbuf);
        return tcas_error_file_while_reading;
    }
    libtcas_z_comp_inflate_init(&strmInf);
    strmInf.avail_in = zSize;
    strmInf.next_in = zbuf;
    count = GETCOUNT(pChunk->cltp);
    size = count * (sizeof(tcas_unit) << 1);    /* every pos_and_color takes up 2 tcas_unit */
    pChunk->pos_and_color = (tcas_unit *)malloc(size);
    strmInf.avail_out = size;
    strmInf.next_out = (tcas_byte *)pChunk->pos_and_color;
    inflate(&strmInf, Z_FINISH);
    free(zbuf);
    inflateEnd(&strmInf);
    return tcas_error_success;
}

tcas_unit *libtcas_compress_pos_and_color(tcas_unit *pos_and_color, tcas_u32 count) {
    z_stream strmDef;
    tcas_u32 size;
    tcas_unit zSize;
    tcas_unit *z_pos_and_color;    /* buffer of zlib compressed pos_and_color */
    size = count * (sizeof(tcas_unit) << 1);    /* every pos_and_color takes up 2 tcas_unit */
    libtcas_z_comp_deflate_init(&strmDef);
    strmDef.avail_in = size;
    strmDef.next_in = (tcas_byte *)pos_and_color;
    size += 1024;       /* make the buffer large enough */
    z_pos_and_color = (tcas_unit *)malloc(size);    /* should be large enough */
    size -= 4;  /* the first four bytes of z_pos_and_color is a tcas_unit that stores the zSize */
    strmDef.avail_out = size;
    strmDef.next_out = (tcas_byte *)(z_pos_and_color + 1);    /* first four bytes of z_pos_and_color stores the zSize */
    deflate(&strmDef, Z_FINISH);
    zSize = size - strmDef.avail_out;
    deflateEnd(&strmDef);
    z_pos_and_color[0] = zSize;
    return z_pos_and_color;
}

tcas_bool libtcas_compress_chunks_z(tcas_unit *chunksBuf, tcas_u32 chunkCount, TCAS_pChunk *ppChunks) {
    tcas_u32 i, offset, count;
    TCAS_pChunk pChunks;
    pChunks = (TCAS_pChunk)malloc(chunkCount * sizeof(TCAS_Chunk));
    offset = 0;
    for (i = 0; i < chunkCount; i++) {
        pChunks[i].startTime = chunksBuf[offset++];
        pChunks[i].endTime = chunksBuf[offset++];
        pChunks[i].cltp = chunksBuf[offset++];
        count = GETCOUNT(pChunks[i].cltp);
        pChunks[i].pos_and_color = libtcas_compress_pos_and_color(chunksBuf + offset, count);
        offset += (count << 1);
    }
    *ppChunks = pChunks;
    return TCAS_TRUE;
}

TCAS_Error_Code libtcas_write_chunk_z(TCAS_pFile pFile, const TCAS_pChunk pChunk) {
    tcas_unit zSize, zSizePadded;
    zSize = pChunk->pos_and_color[0];
    zSizePadded = zSize >> 2;
    if ((0x00000003 & zSize) != 0)
        zSizePadded++;
    if (fwrite(pChunk, sizeof(tcas_unit), 3, pFile->fp) != 3) return tcas_error_file_while_writing;   /* startTime endTime cltp takes up 3 tcas_unit */
    if (fwrite(pChunk->pos_and_color, sizeof(tcas_unit), 1 + zSizePadded, pFile->fp) != 1 + zSizePadded) return tcas_error_file_while_writing;    /* every pos_and_color takes up 2 tcas_unit */
    return tcas_error_success;
}

TCAS_Error_Code libtcas_write_chunks_z(TCAS_pFile pFile, const TCAS_pChunk pChunks, tcas_u32 count, tcas_bool freeChunks) {
    TCAS_Error_Code error;
    tcas_u32 i = 0;
    if (freeChunks) {
        do {
            error = libtcas_write_chunk_z(pFile, &pChunks[i]);
            libtcas_free_chunk(&pChunks[i]);
            i++;
        } while (i < count && tcas_error_success == error);
    } else {
        do error = libtcas_write_chunk_z(pFile, &pChunks[i++]);
        while (i < count && tcas_error_success == error);
    }
    return error;
}

TCAS_Error_Code libtcas_get_min_max_time_and_chunks_z(const TCAS_pFile pFile, tcas_s32 *pMinTime, tcas_s32 *pMaxTime, tcas_unit *pChunkCount) {
    TCAS_Error_Code error;
    fpos_t position;
    tcas_s32 mintime, maxtime;
    tcas_u32 count;    /* count is the same as *chunks */
    tcas_unit buf[4];    /* to hold startTime endTime cltp zSize */
    tcas_unit zSizePadded;
    TCAS_Header header;
    mintime = TCAS_INIT_MIN_TIME;
    maxtime = TCAS_INIT_MAX_TIME;
    count = 0;
    fgetpos(pFile->fp, &position);    /* remember file position indicator */
    error = libtcas_read_header(pFile, &header, TCAS_FALSE);
    if (tcas_error_success != error) {
        fsetpos(pFile->fp, &position);    /* reset file position indicator */
        return error;
    }
    if (TCAS_FILE_TYPE_COMPRESSED_Z != GETHI16B(header.flag) && TCAS_FILE_TYPE_PARSED_Z != GETHI16B(header.flag)) {
        fsetpos(pFile->fp, &position);    /* reset file position indicator */
        return tcas_error_file_type_not_match;
    }
    while (1) {
        if (fread(buf, sizeof(tcas_unit), 4, pFile->fp) != 4) {   /* startTime endTime cltp zSize takes up 4 tcas_unit */
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            *pMinTime = mintime;
            *pMaxTime = maxtime;
            *pChunkCount = count;
            return tcas_error_success;
        }
        if (feof(pFile->fp) == 0) {
            zSizePadded = buf[3] >> 2;
            if ((0x00000003 & buf[3]) != 0)
                zSizePadded++;
            tcas_fseek(pFile->fp, zSizePadded * sizeof(tcas_unit), SEEK_CUR);
            mintime = __min(mintime, (tcas_s32)buf[0]);
            maxtime = __max(maxtime, (tcas_s32)buf[1]);
            count++;
        } else break;
    }
    fsetpos(pFile->fp, &position);    /* reset file position indicator */
    *pMinTime = mintime;
    *pMaxTime = maxtime;
    *pChunkCount = count;
    return tcas_error_success;
}

TCAS_Error_Code libtcas_create_ordered_index_z(const TCAS_pFile pFile, const TCAS_pHeader pHeader, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, TCAS_pOrderedIndex *ppOrderedIndexs, tcas_u32 *pOrderedIndexCount) {
    TCAS_Error_Code error;
    fpos_t position;
    TCAS_pOrderedIndex pOrderedIndexs;
    tcas_u32 i, offset, chunks, zSizePadded;
    tcas_unit buf[4];    /* a temp buffer to hold startTime endTime cltp zSize */
    fgetpos(pFile->fp, &position);    /* remember file position indicator */
    if (NULL == pHeader) {
        TCAS_Header header;
        error = libtcas_read_header(pFile, &header, TCAS_FALSE);
        if (tcas_error_success != error) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            return error;
        }
        if (TCAS_FILE_TYPE_COMPRESSED_Z != GETHI16B(header.flag)) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            return tcas_error_file_type_not_match;
        }
        chunks = header.chunks;    /* get the amount of chunks */
    } else {
        if (TCAS_FILE_TYPE_COMPRESSED_Z != GETHI16B(pHeader->flag)) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            return tcas_error_file_type_not_match;
        }
        libtcas_set_file_position_indicator(pFile, tcas_fpi_header);
        chunks = pHeader->chunks;    /* get the amount of chunks */
    }
    pOrderedIndexs = (TCAS_pOrderedIndex)malloc(chunks * sizeof(TCAS_OrderedIndex));
    offset = TCAS_HEADER_OFFSET;
    if (0 == fpsNumerator || 0 == fpsDenominator) {
        for (i = 0; i < chunks; i++) {
            if (fread(buf, sizeof(tcas_unit), 4, pFile->fp) != 4) {   /* startTime endTime cltp zSize takes up 4 tcas_unit */
                free(pOrderedIndexs);
                fsetpos(pFile->fp, &position);    /* reset file position indicator */
                return tcas_error_file_while_reading;
            }
            pOrderedIndexs[i].first = buf[0];    /* startTime */
            pOrderedIndexs[i].last = buf[1];     /* endTime */
            if (pOrderedIndexs[i].first == pOrderedIndexs[i].last && 0 == GETTYPE(buf[2]))  /* a normal chunk which has no effect */
                pOrderedIndexs[i].id_pair = MAKEINDEXIP(0, 1);  /* will not be used */
            else
                pOrderedIndexs[i].id_pair = MAKEINDEXIP(i + 1, GETPAIR(buf[2]));            /* used to reserve the order of chunks, note that valid id starts from 1 */
            pOrderedIndexs[i].offset = offset;
            zSizePadded = buf[3] >> 2;
            if ((0x00000003 & buf[3]) != 0)    /* z_pos_and_color is padded to a multiple of sizeof(tcas_unit), zSize is the real size of z_pos_and_color */
                zSizePadded++;
            offset += 4 + zSizePadded;
            tcas_fseek(pFile->fp, zSizePadded * sizeof(tcas_unit), SEEK_CUR);
        }
    } else {
        for (i = 0; i < chunks; i++) {
            if (fread(buf, sizeof(tcas_unit), 4, pFile->fp) != 4) {   /* startTime endTime cltp zSize takes up 4 tcas_unit */
                free(pOrderedIndexs);
                fsetpos(pFile->fp, &position);    /* reset file position indicator */
                return tcas_error_file_while_reading;
            }
            pOrderedIndexs[i].first = libtcas_MulDiv((tcas_s32)buf[0], fpsNumerator, fpsDenominator * 1000) + 1;    /* firstFrame, note: +1 is just intend to make it compatible with VSFilter */
            pOrderedIndexs[i].last = libtcas_MulDiv((tcas_s32)buf[1], fpsNumerator, fpsDenominator * 1000) + 1;    /* lastFrame, note: +1 is just intend to make it compatible with VSFilter */
            if (pOrderedIndexs[i].first == pOrderedIndexs[i].last && 0 == GETTYPE(buf[2]))  /* a normal chunk which has no effect */
                pOrderedIndexs[i].id_pair = MAKEINDEXIP(0, 1);  /* will not be used */
            else
                pOrderedIndexs[i].id_pair = MAKEINDEXIP(i + 1, GETPAIR(buf[2]));            /* used to reserve the order of chunks */
            pOrderedIndexs[i].offset = offset;
            zSizePadded = buf[3] >> 2;
            if ((0x00000003 & buf[3]) != 0)    /* z_pos_and_color is padded to a multiple of sizeof(tcas_unit), zSize is the real size of z_pos_and_color */
                zSizePadded++;
            offset += 4 + zSizePadded;
            tcas_fseek(pFile->fp, zSizePadded * sizeof(tcas_unit), SEEK_CUR);
        }
    }
    fsetpos(pFile->fp, &position);    /* reset file position indicator */
    *ppOrderedIndexs = pOrderedIndexs;
    if (pOrderedIndexCount)
        *pOrderedIndexCount = chunks;
    return tcas_error_success;
}

TCAS_Error_Code libtcas_index_parsed_tcas_file_z(const TCAS_pFile pFile, const TCAS_pHeader pHeader, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, TCAS_pCompactedIndex *ppParsedIndexs, tcas_u32 *pIndexCount) {
    TCAS_Error_Code error;
    fpos_t position;
    TCAS_pCompactedIndex pParsedIndexs;
    tcas_u32 i, offset, chunks, zSizePadded;
    tcas_unit buf[4];    /* a temp buffer to hold startTime endTime cltp zSize */
    fgetpos(pFile->fp, &position);    /* remember file position indicator */
    if (NULL == pHeader) {
        TCAS_Header header;
        error = libtcas_read_header(pFile, &header, TCAS_FALSE);
        if (tcas_error_success != error) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            return error;
        }
        if (TCAS_FILE_TYPE_PARSED_Z != GETHI16B(header.flag)) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            return tcas_error_file_type_not_match;
        }
        chunks = header.chunks;    /* get the amount of chunks */
    } else {
        if (TCAS_FILE_TYPE_PARSED_Z != GETHI16B(pHeader->flag)) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            return tcas_error_file_type_not_match;
        }
        libtcas_set_file_position_indicator(pFile, tcas_fpi_header);
        chunks = pHeader->chunks;    /* get the amount of chunks */
    }
    pParsedIndexs = (TCAS_pCompactedIndex)malloc(chunks * sizeof(TCAS_CompactedIndex));
    offset = TCAS_HEADER_OFFSET;
    if (0 == fpsNumerator || 0 == fpsDenominator) {
        for (i = 0; i < chunks; i++) {
            if (fread(buf, sizeof(tcas_unit), 4, pFile->fp) != 4) {   /* startTime endTime cltp zSize takes up 4 tcas_unit */
                free(pParsedIndexs);
                fsetpos(pFile->fp, &position);    /* reset file position indicator */
                return tcas_error_file_while_reading;
            }
            pParsedIndexs[i].first = buf[0];    /* startTime */
            pParsedIndexs[i].last = buf[1];     /* endTime */
            pParsedIndexs[i].offset = offset;
            zSizePadded = buf[3] >> 2;
            if ((0x00000003 & buf[3]) != 0)    /* z_pos_and_color is padded to a multiple of sizeof(tcas_unit), zSize is the real size of z_pos_and_color */
                zSizePadded++;
            offset += 4 + zSizePadded;
            tcas_fseek(pFile->fp, zSizePadded * sizeof(tcas_unit), SEEK_CUR);
        }
    } else {
        for (i = 0; i < chunks; i++) {
            if (fread(buf, sizeof(tcas_unit), 4, pFile->fp) != 4) {   /* startTime endTime cltp takes up 3 tcas_unit */
                free(pParsedIndexs);
                fsetpos(pFile->fp, &position);    /* reset file position indicator */
                return tcas_error_file_while_reading;
            }
            pParsedIndexs[i].first = libtcas_MulDiv((tcas_s32)buf[0], fpsNumerator, fpsDenominator * 1000) + 1;    /* firstFrame, note: +1 is just intend to make it compatible with VSFilter */
            pParsedIndexs[i].last = libtcas_MulDiv((tcas_s32)buf[1], fpsNumerator, fpsDenominator * 1000) + 1;    /* lastFrame, note: +1 is just intend to make it compatible with VSFilter */
            pParsedIndexs[i].offset = offset;
            zSizePadded = buf[3] >> 2;
            if ((0x00000003 & buf[3]) != 0)    /* z_pos_and_color is padded to a multiple of sizeof(tcas_unit), zSize is the real size of z_pos_and_color */
                zSizePadded++;
            offset += 4 + zSizePadded;
            tcas_fseek(pFile->fp, zSizePadded * sizeof(tcas_unit), SEEK_CUR);
        }
    }
    fsetpos(pFile->fp, &position);    /* reset file position indicator */
    *ppParsedIndexs = pParsedIndexs;
    if (pIndexCount)
        *pIndexCount = chunks;
    return tcas_error_success;
}

/* libtcas_create_frame_with_linearized_index_z function() */

/* the function only cares about normal chunks, the first time you pass the pChunksCache to this function should set pChunksCache->streamCount = 0 */
static void _libtcas_parse_linearized_index_streams_normal_z(TCAS_pFile pFile, const TCAS_IndexStreamsPtr indexStreams, tcas_s32 n, TCAS_pChunksCache pChunksCache) {
    tcas_u32 i, count, indexCount;
    tcas_bool found;
    TCAS_LinearIndexsPtr linearIndexsTemp;
    TCAS_pOrderedIndex pOrderedIndexLast;  /* store the index that read at last time */
    TCAS_pOrderedIndex pOrderedIndexNext;  /* the index next to the last read one */
    count = vector_get_count(indexStreams);
    for (i = 0; i < count; i++) {    /* check every index stream */
        vector_retrieve(indexStreams, i, &linearIndexsTemp);  /* get the ith index stream, low copy, do not free linearIndexsTemp */
        indexCount = vector_get_count(linearIndexsTemp);
        pOrderedIndexLast = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);  /* get the last ordered index of the ith linearized index stream */
        found = TCAS_FALSE;
        if (pOrderedIndexLast->first <= n && n < pOrderedIndexLast->last) {  /* last chunk is still valid */
            if (NULL == pChunksCache->pChunks[i].pos_and_color) {  /* not read yet, this may happen just after the initialization */
                libtcas_read_specified_chunk_z(pFile, ((tcas_u64)pOrderedIndexLast->offset) << 2, &pChunksCache->pChunks[i]);  /* otherwise, we do not have to read it again */
                pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexLast->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
            }
            found = TCAS_TRUE;
            continue;  /* pi[i] and pChunks[i] stays unchanged, they are still valid */
        } else {  /* the last chunk is invalid */
            if (pChunksCache->pi[i] + 1 < indexCount) {  /* there is still a next index */
                pOrderedIndexNext = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 1, NULL);  /* get the next index */
                if (pOrderedIndexNext->first <= n && n < pOrderedIndexNext->last) {  /* the next index is valid */
                    if (pChunksCache->pChunks[i].pos_and_color)
                        free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                    libtcas_read_specified_chunk_z(pFile, ((tcas_u64)pOrderedIndexNext->offset) << 2, &pChunksCache->pChunks[i]);  /* cover the last one */
                    pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexNext->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                    pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
                    pChunksCache->pi[i]++;  /* indicate that we have checked the next one */
                    found = TCAS_TRUE;
                    continue;  /* both pi[i] and pChunks[i] have been updated */
                } else if (pOrderedIndexLast->last <= n && n < pOrderedIndexNext->first) {  /* we now know that there is no effect of this nth frame in the stream */
                    if (pChunksCache->pChunks[i].pos_and_color)
                        libtcas_free_chunk(&pChunksCache->pChunks[i]);  /* just erase it */
                    continue;  /* note that pi[i] stays unchanged */
                }
            }
        }
        if (TCAS_FALSE == found) {  /* we cannot find the index easily (maybe the video is moved forward or backward), so we use binary search to try again */
            long result;
            TCAS_OrderedIndex key;
            key.first = n;
            key.last = n;
            result = vector_binary_search(linearIndexsTemp, &key, (vector_compare_func)libtcas_check_normal_index_validity, NULL);
            if (-1 != result) {  /* we find it */
                pChunksCache->pi[i] = result;
                pOrderedIndexLast = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);
                if (pChunksCache->pChunks[i].pos_and_color)
                    free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                libtcas_read_specified_chunk_z(pFile, ((tcas_u64)pOrderedIndexLast->offset) << 2, &pChunksCache->pChunks[i]);  /* cover the last one */
                pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexLast->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
                found = TCAS_TRUE;
                continue;
            }
        }
        if (TCAS_FALSE == found) {  /* if still does not find the index */
            if (pChunksCache->pChunks[i].pos_and_color)
                libtcas_free_chunk(&pChunksCache->pChunks[i]);  /* just erase it */
        }
    }
}

/* the function only cares about key frame chunks, the first time you pass the pChunksCache to this function should set pChunksCache->streamCount = 0 */
static void _libtcas_parse_linearized_index_streams_key_z(TCAS_pFile pFile, const TCAS_IndexStreamsPtr indexStreams, tcas_s32 n, TCAS_pChunksCache pChunksCache) {
    tcas_u32 i, count, indexCount;
    tcas_bool found;
    TCAS_pChunk pInterChunkTemp;
    TCAS_LinearIndexsPtr linearIndexsTemp;
    TCAS_pOrderedIndex pOrderedIndexLastStart;  /* store the index that read at last time */
    TCAS_pOrderedIndex pOrderedIndexLastEnd;
    TCAS_pOrderedIndex pOrderedIndexNextStart;  /* the index next to the last read one */
    TCAS_pOrderedIndex pOrderedIndexNextEnd;
    count = vector_get_count(indexStreams);
    for (i = 0; i < count; i++) {  /* check every index stream */
        vector_retrieve(indexStreams, i, &linearIndexsTemp);  /* get the ith index stream, low copy, do not free linearIndexsTemp */
        indexCount = vector_get_count(linearIndexsTemp);
        pOrderedIndexLastStart = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);  /* get the last ordered index of the ith linearized index stream */
        found = TCAS_FALSE;
        if (pOrderedIndexLastStart->first == pOrderedIndexLastStart->last && 0 != GETINDEXID(pOrderedIndexLastStart->id_pair) && 0 == GETINDEXPAIR(pOrderedIndexLastStart->id_pair)) {  /* the last chunk is a key frame chunks */
            if (pChunksCache->pi[i] + 1 >= indexCount) {  /* this seldom happens, unless there is an error causing the lost of the end key frame chunk */
                if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                    libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                    libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
                }
                continue;
            }
            pOrderedIndexLastEnd = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 1, NULL);  /* then, it must have a next chunk, because the index is always indicating the start key frame chunk */
            if (!(pOrderedIndexLastEnd->first == pOrderedIndexLastEnd->last && 0 != GETINDEXID(pOrderedIndexLastEnd->id_pair) && 1 == GETINDEXPAIR(pOrderedIndexLastEnd->id_pair))) {  /* invalid end key frame chunk, this seldom happens, unless there is an error causing the lost of the end key frame chunk */
                if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                    libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                    libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
                }
                continue;
            }
            if (pOrderedIndexLastStart->first <= n && n < pOrderedIndexLastEnd->last) {  /* last chunk is still valid, note that, the last frame cannot be reached in order to be compatible with normal chunk frames */
                if (NULL == pChunksCache->pChunkPairs[i].start.pos_and_color) {  /* not read yet, this may happen just after the initialization, it is impossible that only one pos_and_color is NULL, must be both or neither */
                    libtcas_read_specified_chunk_z(pFile, ((tcas_u64)pOrderedIndexLastStart->offset) << 2, &pChunksCache->pChunkPairs[i].start);  /* otherwise, we do not have to read it again */
                    libtcas_read_specified_chunk_z(pFile, ((tcas_u64)pOrderedIndexLastEnd->offset) << 2, &pChunksCache->pChunkPairs[i].end);
                    libtcas_key_frame_chunk_add_dips(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end);
                }
                if (1 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                    libtcas_generate_intermediate_frame_chunk_fast(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexLastStart->first, pOrderedIndexLastEnd->last, n, &pInterChunkTemp);
                else if (2 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                    libtcas_generate_intermediate_frame_chunk(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexLastStart->first, pOrderedIndexLastEnd->last, n, &pInterChunkTemp);
                if (pChunksCache->pChunks[i].pos_and_color)
                    free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                memcpy(&pChunksCache->pChunks[i], pInterChunkTemp, sizeof(TCAS_Chunk));
                free(pInterChunkTemp);  /* Note that pInterChunkTemp is dynamically allocated */
                pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexLastStart->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
                found = TCAS_TRUE;
                continue;  /* pi[i] and pChunkPairs[i] stays unchanged, they are still valid */
            } else {  /* the last chunk is invalid */
                if (pChunksCache->pi[i] + 3 < indexCount) {  /* there may still be a key frame chunk index next to the two key frame chunks */
                    pOrderedIndexNextStart = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 2, NULL);  /* get the index next to the two key frame chunks */
                    if (pOrderedIndexNextStart->first == pOrderedIndexNextStart->last && 0 != GETINDEXID(pOrderedIndexNextStart->id_pair) && 0 == GETINDEXPAIR(pOrderedIndexNextStart->id_pair)) {  /* is key frame chunk */
                        pOrderedIndexNextEnd = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 3, NULL);  /* then, it must have a next chunk, because the index is always indicating the start key frame chunk */
                        if (!(pOrderedIndexNextEnd->first == pOrderedIndexNextEnd->last && 0 != GETINDEXID(pOrderedIndexNextEnd->id_pair) && 1 == GETINDEXPAIR(pOrderedIndexNextEnd->id_pair))) {  /* invalid end key frame chunk, this seldom happens, unless there is an error causing the lost of the end key frame chunk */
                            if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
                            }
                            continue;
                        }
                        if (pOrderedIndexNextStart->first <= n && n < pOrderedIndexNextEnd->last) {  /* last chunk is still valid, note that, the last frame cannot be reached in order to be compatible with normal chunk frames */
                            if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                                free(pChunksCache->pChunkPairs[i].start.pos_and_color);
                                free(pChunksCache->pChunkPairs[i].end.pos_and_color);
                            }
                            libtcas_read_specified_chunk_z(pFile, ((tcas_u64)pOrderedIndexNextStart->offset) << 2, &pChunksCache->pChunkPairs[i].start);
                            libtcas_read_specified_chunk_z(pFile, ((tcas_u64)pOrderedIndexNextEnd->offset) << 2, &pChunksCache->pChunkPairs[i].end);
                            libtcas_key_frame_chunk_add_dips(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end);
                            if (1 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                                libtcas_generate_intermediate_frame_chunk_fast(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexNextStart->first, pOrderedIndexNextEnd->last, n, &pInterChunkTemp);
                            else if (2 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                                libtcas_generate_intermediate_frame_chunk(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexNextStart->first, pOrderedIndexNextEnd->last, n, &pInterChunkTemp);
                            if (pChunksCache->pChunks[i].pos_and_color)
                                free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                            memcpy(&pChunksCache->pChunks[i], pInterChunkTemp, sizeof(TCAS_Chunk));
                            free(pInterChunkTemp);  /* Note that pInterChunkTemp is dynamically allocated */
                            pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexNextStart->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                            pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
                            pChunksCache->pi[i] += 2;
                            found = TCAS_TRUE;
                            continue;  /* both pi[i] and pChunkPairs[i] have been updated */
                        } else if (pOrderedIndexLastEnd->last < n && n < pOrderedIndexNextStart->first) {
                            if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
                            }
                            continue;  /* we now know that there is no effect of this nth frame in the stream, note that pi[i] stays unchanged */
                        }
                    }
                }
            }
        }
        if (TCAS_FALSE == found) {  /* we cannot find the index easily (maybe the video is moved forward or backward), so we use binary search to try again */
            long result;
            TCAS_OrderedIndex key;
            key.first = n;
            key.last = n;
            result = vector_binary_search_interval(linearIndexsTemp, &key, (vector_compare_interval_func)libtcas_check_key_index_validity);
            if (-1 != result) {
                pChunksCache->pi[i] = result;
                pOrderedIndexLastStart = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);
                pOrderedIndexLastEnd = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 1, NULL);
                if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                    free(pChunksCache->pChunkPairs[i].start.pos_and_color);
                    free(pChunksCache->pChunkPairs[i].end.pos_and_color);
                }
                libtcas_read_specified_chunk_z(pFile, ((tcas_u64)pOrderedIndexLastStart->offset) << 2, &pChunksCache->pChunkPairs[i].start);  /* cover the last one */
                libtcas_read_specified_chunk_z(pFile, ((tcas_u64)pOrderedIndexLastEnd->offset) << 2, &pChunksCache->pChunkPairs[i].end);  /* cover the last one */
                libtcas_key_frame_chunk_add_dips(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end);
                if (1 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                    libtcas_generate_intermediate_frame_chunk_fast(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexLastStart->first, pOrderedIndexLastEnd->last, n, &pInterChunkTemp);
                else if (2 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                    libtcas_generate_intermediate_frame_chunk(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexLastStart->first, pOrderedIndexLastEnd->last, n, &pInterChunkTemp);
                if (pChunksCache->pChunks[i].pos_and_color)
                    free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                memcpy(&pChunksCache->pChunks[i], pInterChunkTemp, sizeof(TCAS_Chunk));
                free(pInterChunkTemp);  /* Note that pInterChunkTemp is dynamically allocated */
                pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexLastStart->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
                found = TCAS_TRUE;
                continue;
            }
        }
        if (TCAS_FALSE == found) {  /* if still does not find the index */
            if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
            }
        }
    }
}

/* the first time you pass the pChunksCache to this function should set pChunksCache->streamCount = 0 */
TCAS_Error_Code libtcas_create_frame_with_linearized_index_z(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_IndexStreamsPtr indexStreams, tcas_s32 n, TCAS_pChunksCache pChunksCache, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf) {
    tcas_bool keyframing;
    tcas_u16 width, height;
    tcas_u32 i, pitch, size;
    tcas_byte *rgba;
    VectorPtr chunksVector;  /* a vector to hold the effective chunks */
    tcas_u32 chunkCount;  /* number of effective chunks */
    TCAS_pChunk pChunkTemp;
    keyframing = GETLOW16B(pHeader->flag);
    width = GETPOSX(pHeader->resolution);
    height = GETPOSY(pHeader->resolution);
    pitch = width * (sizeof(tcas_byte) << 2);
    size = height * pitch;
    rgba = (tcas_byte *)malloc(size);  /* create frame buffer */
    memset(rgba, 0, size);  /* we must clear the bits of the frame buffer */
    vector_create(&chunksVector, sizeof(TCAS_Chunk), 0, NULL, NULL);  /* note that we use low copy and low cleanup to increase performance */
    _libtcas_parse_linearized_index_streams_normal_z(pFile, indexStreams, n, pChunksCache);
    if (keyframing)
        _libtcas_parse_linearized_index_streams_key_z(pFile, indexStreams, n, pChunksCache);
    for (i = 0; i < pChunksCache->streamCount; i++) {  /* check every stream */
        if (pChunksCache->pChunks[i].pos_and_color)  /* the ith chunk is effective */
            vector_push_back(chunksVector, &pChunksCache->pChunks[i]);
    }
    chunkCount = vector_get_count(chunksVector);
    vector_quicksort(chunksVector, 0, chunkCount - 1, (vector_compare_func)libtcas_compare_order_layer);
    for (i = 0; i < chunkCount; i++) {
        pChunkTemp = (TCAS_pChunk)vector_retrieve(chunksVector, i, NULL);
        libtcas_convert_chunks_to_rgba(pChunkTemp, width, height, rgba);
    }
    vector_destroy(chunksVector);
    if (!(width == targetWidth && height == targetHeight)) {
        tcas_byte *src = rgba;
        rgba = (tcas_byte *)malloc(targetHeight * targetWidth * (sizeof(tcas_byte) << 2));
        libtcas_resample_rgba_bilinear_mmx(src, width, height, rgba, targetWidth, targetHeight);
        free(src);
    }
    *pBuf = rgba;
    return tcas_error_success;
}

/* the first time you pass the pChunksCache to this function should set pChunksCache->streamCount = 0 */
TCAS_Error_Code libtcas_fill_frame_with_linearized_index_z(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_IndexStreamsPtr indexStreams, tcas_s32 n, TCAS_pChunksCache pChunksCache, tcas_byte *buf, tcas_u16 width, tcas_u16 height) {
    tcas_bool keyframing;
    tcas_u32 i;
    VectorPtr chunksVector;  /* a vector to hold the effective chunks */
    tcas_u32 chunkCount;  /* number of effective chunks */
    TCAS_pChunk pChunkTemp;
    keyframing = GETLOW16B(pHeader->flag);
    vector_create(&chunksVector, sizeof(TCAS_Chunk), 0, NULL, NULL);  /* note that we use low copy and low cleanup to increase performance */
    _libtcas_parse_linearized_index_streams_normal_z(pFile, indexStreams, n, pChunksCache);
    if (keyframing)
        _libtcas_parse_linearized_index_streams_key_z(pFile, indexStreams, n, pChunksCache);
    for (i = 0; i < pChunksCache->streamCount; i++) {  /* check every stream */
        if (pChunksCache->pChunks[i].pos_and_color)  /* the ith chunk is effective */
            vector_push_back(chunksVector, &pChunksCache->pChunks[i]);
    }
    chunkCount = vector_get_count(chunksVector);
    vector_quicksort(chunksVector, 0, chunkCount - 1, (vector_compare_func)libtcas_compare_order_layer);
    for (i = 0; i < chunkCount; i++) {
        pChunkTemp = (TCAS_pChunk)vector_retrieve(chunksVector, i, NULL);
        libtcas_convert_chunks_to_rgba(pChunkTemp, width, height, buf);
    }
    vector_destroy(chunksVector);
    return tcas_error_success;
}

TCAS_Error_Code libtcas_create_frame_with_parsed_index_z(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_pCompactedIndex pParsedIndexs, tcas_u32 indexCount, tcas_s32 n, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf) {
    tcas_u16 width, height;
    tcas_u32 pitch, size;
    tcas_byte *rgba;
    long result;
    TCAS_CompactedIndex key;
    TCAS_Chunk chunk;
    width = GETPOSX(pHeader->resolution);
    height = GETPOSY(pHeader->resolution);
    pitch = width * (sizeof(tcas_byte) << 2);
    size = height * pitch;
    rgba = (tcas_byte *)malloc(size);  /* create frame buffer */
    memset(rgba, 0, size);  /* we must clear the bits of the frame buffer */
    key.first = n;
    key.last = n;
    result = libtcas_chunk_index_binary_search(pParsedIndexs, indexCount, &key);
    if (result != -1) {
        libtcas_read_specified_chunk_z(pFile, ((tcas_u64)pParsedIndexs[result].offset) << 2, &chunk);
        libtcas_convert_chunks_to_rgba(&chunk, width, height, rgba);
        free(chunk.pos_and_color);
    }
    if (!(width == targetWidth && height == targetHeight)) {
        tcas_byte *src = rgba;
        rgba = (tcas_byte *)malloc(targetHeight * targetWidth * (sizeof(tcas_byte) << 2));
        libtcas_resample_rgba_bilinear_mmx(src, width, height, rgba, targetWidth, targetHeight);
        free(src);
    }
    *pBuf = rgba;
    return tcas_error_success;
}

