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

#include "hla_double_cache.h"


DWORD WINAPI _libtcas_create_frame_with_chunks_cached_worker_proc(LPVOID lpParam) {
    TCAS_pDoubleCache pDoubleCache;
    TCAS_pDoubleCacheProcArgs pArgs;
    TCAS_pFrameChunksCache pFcc;
    TCAS_QueuedFrame frame;
    VectorPtr vFrameChunks;
    pDoubleCache = (TCAS_pDoubleCache)lpParam;
    pArgs = &pDoubleCache->dcpArgs;    /* a shortcut */
    pFcc = &pArgs->fcc;     /* a shortcut */
    while (pDoubleCache->active) {
        WaitForSingleObject(pDoubleCache->semQueue, INFINITE);    /* wait for room to store the frame */
        if (pDoubleCache->active) {    /* still active */
            EnterCriticalSection(&pDoubleCache->lock);
            if (queue_get_count(pDoubleCache->qFrames) == 0)
                frame.id = pArgs->n;
            else {
                queue_retrieve_front(pDoubleCache->qFrames, &frame);
                if (frame.id == pArgs->n)
                    frame.id += queue_get_count(pDoubleCache->qFrames);
                else
                    frame.id = pArgs->n;
            }
            LeaveCriticalSection(&pDoubleCache->lock);
            while (!libtcas_frame_chunks_cache_get(pFcc, frame.id, &vFrameChunks))
                continue;
            libtcas_create_frame_with_chunks_cached(pFcc, vFrameChunks, pArgs->width, pArgs->height, &frame.buf);
            EnterCriticalSection(&pDoubleCache->lock);
            queue_push_rear(pDoubleCache->qFrames, &frame);
            LeaveCriticalSection(&pDoubleCache->lock);
            ReleaseSemaphore(pDoubleCache->semFrames, 1, NULL);    /* add an available frame */
        }
    }
    return 0;
}

TCAS_Error_Code libtcas_double_cache_init(const char *filename, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, tcas_u16 width, tcas_u16 height, int maxFrameCount, int maxFrameChunkCount, int fileCacheSize, TCAS_pDoubleCache pDoubleCache) {
    TCAS_Error_Code error;
    error = libtcas_frame_chunks_cache_init(filename, fpsNumerator, fpsDenominator, width, height, maxFrameChunkCount, fileCacheSize, &pDoubleCache->dcpArgs.fcc);
    if (tcas_error_success != error)
        return error;
    pDoubleCache->dcpArgs.width = width;
    pDoubleCache->dcpArgs.height = height;
    pDoubleCache->minFrame = (tcas_u32)((tcas_u64)pDoubleCache->dcpArgs.fcc.fccpArgs.header.minTime * fpsNumerator / (fpsDenominator * 1000)) + 1;
    pDoubleCache->maxFrame = (tcas_u32)((tcas_u64)pDoubleCache->dcpArgs.fcc.fccpArgs.header.maxTime * fpsNumerator / (fpsDenominator * 1000)) + 1;
    pDoubleCache->maxFrameCount = maxFrameCount;
    queue_create(&pDoubleCache->qFrames, sizeof(TCAS_QueuedFrame), pDoubleCache->maxFrameCount, NULL, NULL);    /* stores pointers to frame buffers */
    pDoubleCache->semQueue = CreateSemaphore(NULL, 0, pDoubleCache->maxFrameCount, NULL);
    pDoubleCache->semFrames = CreateSemaphore(NULL, 0, pDoubleCache->maxFrameCount, NULL);
    InitializeCriticalSection(&pDoubleCache->lock);
    pDoubleCache->active = 1;
    pDoubleCache->tdWorker = CreateThread(NULL, 0, _libtcas_create_frame_with_chunks_cached_worker_proc, pDoubleCache, 0, &pDoubleCache->threadID);
    return tcas_error_success;
}

void libtcas_double_cache_run(TCAS_pDoubleCache pDoubleCache) {
    pDoubleCache->dcpArgs.n = pDoubleCache->minFrame;
    libtcas_frame_chunks_cache_run(&pDoubleCache->dcpArgs.fcc);
    ReleaseSemaphore(pDoubleCache->semQueue, pDoubleCache->maxFrameCount, NULL);
}

int libtcas_double_cache_get(TCAS_pDoubleCache pDoubleCache, tcas_u32 n, tcas_byte **pBuf) {
    TCAS_QueuedFrame frame;
    WaitForSingleObject(pDoubleCache->semFrames, INFINITE);    /* wait for available frames */
    EnterCriticalSection(&pDoubleCache->lock);
    queue_pop_front(pDoubleCache->qFrames, &frame);
    if (frame.id == n) {    /* the returned frame is just what we want, i.e., the playback moves in a smooth progress */
        pDoubleCache->dcpArgs.n = frame.id + 1;    /* require for next frames */
        LeaveCriticalSection(&pDoubleCache->lock);
        *pBuf = frame.buf;
        ReleaseSemaphore(pDoubleCache->semQueue, 1, NULL);    /* we consumed one frame, so there is an available room we left */
        return 1;
    } else {    /* we should drop all the frames cached, by calling this function in a loop */
        pDoubleCache->dcpArgs.n = n;
        LeaveCriticalSection(&pDoubleCache->lock);
        free(frame.buf);
        *pBuf = NULL;
        ReleaseSemaphore(pDoubleCache->semQueue, 1, NULL);    /* we consumed one frame, so there is an available room we left */
        return 0;
    }
}

static void _libtcas_cleanup_queued_frames(QueuePtr qFrames) {
    unsigned long i, index;
    TCAS_pQueuedFrame pFrame;
    for (i = 0; i < qFrames->count; i++) {
        index = (qFrames->head_position + i) % qFrames->capacity;
        pFrame = (TCAS_pQueuedFrame)((unsigned char *)qFrames->datap + index * qFrames->elementSize);
        free(pFrame->buf);
    }
}

void libtcas_double_cache_fin(TCAS_pDoubleCache pDoubleCache) {
    pDoubleCache->active = 0;
    ReleaseSemaphore(pDoubleCache->semQueue, 1, NULL);   /* to let the thread quit */
    WaitForSingleObject(pDoubleCache->tdWorker, INFINITE);
    CloseHandle(pDoubleCache->tdWorker);
    libtcas_frame_chunks_cache_fin(&pDoubleCache->dcpArgs.fcc);
    CloseHandle(pDoubleCache->semFrames);
    CloseHandle(pDoubleCache->semQueue);
    DeleteCriticalSection(&pDoubleCache->lock);
    _libtcas_cleanup_queued_frames(pDoubleCache->qFrames);
    queue_destroy(pDoubleCache->qFrames);
}

