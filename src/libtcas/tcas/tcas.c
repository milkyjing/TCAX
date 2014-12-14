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

#include "tcas.h"

#ifdef _MSC_VER
#pragma warning(disable: 4996)    /* disable warnings for fopen function() */
#pragma warning(disable: 4799)    /* disable warnings for emms */
#endif    /* _MSC_VER */


tcas_s32 libtcas_MulDiv(tcas_s32 a, tcas_s32 b, tcas_s32 c) {
    int s;
    tcas_s32 d;
    s = 1;
    if (a < 0) {
        a = -a;
        s = -1;
    }
    if (b < 0) {
        b = -b;
        s = -s;
    }
    if (c < 0) {
        c = -c;
        s = -s;
    }
    d = (tcas_s32)(c > 0 ? ((tcas_s64)a * b + (c >> 1)) / c : 0x7FFFFFFF);
    return s > 0 ? d : -d;
}

TCAS_Error_Code libtcas_open_file(TCAS_pFile pFile, const char *filename, TCAS_File_Open_Type type) {
    if (tcas_file_open_existing == type) {
        pFile->fp = fopen(filename, "rb");
        if (!pFile->fp) return tcas_error_file_cannot_open;
    } else if (tcas_file_create_new == type) {
        pFile->fp = fopen(filename, "wb");
        if (!pFile->fp) return tcas_error_file_cannot_create;
    } else if (tcas_file_read_write == type) {
        pFile->fp = fopen(filename, "r+b");
        if (!pFile->fp) return tcas_error_file_cannot_open;
    }
    return tcas_error_success;
}

void libtcas_close_file(TCAS_pFile pFile) {
    if (pFile->fp)
        fclose(pFile->fp);
}

TCAS_Error_Code libtcas_read(TCAS_pFile pFile, tcas_unit *buf, tcas_u32 count) {
    if (fread(buf, sizeof(tcas_unit), count, pFile->fp) != count) return tcas_error_file_while_reading;
    return tcas_error_success;
}

TCAS_Error_Code libtcas_write(TCAS_pFile pFile, const tcas_unit *buf, tcas_u32 count) {
    if (fwrite(buf, sizeof(tcas_unit), count, pFile->fp) != count) return tcas_error_file_while_writing;
    return tcas_error_success;
}

tcas_bool libtcas_check_signature(const TCAS_pFile pFile) {
    fpos_t position;
    tcas_unit signature;
    fgetpos(pFile->fp, &position);    /* remember file position indicator */
    tcas_fseek(pFile->fp, 0, SEEK_SET);
    if (fread(&signature, sizeof(tcas_unit), 1, pFile->fp) != 1) {
        fsetpos(pFile->fp, &position);    /* reset file position indicator */
        return TCAS_ERROR;
    }
    fsetpos(pFile->fp, &position);    /* reset file position indicator */
    if (TCAS_SIGNATURE == signature) return TCAS_TRUE;
    return TCAS_FALSE;
}

void libtcas_set_file_position_indicator(TCAS_pFile pFile, TCAS_File_Position_Indicator position) {
    if (tcas_fpi_set == position) {
        tcas_fseek(pFile->fp, 0, SEEK_SET);
    } else if (tcas_fpi_header == position) {
        tcas_fseek(pFile->fp, sizeof(TCAS_Header), SEEK_SET);
    } else if (tcas_fpi_end == position) {
        tcas_fseek(pFile->fp, 0, SEEK_END);
    }
}

void libtcas_set_header(TCAS_pHeader pHeader, tcas_u16 type, tcas_u16 keyframing, tcas_u16 width, tcas_u16 height, tcas_s32 minTime, tcas_s32 maxTime, tcas_unit chunks, tcas_unit fpsNumerator, tcas_unit fpsDenominator) {
    memset(pHeader, 0, sizeof(TCAS_Header));
    pHeader->signature = TCAS_SIGNATURE;
    pHeader->version = TCAS_VERSION;
    pHeader->flag = MAKE16B16B(keyframing, type);
    pHeader->resolution = MAKEPOS(width, height);
    pHeader->minTime = minTime;
    pHeader->maxTime = maxTime;
    pHeader->chunks = chunks;
    pHeader->fpsNumerator = fpsNumerator;
    pHeader->fpsDenominator = fpsDenominator;
}

TCAS_Error_Code libtcas_read_header(TCAS_pFile pFile, TCAS_pHeader pHeader, tcas_bool keepPosition) {
    if (keepPosition) {
        fpos_t position;
        fgetpos(pFile->fp, &position);    /* remember file position indicator */
        tcas_fseek(pFile->fp, 0, SEEK_SET);
        if (fread(pHeader, sizeof(TCAS_Header), 1, pFile->fp) != 1) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            return tcas_error_file_while_reading;
        }
        fsetpos(pFile->fp, &position);    /* reset file position indicator */
    } else {
        tcas_fseek(pFile->fp, 0, SEEK_SET);
        if (fread(pHeader, sizeof(TCAS_Header), 1, pFile->fp) != 1) return tcas_error_file_while_reading;
    }
    return tcas_error_success;
}

TCAS_Error_Code libtcas_write_header(TCAS_pFile pFile, const TCAS_pHeader pHeader, tcas_bool keepPosition) {
    if (keepPosition) {
        fpos_t position;
        fgetpos(pFile->fp, &position);    /* remember file position indicator */
        tcas_fseek(pFile->fp, 0, SEEK_SET);
        if (fwrite(pHeader, sizeof(TCAS_Header), 1, pFile->fp) != 1) {
            fsetpos(pFile->fp, &position);
            return tcas_error_file_while_writing;
        }
        fsetpos(pFile->fp, &position);    /* reset file position indicator */
    } else {
        tcas_fseek(pFile->fp, 0, SEEK_SET);
        if (fwrite(pHeader, sizeof(TCAS_Header), 1, pFile->fp) != 1) return tcas_error_file_while_writing;
    }
    return tcas_error_success;
}

TCAS_Error_Code libtcas_read_raw_chunks(TCAS_pFile pFile, TCAS_pRawChunk pRawChunks, tcas_u32 count) {
    if (fread(pRawChunks, sizeof(TCAS_RawChunk), count, pFile->fp) != count) return tcas_error_file_while_reading;
    return tcas_error_success;
}

TCAS_Error_Code libtcas_write_raw_chunks(TCAS_pFile pFile, const TCAS_pRawChunk pRawChunks, tcas_u32 count) {
    if (fwrite(pRawChunks, sizeof(TCAS_RawChunk), count, pFile->fp) != count) return tcas_error_file_while_writing;
    return tcas_error_success;
}

void libtcas_alloc_chunk(TCAS_pChunk pChunk, tcas_u32 count) {
    pChunk->pos_and_color = (tcas_unit *)malloc(count * (sizeof(tcas_unit) << 1));    /* every pos_and_color takes up 2 tcas_unit */
}

void libtcas_free_chunk(TCAS_pChunk pChunk) {
    free(pChunk->pos_and_color);
    memset(pChunk, 0, sizeof(TCAS_Chunk));
}

void libtcas_copy_chunk(TCAS_pChunk pChunk, const TCAS_pChunk pChunkSrc) {
    tcas_u32 size;  /* size of pos_and_color */
    size = GETCOUNT(pChunkSrc->cltp) * (sizeof(tcas_unit) << 1);
    memcpy(pChunk, pChunkSrc, sizeof(TCAS_Chunk));
    pChunk->pos_and_color = (tcas_unit *)malloc(size);
    memcpy(pChunk->pos_and_color, pChunkSrc->pos_and_color, size);
}

TCAS_Error_Code libtcas_read_chunk(TCAS_pFile pFile, TCAS_pChunk pChunk) {
    tcas_u32 count;
    if (fread(pChunk, sizeof(tcas_unit), 3, pFile->fp) != 3) return tcas_error_file_while_reading;    /* startTime endTime cltp takes up 3 tcas_unit */
    count = GETCOUNT(pChunk->cltp);
    pChunk->pos_and_color = (tcas_unit *)malloc(count * (sizeof(tcas_unit) << 1));    /* every pos_and_color takes up 2 tcas_unit */
    if (fread(pChunk->pos_and_color, sizeof(tcas_unit) << 1, count, pFile->fp) != count) {
        free(pChunk->pos_and_color);
        return tcas_error_file_while_reading;
    }
    return tcas_error_success;
}

TCAS_Error_Code libtcas_read_specified_chunk(TCAS_pFile pFile, tcas_u64 offset, TCAS_pChunk pChunk) {
    tcas_u32 count;
    tcas_fseek(pFile->fp, offset, SEEK_SET);
    if (fread(pChunk, sizeof(tcas_unit), 3, pFile->fp) != 3) return tcas_error_file_while_reading;    /* startTime endTime cltp takes up 3 tcas_unit */
    count = GETCOUNT(pChunk->cltp);
    pChunk->pos_and_color = (tcas_unit *)malloc(count * (sizeof(tcas_unit) << 1));    /* every pos_and_color takes up 2 tcas_unit */
    if (fread(pChunk->pos_and_color, sizeof(tcas_unit) << 1, count, pFile->fp) != count) {
        free(pChunk->pos_and_color);
        return tcas_error_file_while_reading;
    }
    return tcas_error_success;
}

TCAS_Error_Code libtcas_read_specified_chunk_from_stream(const tcas_unit *tcasBuf, tcas_u64 offset, TCAS_pChunk pChunk) {
    tcas_u32 count;
    memcpy(pChunk, (const tcas_byte *)tcasBuf + offset, 3 * sizeof(tcas_unit));
    count = GETCOUNT(pChunk->cltp);
    pChunk->pos_and_color = (tcas_unit *)malloc(count * (sizeof(tcas_unit) << 1));    /* every pos_and_color takes up 2 tcas_unit */
    memcpy(pChunk->pos_and_color, (const tcas_byte *)tcasBuf + offset + 3 * sizeof(tcas_unit), count * (sizeof(tcas_unit) << 1));
    return tcas_error_success;
}

TCAS_Error_Code libtcas_read_chunks(TCAS_pFile pFile, TCAS_pChunk pChunks, tcas_u32 count) {
    TCAS_Error_Code error;
    tcas_u32 i = 0;
    do error = libtcas_read_chunk(pFile, &pChunks[i++]);
    while (i < count && tcas_error_success == error);
    return error;
}

TCAS_Error_Code libtcas_write_chunk(TCAS_pFile pFile, const TCAS_pChunk pChunk) {
    int count;
    count = GETCOUNT(pChunk->cltp);
    if (fwrite(pChunk, sizeof(tcas_unit), 3, pFile->fp) != 3) return tcas_error_file_while_writing;   /* startTime endTime cltp takes up 3 tcas_unit */
    if (fwrite(pChunk->pos_and_color, sizeof(tcas_unit) << 1, count, pFile->fp) != count) return tcas_error_file_while_writing;    /* every pos_and_color takes up 2 tcas_unit */
    return tcas_error_success;
}

TCAS_Error_Code libtcas_write_chunks(TCAS_pFile pFile, const TCAS_pChunk pChunks, tcas_u32 count, tcas_bool freeChunks) {
    TCAS_Error_Code error;
    tcas_u32 i = 0;
    if (freeChunks) {
        do {
            error = libtcas_write_chunk(pFile, &pChunks[i]);
            libtcas_free_chunk(&pChunks[i]);
            i++;
        } while (i < count && tcas_error_success == error);
    } else {
        do error = libtcas_write_chunk(pFile, &pChunks[i++]);
        while (i < count && tcas_error_success == error);
    }
    return error;
}

const tcas_unit *libtcas_compress_raw_chunks(const TCAS_pRawChunk pRawChunks, tcas_u32 rawChunks, tcas_bool reallocBuf, tcas_unit **pBuf, tcas_u32 *pChunkCount, tcas_u32 *pUnitCount) {
    tcas_u32 i, count, compCount, singleCount;
    tcas_s32 prevStartTime, prevEndTime;
    tcas_unit prevTypePair, prevLayer;
    tcas_unit *compBuf;
    compBuf = (tcas_unit *)malloc(rawChunks * sizeof(TCAS_RawChunk));    /* allocate a block of large enough memory buffer */
    compBuf[0] = pRawChunks[0].startTime;
    compBuf[1] = pRawChunks[0].endTime;
    compBuf[2] = MAKECLTP(1, pRawChunks[0].layer, GETRAWTYPE(pRawChunks[0].type_pair), GETRAWPAIR(pRawChunks[0].type_pair));
    compBuf[3] = MAKEPOS(pRawChunks[0].posX, pRawChunks[0].posY);
    compBuf[4] = MAKERGBA(pRawChunks[0].r, pRawChunks[0].g, pRawChunks[0].b, pRawChunks[0].a);
    count       = 5;    /* amount of tcas_unit being used in compBuf, 5 == sizeof(TCAS_RawChunk) / sizeof(tcas_unit) */
    compCount   = 0;    /* the compressing times */
    singleCount = 0;    /* counter in a compressed chunk */
    prevStartTime = pRawChunks[0].startTime;
    prevEndTime   = pRawChunks[0].endTime;
    prevTypePair  = pRawChunks[0].type_pair;
    prevLayer     = pRawChunks[0].layer;
    for (i = 1; i < rawChunks; i++) {
        /* compress one time can save 3 * DWORDs space!
           In the compressing process, adjacent raw DIPs who have the same StartT EndT Layer frameType and pair bit can share the same chunk */
        if (prevStartTime == pRawChunks[i].startTime && prevEndTime == pRawChunks[i].endTime && prevTypePair == pRawChunks[i].type_pair && prevLayer == pRawChunks[i].layer) {
            compCount++;
            singleCount++;
            compBuf[5 * i - 3 * compCount + 3] = MAKEPOS(pRawChunks[i].posX, pRawChunks[i].posY);
            compBuf[5 * i - 3 * compCount + 4] = MAKERGBA(pRawChunks[i].r, pRawChunks[i].g, pRawChunks[i].b, pRawChunks[i].a);
            compBuf[5 * i - 3 * compCount - 2 * singleCount + 2]++;
            count += 2;
        } else {
            compBuf[5 * i - 3 * compCount + 0] = pRawChunks[i].startTime;
            compBuf[5 * i - 3 * compCount + 1] = pRawChunks[i].endTime;
            compBuf[5 * i - 3 * compCount + 2] = MAKECLTP(1, pRawChunks[i].layer, GETRAWTYPE(pRawChunks[i].type_pair), GETRAWPAIR(pRawChunks[i].type_pair));
            compBuf[5 * i - 3 * compCount + 3] = MAKEPOS(pRawChunks[i].posX, pRawChunks[i].posY);
            compBuf[5 * i - 3 * compCount + 4] = MAKERGBA(pRawChunks[i].r, pRawChunks[i].g, pRawChunks[i].b, pRawChunks[i].a);
            prevStartTime = pRawChunks[i].startTime;
            prevEndTime   = pRawChunks[i].endTime;
            prevTypePair  = pRawChunks[i].type_pair;
            prevLayer     = pRawChunks[i].layer;
            singleCount   = 0;
            count += 5;
        }
    }
    if (reallocBuf) compBuf = (tcas_unit *)realloc(compBuf, count * sizeof(tcas_unit));    /* sweep no-used memory */
    *pBuf = compBuf;
    *pChunkCount = rawChunks - compCount;
    *pUnitCount = count;
    return (const tcas_unit *)compBuf;
}

const TCAS_pChunk libtcas_convert_raw_chunks(const TCAS_pRawChunk pRawChunks, tcas_u32 rawChunks, TCAS_pChunk *ppChunks, tcas_u32 *pChunkCount) {
    tcas_unit *buf;
    tcas_u32 i, index, size, compChunks, units;
    TCAS_pChunk pChunks;
    libtcas_compress_raw_chunks(pRawChunks, rawChunks, TCAS_FALSE, &buf, &compChunks, &units);
    pChunks = (TCAS_pChunk)malloc(compChunks * sizeof(TCAS_Chunk));
    index = 0;
    for (i = 0; i < compChunks; i++) {
        pChunks[i].startTime = buf[index + 0];
        pChunks[i].endTime = buf[index + 1];
        pChunks[i].cltp = buf[index + 2];
        size = GETCOUNT(pChunks[i].cltp) * (sizeof(tcas_unit) << 1);
        pChunks[i].pos_and_color = (tcas_unit *)malloc(size);
        memcpy(pChunks[i].pos_and_color, &buf[index + 3], size);
        index += 3 + (GETCOUNT(pChunks[i].cltp) << 1);
    }
    free(buf);
    *ppChunks = pChunks;
    *pChunkCount = compChunks;
    return (const TCAS_pChunk)pChunks;
}

const tcas_unit *libtcas_uncompress_chunk(const TCAS_pChunk pChunk, tcas_unit **pBuf, tcas_u32 *pRawChunkCount, tcas_u32 *pUnitCount) {
    tcas_u32 i, count, size;    /* count equals *rawChunks */
    tcas_unit *buf;
    count = GETCOUNT(pChunk->cltp);
    size = count * sizeof(TCAS_RawChunk);
    buf = (tcas_unit *)malloc(size);
    for (i = 0; i < count; i++) {
        buf[5 * i + 0] = pChunk->startTime;
        buf[5 * i + 1] = pChunk->endTime;
        buf[5 * i + 2] = MAKE16B16B(GETLAYER(pChunk->cltp), MAKERAWTP(GETTYPE(pChunk->cltp), GETPAIR(pChunk->cltp)));
        buf[5 * i + 3] = pChunk->pos_and_color[2 * i + 0];
        buf[5 * i + 4] = pChunk->pos_and_color[2 * i + 1];
    }
    *pBuf = buf;
    *pRawChunkCount = count;
    *pUnitCount = size >> 2;
    return (const tcas_unit *)buf;
}

const TCAS_pRawChunk libtcas_convert_chunk(const TCAS_pChunk pChunk, TCAS_pRawChunk *ppRawChunks, tcas_u32 *pRawChunkCount) {
    tcas_u32 units;
    tcas_unit *buf;
    libtcas_uncompress_chunk(pChunk, &buf, pRawChunkCount, &units);
    *ppRawChunks = (TCAS_pRawChunk)buf;
    return (const TCAS_pRawChunk)buf;
}

const tcas_unit *libtcas_uncompress_chunks(TCAS_pChunk pChunks, tcas_u32 chunks, tcas_unit **pBuf, tcas_u32 *pRawChunkCount, tcas_u32 *pUnitCount, tcas_bool freeChunks) {
    tcas_u32 i, j, index, n, count, size;    /* count equals *rawChunks */
    tcas_unit *buf;
    count = 0;
    for (i = 0; i < chunks; i++) {
        count += GETCOUNT(pChunks[i].cltp);
    }
    size = count * sizeof(TCAS_RawChunk);
    buf = (tcas_unit *)malloc(size);
    index = 0;
    if (freeChunks) {
        for (i = 0; i < chunks; i++) {
            n = GETCOUNT(pChunks[i].cltp);
            for (j = 0; j < n; j++) {
                buf[5 * index + 0] = pChunks[i].startTime;
                buf[5 * index + 1] = pChunks[i].endTime;
                buf[5 * index + 2] = MAKE16B16B(GETLAYER(pChunks[i].cltp), MAKERAWTP(GETTYPE(pChunks[i].cltp), GETPAIR(pChunks[i].cltp)));
                buf[5 * index + 3] = pChunks[i].pos_and_color[2 * j + 0];
                buf[5 * index + 4] = pChunks[i].pos_and_color[2 * j + 1];
                index++;
            }
            libtcas_free_chunk(&pChunks[i]);
        }
    } else {
        for (i = 0; i < chunks; i++) {
            n = GETCOUNT(pChunks[i].cltp);
            for (j = 0; j < n; j++) {
                buf[5 * index + 0] = pChunks[i].startTime;
                buf[5 * index + 1] = pChunks[i].endTime;
                buf[5 * index + 2] = MAKE16B16B(GETLAYER(pChunks[i].cltp), MAKERAWTP(GETTYPE(pChunks[i].cltp), GETPAIR(pChunks[i].cltp)));
                buf[5 * index + 3] = pChunks[i].pos_and_color[2 * j + 0];
                buf[5 * index + 4] = pChunks[i].pos_and_color[2 * j + 1];
                index++;
            }
        }
    }
    *pBuf = buf;
    *pRawChunkCount = count;
    *pUnitCount = size >> 2;
    return (const tcas_unit *)buf;
}

const TCAS_pRawChunk libtcas_convert_chunks(TCAS_pChunk pChunks, tcas_u32 chunks, TCAS_pRawChunk *ppRawChunks, tcas_u32 *pRawChunkCount, tcas_bool freeChunks) {
    tcas_u32 units;
    tcas_unit *buf;
    libtcas_uncompress_chunks(pChunks, chunks, &buf, pRawChunkCount, &units, freeChunks);
    *ppRawChunks = (TCAS_pRawChunk)buf;
    return (const TCAS_pRawChunk)buf;
}

const TCAS_pChunk libtcas_convert_rgba_to_chunk(const tcas_byte *rgba, tcas_u16 width, tcas_u16 height, tcas_bool reallocBuf, TCAS_pChunk pChunk) {
    tcas_u16 h, w;
    tcas_u32 index, count, offset, size;
    pChunk->pos_and_color = (tcas_unit *)malloc(width * height * (sizeof(tcas_unit) << 1));    /* every pos_and_color occupies 2 tcas_unit */
    index = 0;
    for (h = 0; h < height; h++) {
        for (w = 0; w < width; w++) {
            offset = h * (width << 2) + (w << 2);
            if (0 != rgba[offset + 3]) {
                pChunk->pos_and_color[(index << 1)] = MAKEPOS(w, h);
                pChunk->pos_and_color[(index << 1) + 1] = *((const tcas_unit *)&rgba[offset]);
                index++;
            }
        }
    }
    count = index;
    size = count * (sizeof(tcas_unit) << 1);
    pChunk->cltp = MAKECLTP(count, GETLAYER(pChunk->cltp), GETTYPE(pChunk->cltp), GETPAIR(pChunk->cltp));
    if (reallocBuf) pChunk->pos_and_color = (tcas_unit *)realloc(pChunk->pos_and_color, size);
    return (const TCAS_pChunk)pChunk;
}

tcas_u32 libtcas_blend_color(tcas_u32 back, tcas_u32 over) {
    tcas_u8 r, g, b, a, r1, g1, b1, a1, r2, g2, b2, a2;
    r1 = GETR(back);
    g1 = GETG(back);
    b1 = GETB(back);
    a1 = GETA(back);
    r2 = GETR(over);
    g2 = GETG(over);
    b2 = GETB(over);
    a2 = GETA(over);
    a = 255 - (255 - a1) * (255 - a2) / 255;
    if (0 != a) {
        r = (r2 * a2 + r1 * a1 * (255 - a2) / 255) / a;
        g = (g2 * a2 + g1 * a1 * (255 - a2) / 255) / a;
        b = (b2 * a2 + b1 * a1 * (255 - a2) / 255) / a;
    } else {
        r = 0;
        g = 0;
        b = 0;
    }
    return MAKERGBA(r, g, b, a);
}

const tcas_byte *libtcas_convert_chunk_to_rgba(const TCAS_pChunk pChunk, tcas_u16 width, tcas_u16 height, tcas_byte **pRGBA) {
    tcas_s16 x, y;
    tcas_u32 i, pitch, size, xx, yy;
    tcas_byte r, g, b, a, r0, g0, b0, a0, A;
    tcas_byte *rgba;
    pitch = width * (sizeof(tcas_byte) << 2);
    size = height * pitch;
    rgba = (tcas_byte *)malloc(size);
    memset(rgba, 0, size);
    for (i = 0; i < GETCOUNT(pChunk->cltp); i++) {
        x = GETPOSX(pChunk->pos_and_color[i << 1]);
        y = GETPOSY(pChunk->pos_and_color[i << 1]);
        if (x < 0 || x >= width || y < 0 || y >= height) continue;
        r = GETR(pChunk->pos_and_color[(i << 1) + 1]);
        g = GETG(pChunk->pos_and_color[(i << 1) + 1]);
        b = GETB(pChunk->pos_and_color[(i << 1) + 1]);
        a = GETA(pChunk->pos_and_color[(i << 1) + 1]);
        if (0 == a) continue;
        yy = y * pitch;
        xx = x << 2;
        r0 = rgba[yy + xx];
        g0 = rgba[yy + xx + 1];
        b0 = rgba[yy + xx + 2];
        a0 = rgba[yy + xx + 3];
        A = 255 - (255 - a) * (255 - a0) / 255;
        rgba[yy + xx]     = (r * a + r0 * a0 * (255 - a) / 255) / A;
        rgba[yy + xx + 1] = (g * a + g0 * a0 * (255 - a) / 255) / A;
        rgba[yy + xx + 2] = (b * a + b0 * a0 * (255 - a) / 255) / A;
        rgba[yy + xx + 3] =  A;
    }
    *pRGBA = rgba;
    return (const tcas_byte *)rgba;
}

void libtcas_convert_chunks_to_rgba(const TCAS_pChunk pChunks, tcas_u16 width, tcas_u16 height, tcas_byte *rgba) {
    tcas_s16 x, y;
    tcas_u32 i, pitch, xx, yy;
    tcas_byte r, g, b, a, r0, g0, b0, a0, A;
    pitch = width << 2;
    for (i = 0; i < GETCOUNT(pChunks->cltp); i++) {
        x = GETPOSX(pChunks->pos_and_color[i << 1]);
        y = GETPOSY(pChunks->pos_and_color[i << 1]);
        if (x < 0 || x >= width || y < 0 || y >= height) continue;
        r = GETR(pChunks->pos_and_color[(i << 1) + 1]);
        g = GETG(pChunks->pos_and_color[(i << 1) + 1]);
        b = GETB(pChunks->pos_and_color[(i << 1) + 1]);
        a = GETA(pChunks->pos_and_color[(i << 1) + 1]);
        if (0 == a) continue;
        yy = y * pitch;
        xx = x << 2;
        r0 = rgba[yy + xx];
        g0 = rgba[yy + xx + 1];
        b0 = rgba[yy + xx + 2];
        a0 = rgba[yy + xx + 3];
        A = 255 - (255 - a) * (255 - a0) / 255;
        rgba[yy + xx]     = (r * a + r0 * a0 * (255 - a) / 255) / A;
        rgba[yy + xx + 1] = (g * a + g0 * a0 * (255 - a) / 255) / A;
        rgba[yy + xx + 2] = (b * a + b0 * a0 * (255 - a) / 255) / A;
        rgba[yy + xx + 3] =  A;
    }
}

/****************** Deprecated Resampling functions ******************/

void _libtcas_resample_rgba_nearest(const tcas_byte *src, tcas_u16 width, tcas_u16 height, tcas_byte **pRGBA, tcas_u16 targetWidth, tcas_u16 targetHeight) {
    tcas_u16 h, w;
    tcas_u32 pitch, targetPitch, targetSize, Sx, Sy, Dx, Dy;
    tcas_byte *dst;
    pitch = width * (sizeof(tcas_byte) << 2);
    targetPitch = targetWidth * (sizeof(tcas_byte) << 2);
    targetSize = targetHeight * targetPitch;
    dst = (tcas_byte *)malloc(targetSize);
    for (h = 0; h < targetHeight; h++) {
        Dy = h * targetPitch;    /* Destination buffer postion-y */
        Sy = (h * height / targetHeight) * pitch;  /* Source buffer postion-y */
        for (w = 0; w < targetWidth; w++) {
            Dx = w << 2;       /* Destination buffer postion-x */
            Sx = (w * width / targetWidth) << 2;    /* Source buffer postion-x */
            if (0 != src[Sy + Sx + 3]) {             /* we predict that there are a lot of transparent pixels */
                dst[Dy + Dx]     = src[Sy + Sx];
                dst[Dy + Dx + 1] = src[Sy + Sx + 1];
                dst[Dy + Dx + 2] = src[Sy + Sx + 2];
                dst[Dy + Dx + 3] = src[Sy + Sx + 3];
            }
        }
    }
    *pRGBA = dst;
}

static double _libtcas_filter_MitchellNetravali(double x, double b, double c) {
    double p0, p2, p3, q0, q1, q2, q3;
    p0 = (   6 -  2 * b          ) / 6.0;
    p2 = ( -18 + 12 * b +  6 * c ) / 6.0;
    p3 = (  12 -  9 * b -  6 * c ) / 6.0;
    q0 = (        8 * b + 24 * c ) / 6.0;
    q1 = (     - 12 * b - 48 * c ) / 6.0;
    q2 = (        6 * b + 30 * c ) / 6.0;
    q3 = (     -      b -  6 * c ) / 6.0;
    if (x < 0) x = -x;    /* x = fabs(x) */
    if (x < 1)
        return p0 + (p2 + p3 * x) * x * x;
    else if (x < 2)
        return q0 + (q1 + (q2 + q3 * x) * x) * x;
    else return 0;
}

static double _libtcas_filter_cubic(double x, double a) {
    if (x < 0) x = -x;    /* x = fabs(x) */
    if (x < 1) return (a + 2) * x * x * x - (a + 3) * x * x + 1;
    else if (x < 2) return a * x * x * x - 5 * a * x * x + 8 * a * x - 4 * a;
    else return 0;
}

static double _libtcas_filter_BSpline(double x) {
    if (x < -2) return 0;
    else if (x < -1) return (2 + x) * (2 + x) * (2 + x) / 6.0;
    else if (x < 0) return (4 + (-6 - 3 * x) * x * x) / 6.0;
    else if (x < 1) return (4 + (-6 + 3 * x) * x * x) / 6.0;
    else if (x < 2) return (2 - x) * (2 - x) * (2 - x) / 6.0;
    else return 0;
}

void _libtcas_resample_rgba_bicubic(const tcas_byte *src, tcas_u16 width, tcas_u16 height, tcas_byte **pRGBA, tcas_u16 targetWidth, tcas_u16 targetHeight) {
    int h, w, m, n, index;
    double fx, fy;
    int ix, iy, xx, yy;
    double xScale, yScale, r1, r2;
    double rr, gg, bb, aa;
    tcas_byte *rgba;
    xScale = targetWidth / (double)width;
    yScale = targetHeight / (double)height;
    rgba = (tcas_byte *)malloc(targetHeight * targetWidth * (sizeof(tcas_byte) << 2));
    for (h = 0; h < targetHeight; h++) {
        fy = h / yScale;
        iy = (int)fy;
        for (w = 0; w < targetWidth; w++) {
            fx = w / xScale;
            ix = (int)fx;
            rr = 0;
            gg = 0;
            bb = 0;
            aa = 0;
            for (m = 0; m < 4; m++) {
                yy = iy + m - 1;
                r1 = _libtcas_filter_cubic(yy - fy, -0.5);
                if (yy < 0) yy = 0;
                if (yy >= height) yy = height - 1;
                for (n = 0; n < 4; n++) {
                    xx = ix + n - 1;
                    r2 = r1 * _libtcas_filter_cubic(xx - fx, -0.5);
                    if (xx < 0) xx = 0;
                    if (xx >= width) xx = width - 1;
                    index = (yy * width + xx) << 2;
                    rr += src[index] * r2;
                    gg += src[index + 1] * r2;
                    bb += src[index + 2] * r2;
                    aa += src[index + 3] * r2;
                }
            }
            if (rr > 255) rr = 255;
            if (rr < 0) rr = 0;
            if (gg > 255) gg = 255;
            if (gg < 0) gg = 0;
            if (bb > 255) bb = 255;
            if (bb < 0) bb = 0;
            if (aa > 255) aa = 255;
            if (aa < 0) aa = 0;
            index = (h * targetWidth + w) << 2;
            rgba[index] = (unsigned char)rr;
            rgba[index + 1] = (unsigned char)gg;
            rgba[index + 2] = (unsigned char)bb;
            rgba[index + 3] = (unsigned char)aa;
        }
    }
    *pRGBA = rgba;
}

/****************** Optimized Resampling functions ******************/

__forceinline static int _clip_to_border(long width, long height, long *x, long *y) {
    int isIn = 1;
    if (*x < 0) {
        isIn = 0;
        *x = 0;
    } else if (*x >= width) {
        isIn = 0;
        *x = width - 1;
    }
    if (*y < 0) {
        isIn = 0;
        *y = 0;
    } else if (*y >= height) {
        isIn = 0;
        *y = height - 1;
    }
    return isIn;
}

__forceinline static tcas_u32 _get_pixels_border(const tcas_byte *buf, long width, long height, long x, long y) {
    int isInPic;
    long pitch;
    tcas_u32 color;
    isInPic = _clip_to_border(width, height, &x, &y);
    pitch = width << 2;
    color = ((tcas_u32 *)(buf + y * pitch))[x];
    if (!isInPic)
        color = MAKERGBA(GETR(color), GETG(color), GETB(color), 0);
    return color;
}

__forceinline static void  _bilinear_fast_MMX(tcas_u32 *pColor0, tcas_u32 *pColor1, unsigned long u_8, unsigned long v_8, tcas_u32 *result) {
    __asm {
        MOVD      MM6,v_8
        MOVD      MM5,u_8
        mov       edx,pColor0
        mov       eax,pColor1
        PXOR      mm7,mm7

        MOVD         MM2,dword ptr [eax]  
        MOVD         MM0,dword ptr [eax+4]
        PUNPCKLWD    MM5,MM5
        PUNPCKLWD    MM6,MM6
        MOVD         MM3,dword ptr [edx]  
        MOVD         MM1,dword ptr [edx+4]
        PUNPCKLDQ    MM5,MM5 
        PUNPCKLBW    MM0,MM7
        PUNPCKLBW    MM1,MM7
        PUNPCKLBW    MM2,MM7
        PUNPCKLBW    MM3,MM7
        PSUBw        MM0,MM2
        PSUBw        MM1,MM3
        PSLLw        MM2,8
        PSLLw        MM3,8
        PMULlw       MM0,MM5
        PMULlw       MM1,MM5
        PUNPCKLDQ    MM6,MM6 
        PADDw        MM0,MM2
        PADDw        MM1,MM3

        PSRLw        MM0,8
        PSRLw        MM1,8
        PSUBw        MM0,MM1
        PSLLw        MM1,8
        PMULlw       MM0,MM6
        mov       eax,result
        PADDw        MM0,MM1

        PSRLw        MM0,8
        PACKUSwb     MM0,MM7
        movd      [eax],MM0 
        //emms
    }
}

static void _bilinear_border_MMX(tcas_byte *buf, long width, long height, long x_16, long y_16, tcas_u32 *result) {
    long x, y;
    unsigned long u_16, v_16;
    tcas_u32 pixel[4];
    x = x_16 >> 16;
    y = y_16 >> 16;
    u_16 = (unsigned short)x_16;
    v_16 = (unsigned short)y_16;
    pixel[0] = _get_pixels_border(buf, width, height, x, y);
    pixel[1] = _get_pixels_border(buf, width, height, x + 1, y);
    pixel[2] = _get_pixels_border(buf, width, height, x, y + 1);
    pixel[3] = _get_pixels_border(buf, width, height, x + 1, y + 1);
    _bilinear_fast_MMX(&pixel[0], &pixel[2], u_16 >> 8, v_16 >> 8, result);
}

void libtcas_resample_rgba_bilinear_mmx(const tcas_byte *src, tcas_u16 width, tcas_u16 height, tcas_byte *dst, tcas_u16 targetWidth, tcas_u16 targetHeight) {
    long srcWidth, srcHeight, dstWidth, dstHeight, srcPitch, dstPitch;
    long xrIntFloat_16, yrIntFloat_16, csDErrorX, csDErrorY;
    long border_y0, border_x0, border_y1, border_x1;
    long srcx_16, srcy_16;
    long x, y;
    unsigned long v_8;
    tcas_u32 *pSrcLine, *pDstLine;
    tcas_u32 *pSrcLineColor;
    tcas_u32 *pColor0, *pColor1;
    srcWidth = width;
    srcHeight = height;
    dstWidth = targetWidth;
    dstHeight = targetHeight;
    xrIntFloat_16 = (srcWidth << 16) / dstWidth + 1;
    yrIntFloat_16 = (srcHeight << 16) / dstHeight + 1;
    csDErrorX = -(1 << 15) + (xrIntFloat_16 >> 1);
    csDErrorY = -(1 << 15) + (yrIntFloat_16 >> 1);
    border_y0 = -csDErrorY / yrIntFloat_16 + 1;       // y0 + y * yr >= 0; y0 = csDErrorY => y >= -csDErrorY / yr
    if (border_y0 >= dstHeight)
        border_y0 = dstHeight;
    border_x0 = -csDErrorX / xrIntFloat_16 + 1;
    if (border_x0 >= dstWidth)
        border_x0 = dstWidth;
    border_y1 = (((srcHeight - 2) << 16) - csDErrorY) / yrIntFloat_16 + 1;   // y0 + y * yr <= (height - 2) => y <= (height - 2 - csDErrorY) / yr
    if (border_y1 < border_y0)
        border_y1 = border_y0;
    border_x1 = (((srcWidth - 2) << 16) - csDErrorX) / xrIntFloat_16 + 1;
    if (border_x1 < border_x0)
        border_x1 = border_x0;
    srcPitch = srcWidth * (sizeof(tcas_byte) << 2);
    dstPitch = dstWidth * (sizeof(tcas_byte) << 2);
    pSrcLine = (tcas_u32 *)src;
    pDstLine = (tcas_u32 *)dst;
    srcy_16 = csDErrorY;
    for (y = 0; y < border_y0; y++) {
        srcx_16 = csDErrorX;
        for (x = 0; x < dstWidth; x++) {
            _bilinear_border_MMX((tcas_byte *)pSrcLine, srcWidth, srcHeight, srcx_16, srcy_16, (tcas_u32 *) &pDstLine[x]); //border
            srcx_16 += xrIntFloat_16;
        }
        srcy_16 += yrIntFloat_16;
        pDstLine = (tcas_u32 *)((tcas_byte *)pDstLine + dstPitch);
    }
    for (y = border_y0; y < border_y1; y++) {
        srcx_16 = csDErrorX;
        for (x=0; x < border_x0; x++) {
            _bilinear_border_MMX((tcas_byte *)pSrcLine, srcWidth, srcHeight, srcx_16, srcy_16, (tcas_u32 *) &pDstLine[x]);//border
            srcx_16 += xrIntFloat_16;
        }
        {
            v_8 = (srcy_16 & 0xFFFF) >> 8;
            pSrcLineColor = (tcas_u32 *)((tcas_byte *)pSrcLine + srcPitch * (srcy_16 >> 16));
            for (x = border_x0; x < border_x1; x++) {
                pColor0 = &pSrcLineColor[srcx_16 >> 16];
                pColor1 = (tcas_u32 *)((tcas_byte *)pColor0 + srcPitch);
                _bilinear_fast_MMX(pColor0, pColor1, (srcx_16 & 0xFFFF) >> 8, v_8, (tcas_u32 *) &pDstLine[x]);
                srcx_16 += xrIntFloat_16;
            }
        }
        for (x = border_x1; x < dstWidth; x++) {
            _bilinear_border_MMX((tcas_byte *)pSrcLine, srcWidth, srcHeight, srcx_16, srcy_16, (tcas_u32 *) &pDstLine[x]);//border
            srcx_16 += xrIntFloat_16;
        }
        srcy_16 += yrIntFloat_16;
        pDstLine = (tcas_u32 *)((tcas_byte *)pDstLine + dstPitch);
    }
    for (y = border_y1; y < dstHeight; y++) {
        srcx_16 = csDErrorX;
        for (x = 0; x < dstWidth; x++) {
            _bilinear_border_MMX((tcas_byte *)pSrcLine, srcWidth, srcHeight, srcx_16, srcy_16, (tcas_u32 *) &pDstLine[x]); //border
            srcx_16 += xrIntFloat_16;
        }
        srcy_16 += yrIntFloat_16;
        pDstLine = (tcas_u32 *)((tcas_byte *)pDstLine + dstPitch);
    }
    __asm emms
}

/**********************************************/

void libtcas_get_chunk_min_max_pos(const TCAS_pChunk pChunk, tcas_s16 *pMinPosX, tcas_s16 *pMinPosY, tcas_s16 *pMaxPosX, tcas_s16 *pMaxPosY) {
    tcas_u32 i;
    tcas_s16 minX, minY, maxX, maxY;  /* when storing position we use tcas_u16, when manipulating position we should use tcas_s16 */
    minX = TCAS_INIT_MIN_POSX;
    minY = TCAS_INIT_MIN_POSY;
    maxX = TCAS_INIT_MAX_POSX;
    maxY = TCAS_INIT_MAX_POSY;
    for (i = 0; i < GETCOUNT(pChunk->cltp); i++) {
        if (TCAS_INVALID_POS == pChunk->pos_and_color[i << 1]) continue;
        minX = __min(minX, GETPOSX(pChunk->pos_and_color[i << 1]));
        minY = __min(minY, GETPOSY(pChunk->pos_and_color[i << 1]));
        maxX = __max(maxX, GETPOSX(pChunk->pos_and_color[i << 1]));
        maxY = __max(maxY, GETPOSY(pChunk->pos_and_color[i << 1]));
    }
    *pMinPosX = minX;
    *pMinPosY = minY;
    *pMaxPosX = maxX;
    *pMaxPosY = maxY;
}

void libtcas_get_raw_chunks_min_max_time(const TCAS_pRawChunk pRawChunks, tcas_u32 rawChunks, TCAS_pHeader pHeader) {
    tcas_u32 i;
    for (i = 0; i < rawChunks; i++) {
        pHeader->minTime = __min(pHeader->minTime, pRawChunks[i].startTime);
        pHeader->maxTime = __max(pHeader->maxTime, pRawChunks[i].endTime);
    }
}

void libtcas_get_chunks_min_max_time(const TCAS_pChunk pChunks, tcas_u32 chunks, tcas_s32 *pMinTime, tcas_s32 *pMaxTime) {
    tcas_u32 i;
    tcas_s32 minTime, maxTime;
    minTime = TCAS_INIT_MIN_TIME;
    maxTime = TCAS_INIT_MAX_TIME;
    for (i = 0; i < chunks; i++) {
        minTime = __min(minTime, pChunks[i].startTime);
        maxTime = __max(maxTime, pChunks[i].endTime);
    }
    *pMinTime = minTime;
    *pMaxTime = maxTime;
}

void libtcas_get_chunk_stream_min_max_time(const tcas_unit *tcasBuf, tcas_u32 chunks, tcas_s32 *pMinTime, tcas_s32 *pMaxTime) {
    tcas_u32 i, offset;
    tcas_s32 minTime, maxTime;
    minTime = TCAS_INIT_MIN_TIME;
    maxTime = TCAS_INIT_MAX_TIME;
    offset = 0;
    for (i = 0; i < chunks; i++) {
        minTime = __min(minTime, (tcas_s32)tcasBuf[offset]);
        maxTime = __max(maxTime, (tcas_s32)tcasBuf[offset + 1]);
        offset += 3 + (GETCOUNT(tcasBuf[offset + 2]) << 1);
    }
    *pMinTime = minTime;
    *pMaxTime = maxTime;
}

TCAS_Error_Code libtcas_count_chunks(const TCAS_pFile pFile, tcas_unit *pChunkCount) {
    TCAS_Error_Code error;
    fpos_t position;
    tcas_u32 count;    /* the same as *chunks */
    tcas_unit buf[1];    /* to hold cltp */
    TCAS_Header header;
    fgetpos(pFile->fp, &position);    /* remember file position indicator */
    error = libtcas_read_header(pFile, &header, TCAS_FALSE);
    if (tcas_error_success != error) {
        fsetpos(pFile->fp, &position);    /* reset file position indicator */
        return error;
    }
    if (TCAS_FILE_TYPE_COMPRESSED != GETHI16B(header.flag)) {
        fsetpos(pFile->fp, &position);    /* reset file position indicator */
        return tcas_error_file_type_not_match;
    }
    tcas_fseek(pFile->fp, 2 * sizeof(tcas_unit), SEEK_CUR);    /* reach the first cltp */
    count = 0;
    while (1) {
        if (fread(buf, sizeof(tcas_unit), 1, pFile->fp) != 1) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            *pChunkCount = count;
            return tcas_error_success;
        }
        if (feof(pFile->fp) == 0) {
            tcas_fseek(pFile->fp, (2 + (GETCOUNT(buf[0]) << 1)) * sizeof(tcas_unit), SEEK_CUR);
            count++;
        } else break;
    }
    fsetpos(pFile->fp, &position);    /* reset file position indicator */
    *pChunkCount = count;
    return tcas_error_success;
}

TCAS_Error_Code libtcas_get_min_max_time_and_chunks(const TCAS_pFile pFile, tcas_s32 *pMinTime, tcas_s32 *pMaxTime, tcas_unit *pChunkCount) {
    TCAS_Error_Code error;
    fpos_t position;
    tcas_s32 mintime, maxtime;
    tcas_u32 count;    /* count is the same as *chunks */
    tcas_unit buf[3];    /* to hold startTime endTime cltp */
    TCAS_Header header;
    mintime = TCAS_INIT_MIN_TIME;
    maxtime = TCAS_INIT_MAX_TIME;
    count = 0;
    fgetpos(pFile->fp, &position);    /* remember file position indicator */
    error = libtcas_read_header(pFile, &header, TCAS_FALSE);
    if (tcas_error_success != error) {
        fsetpos(pFile->fp, &position);    /* reset file position indicator */
        return error;
    }
    if (TCAS_FILE_TYPE_COMPRESSED != GETHI16B(header.flag) && TCAS_FILE_TYPE_PARSED != GETHI16B(header.flag)) {
        fsetpos(pFile->fp, &position);    /* reset file position indicator */
        return tcas_error_file_type_not_match;
    }
    while (1) {
        if (fread(buf, sizeof(tcas_unit), 3, pFile->fp) != 3) {   /* startTime endTime cltp takes up 3 tcas_unit */
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            *pMinTime = mintime;
            *pMaxTime = maxtime;
            *pChunkCount = count;
            return tcas_error_success;
        }
        if (feof(pFile->fp) == 0) {
            tcas_fseek(pFile->fp, (GETCOUNT(buf[2]) << 1) * sizeof(tcas_unit), SEEK_CUR);
            mintime = __min(mintime, (tcas_s32)buf[0]);
            maxtime = __max(maxtime, (tcas_s32)buf[1]);
            count++;
        } else break;
    }
    fsetpos(pFile->fp, &position);    /* reset file position indicator */
    *pMinTime = mintime;
    *pMaxTime = maxtime;
    *pChunkCount = count;
    return tcas_error_success;
}

TCAS_Error_Code libtcas_index_compressed_tcas_file(const TCAS_pFile pFile, const TCAS_pHeader pHeader, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, TCAS_pIndex *ppIndexs, tcas_u32 *pIndexCount) {
    TCAS_Error_Code error;
    fpos_t position;
    TCAS_pIndex pIndexs;
    tcas_u8 maxLayer;    /* record the max layer of chunks */
    tcas_u32 i, offset, chunks;
    tcas_unit buf[3];    /* a temp buffer to hold startTime endTime cltp */
    fgetpos(pFile->fp, &position);    /* remember file position indicator */
    if (NULL == pHeader) {
        TCAS_Header header;
        error = libtcas_read_header(pFile, &header, TCAS_FALSE);
        if (tcas_error_success != error) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            return error;
        }
        if (TCAS_FILE_TYPE_COMPRESSED != GETHI16B(header.flag)) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            return tcas_error_file_type_not_match;
        }
        chunks = header.chunks;    /* get the amount of chunks */
    } else {
        if (TCAS_FILE_TYPE_COMPRESSED != GETHI16B(pHeader->flag)) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            return tcas_error_file_type_not_match;
        }
        libtcas_set_file_position_indicator(pFile, tcas_fpi_header);
        chunks = pHeader->chunks;    /* get the amount of chunks */
    }
    pIndexs = (TCAS_pIndex)malloc(chunks * sizeof(TCAS_Index));
    maxLayer = (tcas_u8)0;
    offset = TCAS_HEADER_OFFSET;
    if (0 == fpsNumerator || 0 == fpsDenominator) {
        for (i = 0; i < chunks; i++) {
            if (fread(buf, sizeof(tcas_unit), 3, pFile->fp) != 3) {   /* startTime endTime cltp takes up 3 tcas_unit */
                free(pIndexs);
                fsetpos(pFile->fp, &position);    /* reset file position indicator */
                return tcas_error_file_while_reading;
            }
            pIndexs[i].first = buf[0];    /* startTime */
            pIndexs[i].last = buf[1];    /* endTime */
            pIndexs[i].cltp = buf[2];
            pIndexs[i].offset = offset;
            maxLayer = __max(maxLayer, GETLAYER(pIndexs[i].cltp));
            offset += 3 + (GETCOUNT(pIndexs[i].cltp) << 1);
            tcas_fseek(pFile->fp, (GETCOUNT(pIndexs[i].cltp) << 1) * sizeof(tcas_unit), SEEK_CUR);
        }
    } else {
        for (i = 0; i < chunks; i++) {
            if (fread(buf, sizeof(tcas_unit), 3, pFile->fp) != 3) {   /* startTime endTime cltp takes up 3 tcas_unit */
                free(pIndexs);
                fsetpos(pFile->fp, &position);    /* reset file position indicator */
                return tcas_error_file_while_reading;
            }
            pIndexs[i].first = libtcas_MulDiv((tcas_s32)buf[0], fpsNumerator, fpsDenominator * 1000) + 1;    /* firstFrame, note: +1 is just intend to make it compatible with VSFilter */
            pIndexs[i].last = libtcas_MulDiv((tcas_s32)buf[1], fpsNumerator, fpsDenominator * 1000) + 1;    /* lastFrame, note: +1 is just intend to make it compatible with VSFilter */
            pIndexs[i].cltp = buf[2];
            pIndexs[i].offset = offset;
            maxLayer = __max(maxLayer, GETLAYER(pIndexs[i].cltp));
            offset += 3 + (GETCOUNT(pIndexs[i].cltp) << 1);
            tcas_fseek(pFile->fp, (GETCOUNT(pIndexs[i].cltp) << 1) * sizeof(tcas_unit), SEEK_CUR);
        }
    }
    fsetpos(pFile->fp, &position);    /* reset file position indicator */
    if (0 != maxLayer) {    /* order chunks by layers */
        tcas_u32 size;
        tcas_u32 *layerChunks, *layerIndex;
        TCAS_pIndex pNewIndexs;
        size = (maxLayer + 1) * sizeof(tcas_u32);
        layerChunks = (tcas_u32 *)malloc(size);   /* count the number of chunks in each layer */
        memset(layerChunks, 0, size);
        for (i = 0; i < chunks; i++) {
            layerChunks[GETLAYER(pIndexs[i].cltp)]++;
        }
        layerIndex = (tcas_u32 *)malloc(size);    /* index of each layer */
        layerIndex[0] = 0;
        for (i = 1; i <= maxLayer; i++) {  /* the higher the layer is, the bigger the i will be */
            layerIndex[i] = layerIndex[i - 1] + layerChunks[i - 1];  /* make room for higer layers, lower layers take up array first */
        }
        free(layerChunks);
        pNewIndexs = (TCAS_pIndex)malloc(chunks * sizeof(TCAS_Index));
        for (i = 0; i < chunks; i++) {
            pNewIndexs[layerIndex[GETLAYER(pIndexs[i].cltp)]++] = pIndexs[i];    /* note: each chunk has a TCAS_Index structure */
        }
        free(layerIndex);
        free(pIndexs);
        pIndexs = pNewIndexs;
    }
    *ppIndexs = pIndexs;
    if (pIndexCount)
        *pIndexCount = chunks;
    return tcas_error_success;
}

TCAS_Error_Code libtcas_index_parsed_tcas_file(const TCAS_pFile pFile, const TCAS_pHeader pHeader, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, TCAS_pCompactedIndex *ppParsedIndexs, tcas_u32 *pIndexCount) {
    TCAS_Error_Code error;
    fpos_t position;
    TCAS_pCompactedIndex pParsedIndexs;
    tcas_u32 i, offset, chunks;
    tcas_unit buf[3];    /* a temp buffer to hold startTime endTime cltp */
    fgetpos(pFile->fp, &position);    /* remember file position indicator */
    if (NULL == pHeader) {
        TCAS_Header header;
        error = libtcas_read_header(pFile, &header, TCAS_FALSE);
        if (tcas_error_success != error) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            return error;
        }
        if (TCAS_FILE_TYPE_PARSED != GETHI16B(header.flag)) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            return tcas_error_file_type_not_match;
        }
        chunks = header.chunks;    /* get the amount of chunks */
    } else {
        if (TCAS_FILE_TYPE_PARSED != GETHI16B(pHeader->flag)) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            return tcas_error_file_type_not_match;
        }
        libtcas_set_file_position_indicator(pFile, tcas_fpi_header);
        chunks = pHeader->chunks;    /* get the amount of chunks */
    }
    pParsedIndexs = (TCAS_pCompactedIndex)malloc(chunks * sizeof(TCAS_CompactedIndex));
    offset = TCAS_HEADER_OFFSET;
    if (0 == fpsNumerator || 0 == fpsDenominator) {
        for (i = 0; i < chunks; i++) {
            if (fread(buf, sizeof(tcas_unit), 3, pFile->fp) != 3) {   /* startTime endTime cltp takes up 3 tcas_unit */
                free(pParsedIndexs);
                fsetpos(pFile->fp, &position);    /* reset file position indicator */
                return tcas_error_file_while_reading;
            }
            pParsedIndexs[i].first = buf[0];    /* startTime */
            pParsedIndexs[i].last = buf[1];     /* endTime */
            pParsedIndexs[i].offset = offset;
            offset += 3 + (GETCOUNT(buf[2]) << 1);
            tcas_fseek(pFile->fp, (GETCOUNT(buf[2]) << 1) * sizeof(tcas_unit), SEEK_CUR);
        }
    } else {
        for (i = 0; i < chunks; i++) {
            if (fread(buf, sizeof(tcas_unit), 3, pFile->fp) != 3) {   /* startTime endTime cltp takes up 3 tcas_unit */
                free(pParsedIndexs);
                fsetpos(pFile->fp, &position);    /* reset file position indicator */
                return tcas_error_file_while_reading;
            }
            pParsedIndexs[i].first = libtcas_MulDiv((tcas_s32)buf[0], fpsNumerator, fpsDenominator * 1000) + 1;    /* firstFrame, note: +1 is just intend to make it compatible with VSFilter */
            pParsedIndexs[i].last = libtcas_MulDiv((tcas_s32)buf[1], fpsNumerator, fpsDenominator * 1000) + 1;    /* lastFrame, note: +1 is just intend to make it compatible with VSFilter */
            pParsedIndexs[i].offset = offset;
            offset += 3 + (GETCOUNT(buf[2]) << 1);
            tcas_fseek(pFile->fp, (GETCOUNT(buf[2]) << 1) * sizeof(tcas_unit), SEEK_CUR);
        }
    }
    fsetpos(pFile->fp, &position);    /* reset file position indicator */
    *ppParsedIndexs = pParsedIndexs;
    if (pIndexCount)
        *pIndexCount = chunks;
    return tcas_error_success;
}

void libtcas_compact_index(const TCAS_pIndex pIndexs, tcas_u32 indexs, TCAS_pCompactedIndex *ppCompactedIndexs) {
    unsigned long i;
    TCAS_pCompactedIndex pCompactedIndexs;
    pCompactedIndexs = (TCAS_pCompactedIndex)malloc(indexs * sizeof(TCAS_CompactedIndex));
    for (i = 0; i < indexs; i++) {
        pCompactedIndexs->first = pIndexs->first;
        pCompactedIndexs->last = pIndexs->last;
        pCompactedIndexs->offset = pIndexs->offset;
    }
    *ppCompactedIndexs = pCompactedIndexs;
}

TCAS_Error_Code libtcas_create_ordered_index(const TCAS_pFile pFile, const TCAS_pHeader pHeader, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, TCAS_pOrderedIndex *ppOrderedIndexs, tcas_u32 *pOrderedIndexCount) {
    TCAS_Error_Code error;
    fpos_t position;
    TCAS_pOrderedIndex pOrderedIndexs;
    tcas_u32 i, offset, chunks;
    tcas_unit buf[3];    /* a temp buffer to hold startTime endTime cltp */
    fgetpos(pFile->fp, &position);    /* remember file position indicator */
    if (NULL == pHeader) {
        TCAS_Header header;
        error = libtcas_read_header(pFile, &header, TCAS_FALSE);
        if (tcas_error_success != error) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            return error;
        }
        if (TCAS_FILE_TYPE_COMPRESSED != GETHI16B(header.flag)) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            return tcas_error_file_type_not_match;
        }
        chunks = header.chunks;    /* get the amount of chunks */
    } else {
        if (TCAS_FILE_TYPE_COMPRESSED != GETHI16B(pHeader->flag)) {
            fsetpos(pFile->fp, &position);    /* reset file position indicator */
            return tcas_error_file_type_not_match;
        }
        libtcas_set_file_position_indicator(pFile, tcas_fpi_header);
        chunks = pHeader->chunks;    /* get the amount of chunks */
    }
    pOrderedIndexs = (TCAS_pOrderedIndex)malloc(chunks * sizeof(TCAS_OrderedIndex));
    offset = TCAS_HEADER_OFFSET;
    if (0 == fpsNumerator || 0 == fpsDenominator) {
        for (i = 0; i < chunks; i++) {
            if (fread(buf, sizeof(tcas_unit), 3, pFile->fp) != 3) {   /* startTime endTime cltp takes up 3 tcas_unit */
                free(pOrderedIndexs);
                fsetpos(pFile->fp, &position);    /* reset file position indicator */
                return tcas_error_file_while_reading;
            }
            pOrderedIndexs[i].first = buf[0];    /* startTime */
            pOrderedIndexs[i].last = buf[1];     /* endTime */
            if (pOrderedIndexs[i].first == pOrderedIndexs[i].last && 0 == GETTYPE(buf[2]))  /* a normal chunk which has no effect */
                pOrderedIndexs[i].id_pair = MAKEINDEXIP(0, 1);  /* will not be used */
            else
                pOrderedIndexs[i].id_pair = MAKEINDEXIP(i + 1, GETPAIR(buf[2]));            /* used to reserve the order of chunks, note that valid id starts from 1 */
            pOrderedIndexs[i].offset = offset;
            offset += 3 + (GETCOUNT(buf[2]) << 1);
            tcas_fseek(pFile->fp, (GETCOUNT(buf[2]) << 1) * sizeof(tcas_unit), SEEK_CUR);
        }
    } else {
        for (i = 0; i < chunks; i++) {
            if (fread(buf, sizeof(tcas_unit), 3, pFile->fp) != 3) {   /* startTime endTime cltp takes up 3 tcas_unit */
                free(pOrderedIndexs);
                fsetpos(pFile->fp, &position);    /* reset file position indicator */
                return tcas_error_file_while_reading;
            }
            pOrderedIndexs[i].first = libtcas_MulDiv((tcas_s32)buf[0], fpsNumerator, fpsDenominator * 1000) + 1;    /* firstFrame, note: +1 is just intend to make it compatible with VSFilter */
            pOrderedIndexs[i].last = libtcas_MulDiv((tcas_s32)buf[1], fpsNumerator, fpsDenominator * 1000) + 1;    /* lastFrame, note: +1 is just intend to make it compatible with VSFilter */
            if (pOrderedIndexs[i].first == pOrderedIndexs[i].last && 0 == GETTYPE(buf[2]))  /* a normal chunk which has no effect */
                pOrderedIndexs[i].id_pair = MAKEINDEXIP(0, 1);  /* will not be used */
            else
                pOrderedIndexs[i].id_pair = MAKEINDEXIP(i + 1, GETPAIR(buf[2]));            /* used to reserve the order of chunks */
            pOrderedIndexs[i].offset = offset;
            offset += 3 + (GETCOUNT(buf[2]) << 1);
            tcas_fseek(pFile->fp, (GETCOUNT(buf[2]) << 1) * sizeof(tcas_unit), SEEK_CUR);
        }
    }
    fsetpos(pFile->fp, &position);    /* reset file position indicator */
    *ppOrderedIndexs = pOrderedIndexs;
    if (pOrderedIndexCount)
        *pOrderedIndexCount = chunks;
    return tcas_error_success;
}

TCAS_Error_Code libtcas_create_ordered_index_from_stream(const tcas_unit *tcasBuf, tcas_u32 chunks, tcas_u32 lastId, tcas_u32 lastOffset, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, TCAS_pOrderedIndex *ppOrderedIndexs, tcas_u32 *pOrderedIndexCount, tcas_u32 *pId, tcas_u32 *pOffset) {
    TCAS_pOrderedIndex pOrderedIndexs;
    tcas_u32 i, offset;
    pOrderedIndexs = (TCAS_pOrderedIndex)malloc(chunks * sizeof(TCAS_OrderedIndex));
    offset = lastOffset;
    if (0 == fpsNumerator || 0 == fpsDenominator) {
        for (i = 0; i < chunks; i++) {
            pOrderedIndexs[i].first = tcasBuf[offset];    /* startTime */
            pOrderedIndexs[i].last = tcasBuf[offset + 1];       /* endTime */
            if (pOrderedIndexs[i].first == pOrderedIndexs[i].last && 0 == GETTYPE(tcasBuf[offset + 2]))  /* a normal chunk which has no effect */
                pOrderedIndexs[i].id_pair = MAKEINDEXIP(0, 1);  /* will not be used */
            else
                pOrderedIndexs[i].id_pair = MAKEINDEXIP(lastId + 1 + i, GETPAIR(tcasBuf[offset + 2]));             /* used to reserve the order of chunks */
            pOrderedIndexs[i].offset = offset;
            offset += 3 + (GETCOUNT(tcasBuf[offset + 2]) << 1);
        }
    } else {
        for (i = 0; i < chunks; i++) {
            pOrderedIndexs[i].first = libtcas_MulDiv((tcas_s32)tcasBuf[offset], fpsNumerator, fpsDenominator * 1000) + 1;    /* firstFrame, note: +1 is just intend to make it compatible with VSFilter */
            pOrderedIndexs[i].last = libtcas_MulDiv((tcas_s32)tcasBuf[offset + 1], fpsNumerator, fpsDenominator * 1000) + 1;    /* lastFrame, note: +1 is just intend to make it compatible with VSFilter */
            if (pOrderedIndexs[i].first == pOrderedIndexs[i].last && 0 == GETTYPE(tcasBuf[offset + 2]))  /* a normal chunk which has no effect */
                pOrderedIndexs[i].id_pair = MAKEINDEXIP(0, 1);  /* will not be used */
            else
                pOrderedIndexs[i].id_pair = MAKEINDEXIP(lastId + 1 + i, GETPAIR(tcasBuf[offset + 2]));             /* used to reserve the order of chunks */
            pOrderedIndexs[i].offset = offset;
            offset += 3 + (GETCOUNT(tcasBuf[offset + 2]) << 1);
        }
    }
    *ppOrderedIndexs = pOrderedIndexs;
    if (pOrderedIndexCount)
        *pOrderedIndexCount = chunks;
    if (pId)
        *pId = lastId + chunks;
    if (pOffset)
        *pOffset = offset;
    return tcas_error_success;
}

/* cleanup function for Vector
 * Note that, since our element of indexStreams is linearIndexs, the cleanup function takes the pointer of an element, 
 * so the parameter should be TCAS_LinearIndexsPtr *
 */
static void _vector_clean_linear_indexs(TCAS_LinearIndexsPtr *pLinearIndexs) {  /* We got the address of linearIndexs */
    vector_clear(*pLinearIndexs);  /* We need the content of linearIndexs */
}

/*
Comment: We can also use
void _vector_clean_linear_indexs(long pLinearIndexs) {
    vector_clear(*(TCAS_LinearIndexsPtr *)pLinearIndexs);
}
the reason why we cannot use 
void _vector_clean_linear_chunks(TCAS_LinearIndexsPtr pLinearIndexs) {
    vector_clear(pLinearIndexs);
}
is that with or without a `*' in the parameter declaration the value of
pLinearIndexs will indeed always be the address of linearIndexs, the only 
difference between the two is the type of pLinearIndexs!
So with the same invokation, functions of method 1 and 3 are completely different!
But why we used to consider them may have the same function? The reason is that 
we always invoke them in different ways, eg. foo1(&a) and foo2(a)
*/

/** in this function we use low copy of linear indexs (just have a copy of TCAS_LinearIndexsPtr other than its content), 
 *  since we won't delete them by TCAS_LinearIndexsPtr, hence increasing performance. 
 *  But we do use cleanup function to clean up all the memory occupied by TCAS_IndexStreamsPtr.
 *
 * Remark: 2011-12-18, implemented a better algorithm to linearize index using the red-black tree, so this function is deprecated.
 */
TCAS_IndexStreamsPtr libtcas_linearize_ordered_indexs_deprecated(TCAS_pOrderedIndex pOrderedIndexs, tcas_u32 indexs, TCAS_IndexStreamsPtr indexStreams) {
    tcas_u32 i, j, count;
    tcas_s32 last;
    VectorPtr vptr;  /* to hold the `last' data member value of a ordered index of a linear index stream */
    TCAS_LinearIndexsPtr linearIndexsTemp;  /* temporary use */
    vector_create(&vptr, sizeof(tcas_s32), 0, NULL, NULL);  /* default copy and cleanup functions will be used */
    if (!indexStreams)
        vector_create(&indexStreams, sizeof(TCAS_LinearIndexsPtr), 0, NULL, (vector_cleanup_func)_vector_clean_linear_indexs);  /* every index stream holds several linear indexs */
    for (i = 0; i < indexs; i++) {
        count = vector_get_count(indexStreams);
        for (j = 0; j < count; j++) {
            vector_retrieve(vptr, j, &last);
            if (pOrderedIndexs[i].first >= last) {
                vector_retrieve(indexStreams, j, &linearIndexsTemp);
                vector_push_back(linearIndexsTemp, &pOrderedIndexs[i]);
                if (pOrderedIndexs[i].first == pOrderedIndexs[i].last && pOrderedIndexs[i].id_pair != MAKEINDEXIP(0, 1))  /* is a key frame chunk */
                    vector_push_back(linearIndexsTemp, &pOrderedIndexs[++i]);
                vector_assign(vptr, j, &pOrderedIndexs[i].last);
                break;
            }
        }
        if (j == count) {
            vector_create(&linearIndexsTemp, sizeof(TCAS_OrderedIndex), 1, NULL, NULL);  /* use default copy and cleanup functions, since there is no pointer in TCAS_OrderedIndex */
            vector_assign(linearIndexsTemp, 0, &pOrderedIndexs[i]);
            vector_push_back(indexStreams, &linearIndexsTemp);  /* put the value of linearIndexsTemp itself not the value it pointed to */
            if (pOrderedIndexs[i].first == pOrderedIndexs[i].last && pOrderedIndexs[i].id_pair != MAKEINDEXIP(0, 1))  /* is a key frame chunk */
                vector_push_back(linearIndexsTemp, &pOrderedIndexs[++i]);
            vector_push_back(vptr, &pOrderedIndexs[i].last);
        }
    }
    vector_destroy(vptr);
    return indexStreams;
}

/* Linearization of chunk index Algorithm Optimized */
typedef struct __tcas_last {
    tcas_u32 id;        /* index of the index stream of the IndexStreams */
    tcas_s32 last;      /* the 'last' data member value of an ordered index of the linear index stream */
} _TCAS_Last, *_TCAS_pLast;

static int _rb_compare(const _TCAS_pLast a, const _TCAS_pLast b, const void *param) {
    if (a->id == b->id)     /* the only condition for the two are the same node */
        return 0;
    else {
        if (a->last < b->last)
            return -1;
        else if (a->last > b->last)
            return 1;
        else {      /* the same 'last' value */
            if (a->id < b->id)
                return -1;
            else
                return 1;
        }
    }
}

static void _rb_clean(_TCAS_pLast p, const void *param) {
    free(p);
}

TCAS_IndexStreamsPtr libtcas_linearize_ordered_indexs(TCAS_pOrderedIndex pOrderedIndexs, tcas_u32 indexs, TCAS_IndexStreamsPtr indexStreams) {
    tcas_u32 i;
    _TCAS_Last minIndexStream;  /* to hold the `last' data member value of a ordered index of a linear index stream */
    _TCAS_pLast pLast;
    RbTreePtr tree;
    TCAS_LinearIndexsPtr linearIndexsTemp;  /* temporary use */
    if (!indexStreams)
        vector_create(&indexStreams, sizeof(TCAS_LinearIndexsPtr), 0, NULL, (vector_cleanup_func)_vector_clean_linear_indexs);  /* every index stream holds several linear indexs */
    i = 0;
    vector_create(&linearIndexsTemp, sizeof(TCAS_OrderedIndex), 1, NULL, NULL);  /* use default copy and cleanup functions, since there is no pointer in TCAS_OrderedIndex */
    vector_assign(linearIndexsTemp, 0, &pOrderedIndexs[i]);
    vector_push_back(indexStreams, &linearIndexsTemp);  /* put the value of linearIndexsTemp itself not the value it pointed to */
    if (pOrderedIndexs[i].first == pOrderedIndexs[i].last && pOrderedIndexs[i].id_pair != MAKEINDEXIP(0, 1))  /* is a key frame chunk */
        vector_push_back(linearIndexsTemp, &pOrderedIndexs[++i]);
    tree = rbtree_create((rb_compare_func)_rb_compare, NULL, (rb_cleanup_func)_rb_clean, NULL);
    pLast = (_TCAS_pLast)malloc(sizeof(_TCAS_Last));
    pLast->id = 0;      /* the first index stream in indexStreams */
    pLast->last = pOrderedIndexs[i].last;
    rbtree_insert(tree, pLast);
    minIndexStream.id = pLast->id;
    minIndexStream.last = pLast->last;
    i++;        /* bug-fix, don't forget to increase the index! */
    for (; i < indexs; i++) {
        if (pOrderedIndexs[i].first >= minIndexStream.last) {
            vector_retrieve(indexStreams, minIndexStream.id, &linearIndexsTemp);
            vector_push_back(linearIndexsTemp, &pOrderedIndexs[i]);
            if (pOrderedIndexs[i].first == pOrderedIndexs[i].last && pOrderedIndexs[i].id_pair != MAKEINDEXIP(0, 1))  /* is a key frame chunk */
                vector_push_back(linearIndexsTemp, &pOrderedIndexs[++i]);
            /* maintain the tree */
            rbtree_delete(tree, &minIndexStream);       /* because the 'last' value of the index stream (minIndexStream) has been changed, so we should remove it, then add a new one */
            pLast = (_TCAS_pLast)malloc(sizeof(_TCAS_Last));
            pLast->id = minIndexStream.id;      /* they are of the same index stream */
            pLast->last = pOrderedIndexs[i].last;   /* but the 'last' value has been changed */
            rbtree_insert(tree, pLast);
            /* maintain the minIndexStream, get the very bottom-left node */
            pLast = (_TCAS_pLast)rbtree_min(tree);
            minIndexStream.id = pLast->id;
            minIndexStream.last = pLast->last;
        } else {
            vector_create(&linearIndexsTemp, sizeof(TCAS_OrderedIndex), 1, NULL, NULL);  /* use default copy and cleanup functions, since there is no pointer in TCAS_OrderedIndex */
            vector_assign(linearIndexsTemp, 0, &pOrderedIndexs[i]);
            vector_push_back(indexStreams, &linearIndexsTemp);  /* put the value of linearIndexsTemp itself not the value it pointed to */
            if (pOrderedIndexs[i].first == pOrderedIndexs[i].last && pOrderedIndexs[i].id_pair != MAKEINDEXIP(0, 1))  /* is a key frame chunk */
                vector_push_back(linearIndexsTemp, &pOrderedIndexs[++i]);
            /* maintain the tree */
            pLast = (_TCAS_pLast)malloc(sizeof(_TCAS_Last));
            pLast->id = vector_get_count(indexStreams) - 1;
            pLast->last = pOrderedIndexs[i].last;
            rbtree_insert(tree, pLast);
            /* maintain the minIndexStream, get the very bottom-left node */
            if (pLast->last < minIndexStream.last) {
                minIndexStream.id = pLast->id;
                minIndexStream.last = pLast->last;
            }
        }
    }
    rbtree_destroy(tree);
    return indexStreams;
}

void libtcas_destroy_index(TCAS_pIndex pIndex) {
    free(pIndex);
}

void libtcas_destroy_compacted_index(TCAS_pCompactedIndex pCompactedIndex) {
    free(pCompactedIndex);
}

void libtcas_destroy_ordered_index(TCAS_pOrderedIndex pOrderedIndex) {
    free(pOrderedIndex);
}

/* implementation of strategy sequence & nearest (others are sequence & average, random & nearest, random & average, which are not implemented yet */
int libtcas_key_frame_chunk_add_dips(TCAS_pChunk pKeyFrameChunkStart, TCAS_pChunk pKeyFrameChunkEnd) {
    int retFlag;
    TCAS_pChunk pChunkLessSide;  /* the key frame chunk of the two which has less DIPs */
    tcas_u32 i, count, lessCount, addDipCount, iRan;
    tcas_unit *posColorNew;  /* to hold new pos_and_color */
    if (!(GETTYPE(pKeyFrameChunkStart->cltp) == GETTYPE(pKeyFrameChunkEnd->cltp) && GETLAYER(pKeyFrameChunkStart->cltp) == GETLAYER(pKeyFrameChunkEnd->cltp))) return -1;  /* key frame type and layer must be the same */
    if (GETCOUNT(pKeyFrameChunkStart->cltp) == GETCOUNT(pKeyFrameChunkEnd->cltp)) return 0;  /* no need to add additional DIPs */
    if (GETCOUNT(pKeyFrameChunkStart->cltp) < GETCOUNT(pKeyFrameChunkEnd->cltp)) {
        pChunkLessSide = pKeyFrameChunkStart;
        count = GETCOUNT(pKeyFrameChunkEnd->cltp);
        lessCount = GETCOUNT(pKeyFrameChunkStart->cltp);
        retFlag = 1;
    } else {
        pChunkLessSide = pKeyFrameChunkEnd;
        count = GETCOUNT(pKeyFrameChunkStart->cltp);
        lessCount = GETCOUNT(pKeyFrameChunkEnd->cltp);
        retFlag = 2;
    }
    addDipCount = count - lessCount;  /* number of DIPs that should be added to make the two chunks have the same number of DIPs */
    posColorNew = (tcas_unit *)malloc(count * (sizeof(tcas_unit) << 1));
    memcpy(posColorNew, pChunkLessSide->pos_and_color, lessCount * (sizeof(tcas_unit) << 1));
    for (i = lessCount; i < count; i++) {
        iRan = (rand() % lessCount) << 1;
        posColorNew[i << 1] = pChunkLessSide->pos_and_color[iRan];
        posColorNew[(i << 1) + 1] = pChunkLessSide->pos_and_color[iRan + 1] & 0x00FFFFFF;
    }
    pChunkLessSide->cltp = MAKECLTP(count, GETLAYER(pChunkLessSide->cltp), GETTYPE(pChunkLessSide->cltp), GETPAIR(pChunkLessSide->cltp));
    free(pChunkLessSide->pos_and_color);
    pChunkLessSide->pos_and_color = posColorNew;
    return retFlag;
}

/* utilities */
static const tcas_byte *_libtcas_convert_pos_and_color_to_rgba(const tcas_unit *pos_and_color, tcas_u32 count, tcas_s16 minPosX, tcas_s16 minPosY, tcas_s16 maxPosX, tcas_s16 maxPosY, tcas_byte **pRGBA) {
    tcas_s16 x, y;
    tcas_u16 width, height;
    tcas_u32 i, i1, i2, pitch, size, xx, yy;
    tcas_byte r, g, b, a, r0, g0, b0, a0, A;
    tcas_byte *rgba;
    width = maxPosX - minPosX + 1;
    height = maxPosY - minPosY + 1;
    pitch = width * (sizeof(tcas_byte) << 2);
    size = height * pitch;
    rgba = (tcas_byte *)malloc(size);
    memset(rgba, 0, size);
    for (i = 0; i < count; i++) {
        i1 = i << 1;
        i2 = i1 + 1;
        x = GETPOSX(pos_and_color[i1]) - minPosX;
        y = GETPOSY(pos_and_color[i1]) - minPosY;
        r = GETR(pos_and_color[i2]);
        g = GETG(pos_and_color[i2]);
        b = GETB(pos_and_color[i2]);
        a = GETA(pos_and_color[i2]);
        if (0 == a) continue;
        yy = y * pitch;
        xx = (tcas_u32)x << 2;
        r0 = rgba[yy + xx];
        g0 = rgba[yy + xx + 1];
        b0 = rgba[yy + xx + 2];
        a0 = rgba[yy + xx + 3];
        A = 255 - (255 - a) * (255 - a0) / 255;
        rgba[yy + xx]     = (r * a + r0 * a0 * (255 - a) / 255) / A;
        rgba[yy + xx + 1] = (g * a + g0 * a0 * (255 - a) / 255) / A;
        rgba[yy + xx + 2] = (b * a + b0 * a0 * (255 - a) / 255) / A;
        rgba[yy + xx + 3] =  A;
    }
    if (pRGBA)
        *pRGBA = rgba;
    return (const tcas_byte *)rgba;
}

/* rgba1 rgba2 rgba3 rgba4 will be freed */
static const tcas_byte *_libtcas_filter_rgba(tcas_byte *rgba1, tcas_byte *rgba2, tcas_byte *rgba3, tcas_byte *rgba4, tcas_u16 width, tcas_u16 height, double xf, double yf, tcas_byte **pRGBA) {
    tcas_u16 w, h;
    tcas_u32 pitch, xx, yy;
    pitch = width * (sizeof(tcas_byte) << 2);
    for (h = 0; h < height; h++) {
        yy = h * pitch;
        for (w = 0; w < width; w++) {
            xx = w << 2;
            rgba1[yy + xx]     = (tcas_byte)((rgba1[yy + xx]     * (1 - xf) + rgba2[yy + xx] * xf)     * (1 - yf) + (rgba3[yy + xx]     * (1 - xf) + rgba4[yy + xx]     * xf) * yf);
            rgba1[yy + xx + 1] = (tcas_byte)((rgba1[yy + xx + 1] * (1 - xf) + rgba2[yy + xx + 1] * xf) * (1 - yf) + (rgba3[yy + xx + 1] * (1 - xf) + rgba4[yy + xx + 1] * xf) * yf);
            rgba1[yy + xx + 2] = (tcas_byte)((rgba1[yy + xx + 2] * (1 - xf) + rgba2[yy + xx + 2] * xf) * (1 - yf) + (rgba3[yy + xx + 2] * (1 - xf) + rgba4[yy + xx + 2] * xf) * yf);
            rgba1[yy + xx + 3] = (tcas_byte)((rgba1[yy + xx + 3] * (1 - xf) + rgba2[yy + xx + 3] * xf) * (1 - yf) + (rgba3[yy + xx + 3] * (1 - xf) + rgba4[yy + xx + 3] * xf) * yf);
        }
    }
    free(rgba2);
    free(rgba3);
    free(rgba4);
    if (pRGBA)
        *pRGBA = rgba1;
    return (const tcas_byte *)rgba1;
}

/* pos_and_color must have the enough space to contain the all potential DIPs */
static tcas_u32 _libtcas_convert_rgba_to_pos_and_color(const tcas_byte *rgba, tcas_s16 minPosX, tcas_s16 minPosY, tcas_s16 maxPosX, tcas_s16 maxPosY, tcas_unit *pos_and_color, tcas_u32 *pCount) {
    tcas_s16 x, y;
    tcas_u16 w, h, width, height;
    tcas_u32 count, pitch, xx, yy;
    width = maxPosX - minPosX + 1;
    height = maxPosY - minPosY + 1;
    pitch = width * (sizeof(tcas_byte) << 2);
    count = 0;
    for (h = 0; h < height; h++) {
        y = h + minPosY;
        yy = h * pitch;
        for (w = 0; w < width; w++) {
            x = w + minPosX;
            xx = w << 2;
            if (0 == rgba[yy + xx + 3]) continue;
            pos_and_color[count << 1] = MAKEPOS(x, y);
            pos_and_color[(count << 1) + 1] = MAKERGBA(rgba[yy + xx], rgba[yy + xx + 1], rgba[yy + xx + 2], rgba[yy + xx + 3]);
            count++;
        }
    }
    if (pCount)
        *pCount = count;
    return count;
}

/* implementation of strategy sequence & nearest (others are sequence & average, random & nearest, random & average, which are not implemented yet */
const TCAS_pChunk libtcas_generate_intermediate_frame_chunk(const TCAS_pChunk pKeyFrameChunkStart, const TCAS_pChunk pKeyFrameChunkEnd, tcas_s32 first, tcas_s32 last, tcas_s32 n, TCAS_pChunk *ppChunk) {
    TCAS_pChunk pChunk;
    if (last == first) {
        pChunk = (TCAS_pChunk)malloc(sizeof(TCAS_Chunk));
        libtcas_copy_chunk(pChunk, pKeyFrameChunkStart);
    } else {
        tcas_u32 i, i1, i2, count;
        double x, y, factor, xf, yf;
        tcas_s16 ix, iy, minPosX, minPosY, maxPosX, maxPosY;
        tcas_unit pos, color;
        tcas_unit *pos_and_color_1, *pos_and_color_2, *pos_and_color_3, *pos_and_color_4, *pos_and_color;
        tcas_byte *rgba1, *rgba2, *rgba3, *rgba4, *rgba;
        factor = (n - first) / (double)(last - first);
        x = GETPOSX(pKeyFrameChunkStart->pos_and_color[0]) + (GETPOSX(pKeyFrameChunkEnd->pos_and_color[0]) - GETPOSX(pKeyFrameChunkStart->pos_and_color[0])) * factor;
        y = GETPOSY(pKeyFrameChunkStart->pos_and_color[0]) + (GETPOSY(pKeyFrameChunkEnd->pos_and_color[0]) - GETPOSY(pKeyFrameChunkStart->pos_and_color[0])) * factor;
        xf = x - (int)x;
        yf = y - (int)y;
        count = GETCOUNT(pKeyFrameChunkStart->cltp);
        pChunk = (TCAS_pChunk)malloc(sizeof(TCAS_Chunk));
        pChunk->startTime = n;  /* actually startTime stores frame index not time */
        pChunk->endTime = n;    /* actually endTime stores frame index not time */
        if (x == (int)x && y == (int)y) {  /* no need to apply bilinear filtering */
            pChunk->cltp = pKeyFrameChunkStart->cltp;
            pChunk->pos_and_color = (tcas_unit *)malloc(count * (sizeof(tcas_unit) << 1));
            for (i = 0; i < count; i++) {
                i1 = i << 1;
                i2 = i1 + 1;
                pChunk->pos_and_color[i1] = LINEAR_INTERPOLATION_POS(pKeyFrameChunkStart->pos_and_color[i1], pKeyFrameChunkEnd->pos_and_color[i1], factor);
                pChunk->pos_and_color[i2] = LINEAR_INTERPOLATION_RGBA(pKeyFrameChunkStart->pos_and_color[i2], pKeyFrameChunkEnd->pos_and_color[i2], factor);
            }
        } else {  /* do the bilinear filtering */
            minPosX = TCAS_INIT_MIN_POSX;
            minPosY = TCAS_INIT_MIN_POSY;
            maxPosX = TCAS_INIT_MAX_POSX;
            maxPosY = TCAS_INIT_MAX_POSY;
            pos_and_color_1 = (tcas_unit *)malloc(count * (sizeof(tcas_unit) << 1));
            pos_and_color_2 = (tcas_unit *)malloc(count * (sizeof(tcas_unit) << 1));
            pos_and_color_3 = (tcas_unit *)malloc(count * (sizeof(tcas_unit) << 1));
            pos_and_color_4 = (tcas_unit *)malloc(count * (sizeof(tcas_unit) << 1));
            for (i = 0; i < count; i++) {
                i1 = i << 1;
                i2 = i1 + 1;
                pos = LINEAR_INTERPOLATION_POS(pKeyFrameChunkStart->pos_and_color[i1], pKeyFrameChunkEnd->pos_and_color[i1], factor);
                color = LINEAR_INTERPOLATION_RGBA(pKeyFrameChunkStart->pos_and_color[i2], pKeyFrameChunkEnd->pos_and_color[i2], factor);
                ix = GETPOSX(pos);
                iy = GETPOSY(pos);
                minPosX = __min(minPosX, ix);
                minPosY = __min(minPosY, iy);
                maxPosX = __max(maxPosX, ix + 1);
                maxPosY = __max(maxPosY, iy + 1);
                pos_and_color_1[i1] = MAKEPOS(ix, iy);
                pos_and_color_1[i2] = color;
                pos_and_color_2[i1] = MAKEPOS(ix + 1, iy);
                pos_and_color_2[i2] = color;
                pos_and_color_3[i1] = MAKEPOS(ix, iy + 1);
                pos_and_color_3[i2] = color;
                pos_and_color_4[i1] = MAKEPOS(ix + 1, iy + 1);
                pos_and_color_4[i2] = color;
            }
            _libtcas_convert_pos_and_color_to_rgba(pos_and_color_1, count, minPosX, minPosY, maxPosX, maxPosY, &rgba1);  /* merge the DIPs with the same position */
            free(pos_and_color_1);
            _libtcas_convert_pos_and_color_to_rgba(pos_and_color_2, count, minPosX, minPosY, maxPosX, maxPosY, &rgba2);  /* merge the DIPs with the same position */
            free(pos_and_color_2);
            _libtcas_convert_pos_and_color_to_rgba(pos_and_color_3, count, minPosX, minPosY, maxPosX, maxPosY, &rgba3);  /* merge the DIPs with the same position */
            free(pos_and_color_3);
            _libtcas_convert_pos_and_color_to_rgba(pos_and_color_4, count, minPosX, minPosY, maxPosX, maxPosY, &rgba4);  /* merge the DIPs with the same position */
            free(pos_and_color_4);
            _libtcas_filter_rgba(rgba1, rgba2, rgba3, rgba4, maxPosX - minPosX + 1, maxPosY - minPosY + 1, xf, yf, &rgba);  /* rgba1, rgba2, rgba3, rgba4 will be freed within the function */
            pos_and_color = (tcas_unit *)malloc((count << 2) * (sizeof(tcas_unit) << 1));  /* should be large enough to contain all the potential DIPs */
            _libtcas_convert_rgba_to_pos_and_color(rgba, minPosX, minPosY, maxPosX, maxPosY, pos_and_color, &count);  /* convert back */
            free(rgba);
            pChunk->cltp = MAKECLTP(count, GETLAYER(pKeyFrameChunkStart->cltp), GETTYPE(pKeyFrameChunkStart->cltp), GETPAIR(pKeyFrameChunkStart->cltp));
            pChunk->pos_and_color = pos_and_color;
        }
    }
    if (ppChunk)
        *ppChunk = pChunk;
    return (const TCAS_pChunk)pChunk;
}

/* implementation of strategy sequence & nearest (others are sequence & average, random & nearest, random & average, which are not implemented yet */
const TCAS_pChunk libtcas_generate_intermediate_frame_chunk_fast(const TCAS_pChunk pKeyFrameChunkStart, const TCAS_pChunk pKeyFrameChunkEnd, tcas_s32 first, tcas_s32 last, tcas_s32 n, TCAS_pChunk *ppChunk) {
    TCAS_pChunk pChunk;
    if (last == first) {
        pChunk = (TCAS_pChunk)malloc(sizeof(TCAS_Chunk));
        libtcas_copy_chunk(pChunk, pKeyFrameChunkStart);
    } else {
        tcas_u32 i, i1, i2, count;
        double factor;
        factor = (n - first) / (double)(last - first);
        count = GETCOUNT(pKeyFrameChunkStart->cltp);
        pChunk = (TCAS_pChunk)malloc(sizeof(TCAS_Chunk));
        pChunk->startTime = n;  /* actually startTime stores frame index not time */
        pChunk->endTime = n;    /* actually endTime stores frame index not time */
        pChunk->cltp = pKeyFrameChunkStart->cltp;
        pChunk->pos_and_color = (tcas_unit *)malloc(count * (sizeof(tcas_unit) << 1));
        for (i = 0; i < count; i++) {
            i1 = i << 1;
            i2 = i1 + 1;
            pChunk->pos_and_color[i1] = LINEAR_INTERPOLATION_POS(pKeyFrameChunkStart->pos_and_color[i1], pKeyFrameChunkEnd->pos_and_color[i1], factor);
            pChunk->pos_and_color[i2] = LINEAR_INTERPOLATION_RGBA(pKeyFrameChunkStart->pos_and_color[i2], pKeyFrameChunkEnd->pos_and_color[i2], factor);
        }
    }
    if (ppChunk)
        *ppChunk = pChunk;
    return (const TCAS_pChunk)pChunk;
}

void libtcas_init_chunks_cache(TCAS_pChunksCache pChunksCache, unsigned long streamCount) {
    pChunksCache->streamCount = streamCount;
    pChunksCache->pi = (tcas_u32 *)malloc(pChunksCache->streamCount * sizeof(tcas_u32));  /* the array of the indexs of the last linearized TCAS_OrderedIndex in the every index stream */
    pChunksCache->pChunks = (TCAS_pChunk)malloc(pChunksCache->streamCount * sizeof(TCAS_Chunk));
    pChunksCache->pChunkPairs = (TCAS_pChunkPair)malloc(pChunksCache->streamCount * sizeof(TCAS_ChunkPair));
    memset(pChunksCache->pi, 0, pChunksCache->streamCount * sizeof(tcas_u32));
    memset(pChunksCache->pChunks, 0, pChunksCache->streamCount * sizeof(TCAS_Chunk));
    memset(pChunksCache->pChunkPairs, 0, pChunksCache->streamCount * sizeof(TCAS_ChunkPair));
}

void libtcas_free_chunks_cache(TCAS_pChunksCache pChunksCache) {
    unsigned long i;
    for (i = 0; i < pChunksCache->streamCount; i++) {
        free(pChunksCache->pChunks[i].pos_and_color);
        free(pChunksCache->pChunkPairs[i].start.pos_and_color);
        free(pChunksCache->pChunkPairs[i].end.pos_and_color);
    }
    pChunksCache->streamCount = 0;
    free(pChunksCache->pi);
    free(pChunksCache->pChunks);
    free(pChunksCache->pChunkPairs);
}


/* HIGH LEVEL APIs */

TCAS_Error_Code libtcas_create_frame_with_index(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_pIndex pIndexs, tcas_s32 n, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf) {
    tcas_bool keyframing;
    TCAS_Chunk chunk;
    TCAS_Chunk keyFrameChunkStart;
    TCAS_Chunk keyFrameChunkEnd;
    tcas_u16 width, height;
    tcas_u32 i, pitch, size;
    tcas_byte *rgba;
    if (TCAS_FILE_TYPE_COMPRESSED != GETHI16B(pHeader->flag)) return tcas_error_file_type_not_support;
    keyframing = GETLOW16B(pHeader->flag);
    width = GETPOSX(pHeader->resolution);
    height = GETPOSY(pHeader->resolution);
    pitch = width * (sizeof(tcas_byte) << 2);
    size = height * pitch;
    rgba = (tcas_byte *)malloc(size);
    memset(rgba, 0, size);
    if (keyframing) {
        for (i = 0; i < pHeader->chunks; i++) {
            if (0 != GETTYPE(pIndexs[i].cltp) && 0 == GETPAIR(pIndexs[i].cltp)) {  /* this is a start key frame */
                if (!(0 != GETTYPE(pIndexs[i + 1].cltp) && 1 == GETPAIR(pIndexs[i + 1].cltp))) {
                    free(rgba);
                    return tcas_error_key_frame_lack_partner;
                }
                if (pIndexs[i].first <= n && n < pIndexs[i + 1].last) {  /* will take effect */
                    TCAS_pChunk pChunk;
                    libtcas_read_specified_chunk(pFile, ((tcas_u64)pIndexs[i].offset) << 2, &keyFrameChunkStart);
                    libtcas_read_specified_chunk(pFile, ((tcas_u64)pIndexs[i + 1].offset) << 2, &keyFrameChunkEnd);
                    libtcas_key_frame_chunk_add_dips(&keyFrameChunkStart, &keyFrameChunkEnd);
                    if (1 == GETTYPE(pIndexs[i].cltp))
                        libtcas_generate_intermediate_frame_chunk_fast(&keyFrameChunkStart, &keyFrameChunkEnd, pIndexs[i].first, pIndexs[i + 1].last, n, &pChunk);
                    else if (2 == GETTYPE(pIndexs[i].cltp))
                        libtcas_generate_intermediate_frame_chunk(&keyFrameChunkStart, &keyFrameChunkEnd, pIndexs[i].first, pIndexs[i + 1].last, n, &pChunk);
                    libtcas_free_chunk(&keyFrameChunkStart);
                    libtcas_free_chunk(&keyFrameChunkEnd);
                    libtcas_convert_chunks_to_rgba(pChunk, width, height, rgba);
                    libtcas_free_chunk(pChunk);
                    free(pChunk);  /* Note that pChunk is dynamically allocated */
                }
                i++;    /* note that key frames come in pair, we must increase i to forbide confusion on key frame chunks and normal chunks */
            } else if (pIndexs[i].first <= n && n < pIndexs[i].last) {
                libtcas_read_specified_chunk(pFile, ((tcas_u64)pIndexs[i].offset) << 2, &chunk);
                libtcas_convert_chunks_to_rgba(&chunk, width, height, rgba);
                libtcas_free_chunk(&chunk);
            }
        }
    } else {
        for (i = 0; i < pHeader->chunks; i++) {
            if (pIndexs[i].first <= n && n < pIndexs[i].last) {
                libtcas_read_specified_chunk(pFile, ((tcas_u64)pIndexs[i].offset) << 2, &chunk);
                libtcas_convert_chunks_to_rgba(&chunk, width, height, rgba);
                libtcas_free_chunk(&chunk);
            }
        }
    }
    if (!(width == targetWidth && height == targetHeight)) {
        tcas_byte *src = rgba;
        rgba = (tcas_byte *)malloc(targetHeight * targetWidth * (sizeof(tcas_byte) << 2));
        libtcas_resample_rgba_bilinear_mmx(src, width, height, rgba, targetWidth, targetHeight);
        free(src);
    }
    *pBuf = rgba;
    return tcas_error_success;
}

TCAS_Error_Code libtcas_create_frame_with_compacted_index(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_pCompactedIndex pCompactedIndexs, tcas_s32 n, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf) {
    TCAS_Chunk chunk;
    tcas_u16 width, height;
    tcas_u32 i, pitch, size;
    tcas_byte *rgba;
    if (TCAS_FILE_TYPE_COMPRESSED != GETHI16B(pHeader->flag)) return tcas_error_file_type_not_support;
    width = GETPOSX(pHeader->resolution);
    height = GETPOSY(pHeader->resolution);
    pitch = width * (sizeof(tcas_byte) << 2);
    size = height * pitch;
    rgba = (tcas_byte *)malloc(size);
    memset(rgba, 0, size);
    for (i = 0; i < pHeader->chunks; i++) {
        if (pCompactedIndexs[i].first <= n && n < pCompactedIndexs[i].last) {
            libtcas_read_specified_chunk(pFile, ((tcas_u64)pCompactedIndexs[i].offset) << 2, &chunk);
            libtcas_convert_chunks_to_rgba(&chunk, width, height, rgba);
            libtcas_free_chunk(&chunk);
        }
    }
    if (!(width == targetWidth && height == targetHeight)) {
        tcas_byte *src = rgba;
        rgba = (tcas_byte *)malloc(targetHeight * targetWidth * (sizeof(tcas_byte) << 2));
        libtcas_resample_rgba_bilinear_mmx(src, width, height, rgba, targetWidth, targetHeight);
        free(src);
    }
    *pBuf = rgba;
    return tcas_error_success;
}

/* libtcas_create_frame_with_linearized_index function() */

int libtcas_check_normal_index_validity(const TCAS_pOrderedIndex src, const TCAS_pOrderedIndex key) {
    if (src->last <= key->first) return -1;
    else if (src->first > key->last) return 1;
    else return 0;
}

/* the function only cares about normal chunks, the first time you pass the pChunksCache to this function should set pChunksCache->streamCount = 0 */
static void _libtcas_parse_linearized_index_streams_normal(TCAS_pFile pFile, const TCAS_IndexStreamsPtr indexStreams, tcas_s32 n, TCAS_pChunksCache pChunksCache) {
    tcas_u32 i, count, indexCount;
    tcas_bool found;
    TCAS_LinearIndexsPtr linearIndexsTemp;
    TCAS_pOrderedIndex pOrderedIndexLast;  /* store the index that read at last time */
    TCAS_pOrderedIndex pOrderedIndexNext;  /* the index next to the last read one */
    count = vector_get_count(indexStreams);
    for (i = 0; i < count; i++) {    /* check every index stream */
        vector_retrieve(indexStreams, i, &linearIndexsTemp);  /* get the ith index stream, low copy, do not free linearIndexsTemp */
        indexCount = vector_get_count(linearIndexsTemp);
        pOrderedIndexLast = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);  /* get the last ordered index of the ith linearized index stream */
        found = TCAS_FALSE;
        if (pOrderedIndexLast->first <= n && n < pOrderedIndexLast->last) {  /* last chunk is still valid */
            if (NULL == pChunksCache->pChunks[i].pos_and_color) {  /* not read yet, this may happen just after the initialization */
                libtcas_read_specified_chunk(pFile, ((tcas_u64)pOrderedIndexLast->offset) << 2, &pChunksCache->pChunks[i]);  /* otherwise, we do not have to read it again */
                pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexLast->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
            }
            found = TCAS_TRUE;
            continue;  /* pi[i] and pChunks[i] stays unchanged, they are still valid */
        } else {  /* the last chunk is invalid */
            if (pChunksCache->pi[i] + 1 < indexCount) {  /* there is still a next index */
                pOrderedIndexNext = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 1, NULL);  /* get the next index */
                if (pOrderedIndexNext->first <= n && n < pOrderedIndexNext->last) {  /* the next index is valid */
                    if (pChunksCache->pChunks[i].pos_and_color)
                        free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                    libtcas_read_specified_chunk(pFile, ((tcas_u64)pOrderedIndexNext->offset) << 2, &pChunksCache->pChunks[i]);  /* cover the last one */
                    pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexNext->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                    pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
                    pChunksCache->pi[i]++;  /* indicate that we have checked the next one */
                    found = TCAS_TRUE;
                    continue;  /* both pi[i] and pChunks[i] have been updated */
                } else if (pOrderedIndexLast->last <= n && n < pOrderedIndexNext->first) {  /* we now know that there is no effect of this nth frame in the stream */
                    if (pChunksCache->pChunks[i].pos_and_color)
                        libtcas_free_chunk(&pChunksCache->pChunks[i]);  /* just erase it */
                    continue;  /* note that pi[i] stays unchanged */
                }
            }
        }
        if (TCAS_FALSE == found) {  /* we cannot find the index easily (maybe the video is moved forward or backward), so we use binary search to try again */
            long result;
            TCAS_OrderedIndex key;
            key.first = n;
            key.last = n;
            result = vector_binary_search(linearIndexsTemp, &key, (vector_compare_func)libtcas_check_normal_index_validity, NULL);
            if (-1 != result) {  /* we find it */
                pChunksCache->pi[i] = result;
                pOrderedIndexLast = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);
                if (pChunksCache->pChunks[i].pos_and_color)
                    free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                libtcas_read_specified_chunk(pFile, ((tcas_u64)pOrderedIndexLast->offset) << 2, &pChunksCache->pChunks[i]);  /* cover the last one */
                pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexLast->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
                found = TCAS_TRUE;
                continue;
            }
        }
        if (TCAS_FALSE == found) {  /* if still does not find the index */
            if (pChunksCache->pChunks[i].pos_and_color)
                libtcas_free_chunk(&pChunksCache->pChunks[i]);  /* just erase it */
        }
    }
}

/* the only difference from _libtcas_parse_linearized_index_streams_normal function is in this one, it use libtcas_read_specified_chunk_from_stream instead of libtcas_read_specified_chunk */
static void _libtcas_parse_linearized_index_streams_normal_from_stream(const tcas_unit *tcasBuf, const TCAS_IndexStreamsPtr indexStreams, tcas_s32 n, TCAS_pChunksCache pChunksCache) {
    tcas_u32 i, count, indexCount;
    tcas_bool found;
    TCAS_LinearIndexsPtr linearIndexsTemp;
    TCAS_pOrderedIndex pOrderedIndexLast;  /* store the index that read at last time */
    TCAS_pOrderedIndex pOrderedIndexNext;  /* the index next to the last read one */
    count = vector_get_count(indexStreams);
    for (i = 0; i < count; i++) {    /* check every index stream */
        vector_retrieve(indexStreams, i, &linearIndexsTemp);  /* get the ith index stream, low copy, do not free linearIndexsTemp */
        indexCount = vector_get_count(linearIndexsTemp);
        pOrderedIndexLast = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);  /* get the last ordered index of the ith linearized index stream */
        found = TCAS_FALSE;
        if (pOrderedIndexLast->first <= n && n < pOrderedIndexLast->last) {  /* last chunk is still valid */
            if (NULL == pChunksCache->pChunks[i].pos_and_color) {  /* not read yet, this may happen just after the initialization */
                libtcas_read_specified_chunk_from_stream(tcasBuf, ((tcas_u64)pOrderedIndexLast->offset) << 2, &pChunksCache->pChunks[i]);  /* otherwise, we do not have to read it again */
                pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexLast->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
            }
            found = TCAS_TRUE;
            continue;  /* pi[i] and pChunks[i] stays unchanged, they are still valid */
        } else {  /* the last chunk is invalid */
            if (pChunksCache->pi[i] + 1 < indexCount) {  /* there is still a next index */
                pOrderedIndexNext = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 1, NULL);  /* get the next index */
                if (pOrderedIndexNext->first <= n && n < pOrderedIndexNext->last) {  /* the next index is valid */
                    if (pChunksCache->pChunks[i].pos_and_color)
                        free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                    libtcas_read_specified_chunk_from_stream(tcasBuf, ((tcas_u64)pOrderedIndexNext->offset) << 2, &pChunksCache->pChunks[i]);  /* cover the last one */
                    pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexNext->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                    pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
                    pChunksCache->pi[i]++;  /* indicate that we have checked the next one */
                    found = TCAS_TRUE;
                    continue;  /* both pi[i] and pChunks[i] have been updated */
                } else if (pOrderedIndexLast->last <= n && n < pOrderedIndexNext->first) {  /* we now know that there is no effect of this nth frame in the stream */
                    if (pChunksCache->pChunks[i].pos_and_color)
                        libtcas_free_chunk(&pChunksCache->pChunks[i]);  /* just erase it */
                    continue;  /* note that pi[i] stays unchanged */
                }
            }
        }
        if (TCAS_FALSE == found) {  /* we cannot find the index easily (maybe the video is moved forward or backward), so we use binary search to try again */
            long result;
            TCAS_OrderedIndex key;
            key.first = n;
            key.last = n;
            result = vector_binary_search(linearIndexsTemp, &key, (vector_compare_func)libtcas_check_normal_index_validity, NULL);
            if (-1 != result) {  /* we find it */
                pChunksCache->pi[i] = result;
                pOrderedIndexLast = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);
                if (pChunksCache->pChunks[i].pos_and_color)
                    free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                libtcas_read_specified_chunk_from_stream(tcasBuf, ((tcas_u64)pOrderedIndexLast->offset) << 2, &pChunksCache->pChunks[i]);  /* cover the last one */
                pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexLast->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
                found = TCAS_TRUE;
                continue;
            }
        }
        if (TCAS_FALSE == found) {  /* if still does not find the index */
            if (pChunksCache->pChunks[i].pos_and_color)
                libtcas_free_chunk(&pChunksCache->pChunks[i]);  /* just erase it */
        }
    }
}

/* will check every two elements */
int libtcas_check_key_index_validity(const TCAS_pOrderedIndex start, const TCAS_pOrderedIndex end, const TCAS_pOrderedIndex key) {
    if (start->first == start->last && end->first == end->last && 0 != GETINDEXID(start->id_pair) && 0 == GETINDEXPAIR(start->id_pair) && 0 != GETINDEXID(end->id_pair) && 1 == GETINDEXPAIR(end->id_pair)) {  /* a key frame chunk pair */
        if (start->first > key->last) return 1;  /* key is on the left of the interval */
        else if (end->last <= key->first) return -1;  /* key is on the right of the interval */
        else return 0;  /* key is in the interval */
    } else {
        if (start->first > key->last) return 1;  /* in fact we can use any other three expressions, like start->last > key->last, etc. */
        else return -1;
    }
}

/* the function only cares about key frame chunks, the first time you pass the pChunksCache to this function should set pChunksCache->streamCount = 0 */
static void _libtcas_parse_linearized_index_streams_key(TCAS_pFile pFile, const TCAS_IndexStreamsPtr indexStreams, tcas_s32 n, TCAS_pChunksCache pChunksCache) {
    tcas_u32 i, count, indexCount;
    tcas_bool found;
    TCAS_pChunk pInterChunkTemp;
    TCAS_LinearIndexsPtr linearIndexsTemp;
    TCAS_pOrderedIndex pOrderedIndexLastStart;  /* store the index that read at last time */
    TCAS_pOrderedIndex pOrderedIndexLastEnd;
    TCAS_pOrderedIndex pOrderedIndexNextStart;  /* the index next to the last read one */
    TCAS_pOrderedIndex pOrderedIndexNextEnd;
    count = vector_get_count(indexStreams);
    for (i = 0; i < count; i++) {  /* check every index stream */
        vector_retrieve(indexStreams, i, &linearIndexsTemp);  /* get the ith index stream, low copy, do not free linearIndexsTemp */
        indexCount = vector_get_count(linearIndexsTemp);
        pOrderedIndexLastStart = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);  /* get the last ordered index of the ith linearized index stream */
        found = TCAS_FALSE;
        if (pOrderedIndexLastStart->first == pOrderedIndexLastStart->last && 0 != GETINDEXID(pOrderedIndexLastStart->id_pair) && 0 == GETINDEXPAIR(pOrderedIndexLastStart->id_pair)) {  /* the last chunk is a key frame chunks */
            if (pChunksCache->pi[i] + 1 >= indexCount) {  /* this seldom happens, unless there is an error causing the lost of the end key frame chunk */
                if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                    libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                    libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
                }
                continue;
            }
            pOrderedIndexLastEnd = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 1, NULL);  /* then, it must have a next chunk, because the index is always indicating the start key frame chunk */
            if (!(pOrderedIndexLastEnd->first == pOrderedIndexLastEnd->last && 0 != GETINDEXID(pOrderedIndexLastEnd->id_pair) && 1 == GETINDEXPAIR(pOrderedIndexLastEnd->id_pair))) {  /* invalid end key frame chunk, this seldom happens, unless there is an error causing the lost of the end key frame chunk */
                if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                    libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                    libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
                }
                continue;
            }
            if (pOrderedIndexLastStart->first <= n && n < pOrderedIndexLastEnd->last) {  /* last chunk is still valid, note that, the last frame cannot be reached in order to be compatible with normal chunk frames */
                if (NULL == pChunksCache->pChunkPairs[i].start.pos_and_color) {  /* not read yet, this may happen just after the initialization, it is impossible that only one pos_and_color is NULL, must be both or neither */
                    libtcas_read_specified_chunk(pFile, ((tcas_u64)pOrderedIndexLastStart->offset) << 2, &pChunksCache->pChunkPairs[i].start);  /* otherwise, we do not have to read it again */
                    libtcas_read_specified_chunk(pFile, ((tcas_u64)pOrderedIndexLastEnd->offset) << 2, &pChunksCache->pChunkPairs[i].end);
                    libtcas_key_frame_chunk_add_dips(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end);
                }
                if (1 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                    libtcas_generate_intermediate_frame_chunk_fast(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexLastStart->first, pOrderedIndexLastEnd->last, n, &pInterChunkTemp);
                else if (2 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                    libtcas_generate_intermediate_frame_chunk(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexLastStart->first, pOrderedIndexLastEnd->last, n, &pInterChunkTemp);
                if (pChunksCache->pChunks[i].pos_and_color)
                    free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                memcpy(&pChunksCache->pChunks[i], pInterChunkTemp, sizeof(TCAS_Chunk));
                free(pInterChunkTemp);  /* Note that pInterChunkTemp is dynamically allocated */
                pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexLastStart->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
                found = TCAS_TRUE;
                continue;  /* pi[i] and pChunkPairs[i] stays unchanged, they are still valid */
            } else {  /* the last chunk is invalid */
                if (pChunksCache->pi[i] + 3 < indexCount) {  /* there may still be a key frame chunk index next to the two key frame chunks */
                    pOrderedIndexNextStart = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 2, NULL);  /* get the index next to the two key frame chunks */
                    if (pOrderedIndexNextStart->first == pOrderedIndexNextStart->last && 0 != GETINDEXID(pOrderedIndexNextStart->id_pair) && 0 == GETINDEXPAIR(pOrderedIndexNextStart->id_pair)) {  /* is key frame chunk */
                        pOrderedIndexNextEnd = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 3, NULL);  /* then, it must have a next chunk, because the index is always indicating the start key frame chunk */
                        if (!(pOrderedIndexNextEnd->first == pOrderedIndexNextEnd->last && 0 != GETINDEXID(pOrderedIndexNextEnd->id_pair) && 1 == GETINDEXPAIR(pOrderedIndexNextEnd->id_pair))) {  /* invalid end key frame chunk, this seldom happens, unless there is an error causing the lost of the end key frame chunk */
                            if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
                            }
                            continue;
                        }
                        if (pOrderedIndexNextStart->first <= n && n < pOrderedIndexNextEnd->last) {  /* last chunk is still valid, note that, the last frame cannot be reached in order to be compatible with normal chunk frames */
                            if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                                free(pChunksCache->pChunkPairs[i].start.pos_and_color);
                                free(pChunksCache->pChunkPairs[i].end.pos_and_color);
                            }
                            libtcas_read_specified_chunk(pFile, ((tcas_u64)pOrderedIndexNextStart->offset) << 2, &pChunksCache->pChunkPairs[i].start);
                            libtcas_read_specified_chunk(pFile, ((tcas_u64)pOrderedIndexNextEnd->offset) << 2, &pChunksCache->pChunkPairs[i].end);
                            libtcas_key_frame_chunk_add_dips(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end);
                            if (1 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                                libtcas_generate_intermediate_frame_chunk_fast(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexNextStart->first, pOrderedIndexNextEnd->last, n, &pInterChunkTemp);
                            else if (2 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                                libtcas_generate_intermediate_frame_chunk(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexNextStart->first, pOrderedIndexNextEnd->last, n, &pInterChunkTemp);
                            if (pChunksCache->pChunks[i].pos_and_color)
                                free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                            memcpy(&pChunksCache->pChunks[i], pInterChunkTemp, sizeof(TCAS_Chunk));
                            free(pInterChunkTemp);  /* Note that pInterChunkTemp is dynamically allocated */
                            pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexNextStart->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                            pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
                            pChunksCache->pi[i] += 2;
                            found = TCAS_TRUE;
                            continue;  /* both pi[i] and pChunkPairs[i] have been updated */
                        } else if (pOrderedIndexLastEnd->last < n && n < pOrderedIndexNextStart->first) {
                            if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
                            }
                            continue;  /* we now know that there is no effect of this nth frame in the stream, note that pi[i] stays unchanged */
                        }
                    }
                }
            }
        }
        if (TCAS_FALSE == found) {  /* we cannot find the index easily (maybe the video is moved forward or backward), so we use binary search to try again */
            long result;
            TCAS_OrderedIndex key;
            key.first = n;
            key.last = n;
            result = vector_binary_search_interval(linearIndexsTemp, &key, (vector_compare_interval_func)libtcas_check_key_index_validity);
            if (-1 != result) {
                pChunksCache->pi[i] = result;
                pOrderedIndexLastStart = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);
                pOrderedIndexLastEnd = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 1, NULL);
                if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                    free(pChunksCache->pChunkPairs[i].start.pos_and_color);
                    free(pChunksCache->pChunkPairs[i].end.pos_and_color);
                }
                libtcas_read_specified_chunk(pFile, ((tcas_u64)pOrderedIndexLastStart->offset) << 2, &pChunksCache->pChunkPairs[i].start);  /* cover the last one */
                libtcas_read_specified_chunk(pFile, ((tcas_u64)pOrderedIndexLastEnd->offset) << 2, &pChunksCache->pChunkPairs[i].end);  /* cover the last one */
                libtcas_key_frame_chunk_add_dips(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end);
                if (1 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                    libtcas_generate_intermediate_frame_chunk_fast(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexLastStart->first, pOrderedIndexLastEnd->last, n, &pInterChunkTemp);
                else if (2 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                    libtcas_generate_intermediate_frame_chunk(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexLastStart->first, pOrderedIndexLastEnd->last, n, &pInterChunkTemp);
                if (pChunksCache->pChunks[i].pos_and_color)
                    free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                memcpy(&pChunksCache->pChunks[i], pInterChunkTemp, sizeof(TCAS_Chunk));
                free(pInterChunkTemp);  /* Note that pInterChunkTemp is dynamically allocated */
                pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexLastStart->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
                found = TCAS_TRUE;
                continue;
            }
        }
        if (TCAS_FALSE == found) {  /* if still does not find the index */
            if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
            }
        }
    }
}

/* the only difference from _libtcas_parse_linearized_index_streams_key function is in this one, it use libtcas_read_specified_chunk_from_stream instead of libtcas_read_specified_chunk */
static void _libtcas_parse_linearized_index_streams_key_from_stream(const tcas_unit *tcasBuf, const TCAS_IndexStreamsPtr indexStreams, tcas_s32 n, TCAS_pChunksCache pChunksCache) {
    tcas_u32 i, count, indexCount;
    tcas_bool found;
    TCAS_pChunk pInterChunkTemp;
    TCAS_LinearIndexsPtr linearIndexsTemp;
    TCAS_pOrderedIndex pOrderedIndexLastStart;  /* store the index that read at last time */
    TCAS_pOrderedIndex pOrderedIndexLastEnd;
    TCAS_pOrderedIndex pOrderedIndexNextStart;  /* the index next to the last read one */
    TCAS_pOrderedIndex pOrderedIndexNextEnd;
    count = vector_get_count(indexStreams);
    for (i = 0; i < count; i++) {  /* check every index stream */
        vector_retrieve(indexStreams, i, &linearIndexsTemp);  /* get the ith index stream, low copy, do not free linearIndexsTemp */
        indexCount = vector_get_count(linearIndexsTemp);
        pOrderedIndexLastStart = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);  /* get the last ordered index of the ith linearized index stream */
        found = TCAS_FALSE;
        if (pOrderedIndexLastStart->first == pOrderedIndexLastStart->last && 0 != GETINDEXID(pOrderedIndexLastStart->id_pair) && 0 == GETINDEXPAIR(pOrderedIndexLastStart->id_pair)) {  /* the last chunk is a key frame chunks */
            if (pChunksCache->pi[i] + 1 >= indexCount) {  /* this seldom happens, unless there is an error causing the lost of the end key frame chunk */
                if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                    libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                    libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
                }
                continue;
            }
            pOrderedIndexLastEnd = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 1, NULL);  /* then, it must have a next chunk, because the index is always indicating the start key frame chunk */
            if (!(pOrderedIndexLastEnd->first == pOrderedIndexLastEnd->last && 0 != GETINDEXID(pOrderedIndexLastEnd->id_pair) && 1 == GETINDEXPAIR(pOrderedIndexLastEnd->id_pair))) {  /* invalid end key frame chunk, this seldom happens, unless there is an error causing the lost of the end key frame chunk */
                if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                    libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                    libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
                }
                continue;
            }
            if (pOrderedIndexLastStart->first <= n && n < pOrderedIndexLastEnd->last) {  /* last chunk is still valid, note that, the last frame cannot be reached in order to be compatible with normal chunk frames */
                if (NULL == pChunksCache->pChunkPairs[i].start.pos_and_color) {  /* not read yet, this may happen just after the initialization, it is impossible that only one pos_and_color is NULL, must be both or neither */
                    libtcas_read_specified_chunk_from_stream(tcasBuf, ((tcas_u64)pOrderedIndexLastStart->offset) << 2, &pChunksCache->pChunkPairs[i].start);  /* otherwise, we do not have to read it again */
                    libtcas_read_specified_chunk_from_stream(tcasBuf, ((tcas_u64)pOrderedIndexLastEnd->offset) << 2, &pChunksCache->pChunkPairs[i].end);
                    libtcas_key_frame_chunk_add_dips(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end);
                }
                if (1 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                    libtcas_generate_intermediate_frame_chunk_fast(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexLastStart->first, pOrderedIndexLastEnd->last, n, &pInterChunkTemp);
                else if (2 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                    libtcas_generate_intermediate_frame_chunk(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexLastStart->first, pOrderedIndexLastEnd->last, n, &pInterChunkTemp);
                if (pChunksCache->pChunks[i].pos_and_color)
                    free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                memcpy(&pChunksCache->pChunks[i], pInterChunkTemp, sizeof(TCAS_Chunk));
                free(pInterChunkTemp);  /* Note that pInterChunkTemp is dynamically allocated */
                pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexLastStart->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
                found = TCAS_TRUE;
                continue;  /* pi[i] and pChunkPairs[i] stays unchanged, they are still valid */
            } else {  /* the last chunk is invalid */
                if (pChunksCache->pi[i] + 3 < indexCount) {  /* there may still be a key frame chunk index next to the two key frame chunks */
                    pOrderedIndexNextStart = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 2, NULL);  /* get the index next to the two key frame chunks */
                    if (pOrderedIndexNextStart->first == pOrderedIndexNextStart->last && 0 != GETINDEXID(pOrderedIndexNextStart->id_pair) && 0 == GETINDEXPAIR(pOrderedIndexNextStart->id_pair)) {  /* is key frame chunk */
                        pOrderedIndexNextEnd = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 3, NULL);  /* then, it must have a next chunk, because the index is always indicating the start key frame chunk */
                        if (!(pOrderedIndexNextEnd->first == pOrderedIndexNextEnd->last && 0 != GETINDEXID(pOrderedIndexNextEnd->id_pair) && 1 == GETINDEXPAIR(pOrderedIndexNextEnd->id_pair))) {  /* invalid end key frame chunk, this seldom happens, unless there is an error causing the lost of the end key frame chunk */
                            if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
                            }
                            continue;
                        }
                        if (pOrderedIndexNextStart->first <= n && n < pOrderedIndexNextEnd->last) {  /* last chunk is still valid, note that, the last frame cannot be reached in order to be compatible with normal chunk frames */
                            if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                                free(pChunksCache->pChunkPairs[i].start.pos_and_color);
                                free(pChunksCache->pChunkPairs[i].end.pos_and_color);
                            }
                            libtcas_read_specified_chunk_from_stream(tcasBuf, ((tcas_u64)pOrderedIndexNextStart->offset) << 2, &pChunksCache->pChunkPairs[i].start);
                            libtcas_read_specified_chunk_from_stream(tcasBuf, ((tcas_u64)pOrderedIndexNextEnd->offset) << 2, &pChunksCache->pChunkPairs[i].end);
                            libtcas_key_frame_chunk_add_dips(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end);
                            if (1 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                                libtcas_generate_intermediate_frame_chunk_fast(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexNextStart->first, pOrderedIndexNextEnd->last, n, &pInterChunkTemp);
                            else if (2 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                                libtcas_generate_intermediate_frame_chunk(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexNextStart->first, pOrderedIndexNextEnd->last, n, &pInterChunkTemp);
                            if (pChunksCache->pChunks[i].pos_and_color)
                                free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                            memcpy(&pChunksCache->pChunks[i], pInterChunkTemp, sizeof(TCAS_Chunk));
                            free(pInterChunkTemp);  /* Note that pInterChunkTemp is dynamically allocated */
                            pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexNextStart->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                            pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
                            pChunksCache->pi[i] += 2;
                            found = TCAS_TRUE;
                            continue;  /* both pi[i] and pChunkPairs[i] have been updated */
                        } else if (pOrderedIndexLastEnd->last < n && n < pOrderedIndexNextStart->first) {
                            if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
                            }
                            continue;  /* we now know that there is no effect of this nth frame in the stream, note that pi[i] stays unchanged */
                        }
                    }
                }
            }
        }
        if (TCAS_FALSE == found) {  /* we cannot find the index easily (maybe the video is moved forward or backward), so we use binary search to try again */
            long result;
            TCAS_OrderedIndex key;
            key.first = n;
            key.last = n;
            result = vector_binary_search_interval(linearIndexsTemp, &key, (vector_compare_interval_func)libtcas_check_key_index_validity);
            if (-1 != result) {
                pChunksCache->pi[i] = result;
                pOrderedIndexLastStart = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i], NULL);
                pOrderedIndexLastEnd = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, pChunksCache->pi[i] + 1, NULL);
                if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                    free(pChunksCache->pChunkPairs[i].start.pos_and_color);
                    free(pChunksCache->pChunkPairs[i].end.pos_and_color);
                }
                libtcas_read_specified_chunk_from_stream(tcasBuf, ((tcas_u64)pOrderedIndexLastStart->offset) << 2, &pChunksCache->pChunkPairs[i].start);  /* cover the last one */
                libtcas_read_specified_chunk_from_stream(tcasBuf, ((tcas_u64)pOrderedIndexLastEnd->offset) << 2, &pChunksCache->pChunkPairs[i].end);  /* cover the last one */
                libtcas_key_frame_chunk_add_dips(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end);
                if (1 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                    libtcas_generate_intermediate_frame_chunk_fast(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexLastStart->first, pOrderedIndexLastEnd->last, n, &pInterChunkTemp);
                else if (2 == GETTYPE(pChunksCache->pChunkPairs[i].start.cltp))
                    libtcas_generate_intermediate_frame_chunk(&pChunksCache->pChunkPairs[i].start, &pChunksCache->pChunkPairs[i].end, pOrderedIndexLastStart->first, pOrderedIndexLastEnd->last, n, &pInterChunkTemp);
                if (pChunksCache->pChunks[i].pos_and_color)
                    free(pChunksCache->pChunks[i].pos_and_color);  /* free the previous chunk */
                memcpy(&pChunksCache->pChunks[i], pInterChunkTemp, sizeof(TCAS_Chunk));
                free(pInterChunkTemp);  /* Note that pInterChunkTemp is dynamically allocated */
                pChunksCache->pChunks[i].startTime = GETINDEXID(pOrderedIndexLastStart->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                pChunksCache->pChunks[i].endTime = GETLAYER(pChunksCache->pChunks[i].cltp);
                found = TCAS_TRUE;
                continue;
            }
        }
        if (TCAS_FALSE == found) {  /* if still does not find the index */
            if (pChunksCache->pChunkPairs[i].start.pos_and_color) {
                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].start);  /* just erase it */
                libtcas_free_chunk(&pChunksCache->pChunkPairs[i].end);  /* just erase it */
            }
        }
    }
}

int libtcas_compare_order_layer(const TCAS_pChunk src, const TCAS_pChunk key) {
    if (src->endTime > key->endTime) return 1;  /* compare layer, in fact endTime here contains layer info, since with the help of indexs we no longer use startTime and endTime data members we can use them to contain some other helpful information */
    else if (src->endTime < key->endTime) return -1;
    else {
        if (src->startTime > key->startTime) return 1;  /* compare id, note that id can not be the same */
        else return -1;
    }
}

/* the first time you pass the pChunksCache to this function should set pChunksCache->streamCount = 0 */
TCAS_Error_Code libtcas_create_frame_with_linearized_index(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_IndexStreamsPtr indexStreams, tcas_s32 n, TCAS_pChunksCache pChunksCache, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf) {
    tcas_bool keyframing;
    tcas_u16 width, height;
    tcas_u32 i, pitch, size;
    tcas_byte *rgba;
    VectorPtr chunksVector;  /* a vector to hold the effective chunks */
    tcas_u32 chunkCount;  /* number of effective chunks */
    TCAS_pChunk pChunkTemp;
    keyframing = GETLOW16B(pHeader->flag);
    width = GETPOSX(pHeader->resolution);
    height = GETPOSY(pHeader->resolution);
    pitch = width * (sizeof(tcas_byte) << 2);
    size = height * pitch;
    rgba = (tcas_byte *)malloc(size);  /* create frame buffer */
    memset(rgba, 0, size);  /* we must clear the bits of the frame buffer */
    vector_create(&chunksVector, sizeof(TCAS_Chunk), 0, NULL, NULL);  /* note that we use low copy and low cleanup to increase performance */
    _libtcas_parse_linearized_index_streams_normal(pFile, indexStreams, n, pChunksCache);
    if (keyframing)
        _libtcas_parse_linearized_index_streams_key(pFile, indexStreams, n, pChunksCache);
    for (i = 0; i < pChunksCache->streamCount; i++) {  /* check every stream */
        if (pChunksCache->pChunks[i].pos_and_color)  /* the ith chunk is effective */
            vector_push_back(chunksVector, &pChunksCache->pChunks[i]);
    }
    chunkCount = vector_get_count(chunksVector);
    vector_quicksort(chunksVector, 0, chunkCount - 1, (vector_compare_func)libtcas_compare_order_layer);
    for (i = 0; i < chunkCount; i++) {
        pChunkTemp = (TCAS_pChunk)vector_retrieve(chunksVector, i, NULL);
        libtcas_convert_chunks_to_rgba(pChunkTemp, width, height, rgba);
    }
    vector_destroy(chunksVector);
    if (!(width == targetWidth && height == targetHeight)) {
        tcas_byte *src = rgba;
        rgba = (tcas_byte *)malloc(targetHeight * targetWidth * (sizeof(tcas_byte) << 2));
        libtcas_resample_rgba_bilinear_mmx(src, width, height, rgba, targetWidth, targetHeight);
        free(src);
    }
    *pBuf = rgba;
    return tcas_error_success;
}

/* the first time you pass the pChunksCache to this function should set pChunksCache->streamCount = 0 */
TCAS_Error_Code libtcas_fill_frame_with_linearized_index(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_IndexStreamsPtr indexStreams, tcas_s32 n, TCAS_pChunksCache pChunksCache, tcas_byte *buf, tcas_u16 width, tcas_u16 height) {
    tcas_bool keyframing;
    tcas_u32 i;
    VectorPtr chunksVector;  /* a vector to hold the effective chunks */
    tcas_u32 chunkCount;  /* number of effective chunks */
    TCAS_pChunk pChunkTemp;
    keyframing = GETLOW16B(pHeader->flag);
    vector_create(&chunksVector, sizeof(TCAS_Chunk), 0, NULL, NULL);  /* note that we use low copy and low cleanup to increase performance */
    _libtcas_parse_linearized_index_streams_normal(pFile, indexStreams, n, pChunksCache);
    if (keyframing)
        _libtcas_parse_linearized_index_streams_key(pFile, indexStreams, n, pChunksCache);
    for (i = 0; i < pChunksCache->streamCount; i++) {  /* check every stream */
        if (pChunksCache->pChunks[i].pos_and_color)  /* the ith chunk is effective */
            vector_push_back(chunksVector, &pChunksCache->pChunks[i]);
    }
    chunkCount = vector_get_count(chunksVector);
    vector_quicksort(chunksVector, 0, chunkCount - 1, (vector_compare_func)libtcas_compare_order_layer);
    for (i = 0; i < chunkCount; i++) {
        pChunkTemp = (TCAS_pChunk)vector_retrieve(chunksVector, i, NULL);
        libtcas_convert_chunks_to_rgba(pChunkTemp, width, height, buf);
    }
    vector_destroy(chunksVector);
    return tcas_error_success;
}

TCAS_Error_Code libtcas_fill_frame_from_stream_with_linearized_index(const tcas_unit *tcasBuf, const TCAS_IndexStreamsPtr indexStreams, tcas_s32 n, TCAS_pChunksCache pChunksCache, tcas_byte *buf, tcas_u16 width, tcas_u16 height) {
    tcas_bool keyframing;
    tcas_u32 i;
    VectorPtr chunksVector;  /* a vector to hold the effective chunks */
    tcas_u32 chunkCount;  /* number of effective chunks */
    TCAS_pChunk pChunkTemp;
    keyframing = TCAS_TRUE;
    vector_create(&chunksVector, sizeof(TCAS_Chunk), 0, NULL, NULL);  /* note that we use low copy and low cleanup to increase performance */
    _libtcas_parse_linearized_index_streams_normal_from_stream(tcasBuf, indexStreams, n, pChunksCache);
    if (keyframing)
        _libtcas_parse_linearized_index_streams_key_from_stream(tcasBuf, indexStreams, n, pChunksCache);
    for (i = 0; i < pChunksCache->streamCount; i++) {  /* check every stream */
        if (pChunksCache->pChunks[i].pos_and_color)  /* the ith chunk is effective */
            vector_push_back(chunksVector, &pChunksCache->pChunks[i]);
    }
    chunkCount = vector_get_count(chunksVector);
    vector_quicksort(chunksVector, 0, chunkCount - 1, (vector_compare_func)libtcas_compare_order_layer);
    for (i = 0; i < chunkCount; i++) {
        pChunkTemp = (TCAS_pChunk)vector_retrieve(chunksVector, i, NULL);
        libtcas_convert_chunks_to_rgba(pChunkTemp, width, height, buf);
    }
    vector_destroy(chunksVector);
    return tcas_error_success;
}

/* libtcas_create_frame_with_parsed_index function */

static int _libtcas_check_index_validity(const TCAS_pCompactedIndex src, const TCAS_pCompactedIndex key) {
    if (src->last <= key->first) return -1;
    else if (src->first > key->last) return 1;
    else return 0;
}

long libtcas_chunk_index_binary_search(const TCAS_pCompactedIndex pParsedIndexs, tcas_u32 indexCount, const TCAS_pCompactedIndex pKey) {
    long bottom, top, mid;
    TCAS_pCompactedIndex pTemp;
    bottom = 0;
    top = indexCount - 1;
    while (bottom < top) {
        mid = (top + bottom) / 2;
        pTemp = pParsedIndexs + mid;
        if (_libtcas_check_index_validity(pTemp, pKey) < 0)
            bottom = mid + 1;
        else
            top = mid;
    }
    pTemp = pParsedIndexs + top;
    if (bottom > top || _libtcas_check_index_validity(pTemp, pKey) != 0) return -1;
    return top;
}

TCAS_Error_Code libtcas_create_frame_with_parsed_index(TCAS_pFile pFile, const TCAS_pHeader pHeader, const TCAS_pCompactedIndex pParsedIndexs, tcas_u32 indexCount, tcas_s32 n, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf) {
    tcas_u16 width, height;
    tcas_u32 pitch, size;
    tcas_byte *rgba;
    long result;
    TCAS_CompactedIndex key;
    TCAS_Chunk chunk;
    width = GETPOSX(pHeader->resolution);
    height = GETPOSY(pHeader->resolution);
    pitch = width * (sizeof(tcas_byte) << 2);
    size = height * pitch;
    rgba = (tcas_byte *)malloc(size);  /* create frame buffer */
    memset(rgba, 0, size);  /* we must clear the bits of the frame buffer */
    key.first = n;
    key.last = n;
    result = libtcas_chunk_index_binary_search(pParsedIndexs, indexCount, &key);
    if (result != -1) {
        libtcas_read_specified_chunk(pFile, ((tcas_u64)pParsedIndexs[result].offset) << 2, &chunk);
        libtcas_convert_chunks_to_rgba(&chunk, width, height, rgba);
        free(chunk.pos_and_color);
    }
    if (!(width == targetWidth && height == targetHeight)) {
        tcas_byte *src = rgba;
        rgba = (tcas_byte *)malloc(targetHeight * targetWidth * (sizeof(tcas_byte) << 2));
        libtcas_resample_rgba_bilinear_mmx(src, width, height, rgba, targetWidth, targetHeight);
        free(src);
    }
    *pBuf = rgba;
    return tcas_error_success;
}

