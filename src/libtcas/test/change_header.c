/*
 *  tcas.h -- interface of the 'libtcas' library
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

#include "../tcas/tcas.h"
#include "../tcas/tcas.c"

int main() {
    TCAS_File file;
    TCAS_Header header;
    tcas_unit minTime, maxTime, chunks;
    if (libtcas_open_file(&file, "test.tcas", tcas_file_read_write) != tcas_error_success) return -1;
    libtcas_get_min_max_time_and_chunks(&file, &minTime, &maxTime, &chunks);
    libtcas_set_header(&header, TCAS_FILE_TYPE_COMPRESSED, 0, 1280, 720, minTime, maxTime, chunks, 23976, 1000);
    libtcas_write_header(&file, &header, 0);
    libtcas_close_file(&file);
    return 0;
}
