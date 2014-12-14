/*
 * hla_frame_cache.h -- High-level frame cache API of the 'libtcas' library
 * Copyright (C) 2011 milkyjing <milkyjing@gmail.com>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * milkyjing
 *
 */

#ifndef LIBTCAS_HLA_FRAME_CACHE_H
#define LIBTCAS_HLA_FRAME_CACHE_H
#pragma once

#include "tcas.h"
#include "queue.h"
#include "hla_z_comp.h"

#include <Windows.h>

#define TCAS_MAX_FRAME_CACHED 20

/**
 * a structure to hold the parameters which are going to be passed to the worker thread
 */
typedef struct _tcas_frame_cache_proc_args {
    TCAS_File file;
    TCAS_Header header;
    TCAS_IndexStreamsPtr indexStreams;
    tcas_s32 n;    /* should be specified during the rendering time, not in the initialization */
    TCAS_ChunksCache chunksCache;
    TCAS_pCompactedIndex pParsedIndexs;    /* for parsed tcas file */
    tcas_u16 width;
    tcas_u16 height;
} TCAS_FrameCacheProcArgs, *TCAS_pFrameCacheProcArgs;

/**
 * a structure to hold the frame id and pointer to its buffer
 */
typedef struct _tcas_queued_frame {
    int id;
    tcas_byte *buf;
} TCAS_QueuedFrame, *TCAS_pQueuedFrame;

/**
 * a structure to hold the basic information of the frame cache facility
 */
typedef struct _tcas_frame_cache {
    QueuePtr qFrames;       /**< a queue of frames being cached, the capacity of the queue is limitted to maxFrameCount */
    int maxFrameCount;      /**< maximum frames that the cache can hold */
    HANDLE semFrames;       /**< a semaphore indicates the available frames in the queue */
    HANDLE semQueue;        /**< a semaphore indicates the available room of the frames queue, the maximum count is limitted to maxFrameCount */
    CRITICAL_SECTION lock;  /**< lock for synchronization */
    HANDLE tdWorker;        /**< the handler to the worker thread */
    DWORD threadID;
    int active;
    TCAS_FrameCacheProcArgs fcpArgs;
    tcas_s32 minFrame;
    tcas_s32 maxFrame;
} TCAS_FrameCache, *TCAS_pFrameCache;


/* Inhibit C++ name-mangling for libtcas functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */
    
/**
 * Initialize the frame cache struct
 * @param filename specify the file name which is to be opened
 * @param fpsNumerator FramePerSecond = fpsNumerator / (double)fpsDenominator, can be 0, in this case, fps will not be used
 * @param fpsDenominator FramePerSecond = fpsNumerator / (double)fpsDenominator, can be 0, in this case, fps will not be used
 * @param width width of the target video
 * @param height height of the target video
 * @param maxFrameCount maximum frames that the cache queue can hold
 * @param pFrameCache a pointer to TCAS_FrameCache structure, which is going to be initialized
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_frame_cache_init(const char *filename, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, tcas_u16 width, tcas_u16 height, int maxFrameCount, TCAS_pFrameCache pFrameCache);

/**
 * Actually run the worker thread to generate frames into the queue
 * @param pFrameCache a pointer to TCAS_FrameCache structure, which should have been initialized
 */
extern void libtcas_frame_cache_run(TCAS_pFrameCache pFrameCache);

/**
 * Get the frame being cached in the queue.
 * Remark: in the main thread, we use this function to ask for the specific frame in the frame cache queue, 
 * during which the first frame will be returned, if the frame returned is not the frame specified, 
 * all the frames in the frame cache queue will be dropped, if they meet, only the first frame will be dropped. 
 * If there is no frames cached, the main thread has to wait for the coming of the very first frame. 
 * After one frame being retrieved, semQueue count will be increase by one.
 *
 * @param pFrameCache a pointer to TCAS_FrameCache structure, which should have been initialized
 * @param n frame index which is wanted
 * @param pBuf a pointer to the buffer which is going to hold the tcas frame data
 * @return 1 - specified frame returned, 0 - frames cached is not the one we wanted, hence should call this function again to get the specified frame
 */
extern int libtcas_frame_cache_get(TCAS_pFrameCache pFrameCache, tcas_s32 n, tcas_byte **pBuf);

/**
 * Finalize the frame cache facility.
 * @param pFrameCache a pointer to TCAS_FrameCache structure, which should have been initialized
 */
extern void libtcas_frame_cache_fin(TCAS_pFrameCache pFrameCache);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* LIBTCAS_HLA_FRAME_CACHE_H */

