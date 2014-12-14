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

#include "hla_frame_cache.h"


static TCAS_Error_Code _libtcas_frame_cache_proc_args_init(const char *filename, tcas_u32 *pFpsNumerator, tcas_u32 *pFpsDenominator, tcas_u16 width, tcas_u16 height, TCAS_pFrameCacheProcArgs pArgs) {
    TCAS_Error_Code error;
    TCAS_pOrderedIndex pOrderedIndexs;
    error = libtcas_open_file(&pArgs->file, filename, tcas_file_open_existing);
    if (tcas_error_success != error) {
        printf("Error: can not open the TCAS file.\n");
        return error;
    }
    error = libtcas_read_header(&pArgs->file, &pArgs->header, 0);
    if (tcas_error_success != error) {
        libtcas_close_file(&pArgs->file);
        printf("Error: can not read the TCAS file.\n");
        return error;
    }
    if ((tcas_u32)(-1) == *pFpsNumerator && 1 == *pFpsDenominator) {
        *pFpsNumerator = pArgs->header.fpsNumerator;
        *pFpsDenominator = pArgs->header.fpsDenominator;
    }
    if (TCAS_FILE_TYPE_COMPRESSED == GETHI16B(pArgs->header.flag)) {
        if (0 == pArgs->header.chunks)
            libtcas_get_min_max_time_and_chunks(&pArgs->file, &pArgs->header.minTime, &pArgs->header.maxTime, &pArgs->header.chunks);
        error = libtcas_create_ordered_index(&pArgs->file, &pArgs->header, *pFpsNumerator, *pFpsDenominator, &pOrderedIndexs, NULL);
        if (tcas_error_success != error) {
            libtcas_close_file(&pArgs->file);
            printf("Error: can not parse the TCAS file, type1.\n");
            return error;
        }
        pArgs->indexStreams = libtcas_linearize_ordered_indexs(pOrderedIndexs, pArgs->header.chunks, NULL);
        free(pOrderedIndexs);
        libtcas_init_chunks_cache(&pArgs->chunksCache, vector_get_count(pArgs->indexStreams));
        pArgs->pParsedIndexs = NULL;
    } else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(pArgs->header.flag)) {
        if (0 == pArgs->header.chunks)
            libtcas_get_min_max_time_and_chunks_z(&pArgs->file, &pArgs->header.minTime, &pArgs->header.maxTime, &pArgs->header.chunks);
        error = libtcas_create_ordered_index_z(&pArgs->file, &pArgs->header, *pFpsNumerator, *pFpsDenominator, &pOrderedIndexs, NULL);
        if (tcas_error_success != error) {
            libtcas_close_file(&pArgs->file);
            printf("Error: can not parse the TCAS file, type2.\n");
            return error;
        }
        pArgs->indexStreams = libtcas_linearize_ordered_indexs(pOrderedIndexs, pArgs->header.chunks, NULL);
        free(pOrderedIndexs);
        libtcas_init_chunks_cache(&pArgs->chunksCache, vector_get_count(pArgs->indexStreams));
        pArgs->pParsedIndexs = NULL;
    } else if (TCAS_FILE_TYPE_PARSED == GETHI16B(pArgs->header.flag)) {
        if (0 == pArgs->header.chunks)
            libtcas_get_min_max_time_and_chunks(&pArgs->file, &pArgs->header.minTime, &pArgs->header.maxTime, &pArgs->header.chunks);
        error = libtcas_index_parsed_tcas_file(&pArgs->file, &pArgs->header, *pFpsNumerator, *pFpsDenominator, &pArgs->pParsedIndexs, NULL);
        if (tcas_error_success != error) {
            libtcas_close_file(&pArgs->file);
            printf("Error: can not parse the TCAS file, type3.\n");
            return error;
        }
        pArgs->indexStreams = NULL;
        pArgs->chunksCache.streamCount = 0;
    } else if (TCAS_FILE_TYPE_PARSED_Z == GETHI16B(pArgs->header.flag)) {
        if (0 == pArgs->header.chunks)
            libtcas_get_min_max_time_and_chunks_z(&pArgs->file, &pArgs->header.minTime, &pArgs->header.maxTime, &pArgs->header.chunks);
        error = libtcas_index_parsed_tcas_file_z(&pArgs->file, &pArgs->header, *pFpsNumerator, *pFpsDenominator, &pArgs->pParsedIndexs, NULL);
        if (tcas_error_success != error) {
            libtcas_close_file(&pArgs->file);
            printf("Error: can not parse the TCAS file, type3.\n");
            return error;
        }
        pArgs->indexStreams = NULL;
        pArgs->chunksCache.streamCount = 0;
    } else {
        libtcas_close_file(&pArgs->file);
        printf("Error: tcasfilter does no support this TCAS file type yet.\n");
        return tcas_error_file_type_not_support;
    }
    pArgs->width = width;
    pArgs->height = height;
    return tcas_error_success;
}

DWORD WINAPI _libtcas_create_frame_with_linearized_index_worker_proc(LPVOID lpParam) {
    TCAS_pFrameCache pFrameCache;
    TCAS_pFrameCacheProcArgs pArgs;
    TCAS_QueuedFrame frame;
    pFrameCache = (TCAS_pFrameCache)lpParam;
    pArgs = &pFrameCache->fcpArgs;    /* a shortcut */
    while (pFrameCache->active) {
        WaitForSingleObject(pFrameCache->semQueue, INFINITE);    /* wait for room to store the frame */
        if (pFrameCache->active) {    /* still active */
            EnterCriticalSection(&pFrameCache->lock);
            if (queue_get_count(pFrameCache->qFrames) == 0)
                frame.id = pArgs->n;
            else {
                queue_retrieve_front(pFrameCache->qFrames, &frame);
                if (frame.id == pArgs->n)
                    frame.id += queue_get_count(pFrameCache->qFrames);
                else
                    frame.id = pArgs->n;
            }
            LeaveCriticalSection(&pFrameCache->lock);
            switch (GETHI16B(pArgs->header.flag)) {
            case TCAS_FILE_TYPE_COMPRESSED:
                libtcas_create_frame_with_linearized_index(&pArgs->file, &pArgs->header, pArgs->indexStreams, frame.id, &pArgs->chunksCache, pArgs->width, pArgs->height, &frame.buf);
                break;
            case TCAS_FILE_TYPE_COMPRESSED_Z:
                libtcas_create_frame_with_linearized_index_z(&pArgs->file, &pArgs->header, pArgs->indexStreams, frame.id, &pArgs->chunksCache, pArgs->width, pArgs->height, &frame.buf);
                break;
            case TCAS_FILE_TYPE_PARSED:
                libtcas_create_frame_with_parsed_index(&pArgs->file, &pArgs->header, pArgs->pParsedIndexs, pArgs->header.chunks, frame.id, pArgs->width, pArgs->height, &frame.buf);
                break;
            case TCAS_FILE_TYPE_PARSED_Z:
                libtcas_create_frame_with_parsed_index_z(&pArgs->file, &pArgs->header, pArgs->pParsedIndexs, pArgs->header.chunks, frame.id, pArgs->width, pArgs->height, &frame.buf);
                break;
            }
            EnterCriticalSection(&pFrameCache->lock);
            queue_push_rear(pFrameCache->qFrames, &frame);
            LeaveCriticalSection(&pFrameCache->lock);
            ReleaseSemaphore(pFrameCache->semFrames, 1, NULL);    /* add an available frame */
        }
    }
    return 0;
}

TCAS_Error_Code libtcas_frame_cache_init(const char *filename, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, tcas_u16 width, tcas_u16 height, int maxFrameCount, TCAS_pFrameCache pFrameCache) {
    TCAS_Error_Code error;
    error = _libtcas_frame_cache_proc_args_init(filename, &fpsNumerator, &fpsDenominator, width, height, &pFrameCache->fcpArgs);
    if (tcas_error_success != error)
        return error;
    pFrameCache->minFrame = libtcas_MulDiv(pFrameCache->fcpArgs.header.minTime, fpsNumerator, fpsDenominator * 1000) + 1;
    pFrameCache->maxFrame = libtcas_MulDiv(pFrameCache->fcpArgs.header.maxTime, fpsNumerator, fpsDenominator * 1000) + 1;
    pFrameCache->maxFrameCount = maxFrameCount;
    queue_create(&pFrameCache->qFrames, sizeof(TCAS_QueuedFrame), pFrameCache->maxFrameCount, NULL, NULL);    /* stores pointers to frame buffers */
    pFrameCache->semQueue = CreateSemaphore(NULL, 0, pFrameCache->maxFrameCount, NULL);
    pFrameCache->semFrames = CreateSemaphore(NULL, 0, pFrameCache->maxFrameCount, NULL);
    InitializeCriticalSection(&pFrameCache->lock);
    pFrameCache->active = 1;
    pFrameCache->tdWorker = CreateThread(NULL, 0, _libtcas_create_frame_with_linearized_index_worker_proc, pFrameCache, 0, &pFrameCache->threadID);
    return tcas_error_success;
}

void libtcas_frame_cache_run(TCAS_pFrameCache pFrameCache) {
    pFrameCache->fcpArgs.n = pFrameCache->minFrame;
    ReleaseSemaphore(pFrameCache->semQueue, pFrameCache->maxFrameCount, NULL);
}

int libtcas_frame_cache_get(TCAS_pFrameCache pFrameCache, tcas_s32 n, tcas_byte **pBuf) {
    TCAS_QueuedFrame frame;
    WaitForSingleObject(pFrameCache->semFrames, INFINITE);    /* wait for available frames */
    EnterCriticalSection(&pFrameCache->lock);
    queue_pop_front(pFrameCache->qFrames, &frame);
    if (frame.id == n) {    /* the returned frame is just what we want, i.e., the playback moves in a smooth progress */
        pFrameCache->fcpArgs.n = frame.id + 1;    /* require for next frames */
        LeaveCriticalSection(&pFrameCache->lock);
        *pBuf = frame.buf;
        ReleaseSemaphore(pFrameCache->semQueue, 1, NULL);    /* we consumed one frame, so there is an available room we left */
        return 1;
    } else {    /* we should drop all the frames cached, by calling this function in a loop */
        pFrameCache->fcpArgs.n = n;
        LeaveCriticalSection(&pFrameCache->lock);
        free(frame.buf);
        *pBuf = NULL;
        ReleaseSemaphore(pFrameCache->semQueue, 1, NULL);    /* we consumed one frame, so there is an available room we left */
        return 0;
    }
}

static void _libtcas_frame_cache_proc_args_fin(TCAS_pFrameCacheProcArgs pArgs) {
    if (pArgs->indexStreams)
        vector_destroy(pArgs->indexStreams);
    if (pArgs->chunksCache.streamCount > 0)
        libtcas_free_chunks_cache(&pArgs->chunksCache);
    if (pArgs->pParsedIndexs)
        free(pArgs->pParsedIndexs);
    libtcas_close_file(&pArgs->file);
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

void libtcas_frame_cache_fin(TCAS_pFrameCache pFrameCache) {
    pFrameCache->active = 0;
    ReleaseSemaphore(pFrameCache->semQueue, 1, NULL);   /* to let the thread quit */
    WaitForSingleObject(pFrameCache->tdWorker, INFINITE);
    CloseHandle(pFrameCache->tdWorker);
    _libtcas_frame_cache_proc_args_fin(&pFrameCache->fcpArgs);
    CloseHandle(pFrameCache->semFrames);
    CloseHandle(pFrameCache->semQueue);
    DeleteCriticalSection(&pFrameCache->lock);
    _libtcas_cleanup_queued_frames(pFrameCache->qFrames);
    queue_destroy(pFrameCache->qFrames);
}

