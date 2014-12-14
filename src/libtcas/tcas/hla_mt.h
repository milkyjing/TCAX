/*
 *  hla_mt.h -- High-level multi-threaded API of the 'libtcas' library
 *
 *  Copyright (C) 2011 milkyjing <milkyjing@gmail.com>
 *
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

#ifndef LIBTCAS_HLA_MT_H
#define LIBTCAS_HLA_MT_H
#pragma once

#include "tcas.h"
#include "threadpool.h"


typedef unsigned long *tcas_p32;  /* 32 bits pointer type */


/* Inhibit C++ name-mangling for libtcas functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */
    
/**
 * Read the compressed chunk specified by the offset from file. Multi-threaded version.
 * Remark: you should allocate memory for pChunk, but not for pChunk->pos_and_color. 
 * Make sure that the file position indicator is just at the beginning of a chunk.
 * The file position indicator is set by offset from the beginning of the file.
 *
 * @param pFile a pointer to TCAS_File structure
 * @param offset the offset of the chunk from the beginning of the file
 * @param pMutex pointer to the global (shared) pthread_mutex_t variable, must be initialized outside the function
 * @param pChunk a pointer to TCAS_Chunk structure that is going to hold chunk data
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_read_specified_chunk_mt(TCAS_pFile pFile, tcas_u64 offset, pthread_mutex_t *pMutex, TCAS_pChunk pChunk);

/**
 * Create a TCAS frame from compressed TCAS file with linearized index. Multi-threaded version
 * Remark: the function supports for compressed TCAS file. whether to use keyframing or not can be configured through flag, 
 * set LOWORD(flag) = 1 to enable keyframing, in this case key frame chunks will not be ignored, otherwise, key frame chunks will 
 * simply be ignored in order to increase performance. Note that, in this function TCAS_Chunk.startTime and TCAS_Chunk.endTime are 
 * not with their original meaning but change to store chunk id and chunk layer, their original task are taken by TCAS_OrderedIndex.first 
 * and TCAS_OrderedIndex.last
 * Note that, you should iniialize pChunksCache using libtcas_init_chunks_cache function, and do not forget to free the memory using 
 * libtcas_free_chunks_cache function after the rendering process.
 *
 * @param pFile a pointer to TCAS_File structure
 * @param pHeader a pointer to TCAS_Header structure
 * @param indexStreams pointer of a Vector in which contains TCAS_LinearIndexsPtr that holds linear indexs
 * @param n specify which frame is going to create
 * @param pChunksCache pointer to TCAS_ChunksCache which caches the read chunks to increase performance
 * @param tp pointer to Threadpool which holds several threads (workers) to increase performance, must be created and initialized outside the function
 * @param targetWidth width of the target video
 * @param targetHeight height of the target video
 * @param pBuf a pointer to a block of memory which is going to hold the TCAS frame
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_create_frame_with_linearized_index_mt(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_IndexStreamsPtr indexStreams, tcas_u32 n, TCAS_pChunksCache pChunksCache, ThreadpoolPtr tp, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* LIBTCAS_HLA_MT_H */

