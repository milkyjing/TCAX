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

#include "hla_file_cache.h"


TCAS_Error_Code libtcas_init_file_cache(TCAS_pFileCache pFileCache, tcas_u32 size, const char *filename) {
    SYSTEM_INFO si;
    HANDLE hFile;
    DWORD fileSizeLow, fileSizeHigh;
    GetSystemInfo(&si);
    pFileCache->nGranul = si.dwAllocationGranularity;
    size += pFileCache->nGranul - size % pFileCache->nGranul;
    pFileCache->mapBuf = NULL;
    pFileCache->offset = TCAS_FILE_CACHE_INVALID_OFFSET;
    pFileCache->count = size >> 2;      /* size / sizeof(tcas_unit), size is sure to be divided by 4, it does not matter even if it cannot be divided */
    hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile) return tcas_error_file_cannot_open;
    fileSizeLow = GetFileSize(hFile, &fileSizeHigh);
    pFileCache->fileSize = MAKE32B32B(fileSizeLow, fileSizeHigh);
    pFileCache->hMapFile = CreateFileMappingA(hFile, NULL, PAGE_READONLY, fileSizeHigh, fileSizeLow, NULL);
    CloseHandle(hFile);
    if (!pFileCache->hMapFile) return tcas_error_file_cannot_open;
    return tcas_error_success;
}

void libtcas_fin_file_cache(TCAS_pFileCache pFileCache) {
    if (pFileCache->mapBuf)
        UnmapViewOfFile(pFileCache->mapBuf);
    if (pFileCache->hMapFile)
        CloseHandle(pFileCache->hMapFile);
}

static tcas_bool _libtcas_get_specified_chunk_from_cache(TCAS_pFileCache pFileCache, tcas_u64 offset, TCAS_pChunk pChunk) {
    tcas_u32 index, count, pcSize;
    if (pFileCache->offset <= offset && offset + 3 * sizeof(tcas_unit) <= pFileCache->offset + (pFileCache->count << 2)) {  /* thus, the chunk may be completely in the buffer */
        index = (tcas_u32)((offset - pFileCache->offset) >> 2);
        count = GETCOUNT(pFileCache->mapBuf[index + 2]);
        pcSize = count * (sizeof(tcas_unit) << 1);      /* size of pos_and_color buffer, every pos_and_color takes up 2 tcas_unit */
        if (offset + 3 * sizeof(tcas_unit) + pcSize <= pFileCache->offset + (pFileCache->count << 2)) {      /* now we are sure the chunk is completely in the buffer */
            memcpy(pChunk, pFileCache->mapBuf + index, 3 * sizeof(tcas_unit));    /* startTime endTime cltp takes up 3 tcas_unit */
            pChunk->pos_and_color = (tcas_unit *)malloc(pcSize);
            memcpy(pChunk->pos_and_color, pFileCache->mapBuf + index + 3, pcSize);
            return TCAS_TRUE;
        }
    }
    return TCAS_FALSE;
}

TCAS_Error_Code libtcas_read_specified_chunk_ex(TCAS_pFileCache pFileCache, tcas_u64 offset, TCAS_pChunk pChunk) {
    DWORD t1;
    t1 = GetTickCount();
    if (!_libtcas_get_specified_chunk_from_cache(pFileCache, offset, pChunk)) {
        tcas_u32 size;
        if (pFileCache->mapBuf)
            UnmapViewOfFile(pFileCache->mapBuf);
        pFileCache->offset = offset - offset % pFileCache->nGranul;
        size = pFileCache->count << 2;
        if (pFileCache->offset + size > pFileCache->fileSize)
            size = (tcas_u32)(pFileCache->fileSize - pFileCache->offset);
        pFileCache->mapBuf = (tcas_unit *)MapViewOfFile(pFileCache->hMapFile, FILE_MAP_READ, GETHI32B(pFileCache->offset), GETLOW32B(pFileCache->offset), size);
        if (!_libtcas_get_specified_chunk_from_cache(pFileCache, offset, pChunk)) return tcas_error_file_cache_too_small;
    }
    pFileCache->temp += GetTickCount() - t1;
    return tcas_error_success;
}

/* libtcas_create_frame_with_linearized_index_ex function() */

static int _check_normal_index_validity_ex(const TCAS_pOrderedIndex src, const TCAS_pOrderedIndex key) {
    if (src->last <= key->first) return -1;
    else if (src->first > key->last) return 1;
    else return 0;
}

/* the function only cares about normal chunks, the first time you pass the pChunksCache to this function should set pChunksCache->streamCount = 0 */
static void _libtcas_parse_linearized_index_streams_normal_ex(TCAS_pFileCache pFileCache, const TCAS_IndexStreamsPtr indexStreams, tcas_u32 n, TCAS_pChunksCache pChunksCache) {
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
                libtcas_read_specified_chunk_ex(pFileCache, ((tcas_u64)pOrderedIndexLast->offset) << 2, &pChunksCache->pChunks[i]);  /* otherwise, we do not have to read it again */
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
                    libtcas_read_specified_chunk_ex(pFileCache, ((tcas_u64)pOrderedIndexNext->offset) << 2, &pChunksCache->pChunks[i]);  /* cover the last one */
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
            result = vector_binary_search(linearIndexsTemp, &key, (vector_compare_func)_check_normal_index_validity_ex, NULL);
            if (-1 != result) {  /* we find it */
                pChunksCache->pi[i] = result;
                pOrderedIndexLast = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);
                if (pChunksCache->pChunks[i].pos_and_color)
                    free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                libtcas_read_specified_chunk_ex(pFileCache, ((tcas_u64)pOrderedIndexLast->offset) << 2, &pChunksCache->pChunks[i]);  /* cover the last one */
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

/* will check every two elements */
static int _check_key_index_validity_ex(const TCAS_pOrderedIndex start, const TCAS_pOrderedIndex end, const TCAS_pOrderedIndex key) {
    if (start->first == start->last && end->first == end->last && 0 != GETINDEXID(start->id_pair) && 0 == GETINDEXPAIR(start->id_pair) && 0 != GETINDEXID(end->id_pair) && 1 == GETINDEXPAIR(end->id_pair)) {  /* a key frame chunk pair */
        if (start->first > key->last) return 1;  /* key is on the left of the interval */
        else if (end->last <= key->first) return -1;  /* key is on the right of the interval */
        else return 0;  /* key is in the interval */
    } else {
        if (start->first > key->last) return 1;  /* in fact we can use any other three expressions, like start->last > key->last, etc. */
        else return -1;
    }
}

/* the function only cares about key frame chunks, the first time you pass the pChunksCache to this function should set pChunksCache->streamCount = 0 */
static void _libtcas_parse_linearized_index_streams_key_ex(TCAS_pFileCache pFileCache, const TCAS_IndexStreamsPtr indexStreams, tcas_u32 n, TCAS_pChunksCache pChunksCache) {
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
                    libtcas_read_specified_chunk_ex(pFileCache, ((tcas_u64)pOrderedIndexLastStart->offset) << 2, &pChunksCache->pChunkPairs[i].start);  /* otherwise, we do not have to read it again */
                    libtcas_read_specified_chunk_ex(pFileCache, ((tcas_u64)pOrderedIndexLastEnd->offset) << 2, &pChunksCache->pChunkPairs[i].end);
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
                            libtcas_read_specified_chunk_ex(pFileCache, ((tcas_u64)pOrderedIndexNextStart->offset) << 2, &pChunksCache->pChunkPairs[i].start);
                            libtcas_read_specified_chunk_ex(pFileCache, ((tcas_u64)pOrderedIndexNextEnd->offset) << 2, &pChunksCache->pChunkPairs[i].end);
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
            result = vector_binary_search_interval(linearIndexsTemp, &key, (vector_compare_interval_func)_check_key_index_validity_ex);
            if (-1 != result) {
                pChunksCache->pi[i] = result;
                pOrderedIndexLastStart = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);
                pOrderedIndexLastEnd = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 1, NULL);
                if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                    free(pChunksCache->pChunkPairs[i].start.pos_and_color);
                    free(pChunksCache->pChunkPairs[i].end.pos_and_color);
                }
                libtcas_read_specified_chunk_ex(pFileCache, ((tcas_u64)pOrderedIndexLastStart->offset) << 2, &pChunksCache->pChunkPairs[i].start);  /* cover the last one */
                libtcas_read_specified_chunk_ex(pFileCache, ((tcas_u64)pOrderedIndexLastEnd->offset) << 2, &pChunksCache->pChunkPairs[i].end);  /* cover the last one */
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

static int _compare_order_layer_ex(const TCAS_pChunk src, const TCAS_pChunk key) {
    if (src->endTime > key->endTime) return 1;  /* compare layer, in fact endTime here contains layer info, since with the help of indexs we no longer use startTime and endTime data members we can use them to contain some other helpful information */
    else if (src->endTime < key->endTime) return -1;
    else {
        if (src->startTime > key->startTime) return 1;  /* compare id, note that id can not be the same */
        else return -1;
    }
}

/* the first time you pass the pChunksCache to this function should set pChunksCache->streamCount = 0 */
TCAS_Error_Code libtcas_create_frame_with_linearized_index_ex(TCAS_pFileCache pFileCache, const TCAS_pHeader pHeader, const TCAS_IndexStreamsPtr indexStreams, tcas_u32 n, TCAS_pChunksCache pChunksCache, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf) {
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
    vector_create(&chunksVector, sizeof(TCAS_Chunk), 0, NULL, NULL);  /* note that we use low copy and low leanup to increase performance */
    _libtcas_parse_linearized_index_streams_normal_ex(pFileCache, indexStreams, n, pChunksCache);
    if (keyframing)
        _libtcas_parse_linearized_index_streams_key_ex(pFileCache, indexStreams, n, pChunksCache);
    for (i = 0; i < pChunksCache->streamCount; i++) {  /* check every stream */
        if (pChunksCache->pChunks[i].pos_and_color)  /* the ith chunk is effective */
            vector_push_back(chunksVector, &pChunksCache->pChunks[i]);
    }
    chunkCount = vector_get_count(chunksVector);
    vector_quicksort(chunksVector, 0, chunkCount - 1, (vector_compare_func)_compare_order_layer_ex);
    for (i = 0; i < chunkCount; i++) {
        pChunkTemp = (TCAS_pChunk)vector_retrieve(chunksVector, i, NULL);
        libtcas_convert_chunks_to_rgba(pChunkTemp, width, height, rgba);
    }
    vector_destroy(chunksVector);
    if (!(width == targetWidth && height == targetHeight)) {
        tcas_byte *src = rgba;
        libtcas_resample_rgba(src, width, height, &rgba, targetWidth, targetHeight);
        free(src);
    }
    *pBuf = rgba;
    return tcas_error_success;
}

