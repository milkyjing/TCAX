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

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif    /* _MSC_VER */

/* Convert between unicode big endian and unicode little endian */
static void _libtm_convert_endian(wchar_t *buffer, int count) {
    int i;
    unsigned char temp;
    unsigned char *buf;
    buf = (unsigned char *)buffer;
    for (i = 0; i < count; i ++) {
        temp = buf[i << 1];
        buf[i << 1] = buf[(i << 1) + 1];
        buf[(i << 1) + 1] = temp;
    }
}

static void _libtm_face_set_size(FT_Face face, int size) {
    TT_HoriHeader *hori;
    TT_OS2 *os2;
    double scale;
    FT_Size_RequestRec rq;
    FT_Size_Metrics *mt;
    hori = (TT_HoriHeader *)FT_Get_Sfnt_Table(face, ft_sfnt_hhea);
    os2 = (TT_OS2 *)FT_Get_Sfnt_Table(face, ft_sfnt_os2);
    scale = 1;
    /* VSFilter uses metrics from TrueType OS/2 table, The idea was borrowed from libass */
    if (hori && os2) {
        int hori_height;
        int os2_height;
        hori_height = hori->Ascender - hori->Descender;
        os2_height = os2->usWinAscent + os2->usWinDescent;
        if (hori_height && os2_height)
            scale = hori_height / (double)os2_height;
    }
    rq.type = FT_SIZE_REQUEST_TYPE_REAL_DIM;
    rq.width = 0;
    rq.height = (long)(size * scale * 64);
    rq.horiResolution = 0;
    rq.vertResolution = 0;
    FT_Request_Size(face, &rq);
    mt = &face->size->metrics;
    mt->ascender = (long)(mt->ascender / scale);
    mt->descender = (long)(mt->descender / scale);
    mt->height = (long)(mt->height / scale);
}

TM_Error_Code libtm_init_font(TM_pFont pFont, const char *filename, int id, int size, double spaceScale) {
    int len;
    len = (strlen(filename) + 1) * sizeof(char);
    pFont->filename = (char *)malloc(len);
    memcpy(pFont->filename, filename, len);
    pFont->id = id;
    pFont->size = size;
    pFont->spaceScale = spaceScale;
    if (FT_Init_FreeType(&pFont->library) != 0) {
        free(pFont->filename);
        return tm_error_init_fail;
    }
    if (FT_New_Face(pFont->library, pFont->filename, pFont->id, &pFont->face) != 0) {
        free(pFont->filename);
        FT_Done_FreeType(pFont->library);
        return tm_error_init_fail;
    }
    /* FT_Select_Charmap(pFont->face, FT_ENCODING_UNICODE); */
    if (!pFont->face->charmap) {
        free(pFont->filename);
        FT_Done_Face(pFont->face);
        FT_Done_FreeType(pFont->library);
        return tm_error_init_fail;
    }
    _libtm_face_set_size(pFont->face, pFont->size);
    pFont->ascender = pFont->face->size->metrics.ascender / 64;
    pFont->descender = pFont->face->size->metrics.descender / 64;
    /* find font face name */
    pFont->name = NULL;
    if (FT_IS_SFNT(pFont->face)) {
        int i, count;
        FT_SfntName name;
        i = 0;
        count = FT_Get_Sfnt_Name_Count(pFont->face);
        for (i = 0; i < count; i ++) {
            FT_Get_Sfnt_Name(pFont->face, i, &name);
            if (TT_NAME_ID_FULL_NAME == name.name_id && TT_MS_ID_UNICODE_CS == name.encoding_id && TT_PLATFORM_MICROSOFT == name.platform_id) {
                pFont->name = (wchar_t *)malloc(name.string_len + sizeof(wchar_t));
                memcpy(pFont->name, name.string, name.string_len);
                _libtm_convert_endian(pFont->name, name.string_len >> 1);
                pFont->name[name.string_len >> 1] = L'\0';
                break;
            }
        }
    }
    return tm_error_success;
}

TM_Error_Code libtm_fill_tm_text_hori(const TM_pFont pFont, const SYL_pSyllable pSyllable, int spacing, TM_pTextHori pTextHori) {
    int i, j, k, count;
    FT_UInt index;
    pTextHori->lines = pSyllable->lines;
    pTextHori->texts = pSyllable->syls;
    pTextHori->textCount1D = (int *)malloc(pTextHori->lines * sizeof(int));
    memcpy(pTextHori->textCount1D, pSyllable->sylCount1D, pSyllable->lines * sizeof(int));
    pTextHori->spacing = spacing;
    pTextHori->width2D = (int *)malloc(pTextHori->texts * sizeof(int));
    pTextHori->height2D = (int *)malloc(pTextHori->texts * sizeof(int));
    pTextHori->kerning2D = (int *)malloc(pTextHori->texts * sizeof(int));
    pTextHori->advance2D = (int *)malloc(pTextHori->texts * sizeof(int));
    pTextHori->advanceDiff2D = (int *)malloc(pTextHori->texts * sizeof(int));
    pTextHori->length1D = (int *)malloc(pTextHori->lines * sizeof(int));
    pTextHori->initX2D = (int *)malloc(pTextHori->texts * sizeof(int));
    pTextHori->initY2D = (int *)malloc(pTextHori->texts * sizeof(int));
    pTextHori->bearingY2D = (int *)malloc(pTextHori->texts * sizeof(int));
    for (i = 0; i < pSyllable->syls; i ++) {
        if (L'\0' == pSyllable->syl2D[i]) {
            pTextHori->width2D[i] = 0;
            pTextHori->height2D[i] = 0;
            pTextHori->advance2D[i] = 0;
            pTextHori->initX2D[i] = 0;
            pTextHori->initY2D[i] = 0;
            pTextHori->bearingY2D[i] = 0;
        } else {
            index = FT_Get_Char_Index(pFont->face, pSyllable->syl2D[i]);
            if (0 == index) {
                wchar_t szUni[2];
                char szAnsi[5];
                free(pTextHori->textCount1D);
                free(pTextHori->width2D);
                free(pTextHori->height2D);
                free(pTextHori->kerning2D);
                free(pTextHori->advance2D);
                free(pTextHori->advanceDiff2D);
                free(pTextHori->length1D);
                free(pTextHori->initX2D);
                free(pTextHori->initY2D);
                free(pTextHori->bearingY2D);
                szUni[0] = pSyllable->syl2D[i];
                szUni[1] = L'\0';
                WideCharToMultiByte(CP_ACP, 0, szUni, 1, szAnsi, 5, NULL, NULL);
                printf("Error: the font does not contain the character `%s'\n", szAnsi);
                return tm_error_invalid_text;
            }
            FT_Load_Glyph(pFont->face, index, FT_LOAD_DEFAULT);
            pTextHori->width2D[i] = pFont->face->glyph->metrics.width >> 6;
            pTextHori->height2D[i] = pFont->face->glyph->metrics.height >> 6;
            pTextHori->advance2D[i] = pFont->face->glyph->metrics.horiAdvance >> 6;
            pTextHori->initX2D[i] = pFont->face->glyph->metrics.horiBearingX / 64;
            pTextHori->initY2D[i] = pFont->size + ((pFont->face->size->metrics.descender - pFont->face->glyph->metrics.horiBearingY) / 64);
            pTextHori->bearingY2D[i] = pFont->face->glyph->metrics.horiBearingY / 64;
            if (L' ' == pSyllable->syl2D[i] || L'　' == pSyllable->syl2D[i]) {
                pTextHori->width2D[i] = (int)(pTextHori->width2D[i] * pFont->spaceScale);
                pTextHori->advance2D[i] = (int)(pTextHori->advance2D[i] * pFont->spaceScale);
                pTextHori->initX2D[i] = (int)(pTextHori->initX2D[i] * pFont->spaceScale);
            }
        }
    }
    if (FT_HAS_KERNING(pFont->face)) {
        FT_UInt left;
        FT_UInt right;
        FT_Vector kerning;
        k = 0;
        count = 0;
        for (i = 0; i < pSyllable->lines; i ++) {
            pTextHori->kerning2D[k] = 0;
            count ++;
            for (j = 1; j < pSyllable->sylCount1D[i]; j ++) {
                left  = FT_Get_Char_Index(pFont->face, pSyllable->syl2D[count - 1]);
                right = FT_Get_Char_Index(pFont->face, pSyllable->syl2D[count]);
                FT_Get_Kerning(pFont->face, left, right, FT_KERNING_DEFAULT, &kerning);
                pTextHori->kerning2D[count] = kerning.x / 64;
                count ++;
            }
            k += pSyllable->sylCount1D[i];
        }
    } else memset(pTextHori->kerning2D, 0, pTextHori->texts * sizeof(int));
    count = 0;
    for (i = 0; i < pTextHori->lines; i ++) {
        pTextHori->length1D[i] = 0;
        for (j = 0; j < pTextHori->textCount1D[i]; j ++) {
            pTextHori->length1D[i] += pTextHori->advance2D[count] + pTextHori->kerning2D[count] + pTextHori->spacing;
            count ++;
        }
        pTextHori->length1D[i] -= pTextHori->spacing;
    }
    k = 0;
    count = 0;
    for (i = 0; i < pTextHori->lines; i ++) {
        pTextHori->advanceDiff2D[k] = 0;
        count ++;
        for (j = 1; j < pTextHori->textCount1D[i]; j ++) {
            pTextHori->advanceDiff2D[count] = pTextHori->advanceDiff2D[count - 1] + pTextHori->advance2D[count - 1] + pTextHori->kerning2D[count] + pTextHori->spacing;
            count ++;
        }
        k += pTextHori->textCount1D[i];
    }
    return tm_error_success;
}

TM_Error_Code libtm_fill_tm_text_vert(const TM_pFont pFont, const SYL_pSyllable pSyllable, int spacing, TM_pTextVert pTextVert) {
    int i, j, k, count;
    FT_UInt index;
    pTextVert->lines = pSyllable->lines;
    pTextVert->texts = pSyllable->syls;
    pTextVert->textCount1D = (int *)malloc(pTextVert->lines * sizeof(int));
    memcpy(pTextVert->textCount1D, pSyllable->sylCount1D, pSyllable->lines * sizeof(int));
    pTextVert->spacing = spacing;
    pTextVert->width2D = (int *)malloc(pTextVert->texts * sizeof(int));
    pTextVert->height2D = (int *)malloc(pTextVert->texts * sizeof(int));
    pTextVert->advance2D = (int *)malloc(pTextVert->texts * sizeof(int));
    pTextVert->advanceDiff2D = (int *)malloc(pTextVert->texts * sizeof(int));
    pTextVert->length1D = (int *)malloc(pTextVert->lines * sizeof(int));
    pTextVert->initX2D = (int *)malloc(pTextVert->texts * sizeof(int));
    pTextVert->initY2D = (int *)malloc(pTextVert->texts * sizeof(int));
    pTextVert->bearingX2D = (int *)malloc(pTextVert->texts * sizeof(int));
    for (i = 0; i < pSyllable->syls; i ++) {
        if (L'\0' == pSyllable->syl2D[i]) {
            pTextVert->width2D[i] = 0;
            pTextVert->height2D[i] = 0;
            pTextVert->advance2D[i] = 0;
            pTextVert->initX2D[i] = 0;
            pTextVert->initY2D[i] = 0;
            pTextVert->bearingX2D[i] = 0;
        } else {
            index = FT_Get_Char_Index(pFont->face, pSyllable->syl2D[i]);
            if (0 == index) {
                wchar_t szUni[2];
                char szAnsi[5];
                free(pTextVert->textCount1D);
                free(pTextVert->width2D);
                free(pTextVert->height2D);
                free(pTextVert->advance2D);
                free(pTextVert->advanceDiff2D);
                free(pTextVert->length1D);
                free(pTextVert->initX2D);
                free(pTextVert->initY2D);
                free(pTextVert->bearingX2D);
                szUni[0] = pSyllable->syl2D[i];
                szUni[1] = L'\0';
                WideCharToMultiByte(CP_ACP, 0, szUni, 1, szAnsi, 5, NULL, NULL);
                printf("Error: the font does not contain the character `%s'\n", szAnsi);
                return tm_error_invalid_text;
            }
            FT_Load_Glyph(pFont->face, index, FT_LOAD_DEFAULT);
            pTextVert->width2D[i] = pFont->face->glyph->metrics.width >> 6;
            pTextVert->height2D[i] = pFont->face->glyph->metrics.height >> 6;
            pTextVert->advance2D[i] = pFont->face->glyph->metrics.vertAdvance >> 6;
            pTextVert->initX2D[i] = ((pFont->size << 5) + pFont->face->glyph->metrics.vertBearingX) / 64;
            pTextVert->initY2D[i] = (-pFont->face->glyph->metrics.vertBearingY) / 64;
            pTextVert->bearingX2D[i] = (-pFont->face->glyph->metrics.vertBearingX) / 64;
            if (L' ' == pSyllable->syl2D[i] || L'　' == pSyllable->syl2D[i]) {
                pTextVert->width2D[i] = (int)(pTextVert->width2D[i] * pFont->spaceScale);
                pTextVert->advance2D[i] = (int)(pTextVert->advance2D[i] * pFont->spaceScale);
                pTextVert->initY2D[i] = (int)(pTextVert->initY2D[i] * pFont->spaceScale);
            }
        }
    }
    count = 0;
    for (i = 0; i < pTextVert->lines; i ++) {
        pTextVert->length1D[i] = 0;
        for (j = 0; j < pTextVert->textCount1D[i]; j ++) {
            pTextVert->length1D[i] += pTextVert->advance2D[count] + pTextVert->spacing;
            count ++;
        }
        pTextVert->length1D[i] -= pTextVert->spacing;
    }
    k = 0;
    count = 0;
    for (i = 0; i < pTextVert->lines; i ++) {
        pTextVert->advanceDiff2D[k] = 0;
        count ++;
        for (j = 1; j < pTextVert->textCount1D[i]; j ++) {
            pTextVert->advanceDiff2D[count] = pTextVert->advanceDiff2D[count - 1] + pTextVert->advance2D[count - 1] + pTextVert->spacing;
            count ++;
        }
        k += pTextVert->textCount1D[i];
    }
    return tm_error_success;
}

void libtm_fill_tm_word_hori(const TM_pFont pFont, const SYL_pWord pWord, const TM_pTextHori pTextHori, TM_pWordHori pWordHori) {
    int i, j, k, len, count, height;
    int *kSylCount2D;    /* number of texts (syllables) in a word */
    pWordHori->lines = pWord->lines;
    pWordHori->words = pWord->words;
    pWordHori->wordCount1D = (int *)malloc(pWordHori->lines * sizeof(int));
    memcpy(pWordHori->wordCount1D, pWord->kCount1D, pWord->lines * sizeof(int));
    pWordHori->spacing = pTextHori->spacing;
    pWordHori->width2D = (int *)malloc(pWordHori->words * sizeof(int));
    pWordHori->height2D = (int *)malloc(pWordHori->words * sizeof(int));
    pWordHori->kerning2D = (int *)malloc(pWordHori->words * sizeof(int));
    pWordHori->advance2D = (int *)malloc(pWordHori->words * sizeof(int));
    pWordHori->advanceDiff2D = (int *)malloc(pWordHori->words * sizeof(int));
    pWordHori->length1D = (int *)malloc(pWordHori->lines * sizeof(int));
    memcpy(pWordHori->length1D, pTextHori->length1D, pTextHori->lines * sizeof(int));
    pWordHori->initX2D = (int *)malloc(pWordHori->words * sizeof(int));
    pWordHori->initY2D = (int *)malloc(pWordHori->words * sizeof(int));
    pWordHori->bearingY2D = (int *)malloc(pWordHori->words * sizeof(int));
    kSylCount2D = (int *)malloc(pWord->words * sizeof(int));
    memset(kSylCount2D, 0, pWord->words * sizeof(int));
    count = 0;
    for (i = 0; i < pWord->lines; i ++) {
        for (j = 0; j < pWord->kCount1D[i]; j ++) {
            len = wcslen(pWord->word2D[count]);
            kSylCount2D[count] = (0 == len) ? 1 : len;
            count ++;
        }
    }
    count = 0;
    for (i = 0; i < pWordHori->words; i ++) {
        k = count;
        pWordHori->advance2D[i] = 0;
        pWordHori->initY2D[i] = pTextHori->initY2D[k];
        pWordHori->bearingY2D[i] = pTextHori->bearingY2D[k];
        height = pTextHori->height2D[k] - pTextHori->bearingY2D[k];    /* height is the length of the text that belows the base line */
        for (j = 0; j < kSylCount2D[i]; j ++) {
            pWordHori->advance2D[i] += pTextHori->advance2D[count] + pTextHori->kerning2D[count] + pTextHori->spacing;
            pWordHori->initY2D[i] = __min(pWordHori->initY2D[i], pTextHori->initY2D[count]);
            pWordHori->bearingY2D[i] = __max(pWordHori->bearingY2D[i], pTextHori->bearingY2D[count]);
            height = __max(height, pTextHori->height2D[count] - pTextHori->bearingY2D[count]);
            count ++;
        }
        pWordHori->advance2D[i] -= pTextHori->kerning2D[k] + pTextHori->spacing;
        pWordHori->kerning2D[i] = pTextHori->kerning2D[k];
        pWordHori->initX2D[i] = pTextHori->initX2D[k];
        pWordHori->width2D[i] = pWordHori->advance2D[i] - pWordHori->initX2D[i] - (pTextHori->advance2D[count - 1] - pTextHori->initX2D[count - 1] - pTextHori->width2D[count - 1]);
        pWordHori->height2D[i] = pWordHori->bearingY2D[i] + height;
    }
    free(kSylCount2D);
    k = 0;
    count = 0;
    for (i = 0; i < pWordHori->lines; i ++) {
        pWordHori->advanceDiff2D[k] = 0;
        count ++;
        for (j = 1; j < pWordHori->wordCount1D[i]; j ++) {
            pWordHori->advanceDiff2D[count] = pWordHori->advanceDiff2D[count - 1] + pWordHori->advance2D[count - 1] + pWordHori->kerning2D[count] + pWordHori->spacing;
            count ++;
        }
        k += pWordHori->wordCount1D[i];
    }
}

void libtm_fill_tm_word_vert(const TM_pFont pFont, const SYL_pWord pWord, const TM_pTextVert pTextVert, TM_pWordVert pWordVert) {
    int i, j, k, len, count, width;
    int *kSylCount2D;    /* number of texts (syllables) in a word */
    pWordVert->lines = pWord->lines;
    pWordVert->words = pWord->words;
    pWordVert->wordCount1D = (int *)malloc(pWordVert->lines * sizeof(int));
    memcpy(pWordVert->wordCount1D, pWord->kCount1D, pWord->lines * sizeof(int));
    pWordVert->spacing = pTextVert->spacing;
    pWordVert->width2D = (int *)malloc(pWordVert->words * sizeof(int));
    pWordVert->height2D = (int *)malloc(pWordVert->words * sizeof(int));
    pWordVert->advance2D = (int *)malloc(pWordVert->words * sizeof(int));
    pWordVert->advanceDiff2D = (int *)malloc(pWordVert->words * sizeof(int));
    pWordVert->length1D = (int *)malloc(pWordVert->lines * sizeof(int));
    memcpy(pWordVert->length1D, pTextVert->length1D, pTextVert->lines * sizeof(int));
    pWordVert->initX2D = (int *)malloc(pWordVert->words * sizeof(int));
    pWordVert->initY2D = (int *)malloc(pWordVert->words * sizeof(int));
    pWordVert->bearingX2D = (int *)malloc(pWordVert->words * sizeof(int));
    kSylCount2D = (int *)malloc(pWord->words * sizeof(int));
    memset(kSylCount2D, 0, pWord->words * sizeof(int));
    count = 0;
    for (i = 0; i < pWord->lines; i ++) {
        for (j = 0; j < pWord->kCount1D[i]; j ++) {
            len = wcslen(pWord->word2D[count]);
            kSylCount2D[count] = (0 == len) ? 1 : len;
            count ++;
        }
    }
    count = 0;
    for (i = 0; i < pWordVert->words; i ++) {
        k = count;
        pWordVert->advance2D[i] = 0;
        pWordVert->initX2D[i] = pTextVert->initX2D[k];
        pWordVert->bearingX2D[i] = pTextVert->bearingX2D[k];
        width = pTextVert->width2D[k] - pTextVert->bearingX2D[k];    /* width is the length of the text that on the left side of the original line */
        for (j = 0; j < kSylCount2D[i]; j ++) {
            pWordVert->advance2D[i] += pTextVert->advance2D[count] + pTextVert->spacing;
            pWordVert->initX2D[i] = __min(pWordVert->initX2D[i], pTextVert->initX2D[count]);
            pWordVert->bearingX2D[i] = __max(pWordVert->bearingX2D[i], pTextVert->bearingX2D[count]);
            width = __max(width, pTextVert->width2D[count] - pTextVert->bearingX2D[count]);
            count ++;
        }
        pWordVert->advance2D[i] -= pTextVert->spacing;
        pWordVert->initY2D[i] = pTextVert->initY2D[k];
        pWordVert->height2D[i] = pWordVert->advance2D[i] - pWordVert->initY2D[i] - (pTextVert->advance2D[count - 1] - pTextVert->initY2D[count - 1] - pTextVert->height2D[count - 1]);
        pWordVert->width2D[i] = pWordVert->bearingX2D[i] + width;
    }
    free(kSylCount2D);
    k = 0;
    count = 0;
    for (i = 0; i < pWordVert->lines; i ++) {
        pWordVert->advanceDiff2D[k] = 0;
        count ++;
        for (j = 1; j < pWordVert->wordCount1D[i]; j ++) {
            pWordVert->advanceDiff2D[count] = pWordVert->advanceDiff2D[count - 1] + pWordVert->advance2D[count - 1] + pWordVert->spacing;
            count ++;
        }
        k += pWordVert->wordCount1D[i];
    }
}

void libtm_fill_tm_adv_hori(const TM_pFont pFont, const SYL_pAdv pAdv, const TM_pTextHori pTextHori, TM_pAdvHori pAdvHori) {
    int i, j, k, len, count, height;
    int *advSylCount2D;    /* number of texts (syllables) in a adv word */
    pAdvHori->lines = pAdv->lines;
    pAdvHori->advs = pAdv->advs;
    pAdvHori->advCount1D = (int *)malloc(pAdvHori->lines * sizeof(int));
    memcpy(pAdvHori->advCount1D, pAdv->advCount1D, pAdv->lines * sizeof(int));
    pAdvHori->spacing = pTextHori->spacing;
    pAdvHori->width2D = (int *)malloc(pAdvHori->advs * sizeof(int));
    pAdvHori->height2D = (int *)malloc(pAdvHori->advs * sizeof(int));
    pAdvHori->kerning2D = (int *)malloc(pAdvHori->advs * sizeof(int));
    pAdvHori->advance2D = (int *)malloc(pAdvHori->advs * sizeof(int));
    pAdvHori->advanceDiff2D = (int *)malloc(pAdvHori->advs * sizeof(int));
    pAdvHori->length1D = (int *)malloc(pAdvHori->lines * sizeof(int));
    memcpy(pAdvHori->length1D, pTextHori->length1D, pTextHori->lines * sizeof(int));
    pAdvHori->initX2D = (int *)malloc(pAdvHori->advs * sizeof(int));
    pAdvHori->initY2D = (int *)malloc(pAdvHori->advs * sizeof(int));
    pAdvHori->bearingY2D = (int *)malloc(pAdvHori->advs * sizeof(int));
    advSylCount2D = (int *)malloc(pAdv->advs * sizeof(int));
    memset(advSylCount2D, 0, pAdv->advs * sizeof(int));
    count = 0;
    for (i = 0; i < pAdv->lines; i ++) {
        for (j = 0; j < pAdv->advCount1D[i]; j ++) {
            len = wcslen(pAdv->adv2D[count]);
            advSylCount2D[count] = (0 == len) ? 1 : len;
            count ++;
        }
    }
    count = 0;
    for (i = 0; i < pAdvHori->advs; i ++) {
        k = count;
        pAdvHori->advance2D[i] = 0;
        pAdvHori->initY2D[i] = pTextHori->initY2D[k];
        pAdvHori->bearingY2D[i] = pTextHori->bearingY2D[k];
        height = pTextHori->height2D[k] - pTextHori->bearingY2D[k];    /* height is the length of the text that belows the base line */
        for (j = 0; j < advSylCount2D[i]; j ++) {
            pAdvHori->advance2D[i] += pTextHori->advance2D[count] + pTextHori->kerning2D[count] + pTextHori->spacing;
            pAdvHori->initY2D[i] = __min(pAdvHori->initY2D[i], pTextHori->initY2D[count]);
            pAdvHori->bearingY2D[i] = __max(pAdvHori->bearingY2D[i], pTextHori->bearingY2D[count]);
            height = __max(height, pTextHori->height2D[count] - pTextHori->bearingY2D[count]);
            count ++;
        }
        pAdvHori->advance2D[i] -= pTextHori->kerning2D[k] + pTextHori->spacing;
        pAdvHori->kerning2D[i] = pTextHori->kerning2D[k];
        pAdvHori->initX2D[i] = pTextHori->initX2D[k];
        pAdvHori->width2D[i] = pAdvHori->advance2D[i] - pAdvHori->initX2D[i] - (pTextHori->advance2D[count - 1] - pTextHori->initX2D[count - 1] - pTextHori->width2D[count - 1]);
        pAdvHori->height2D[i] = pAdvHori->bearingY2D[i] + height;
    }
    free(advSylCount2D);
    k = 0;
    count = 0;
    for (i = 0; i < pAdvHori->lines; i ++) {
        pAdvHori->advanceDiff2D[k] = 0;
        count ++;
        for (j = 1; j < pAdvHori->advCount1D[i]; j ++) {
            pAdvHori->advanceDiff2D[count] = pAdvHori->advanceDiff2D[count - 1] + pAdvHori->advance2D[count - 1] + pAdvHori->kerning2D[count] + pAdvHori->spacing;
            count ++;
        }
        k += pAdvHori->advCount1D[i];
    }
}

void libtm_fill_tm_adv_vert(const TM_pFont pFont, const SYL_pAdv pAdv, const TM_pTextVert pTextVert, TM_pAdvVert pAdvVert) {
    int i, j, k, len, count, width;
    int *advSylCount2D;    /* number of texts (syllables) in a adv word */
    pAdvVert->lines = pAdv->lines;
    pAdvVert->advs = pAdv->advs;
    pAdvVert->advCount1D = (int *)malloc(pAdvVert->lines * sizeof(int));
    memcpy(pAdvVert->advCount1D, pAdv->advCount1D, pAdv->lines * sizeof(int));
    pAdvVert->spacing = pTextVert->spacing;
    pAdvVert->width2D = (int *)malloc(pAdvVert->advs * sizeof(int));
    pAdvVert->height2D = (int *)malloc(pAdvVert->advs * sizeof(int));
    pAdvVert->advance2D = (int *)malloc(pAdvVert->advs * sizeof(int));
    pAdvVert->advanceDiff2D = (int *)malloc(pAdvVert->advs * sizeof(int));
    pAdvVert->length1D = (int *)malloc(pAdvVert->lines * sizeof(int));
    memcpy(pAdvVert->length1D, pTextVert->length1D, pTextVert->lines * sizeof(int));
    pAdvVert->initX2D = (int *)malloc(pAdvVert->advs * sizeof(int));
    pAdvVert->initY2D = (int *)malloc(pAdvVert->advs * sizeof(int));
    pAdvVert->bearingX2D = (int *)malloc(pAdvVert->advs * sizeof(int));
    advSylCount2D = (int *)malloc(pAdv->advs * sizeof(int));
    memset(advSylCount2D, 0, pAdv->advs * sizeof(int));
    count = 0;
    for (i = 0; i < pAdv->lines; i ++) {
        for (j = 0; j < pAdv->advCount1D[i]; j ++) {
            len = wcslen(pAdv->adv2D[count]);
            advSylCount2D[count] = (0 == len) ? 1 : len;
            count ++;
        }
    }
    count = 0;
    for (i = 0; i < pAdvVert->advs; i ++) {
        k = count;
        pAdvVert->advance2D[i] = 0;
        pAdvVert->initX2D[i] = pTextVert->initX2D[k];
        pAdvVert->bearingX2D[i] = pTextVert->bearingX2D[k];
        width = pTextVert->width2D[k] - pTextVert->bearingX2D[k];    /* height is the length of the text that belows the base line */
        for (j = 0; j < advSylCount2D[i]; j ++) {
            pAdvVert->advance2D[i] += pTextVert->advance2D[count] + pTextVert->spacing;
            pAdvVert->initX2D[i] = __min(pAdvVert->initX2D[i], pTextVert->initX2D[count]);
            pAdvVert->bearingX2D[i] = __max(pAdvVert->bearingX2D[i], pTextVert->bearingX2D[count]);
            width = __max(width, pTextVert->width2D[count] - pTextVert->bearingX2D[count]);
            count ++;
        }
        pAdvVert->advance2D[i] -= pTextVert->spacing;
        pAdvVert->initY2D[i] = pTextVert->initY2D[k];
        pAdvVert->height2D[i] = pAdvVert->advance2D[i] - pAdvVert->initY2D[i] - (pTextVert->advance2D[count - 1] - pTextVert->initY2D[count - 1] - pTextVert->height2D[count - 1]);
        pAdvVert->width2D[i] = pAdvVert->bearingX2D[i] + width;
    }
    free(advSylCount2D);
    k = 0;
    count = 0;
    for (i = 0; i < pAdvVert->lines; i ++) {
        pAdvVert->advanceDiff2D[k] = 0;
        count ++;
        for (j = 1; j < pAdvVert->advCount1D[i]; j ++) {
            pAdvVert->advanceDiff2D[count] = pAdvVert->advanceDiff2D[count - 1] + pAdvVert->advance2D[count - 1] + pAdvVert->spacing;
            count ++;
        }
        k += pAdvVert->advCount1D[i];
    }
}

void libtm_info_to_tcax_with_text_hori(const TM_pFont pFont, const TM_pTextHori pTextHori, TM_pInfoToTcaxHori pInfoToTcax) {
    pInfoToTcax->ascender = pFont->ascender;
    pInfoToTcax->descender = pFont->descender;
    pInfoToTcax->lines = pTextHori->lines;
    pInfoToTcax->count = pTextHori->texts;
    pInfoToTcax->count1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->count1D, pTextHori->textCount1D, pTextHori->lines * sizeof(int));
    pInfoToTcax->width2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->width2D, pTextHori->width2D, pTextHori->texts * sizeof(int));
    pInfoToTcax->height2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->height2D, pTextHori->height2D, pTextHori->texts * sizeof(int));
    pInfoToTcax->kerning2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->kerning2D, pTextHori->kerning2D, pTextHori->texts * sizeof(int));
    pInfoToTcax->advance2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->advance2D, pTextHori->advance2D, pTextHori->texts * sizeof(int));
    pInfoToTcax->advanceDiff2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->advanceDiff2D, pTextHori->advanceDiff2D, pTextHori->texts * sizeof(int));
    pInfoToTcax->length1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->length1D, pTextHori->length1D, pTextHori->lines * sizeof(int));
    pInfoToTcax->initX2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->initX2D, pTextHori->initX2D, pTextHori->texts * sizeof(int));
    pInfoToTcax->initY2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->initY2D, pTextHori->initY2D, pTextHori->texts * sizeof(int));
    pInfoToTcax->bearingY2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->bearingY2D, pTextHori->bearingY2D, pTextHori->texts * sizeof(int));
}

void libtm_info_to_tcax_with_text_vert(const TM_pFont pFont, const TM_pTextVert pTextVert, TM_pInfoToTcaxVert pInfoToTcax) {
    pInfoToTcax->fontSize = pFont->size;
    pInfoToTcax->lines = pTextVert->lines;
    pInfoToTcax->count = pTextVert->texts;
    pInfoToTcax->count1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->count1D, pTextVert->textCount1D, pTextVert->lines * sizeof(int));
    pInfoToTcax->width2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->width2D, pTextVert->width2D, pTextVert->texts * sizeof(int));
    pInfoToTcax->height2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->height2D, pTextVert->height2D, pTextVert->texts * sizeof(int));
    pInfoToTcax->advance2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->advance2D, pTextVert->advance2D, pTextVert->texts * sizeof(int));
    pInfoToTcax->advanceDiff2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->advanceDiff2D, pTextVert->advanceDiff2D, pTextVert->texts * sizeof(int));
    pInfoToTcax->length1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->length1D, pTextVert->length1D, pTextVert->lines * sizeof(int));
    pInfoToTcax->initX2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->initX2D, pTextVert->initX2D, pTextVert->texts * sizeof(int));
    pInfoToTcax->initY2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->initY2D, pTextVert->initY2D, pTextVert->texts * sizeof(int));
    pInfoToTcax->bearingX2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->bearingX2D, pTextVert->bearingX2D, pTextVert->texts * sizeof(int));
}

void libtm_info_to_tcax_with_word_hori(const TM_pFont pFont, const TM_pWordHori pWordHori, TM_pInfoToTcaxHori pInfoToTcax) {
    pInfoToTcax->ascender = pFont->ascender;
    pInfoToTcax->descender = pFont->descender;
    pInfoToTcax->lines = pWordHori->lines;
    pInfoToTcax->count = pWordHori->words;
    pInfoToTcax->count1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->count1D, pWordHori->wordCount1D, pWordHori->lines * sizeof(int));
    pInfoToTcax->width2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->width2D, pWordHori->width2D, pWordHori->words * sizeof(int));
    pInfoToTcax->height2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->height2D, pWordHori->height2D, pWordHori->words * sizeof(int));
    pInfoToTcax->kerning2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->kerning2D, pWordHori->kerning2D, pWordHori->words * sizeof(int));
    pInfoToTcax->advance2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->advance2D, pWordHori->advance2D, pWordHori->words * sizeof(int));
    pInfoToTcax->advanceDiff2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->advanceDiff2D, pWordHori->advanceDiff2D, pWordHori->words * sizeof(int));
    pInfoToTcax->length1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->length1D, pWordHori->length1D, pWordHori->lines * sizeof(int));
    pInfoToTcax->initX2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->initX2D, pWordHori->initX2D, pWordHori->words * sizeof(int));
    pInfoToTcax->initY2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->initY2D, pWordHori->initY2D, pWordHori->words * sizeof(int));
    pInfoToTcax->bearingY2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->bearingY2D, pWordHori->bearingY2D, pWordHori->words * sizeof(int));
}

void libtm_info_to_tcax_with_word_vert(const TM_pFont pFont, const TM_pWordVert pWordVert, TM_pInfoToTcaxVert pInfoToTcax) {
    pInfoToTcax->fontSize = pFont->size;
    pInfoToTcax->lines = pWordVert->lines;
    pInfoToTcax->count = pWordVert->words;
    pInfoToTcax->count1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->count1D, pWordVert->wordCount1D, pWordVert->lines * sizeof(int));
    pInfoToTcax->width2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->width2D, pWordVert->width2D, pWordVert->words * sizeof(int));
    pInfoToTcax->height2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->height2D, pWordVert->height2D, pWordVert->words * sizeof(int));
    pInfoToTcax->advance2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->advance2D, pWordVert->advance2D, pWordVert->words * sizeof(int));
    pInfoToTcax->advanceDiff2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->advanceDiff2D, pWordVert->advanceDiff2D, pWordVert->words * sizeof(int));
    pInfoToTcax->length1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->length1D, pWordVert->length1D, pWordVert->lines * sizeof(int));
    pInfoToTcax->initX2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->initX2D, pWordVert->initX2D, pWordVert->words * sizeof(int));
    pInfoToTcax->initY2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->initY2D, pWordVert->initY2D, pWordVert->words * sizeof(int));
    pInfoToTcax->bearingX2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->bearingX2D, pWordVert->bearingX2D, pWordVert->words * sizeof(int));
}

void libtm_info_to_tcax_with_adv_hori(const TM_pFont pFont, const TM_pAdvHori pAdvHori, TM_pInfoToTcaxHori pInfoToTcax) {
    pInfoToTcax->ascender = pFont->ascender;
    pInfoToTcax->descender = pFont->descender;
    pInfoToTcax->lines = pAdvHori->lines;
    pInfoToTcax->count = pAdvHori->advs;
    pInfoToTcax->count1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->count1D, pAdvHori->advCount1D, pAdvHori->lines * sizeof(int));
    pInfoToTcax->width2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->width2D, pAdvHori->width2D, pAdvHori->advs * sizeof(int));
    pInfoToTcax->height2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->height2D, pAdvHori->height2D, pAdvHori->advs * sizeof(int));
    pInfoToTcax->kerning2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->kerning2D, pAdvHori->kerning2D, pAdvHori->advs * sizeof(int));
    pInfoToTcax->advance2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->advance2D, pAdvHori->advance2D, pAdvHori->advs * sizeof(int));
    pInfoToTcax->advanceDiff2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->advanceDiff2D, pAdvHori->advanceDiff2D, pAdvHori->advs * sizeof(int));
    pInfoToTcax->length1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->length1D, pAdvHori->length1D, pAdvHori->lines * sizeof(int));
    pInfoToTcax->initX2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->initX2D, pAdvHori->initX2D, pAdvHori->advs * sizeof(int));
    pInfoToTcax->initY2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->initY2D, pAdvHori->initY2D, pAdvHori->advs * sizeof(int));
    pInfoToTcax->bearingY2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->bearingY2D, pAdvHori->bearingY2D, pAdvHori->advs * sizeof(int));
}

void libtm_info_to_tcax_with_adv_vert(const TM_pFont pFont, const TM_pAdvVert pAdvVert, TM_pInfoToTcaxVert pInfoToTcax) {
    pInfoToTcax->fontSize = pFont->size;
    pInfoToTcax->lines = pAdvVert->lines;
    pInfoToTcax->count = pAdvVert->advs;
    pInfoToTcax->count1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->count1D, pAdvVert->advCount1D, pAdvVert->lines * sizeof(int));
    pInfoToTcax->width2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->width2D, pAdvVert->width2D, pAdvVert->advs * sizeof(int));
    pInfoToTcax->height2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->height2D, pAdvVert->height2D, pAdvVert->advs * sizeof(int));
    pInfoToTcax->advance2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->advance2D, pAdvVert->advance2D, pAdvVert->advs * sizeof(int));
    pInfoToTcax->advanceDiff2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->advanceDiff2D, pAdvVert->advanceDiff2D, pAdvVert->advs * sizeof(int));
    pInfoToTcax->length1D = (int *)malloc(pInfoToTcax->lines * sizeof(int));
    memcpy(pInfoToTcax->length1D, pAdvVert->length1D, pAdvVert->lines * sizeof(int));
    pInfoToTcax->initX2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->initX2D, pAdvVert->initX2D, pAdvVert->advs * sizeof(int));
    pInfoToTcax->initY2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->initY2D, pAdvVert->initY2D, pAdvVert->advs * sizeof(int));
    pInfoToTcax->bearingX2D = (int *)malloc(pInfoToTcax->count * sizeof(int));
    memcpy(pInfoToTcax->bearingX2D, pAdvVert->bearingX2D, pAdvVert->advs * sizeof(int));
}

void libtm_free_font(TM_pFont pFont) {
    FT_Done_Face(pFont->face);
    FT_Done_FreeType(pFont->library);
    free(pFont->filename);
    free(pFont->name);
}

void libtm_free_tm_text_hori(TM_pTextHori pTextHori) {
    free(pTextHori->textCount1D);
    free(pTextHori->width2D);
    free(pTextHori->height2D);
    free(pTextHori->kerning2D);
    free(pTextHori->advance2D);
    free(pTextHori->advanceDiff2D);
    free(pTextHori->length1D);
    free(pTextHori->initX2D);
    free(pTextHori->initY2D);
    free(pTextHori->bearingY2D);
}

void libtm_free_tm_text_vert(TM_pTextVert pTextVert) {
    free(pTextVert->textCount1D);
    free(pTextVert->width2D);
    free(pTextVert->height2D);
    free(pTextVert->advance2D);
    free(pTextVert->advanceDiff2D);
    free(pTextVert->length1D);
    free(pTextVert->initX2D);
    free(pTextVert->initY2D);
    free(pTextVert->bearingX2D);
}

void libtm_free_tm_word_hori(TM_pWordHori pWordHori) {
    free(pWordHori->wordCount1D);
    free(pWordHori->width2D);
    free(pWordHori->height2D);
    free(pWordHori->kerning2D);
    free(pWordHori->advance2D);
    free(pWordHori->advanceDiff2D);
    free(pWordHori->length1D);
    free(pWordHori->initX2D);
    free(pWordHori->initY2D);
    free(pWordHori->bearingY2D);
}

void libtm_free_tm_word_vert(TM_pWordVert pWordVert) {
    free(pWordVert->wordCount1D);
    free(pWordVert->width2D);
    free(pWordVert->height2D);
    free(pWordVert->advance2D);
    free(pWordVert->advanceDiff2D);
    free(pWordVert->length1D);
    free(pWordVert->initX2D);
    free(pWordVert->initY2D);
    free(pWordVert->bearingX2D);
}

void libtm_free_tm_adv_hori(TM_pAdvHori pAdvHori) {
    free(pAdvHori->advCount1D);
    free(pAdvHori->width2D);
    free(pAdvHori->height2D);
    free(pAdvHori->kerning2D);
    free(pAdvHori->advance2D);
    free(pAdvHori->advanceDiff2D);
    free(pAdvHori->length1D);
    free(pAdvHori->initX2D);
    free(pAdvHori->initY2D);
    free(pAdvHori->bearingY2D);
}

void libtm_free_tm_adv_vert(TM_pAdvVert pAdvVert) {
    free(pAdvVert->advCount1D);
    free(pAdvVert->width2D);
    free(pAdvVert->height2D);
    free(pAdvVert->advance2D);
    free(pAdvVert->advanceDiff2D);
    free(pAdvVert->length1D);
    free(pAdvVert->initX2D);
    free(pAdvVert->initY2D);
    free(pAdvVert->bearingX2D);
}

void libtm_free_info_hori(TM_pInfoToTcaxHori pInfoToTcax) {
    free(pInfoToTcax->count1D);
    free(pInfoToTcax->width2D);
    free(pInfoToTcax->height2D);
    free(pInfoToTcax->kerning2D);
    free(pInfoToTcax->advance2D);
    free(pInfoToTcax->advanceDiff2D);
    free(pInfoToTcax->length1D);
    free(pInfoToTcax->initX2D);
    free(pInfoToTcax->initY2D);
    free(pInfoToTcax->bearingY2D);
}

void libtm_free_info_vert(TM_pInfoToTcaxVert pInfoToTcax) {
    free(pInfoToTcax->count1D);
    free(pInfoToTcax->width2D);
    free(pInfoToTcax->height2D);
    free(pInfoToTcax->advance2D);
    free(pInfoToTcax->advanceDiff2D);
    free(pInfoToTcax->length1D);
    free(pInfoToTcax->initX2D);
    free(pInfoToTcax->initY2D);
    free(pInfoToTcax->bearingX2D);
}

