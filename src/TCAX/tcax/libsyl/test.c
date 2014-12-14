/* 
 *  Copyright (C) 2009-2010 milkyjing <milkyjing@gmail.com>
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


#include "syl.h"


int main() {
    const char *filename = "test.ass";
    wchar_t *uni;
    int i, j, count;
    SYL_Line line;
    SYL_Word word;
    SYL_Syllable syl;
    SYL_Adv adv;
    SYL_InfoToTcax info;
    if (libsyl_read_file_to_unicode(filename, &uni, &count) != syl_error_success) return -1;
    //MessageBox(NULL, uni, L"D", MB_OK);
    /*for (i = 0; i < 100000; i++) {
        if (libsyl_parse_syl_string(uni, count, &line) != syl_error_success) return -1;
        libsyl_free_line(&line);
    }*/
    if (libsyl_parse_syl_string(uni, count, &line) != syl_error_success) return -1;
    if (libsyl_check_line_format(&line, &i, &j) != syl_error_success) {
        printf("%i %i", i, j);
        return -1;
    }
    libsyl_add_k0_to_white_spaces(&line);
    libsyl_parse_line(&line, &word);
    libsyl_parse_word_to_syllable(&word, &syl);
    libsyl_parse_word_to_adv(&word, &adv);
    libsyl_info_to_tcax_with_word(&line, &word, &info);
    libsyl_free_info(&info);
    libsyl_info_to_tcax_with_syl(&line, &syl, &info);
    libsyl_free_info(&info);
    libsyl_info_to_tcax_with_adv(&line, &adv, &info);
    libsyl_free_info(&info);
    MessageBoxW(NULL, line.header, L"D", MB_OK);
    libsyl_free_adv(&adv);
    libsyl_free_syllable(&syl);
    libsyl_free_word(&word);
    libsyl_free_line(&line);
    free(uni);
    printf("Good");
    return 0;
}

