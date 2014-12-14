/* 
 *  Copyright (C) 2009-2011 milkyjing <milkyjing@gmail.com>
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

#include "text.h"


void tcaxlib_sz_unicode_to_ansi(const wchar_t *uni, char **ansi) {
    int size;
    char *sz;
    size = WideCharToMultiByte(CP_ACP, 0, uni, -1, NULL, 0, NULL, NULL);
    sz = (char *)malloc(size * sizeof(char));
    WideCharToMultiByte(CP_ACP, 0, uni, -1, sz, size, NULL, NULL);
    *ansi = sz;
}

static void _tcaxlib_face_set_size(FT_Face face, int size) {
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

int tcaxlib_init_font(TCAX_pFont pFont, const wchar_t *fontFilename, int fontFaceID, int fontSize, int spacing, double spaceScale, unsigned long color, int bord, int is_outline) {
    tcaxlib_sz_unicode_to_ansi(fontFilename, &pFont->filename);
    pFont->id = fontFaceID;
    pFont->size = fontSize;
    pFont->spacing = spacing;
    pFont->spaceScale = spaceScale;
    pFont->color = color;
    pFont->bord = bord;
    pFont->is_outline = is_outline;
    if (FT_Init_FreeType(&pFont->library) != 0) {
        free(pFont->filename);
        return -1;
    }
    if (FT_New_Face(pFont->library, pFont->filename, pFont->id, &pFont->face) != 0) {
        free(pFont->filename);
        FT_Done_FreeType(pFont->library);
        return -1;
    }
    /* FT_Select_Charmap(pFont->face, FT_ENCODING_UNICODE); */
    if (!pFont->face->charmap) {
        free(pFont->filename);
        FT_Done_Face(pFont->face);
        FT_Done_FreeType(pFont->library);
        return -1;
    }
    _tcaxlib_face_set_size(pFont->face, pFont->size);
    return 0;
}

void tcaxlib_fin_font(TCAX_pFont pFont) {
    free(pFont->filename);
    FT_Done_Face(pFont->face);
    FT_Done_FreeType(pFont->library);
}

static int _tcaxlib_get_text_pix(const TCAX_pFont pFont, wchar_t text, TCAX_pPix pPix) {
    int i, l, m;
    FT_UInt index;
    index = FT_Get_Char_Index(pFont->face, text);
    if (0 == index) {
        printf("TextPix Error: find an invalid text that does not exists in the font.\n");
        return -1;
    }
    if (0 == pFont->bord) {
        ///FT_Load_Glyph(pFont->face, index, FT_LOAD_RENDER);
            FT_Glyph glyph;
            FT_BitmapGlyph bitmap_glyph;
            FT_Load_Glyph(pFont->face, index, FT_LOAD_NO_BITMAP);
            FT_Get_Glyph(pFont->face->glyph, &glyph);
            FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, TRUE);
            bitmap_glyph = (FT_BitmapGlyph)glyph;
        pPix->initX = bitmap_glyph->left;    //pFont->face->glyph->metrics.horiBearingX / 64;
        pPix->initY = pFont->size + pFont->face->size->metrics.descender / 64 - bitmap_glyph->top;    ///pFont->size + ((pFont->face->size->metrics.descender - pFont->face->glyph->metrics.horiBearingY) / 64);
        pPix->width = bitmap_glyph->bitmap.width;    //pFont->face->glyph->bitmap.width;
        pPix->height = bitmap_glyph->bitmap.rows;    //pFont->face->glyph->bitmap.rows;
        pPix->size = pPix->height * (pPix->width << 2);
        pPix->buf = (unsigned char *)malloc(pPix->size * sizeof(unsigned char));
        for (l = 0; l < pPix->height; l++) {
            for (m = 0; m < pPix->width; m++) {
                i = (l * pPix->width + m) << 2;
                *((unsigned long *)&pPix->buf[i]) = pFont->color;
                pPix->buf[i + 3] = bitmap_glyph->bitmap.buffer[l * pPix->width + m];
            }
        }
        FT_Done_Glyph(glyph);
    } else if (pFont->is_outline) {    /* has border and only retrieve outline pixels */
        if (pFont->bord > 0) {
            int initX_old, initY_old, initX_new, initY_new;    /* variables with the _old post-fix hold the value of the original text glyph */
            int width_old, height_old, width_new, height_new;
            int left, right, top, bottom, offset_x, offset_y;
            unsigned char *buf_old;
            unsigned char *buf_new;
            FT_Glyph glyph;
            FT_Glyph glyph_old;
            FT_Stroker stroker;
            FT_BitmapGlyph bitmap_glyph;
            FT_BitmapGlyph bitmap_glyph_old;
            FT_Load_Glyph(pFont->face, index, FT_LOAD_NO_BITMAP);
            FT_Get_Glyph(pFont->face->glyph, &glyph);
            glyph_old = glyph;
            FT_Stroker_New(pFont->library, &stroker);
            FT_Stroker_Set(stroker, pFont->bord << 6, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
            FT_Glyph_StrokeBorder(&glyph, stroker, FALSE, FALSE);
            FT_Stroker_Done(stroker);
            FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, TRUE);
            bitmap_glyph = (FT_BitmapGlyph)glyph;
            FT_Glyph_To_Bitmap(&glyph_old, FT_RENDER_MODE_NORMAL, NULL, TRUE);
            bitmap_glyph_old = (FT_BitmapGlyph)glyph_old;
            initX_old = bitmap_glyph_old->left;
            initY_old = pFont->size + (pFont->face->size->metrics.descender / 64) - bitmap_glyph_old->top;
            width_old = bitmap_glyph_old->bitmap.width;
            height_old = bitmap_glyph_old->bitmap.rows;
            initX_new = bitmap_glyph->left;
            initY_new = pFont->size + (pFont->face->size->metrics.descender / 64) - bitmap_glyph->top;
            width_new = bitmap_glyph->bitmap.width;
            height_new = bitmap_glyph->bitmap.rows;
            left = __min(initX_old, initX_new);
            right = __max(initX_old + width_old, initX_new + width_new);
            top = __min(initY_old, initY_new);
            bottom = __max(initY_old + height_old, initY_new + height_new);
            pPix->initX = left;
            pPix->initY = top;
            pPix->width = right - left;
            pPix->height = bottom - top;
            pPix->size = pPix->height * (pPix->width << 2);
            offset_x = initX_old - left;
            offset_y = initY_old - top;
            buf_old = (unsigned char *)malloc(pPix->size * sizeof(unsigned char));
            memset(buf_old, 0, pPix->size * sizeof(unsigned char));
            for (l = 0; l < height_old; l++) {
                for (m = 0; m < width_old; m++)
                    buf_old[(l + offset_y) * pPix->width + m + offset_x] = bitmap_glyph_old->bitmap.buffer[l * width_old + m];
            }
            FT_Done_Glyph(glyph_old);
            offset_x = initX_new - left;
            offset_y = initY_new - top;
            buf_new = (unsigned char *)malloc(pPix->size * sizeof(unsigned char));
            memset(buf_new, 0, pPix->size * sizeof(unsigned char));
            for (l = 0; l < height_new; l++) {
                for (m = 0; m < width_new; m++)
                    buf_new[(l + offset_y) * pPix->width + m + offset_x] = bitmap_glyph->bitmap.buffer[l * width_new + m];
            }
            FT_Done_Glyph(glyph);
            pPix->buf = (unsigned char *)malloc(pPix->size * sizeof(unsigned char));
            for (l = 0; l < pPix->height; l++) {
                for (m = 0; m < pPix->width; m++) {
                    i = 4 * (l * pPix->width + m);
                    *((unsigned long *)&pPix->buf[i]) = pFont->color;
                    pPix->buf[i + 3] = CLIP_0_255(buf_new[l * pPix->width + m] - buf_old[l * pPix->width + m]);
                }
            }
            free(buf_old);
            free(buf_new);
        } else {
            printf("TextPix Error: if you want to get the outline of texts, the `bord' can not be minus.\n");
            return -1;
        }
    } else {    /* can handle cases for both bord < 0 and bord > 0 */
        FT_Glyph glyph;
        FT_Stroker stroker;
        FT_BitmapGlyph bitmap_glyph;
        FT_Load_Glyph(pFont->face, index, FT_LOAD_NO_BITMAP);
        FT_Get_Glyph(pFont->face->glyph, &glyph);
        FT_Stroker_New(pFont->library, &stroker);
        FT_Stroker_Set(stroker, pFont->bord * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);    /* should use `* 64' instead of `<< 6' in case of bord < 0 */
        FT_Glyph_StrokeBorder(&glyph, stroker, FALSE, TRUE);
        FT_Stroker_Done(stroker);
        FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, TRUE);
        bitmap_glyph = (FT_BitmapGlyph)glyph;
        pPix->initX = bitmap_glyph->left;
        pPix->initY = pFont->size + (pFont->face->size->metrics.descender / 64) - bitmap_glyph->top;
        pPix->width = bitmap_glyph->bitmap.width;
        pPix->height = bitmap_glyph->bitmap.rows;
        pPix->size = pPix->height * (pPix->width << 2);
        pPix->buf = (unsigned char *)malloc(pPix->size * sizeof(unsigned char));
        for (l = 0; l < pPix->height; l++) {
            for (m = 0; m < pPix->width; m++) {
                i = 4 * (l * pPix->width + m);
                *((unsigned long *)&pPix->buf[i]) = pFont->color;
                pPix->buf[i + 3] = bitmap_glyph->bitmap.buffer[l * pPix->width + m];
            }
        }
        FT_Done_Glyph(glyph);
    }
    return 0;
}

static int _tcaxlib_get_texts_pix(const TCAX_pFont pFont, const wchar_t *text, TCAX_pPix pPix) {
    int i, j, count, above, below, h, w, offset_x, offset_y, offset_dst, offset_src;
    int *pAdvanceHori;
    int *pBearingYHori;
    int *pKerning;
    FT_UInt index;
    TCAX_pPix pPixTemp;
    count = wcslen(text);
    pAdvanceHori = (int *)malloc(count * sizeof(int));
    pBearingYHori = (int *)malloc(count * sizeof(int));
    pKerning = (int *)malloc(count * sizeof(int));
    pPixTemp = (TCAX_pPix)malloc(count * sizeof(TCAX_Pix));
    for (i = 0; i < count; i++) {
        index = FT_Get_Char_Index(pFont->face, text[i]);
        if (0 == index) {
            free(pAdvanceHori);
            free(pBearingYHori);
            free(pKerning);
            free(pPixTemp);
            printf("TextPix Error: find an invalid text that does not exists in the font.\n");
            return -1;
        }
        FT_Load_Glyph(pFont->face, index, FT_LOAD_DEFAULT);
        pAdvanceHori[i] = pFont->face->glyph->metrics.horiAdvance >> 6;
        pBearingYHori[i] = pFont->face->glyph->metrics.horiBearingY / 64;
        if (L' ' == text[i] || L'　' == text[i])
            pAdvanceHori[i] = (int)(pAdvanceHori[i] * pFont->spaceScale);
        if (_tcaxlib_get_text_pix(pFont, text[i], &pPixTemp[i]) != 0) {
            free(pAdvanceHori);
            free(pBearingYHori);
            free(pKerning);
            for (j = 0; j < i; j++)
                free(pPixTemp[j].buf);
            free(pPixTemp);
            return -1;
        }
    }
    if (FT_HAS_KERNING(pFont->face)) {
        FT_UInt left;
        FT_UInt right;
        FT_Vector kerning;
        for (i = 0; i < count - 1; i++) {
            left  = FT_Get_Char_Index(pFont->face, text[i]);
            right = FT_Get_Char_Index(pFont->face, text[i + 1]);
            FT_Get_Kerning(pFont->face, left, right, FT_KERNING_DEFAULT, &kerning);
            pKerning[i] = kerning.x / 64;
        }
        pKerning[i] = 0;
    } else memset(pKerning, 0, count * sizeof(int));
    pPix->initX = pPixTemp[0].initX;
    pPix->initY = pPixTemp[0].initY;
    for (i = 1; i < count; i++)
        pPix->initY = __min(pPix->initY, pPixTemp[i].initY);
    pPix->width = -(int)pPixTemp[0].initX;
    for (i = 0; i < count; i++)
        pPix->width += pAdvanceHori[i] + pKerning[i] + pFont->spacing;
    pPix->width += pPixTemp[count - 1].width + (int)pPixTemp[count - 1].initX - pAdvanceHori[count - 1] - pFont->spacing;
    above = pBearingYHori[0];
    below = pPixTemp[0].height - pBearingYHori[0];
    for (i = 1; i < count; i++) {
        above = __max(above, pBearingYHori[i]);
        below = __max(below, pPixTemp[i].height - pBearingYHori[i]);
    }
    pPix->height = above + below;
    pPix->size = pPix->height * (pPix->width << 2);
    pPix->buf = (unsigned char *)malloc(pPix->size * sizeof(unsigned char));
    memset(pPix->buf, 0, pPix->size * sizeof(unsigned char));
    offset_x = -(int)pPixTemp[0].initX;
    for (i = 0; i < count; i++) {
        offset_y = above - pBearingYHori[i];
        for (h = 0; h < pPixTemp[i].height; h++) {
            for (w = 0; w < pPixTemp[i].width; w++) {
                offset_dst = ((h + offset_y) * pPix->width + w + offset_x + (int)pPixTemp[i].initX) << 2;
                offset_src = (h * pPixTemp[i].width + w) << 2;
                if (0 != pPixTemp[i].buf[offset_src + 3])
                    memcpy(pPix->buf + offset_dst, pPixTemp[i].buf + offset_src, 4 * sizeof(unsigned char));
            }
        }
        offset_x += pAdvanceHori[i] + pKerning[i] + pFont->spacing;
        free(pPixTemp[i].buf);
    }
    free(pAdvanceHori);
    free(pBearingYHori);
    free(pKerning);
    free(pPixTemp);
    return 0;
}

TCAX_PyFont tcaxlib_init_py_font(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5, *pyArg6, *pyArg7, *pyArg8;
    const wchar_t *fontFilename;
    int fontFaceID;
    int fontSize;
    int spacing;
    double spaceScale;
    unsigned long color;
    int bord;
    int is_outline;
    TCAX_pFont pFont;
    if (PyTuple_GET_SIZE(args) < 8) {
        PyErr_SetString(PyExc_RuntimeError, "InitFont error, too less parameters - `(font_file, face_id, font_size, spacing, space_scale, color, bord, is_outline)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyUnicode_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "InitFont error, the 1st param should be a unicode string - `font_file'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "InitFont error, the 2nd param should be an integer - `face_id'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "InitFont error, the 3rd param should be an integer - `font_size'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "InitFont error, the 4th param should be an integer - `spacing'\n");
        return NULL;
    }
    pyArg5 = PyTuple_GET_ITEM(args, 4);
    if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
        PyErr_SetString(PyExc_RuntimeError, "InitFont error, the 5th param should be a float - `space_scale'\n");
        return NULL;
    }
    pyArg6 = PyTuple_GET_ITEM(args, 5);
    if (!PyLong_Check(pyArg6)) {
        PyErr_SetString(PyExc_RuntimeError, "InitFont error, the 6th param should be an integer - `color'\n");
        return NULL;
    }
    pyArg7 = PyTuple_GET_ITEM(args, 6);
    if (!PyLong_Check(pyArg7) && !PyFloat_Check(pyArg7)) {
        PyErr_SetString(PyExc_RuntimeError, "InitFont error, the 7th param should be an integer - `bord'\n");
        return NULL;
    }
    pyArg8 = PyTuple_GET_ITEM(args, 7);
    if (!PyLong_Check(pyArg8)) {
        PyErr_SetString(PyExc_RuntimeError, "InitFont error, the 8th param should be an integer - `is_outline'\n");
        return NULL;
    }
    fontFilename = (const wchar_t *)PyUnicode_AsUnicode(pyArg1);
    fontFaceID = (int)PyLong_AsLong(pyArg2);
    fontSize = (int)PyLong_AsLong(pyArg3);
    spacing = (int)PyLong_AsLong(pyArg4);
    spaceScale = PyFloat_AsDouble(pyArg5);
    color = (unsigned long)PyLong_AsLong(pyArg6);
    bord = (int)PyLong_AsLong(pyArg7);
    is_outline = (int)PyLong_AsLong(pyArg8);
    pFont = (TCAX_pFont)malloc(sizeof(TCAX_Font));
    if (tcaxlib_init_font(pFont, fontFilename, fontFaceID, fontSize, spacing, spaceScale, color, bord, is_outline) != 0) {
        free(pFont);
        PyErr_SetString(PyExc_RuntimeError, "TextPix error, failed to initialize the font!\n");
        return NULL;
    }
    return PyLong_FromUnsignedLong((unsigned long)pFont);
}

TCAX_Py_Error_Code tcaxlib_fin_py_font(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    TCAX_pFont pFont;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "FinFont error, too less parameters - `(pyFont)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "FinFont error, the 1st param should be an integer - `pyFont'\n");
        return NULL;
    }
    pFont = (TCAX_pFont)PyLong_AsUnsignedLong(pyArg1);
    tcaxlib_fin_font(pFont);
    free(pFont);
    return PyLong_FromLong(0);    /* 0 - means success */
}

TCAX_PyPix tcaxlib_get_pix_from_text(PyObject *self, PyObject *args) {
    int arg_num;
    arg_num = PyTuple_GET_SIZE(args);
    if (arg_num >= 2 && arg_num < 9) {
        PyObject *pyArg1, *pyArg2;
        TCAX_pFont pFont;
        TCAX_Pix pix;
        const wchar_t *text;
        pyArg1 = PyTuple_GET_ITEM(args, 0);
        if (!PyLong_Check(pyArg1)) {
            PyErr_SetString(PyExc_RuntimeError, "TextPix error, the 1st param should be an integer - `pyFont'\n");
            return NULL;
        }
        pyArg2 = PyTuple_GET_ITEM(args, 1);
        if (!PyUnicode_Check(pyArg2)) {
            PyErr_SetString(PyExc_RuntimeError, "TextPix error, the 2nd param should be a unicode string - `text'\n");
            return NULL;
        }
        pFont = (TCAX_pFont)PyLong_AsUnsignedLong(pyArg1);
        text = (const wchar_t *)PyUnicode_AsUnicode(pyArg2);
        if (wcslen(text) <= 1) {
            if (_tcaxlib_get_text_pix(pFont, text[0], &pix) != 0) {
                PyErr_SetString(PyExc_RuntimeError, "TextPix error, failed to get the PIX!\n");
                return NULL;
            }
        } else {
            if (_tcaxlib_get_texts_pix(pFont, text, &pix) != 0) {
                PyErr_SetString(PyExc_RuntimeError, "TextPix error, failed to get the PIX!\n");
                return NULL;
            }
        }
        return tcaxlib_convert_pix(&pix, 1);
    } else if (arg_num >= 9) {
        PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5, *pyArg6, *pyArg7, *pyArg8, *pyArg9;
        const wchar_t *fontFilename;
        int fontFaceID;
        int fontSize;
        int spacing;
        double spaceScale;
        unsigned long color;
        int bord;
        int is_outline;
        const wchar_t *text;
        TCAX_Font font;
        TCAX_Pix pix;
        pyArg1 = PyTuple_GET_ITEM(args, 0);
        if (!PyUnicode_Check(pyArg1)) {
            PyErr_SetString(PyExc_RuntimeError, "TextPix error, the 1st param should be a unicode string - `font_file'\n");
            return NULL;
        }
        pyArg2 = PyTuple_GET_ITEM(args, 1);
        if (!PyLong_Check(pyArg2)) {
            PyErr_SetString(PyExc_RuntimeError, "TextPix error, the 2nd param should be an integer - `face_id'\n");
            return NULL;
        }
        pyArg3 = PyTuple_GET_ITEM(args, 2);
        if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
            PyErr_SetString(PyExc_RuntimeError, "TextPix error, the 3rd param should be an integer - `font_size'\n");
            return NULL;
        }
        pyArg4 = PyTuple_GET_ITEM(args, 3);
        if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
            PyErr_SetString(PyExc_RuntimeError, "TextPix error, the 4th param should be an integer - `spacing'\n");
            return NULL;
        }
        pyArg5 = PyTuple_GET_ITEM(args, 4);
        if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
            PyErr_SetString(PyExc_RuntimeError, "TextPix error, the 5th param should be a float - `space_scale'\n");
            return NULL;
        }
        pyArg6 = PyTuple_GET_ITEM(args, 5);
        if (!PyLong_Check(pyArg6)) {
            PyErr_SetString(PyExc_RuntimeError, "TextPix error, the 6th param should be an integer - `color'\n");
            return NULL;
        }
        pyArg7 = PyTuple_GET_ITEM(args, 6);
        if (!PyLong_Check(pyArg7) && !PyFloat_Check(pyArg7)) {
            PyErr_SetString(PyExc_RuntimeError, "TextPix error, the 7th param should be an integer - `bord'\n");
            return NULL;
        }
        pyArg8 = PyTuple_GET_ITEM(args, 7);
        if (!PyLong_Check(pyArg8)) {
            PyErr_SetString(PyExc_RuntimeError, "TextPix error, the 8th param should be an integer - `is_outline'\n");
            return NULL;
        }
        pyArg9 = PyTuple_GET_ITEM(args, 8);
        if (!PyUnicode_Check(pyArg9)) {
            PyErr_SetString(PyExc_RuntimeError, "TextPix error, the 9th param should be a unicode string - `text'\n");
            return NULL;
        }
        fontFilename = (const wchar_t *)PyUnicode_AsUnicode(pyArg1);
        fontFaceID = (int)PyLong_AsLong(pyArg2);
        fontSize = (int)PyLong_AsLong(pyArg3);
        spacing = (int)PyLong_AsLong(pyArg4);
        spaceScale = PyFloat_AsDouble(pyArg5);
        color = (unsigned long)PyLong_AsLong(pyArg6);
        bord = (int)PyLong_AsLong(pyArg7);
        is_outline = (int)PyLong_AsLong(pyArg8);
        text = (const wchar_t *)PyUnicode_AsUnicode(pyArg9);
        if (tcaxlib_init_font(&font, fontFilename, fontFaceID, fontSize, spacing, spaceScale, color, bord, is_outline) != 0) {
            PyErr_SetString(PyExc_RuntimeError, "TextPix error, failed to initialize the font!\n");
            return NULL;
        }
        if (wcslen(text) <= 1) {
            if (_tcaxlib_get_text_pix(&font, text[0], &pix) != 0) {
                tcaxlib_fin_font(&font);
                PyErr_SetString(PyExc_RuntimeError, "TextPix error, failed to get the PIX!\n");
                return NULL;
            }
        } else {
            if (_tcaxlib_get_texts_pix(&font, text, &pix) != 0) {
                tcaxlib_fin_font(&font);
                PyErr_SetString(PyExc_RuntimeError, "TextPix error, failed to get the PIX!\n");
                return NULL;
            }
        }
        tcaxlib_fin_font(&font);
        return tcaxlib_convert_pix(&pix, 1);
    } else {
        PyErr_SetString(PyExc_RuntimeError, "TextPix error, too less parameters - `(pyFont, text)' or `(font_file, face_id, font_size, spacing, space_scale, color, bord, is_outline, text)'\n");
        return NULL;
    }
}

/* Bezier Curve Evaluation using De Casteljau's Algorithm */
void linear_interpolation(double *pxt, double *pyt, double xa, double ya, double xb, double yb, double t) {
    *pxt = xa + (xb - xa) * t;
    *pyt = ya + (yb - ya) * t;
}

void conic_bezier(double *pxt, double *pyt, double xs, double ys, double xc, double yc, double xe, double ye, double t) {
    double xsc, ysc, xce, yce;
    linear_interpolation(&xsc, &ysc, xs, ys, xc, yc, t);
    linear_interpolation(&xce, &yce, xc, yc, xe, ye, t);
    linear_interpolation(pxt, pyt, xsc, ysc, xce, yce, t);
}

void cubic_bezier(double *pxt, double *pyt, double xs, double ys, double xc1, double yc1, double xc2, double yc2, double xe, double ye, double t) {
    double x10, y10, x11, y11, x12, y12, x20, y20, x21, y21;    /* 00 = s, 01 = c1, 02 = c2, 03 = e */
    linear_interpolation(&x10, &y10, xs, ys, xc1, yc1, t);
    linear_interpolation(&x11, &y11, xc1, yc1, xc2, yc2, t);
    linear_interpolation(&x12, &y12, xc2, yc2, xe, ye, t);
    linear_interpolation(&x20, &y20, x10, y10, x11, y11, t);
    linear_interpolation(&x21, &y21, x11, y11, x12, y12, t);
    linear_interpolation(pxt, pyt, x20, y20, x21, y21, t);
}

static double _max_distance_3(double x1, double x2, double x3) {
    double left, right;
    left = x1;
    if (left > x2)
        left = x2;
    if (left > x3)
        left = x3;
    right = x3;
    if (right < x2)
        right = x2;
    if (right < x1)
        right = x1;
    return right - left;
}

static double _max_distance_4(double x1, double x2, double x3, double x4) {
    double left, right;
    left = x1;
    if (left > x2)
        left = x2;
    if (left > x3)
        left = x3;
    if (left > x4)
        left = x4;
    right = x4;
    if (right < x3)
        right = x3;
    if (right < x2)
        right = x2;
    if (right < x1)
        right = x1;
    return right - left;
}

static int _outline_points_move_to(const FT_Vector *to, void *user) {
    TCAX_pOutlinePoints pOutline = (TCAX_pOutlinePoints)user;
    tcaxlib_points_append(&pOutline->points, to->x / 64.0, pOutline->height - to->y / 64.0, 255);
    pOutline->lastX = to->x / 64.0;
    pOutline->lastY = to->y / 64.0;
    return 0;
}

/*
static int _outline_points_line_to(const FT_Vector *to, void *user) {
    double xMin, yMin, xMax, yMax, xTo, yTo;
    int i, xSteps, ySteps;
    TCAX_pOutlinePoints pOutline = (TCAX_pOutlinePoints)user;
    xTo = to->x / 64.0;
    if (xTo < pOutline->lastX) {
        xMin = xTo;
        xMax = pOutline->lastX;
    } else {
        xMin = pOutline->lastX;
        xMax = xTo;
    }
    yTo = to->y / 64.0;
    if (yTo < pOutline->lastY) {
        yMin = yTo;
        yMax = pOutline->lastY;
    } else {
        yMin = pOutline->lastY;
        yMax = yTo;
    }
    xSteps = (int)(xMax - xMin + 0.5);
    ySteps = (int)(yMax - yMin + 0.5);
    if (xSteps < ySteps) {    // then we will have ySteps points 
        for (i = 1; i < ySteps + 1; i++)
            tcaxlib_points_append(&pOutline->points, xMin + (xMax - xMin) * i / (double)ySteps, yMin + i, 255);
    } else {
        for (i = 1; i < xSteps + 1; i++)
            tcaxlib_points_append(&pOutline->points, xMin + i, yMin + (yMax - yMin) * i / (double)xSteps, 255);
    }
    pOutline->lastX = xTo;
    pOutline->lastY = yTo;
    return 0;
}
*/

static int _outline_points_line_to(const FT_Vector *to, void *user) {
    double xs, ys, xe, ye, x, y, t, step;
    /* int i, points; */
    TCAX_pOutlinePoints pOutline = (TCAX_pOutlinePoints)user;
    xs = pOutline->lastX;
    ys = pOutline->lastY;
    xe = to->x / 64.0;
    ye = to->y / 64.0;
    step = 1 / (__max(abs(xe - xs), abs(ye - ys)) * pOutline->density);
    for (t = step; t < 1 + step; t += step) {
        linear_interpolation(&x, &y, xs, ys, xe, ye, t);
        tcaxlib_points_append(&pOutline->points, x, pOutline->height - y, 255);
    }
    pOutline->lastX = xe;
    pOutline->lastY = ye;
    return 0;
}

static int _outline_points_conic_to(const FT_Vector *control, const FT_Vector *to, void *user) {
    double xs, ys, xc, yc, xe, ye, x, y, t, step;
    /* int i, points; */
    TCAX_pOutlinePoints pOutline = (TCAX_pOutlinePoints)user;
    xs = pOutline->lastX;
    ys = pOutline->lastY;
    xc = control->x / 64.0;
    yc = control->y / 64.0;
    xe = to->x / 64.0;
    ye = to->y / 64.0;
    step = 1 / (__max(_max_distance_3(xs, xc, xe), _max_distance_3(ys, yc, ye)) * pOutline->density);
    for (t = step; t < 1 + step; t += step) {
        conic_bezier(&x, &y, xs, ys, xc, yc, xe, ye, t);
        tcaxlib_points_append(&pOutline->points, x, pOutline->height - y, 255);
    }
    /*points = (int)(__max(_max_distance(xs, xc, xe), _max_distance(ys, yc, ye)) * pOutline->density + 0.5);
    for (i = 1; i < points + 1; i++) {
        t = i / (double)points;
        x = (1 - t) * (1 - t) * xs + 2 * t * (1 - t) * xc + t * t * xe;
        y = (1 - t) * (1 - t) * ys + 2 * t * (1 - t) * yc + t * t * ye;
        tcaxlib_points_append(&pOutline->points, x, pOutline->height - y, 255);
    }*/
    pOutline->lastX = xe;
    pOutline->lastY = ye;
    return 0;
}

static int _outline_points_cubic_to(const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user) {
    double xs, ys, xc1, yc1, xc2, yc2, xe, ye, x, y, t, step;
    TCAX_pOutlinePoints pOutline = (TCAX_pOutlinePoints)user;
    xs = pOutline->lastX;
    ys = pOutline->lastY;
    xc1 = control1->x / 64.0;
    yc1 = control1->y / 64.0;
    xc2 = control2->x / 64.0;
    yc2 = control2->y / 64.0;
    xe = to->x / 64.0;
    ye = to->y / 64.0;
    step = 1 / (__max(_max_distance_4(xs, xc1, xc2, xe), _max_distance_4(ys, yc1, yc2, ye)) * pOutline->density);
    for (t = step; t < 1 + step; t += step) {
        cubic_bezier(&x, &y, xs, ys, xc1, yc1, xc2, yc2, xe, ye, t);
        tcaxlib_points_append(&pOutline->points, x, pOutline->height - y, 255);
    }
    pOutline->lastX = xe;
    pOutline->lastY = ye;
    return 0;
}

static int _tcaxlib_get_text_outline_points(const TCAX_pFont pFont, wchar_t text, double density, double **pXBuf, double **pYBuf, unsigned char **pABuf, int *pCount) {
    FT_UInt index;
    FT_Glyph glyph;
    FT_OutlineGlyph outline;
    FT_Outline_Funcs outInterface;
    TCAX_OutlinePoints outlinePoints;
    /* get parameters from script */
    index = FT_Get_Char_Index(pFont->face, text);
    if (0 == index) {
        printf("TextOutlinePoints Error: find an invalid text that does not exists in the font.\n");
        return -1;
    }
    FT_Load_Glyph(pFont->face, index, FT_LOAD_DEFAULT);
    FT_Get_Glyph(pFont->face->glyph, &glyph);
    if (FT_GLYPH_FORMAT_OUTLINE != glyph->format) {
        printf("TextOutlinePoints Error: cannot get the outline of the text.\n");
        return -1;
    }
    outline = (FT_OutlineGlyph)glyph;
    outlinePoints.density = density;
    outlinePoints.height = pFont->size + pFont->face->size->metrics.descender / 64;
    outlinePoints.lastX = 0;
    outlinePoints.lastY = 0;
    outlinePoints.points.count = 0;
    outlinePoints.points.capacity = 100;
    outlinePoints.points.xBuf = (double *)malloc(outlinePoints.points.capacity * sizeof(double));
    outlinePoints.points.yBuf = (double *)malloc(outlinePoints.points.capacity * sizeof(double));
    outlinePoints.points.aBuf = (unsigned char *)malloc(outlinePoints.points.capacity * sizeof(unsigned char));
    outInterface.move_to = (FT_Outline_MoveToFunc)_outline_points_move_to;
    outInterface.line_to = (FT_Outline_LineToFunc)_outline_points_line_to;
    outInterface.conic_to = (FT_Outline_ConicToFunc)_outline_points_conic_to;
    outInterface.cubic_to = (FT_Outline_CubicToFunc)_outline_points_cubic_to;
    outInterface.shift = 0;
    outInterface.delta = 0;
    FT_Outline_Decompose(&outline->outline, &outInterface, &outlinePoints);
    FT_Done_Glyph(glyph);
    *pXBuf = outlinePoints.points.xBuf;
    *pYBuf = outlinePoints.points.yBuf;
    *pABuf = outlinePoints.points.aBuf;
    *pCount = outlinePoints.points.count;
    return 0;
}

TCAX_PyPoints tcaxlib_get_text_outline_as_points(PyObject *self, PyObject *args) {
    int arg_num;
    arg_num = PyTuple_GET_SIZE(args);
    if (arg_num >= 3 && arg_num < 5) {
        PyObject *pyArg1, *pyArg2, *pyArg3;
        TCAX_pFont pFont;
        const wchar_t *text;
        double density;
        double *xBuf, *yBuf;
        unsigned char *aBuf;
        int i, count;
        TCAX_PyPoints pyPoints;
        PyObject *pyPointTemp;
        pyArg1 = PyTuple_GET_ITEM(args, 0);
        if (!PyLong_Check(pyArg1)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlinePoints error, the 1st param should be an integer - `pyFont'\n");
            return NULL;
        }
        pyArg2 = PyTuple_GET_ITEM(args, 1);
        if (!PyUnicode_Check(pyArg2)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlinePoints error, the 2nd param should be a unicode string - `text'\n");
            return NULL;
        }
        pyArg3 = PyTuple_GET_ITEM(args, 2);
        if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlinePoints error, the 3rd param should be a float - `density'\n");
            return NULL;
        }
        pFont = (TCAX_pFont)PyLong_AsUnsignedLong(pyArg1);
        text = (const wchar_t *)PyUnicode_AsUnicode(pyArg2);
        density = PyFloat_AsDouble(pyArg3);
        if (_tcaxlib_get_text_outline_points(pFont, text[0], density, &xBuf, &yBuf, &aBuf, &count) != 0) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlinePoints error, failed to get the outline!\n");
            return NULL;
        }
        pyPoints = PyTuple_New(count);
        for (i = 0; i < count; i++) {
            pyPointTemp = PyTuple_New(3);
            PyTuple_SET_ITEM(pyPointTemp, 0, PyFloat_FromDouble(xBuf[i]));
            PyTuple_SET_ITEM(pyPointTemp, 1, PyFloat_FromDouble(yBuf[i]));
            PyTuple_SET_ITEM(pyPointTemp, 2, PyLong_FromLong(aBuf[i]));
            PyTuple_SET_ITEM(pyPoints, i, pyPointTemp);
        }
        free(xBuf);
        free(yBuf);
        free(aBuf);
        return pyPoints;
    } else if (arg_num >= 5) {
        PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5;
        const wchar_t *fontFilename;
        int fontFaceID;
        int fontSize;
        const wchar_t *text;
        double density;
        TCAX_Font font;
        double *xBuf, *yBuf;
        unsigned char *aBuf;
        int i, count;
        TCAX_PyPoints pyPoints;
        PyObject *pyPointTemp;
        pyArg1 = PyTuple_GET_ITEM(args, 0);
        if (!PyUnicode_Check(pyArg1)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlinePoints error, the 1st param should be a unicode string - `font_file'\n");
            return NULL;
        }
        pyArg2 = PyTuple_GET_ITEM(args, 1);
        if (!PyLong_Check(pyArg2)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlinePoints error, the 2nd param should be an integer - `face_id'\n");
            return NULL;
        }
        pyArg3 = PyTuple_GET_ITEM(args, 2);
        if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlinePoints error, the 3rd param should be an integer - `font_size'\n");
            return NULL;
        }
        pyArg4 = PyTuple_GET_ITEM(args, 3);
        if (!PyUnicode_Check(pyArg4)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlinePoints error, the 4th param should be a unicode string - `text'\n");
            return NULL;
        }
        pyArg5 = PyTuple_GET_ITEM(args, 4);
        if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlinePoints error, the 5th param should be a float - `density'\n");
            return NULL;
        }
        fontFilename = (const wchar_t *)PyUnicode_AsUnicode(pyArg1);
        fontFaceID = (int)PyLong_AsLong(pyArg2);
        fontSize = (int)PyLong_AsLong(pyArg3);
        text = (const wchar_t *)PyUnicode_AsUnicode(pyArg4);
        density = PyFloat_AsDouble(pyArg5);
        if (tcaxlib_init_font(&font, fontFilename, fontFaceID, fontSize, 0, 0, 0, 0, 0) != 0) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlinePoints error, failed to initialize the font!\n");
            return NULL;
        }
        if (_tcaxlib_get_text_outline_points(&font, text[0], density, &xBuf, &yBuf, &aBuf, &count) != 0) {
            tcaxlib_fin_font(&font);
            PyErr_SetString(PyExc_RuntimeError, "TextOutlinePoints error, failed to get the outline!\n");
            return NULL;
        }
        tcaxlib_fin_font(&font);
        pyPoints = PyTuple_New(count);
        for (i = 0; i < count; i++) {
            pyPointTemp = PyTuple_New(3);
            PyTuple_SET_ITEM(pyPointTemp, 0, PyFloat_FromDouble(xBuf[i]));
            PyTuple_SET_ITEM(pyPointTemp, 1, PyFloat_FromDouble(yBuf[i]));
            PyTuple_SET_ITEM(pyPointTemp, 2, PyLong_FromLong(aBuf[i]));
            PyTuple_SET_ITEM(pyPoints, i, pyPointTemp);
        }
        free(xBuf);
        free(yBuf);
        free(aBuf);
        return pyPoints;
    } else {
        PyErr_SetString(PyExc_RuntimeError, "TextOutlinePoints error, too less parameters - `(pyFont, text, density)' or `(font_file, face_id, font_size, text, density)'\n");
        return NULL;
    }
}

static int _tcaxlib_get_text_metrics(const TCAX_pFont pFont, const wchar_t *text, TCAX_pTextMetrics pTextMetrics) {
    int i, count, height;
    FT_UInt index;
    int *pWidth;
    int *pHeight;
    int *pHoriBearingX;
    int *pHoriBearingY;
    int *pHoriAdvance;
    int *pVertBearingX;
    int *pVertBearingY;
    int *pVertAdvance;
    int *pKerning;
    count = wcslen(text);
    pWidth = (int *)malloc(count * sizeof(int));
    pHeight = (int *)malloc(count * sizeof(int));
    pHoriBearingX = (int *)malloc(count * sizeof(int));
    pHoriBearingY = (int *)malloc(count * sizeof(int));
    pHoriAdvance = (int *)malloc(count * sizeof(int));
    pVertBearingX = (int *)malloc(count * sizeof(int));
    pVertBearingY = (int *)malloc(count * sizeof(int));
    pVertAdvance = (int *)malloc(count * sizeof(int));
    pKerning = (int *)malloc(count * sizeof(int));
    for (i = 0; i < count; i++) {
        index = FT_Get_Char_Index(pFont->face, text[i]);
        if (0 == index) {
            free(pWidth);
            free(pHeight);
            free(pHoriBearingX);
            free(pHoriBearingY);
            free(pHoriAdvance);
            free(pVertBearingX);
            free(pVertBearingY);
            free(pVertAdvance);
            free(pKerning);
            printf("TextMetrics Error: find an invalid text that does not exists in the font.\n");
            return -1;
        }
        FT_Load_Glyph(pFont->face, index, FT_LOAD_DEFAULT);
        pWidth[i] = pFont->face->glyph->metrics.width >> 6;
        pHeight[i] = pFont->face->glyph->metrics.height >> 6;
        pHoriBearingX[i] = pFont->face->glyph->metrics.horiBearingX / 64;
        pHoriBearingY[i] = pFont->face->glyph->metrics.horiBearingY / 64;
        pHoriAdvance[i] = pFont->face->glyph->metrics.horiAdvance >> 6;
        pVertBearingX[i] = pFont->face->glyph->metrics.vertBearingX / 64;
        pVertBearingY[i] = pFont->face->glyph->metrics.vertBearingY / 64;
        pVertAdvance[i] = pFont->face->glyph->metrics.vertAdvance >> 6;
    }
    if (FT_HAS_KERNING(pFont->face)) {
        FT_UInt left;
        FT_UInt right;
        FT_Vector kerning;
        for (i = 0; i < count - 1; i++) {
            left  = FT_Get_Char_Index(pFont->face, text[i]);
            right = FT_Get_Char_Index(pFont->face, text[i + 1]);
            FT_Get_Kerning(pFont->face, left, right, FT_KERNING_DEFAULT, &kerning);
            pKerning[i] = kerning.x / 64;
        }
        pKerning[i] = 0;
    } else memset(pKerning, 0, count * sizeof(int));
    pTextMetrics->horiAdvance = 0;
    pTextMetrics->width = 0;
    height = pHeight[0] - pHoriBearingY[0];
    pTextMetrics->horiBearingX = pHoriBearingX[0];
    pTextMetrics->horiBearingY = pHoriBearingY[0];
    pTextMetrics->vertAdvance = 0;
    pTextMetrics->vertBearingX = pVertBearingX[0];
    pTextMetrics->vertBearingY = pVertBearingY[0];
    for (i = 0; i < count; i++) {
        pTextMetrics->horiAdvance += pHoriAdvance[i] + pKerning[i] + pFont->spacing;
        height = __max(height, pHeight[i] - pHoriBearingY[i]);
        pTextMetrics->horiBearingY = __max(pTextMetrics->horiBearingY, pHoriBearingY[i]);
        pTextMetrics->vertAdvance += pVertAdvance[i] + pFont->spacing;    /* kerning only supports horizontal texts */
        pTextMetrics->vertBearingX = __min(pTextMetrics->vertBearingX, pVertBearingX[i]);    /* in most cases it's negative for vertical texts */
    }
    pTextMetrics->horiAdvance -= pFont->spacing;
    pTextMetrics->width = pTextMetrics->horiAdvance - pHoriBearingX[0] - (pHoriAdvance[count - 1] - pHoriBearingX[count - 1] - pWidth[count - 1]);
    pTextMetrics->height = height + pTextMetrics->horiBearingY;
    pTextMetrics->vertAdvance -= pFont->spacing;
    free(pWidth);
    free(pHeight);
    free(pHoriBearingX);
    free(pHoriBearingY);
    free(pHoriAdvance);
    free(pVertBearingX);
    free(pVertBearingY);
    free(pVertAdvance);
    free(pKerning);
    pTextMetrics->x_ppem = pFont->face->size->metrics.x_ppem;
    pTextMetrics->y_ppem = pFont->face->size->metrics.y_ppem;
    pTextMetrics->x_scale = pFont->face->size->metrics.x_scale;
    pTextMetrics->y_scale = pFont->face->size->metrics.y_scale;
    pTextMetrics->ascender = pFont->face->size->metrics.ascender / 64;
    pTextMetrics->descender = pFont->face->size->metrics.descender / 64;
    pTextMetrics->px_height = pFont->face->size->metrics.height >> 6;
    pTextMetrics->max_advance = pFont->face->size->metrics.max_advance >> 6;
    return 0;
}

static TCAX_PyTextMetrics _tcaxlib_convert_text_metrics(const TCAX_pTextMetrics pTextMetrics) {
    TCAX_PyTextMetrics pyTextMetrics;
    pyTextMetrics = PyTuple_New(16);
    PyTuple_SET_ITEM(pyTextMetrics,  0, PyLong_FromLong(pTextMetrics->width));
    PyTuple_SET_ITEM(pyTextMetrics,  1, PyLong_FromLong(pTextMetrics->height));
    PyTuple_SET_ITEM(pyTextMetrics,  2, PyLong_FromLong(pTextMetrics->horiBearingX));
    PyTuple_SET_ITEM(pyTextMetrics,  3, PyLong_FromLong(pTextMetrics->horiBearingY));
    PyTuple_SET_ITEM(pyTextMetrics,  4, PyLong_FromLong(pTextMetrics->horiAdvance));
    PyTuple_SET_ITEM(pyTextMetrics,  5, PyLong_FromLong(pTextMetrics->vertBearingX));
    PyTuple_SET_ITEM(pyTextMetrics,  6, PyLong_FromLong(pTextMetrics->vertBearingY));
    PyTuple_SET_ITEM(pyTextMetrics,  7, PyLong_FromLong(pTextMetrics->vertAdvance));
    PyTuple_SET_ITEM(pyTextMetrics,  8, PyLong_FromLong(pTextMetrics->x_ppem));
    PyTuple_SET_ITEM(pyTextMetrics,  9, PyLong_FromLong(pTextMetrics->y_ppem));
    PyTuple_SET_ITEM(pyTextMetrics, 10, PyFloat_FromDouble(pTextMetrics->x_scale / 65536.0));
    PyTuple_SET_ITEM(pyTextMetrics, 11, PyFloat_FromDouble(pTextMetrics->y_scale / 65536.0));
    PyTuple_SET_ITEM(pyTextMetrics, 12, PyLong_FromLong(pTextMetrics->ascender));
    PyTuple_SET_ITEM(pyTextMetrics, 13, PyLong_FromLong(pTextMetrics->descender));
    PyTuple_SET_ITEM(pyTextMetrics, 14, PyLong_FromLong(pTextMetrics->px_height));
    PyTuple_SET_ITEM(pyTextMetrics, 15, PyLong_FromLong(pTextMetrics->max_advance));
    return pyTextMetrics;
}

TCAX_PyTextMetrics tcaxlib_get_text_metrics(PyObject *self, PyObject *args) {
    int arg_num;
    arg_num = PyTuple_GET_SIZE(args);
    if (arg_num >= 2 && arg_num < 6) {
        PyObject *pyArg1, *pyArg2;
        TCAX_pFont pFont;
        TCAX_TextMetrics textMetrics;
        const wchar_t *text;
        pyArg1 = PyTuple_GET_ITEM(args, 0);
        if (!PyLong_Check(pyArg1)) {
            PyErr_SetString(PyExc_RuntimeError, "TextMetrics error, the 1st param should be an integer - `pyFont'\n");
            return NULL;
        }
        pyArg2 = PyTuple_GET_ITEM(args, 1);
        if (!PyUnicode_Check(pyArg2)) {
            PyErr_SetString(PyExc_RuntimeError, "TextMetrics error, the 2nd param should be a unicode string - `text'\n");
            return NULL;
        }
        pFont = (TCAX_pFont)PyLong_AsUnsignedLong(pyArg1);
        text = (const wchar_t *)PyUnicode_AsUnicode(pyArg2);
        if (_tcaxlib_get_text_metrics(pFont, text, &textMetrics) != 0) {
            PyErr_SetString(PyExc_RuntimeError, "TextMetrics error, failed to get the text metrics!\n");
            return NULL;
        }
        return _tcaxlib_convert_text_metrics(&textMetrics);
    } else if (arg_num >= 6) {
        PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5, *pyArg6;
        const wchar_t *fontFilename;
        int fontFaceID;
        int fontSize;
        int spacing;
        double spaceScale;
        const wchar_t *text;
        TCAX_Font font;
        TCAX_TextMetrics textMetrics;
        pyArg1 = PyTuple_GET_ITEM(args, 0);
        if (!PyUnicode_Check(pyArg1)) {
            PyErr_SetString(PyExc_RuntimeError, "TextMetrics error, the 1st param should be a unicode string - `font_file'\n");
            return NULL;
        }
        pyArg2 = PyTuple_GET_ITEM(args, 1);
        if (!PyLong_Check(pyArg2)) {
            PyErr_SetString(PyExc_RuntimeError, "TextMetrics error, the 2nd param should be an integer - `face_id'\n");
            return NULL;
        }
        pyArg3 = PyTuple_GET_ITEM(args, 2);
        if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
            PyErr_SetString(PyExc_RuntimeError, "TextMetrics error, the 3rd param should be an integer - `font_size'\n");
            return NULL;
        }
        pyArg4 = PyTuple_GET_ITEM(args, 3);
        if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
            PyErr_SetString(PyExc_RuntimeError, "TextMetrics error, the 4th param should be an integer - `spacing'\n");
            return NULL;
        }
        pyArg5 = PyTuple_GET_ITEM(args, 4);
        if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
            PyErr_SetString(PyExc_RuntimeError, "TextMetrics error, the 5th param should be a float - `space_scale'\n");
            return NULL;
        }
        pyArg6 = PyTuple_GET_ITEM(args, 5);
        if (!PyUnicode_Check(pyArg6)) {
            PyErr_SetString(PyExc_RuntimeError, "TextMetrics error, the 6th param should be a unicode string - `text'\n");
            return NULL;
        }
        fontFilename = (const wchar_t *)PyUnicode_AsUnicode(pyArg1);
        fontFaceID = (int)PyLong_AsLong(pyArg2);
        fontSize = (int)PyLong_AsLong(pyArg3);
        spacing = (int)PyLong_AsLong(pyArg4);
        spaceScale = PyFloat_AsDouble(pyArg5);
        text = (const wchar_t *)PyUnicode_AsUnicode(pyArg6);
        if (tcaxlib_init_font(&font, fontFilename, fontFaceID, fontSize, spacing, spaceScale, 0, 0, 0) != 0) {
            PyErr_SetString(PyExc_RuntimeError, "TextMetrics error, failed to initialize the font!\n");
            return NULL;
        }
        if (_tcaxlib_get_text_metrics(&font, text, &textMetrics) != 0) {
            PyErr_SetString(PyExc_RuntimeError, "TextMetrics error, failed to get the text metrics!\n");
            return NULL;
        }
        tcaxlib_fin_font(&font);
        return _tcaxlib_convert_text_metrics(&textMetrics);
    } else {
        PyErr_SetString(PyExc_RuntimeError, "TextMetrics error, too less parameters - `(pyFont, text)' or `(fontFilename, fontFaceID, fontSize, spacing, spaceScale, text)'\n");
        return NULL;
    }
}

