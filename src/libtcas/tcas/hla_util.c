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

#include "hla_util.h"


TCAS_Error_Code libtcas_file_shift_time(const char *filename, int iShiftTime) {
    TCAS_File file;
    TCAS_Header header;
    TCAS_Error_Code error;
    error = libtcas_open_file(&file, filename, tcas_file_read_write);
    if (tcas_error_success != error)
        return error;
    libtcas_read_header(&file, &header, tcas_false);
    header.minTime += iShiftTime;
    header.maxTime += iShiftTime;
    libtcas_write_header(&file, &header, tcas_false);
    if (TCAS_FILE_TYPE_COMPRESSED == GETHI16B(header.flag) || TCAS_FILE_TYPE_PARSED == GETHI16B(header.flag)) {
        tcas_u32 i;
        long offset;
        tcas_unit buf[3];    /* a temp buffer to hold startTime endTime cltp */
        for (i = 0; i < header.chunks; i++) {
            fread(buf, sizeof(tcas_unit), 3, file.fp);
            buf[0] += iShiftTime;
            buf[1] += iShiftTime;
            offset = -3 * (int)sizeof(tcas_unit);
            tcas_fseek(file.fp, offset, SEEK_CUR);
            fwrite(buf, sizeof(tcas_unit), 2, file.fp);
            tcas_fseek(file.fp, (1 + (GETCOUNT(buf[2]) << 1)) * sizeof(tcas_unit), SEEK_CUR);
        }
    } else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(header.flag) || TCAS_FILE_TYPE_PARSED_Z == GETHI16B(header.flag)) {
        tcas_u32 i;
        long offset;
        tcas_unit zSizePadded;
        tcas_unit buf[4];    /* a temp buffer to hold startTime endTime cltp zSize */
        for (i = 0; i < header.chunks; i++) {
            fread(buf, sizeof(tcas_unit), 4, file.fp);
            buf[0] += iShiftTime;
            buf[1] += iShiftTime;
            offset = -4 * (int)sizeof(tcas_unit);
            tcas_fseek(file.fp, offset, SEEK_CUR);
            fwrite(buf, sizeof(tcas_unit), 2, file.fp);
            zSizePadded = buf[3] >> 2;
            if ((0x00000003 & buf[3]) != 0)
                zSizePadded++;
            tcas_fseek(file.fp, (2 + zSizePadded) * sizeof(tcas_unit), SEEK_CUR);
        }
    } else {
        libtcas_close_file(&file);
        return tcas_error_file_type_not_support;
    }
    libtcas_close_file(&file);
    return tcas_error_success;
}

