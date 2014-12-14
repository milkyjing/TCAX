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

#include "utility.h"


/**
 * pString->buf need to be allocated before being passed to the function, and it will be reallocated if more space is needed
 * pString->len and pString->capacity should also be specified before being passed to the function, and their value may be modified
 * step is the value that each time the capacity will be increased
 */
static void _tcaxlib_string_append(TCAX_pString pString, const wchar_t *ap, int apLen) {
    wchar_t *wcs;
    int len, capacity;
    wcs = pString->buf;
    len = pString->len;
    capacity = pString->capacity;
    if (len + apLen + 1 > capacity) {  /* we need more space */
        while (len + apLen + 1 > capacity)
            capacity += capacity / 2 + 1;
        wcs = (wchar_t *)realloc(wcs, capacity * sizeof(wchar_t));
        pString->buf = wcs;
        pString->capacity = capacity;
    }
    memcpy(wcs + len, ap, apLen * sizeof(wchar_t));
    len += apLen;
    wcs[len] = L'\0';
    pString->len = len;
}

static int _outline_drawing_move_to(const FT_Vector *to, void *user) {
    wchar_t buf[256];
    TCAX_pOutlineString pOutline = (TCAX_pOutlineString)user;
    wsprintf(buf, L"m %d %d ", pOutline->offset.x + to->x, pOutline->offset.y + pOutline->height - to->y);
    _tcaxlib_string_append(&pOutline->string, buf, wcslen(buf));
    pOutline->last.x = to->x;
    pOutline->last.y = to->y;
    return 0;
}

static int _outline_drawing_line_to(const FT_Vector *to, void *user) {
    wchar_t buf[256];
    TCAX_pOutlineString pOutline = (TCAX_pOutlineString)user;
    wsprintf(buf, L"l %d %d ", pOutline->offset.x + to->x, pOutline->offset.y + pOutline->height - to->y);
    _tcaxlib_string_append(&pOutline->string, buf, wcslen(buf));
    pOutline->last.x = to->x;
    pOutline->last.y = to->y;
    return 0;
}

static int _outline_drawing_conic_to(const FT_Vector *control, const FT_Vector *to, void *user) {
    wchar_t buf[256];
    int xs, ys, xc, yc, xe, ye, xc1, yc1, xc2, yc2;
    TCAX_pOutlineString pOutline = (TCAX_pOutlineString)user;
    xs = pOutline->last.x;
    ys = pOutline->last.y;
    xc = control->x;
    yc = control->y;
    xe = to->x;
    ye = to->y;
    xc1 = (xs + 2 * xc) / 3;
    yc1 = (ys + 2 * yc) / 3;
    xc2 = (xe + 2 * xc) / 3;
    yc2 = (ye + 2 * yc) / 3;
    wsprintf(buf, L"b %d %d %d %d %d %d ", pOutline->offset.x + xc1, pOutline->offset.y + pOutline->height - yc1, pOutline->offset.x + xc2, pOutline->offset.y + pOutline->height - yc2, pOutline->offset.x + xe, pOutline->offset.y + pOutline->height - ye);
    _tcaxlib_string_append(&pOutline->string, buf, wcslen(buf));
    pOutline->last.x = to->x;
    pOutline->last.y = to->y;
    return 0;
}

static int _outline_drawing_cubic_to(const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user) {
    wchar_t buf[256];
    TCAX_pOutlineString pOutline = (TCAX_pOutlineString)user;
    wsprintf(buf, L"b %d %d %d %d %d %d ", pOutline->offset.x + control1->x, pOutline->offset.y + pOutline->height - control1->y, pOutline->offset.x + control2->x, pOutline->offset.y + pOutline->height - control2->y, pOutline->offset.x + to->x, pOutline->offset.y + pOutline->height - to->y);
    _tcaxlib_string_append(&pOutline->string, buf, wcslen(buf));
    pOutline->last.x = to->x;
    pOutline->last.y = to->y;
    return 0;
}

static int _tcaxlib_get_text_outline_drawing_string(const TCAX_pFont pFont, wchar_t text, int x, int y, wchar_t **pDrawing, int *pLen) {
    FT_UInt index;
    FT_Glyph glyph;
    FT_OutlineGlyph outline;
    FT_Outline_Funcs outInterface;
    TCAX_OutlineString outlineString;
    /* get parameters from script */
    index = FT_Get_Char_Index(pFont->face, text);
    if (0 == index) {
        printf("TextOutlineDraw Error: find an invalid text that does not exists in the font.\n");
        return -1;
    }
    FT_Load_Glyph(pFont->face, index, FT_LOAD_DEFAULT);
    FT_Get_Glyph(pFont->face->glyph, &glyph);
    if (FT_GLYPH_FORMAT_OUTLINE != glyph->format) {
        printf("TextOutlineDraw Error: cannot get the outline of the text.\n");
        return -1;
    }
    outline = (FT_OutlineGlyph)glyph;
    outlineString.offset.x = x * 64;
    outlineString.offset.y = y * 64;
    outlineString.height = (pFont->size << 6) + pFont->face->size->metrics.descender;
    outlineString.last.x = 0;
    outlineString.last.y = 0;
    outlineString.string.len = 0;
    outlineString.string.capacity = 10;
    outlineString.string.buf = (wchar_t *)malloc(outlineString.string.capacity * sizeof(wchar_t));
    outInterface.move_to = (FT_Outline_MoveToFunc)_outline_drawing_move_to;
    outInterface.line_to = (FT_Outline_LineToFunc)_outline_drawing_line_to;
    outInterface.conic_to = (FT_Outline_ConicToFunc)_outline_drawing_conic_to;
    outInterface.cubic_to = (FT_Outline_CubicToFunc)_outline_drawing_cubic_to;
    outInterface.shift = 0;
    outInterface.delta = 0;
    FT_Outline_Decompose(&outline->outline, &outInterface, &outlineString);
    _tcaxlib_string_append(&outlineString.string, L"c", 1);
    FT_Done_Glyph(glyph);
    *pDrawing = outlineString.string.buf;
    *pLen = outlineString.string.len;
    return 0;
}

TCAX_PyString tcaxlib_get_text_outline_as_string(PyObject *self, PyObject *args) {
    int arg_num;
    arg_num = PyTuple_GET_SIZE(args);
    if (arg_num >= 4 && arg_num < 6) {
        PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4;
        TCAX_pFont pFont;
        const wchar_t *text;
        int x, y;
        wchar_t *drawing;
        int len;
        TCAX_PyString pyDrawing;
        pyArg1 = PyTuple_GET_ITEM(args, 0);
        if (!PyLong_Check(pyArg1)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlineDraw error, the 1st param should be an integer - `pyFont'\n");
            return NULL;
        }
        pyArg2 = PyTuple_GET_ITEM(args, 1);
        if (!PyUnicode_Check(pyArg2)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlineDraw error, the 2nd param should be a unicode string - `text'\n");
            return NULL;
        }
        pyArg3 = PyTuple_GET_ITEM(args, 2);
        if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlineDraw error, the 3rd param should be an integer - `x'\n");
            return NULL;
        }
        pyArg4 = PyTuple_GET_ITEM(args, 3);
        if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlineDraw error, the 4th param should be an integer - `y'\n");
            return NULL;
        }
        pFont = (TCAX_pFont)PyLong_AsUnsignedLong(pyArg1);
        text = (const wchar_t *)PyUnicode_AsUnicode(pyArg2);
        x = (int)PyLong_AsLong(pyArg3);
        y = (int)PyLong_AsLong(pyArg4);
        if (_tcaxlib_get_text_outline_drawing_string(pFont, text[0], x, y, &drawing, &len) != 0) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlineDraw error, failed to get the outline!\n");
            return NULL;
        }
        pyDrawing = PyUnicode_FromUnicode((const Py_UNICODE *)drawing, len);
        free(drawing);
        return pyDrawing;
    } else if (arg_num >= 6) {
        PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5, *pyArg6;
        const wchar_t *fontFilename;
        int fontFaceID;
        int fontSize;
        const wchar_t *text;
        int x, y;
        TCAX_Font font;
        wchar_t *drawing;
        int len;
        TCAX_PyString pyDrawing;
        pyArg1 = PyTuple_GET_ITEM(args, 0);
        if (!PyUnicode_Check(pyArg1)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlineDraw error, the 1st param should be a unicode string - `font_file'\n");
            return NULL;
        }
        pyArg2 = PyTuple_GET_ITEM(args, 1);
        if (!PyLong_Check(pyArg2)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlineDraw error, the 2nd param should be an integer - `face_id'\n");
            return NULL;
        }
        pyArg3 = PyTuple_GET_ITEM(args, 2);
        if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlineDraw error, the 3rd param should be an integer - `font_size'\n");
            return NULL;
        }
        pyArg4 = PyTuple_GET_ITEM(args, 3);
        if (!PyUnicode_Check(pyArg4)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlineDraw error, the 4th param should be a unicode string - `text'\n");
            return NULL;
        }
        pyArg5 = PyTuple_GET_ITEM(args, 4);
        if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlineDraw error, the 5th param should be a float - `x'\n");
            return NULL;
        }
        pyArg6 = PyTuple_GET_ITEM(args, 5);
        if (!PyLong_Check(pyArg6) && !PyFloat_Check(pyArg6)) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlineDraw error, the 6th param should be an integer - `y'\n");
            return NULL;
        }
        fontFilename = (const wchar_t *)PyUnicode_AsUnicode(pyArg1);
        fontFaceID = (int)PyLong_AsLong(pyArg2);
        fontSize = (int)PyLong_AsLong(pyArg3);
        text = (const wchar_t *)PyUnicode_AsUnicode(pyArg4);
        x = (int)PyLong_AsLong(pyArg5);
        y = (int)PyLong_AsLong(pyArg6);
        if (tcaxlib_init_font(&font, fontFilename, fontFaceID, fontSize, 0, 0, 0, 0, 0) != 0) {
            PyErr_SetString(PyExc_RuntimeError, "TextOutlineDraw error, failed to initialize the font!\n");
            return NULL;
        }
        if (_tcaxlib_get_text_outline_drawing_string(&font, text[0], x, y, &drawing, &len) != 0) {
            tcaxlib_fin_font(&font);
            PyErr_SetString(PyExc_RuntimeError, "TextOutlineDraw error, failed to get the outline!\n");
            return NULL;
        }
        tcaxlib_fin_font(&font);
        pyDrawing = PyUnicode_FromUnicode((const Py_UNICODE *)drawing, len);
        free(drawing);
        return pyDrawing;
    } else {
        PyErr_SetString(PyExc_RuntimeError, "TextOutlineDraw error, too less parameters - `(pyFont, text, x, y)' or `(font_file, face_id, font_size, text, x, y)'\n");
        return NULL;
    }
}

static int _tcaxlib_is_text_c_or_j_or_k(const wchar_t *text) {
    int i, count;
    count = wcslen(text);
    for (i = 0; i < count; i++) {
        if (text[0] >= 0x3000 && text[1] <= 0x9FFF) return 1;
    }
    return 0;
}

TCAX_Py_Error_Code tcaxlib_is_c_or_j_or_k(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    const wchar_t *text;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "IsCjk error, too less parameters - `(text)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyUnicode_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "IsCjk error, the 1st param should be a unicode string - `text'\n");
        return NULL;
    }
    text = (const wchar_t *)PyUnicode_AsUnicode(pyArg1);
    if (_tcaxlib_is_text_c_or_j_or_k(text)) return Py_BuildValue("i", 1);
    else return PyLong_FromLong(0);
}

TCAX_PyString tcaxlib_vertical_layout_ass(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    int count, len;
    const wchar_t *text;
    wchar_t *textWithAssTags;
    TCAX_PyString pyTextWithAssTags;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "VertLayout error, too less parameters - `(text)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyUnicode_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "VertLayout error, the 1st param should be a unicode string - `text'\n");
        return NULL;
    }
    text = (const wchar_t *)PyUnicode_AsUnicode(pyArg1);
    count = wcslen(text);
    if (_tcaxlib_is_text_c_or_j_or_k(text)) {    /* Chinese or Japanese or Korea characters needn't be rotated */
        int i, offset;
        len = 1 + (count - 1) * 3;    /* T\\NT\\NT\0 */
        textWithAssTags = (wchar_t *)malloc((len + 1) * sizeof(wchar_t));
        offset = 0;
        textWithAssTags[offset] = text[0];
        offset++;
        for (i = 1; i < count; i++) {
            memcpy(textWithAssTags + offset, L"\\N", 2 * sizeof(wchar_t));
            offset += 2;
            textWithAssTags[offset] = text[i];
            offset++;
        }
        textWithAssTags[offset] = L'\0';
    } else {
        len = 9 + count;
        textWithAssTags = (wchar_t *)malloc((len + 1) * sizeof(wchar_t));
        memcpy(textWithAssTags, L"{\\frz270}", 9 * sizeof(wchar_t));
        memcpy(textWithAssTags + 9, text, count * sizeof(wchar_t));
        textWithAssTags[9 + count] = L'\0';
    }
    pyTextWithAssTags = PyUnicode_FromUnicode(textWithAssTags, len);
    free(textWithAssTags);
    return pyTextWithAssTags;
}

TCAX_Py_Error_Code tcaxlib_show_progress(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4;
    int total, completed, file_id, file_num;
    HANDLE hStdout;
    COORD coord;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (PyTuple_GET_SIZE(args) < 4) {
        PyErr_SetString(PyExc_RuntimeError, "ShowProgress error, too less parameters - `(total, comopleted, file_id, file_num)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "ShowProgress error, the 1st param should be an integer - `total'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "ShowProgress error, the 2nd param should be an integer - `comopleted'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "ShowProgress error, the 3rd param should be an integer - `file_id'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "ShowProgress error, the 4th param should be an integer - `file_num'\n");
        return NULL;
    }
    total = (int)PyLong_AsLong(pyArg1);
    completed = (int)PyLong_AsLong(pyArg2);
    file_id = (int)PyLong_AsLong(pyArg3);
    file_num = (int)PyLong_AsLong(pyArg4);
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hStdout, &csbi);
    coord.X = 0;
    coord.Y = (1 == completed) ? csbi.dwCursorPosition.Y + 1 : csbi.dwCursorPosition.Y - 1;
    SetConsoleCursorPosition(hStdout, coord);
    printf("Progress: %.2f%c\n", 100 * (file_id + completed / (double)total) / file_num, '%');
    return PyLong_FromLong(0);
}

TCAX_PyPoints tcaxlib_bezier_curve_linear(PyObject *self, PyObject *args) {   /* the return value has a pattern of ((x1, y1, a1), (x2, y2, a2), ...) */
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5;
    int i, points;
    double xs, ys, xe, ye, x, y, t;
    TCAX_PyPoints pyPoints;
    PyObject *pyPointTemp;
    if (PyTuple_GET_SIZE(args) < 5) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier1 error, too less parameters - `(nPoints, xs, ys, xe, ye)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier1 error, the 1st param should be an integer - `nPoints'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier1 error, the 2nd param should be a float - `xs'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier1 error, the 3rd param should be a float - `ys'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier1 error, the 4th param should be a float - `xe'\n");
        return NULL;
    }
    pyArg5 = PyTuple_GET_ITEM(args, 4);
    if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier1 error, the 5th param should be a float - `ye'\n");
        return NULL;
    }
    points = PyLong_AsLong(pyArg1);
    xs = PyFloat_AsDouble(pyArg2);
    ys = PyFloat_AsDouble(pyArg3);
    xe = PyFloat_AsDouble(pyArg4);
    ye = PyFloat_AsDouble(pyArg5);
    pyPoints = PyTuple_New(points);
    for (i = 0; i < points; i++) {
        t = i / (double)(points - 1);
        /*x = (1 - t) * xs + t * xe;
        y = (1 - t) * ys + t * ye;*/
        linear_interpolation(&x, &y, xs, ys, xe, ye, t);
        pyPointTemp = PyTuple_New(3);
        PyTuple_SET_ITEM(pyPointTemp, 0, PyFloat_FromDouble(x));
        PyTuple_SET_ITEM(pyPointTemp, 1, PyFloat_FromDouble(y));
        PyTuple_SET_ITEM(pyPointTemp, 2, PyLong_FromLong(255));
        PyTuple_SET_ITEM(pyPoints, i, pyPointTemp);
    }
    return pyPoints;
}

TCAX_PyPoints tcaxlib_bezier_curve_quadratic(PyObject *self, PyObject *args) {   /* the return value has a pattern of ((x1, y1, a1), (x2, y2, a2), ...) */
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5, *pyArg6, *pyArg7;
    int i, points;
    double xs, ys, xe, ye, xc, yc, x, y, t;
    TCAX_PyPoints pyPoints;
    PyObject *pyPointTemp;
    if (PyTuple_GET_SIZE(args) < 7) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier2 error, too less parameters - `(nPoints, xs, ys, xe, ye, xc, yc)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier2 error, the 1st param should be an integer - `nPoints'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier2 error, the 2nd param should be a float - `xs'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier2 error, the 3rd param should be a float - `ys'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier2 error, the 4th param should be a float - `xe'\n");
        return NULL;
    }
    pyArg5 = PyTuple_GET_ITEM(args, 4);
    if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier2 error, the 5th param should be a float - `ye'\n");
        return NULL;
    }
    pyArg6 = PyTuple_GET_ITEM(args, 5);
    if (!PyLong_Check(pyArg6) && !PyFloat_Check(pyArg6)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier2 error, the 6th param should be a float - `xc'\n");
        return NULL;
    }
    pyArg7 = PyTuple_GET_ITEM(args, 6);
    if (!PyLong_Check(pyArg7) && !PyFloat_Check(pyArg7)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier2 error, the 7th param should be a float - `yc'\n");
        return NULL;
    }
    points = PyLong_AsLong(pyArg1);
    xs = PyFloat_AsDouble(pyArg2);
    ys = PyFloat_AsDouble(pyArg3);
    xe = PyFloat_AsDouble(pyArg4);
    ye = PyFloat_AsDouble(pyArg5);
    xc = PyFloat_AsDouble(pyArg6);
    yc = PyFloat_AsDouble(pyArg7);
    pyPoints = PyTuple_New(points);
    for (i = 0; i < points; i++) {
        t = i / (double)(points - 1);
        /*x = (1 - t) * (1 - t) * xs + 2 * t * (1 - t) * xc + t * t * xe;
        y = (1 - t) * (1 - t) * ys + 2 * t * (1 - t) * yc + t * t * ye;*/
        conic_bezier(&x, &y, xs, ys, xc, yc, xe, ye, t);
        pyPointTemp = PyTuple_New(3);
        PyTuple_SET_ITEM(pyPointTemp, 0, PyFloat_FromDouble(x));
        PyTuple_SET_ITEM(pyPointTemp, 1, PyFloat_FromDouble(y));
        PyTuple_SET_ITEM(pyPointTemp, 2, PyLong_FromLong(255));
        PyTuple_SET_ITEM(pyPoints, i, pyPointTemp);
    }
    return pyPoints;
}

TCAX_PyPoints tcaxlib_bezier_curve_cubic(PyObject *self, PyObject *args) {   /* the return value has a pattern of ((x1, y1, a1), (x2, y2, a2), ...) */
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5, *pyArg6, *pyArg7, *pyArg8, *pyArg9;
    int i, points;
    double xs, ys, xe, ye, xc1, yc1, xc2, yc2, x, y, t;
    TCAX_PyPoints pyPoints;
    PyObject *pyPointTemp;
    if (PyTuple_GET_SIZE(args) < 9) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier3 error, too less parameters - `(nPoints, xs, ys, xe, ye, xc1, yc1, xc2, yc2)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier3 error, the 1st param should be an integer - `nPoints'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier3 error, the 2nd param should be a float - `xs'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier3 error, the 3rd param should be a float - `ys'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier3 error, the 4th param should be a float - `xe'\n");
        return NULL;
    }
    pyArg5 = PyTuple_GET_ITEM(args, 4);
    if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier3 error, the 5th param should be a float - `ye'\n");
        return NULL;
    }
    pyArg6 = PyTuple_GET_ITEM(args, 5);
    if (!PyLong_Check(pyArg6) && !PyFloat_Check(pyArg6)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier3 error, the 6th param should be a float - `xc1'\n");
        return NULL;
    }
    pyArg7 = PyTuple_GET_ITEM(args, 6);
    if (!PyLong_Check(pyArg7) && !PyFloat_Check(pyArg7)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier3 error, the 7th param should be a float - `yc1'\n");
        return NULL;
    }
    pyArg8 = PyTuple_GET_ITEM(args, 7);
    if (!PyLong_Check(pyArg8) && !PyFloat_Check(pyArg8)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier3 error, the 8th param should be a float - `xc2'\n");
        return NULL;
    }
    pyArg9 = PyTuple_GET_ITEM(args, 8);
    if (!PyLong_Check(pyArg9) && !PyFloat_Check(pyArg9)) {
        PyErr_SetString(PyExc_RuntimeError, "Bezier3 error, the 9th param should be a float - `yc2'\n");
        return NULL;
    }
    points = PyLong_AsLong(pyArg1);
    xs = PyFloat_AsDouble(pyArg2);
    ys = PyFloat_AsDouble(pyArg3);
    xe = PyFloat_AsDouble(pyArg4);
    ye = PyFloat_AsDouble(pyArg5);
    xc1 = PyFloat_AsDouble(pyArg6);
    yc1 = PyFloat_AsDouble(pyArg7);
    xc2 = PyFloat_AsDouble(pyArg8);
    yc2 = PyFloat_AsDouble(pyArg9);
    pyPoints = PyTuple_New(points);
    for (i = 0; i < points; i++) {
        t = i / (double)(points - 1);
        /*x = (1 - t) * (1 - t) * (1 - t) * xs + 3 * t * (1 - t) * (1 - t) * xc1 + 3 * t * t * (1 - t) * xc2 + t * t * t * xe;
        y = (1 - t) * (1 - t) * (1 - t) * ys + 3 * t * (1 - t) * (1 - t) * yc1 + 3 * t * t * (1 - t) * yc2 + t * t * t * ye;*/
        cubic_bezier(&x, &y, xs, ys, xc1, yc1, xc2, yc2, xe, ye, t);
        pyPointTemp = PyTuple_New(3);
        PyTuple_SET_ITEM(pyPointTemp, 0, PyFloat_FromDouble(x));
        PyTuple_SET_ITEM(pyPointTemp, 1, PyFloat_FromDouble(y));
        PyTuple_SET_ITEM(pyPointTemp, 2, PyLong_FromLong(255));
        PyTuple_SET_ITEM(pyPoints, i, pyPointTemp);
    }
    return pyPoints;
}

long _Fac(int n) {
    int i;
    long N = 1;
    for (i = 2; i <= n; i++)
        N *= i;
    return N;
}

long _Combi(int n, int m) {
    return _Fac(n) / (_Fac(n - m) * _Fac(m));
}

int _Randint(int r1, int r2) {
    if (r1 > r2) return r2 + rand() % (r1 - r2 + 1);
    return r1 + rand() % (r2 - r1 + 1);
}

TCAX_PyPoints tcaxlib_bezier_curve_random(PyObject *self, PyObject *args) {   /* the return value has a pattern of ((x1, y1, a1), (x2, y2, a2), ...) */
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5, *pyArg6, *pyArg7, *pyArg8, *pyArg9, *pyArg10;
    int i, j, points, order;
    double xs, ys, xe, ye, xl1, yl1, xl2, yl2, x, y, t;
    int x1, y1, x2, y2;
    double *xptr, *yptr;
    TCAX_PyPoints pyPoints;
    PyObject *pyPointTemp;
    if (PyTuple_GET_SIZE(args) < 10) {
        PyErr_SetString(PyExc_RuntimeError, "BezierN error, too less parameters - `(nPoints, xs, ys, xe, ye, xl1, yl1, xl2, yl2, order)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "BezierN error, the 1st param should be an integer - `nPoints'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "BezierN error, the 2nd param should be a float - `xs'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "BezierN error, the 3rd param should be a float - `ys'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "BezierN error, the 4th param should be a float - `xe'\n");
        return NULL;
    }
    pyArg5 = PyTuple_GET_ITEM(args, 4);
    if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
        PyErr_SetString(PyExc_RuntimeError, "BezierN error, the 5th param should be a float - `ye'\n");
        return NULL;
    }
    pyArg6 = PyTuple_GET_ITEM(args, 5);
    if (!PyLong_Check(pyArg6) && !PyFloat_Check(pyArg6)) {
        PyErr_SetString(PyExc_RuntimeError, "BezierN error, the 6th param should be a float - `xl1'\n");
        return NULL;
    }
    pyArg7 = PyTuple_GET_ITEM(args, 6);
    if (!PyLong_Check(pyArg7) && !PyFloat_Check(pyArg7)) {
        PyErr_SetString(PyExc_RuntimeError, "BezierN error, the 7th param should be a float - `yl1'\n");
        return NULL;
    }
    pyArg8 = PyTuple_GET_ITEM(args, 7);
    if (!PyLong_Check(pyArg8) && !PyFloat_Check(pyArg8)) {
        PyErr_SetString(PyExc_RuntimeError, "BezierN error, the 8th param should be a float - `xl2'\n");
        return NULL;
    }
    pyArg9 = PyTuple_GET_ITEM(args, 8);
    if (!PyLong_Check(pyArg9) && !PyFloat_Check(pyArg9)) {
        PyErr_SetString(PyExc_RuntimeError, "BezierN error, the 9th param should be a float - `yl2'\n");
        return NULL;
    }
    pyArg10 = PyTuple_GET_ITEM(args, 9);
    if (!PyLong_Check(pyArg10)) {
        PyErr_SetString(PyExc_RuntimeError, "BezierN error, the 10th param should be an integer - `order'\n");
        return NULL;
    }
    points = PyLong_AsLong(pyArg1);
    xs = PyFloat_AsDouble(pyArg2);
    ys = PyFloat_AsDouble(pyArg3);
    xe = PyFloat_AsDouble(pyArg4);
    ye = PyFloat_AsDouble(pyArg5);
    xl1 = PyFloat_AsDouble(pyArg6);
    yl1 = PyFloat_AsDouble(pyArg7);
    xl2 = PyFloat_AsDouble(pyArg8);
    yl2 = PyFloat_AsDouble(pyArg9);
    order = PyLong_AsLong(pyArg10);
    x1 = (int)__min(xl1, xl2);
    y1 = (int)__min(yl1, yl2);
    x2 = (int)__max(xl1, xl2);
    y2 = (int)__max(yl1, yl2);
    xptr = (double *)malloc((order + 1) * sizeof(double));
    yptr = (double *)malloc((order + 1) * sizeof(double));
    xptr[0] = xs;
    yptr[0] = ys;
    xptr[order] = xe;
    xptr[order] = ye;
    for (i = 1; i < order; i++) {
        xptr[i] = _Randint(x1, x2);
        yptr[i] = _Randint(y1, y2);
    }
    pyPoints = PyTuple_New(points);
    for (i = 0; i < points; i++) {
        x = 0;
        y = 0;
        t = i / (double)(points - 1);
        for (j = 0; j < order + 1; j++) {
            x += _Combi(order, j) * pow(1 - t, order - j) * pow(t, j) * xptr[j];
            y += _Combi(order, j) * pow(1 - t, order - j) * pow(t, j) * yptr[j];
        }
        pyPointTemp = PyTuple_New(3);
        PyTuple_SET_ITEM(pyPointTemp, 0, PyFloat_FromDouble(x));
        PyTuple_SET_ITEM(pyPointTemp, 1, PyFloat_FromDouble(y));
        PyTuple_SET_ITEM(pyPointTemp, 2, PyLong_FromLong(255));
        PyTuple_SET_ITEM(pyPoints, i, pyPointTemp);
    }
    free(xptr);
    free(yptr);
    return pyPoints;
}

TCAX_PyFloat tcaxlib_get_actual_font_size(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3;
    const wchar_t *fontFilename;
    int fontFaceID;
    int fontSize;
    char *filename;
    FT_Library library;
    FT_Face face;
    TT_HoriHeader *hori;
    TT_OS2 *os2;
    double scale;
    if (PyTuple_GET_SIZE(args) < 3) {
        PyErr_SetString(PyExc_RuntimeError, "GetFontSize error, too less parameters - `(font_file, face_id, font_size)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyUnicode_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "GetFontSize error, the 1st param should be a unicode string - `font_file'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "GetFontSize error, the 2nd param should be an integer - `face_id'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "GetFontSize error, the 3rd param should be an integer - `font_size'\n");
        return NULL;
    }
    fontFilename = (const wchar_t *)PyUnicode_AsUnicode(pyArg1);
    fontFaceID = (int)PyLong_AsLong(pyArg2);
    fontSize = (int)PyLong_AsLong(pyArg3);
    tcaxlib_sz_unicode_to_ansi(fontFilename, &filename);
    if (FT_Init_FreeType(&library) != 0) {
        free(filename);
        PyErr_SetString(PyExc_RuntimeError, "GetFontSize error, failed to initialize the font.\n");
        return NULL;
    }
    if (FT_New_Face(library, filename, fontFaceID, &face) != 0) {
        free(filename);
        FT_Done_FreeType(library);
        PyErr_SetString(PyExc_RuntimeError, "GetFontSize error, failed to initialize the font.\n");
        return NULL;
    }
    free(filename);
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
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    return PyFloat_FromDouble(fontSize * scale);
}

TCAX_PyLong tcaxlib_calc_cairo_font_size(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3;
    const wchar_t *fontFilename;
    int fontFaceID;
    int fontSize;
    char *filename;
    FT_Library library;
    FT_Face face;
    TT_HoriHeader *hori;
    TT_OS2 *os2;
    double scale;
    FT_Size_RequestRec rq;
    int fontSize2;  /* the NOMINAL font size */
    if (PyTuple_GET_SIZE(args) < 3) {
        PyErr_SetString(PyExc_RuntimeError, "CairoFontSize error, too less parameters - `(font_file, face_id, font_size)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyUnicode_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "CairoFontSize error, the 1st param should be a unicode string - `font_file'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "CairoFontSize error, the 2nd param should be an integer - `face_id'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "CairoFontSize error, the 3rd param should be an integer - `font_size'\n");
        return NULL;
    }
    fontFilename = (const wchar_t *)PyUnicode_AsUnicode(pyArg1);
    fontFaceID = (int)PyLong_AsLong(pyArg2);
    fontSize = (int)PyLong_AsLong(pyArg3);
    tcaxlib_sz_unicode_to_ansi(fontFilename, &filename);
    if (FT_Init_FreeType(&library) != 0) {
        free(filename);
        PyErr_SetString(PyExc_RuntimeError, "CairoFontSize error, failed to initialize the font.\n");
        return NULL;
    }
    if (FT_New_Face(library, filename, fontFaceID, &face) != 0) {
        free(filename);
        FT_Done_FreeType(library);
        PyErr_SetString(PyExc_RuntimeError, "CairoFontSize error, failed to initialize the font.\n");
        return NULL;
    }
    free(filename);
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
    rq.height = (long)(fontSize * scale * 64);
    rq.horiResolution = 0;
    rq.vertResolution = 0;
    FT_Request_Size(face, &rq);
    fontSize2 = (int)(fontSize * scale * face->units_per_EM / (double)(face->ascender - face->descender) + 0.5);
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    return PyLong_FromLong(fontSize2);
}

