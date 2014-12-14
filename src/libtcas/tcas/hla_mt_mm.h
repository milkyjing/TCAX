/*
 * hla_mt_mm.h -- High-level multi-threaded API with map file into memory support
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

#ifndef LIBTCAS_HLA_MT_MM_H
#define LIBTCAS_HLA_MT_MM_H
#pragma once

#include "tcas.h"
#include "threadpool.h"
#include "mapfile.h"

typedef unsigned long *tcas_p32;  /* 32 bits pointer type */


/* Inhibit C++ name-mangling for libtcas functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * Get the compressed chunk specified by the offset from file.
 * Remark: you should allocate memory for pChunk, but not for pChunk->pos_and_color. 
 *
 * @param fileMapBuf mapped file buffer
 * @param offset the offset of the chunk from the beginning of the file in tcas_unit unit, i.e. 32 bits 1 offset
 * @param pChunk a pointer to TCAS_Chunk structure that is going to hold chunk data
 */
extern void libtcas_get_specified_chunk(const tcas_unit *fileMapBuf, tcas_u32 offset, TCAS_pChunk pChunk);

/**
 * Create the ordered index of a compressed TCAS file using specified FPS. Using in-memory mapped file buffer support.
 * Remark: In this function, if fpsDenominator = 0, then TCAS_OrderedIndex.first means startTime and TCAS_OrderedIndex.last means endTime 
 * otherwise, TCAS_OrderedIndex.first means firstFrame and TCAS_OrderedIndex.last means lastFrame.
 * Note that, TCAS_OrderedIndex.id_pair = MAKEINDEXIP(0, 1) indicates invalid (no effect) chunks.
 *
 * @param fileMapBuf mapped file buffer
 * @param pHeader a pointer to TCAS_Header structure that holds the information of the header
 * @param fpsNumerator FramePerSecond = fpsNumerator / (double)fpsDenominator, can be 0, in this case, fps will not be used
 * @param fpsDenominator FramePerSecond = fpsNumerator / (double)fpsDenominator, can be 0, in this case, fps will not be used
 * @param ppOrderedIndexs a pointer to TCAS_pOrderedIndex that is going to hold the compressed TCAS Ordered Index
 * @param pOrderedIndexCount pointer to the amount of output ordered indexs, equal to chunks, can be NULL
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_create_ordered_index_mm(const tcas_unit *fileMapBuf, const TCAS_pHeader pHeader, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, TCAS_pOrderedIndex *ppOrderedIndexs, tcas_u32 *pOrderedIndexCount);

/**
 * Create a TCAS frame from compressed TCAS file with linearized index. Multi-threaded with in-memory mapped file support version
 * Remark: the function supports for compressed TCAS file. whether to use keyframing or not can be configured through flag, 
 * set LOWORD(flag) = 1 to enable keyframing, in this case key frame chunks will not be ignored, otherwise, key frame chunks will 
 * simply be ignored in order to increase performance. Note that, in this function TCAS_Chunk.startTime and TCAS_Chunk.endTime are 
 * not with their original meaning but change to store chunk id and chunk layer, their original task are taken by TCAS_OrderedIndex.first 
 * and TCAS_OrderedIndex.last
 * Note that, you should iniialize pChunksCache using libtcas_init_chunks_cache function, and do not forget to free the memory using 
 * libtcas_free_chunks_cache function after the rendering process.
 *
 * @param fileMapBuf mapped file buffer
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
extern TCAS_Error_Code libtcas_create_frame_with_linearized_index_mt_mm(tcas_unit *fileMapBuf, const TCAS_pHeader pHeader, const TCAS_IndexStreamsPtr indexStreams, tcas_u32 n, TCAS_pChunksCache pChunksCache, ThreadpoolPtr tp, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* LIBTCAS_HLA_MT_MM_H */

