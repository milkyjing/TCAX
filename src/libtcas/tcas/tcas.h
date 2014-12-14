/*
 *  tcas.h -- interface of the 'libtcas' library, provides the very basic functionality
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

#ifndef LIBTCAS_TCAS_H
#define LIBTCAS_TCAS_H
#pragma once

#include <stdio.h>

#include "vector.h"
#include "rb.h"


/* version info */
#define LIBTCAS_VERSION 0x00008000    /**< High word of LIBTCAS_VERSION indicates major libtcas version, and low word indicates minor version */
#define LIBTCAS_VER_STR "0.8"

#define TCAS_VERSION 0x10002000
#define TCAS_VER_STR "1.2"

/* These types are used to describe TCAS file data */
typedef long long tcas_s64;
typedef unsigned long long tcas_u64;
typedef signed long tcas_s32;
typedef unsigned long tcas_u32;
typedef signed short tcas_s16;  /* used for posx and posy when rendering */
typedef unsigned short tcas_u16;
typedef signed char tcas_s8;
typedef unsigned char tcas_u8;
typedef tcas_u32 tcas_unit;
typedef tcas_u32 tcas_dword;
typedef tcas_u16 tcas_word;
typedef tcas_u8 tcas_byte;

#define TCAS_SIGNATURE ((tcas_unit)0x53414354)    /**< The signature of TCAS file is 0x54 0x43 0x41 0x53, which means `TCAS' in ASCII */

/* predefined values */
#define TCAS_HEADER_OFFSET (sizeof(TCAS_Header) >> 2)

#define TCAS_MIN_TIME ((tcas_s32)0x80000000)  /* -2147483648 */
#define TCAS_MAX_TIME ((tcas_s32)0x7FFFFFFF)  /* 2147483647 */
#define TCAS_INIT_MIN_TIME TCAS_MAX_TIME
#define TCAS_INIT_MAX_TIME TCAS_MIN_TIME

#define TCAS_RESERVED_POSX ((tcas_s16)0x8000)  /* -32768 */
#define TCAS_RESERVED_POSY ((tcas_s16)0x8000)
#define TCAS_RESERVED_POS ((tcas_u32)0x80008000)
#define TCAS_INVALID_POSX TCAS_RESERVED_POSX
#define TCAS_INVALID_POSY TCAS_RESERVED_POSY
#define TCAS_INVALID_POS TCAS_RESERVED_POS
#define TCAS_MAX_POSX ((tcas_s16)0x7FFF)  /* 32767 */
#define TCAS_MAX_POSY ((tcas_s16)0x7FFF)
#define TCAS_MIN_POSX ((tcas_s16)0x8001)  /* -32767 */
#define TCAS_MIN_POSY ((tcas_s16)0x8001)
#define TCAS_INIT_MIN_POSX TCAS_MAX_POSX
#define TCAS_INIT_MIN_POSY TCAS_MAX_POSY
#define TCAS_INIT_MAX_POSX TCAS_MIN_POSX
#define TCAS_INIT_MAX_POSY TCAS_MIN_POSY

/* tcas file types */
#define TCAS_FILE_TYPE_RAW 0
#define TCAS_FILE_TYPE_COMPRESSED 1
#define TCAS_FILE_TYPE_COMPRESSED_Z 2     /**< compressed chunks with pos_and_color compressed by zlib, check hla_z_comp.h for more information */
#define TCAS_FILE_TYPE_PARSED 3
#define TCAS_FILE_TYPE_PARSED_Z 4

#define TCAS_KEY_FRAMING_DISABLED 0
#define TCAS_KEY_FRAMING_ENABLED 1

/* error true false */
#define TCAS_ERROR -1
#define TCAS_FAIL TCAS_ERROR
#define TCAS_FALSE 0
#define TCAS_TRUE 1
#define tcas_error TCAS_ERROR
#define tcas_fail TCAS_FAIL
#define tcas_false TCAS_FALSE
#define tcas_true TCAS_TRUE

typedef int tcas_bool;    /**< -1 - error/fail, 0 - false, 1 - true */

typedef VectorPtr TCAS_IndexStreamsPtr;  /**< every index stream will hold several linear indexs */
typedef VectorPtr TCAS_LinearIndexsPtr;

/* These macros help to convert different data types */
/* pos */
#define MAKE16B16B(x, y) ((tcas_u32)(((tcas_u16)(x)) | (((tcas_u32)(y)) << 16)))
#define MAKEPOS MAKE16B16B
#define GETLOW16B(x) ((tcas_u16)(x))
#define GETPOSX (tcas_s16)GETLOW16B
#define GETHI16B(x) ((tcas_u16)(((tcas_u32)(x)) >> 16))
#define GETPOSY (tcas_s16)GETHI16B
/* color */
#define MAKE8B8B8B8B(a, b, c, d) ((tcas_u32)(((tcas_u8)(a)) | (((tcas_u32)((tcas_u8)(b))) << 8) | (((tcas_u32)((tcas_u8)(c))) << 16) | (((tcas_u32)((tcas_u8)(d))) << 24)))
#define MAKERGBA MAKE8B8B8B8B
#define GETR(a) ((tcas_u8)(a))
#define GETG(a) ((tcas_u8)(((tcas_u32)(a)) >> 8))
#define GETB(a) ((tcas_u8)(((tcas_u32)(a)) >> 16))
#define GETA(a) ((tcas_u8)(((tcas_u32)(a)) >> 24))
/* count_layer_type_pair tcas_unit */
#define MAKE24B4B3B1B(c, l, t, p) ((0x00FFFFFF & (tcas_u32)(c)) | ((0x0000000F & (tcas_u32)(l)) << 24) | ((0x00000007 & (tcas_u32)(t)) << 28) | ((0x00000001 & (tcas_u32)(p)) << 31))
#define MAKECLTP MAKE24B4B3B1B
#define GETHI1B(x) ((tcas_u8)(((tcas_u32)(x)) >> 31))
#define GETPAIR GETHI1B
#define GETHI3B(x) ((tcas_u8)((0x7FFFFFFF & (tcas_u32)(x)) >> 28))
#define GETTYPE GETHI3B
#define GETME4B(x) ((tcas_u8)(0x0000000F & (((tcas_u32)(x)) >> 24)))
#define GETLAYER GETME4B
#define GETLOW24B(x) (0x00FFFFFF & (tcas_u32)(x))
#define GETCOUNT GETLOW24B
/* utility, used in raw chunk to build or retrieve type and pair bits */
#define MAKE15B1B(t, p) ((0x0007 & (tcas_u16)(t)) | ((0x0001 & (tcas_u16)(p)) << 15))  /* type only occupies 3 bits, and pair only 1 bit */
#define MAKERAWTP MAKE15B1B
#define GETRAWPAIR(x) ((tcas_u8)(((tcas_u16)(x)) >> 15))
#define GETRAWTYPE(x) ((tcas_u8)(0x0007 & (tcas_u16)(x)))
/* utility, used in index structures */
#define MAKE31B1B(i, p) (((tcas_u32)(i)) | ((0x00000001 & (tcas_u32)(p)) << 31))
#define MAKEINDEXIP MAKE31B1B
#define GETINDEXID(x) (0x7FFFFFFF & (tcas_u32)(x))
#define GETINDEXPAIR(x) ((tcas_u8)(((tcas_u32)(x)) >> 31))

#if defined __GNUC__
#   define _FILE_OFFSET_BITS 64
#   define tcas_fseek fseeko64
#else
#   if defined _MSC_VER
#       define tcas_fseek _fseeki64
#   else
#       define tcas_fseek fseek
#   endif
#endif

/* These macros help to deal with some mathematics expressions */
#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#define __min(a,b)  (((a) < (b)) ? (a) : (b))
#define __abs(a)    (((a) > 0) ? (a) : (-(a)))

/**
 * a shortcut macro for counting raw chunks through filesize
 */
#define COUNT_RAW_CHUNKS(filesize) (((filesize) - sizeof(TCAS_Header)) / sizeof(TCAS_RawChunk))    /**< count chunks of raw TCAS file */

/**
 * linear interpolation for position, used in generation of intermediate frame chunks
 * pos1 and pos2 are of tcas_u32 type and is created by MAKEPOS macros
 * factor is of double type, measures the distance of the interpolated point from pos1 and pos2
 */
#define LINEAR_INTERPOLATION_POS(pos1, pos2, factor) MAKEPOS(GETPOSX(pos1) + (GETPOSX(pos2) - GETPOSX(pos1)) * (double)(factor), GETPOSY(pos1) + (GETPOSY(pos2) - GETPOSY(pos1)) * (double)(factor))

/**
 * linear interpolation for rgba, used in generation of intermediate frame chunks
 * rgba1 and rgba2 are of tcas_u32 type and is created by MAKERGBA macros
 * factor is of double type, measures the distance of the interpolated rgba from rgba1 and rgba2
 */
#define LINEAR_INTERPOLATION_RGBA(rgba1, rgba2, factor) MAKERGBA(GETR(rgba1) + (GETR(rgba2) - GETR(rgba1)) * (double)(factor), GETG(rgba1) + (GETG(rgba2) - GETG(rgba1)) * (double)(factor), GETB(rgba1) + (GETB(rgba2) - GETB(rgba1)) * (double)(factor), GETA(rgba1) + (GETA(rgba2) - GETA(rgba1)) * (double)(factor))


/**
 * TCAS_Error_Code enumeration defines some common errors.
 */
typedef enum _tcas_error_code {
    tcas_error_success = 0,    /**< indicates a successful process */
    tcas_error_null_pointer,  /**< receive a null pointer which is not desired */
    tcas_error_file_not_found,  /**< can not find the target TCAS file */
    tcas_error_file_cannot_open,   /**< can not open the target TCAS file */
    tcas_error_file_cannot_create,  /**< can not create the target TCAS file */
    tcas_error_file_while_reading,  /**< some error occurred while reading a TCAS file */
    tcas_error_file_while_writing,    /**< some error occurred while writing to a TCAS file */
    tcas_error_file_type_not_match,    /**< the target TCAS file's type does not match the one which is required */
    tcas_error_file_type_not_support,    /**< the target TCAS file's type has not been supported yet */
    tcas_error_key_frame_lack_partner,    /**< key frame chunks should come in pairs */
    tcas_error_file_cache_too_small     /* the file cache is too small to hold a complete chunk */
} TCAS_Error_Code;

/**
 * TCAS_File_Open_Type enumeration defines two kinds of file opening type. 
 * This is only used in libtcas_open_file function.
 */
typedef enum _tcas_file_open_type {
    tcas_file_open_existing,    /**< open an existing TCAS file */
    tcas_file_create_new,    /**< create a new TCAS file */
    tcas_file_read_write     /**< open a TCAS file with `read-write' permission */
} TCAS_File_Open_Type;

/**
 * TCAS_File_Position_Indicator enumeration defines three positions of file position indicator.
 */
typedef enum _tcas_file_position_indicator {
    tcas_fpi_set,      /**< position at the beginning of the file */
    tcas_fpi_header,   /**< position right after the TCAS header */
    tcas_fpi_end       /**< position at the end of the file */
} TCAS_File_Position_Indicator;

/**
 * TCAS_File structure is used for handling TCAS file I/O. 
 * Remark: you can change data members except for `fp' freely, 
 * the data members of TCAS_File can be used as `global' variables and can pass through functions.
 */
typedef struct _tcas_file {
    FILE *fp;             /**< a pointer to FILE structure */
    tcas_unit flag;       /**< use as a temp variable to hold the flag of the TCAS file */
    tcas_s32 minTime;     /**< use as a temp variable to hold the starting time of the TCAS FX */
    tcas_s32 maxTime;     /**< use as a temp variable to hold the end time of the TCAS FX */
    tcas_unit chunks;     /**< use as a temp variable to hold the data chunks of the TCAS file */
    tcas_unit temp;       /**< use as a temp variable */
} TCAS_File, *TCAS_pFile;

/**
 * TCAS_Header structure is used to represent TCAS file's header part. 
 * Note that the order of data members do matter according to the specification of TCAS, therefore cannot be changed.
 */
typedef struct _tcas_header {
    /**
     * The Signature of TCAS file is 0x54 0x43 0x41 0x53, which means TCAS in ASCII.
     */
    tcas_unit signature;
    /**
     * Low word of Version indicates minor TCAS file version, the initial minor TCAS file version is 9. 
     * High word of Version indicates major TCAS file version, the initial major TCAS file version is 0.
     */
    tcas_unit version;
    /**
     * HIWORD of Flag indicates TCAS file type, LOWORD of Flag indicates enable keyframing or not, 
     * if HIWORD is 0, then it means the TCAS file contains raw TCAS FX data which is made up of dozens of raw DIPs, 
     * if HIWORD is 1, then it means the TCAS file has been compressed, and contains packed DIPs. 
     * Raw TCAS file is usually used as temp TCAS file between softwares, 
     * compressed TCAS file is the most commonly used one as permanent storage.
     */
    tcas_unit flag;
    /**
     * Low word of Resolution indicates the horizonal resolution that the TCAS FX is dealing with. 
     * High word of Resolution indicates the vertical resolution that the TCAS FX is dealing with. 
     * The unit of Resolution is pixel. Note that the width and height are both from -32767 to 32767, 
     * which is much enough as video resolution.
     */
    tcas_unit resolution;
    /**
     * MinTime indicates the first time that the TCAS FX has taken effects. 
     * The unit of MinTime is millisecond, so the range of  MinTime is [-2147483648, 2147483647], more than 1 thousand hours.
     */
    tcas_s32 minTime;
    /**
     * MaxTime indicates the first time that the TCAS FX has ended its effects. 
     * The unit of MaxTime is millisecond, so the range of  MaxTime is [-2147483648, 2147483647], more than 1 thousand hours.
     */
    tcas_s32 maxTime;
    /**
     * Chunks indicates the number of data chunks in the body part of TCAS file.
     */
    tcas_unit chunks;
    /**
     * fpsNumerator indicates the numerator of target video's FPS
     */
    tcas_unit fpsNumerator;
    /**
     * fpsDenominator indicates the denominator of target video's FPS
     */
    tcas_unit fpsDenominator;
    /**
     * Reserved, will be used in later versions.
     */
    tcas_unit reserved[7];
} TCAS_Header, *TCAS_pHeader;

/**
 * TCAS_RawChunk structure is used to describe raw TCAS file's minimal unit. 
 * if we use tcas_unit to define the whole position, then it has form of 0xposYposX, 
 * if we use tcas_unit to define the whole color, then it has form of 0xaabbggrr.
 * Note that, actually frameType only occupies 3 bits of memory, pair bit only occupies 1 bit of memory and layer occupies 5 bits, 
 * the reason that we use tcas_u16 to hold them is to make the total size of TCAS_RawChunk equals to 5 DWORDs.
 * Note that the order of data members do matter according to the specification of TCAS, therefore cannot be changed.
 */
typedef struct _tcas_raw_chunk {
    tcas_s32 startTime;  /**< starting time of the chunk */
    tcas_s32 endTime;    /**< end time of the chunk */
    tcas_u16 layer;      /**< layer of the chunk */
    tcas_u16 type_pair;  /**< frameType and pair bit of the chunk */
    tcas_s16 posX;       /**< x-axis position of the DIP in the chunk */
    tcas_s16 posY;       /**< y-axis position of the DIP in the chunk */
    tcas_u8  r;          /**< red channel of the DIP color */
    tcas_u8  g;          /**< green channel of the DIP color */
    tcas_u8  b;          /**< blue channel of the DIP color */
    tcas_u8  a;          /**< alpha channel of the DIP color */
} TCAS_RawChunk, *TCAS_pRawChunk;

/**
 * TCAS_Chunk structure is used to describe compressed TCAS file's minimal unit.
 * Note that the order of data members do matter according to the specification of TCAS, therefore cannot be changed.
 */
typedef struct _tcas_chunk {
    tcas_s32  startTime;        /**< starting time of the chunk */
    tcas_s32  endTime;          /**< end time of the chunk */
    tcas_unit cltp;             /**< count_layer_type_pair, pair occupies the high 1 bit, type occupies the high 3 bits, layer occupies the high but three 4 bits, and count occupies the low 24 bits */
    tcas_unit *pos_and_color;   /**< every pos_and_color occupies 2 tcas_unit */
} TCAS_Chunk, *TCAS_pChunk;

typedef TCAS_Chunk TCAS_CompChunk;
typedef TCAS_Chunk TCAS_CompressedChunk;
typedef TCAS_pChunk TCAS_pCompChunk;
typedef TCAS_pChunk TCAS_pCompressedChunk;

/**
 * TCAS_Index structure is used for indexing compressed TCAS file. 
 * The difference between TCAS_Index and TCAS_OrderedIndex is that TCAS_OrderedIndex does not have to reorder the chunks by their layer.
 * Remark: because offset is a 32-bit value, so the max size of compressed TCAS file supported by libtcas is 
 * limited to sizeof(tcas_unit) * 4GB = 16GB, thus the max TCAS file size supported by libtcas is also limited to 16GB.
 */
typedef struct _tcas_index {
    tcas_s32  first;            /**< may be startTime or firstFrame depending on which function fills this structure */
    tcas_s32  last;             /**< may be endTime or lastFrame depending on which function fills this structure */
    tcas_unit cltp;             /**< count_layer_type_pair, pair occupies the high 1 bit, type occupies the high 3 bits, layer occupies the high but three 4 bits, and count occupies the low 24 bits */
    tcas_u32  offset;           /**< chunk offset in the TCAS file, file position indicator = offset * sizeof(tcas_unit) */
} TCAS_Index, *TCAS_pIndex;

/**
 * TCAS_CompactedIndex structure is used for parsing compressed TCAS file, and kept as index file. 
 * After generating TCAS_Index you can convert it to TCAS_CompactedIndex, since cltp data member is not 
 * necessary when parsing compressed TCAS file. And keep it as file rather than TCAS_Index can save space.
 * Remark: because offset is a 32-bit value, so the max size of compressed TCAS file supported by libtcas is 
 * limited to sizeof(tcas_unit) * 4GB = 16GB, thus the max TCAS file size supported by libtcas is also limited to 16GB.
 */
typedef struct _tcas_compacted_index {
    tcas_s32 first;             /**< may be startTime or firstFrame depending on which function fills this structure */
    tcas_s32 last;              /**< may be endTime or lastFrame depending on which function fills this structure */
    tcas_u32 offset;            /**< chunk offset in the TCAS file, file position indicator = offset * sizeof(tcas_unit) */
} TCAS_CompactedIndex, *TCAS_pCompactedIndex;

/**
 * TCAS_OrderedIndex structure is used for linearizing compressed TCAS file, and kept as index file. 
 * It reserves the order that the chunks were genreated, and can be linearized to linear index streams.
 * The difference between TCAS_OrderedIndex and TCAS_Index is that TCAS_Index has to reorder the chunks by their layer.
 * Remark: because offset is a 32-bit value, so the max size of compressed TCAS file supported by libtcas is 
 * limited to sizeof(tcas_unit) * 4GB = 16GB, thus the max TCAS file size supported by libtcas is also limited to 16GB.
 */
typedef struct _tcas_ordered_index {
    tcas_s32 first;             /**< may be startTime or firstFrame depending on which function fills this structure */
    tcas_s32 last;              /**< may be endTime or lastFrame depending on which function fills this structure */
    tcas_u32 id_pair;           /**< identifier of chunk used to reserve the order of chunks for the original order will be messed up according to linearization, id occupies the low 31 bits, pair bit occupies the high 1 bit */
    tcas_u32 offset;            /**< chunk offset in the TCAS file, file position indicator = offset * sizeof(tcas_unit) */
} TCAS_OrderedIndex, *TCAS_pOrderedIndex;

/**
 * TCAS_KeyFrameChunkPair structure
 * Used for storing a key frame chunk pair
 */
typedef struct _tcas_key_frame_chunk_pair {
    TCAS_Chunk start;
    TCAS_Chunk end;
} TCAS_KeyFrameChunkPair, *TCAS_pKeyFrameChunkPair;

typedef TCAS_KeyFrameChunkPair TCAS_ChunkPair;
typedef TCAS_pKeyFrameChunkPair TCAS_pChunkPair;

/**
 * TCAS_ChunksCache structure
 * Used for caching the read chunks when parsing compressed TCAS file using linearized index streams
 */
typedef struct _tcas_chunks_cache {
    tcas_u32 streamCount;         /**< the number of linearized index streams */
    tcas_u32 *pi;                 /**< the address of the indexs of last effective or non-effective indexs in each linearized index streams */
    TCAS_pChunk pChunks;          /**< the address of the TCAS_Chunk array that contains normal chunks or generated intermediate chunks of each stream */
    TCAS_pChunkPair pChunkPairs;  /**< the address of the TCAS_ChunkPair array that contains key frame chunks of each stream */
} TCAS_ChunksCache, *TCAS_pChunksCache;


/* Inhibit C++ name-mangling for libtcas functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * MulDiv.
 */
extern tcas_s32 libtcas_MulDiv(tcas_s32 a, tcas_s32 b, tcas_s32 c);

/**
 * Open an existing file or create a new file, file should open in binary mode.
 * @param pFile a pointer to TCAS_File structure
 * @param filename specify the file name which is to be opened
 * @param type specify file opening type
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_open_file(TCAS_pFile pFile, const char *filename, TCAS_File_Open_Type type);

/**
 * Close the tcas file handler associated with TCAS_File structure.
 * @param pFile a pointer to TCAS_File structure
 */
extern void libtcas_close_file(TCAS_pFile pFile);

/**
 * Can read any kinds of data from TCAS file, but we do not recommend you using this function.
 * @see libtcas_read_header()
 * @see libtcas_read_raw_chunks()
 * @see libtcas_read_chunk()
 * @see libtcas_read_specified_chunk()
 * @see libtcas_read_chunks()
 * @param pFile a pointer to TCAS_File structure
 * @param buf memory buffer that is going to hold the data read from the TCAS file, should be allocated before passing it to the function
 * @param count indicates the amount of tcas_unit that is going to read
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_read(TCAS_pFile pFile, tcas_unit *buf, tcas_u32 count);

/**
 * Can write any kinds of data to TCAS file, but we do not recommend you using this function.
 * @see libtcas_write_header()
 * @see libtcas_write_raw_chunks()
 * @see libtcas_write_chunk()
 * @see libtcas_write_chunks()
 * @param pFile a pointer to TCAS_File structure
 * @param buf memory buffer that is going to be written to the TCAS file
 * @param count indicates the amount of tcas_unit that is going to write
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_write(TCAS_pFile pFile, const tcas_unit *buf, tcas_u32 count);

/**
 * Check the file's signature to see whether it's a TCAS file or not. 
 * Remark: this function will NOT change the file position indicator.
 *
 * @param pFile a pointer to TCAS_File structure
 * @return tcas_bool
 */
extern tcas_bool libtcas_check_signature(const TCAS_pFile pFile);

/**
 * Set file position indicator.
 * @param pFile a pointer to TCAS_File structure
 * @param position tcas_fpi_set - set the file position indicator to the beginning of the file
 *                 tcas_fpi_header - set the file position indicator to right after the TCAS header
 *                 tcas_fpi_end - set the file position indicator to the end of the file
 */
extern void libtcas_set_file_position_indicator(TCAS_pFile pFile, TCAS_File_Position_Indicator position);

/**
 * Use values specified by the params to fill the TCAS_Header structure.
 * @param pHeader a pointer to TCAS_pHeader structure that is going to be filled
 * @param type TCAS file type, can be TCAS_FILE_TYPE_RAW or TCAS_FILE_TYPE_COMPRESSED
 * @param keyframing 1 - enable keyframing technique, 0 - disable keyframing technique
 * @param width horizonal resolution of TCAS FX
 * @param height vertical resolution of TCAS FX
 * @param minTime indicates the first time that the TCAS FX has taken effects
 * @param maxTime indicates the first time that the TCAS FX has ended its effects
 * @param chunks indicates the number of data chunks in the body part of the TCAS file
 * @param fpsNumerator indicates the numerator of target video's FPS
 * @param fpsDenominator indicates the denominator of target video's FPS
 */
extern void libtcas_set_header(TCAS_pHeader pHeader, tcas_u16 type, tcas_u16 keyframing, tcas_u16 width, tcas_u16 height, tcas_s32 minTime, tcas_s32 maxTime, tcas_unit chunks, tcas_unit fpsNumerator, tcas_unit fpsDenominator);

/**
 * Read the header part of TCAS file and assign values to TCAS_Header structure.
 * @param pFile a pointer to TCAS_File structure
 * @param pHeader a pointer to TCAS_Header structure
 * @param keepPosition TCAS_TRUE - keep the file position indicator, TCAS_FALSE do not keep the file position indicator
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_read_header(TCAS_pFile pFile, TCAS_pHeader pHeader, tcas_bool keepPosition);

/**
 * Write TCAS_Header strucutre to file.
 * @param pFile a pointer to TCAS_File structure
 * @param pHeader a pointer to TCAS_Header structure which is going to be written to the TCAS file
 * @param keepPosition TCAS_TRUE - keep the file position indicator, TCAS_FALSE do not keep file the position indicator
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_write_header(TCAS_pFile pFile, const TCAS_pHeader pHeader, tcas_bool keepPosition);

/**
 * Read raw chunks from file. 
 * Remark: make sure that the file type is TCAS_FILE_TYPE_RAW and the file position indicator is just at the beginning of a chunk.
 *
 * @param pFile a pointer to TCAS_File structure
 * @param pRawChunks the address of TCAS_RawChunk array that is going to hold chunk data
 * @param count indicates the amount of raw chunks that is going to read
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_read_raw_chunks(TCAS_pFile pFile, TCAS_pRawChunk pRawChunks, tcas_u32 count);

/**
 * Write raw chunks to file. 
 * Remark: make sure that the file type is TCAS_FILE_TYPE_RAW and the file position indicator is just at the beginning of a chunk.
 *
 * @param pFile a pointer to TCAS_File structure
 * @param pRawChunks the address of TCAS_RawChunk array that is going to be written to file
 * @param count indicates the amount of TCAS_RawChunk that is going to write
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_write_raw_chunks(TCAS_pFile pFile, const TCAS_pRawChunk pRawChunks, tcas_u32 count);

/**
 * Allocate memory for one compressed chunk to store amount number of packed DIPs.
 * @param pChunk a pointer to TCAS_Chunk structure
 * @param count indicates the amount of packed DIPs
 */
extern void libtcas_alloc_chunk(TCAS_pChunk pChunk, tcas_u32 count);

/**
 * Free memory of a compressed chunk, but not including the memory that is occupied by TCAS_Chunk structure.
 * @param pChunk a pointer to TCAS_Chunk structure
 */
extern void libtcas_free_chunk(TCAS_pChunk pChunk);

/**
 * Make a copy of a compressed chunk.
 * @param pChunk a pointer to TCAS_Chunk structure that is going to hold the copy
 * @param pChunkSrc a pointer to TCAS_Chunk structure that holds the source
 */
extern void libtcas_copy_chunk(TCAS_pChunk pChunk, const TCAS_pChunk pChunkSrc);

/**
 * Read a compressed chunk from file. 
 * Remark: you should allocate memory for pChunk, but not for pChunk->pos_and_color. 
 * Make sure that the file type is TCAS_FILE_TYPE_COMPRESSED and the file position indicator is just at the beginning of a chunk.
 *
 * @param pFile a pointer to TCAS_File structure
 * @param pChunk a pointer to TCAS_Chunk structure that is going to hold chunk data
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_read_chunk(TCAS_pFile pFile, TCAS_pChunk pChunk);

/**
 * Read the compressed chunk specified by the offset from file. 
 * Remark: you should allocate memory for pChunk, but not for pChunk->pos_and_color. 
 * Make sure that the file type is TCAS_FILE_TYPE_COMPRESSED and the file position indicator is just at the beginning of a chunk.
 * The file position indicator is set by offset from the beginning of the file.
 *
 * @param pFile a pointer to TCAS_File structure
 * @param offset the offset of the chunk from the beginning of the file
 * @param pChunk a pointer to TCAS_Chunk structure that is going to hold chunk data
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_read_specified_chunk(TCAS_pFile pFile, tcas_u64 offset, TCAS_pChunk pChunk);

/**
 * Remark: the offset should be in the tcasBuf
 * 
 * @see libtcas_create_ordered_index_from_stream()
 * @see libtcas_fill_frame_from_stream_with_linearized_index()
 */
extern TCAS_Error_Code libtcas_read_specified_chunk_from_stream(const tcas_unit *tcasBuf, tcas_u64 offset, TCAS_pChunk pChunk);

/**
 * Read compressed chunks from file. 
 * Remark: you should allocate memory for pChunk, but not for pChunk->pos_and_color. 
 * Make sure that the file type is TCAS_FILE_TYPE_COMPRESSED and the file position indicator is just at the beginning of a chunk.
 *
 * @see libtcas_read_chunk()
 * @param pFile a pointer to TCAS_File structure
 * @param pChunks the address of TCAS_Chunk array that is going to hold chunk data
 * @param count indicates the amount of compressed chunks that is going to read
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_read_chunks(TCAS_pFile pFile, TCAS_pChunk pChunks, tcas_u32 count);

/**
 * Write one compressed chunk to file. 
 * Remark: make sure that the file type is TCAS_FILE_TYPE_COMPRESSED and the file position indicator is just at the beginning of a chunk.
 *
 * @param pFile a pointer to TCAS_File structure
 * @param pChunk a pointer to TCAS_Chunk structure that is going to be written to file
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_write_chunk(TCAS_pFile pFile, const TCAS_pChunk pChunk);

/**
 * Write compressed chunks to file. 
 * Remark: make sure that the file type is TCAS_FILE_TYPE_COMPRESSED and the file position indicator is just at the beginning of a chunk.
 *
 * @see libtcas_write_chunk()
 * @param pFile a pointer to TCAS_File structure
 * @param pChunks the address of TCAS_Chunk array that is going to be written to file
 * @param count indicates the amount of TCAS_Chunk that is going to write
 * @param freeChunks TCAS_TRUE - free chunks after writing them to file, TCAS_FALSE - keep chunks
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_write_chunks(TCAS_pFile pFile, const TCAS_pChunk pChunks, tcas_u32 count, tcas_bool freeChunks);

/**
 * Compress raw chunks (in stream), the result of compression largely depends on the 
 * order that the raw chunks were sent to the function, i.e., locality is a big issue 
 * when using this function. Remark: In the compressing process, adjacent raw DIPs who have 
 * the same StartT EndT frameType and Layer can share the same chunk, therefore compressed. 
 * Compress one DIP save 3 tcas_unit space.
 *
 * @param pRawChunks the address of TCAS_RawChunk array that is going to be compressed
 * @param rawChunks the amount of input raw chunks
 * @param reallocBuf if set to true *pBuf will be smaller, but needs extra effort
 * @param pBuf a pointer to a block of memory buffer that is going to hold the compressed data
 * @param pChunkCount pointer to the amount of output compressed chunks
 * @param pUnitCount pointer to the amount of tcas_unit in the buffer
 * @return const tcas_unit *
 */
extern const tcas_unit *libtcas_compress_raw_chunks(const TCAS_pRawChunk pRawChunks, tcas_u32 rawChunks, tcas_bool reallocBuf, tcas_unit **pBuf, tcas_u32 *pChunkCount, tcas_u32 *pUnitCount);

/**
 * Convert raw chunks to compressed chunks, the result of compression largely depends on the 
 * order that the raw chunks were sent to the function, i.e., locality is a big issue 
 * when using this function. Remark: In the compressing process, adjacent raw DIPs who have 
 * the same StartT EndT frameType and Layer can share the same chunk.
 *
 * @see libtcas_compress_raw_chunks()
 * @param pRawChunks the address of TCAS_RawChunk array that is going to be compressed
 * @param rawChunks the amount of input raw chunks
 * @param ppChunks a pointer to TCAS_pChunk that is going to hold the compressed data
 * @param pChunkCount pointer to the amount of output compressed chunks
 * @return const TCAS_pChunk
 */
extern const TCAS_pChunk libtcas_convert_raw_chunks(const TCAS_pRawChunk pRawChunks, tcas_u32 rawChunks, TCAS_pChunk *ppChunks, tcas_u32 *pChunkCount);

/**
 * Uncompress a compressed chunk (in stream).
 * @param pChunk a pointer to a compressed TCAS chunk that is going to be uncompressed
 * @param pBuf a pointer to a block of memory buffer that is going to hold the uncompressed data
 * @param pRawChunkCount pointer to the amount of output raw chunks
 * @param pUnitCount pointer to the amount of tcas_unit in the buffer
 * @return const tcas_unit *
 */
extern const tcas_unit *libtcas_uncompress_chunk(const TCAS_pChunk pChunk, tcas_unit **pBuf, tcas_u32 *pRawChunkCount, tcas_u32 *pUnitCount);

/**
 * Convert a compressed chunk to raw chunks.
 * @see libtcas_uncompress_chunk()
 * @param pChunk a pointer to a compressed TCAS chunk that is going to be uncompressed
 * @param ppRawChunks a pointer to TCAS_pRawChunk that is going to hold the uncompressed data
 * @param pRawChunkCount pointer to the amount of output raw chunks
 * @return const TCAS_pRawChunk
 */
extern const TCAS_pRawChunk libtcas_convert_chunk(const TCAS_pChunk pChunk, TCAS_pRawChunk *ppRawChunks, tcas_u32 *pRawChunkCount);

/**
 * Uncompress compressed chunks (in stream).
 * @param pChunks the address of TCAS_Chunk array that is going to be uncompressed
 * @param chunks the amount of input compressed chunks
 * @param pBuf a pointer to a block of memory buffer that is going to hold the uncompressed data
 * @param pRawChunkCount pointer to the amount of output raw chunks
 * @param pUnitCount pointer to the amount of tcas_unit in the buffer
 * @param freeChunks TCAS_TRUE - free chunks after writing them to file, TCAS_FALSE - keep chunks
 * @return const tcas_unit *
 */
extern const tcas_unit *libtcas_uncompress_chunks(TCAS_pChunk pChunks, tcas_u32 chunks, tcas_unit **pBuf, tcas_u32 *pRawChunkCount, tcas_u32 *pUnitCount, tcas_bool freeChunks);

/**
 * Convert compressed chunks to raw chunks.
 * @see libtcas_uncompress_chunks()
 * @param pChunks the address of TCAS_Chunk array that is going to be uncompressed
 * @param chunks the amount of input compressed chunks
 * @param ppRawChunks a pointer to TCAS_pRawChunk that is going to hold the uncompressed data
 * @param pRawChunkCount pointer to the amount of output raw chunks
 * @param freeChunks TCAS_TRUE - free chunks after writing them to file, TCAS_FALSE - keep chunks
 * @return const TCAS_pRawChunk
 */
extern const TCAS_pRawChunk libtcas_convert_chunks(TCAS_pChunk pChunks, tcas_u32 chunks, TCAS_pRawChunk *ppRawChunks, tcas_u32 *pRawChunkCount, tcas_bool freeChunks);

/**
 * Convert a RGBA array to compressed chunk. 
 * Remark: the function will not change the value of startTime, endTime, type and layer byte in the output chunk.
 *
 * @see libtcas_convert_chunk_to_rgba()
 * @see libtcas_convert_chunks_to_rgba()
 * @param rgba the address of a RGBA array
 * @param width width of RGBA array
 * @param height height of RGBA array
 * @param reallocBuf if set to true pChunk->pos_and_color will be smaller, but needs extra effort
 * @param pChunk a pointer to TCAS_Chunk that is going to hold the converted data
 * @return const TCAS_pChunk
 */
extern const TCAS_pChunk libtcas_convert_rgba_to_chunk(const tcas_byte *rgba, tcas_u16 width, tcas_u16 height, tcas_bool reallocBuf, TCAS_pChunk pChunk);

/**
 * Blend two colors.
 *
 * @param back color of the background
 * @param over color of the overlay
 * @return the blended color
 */
extern tcas_u32 libtcas_blend_color(tcas_u32 back, tcas_u32 over);

/**
 * Convert a chunk to RGBA array. 
 * Remark: the function will just ignore the value of startTime, endTime and layer byte.
 *
 * @see libtcas_convert_rgba_to_chunk()
 * @see libtcas_convert_chunks_to_rgba()
 * @param pChunk a pointer to TCAS_Chunk that is going to be converted
 * @param width width of target RGBA array
 * @param height height of target RGBA array
 * @param pRGBA pointer to the address of a RGBA array
 * @return const tcas_byte *
 */
extern const tcas_byte *libtcas_convert_chunk_to_rgba(const TCAS_pChunk pChunk, tcas_u16 width, tcas_u16 height, tcas_byte **pRGBA);

/**
 * Convert chunks to RGBA array. 
 * Remark: the function will just ignore the value of startTime, endTime and layer byte. 
 * You should allocate memory for the target RGBA array. Note that this function only converts one chunk a time, 
 * if you want to convert more than one chunks, just use the function more than once.
 *
 * @see libtcas_convert_rgba_to_chunk()
 * @see libtcas_convert_chunk_to_rgba()
 * @param pChunks a pointer to TCAS_Chunk that is going to be converted
 * @param width width of target RGBA array
 * @param height height of target RGBA array
 * @param rgba pointer to the RGBA array that is going to hold the converted data
 */
extern void libtcas_convert_chunks_to_rgba(const TCAS_pChunk pChunks, tcas_u16 width, tcas_u16 height, tcas_byte *rgba);

/**
 * Resample RGBA array.
 * @param src the address of source RGBA array
 * @param width width of the source RGBA array
 * @param height height of the source RGBA array
 * @param dst the address of the destination RGBA array
 * @param targetWidth width of target RGBA array
 * @param targetHeight height of target RGBA array
 */
extern void libtcas_resample_rgba_bilinear_mmx(const tcas_byte *src, tcas_u16 width, tcas_u16 height, tcas_byte *dst, tcas_u16 targetWidth, tcas_u16 targetHeight);

/**
 * Get minPosX minPosY maxPosX maxPosY of a chunk.
 * @param pChunk a pointer to TCAS_Chunk structure
 * @param pMinPosX pointer to the minimal x-axis value of the target chunk
 * @param pMinPosY pointer to the minimal y-axis value of the target chunk
 * @param pMaxPosX pointer to the maximal x-axis value of the target chunk
 * @param pMaxPosY pointer to the maximal y-axis value of the target chunk
 */
extern void libtcas_get_chunk_min_max_pos(const TCAS_pChunk pChunk, tcas_s16 *pMinPosX, tcas_s16 *pMinPosY, tcas_s16 *pMaxPosX, tcas_s16 *pMaxPosY);

/**
 * Get minimal and maximal time of raw chunks.
 * @param pRawChunks a pointer to TCAS_RawChunk structure
 * @param rawChunks the amount of input raw chunks
 * @param pHeader pointer to TCAS_Header structure that is going to change its minimal start time value and maximal end time value from the target chunks
 */
extern void libtcas_get_raw_chunks_min_max_time(const TCAS_pRawChunk pRawChunks, tcas_u32 rawChunks, TCAS_pHeader pHeader);

/**
 * Get minimal and maximal time of compressed chunks.
 * @param pChunk a pointer to TCAS_Chunk structure
 * @param chunks the amount of input compressed chunks
 * @param pMinTime pointer to the minimal start time value of the target chunks
 * @param pMaxTime pointer to the maximal end time value of the target chunks
 */
extern void libtcas_get_chunks_min_max_time(const TCAS_pChunk pChunks, tcas_u32 chunks, tcas_s32 *pMinTime, tcas_s32 *pMaxTime);

/**
 * Get minimal and maximal time of compressed chunks (from TCAS buf).
 * @param tcasBuf a pointer to TCAS_BUF
 * @param chunks the amount of input compressed chunks
 * @param pMinTime pointer to the minimal start time value of the target chunks
 * @param pMaxTime pointer to the maximal end time value of the target chunks
 */
extern void libtcas_get_chunk_stream_min_max_time(const tcas_unit *tcasBuf, tcas_u32 chunks, tcas_s32 *pMinTime, tcas_s32 *pMaxTime);

/**
 * Count the compressed chunks of TCAS FX data in non-raw TCAS file. 
 * Remark: make sure that the file type is TCAS_FILE_TYPE_COMPRESSED, 
 * however, this function will check whether the file is TCAS_FILE_TYPE_COMPRESSED
 * this function will NOT change the file position indicator. 
 * Chunks in raw TCAS file can be calculate by macros COUNT_RAW_CHUNKS.
 *
 * @see libtcas_get_min_max_time_and_chunks()
 * @param pFile a pointer to TCAS_File structure
 * @param pChunkCount pointer to the amount of chunks in TCAS file
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_count_chunks(const TCAS_pFile pFile, tcas_unit *pChunkCount);

/**
 * Get minTime and maxTime of TCAS FX data in TCAS file and also count the chunks. 
 * Remark: make sure that the file type is TCAS_FILE_TYPE_COMPRESSED or TCAS_FILE_TYPE_PARSED, 
 * however, this function will check whether the file is TCAS_FILE_TYPE_COMPRESSED or TCAS_FILE_TYPE_PARSED
 * this function will NOT change the file position indicator. 
 *
 * @see libtcas_count_chunks()
 * @param pFile a pointer to TCAS_File structure
 * @param pMinTime pointer to the first time that the TCAS FX has taken effects
 * @param pMaxTime pointer to the first time that the TCAS FX has ended its effects
 * @param pChunkCount pointer to the amount of chunks in TCAS file
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_get_min_max_time_and_chunks(const TCAS_pFile pFile, tcas_s32 *pMinTime, tcas_s32 *pMaxTime, tcas_unit *pChunkCount);

/**
 * Create the index for a compressed TCAS file using specified FPS. 
 * Remark: make sure that the file type is TCAS_FILE_TYPE_COMPRESSED, 
 * however, this function will check whether the file is TCAS_FILE_TYPE_COMPRESSED
 * this function will NOT change the file position indicator. 
 * In this function, if fpsDenominator = 0, then TCAS_Index.first means startTime and TCAS_Index.last means endTime 
 * otherwise, TCAS_Index.first means firstFrame and TCAS_Index.last means lastFrame.
 *
 * @param pFile a pointer to TCAS_File structure
 * @param pHeader a pointer to TCAS_Header structure that holds the information of the header, if pHeader is NULL, then the function will read it from file
 * @param fpsNumerator FramePerSecond = fpsNumerator / (double)fpsDenominator, can be 0, in this case, fps will not be used
 * @param fpsDenominator FramePerSecond = fpsNumerator / (double)fpsDenominator, can be 0, in this case, fps will not be used
 * @param ppIndexs a pointer to TCAS_pIndex that is going to hold the parsed TCAS Index
 * @param pIndexCount pointer to the amount of output indexs, equal to chunks, can be NULL
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_index_compressed_tcas_file(const TCAS_pFile pFile, const TCAS_pHeader pHeader, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, TCAS_pIndex *ppIndexs, tcas_u32 *pIndexCount);

/**
 * Create the index for a parsed TCAS file using specified FPS. 
 * Remark: make sure that the file type is TCAS_FILE_TYPE_PARSED, 
 * however, this function will check whether the file is TCAS_FILE_TYPE_PARSED
 * this function will NOT change the file position indicator. 
 * In this function, if fpsDenominator = 0, then TCAS_Index.first means startTime and TCAS_Index.last means endTime 
 * otherwise, TCAS_Index.first means firstFrame and TCAS_Index.last means lastFrame.
 *
 * @param pFile a pointer to TCAS_File structure
 * @param pHeader a pointer to TCAS_Header structure that holds the information of the header, if pHeader is NULL, then the function will read it from file
 * @param fpsNumerator FramePerSecond = fpsNumerator / (double)fpsDenominator, can be 0, in this case, fps will not be used
 * @param fpsDenominator FramePerSecond = fpsNumerator / (double)fpsDenominator, can be 0, in this case, fps will not be used
 * @param ppParsedIndexs a pointer to TCAS_pCompactedIndex that is going to hold the parsed TCAS Index
 * @param pIndexCount pointer to the amount of output indexs, equal to chunks, can be NULL
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_index_parsed_tcas_file(const TCAS_pFile pFile, const TCAS_pHeader pHeader, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, TCAS_pCompactedIndex *ppParsedIndexs, tcas_u32 *pIndexCount);

/**
 * Compact the TCAS_Index to TCAS_CompactedIndex.
 * @param pIndexs the address of TCAS_Index array which will be compacted
 * @param indexs the amount of input indexs
 * @param ppCompactedIndexs pointer to the address of TCAS_CompactedIndex array which will hold the compacted index
 */
extern void libtcas_compact_index(const TCAS_pIndex pIndexs, tcas_u32 indexs, TCAS_pCompactedIndex *ppCompactedIndexs);

/**
 * Create the ordered index of a compressed TCAS file using specified FPS. 
 * Remark: make sure that the file type is TCAS_FILE_TYPE_COMPRESSED, 
 * however, this function will check whether the file is TCAS_FILE_TYPE_COMPRESSED
 * this function will NOT change the file position indicator. 
 * In this function, if fpsDenominator = 0, then TCAS_OrderedIndex.first means startTime and TCAS_OrderedIndex.last means endTime 
 * otherwise, TCAS_OrderedIndex.first means firstFrame and TCAS_OrderedIndex.last means lastFrame.
 * Note that, TCAS_OrderedIndex.id_pair = MAKEINDEXIP(0, 1) indicates invalid (no effect) chunks.
 *
 * @param pFile a pointer to TCAS_File structure
 * @param pHeader a pointer to TCAS_Header structure that holds the information of the header, if pHeader is NULL, then the function will read it from file
 * @param fpsNumerator FramePerSecond = fpsNumerator / (double)fpsDenominator, can be 0, in this case, fps will not be used
 * @param fpsDenominator FramePerSecond = fpsNumerator / (double)fpsDenominator, can be 0, in this case, fps will not be used
 * @param ppOrderedIndexs a pointer to TCAS_pOrderedIndex that is going to hold the compressed TCAS Ordered Index
 * @param pOrderedIndexCount pointer to the amount of output ordered indexs, equal to chunks, can be NULL
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_create_ordered_index(const TCAS_pFile pFile, const TCAS_pHeader pHeader, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, TCAS_pOrderedIndex *ppOrderedIndexs, tcas_u32 *pOrderedIndexCount);

/**
 * Create the ordered index from a compressed TCAS chunk stream using specified FPS. 
 * Remark: in this function, if fpsDenominator = 0, then TCAS_OrderedIndex.first means startTime and TCAS_OrderedIndex.last means endTime 
 * otherwise, TCAS_OrderedIndex.first means firstFrame and TCAS_OrderedIndex.last means lastFrame.
 * Note that, TCAS_OrderedIndex.id_pair = MAKEINDEXIP(0, 1) indicates invalid (no effect) chunks.
 *
 * @param tcasBuf the address of TCAS_BUF stream of which the ordered index will be created
 * @param chunks the amount of input compressed chunks
 * @param lastId the last `id' of ordered index, for the first time invoking this function it should better be 0 or 1 indicating the first index, after that it should be *pId from the last invokation
 * @param lastOffset the last `offset' of ordered index, for the first time invoking this function it should be sizeof(TCAS_Header) >> 2, after that it should be *pOffset from the last invokation
 * @param fpsNumerator FramePerSecond = fpsNumerator / (double)fpsDenominator, can be 0, in this case, fps will not be used
 * @param fpsDenominator FramePerSecond = fpsNumerator / (double)fpsDenominator, can be 0, in this case, fps will not be used
 * @param ppOrderedIndexs a pointer to TCAS_pOrderedIndex that is going to hold the compressed TCAS Ordered Index
 * @param pOrderedIndexCount pointer to the amount of output ordered indexs, equal to chunks, can be NULL
 * @param pId pointer to the id which will hold the current id of the last index it created
 * @param pOffset pointer to the offset which will hold the current offset of the last index it created
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_create_ordered_index_from_stream(const tcas_unit *tcasBuf, tcas_u32 chunks, tcas_u32 lastId, tcas_u32 lastOffset, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, TCAS_pOrderedIndex *ppOrderedIndexs, tcas_u32 *pOrderedIndexCount, tcas_u32 *pId, tcas_u32 *pOffset);

/**
 * Linearize ordered index to linear index streams.
 * @param pOrderedIndexs the address of TCAS_OrderedIndex array that is going to be linearized
 * @param indexs the amount of input indexs
 * @param indexStreams a pointer to Vector in which several index streams that contains linear indexs will be hold, indexStreams = NULL means that it is the first time that the function be invoked, and you should assign the return value of this function to it, and in the later invokation you should pass that value as parameter
 * @return TCAS_IndexStreamsPtr
 */
extern TCAS_IndexStreamsPtr libtcas_linearize_ordered_indexs(TCAS_pOrderedIndex pOrderedIndexs, tcas_u32 indexs, TCAS_IndexStreamsPtr indexStreams);

extern TCAS_IndexStreamsPtr libtcas_linearize_ordered_indexs_deprecated(TCAS_pOrderedIndex pOrderedIndexs, tcas_u32 indexs, TCAS_IndexStreamsPtr indexStreams);

/**
 * Free the memory owned by TCAS_pIndex structure.
 * @param pIndex the address of TCAS_Index array that is going to be freed
 */
extern void libtcas_destroy_index(TCAS_pIndex pIndex);

/**
 * Free the memory owned by TCAS_pCompactedIndex structure.
 * @param pCompactedIndex the address of TCAS_CompactedIndex array that is going to be freed
 */
extern void libtcas_destroy_compacted_index(TCAS_pCompactedIndex pCompactedIndex);

/**
 * Free the memory owned by TCAS_pOrderedIndex structure.
 * @param pOrderedIndex the address of TCAS_OrderedIndex array that is going to be freed
 */
extern void libtcas_destroy_ordered_index(TCAS_pOrderedIndex pOrderedIndex);

/**
 * Add additional DIPs to pKeyFrameChunkStart or pKeyFrameChunkEnd. 
 * @param pKeyFrameChunkStart a pointer to TCAS_Chunk structure which contains the start key frame chunk, additional DIPs may add here or pKeyFrameChunkEnd
 * @param pKeyFrameChunkEnd a pointer to TCAS_Chunk structure which contains the end key frame chunk, additional DIPs may add here or pKeyFrameChunkStart
 * @return int -1 - error occurred, 0 - not changed at all, 1 - pKeyFrameChunkStart added additional DIPs, 2 - pKeyFrameChunkEnd added addtional DIPs
 */
extern int libtcas_key_frame_chunk_add_dips(TCAS_pChunk pKeyFrameChunkStart, TCAS_pChunk pKeyFrameChunkEnd);

/**
 * Create an intermediate TCAS frame chunk by two key frame chunks. 
 * Remark: before invoking this function you MUST use libtcas_key_frame_chunk_add_dips function(), 
 * and pass pKeyFrameChunkStart, pKeyFrameChunkEnd to it as parameters. Note that, this function uses 
 * linear interpolation and bilinear filtering (to smooth motion and avoid jerkiness)
 *
 * @see libtcas_generate_intermediate_frame_chunk_fast()
 * @param pKeyFrameChunkStart a pointer to TCAS_Chunk structure which contains the start key frame chunk
 * @param pKeyFrameChunkEnd a pointer to TCAS_Chunk structure which contains the end key frame chunk
 * @param first the start frame of the key frame
 * @param last the end frame of the key frame
 * @param n specify which frame is going to create
 * @param ppChunk a pointer to TCAS_pChunk that is going to hold the intermediate frame chunk
 * @return const TCAS_pChunk
 */
extern const TCAS_pChunk libtcas_generate_intermediate_frame_chunk(const TCAS_pChunk pKeyFrameChunkStart, const TCAS_pChunk pKeyFrameChunkEnd, tcas_s32 first, tcas_s32 last, tcas_s32 n, TCAS_pChunk *ppChunk);

/**
 * Create an intermediate TCAS frame chunk by two key frame chunks. 
 * Remark: before invoking this function you MUST use libtcas_key_frame_chunk_add_dips function(), 
 * and pass pKeyFrameChunkStart, pKeyFrameChunkEnd to it as parameters. Note that, this function does not 
 * use bilinear filtering, so it is must faster, but with jerkiness.
 *
 * @see libtcas_generate_intermediate_frame_chunk()
 * @param pKeyFrameChunkStart a pointer to TCAS_Chunk structure which contains the start key frame chunk
 * @param pKeyFrameChunkEnd a pointer to TCAS_Chunk structure which contains the end key frame chunk
 * @param first the start frame of the key frame
 * @param last the end frame of the key frame
 * @param n specify which frame is going to create
 * @param ppChunk a pointer to TCAS_pChunk that is going to hold the intermediate frame chunk
 * @return const TCAS_pChunk
 */
extern const TCAS_pChunk libtcas_generate_intermediate_frame_chunk_fast(const TCAS_pChunk pKeyFrameChunkStart, const TCAS_pChunk pKeyFrameChunkEnd, tcas_s32 first, tcas_s32 last, tcas_s32 n, TCAS_pChunk *ppChunk);

/**
 * Initialize the TCAS_ChunksCache. 
 * Remark: each stream will get one chunk cache as well as a key frame chunk pair cache.
 *
 * @param pChunksCache pointer to TCAS_ChunksCache which is going to be initialized
 * @param streamCount number of streams
 */
extern void libtcas_init_chunks_cache(TCAS_pChunksCache pChunksCache, unsigned long streamCount);

/**
 * Free the memory occupied by the TCAS_ChunksCache. 
 * @param pChunksCache pointer to TCAS_ChunksCache which is going to be freed
 */
extern void libtcas_free_chunks_cache(TCAS_pChunksCache pChunksCache);


/* Utility Functions */

extern int libtcas_check_normal_index_validity(const TCAS_pOrderedIndex src, const TCAS_pOrderedIndex key);

extern int libtcas_check_key_index_validity(const TCAS_pOrderedIndex start, const TCAS_pOrderedIndex end, const TCAS_pOrderedIndex key);

extern int libtcas_compare_order_layer(const TCAS_pChunk src, const TCAS_pChunk key);

extern long libtcas_chunk_index_binary_search(const TCAS_pCompactedIndex pParsedIndexs, tcas_u32 indexCount, const TCAS_pCompactedIndex pKey);


/* HIGH LEVEL APIs */

/**
 * Create a TCAS frame from compressed TCAS file with index. 
 * Remark: make sure that the file type is TCAS_FILE_TYPE_COMPRESSED, 
 * however, this function will check whether the file is TCAS_FILE_TYPE_COMPRESSED
 * The performance may be poor. Instead, please try linear index.
 *
 * @param pFile a pointer to TCAS_File structure
 * @param pHeader a pointer to TCAS_Header structure
 * @param pIndexs the address of TCAS_Index array which holds the parsed index of the compressed TCAS file, TCAS_Index.first means firstFrame and TCAS_Index.last means lastFrame
 * @param n specify which frame is going to create
 * @param targetWidth width of the target video
 * @param targetHeight height of the target video
 * @param pBuf a pointer to a block of memory which is going to hold the TCAS frame
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_create_frame_with_index(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_pIndex pIndexs, tcas_s32 n, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf);

/**
 * Create a TCAS frame from compressed TCAS file with compacted index. 
 * Remark: make sure that the file type is TCAS_FILE_TYPE_COMPRESSED, 
 * however, this function will check whether the file is TCAS_FILE_TYPE_COMPRESSED
 * The performance may be poor. Instead, please try linear index.
 * Note that key framing features are not surported by this function, 
 * and key frame chunks will be simply ignored.
 *
 * @param pFile a pointer to TCAS_File structure
 * @param pHeader a pointer to TCAS_Header structure
 * @param pCompactedIndexs the address of TCAS_CompactedIndex array which holds the parsed compacted index of the compressed TCAS file, TCAS_CompactedIndex.first means firstFrame and TCAS_CompactedIndex.last means lastFrame
 * @param n specify which frame is going to create
 * @param targetWidth width of the target video
 * @param targetHeight height of the target video
 * @param pBuf a pointer to a block of memory which is going to hold the TCAS frame
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_create_frame_with_compacted_index(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_pCompactedIndex pCompactedIndexs, tcas_s32 n, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf);

/**
 * Create a TCAS frame from compressed TCAS file with linearized index. 
 * Remark: make sure that the file type is TCAS_FILE_TYPE_COMPRESSED, 
 * whether to use keyframing or not can be configured through flag, 
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
 * @param targetWidth width of the target video
 * @param targetHeight height of the target video
 * @param pBuf a pointer to a block of memory which is going to hold the TCAS frame
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_create_frame_with_linearized_index(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_IndexStreamsPtr indexStreams, tcas_s32 n, TCAS_pChunksCache pChunksCache, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf);

/**
 * The only difference from libtcas_create_frame_with_linearized_index function is that, you should allocate the buf
 * and resizing of frame is not supported
 *
 * @see libtcas_create_frame_with_linearized_index()
 */
extern TCAS_Error_Code libtcas_fill_frame_with_linearized_index(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_IndexStreamsPtr indexStreams, tcas_s32 n, TCAS_pChunksCache pChunksCache, tcas_byte *buf, tcas_u16 width, tcas_u16 height);

/**
 * The only difference from libtcas_fill_frame_with_linearized_index is that it use in-stream stuff
 *
 * @see libtcas_create_frame_with_linearized_index
 */
extern TCAS_Error_Code libtcas_fill_frame_from_stream_with_linearized_index(const tcas_unit *tcasBuf, const TCAS_IndexStreamsPtr indexStreams, tcas_s32 n, TCAS_pChunksCache pChunksCache, tcas_byte *buf, tcas_u16 width, tcas_u16 height);

/**
 * Create a TCAS frame from compressed TCAS file with linearized index. 
 * Remark: make sure that the file type is TCAS_FILE_TYPE_PARSED, 
 * and the pParsedIndexs is generated by the libtcas_index_parsed_tcas_file function
 */
extern TCAS_Error_Code libtcas_create_frame_with_parsed_index(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_pCompactedIndex pParsedIndexs, tcas_u32 indexCount, tcas_s32 n, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* LIBTCAS_TCAS_H */

