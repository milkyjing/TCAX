/*
 * hla_file_cache.h -- High-level file cache API of the 'libtcas' library
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

#ifndef LIBTCAS_HLA_FILE_CACHE_H
#define LIBTCAS_HLA_FILE_CACHE_H
#pragma once

#include "tcas.h"
#include "queue.h"

#include <Windows.h>

/**
 * a structure to hold the parameters which are going to be passed to the worker thread
 */
typedef struct _tcas_file_cache_proc_args {
    TCAS_pFile pFile;
    TCAS_IndexStreamsPtr indexStreams;
    tcas_u32 n;    /* should be specified during the rendering time, not in the initialization */
} TCAS_FileCacheProcArgs, *TCAS_pFileCacheProcArgs;

/**
 * a structure to hold the chunk offset in the file and pointer to its buffer
 */
typedef struct _tcas_queued_chunk {
    tcas_u32 offset;    /**< offset of the chunk in the file */
    TCAS_Chunk chunk;     /**< buffer of the chunk */
} TCAS_QueuedChunk, *TCAS_pQueuedChunk;

/**
 * a structure to hold the basic information of the file cache facility
 */
typedef struct _tcas_file_cache {
    QueuePtr qChunks;       /**< a queue of chunks being cached, the capacity of the queue is limitted to maxChunkCount */
    int maxChunkCount;      /**< maximum chunks that the cache can hold */
    HANDLE semChunks;       /**< a semaphore indicates the available chunks in the queue */
    HANDLE semQueue;        /**< a semaphore indicates the available room of the chunks queue, the maximum count is limitted to maxChunkCount */
    CRITICAL_SECTION lock;  /**< lock for synchronization */
    HANDLE tdWorker;        /**< the handler to the worker thread */
    DWORD threadID;
    int active;
    TCAS_FileCacheProcArgs fcpArgs;
    tcas_u32 minFrame;
    tcas_u32 maxFrame;
} TCAS_FileCache, *TCAS_pFileCache;


/* Inhibit C++ name-mangling for libtcas functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */
    
/**
 * Initialize the file cache struct
 * @param pFile the pointer to the TCAS_File structure, which is just a reference to the initialized instance
 * @param indexStreams the address of TCAS_OrderedIndex array which has been linearized
 * @param minFrame minimum frame index
 * @param maxFrame maximum frame index
 * @param maxChunkCount maximum chunks that the cache queue can hold
 * @param pFileCache a pointer to TCAS_FileCache structure, which is going to be initialized
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_file_cache_init(TCAS_pFile pFile, TCAS_IndexStreamsPtr indexStreams, tcas_u32 minFrame, tcas_u32 maxFrame, int maxChunkCount, TCAS_pFileCache pFileCache);

/**
 * Actually run the worker thread to pre-read chunks from the file into the queue
 * @param pFileCache a pointer to TCAS_FileCache structure, which should have been initialized
 */
extern void libtcas_file_cache_run(TCAS_pFileCache pFileCache);

/**
 * Get the chunk being cached in the queue.
 * Remark: in the main thread, we use this function to ask for the specific chunk in the chunk cache queue, 
 * during which the first chunk will be returned, if the chunk returned is not the chunk specified, 
 * all the chunks in the chunk cache queue will be dropped, if they meet, only the first chunk will be dropped. 
 * If there is no chunks cached, the main thread has to wait for the coming of the very first chunk. 
 * After one frame being retrieved, semQueue count will be increase by one.
 *
 * @param pFileCache a pointer to TCAS_FileCache structure, which should have been initialized
 * @param offset offset of the chunk which is wanted
 * @param ppChunk a pointer to TCAS_pChunk that is going to hold the frame chunk
 * @return 1 - specified chunk returned, 0 - chunks cached is not the one we wanted, hence should call this function again to get the specified chunk
 */
extern int libtcas_file_cache_get_chunk(TCAS_pFileCache pFileCache, tcas_u32 offset, TCAS_pChunk *ppChunk);

/**
 * Finalize the file cache facility.
 * @param pFileCache a pointer to TCAS_FileCache structure, which should have been initialized
 */
extern void libtcas_file_cache_fin(TCAS_pFileCache pFileCache);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* LIBTCAS_HLA_FILE_CACHE_H */

