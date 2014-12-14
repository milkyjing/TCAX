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

#include "tm.h"
#include "../libsyl/syl.c"


int main() {
    wchar_t *uni;
    int i, j, count;
    SYL_Line line;
    SYL_Word word;
    SYL_Adv  adv;
    SYL_Syllable syl;
    TM_Font font;
    TM_TextHori textHori;
    TM_WordHori wordHori;
    TM_AdvHori advHori;
    TM_InfoToTcaxHori info;
    TM_InfoToTcaxVert infoVert;
    TM_TextVert textVert;
    TM_WordVert wordVert;
    TM_AdvVert advVert;
    if (libsyl_read_file_to_unicode("test.ass", &uni, &count) != syl_error_success) return -1;
    if (libsyl_parse_syl_string(uni, count, &line) != syl_error_success) return -1;
    if (libsyl_check_line_format(&line, &i, &j) != syl_error_success) {
        printf("%i %i", i, j);
        return -1;
    }
    libsyl_parse_line(&line, &word);
    libsyl_parse_word_to_syllable(&word, &syl);
    libsyl_parse_word_to_adv(&word, &adv);
    if (libtm_init_font(&font, "DFMrm5.ttc", 1, 36, 1) != tm_error_success) return -1;
    if (libtm_fill_tm_text_hori(&font, &syl, 0, &textHori) != tm_error_success) return -1;
    if (libtm_fill_tm_text_vert(&font, &syl, 0, &textVert) != tm_error_success) return -1;
    libtm_fill_tm_word_hori(&font, &word, &textHori, &wordHori);
    libtm_fill_tm_word_vert(&font, &word, &textVert, &wordVert);
    libtm_fill_tm_adv_vert(&font, &adv, &textVert, &advVert);
    libtm_fill_tm_adv_hori(&font, &adv, &textHori, &advHori);
    libtm_info_to_tcax_with_text_hori(&font, &textHori, &info);
    libtm_free_info_hori(&info);
    libtm_info_to_tcax_with_word_hori(&font, &wordHori, &info);
    libtm_free_info_hori(&info);
    libtm_info_to_tcax_with_adv_hori(&font, &advHori, &info);
    libtm_free_info_hori(&info);
    libtm_info_to_tcax_with_text_vert(&font, &textVert, &infoVert);
    libtm_free_info_vert(&infoVert);
    libtm_info_to_tcax_with_word_vert(&font, &wordVert, &infoVert);
    libtm_free_info_vert(&infoVert);
    libtm_info_to_tcax_with_adv_vert(&font, &advVert, &infoVert);
    libtm_free_info_vert(&infoVert);
    libtm_free_tm_adv_hori(&advHori);
    libtm_free_tm_word_hori(&wordHori);
    libtm_free_tm_text_hori(&textHori);
    libtm_free_tm_adv_vert(&advVert);
    libtm_free_tm_word_vert(&wordVert);
    libtm_free_tm_text_vert(&textVert);
    libsyl_free_syllable(&syl);
    libsyl_free_adv(&adv);
    libsyl_free_word(&word);
    libsyl_free_line(&line);
    free(uni);
    libtm_free_font(&font);
    printf("Good!");
    return 0;
}

