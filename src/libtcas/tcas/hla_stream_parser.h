/*
 * hla_stream_parser.h -- High-level stream parser API of the 'libtcas' library
 * Copyright (C) 2012 milkyjing <milkyjing@gmail.com>
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

#ifndef LIBTCAS_HLA_STREAM_PARSER_H
#define LIBTCAS_HLA_STREAM_PARSER_H
#pragma once

#include "tcas.h"
#include "hla_z_comp.h"

typedef struct _tcas_stream_parser {
    const tcas_unit *tcasBuf;     /**< compressed tcas buf stream */
    tcas_u32 chunks;              /**< number of compressed chunks in the stream */
    TCAS_IndexStreamsPtr indexStreams;
    TCAS_ChunksCache chunksCache;
    tcas_u32 fpsNumerator;
    tcas_u32 fpsDenominator;
    tcas_u16 width;
    tcas_u16 height;
    tcas_s32 minFrame;
    tcas_s32 maxFrame;
} TCAS_StreamParser, *TCAS_pStreamParser;


/* Inhibit C++ name-mangling for libtcas functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */
    
/**
 * Initialize the stream parser struct.
 * Remark: the tcasBuf should be TCAS_FILE_TYPE_COMPRESSED
 *
 * @param tcasBuf pointer to compressed tcas buf stream
 * @param chunks number of compressed chunks in the stream
 * @param fpsNumerator FramePerSecond = fpsNumerator / (double)fpsDenominator, can be 0, in this case, fps will not be used, or can be -1 (& fpsDenominator == 1), then the TCAS fps will be used
 * @param fpsDenominator FramePerSecond = fpsNumerator / (double)fpsDenominator, can be 0, in this case, fps will not be used, or can be 1 (& fpsNumerator == -1), then the TCAS fps will be used
 * @param width width of the target video
 * @param height height of the target video
 * @param pParser a pointer to TCAS_pStreamParser structure, which is going to be initialized
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_stream_parser_init(const tcas_unit *tcasBuf, tcas_u32 chunks, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, tcas_u16 width, tcas_u16 height, TCAS_pStreamParser pParser);

/**
 * Parse non-linear TCAS_BUF to parsed TCAS_BUF.
 * Remark: from TCAS_FILE_TYPE_COMPRESSED to TCAS_FILE_TYPE_PARSED
 *
 * @param pParser a pointer to TCAS_StreamParser structure, which should have been initialized
 * @param chunksVector a pointer to the vector which contains the parsed chunks
 * @return 0 - on success
 */
extern int libtcas_stream_parser_parse(TCAS_pStreamParser pParser, VectorPtr chunksVector);

extern int libtcas_stream_parser_parse_z(TCAS_pStreamParser pParser, VectorPtr chunksVector);

/**
 * Finalize the framer facility.
 * @param pParser a pointer to TCAS_StreamParser structure, which should have been initialized
 */
extern void libtcas_stream_parser_fin(TCAS_pStreamParser pParser);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* LIBTCAS_HLA_STREAM_PARSER_H */

