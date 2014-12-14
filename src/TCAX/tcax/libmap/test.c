/* 
 *  Copyright (C) 2011 milkyjing <milkyjing@gmail.com>
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */




#include "tcc.h"


int main() {
    const char *filename = "test2.tcc";
    wchar_t *uni;
    wchar_t *tcc;
    wchar_t *header;
    int count;
    TCC_Attributes attr;
    TCC_InfoToTcax info;
    libtcc_read_file_to_unicode(filename, &uni, &count);
    /***************/
    if (libtcc_check_format(uni, count) != tcc_error_success) return -1;
    if (libtcc_parse_tcc_string(uni, count, &attr) != tcc_error_success) return -1;
    libtcc_info_to_tcax(&attr, &info);
    libtcc_free_info(&info);
    libtcc_make_ass_header(&attr, &header, &count);
    MessageBoxW(NULL, header, L"D", MB_OK);
    free(header);
    libtcc_convert_attributes_to_tcc_string(&attr, &tcc, &count);
    wprintf(L"%s\n", tcc);
    wprintf(L"%i", count);
    libtcc_write_tcc_string_to_file("test3.tcc", tcc, count);
    /***************/
    libtcc_free_attributes(&attr);
    free(uni);
    return 0;
}



/*
#include "map.h"
#include <stdio.h>

void _map_clean_key(Map *m) {
    free(m->keyBuf);
}

void _map_clean_value(Map *m) {
    free(m->valueBuf);
}

int main() {
    Map m;
    int key, value, temp;
    map_init(&m, sizeof(int), sizeof(int), 5, _map_clean_key, _map_clean_value);
    key = 9;
    value = 8;
    map_push_back(&m, &key, &value);
    key = 7;
    value = 10;
    map_push_back(&m, &key, &value);
    temp = 9;
    map_pop_back(&m);
    if (map_retrieve(&m, &key, &temp) == 0)
    //printf("%i\n", map_get_offset(&m));
    printf("%i\n", temp);
    return 0;
}



#include "string.h"
#include <stdio.h>

int main() {
    String s;
    string_init(&s, 0);
    string_clear(&s);
    string_append(&s, L"too", 2);
    //string_assign(&s, L"L", 1);
    wprintf(string_get_buffer(&s));
    string_clear(&s);
    return 0;
}
*/
