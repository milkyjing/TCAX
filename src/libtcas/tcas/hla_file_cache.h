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
#include "hla_z_comp.h"

#include <Windows.h>

#define TCAS_FILE_CACHE_INVALID_OFFSET 0xFFFFFFFFFFFFFFFF   /* maximum 64 bit unsigned integer */
#define TCAS_DEFAULT_FILE_CACHE_SIZE 0x02000000    /* 32 MB */

#define MAKE32B32B(x, y) ((tcas_u64)(((tcas_u32)(x)) | (((tcas_u64)(y)) << 32)))
#define GETLOW32B(x) ((tcas_u32)(x))
#define GETHI32B(x) ((tcas_u32)(((tcas_u64)(x)) >> 32))


/**
 * TCAS_FileCache structure is used for caching a piece of file with lots of chunks
 */
typedef struct _tcas_file_cache {
    TCAS_pFile pFile;     /**< handle of the file, used when mapping the file into memory */
    tcas_u64 offset;      /**< offset of the buffer in the file */
    tcas_u32 count;       /**< number of tcas_unit in the buffer */
    tcas_unit *buf;       /**< hold the content of the buffer */
    tcas_u32 temp;        /**< for temp use */
} TCAS_FileCache, *TCAS_pFileCache;

/**
 * TCAS_MapFileCache structure is used for caching a piece of file with lots of chunks using mapping file technique
 */
typedef struct _tcas_map_file_cache {
    HANDLE hMapFile;      /**< handle of the file, used when mapping the file into memory */
    ULONGLONG fileSize;   /**< file size */
    DWORD nGranul;        /**< granularity of file system, usually 64 KB */
    tcas_u64 offset;      /**< offset of the buffer in the file */
    tcas_u32 count;       /**< number of tcas_unit in the buffer */
    tcas_unit *mapBuf;    /**< hold the content of the buffer */
    tcas_u32 temp;        /**< for temp use */
} TCAS_MapFileCache, *TCAS_pMapFileCache;


/* Inhibit C++ name-mangling for libtcas functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */
    
/**
 * Initialize the file cache struct
 * @param pFileCache pointer to the TCAS_FileCache struct
 * @param size size of the buffer
 * @param pFile a pointer to the TCAS_File structure
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_init_file_cache(TCAS_pFileCache pFileCache, tcas_u32 size, TCAS_pFile pFile);
 
/**
 * Initialize the map file cache struct
 * @param pMapFileCache pointer to the TCAS_MapFileCache struct
 * @param size size of the buffer
 * @param filename the name of the TCAS file
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_init_map_file_cache(TCAS_pMapFileCache pMapFileCache, tcas_u32 size, const char *filename);

/**
 * Finalize the file cache struct
 * @param pFileCache pointer to the TCAS_FileCache struct
 * @param size size of the buffer
 */
extern void libtcas_fin_file_cache(TCAS_pFileCache pFileCache);

/**
 * Finalize the map file cache struct
 * @param pFileCache pointer to the TCAS_FileCache struct
 * @param size size of the buffer
 */
extern void libtcas_fin_map_file_cache(TCAS_pMapFileCache pMapFileCache);

/**
 * Use file cache support, both for z compressed or normal compressed chunks
 * @see libtcas_read_specified_chunk()
 */
extern TCAS_Error_Code libtcas_read_specified_chunk_ex(TCAS_pFileCache pFileCache, tcas_u64 offset, TCAS_pChunk pChunk);

extern TCAS_Error_Code libtcas_read_specified_chunk_ex_z(TCAS_pFileCache pFileCache, tcas_u64 offset, TCAS_pChunk pChunk);

/**
 * Use file cache support with the mapping file technique, both for z compressed or normal compressed chunks
 * @see libtcas_read_specified_chunk()
 */
extern TCAS_Error_Code libtcas_read_specified_chunk_ex_map(TCAS_pMapFileCache pMapFileCache, tcas_u64 offset, TCAS_pChunk pChunk);

extern TCAS_Error_Code libtcas_read_specified_chunk_ex_map_z(TCAS_pMapFileCache pMapFileCache, tcas_u64 offset, TCAS_pChunk pChunk);

/**
 * Use file cache support
 * @see libtcas_create_frame_with_linearized_index()
 */
extern TCAS_Error_Code libtcas_create_frame_with_linearized_index_ex(TCAS_pFileCache pFileCache, const TCAS_pHeader pHeader, const TCAS_IndexStreamsPtr indexStreams, tcas_u32 n, TCAS_pChunksCache pChunksCache, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf);

/**
 * Use file cache support with the mapping file technique
 * @see libtcas_create_frame_with_linearized_index()
 */
extern TCAS_Error_Code libtcas_create_frame_with_linearized_index_ex_map(TCAS_pMapFileCache pMapFileCache, const TCAS_pHeader pHeader, const TCAS_IndexStreamsPtr indexStreams, tcas_u32 n, TCAS_pChunksCache pChunksCache, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* LIBTCAS_HLA_FILE_CACHE_H */

