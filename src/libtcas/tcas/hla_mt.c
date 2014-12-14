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

#include "hla_mt.h"


TCAS_Error_Code libtcas_read_specified_chunk_mt(TCAS_pFile pFile, tcas_u64 offset, pthread_mutex_t *pMutex, TCAS_pChunk pChunk) {
    tcas_u32 count;
    pthread_mutex_lock(pMutex);
    tcas_fseek(pFile->fp, offset, SEEK_SET);
    if (fread(pChunk, sizeof(tcas_unit), 3, pFile->fp) != 3) {    /* startTime endTime cltp takes up 3 tcas_unit */
        pthread_mutex_unlock(pMutex);
        return tcas_error_file_while_reading;
    }
    count = GETCOUNT(pChunk->cltp);
    pChunk->pos_and_color = (tcas_unit *)malloc(count * (sizeof(tcas_unit) << 1));    /* every pos_and_color takes up 2 tcas_unit */
    if (fread(pChunk->pos_and_color, sizeof(tcas_unit) << 1, count, pFile->fp) != count) {
        free(pChunk->pos_and_color);
        pthread_mutex_unlock(pMutex);
        return tcas_error_file_while_reading;
    }
    pthread_mutex_unlock(pMutex);
    return tcas_error_success;
}


/* libtcas_create_frame_with_linearized_index_mt function() */

static int _check_normal_index_validity(const TCAS_pOrderedIndex src, const TCAS_pOrderedIndex key) {
    if (src->last <= key->first) return -1;
    else if (src->first > key->last) return 1;
    else return 0;
}

static void *_work_parse_linearized_index_streams_normal_mt(void *args) {
    /* variables come directly from _libtcas_parse_linearized_index_streams_normal_mt function parameters */
    TCAS_pFile pFile;
    TCAS_IndexStreamsPtr indexStreams;
    tcas_u32 n;
    TCAS_pChunksCache pChunksCache;
    pthread_mutex_t *pMutex;
    /* other parameters */
    tcas_u32 i, indexCount;
    tcas_bool found;
    TCAS_LinearIndexsPtr linearIndexsTemp;
    TCAS_pOrderedIndex pOrderedIndexLast;  /* store the index that read at last time */
    TCAS_pOrderedIndex pOrderedIndexNext;  /* the index next to the last read one */
    pFile = (TCAS_pFile)((tcas_p32)args)[0];  /* get pFile from args */
    indexStreams = (TCAS_IndexStreamsPtr)((tcas_p32)args)[1];  /* get indexStreams from args */
    n = (tcas_u32)((tcas_p32)args)[2];  /* get n from args */
    pChunksCache = (TCAS_pChunksCache)((tcas_p32)args)[3];  /* get pChunksCache from args */
    pMutex = (pthread_mutex_t *)((tcas_p32)args)[4];  /* get pMutex from args */
    i = (tcas_u32)((tcas_p32)args)[5];  /* get i from args */
    vector_retrieve(indexStreams, i, &linearIndexsTemp);  /* get the ith index stream, low copy, do not free linearIndexsTemp */
    indexCount = vector_get_count(linearIndexsTemp);
    pOrderedIndexLast = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);  /* get the last ordered index of the ith linearized index stream */
    found = TCAS_FALSE;
    if (pOrderedIndexLast->first <= n && n < pOrderedIndexLast->last) {  /* last chunk is still valid */
        if (NULL == pChunksCache->pChunks[i].pos_and_color) {  /* not read yet, this may happen just after the initialization */
            libtcas_read_specified_chunk_mt(pFile, ((tcas_u64)pOrderedIndexLast->offset) << 2, pMutex, &pChunksCache->pChunks[i]);  /* otherwise, we do not have to read it again */
            pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexLast->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
            pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
        }
        found = TCAS_TRUE;
        return (void *)0;  /* pi[i] and pChunks[i] stays unchanged, they are still valid */
    } else {  /* the last chunk is invalid */
        if (pChunksCache->pi[i] + 1 < indexCount) {  /* there is still a next index */
            pOrderedIndexNext = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 1, NULL);  /* get the next index */
            if (pOrderedIndexNext->first <= n && n < pOrderedIndexNext->last) {  /* the next index is valid */
                if (pChunksCache->pChunks[i].pos_and_color)
                    free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                libtcas_read_specified_chunk_mt(pFile, ((tcas_u64)pOrderedIndexNext->offset) << 2, pMutex, &pChunksCache->pChunks[i]);  /* cover the last one */
                pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexNext->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
                pChunksCache->pi[i]++;  /* indicate that we have checked the next one */
                found = TCAS_TRUE;
                return (void *)0;  /* both pi[i] and pChunks[i] have been updated */
            } else if (pOrderedIndexLast->last <= n && n < pOrderedIndexNext->first) {  /* we now know that there is no effect of this nth frame in the stream */
                if (pChunksCache->pChunks[i].pos_and_color)
                    libtcas_free_chunk(&pChunksCache->pChunks[i]);  /* just erase it */
                return (void *)0;  /* note that pi[i] stays unchanged */
            }
        }
    }
    if (TCAS_FALSE == found) {  /* we cannot find the index easily (maybe the video is moved forward or backward), so we use binary search to try again */
        long result;
        TCAS_OrderedIndex key;
        key.first = n;
        key.last = n;
        result = vector_binary_search(linearIndexsTemp, &key, (vector_compare_func)_check_normal_index_validity, NULL);
        if (-1 != result) {  /* we find it */
            pChunksCache->pi[i] = result;
            pOrderedIndexLast = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);
            if (pChunksCache->pChunks[i].pos_and_color)
                free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
            libtcas_read_specified_chunk_mt(pFile, ((tcas_u64)pOrderedIndexLast->offset) << 2, pMutex, &pChunksCache->pChunks[i]);  /* cover the last one */
            pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexLast->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
            pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
            found = TCAS_TRUE;
            return (void *)0;
        }
    }
    if (TCAS_FALSE == found) {  /* if still does not find the index */
        if (pChunksCache->pChunks[i].pos_and_color)
            libtcas_free_chunk(&pChunksCache->pChunks[i]);  /* just erase it */
    }
    return (void *)0;
}

static void _libtcas_parse_linearized_index_streams_normal_mt(TCAS_pFile pFile, const TCAS_IndexStreamsPtr indexStreams, tcas_u32 n, TCAS_pChunksCache pChunksCache, ThreadpoolPtr tp, pthread_mutex_t *pMutex) {
    tcas_u32 i, size, count;
    ThreadpoolWork work;
    count = vector_get_count(indexStreams);
    size = 6 * sizeof(tcas_p32);  /* there are 6 parameters */
    work.execution_route = (tpw_func)_work_parse_linearized_index_streams_normal_mt;
    work.args_deepcopy = 1;  /* set args_deepcopy as true */
    for (i = 0; i < count; i++) {    /* check every index stream */
        work.args = malloc(size);
        ((tcas_p32 *)work.args)[0] = (tcas_p32)pFile;
        ((tcas_p32 *)work.args)[1] = (tcas_p32)indexStreams;
        ((tcas_p32 *)work.args)[2] = (tcas_p32)n;
        ((tcas_p32 *)work.args)[3] = (tcas_p32)pChunksCache;
        ((tcas_p32 *)work.args)[4] = (tcas_p32)pMutex;
        ((tcas_p32 *)work.args)[5] = (tcas_p32)i;
        tp_assign_work(tp, &work);
    }
    tp_wait(tp);
}

/* will check every two elements */
static int _check_key_index_validity(const TCAS_pOrderedIndex start, const TCAS_pOrderedIndex end, const TCAS_pOrderedIndex key) {
    if (start->first == start->last && end->first == end->last && 0 != GETINDEXID(start->id_pair) && 0 == GETINDEXPAIR(start->id_pair) && 0 != GETINDEXID(end->id_pair) && 1 == GETINDEXPAIR(end->id_pair)) {  /* a key frame chunk pair */
        if (start->first > key->last) return 1;  /* key is on the left of the interval */
        else if (end->last <= key->first) return -1;  /* key is on the right of the interval */
        else return 0;  /* key is in the interval */
    } else {
        if (start->first > key->last) return 1;  /* in fact we can use any other three expressions, like start->last > key->last, etc. */
        else return -1;
    }
}

static void *_work_parse_linearized_index_streams_key_mt(void *args) {
    /* variables come directly from _libtcas_parse_linearized_index_streams_normal_mt function parameters */
    TCAS_pFile pFile;
    TCAS_IndexStreamsPtr indexStreams;
    tcas_u32 n;
    TCAS_pChunksCache pChunksCache;
    pthread_mutex_t *pMutex;
    /* other parameters */
    tcas_u32 i, indexCount;
    tcas_bool found;
    TCAS_pChunk pInterChunkTemp;
    TCAS_LinearIndexsPtr linearIndexsTemp;
    TCAS_pOrderedIndex pOrderedIndexLastStart;  /* store the index that read at last time */
    TCAS_pOrderedIndex pOrderedIndexLastEnd;
    TCAS_pOrderedIndex pOrderedIndexNextStart;  /* the index next to the last read one */
    TCAS_pOrderedIndex pOrderedIndexNextEnd;
    pFile = (TCAS_pFile)((tcas_p32)args)[0];  /* get pFile from args */
    indexStreams = (TCAS_IndexStreamsPtr)((tcas_p32)args)[1];  /* get indexStreams from args */
    n = (tcas_u32)((tcas_p32)args)[2];  /* get n from args */
    pChunksCache = (TCAS_pChunksCache)((tcas_p32)args)[3];  /* get pChunksCache from args */
    pMutex = (pthread_mutex_t *)((tcas_p32)args)[4];  /* get pMutex from args */
    i = (tcas_u32)((tcas_p32)args)[5];  /* get i from args */
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
            return (void *)0;
        }
        pOrderedIndexLastEnd = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 1, NULL);  /* then, it must have a next chunk, because the index is always indicating the start key frame chunk */
        if (!(pOrderedIndexLastEnd->first == pOrderedIndexLastEnd->last && 0 != GETINDEXID(pOrderedIndexLastEnd->id_pair) && 1 == GETINDEXPAIR(pOrderedIndexLastEnd->id_pair))) {  /* invalid end key frame chunk, this seldom happens, unless there is an error causing the lost of the end key frame chunk */
            if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
            }
            return (void *)0;
        }
        if (pOrderedIndexLastStart->first <= n && n < pOrderedIndexLastEnd->last) {  /* last chunk is still valid, note that, the last frame cannot be reached in order to be compatible with normal chunk frames */
            if (NULL == pChunksCache->pChunkPairs[i].start.pos_and_color) {  /* not read yet, this may happen just after the initialization, it is impossible that only one pos_and_color is NULL, must be both or neither */
                libtcas_read_specified_chunk_mt(pFile, ((tcas_u64)pOrderedIndexLastStart->offset) << 2, pMutex, &pChunksCache->pChunkPairs[i].start);  /* otherwise, we do not have to read it again */
                libtcas_read_specified_chunk_mt(pFile, ((tcas_u64)pOrderedIndexLastEnd->offset) << 2, pMutex, &pChunksCache->pChunkPairs[i].end);
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
            return (void *)0;  /* pi[i] and pChunkPairs[i] stays unchanged, they are still valid */
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
                        return (void *)0;
                    }
                    if (pOrderedIndexNextStart->first <= n && n < pOrderedIndexNextEnd->last) {  /* last chunk is still valid, note that, the last frame cannot be reached in order to be compatible with normal chunk frames */
                        if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                            free(pChunksCache->pChunkPairs[i].start.pos_and_color);
                            free(pChunksCache->pChunkPairs[i].end.pos_and_color);
                        }
                        libtcas_read_specified_chunk_mt(pFile, ((tcas_u64)pOrderedIndexNextStart->offset) << 2, pMutex, &pChunksCache->pChunkPairs[i].start);
                        libtcas_read_specified_chunk_mt(pFile, ((tcas_u64)pOrderedIndexNextEnd->offset) << 2, pMutex, &pChunksCache->pChunkPairs[i].end);
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
                        return (void *)0;  /* both pi[i] and pChunkPairs[i] have been updated */
                    } else if (pOrderedIndexLastEnd->last < n && n < pOrderedIndexNextStart->first) {
                        if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                            libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                            libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
                        }
                        return (void *)0;  /* we now know that there is no effect of this nth frame in the stream, note that pi[i] stays unchanged */
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
        result = vector_binary_search_interval(linearIndexsTemp, &key, (vector_compare_interval_func)_check_key_index_validity);
        if (-1 != result) {
            pChunksCache->pi[i] = result;
            pOrderedIndexLastStart = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);
            pOrderedIndexLastEnd = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 1, NULL);
            if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                free(pChunksCache->pChunkPairs[i].start.pos_and_color);
                free(pChunksCache->pChunkPairs[i].end.pos_and_color);
            }
            libtcas_read_specified_chunk_mt(pFile, ((tcas_u64)pOrderedIndexLastStart->offset) << 2, pMutex, &pChunksCache->pChunkPairs[i].start);  /* cover the last one */
            libtcas_read_specified_chunk_mt(pFile, ((tcas_u64)pOrderedIndexLastEnd->offset) << 2, pMutex, &pChunksCache->pChunkPairs[i].end);  /* cover the last one */
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
            return (void *)0;
        }
    }
    if (TCAS_FALSE == found) {  /* if still does not find the index */
        if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
            libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
            libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
        }
    }
    return (void *)0;
}

/* the function only cares about key frame chunks */
static void _libtcas_parse_linearized_index_streams_key_mt(TCAS_pFile pFile, const TCAS_IndexStreamsPtr indexStreams, tcas_u32 n, TCAS_pChunksCache pChunksCache, ThreadpoolPtr tp, pthread_mutex_t *pMutex) {
    tcas_u32 i, size, count;
    ThreadpoolWork work;
    count = vector_get_count(indexStreams);
    size = 6 * sizeof(tcas_p32);  /* there are 6 parameters */
    work.execution_route = (tpw_func)_work_parse_linearized_index_streams_key_mt;
    work.args_deepcopy = 1;  /* set args_deepcopy as true */
    for (i = 0; i < count; i++) {    /* check every index stream */
        work.args = malloc(size);
        ((tcas_p32 *)work.args)[0] = (tcas_p32)pFile;
        ((tcas_p32 *)work.args)[1] = (tcas_p32)indexStreams;
        ((tcas_p32 *)work.args)[2] = (tcas_p32)n;
        ((tcas_p32 *)work.args)[3] = (tcas_p32)pChunksCache;
        ((tcas_p32 *)work.args)[4] = (tcas_p32)pMutex;
        ((tcas_p32 *)work.args)[5] = (tcas_p32)i;
        tp_assign_work(tp, &work);
    }
    tp_wait(tp);
}

static int _compare_order_layer(const TCAS_pChunk src, const TCAS_pChunk key) {
    if (src->endTime > key->endTime) return 1;  /* compare layer, in fact endTime here contains layer info, since with the help of indexs we no longer use startTime and endTime data members we can use them to contain some other helpful information */
    else if (src->endTime < key->endTime) return -1;
    else {
        if (src->startTime > key->startTime) return 1;  /* compare id, note that id can not be the same */
        else return -1;
    }
}

/* the first time you pass the pChunksCache to this function should set pChunksCache->streamCount = 0 */
TCAS_Error_Code libtcas_create_frame_with_linearized_index_mt(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_IndexStreamsPtr indexStreams, tcas_u32 n, TCAS_pChunksCache pChunksCache, ThreadpoolPtr tp, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf) {
    pthread_mutex_t lock;
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
    pthread_mutex_init(&lock, NULL);
    _libtcas_parse_linearized_index_streams_normal_mt(pFile, indexStreams, n, pChunksCache, tp, &lock);
    if (keyframing)
        _libtcas_parse_linearized_index_streams_key_mt(pFile, indexStreams, n, pChunksCache, tp, &lock);
    pthread_mutex_destroy(&lock);
    for (i = 0; i < pChunksCache->streamCount; i++) {  /* check every stream */
        if (pChunksCache->pChunks[i].pos_and_color)  /* the ith chunk is effective */
            vector_push_back(chunksVector, &pChunksCache->pChunks[i]);
    }
    chunkCount = vector_get_count(chunksVector);
    vector_quicksort(chunksVector, 0, chunkCount - 1, (vector_compare_func)_compare_order_layer);
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

