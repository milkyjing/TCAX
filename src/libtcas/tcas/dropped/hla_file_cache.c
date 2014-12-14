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


static int _check_normal_index_validity(const TCAS_pOrderedIndex src, const TCAS_pOrderedIndex key) {
    if (src->last <= key->first) return -1;
    else if (src->first > key->last) return 1;
    else return 0;
}

DWORD WINAPI _libtcas_read_specified_chunk_worker_proc(LPVOID lpParam) {
    TCAS_pFileCache pFileCache;
    TCAS_pFileCacheProcArgs pArgs;
    tcas_u32 i, count;
    long result;
    TCAS_LinearIndexsPtr linearIndexsTemp;
    TCAS_OrderedIndex key;
    TCAS_pOrderedIndex pOrderedIndex;
    TCAS_QueuedChunk queuedChunk;
    pFileCache = (TCAS_pFileCache)lpParam;
    pArgs = &pFileCache->fcpArgs;    /* a shortcut */
    while (pFileCache->active) {
        WaitForSingleObject(pFileCache->semQueue, INFINITE);    /* wait for room to store the frame */
        if (pFileCache->active) {    /* still active */
            count = vector_get_count(pArgs->indexStreams);
            for (i = 0; i < count; i++) {    /* check every index stream */
                vector_retrieve(pArgs->indexStreams, i, &linearIndexsTemp);  /* get the ith index stream, low copy, do not free linearIndexsTemp */
                key.first = pArgs->n;
                key.last = pArgs->n;
                result = vector_binary_search(linearIndexsTemp, &key, (vector_compare_func)_check_normal_index_validity, NULL);
                if (-1 != result) {  /* we find it */
                    pOrderedIndex = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, result, NULL);
                    libtcas_read_specified_chunk(pArgs->pFile, ((tcas_s64)pOrderedIndex->offset) << 2, &queuedChunk.chunk);  /* cover the last one */
                }
            EnterCriticalSection(&pFileCache->lock);
            /*if (queue_get_count(pFileCache->qChunks) == 0)
                frame.id = pArgs->n;
            else {
                queue_retrieve_front(pFileCache->qFrames, &frame);
                if (frame.id == pArgs->n)
                    frame.id += queue_get_count(pFileCache->qFrames);
                else
                    frame.id = pArgs->n;
            }
            LeaveCriticalSection(&pFileCache->lock);
            libtcas_create_frame_with_linearized_index(&pArgs->file, &pArgs->header, pArgs->indexStreams, frame.id, &pArgs->chunksCache, pArgs->width, pArgs->height, &frame.buf);
            EnterCriticalSection(&pFileCache->lock);*/
            queue_push_rear(pFileCache->qChunks, &queuedChunk);
            LeaveCriticalSection(&pFileCache->lock);
            ReleaseSemaphore(pFileCache->semChunks, 1, NULL);    /* add an available chunk */
        }
    }
    return 0;
}

TCAS_Error_Code libtcas_file_cache_init(TCAS_pFile pFile, TCAS_IndexStreamsPtr indexStreams, tcas_u32 minFrame, tcas_u32 maxFrame, int maxChunkCount, TCAS_pFileCache pFileCache) {
    pFileCache->fcpArgs.pFile = pFile;
    pFileCache->fcpArgs.indexStreams = indexStreams;
    pFileCache->maxChunkCount = maxChunkCount;
    queue_create(&pFileCache->qChunks, sizeof(TCAS_QueuedChunk), pFileCache->maxChunkCount, NULL, NULL);    /* stores pointers to chunk buffers */
    pFileCache->semQueue = CreateSemaphore(NULL, 0, pFileCache->maxChunkCount, NULL);
    pFileCache->semChunks = CreateSemaphore(NULL, 0, pFileCache->maxChunkCount, NULL);
    InitializeCriticalSection(&pFileCache->lock);
    pFileCache->active = 1;
    pFileCache->tdWorker = CreateThread(NULL, 0, _libtcas_read_specified_chunk_worker_proc, pFileCache, 0, &pFileCache->threadID);
    return tcas_error_success;
}

void libtcas_file_cache_run(TCAS_pFileCache pFileCache) {
    pFileCache->fcpArgs.n = pFileCache->minFrame;
    ReleaseSemaphore(pFileCache->semQueue, pFileCache->maxChunkCount, NULL);
}

int libtcas_file_cache_get_chunk(TCAS_pFileCache pFileCache, tcas_u32 offset, TCAS_pChunk *ppChunk) {
    TCAS_QueuedChunk chunk;
    WaitForSingleObject(pFileCache->semChunks, INFINITE);    /* wait for available chunks */
    EnterCriticalSection(&pFileCache->lock);
    queue_pop_front(pFileCache->qChunks, &chunk);
    if (frame.id == n) {    /* the returned frame is just what we want, i.e., the playback moves in a smooth progress */
        pFileCache->fcpArgs.n = frame.id + 1;    /* require for next frames */
        LeaveCriticalSection(&pFileCache->lock);
        *pBuf = frame.buf;
        ReleaseSemaphore(pFileCache->semQueue, 1, NULL);    /* we consumed one frame, so there is an available room we left */
        return 1;
    } else {    /* we should drop all the frames cached, by calling this function in a loop */
        pFileCache->fcpArgs.n = n;
        LeaveCriticalSection(&pFileCache->lock);
        free(frame.buf);
        *pBuf = NULL;
        ReleaseSemaphore(pFileCache->semQueue, 1, NULL);    /* we consumed one frame, so there is an available room we left */
        return 0;
    }
}

void _libtcas_cleanup_queued_chunks(QueuePtr qChunks) {
    unsigned long i, index;
    TCAS_pQueuedChunk pQueuedChunk;
    for (i = 0; i < qChunks->count; i++) {
        index = (qChunks->head_position + i) % qChunks->capacity;
        pQueuedChunk = (TCAS_pQueuedChunk)((unsigned char *)qChunks->datap + index * qChunks->elementSize);
        free(pQueuedChunk->chunk.pos_and_color);
    }
}

void libtcas_file_cache_fin(TCAS_pFileCache pFileCache) {
    pFileCache->active = 0;
    ReleaseSemaphore(pFileCache->semQueue, 1, NULL);   /* to let the thread quit */
    WaitForSingleObject(pFileCache->tdWorker, INFINITE);
    CloseHandle(pFileCache->tdWorker);
    CloseHandle(pFileCache->semChunks);
    CloseHandle(pFileCache->semQueue);
    DeleteCriticalSection(&pFileCache->lock);
    _libtcas_cleanup_queued_chunks(pFileCache->qChunks);
    queue_destroy(pFileCache->qChunks);
}

