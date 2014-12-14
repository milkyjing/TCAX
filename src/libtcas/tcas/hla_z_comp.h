/*
 * hla_z_comp.h -- High-level zlib compression of pos_and_color API of the 'libtcas' library
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

#ifndef LIBTCAS_HLA_Z_COMP_H
#define LIBTCAS_HLA_Z_COMP_H
#pragma once

#include "tcas.h"
#include "./zlib/zlib.h"

#pragma comment(lib, "zlib.lib")


/* Inhibit C++ name-mangling for libtcas functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */
   
/**
 * init a inflate z stream
 */
extern tcas_bool libtcas_z_comp_inflate_init(z_stream *pStrmInf);

/**
 * init a deflate z stream
 */
extern tcas_bool libtcas_z_comp_deflate_init(z_stream *pStrmDef);

/**
 * Read the specified compressed chunk whose pos_and_color was compressed by zlib. 
 * Remark: you should allocate memory for pChunk, but not for pChunk->pos_and_color. 
 * Make sure that the file type is TCAS_FILE_TYPE_COMPRESSED_Z and the file position indicator is just at the beginning of a chunk.
 * The file position indicator is set by offset from the beginning of the file.
 *
 * @see libtcas_read_specified_chunk()
 * @param pFile a pointer to TCAS_File structure
 * @param offset the offset of the chunk from the beginning of the file
 * @param pChunk a pointer to TCAS_Chunk structure that is going to hold chunk data
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_read_specified_chunk_z(TCAS_pFile pFile, tcas_u64 offset, TCAS_pChunk pChunk);

/**
 * compress the pos_and_color to z_pos_and_color
 * @param pos_and_color pChunk->pos_and_color
 * @param count GETCOUNT(pChunk->cltp)
 * @return z_pos_and_color
 */
extern tcas_unit *libtcas_compress_pos_and_color(tcas_unit *pos_and_color, tcas_u32 count);

/**
 * Compress pos_and_color of compressed chunks using zlib, the z_pos_and_color buffer is padded to 
 * a multiple of tcas_unit (4 bytes), the new layout of pos_and_color is `zSize z_pos_and_color', 
 * zSize indicates the real size of the z_pos_and_color buffer, while in the physical file, the space 
 * that the z_pos_and_color takes will be a multiple of tcas_unit
 *
 * @see libtcas_compress_raw_chunks()
 * @param chunksBuf a stream of compressed chunks
 * @param chunkCount the amount of input compressed chunks
 * @param ppChunks a pointer to TCAS_pChunk that is going to hold the compressed data
 * @return tcas_bool
 */
extern tcas_bool libtcas_compress_chunks_z(tcas_unit *chunksBuf, tcas_u32 chunkCount, TCAS_pChunk *ppChunks);

/**
 * write a z compressed chunk to file
 * @see libtcas_write_chunk()
 * @param pFile a pointer to TCAS_File structure
 * @param pChunk the address of TCAS_Chunk that is going to be written to file
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_write_chunk_z(TCAS_pFile pFile, const TCAS_pChunk pChunk);

/**
 * Write compressed chunks whose pos_and_color are compressed by zlib to file. 
 * Remark: make sure that the file type is TCAS_FILE_TYPE_COMPRESSED_Z and the file position indicator is just at the beginning of a chunk.
 *
 * @see libtcas_write_chunks()
 * @param pFile a pointer to TCAS_File structure
 * @param pChunks the address of TCAS_Chunk array that is going to be written to file
 * @param count indicates the amount of TCAS_Chunk that is going to write
 * @param freeChunks TCAS_TRUE - free chunks after writing them to file, TCAS_FALSE - keep chunks
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_write_chunks_z(TCAS_pFile pFile, const TCAS_pChunk pChunks, tcas_u32 count, tcas_bool freeChunks);

/**
 * Get minTime and maxTime of TCAS FX data in TCAS file and also count the chunks. 
 * @see libtcas_get_min_max_time_and_chunks()
 */
extern TCAS_Error_Code libtcas_get_min_max_time_and_chunks_z(const TCAS_pFile pFile, tcas_s32 *pMinTime, tcas_s32 *pMaxTime, tcas_unit *pChunkCount);

/**
 * Create ordered index of z compressed TCAS file
 * @see libtcas_create_ordered_index()
 */
extern TCAS_Error_Code libtcas_create_ordered_index_z(const TCAS_pFile pFile, const TCAS_pHeader pHeader, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, TCAS_pOrderedIndex *ppOrderedIndexs, tcas_u32 *pOrderedIndexCount);

/**
 * Create parsed index of z compressed parsed TCAS file
 * @see libtcas_index_parsed_tcas_file()
 */
extern TCAS_Error_Code libtcas_index_parsed_tcas_file_z(const TCAS_pFile pFile, const TCAS_pHeader pHeader, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, TCAS_pCompactedIndex *ppParsedIndexs, tcas_u32 *pIndexCount);

/**
 * Create TCAS frame of a z compressed TCAS file
 * @see libtcas_create_frame_with_linearized_index()
 */
extern TCAS_Error_Code libtcas_create_frame_with_linearized_index_z(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_IndexStreamsPtr indexStreams, tcas_s32 n, TCAS_pChunksCache pChunksCache, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf);

/**
 * The only difference from libtcas_create_frame_with_linearized_index_z function is that, you should allocate the buf
 * and resizing of frame is not supported
 *
 * @see libtcas_create_frame_with_linearized_index_z()
 */
extern TCAS_Error_Code libtcas_fill_frame_with_linearized_index_z(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_IndexStreamsPtr indexStreams, tcas_s32 n, TCAS_pChunksCache pChunksCache, tcas_byte *buf, tcas_u16 width, tcas_u16 height);

/**
 * Create a TCAS frame from compressed TCAS file with linearized index. 
 * Remark: make sure that the file type is TCAS_FILE_TYPE_PARSED_Z, 
 * and the pParsedIndexs is generated by the libtcas_index_parsed_tcas_file_z function
 */
extern TCAS_Error_Code libtcas_create_frame_with_parsed_index_z(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_pCompactedIndex pParsedIndexs, tcas_u32 indexCount, tcas_s32 n, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* LIBTCAS_HLA_Z_COMP_H */

