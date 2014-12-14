/*
 * hla_framer.h -- High-level frame serving API of the 'libtcas' library
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

#ifndef LIBTCAS_HLA_FRAMER_H
#define LIBTCAS_HLA_FRAMER_H
#pragma once

#include "tcas.h"
#include "hla_z_comp.h"

typedef struct _tcas_framer {
    TCAS_File file;
    TCAS_Header header;
    TCAS_IndexStreamsPtr indexStreams;
    TCAS_ChunksCache chunksCache;
    TCAS_pCompactedIndex pParsedIndexs;    /* for parsed tcas file */
    tcas_u16 width;
    tcas_u16 height;
    tcas_s32 minFrame;
    tcas_s32 maxFrame;
} TCAS_Framer, *TCAS_pFramer;


/* Inhibit C++ name-mangling for libtcas functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */
    
/**
 * Initialize the framer struct
 * @param filename specify the file name which is to be opened
 * @param fpsNumerator FramePerSecond = fpsNumerator / (double)fpsDenominator, can be 0, in this case, fps will not be used, or can be -1 (& fpsDenominator == 1), then the TCAS fps will be used
 * @param fpsDenominator FramePerSecond = fpsNumerator / (double)fpsDenominator, can be 0, in this case, fps will not be used, or can be 1 (& fpsNumerator == -1), then the TCAS fps will be used
 * @param width width of the target video
 * @param height height of the target video
 * @param pFramer a pointer to TCAS_Framer structure, which is going to be initialized
 * @return TCAS_Error_Code
 */
extern TCAS_Error_Code libtcas_framer_init(const char *filename, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, tcas_u16 width, tcas_u16 height, TCAS_pFramer pFramer);

/**
 * Get the specified frame.
 * @param pFramer a pointer to TCAS_pFramer structure, which should have been initialized
 * @param n frame index which is wanted
 * @param pBuf a pointer to the buffer which is going to hold the tcas frame data
 * @return 0 - on success
 */
extern int libtcas_framer_get(TCAS_pFramer pFramer, tcas_s32 n, tcas_byte **pBuf);

/**
 * Finalize the framer facility.
 * @param pFramer a pointer to TCAS_pFramer structure, which should have been initialized
 */
extern void libtcas_framer_fin(TCAS_pFramer pFramer);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* LIBTCAS_HLA_FRAMER_H */

