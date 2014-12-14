/* 
 *  Copyright (C) 2012 milkyjing <milkyjing@gmail.com>
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

#include <Python.h>
#include <Windows.h>

#define SCALE_FACTOR 8


typedef struct _gdifont_font {
    HDC hDC;
    HFONT hFont;
    HPEN hPen;
    double fontSize;
    double spacing;
    double spaceScale;
    double bord;
    int is_outline;
} gdifont_font;

typedef struct _gdifont_points {
    double *xBuf;
    double *yBuf;
    double *aBuf;
    int count;       /**< number of points */
    int capacity;    /**< maximun points that the buffer can contain */
} gdifont_points;

typedef PyObject *TCAX_PyPoints;

typedef struct _gdifont_string {
    wchar_t *buf;
    int len;
    int capacity;
} gdifont_string;

static void _gdifont_points_append(gdifont_points *pPoints, double x, double y, double a) {
    double *xBuf, *yBuf, *aBuf;
    int count, capacity;
    xBuf = pPoints->xBuf;
    yBuf = pPoints->yBuf;
    aBuf = pPoints->aBuf;
    count = pPoints->count;
    capacity = pPoints->capacity;
    if (count + 1 > capacity) {  /* we need more space */
        capacity += capacity / 2 + 1;
        xBuf = (double *)realloc(xBuf, capacity * sizeof(double));
        yBuf = (double *)realloc(yBuf, capacity * sizeof(double));
        aBuf = (double *)realloc(aBuf, capacity * sizeof(double));
        pPoints->xBuf = xBuf;
        pPoints->yBuf = yBuf;
        pPoints->aBuf = aBuf;
        pPoints->capacity = capacity;
    }
    xBuf[count] = x;
    yBuf[count] = y;
    aBuf[count] = a;
    count++;
    pPoints->count = count;
}

static void _gdifont_points_clear(gdifont_points *pPoints) {
    free(pPoints->xBuf);
    free(pPoints->yBuf);
    free(pPoints->aBuf);
    pPoints->xBuf = NULL;
    pPoints->yBuf = NULL;
    pPoints->aBuf = NULL;
    pPoints->count = 0;
    pPoints->capacity = 0;
}

static void _gdifont_string_append(gdifont_string *pString, const wchar_t *ap, int apLen) {
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

static void _gdifont_string_clear(gdifont_string *pString) {
    free(pString->buf);
    pString->buf = NULL;
    pString->len = 0;
    pString->capacity = 0;
}

/* Create a GDI font */
static HFONT _gdifont_create_font(HDC hDC, const wchar_t *faceName, double fontSize) {
    LOGFONTW lf;
    HFONT hFont;
    ZeroMemory(&lf, sizeof(LOGFONTW));
    lf.lfHeight = (long)(fontSize * SCALE_FACTOR + 0.5);
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_TT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = ANTIALIASED_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    wcsncpy(lf.lfFaceName, faceName, 32);
    hFont = CreateFontIndirectW(&lf);
    return hFont;
}

static PyObject *gdifont_init_font(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4, *pyArg5, *pyArg6;
    const wchar_t *faceName;
    gdifont_font *font;
    HDC hDC;
    HFONT hFont;
    HPEN hPen;
    if (PyTuple_GET_SIZE(args) < 6) {
        PyErr_SetString(PyExc_RuntimeError, "gfInitFont error, too less parameters - `(font_face_name, font_size, spacing, space_scale, bord, is_outline)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyUnicode_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "gfInitFont error, the 1st param should be a unicode string - `font_face_name'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "gfInitFont error, the 2nd param should be a float - `font_size'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "gfInitFont error, the 3rd param should be a float - `spacing'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "gfInitFont error, the 4th param should be a float - `space_scale'\n");
        return NULL;
    }
    pyArg5 = PyTuple_GET_ITEM(args, 4);
    if (!PyLong_Check(pyArg5) && !PyFloat_Check(pyArg5)) {
        PyErr_SetString(PyExc_RuntimeError, "gfInitFont error, the 5th param should be a float - `bord'\n");
        return NULL;
    }
    pyArg6 = PyTuple_GET_ITEM(args, 5);
    if (!PyLong_Check(pyArg6)) {
        PyErr_SetString(PyExc_RuntimeError, "gfInitFont error, the 6th param should be an integer - `is_outline'\n");
        return NULL;
    }
    font = (gdifont_font *)malloc(sizeof(gdifont_font));
    faceName = (const wchar_t *)PyUnicode_AsUnicode(pyArg1);
    font->fontSize = PyFloat_AsDouble(pyArg2);
    font->spacing = PyFloat_AsDouble(pyArg3);
    font->spaceScale = PyFloat_AsDouble(pyArg4);
    font->bord = PyFloat_AsDouble(pyArg5);
    font->is_outline = PyLong_AsLong(pyArg6);
    hDC = CreateCompatibleDC(NULL);
    SetMapMode(hDC, MM_TEXT);
    SetBkMode(hDC, TRANSPARENT);
    SetPolyFillMode(hDC, WINDING);
    hFont = _gdifont_create_font(hDC, faceName, font->fontSize);
    SelectObject(hDC, hFont);
    if (font->bord < 0)
        hPen = CreatePen(PS_SOLID, (int)(font->bord * SCALE_FACTOR + 0.5), 0);
    else
        hPen = CreatePen(PS_SOLID, (int)(font->bord * SCALE_FACTOR + 0.5), RGB(255, 255, 255));
    SelectObject(hDC, hPen);
    font->hDC = hDC;
    font->hFont = hFont;
    font->hPen = hPen;
    return PyLong_FromUnsignedLong((unsigned long)font);
}

/* Destroy a GDI font */
static PyObject *gdifont_fin_font(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    gdifont_font *font;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "gfFinFont error, too less parameters - `(gdi_font)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "gfFinFont error, the 1st param should be an integer - `gdi_font'\n");
        return NULL;
    }
    font = (gdifont_font *)PyLong_AsUnsignedLong(pyArg1);
    DeleteObject(font->hPen);
    DeleteObject(font->hFont);
    DeleteObject(font->hDC);
    free(font);
    return PyLong_FromLong(0);
}

/* Get Points from text */
static void _gdifont_get_points(HDC hDC, int width, int height, double bord, gdifont_points *pPoints) {
    int w, h, xx, yy, a, divisor;
    divisor = SCALE_FACTOR * SCALE_FACTOR;
    for (h = 0; h < height; h += SCALE_FACTOR) {
        for (w = 0; w < width; w += SCALE_FACTOR) {
            a = 0;
            for (yy = 0; yy < SCALE_FACTOR; yy++)
                for (xx = 0; xx < SCALE_FACTOR; xx++)
                    a += GetRValue(GetPixel(hDC, w + xx, h + yy));
            if (a > 0)
                _gdifont_points_append(pPoints, w / (double)SCALE_FACTOR - bord, h / (double)SCALE_FACTOR - bord, a / (double)divisor);
        }
    }
}

static void _gdifont_text_points(gdifont_font *font, const wchar_t *text, gdifont_points *pPoints) {
    HDC hDC;
    HBITMAP hBmp;
    int i, count, offsetX, offsetY, spacing, width, height;
    SIZE size;
    hDC = font->hDC;
    BeginPath(hDC);
    offsetX = (int)(font->bord * SCALE_FACTOR + 0.5);
    offsetY = offsetX;
    spacing = (int)(font->spacing * SCALE_FACTOR + 0.5);
    count = wcslen(text);
    for (i = 0; i < count; i++) {
        TextOutW(hDC, offsetX, offsetY, text + i, 1);
        GetTextExtentPoint32W(hDC, text + i, 1, &size);
        if (' ' == text[i] || '　' == text[i])
            size.cx = (long)(size.cx * font->spaceScale + 0.5);
        offsetX += size.cx + spacing;
    }
    EndPath(hDC);
    width = (int)(((font->fontSize + font->spacing) * count + 2 * font->bord) * SCALE_FACTOR + 0.5);
    height = (int)((font->fontSize + 2 * font->bord) * SCALE_FACTOR + 0.5);
    hBmp = CreateCompatibleBitmap(hDC, width, height);      /* a bitmap which is large enough to hold the text */
    SelectObject(hDC, hBmp);
    if (font->bord == 0)
        FillPath(hDC);
    else {
        if (font->is_outline && font->bord > 0)
            StrokePath(hDC);
        else
            StrokeAndFillPath(hDC);
    }
    _gdifont_get_points(hDC, width, height, font->bord, pPoints);
    DeleteObject(hBmp);
}

static TCAX_PyPoints gdifont_get_points(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    gdifont_font *font;
    const wchar_t *text;
    gdifont_points points;
    int i;
    PyObject *temp;
    TCAX_PyPoints pyPoints;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "gfGetPoints error, too less parameters - `(gdi_font, text)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "gfGetPoints error, the 1st param should be an integer - `gdi_font'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyUnicode_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "gfGetPoints error, the 2nd param should be a unicode string - `text'\n");
        return NULL;
    }
    font = (gdifont_font *)PyLong_AsUnsignedLong(pyArg1);
    text = (const wchar_t *)PyUnicode_AsUnicode(pyArg2);
    ZeroMemory(&points, sizeof(gdifont_points));
    _gdifont_text_points(font, text, &points);
    pyPoints = PyTuple_New(points.count);
    for (i = 0; i < points.count; i++) {
        temp = PyTuple_New(3);
        PyTuple_SetItem(temp, 0, PyFloat_FromDouble(points.xBuf[i]));
        PyTuple_SetItem(temp, 1, PyFloat_FromDouble(points.yBuf[i]));
        PyTuple_SetItem(temp, 2, PyFloat_FromDouble(points.aBuf[i]));
        PyTuple_SetItem(pyPoints, i, temp);
    }
    _gdifont_points_clear(&points);
    return pyPoints;
}

/* Get Outline from text */
static void _gdifont_text_outline(gdifont_font *font, const wchar_t *text, double x, double y, gdifont_string *pString) {
    HDC hDC;
    int i, count, offsetX, offsetY, spacing;
    SIZE size;
    POINT *pts;
    BYTE *types;
    wchar_t buf[256];
    hDC = font->hDC;
    BeginPath(hDC);
    offsetX = (int)(x * SCALE_FACTOR + 0.5);
    offsetY = (int)(y * SCALE_FACTOR + 0.5);
    spacing = (int)(font->spacing * SCALE_FACTOR + 0.5);
    count = wcslen(text);
    for (i = 0; i < count; i++) {
        TextOutW(hDC, offsetX, offsetY, text + i, 1);
        GetTextExtentPoint32W(hDC, text + i, 1, &size);
        if (' ' == text[i] || '　' == text[i])
            size.cx = (long)(size.cx * font->spaceScale + 0.5);
        offsetX += size.cx + spacing;
    }
    EndPath(hDC);
    FlattenPath(hDC);
    count = GetPath(hDC, NULL, NULL, 0);
    pts = (POINT *)malloc(count * sizeof(POINT));
    types = (BYTE *)malloc(count * sizeof(BYTE));
    GetPath(hDC, pts, types, count);
    for (i = 0; i < count; i++) {
        if (PT_MOVETO == types[i]) {
            wsprintf(buf, L"m %d %d ", pts[i].x, pts[i].y);
            _gdifont_string_append(pString, buf, wcslen(buf));
        } else if (PT_LINETO == types[i]) {
            wsprintf(buf, L"l %d %d ", pts[i].x, pts[i].y);
            _gdifont_string_append(pString, buf, wcslen(buf));
        } else if ((PT_CLOSEFIGURE | PT_LINETO) == types[i]) {
            wsprintf(buf, L"l %d %d c ", pts[i].x, pts[i].y);
            _gdifont_string_append(pString, buf, wcslen(buf));
        }
    }
    free(pts);
    free(types);
}

static PyObject *gdifont_get_outline(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4;
    gdifont_font *font;
    const wchar_t *text;
    double x, y;
    gdifont_string str;
    PyObject *pyDrawing;
    if (PyTuple_GET_SIZE(args) < 4) {
        PyErr_SetString(PyExc_RuntimeError, "gfGetOutline error, too less parameters - `(gdi_font, text, x, y)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "gfGetOutline error, the 1st param should be an integer - `gdi_font'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyUnicode_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "gfGetOutline error, the 2nd param should be a unicode string - `text'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "gfGetOutline error, the 3rd param should be a float - `x'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "gfGetOutline error, the 4th param should be a float - `y'\n");
        return NULL;
    }
    font = (gdifont_font *)PyLong_AsUnsignedLong(pyArg1);
    text = (const wchar_t *)PyUnicode_AsUnicode(pyArg2);
    x = PyFloat_AsDouble(pyArg3);
    y = PyFloat_AsDouble(pyArg4);
    ZeroMemory(&str, sizeof(gdifont_string));
    _gdifont_text_outline(font, text, x, y, &str);
    pyDrawing = PyUnicode_FromUnicode((const Py_UNICODE *)str.buf, str.len);
    _gdifont_string_clear(&str);
    return pyDrawing;
}

/* Get Outline Points from text */
static void _linear_interpolation(double *pxt, double *pyt, double xa, double ya, double xb, double yb, double t) {
    *pxt = xa + (xb - xa) * t;
    *pyt = ya + (yb - ya) * t;
}

static void _outline_points_line_to(gdifont_points *points, double density, double lastX, double lastY, double toX, double toY) {
    double xs, ys, xe, ye, x, y, t, step;
    xs = lastX;
    ys = lastY;
    xe = toX;
    ye = toY;
    step = 1 / (__max(abs(xe - xs), abs(ye - ys)) * density);
    for (t = step; t < 1 + step; t += step) {
        _linear_interpolation(&x, &y, xs, ys, xe, ye, t);
        _gdifont_points_append(points, x, y, 255);
    }
}

static void _gdifont_text_outline_points(gdifont_font *font, const wchar_t *text, double density, double **pXBuf, double **pYBuf, double **pABuf, int *pCount) {
    HDC hDC;
    int i, count, offsetX, offsetY, spacing;
    SIZE size;
    POINT *pts;
    BYTE *types;
    gdifont_points points;
    double oriX, oriY, lastX, lastY;
    hDC = font->hDC;
    BeginPath(hDC);
    offsetX = 0;
    offsetY = 0;
    spacing = (int)(font->spacing * SCALE_FACTOR + 0.5);
    count = wcslen(text);
    for (i = 0; i < count; i++) {
        TextOutW(hDC, offsetX, offsetY, text + i, 1);
        GetTextExtentPoint32W(hDC, text + i, 1, &size);
        if (' ' == text[i] || '　' == text[i])
            size.cx = (long)(size.cx * font->spaceScale + 0.5);
        offsetX += size.cx + spacing;
    }
    EndPath(hDC);
    FlattenPath(hDC);
    count = GetPath(hDC, NULL, NULL, 0);
    pts = (POINT *)malloc(count * sizeof(POINT));
    types = (BYTE *)malloc(count * sizeof(BYTE));
    GetPath(hDC, pts, types, count);
    points.count = 0;
    points.capacity = 100;
    points.xBuf = (double *)malloc(points.capacity * sizeof(double));
    points.yBuf = (double *)malloc(points.capacity * sizeof(double));
    points.aBuf = (double *)malloc(points.capacity * sizeof(double));
    for (i = 0; i < count; i++) {
        if (PT_MOVETO == types[i]) {
            oriX = pts[i].x / (double)SCALE_FACTOR;
            oriY = pts[i].y / (double)SCALE_FACTOR;
            _gdifont_points_append(&points, oriX, oriY, 255);
            lastX = oriX;
            lastY = oriY;
        } else if (PT_LINETO == types[i]) {
            _outline_points_line_to(&points, density, lastX, lastY, pts[i].x / (double)SCALE_FACTOR, pts[i].y / (double)SCALE_FACTOR);
            lastX = pts[i].x / (double)SCALE_FACTOR;
            lastY = pts[i].y / (double)SCALE_FACTOR;
        } else if ((PT_CLOSEFIGURE | PT_LINETO) == types[i]) {
            _outline_points_line_to(&points, density, lastX, lastY, pts[i].x / (double)SCALE_FACTOR, pts[i].y / (double)SCALE_FACTOR);
            lastX = pts[i].x / (double)SCALE_FACTOR;
            lastY = pts[i].y / (double)SCALE_FACTOR;
            _outline_points_line_to(&points, density, lastX, lastY, oriX, oriY);
        }
    }
    free(pts);
    free(types);
    *pXBuf = points.xBuf;
    *pYBuf = points.yBuf;
    *pABuf = points.aBuf;
    *pCount = points.count;
}

static TCAX_PyPoints gdifont_get_outline_points(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3;
    gdifont_font *font;
    const wchar_t *text;
    double density;
    double *xBuf, *yBuf, *aBuf;
    int i, count;
    TCAX_PyPoints pyPoints;
    PyObject *pyPointTemp;
    if (PyTuple_GET_SIZE(args) < 3) {
        PyErr_SetString(PyExc_RuntimeError, "gfGetOutlinePoints error, too less parameters - `(gdi_font, text, density)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "gfGetOutlinePoints error, the 1st param should be an integer - `gdi_font'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyUnicode_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "gfGetOutlinePoints error, the 2nd param should be a unicode string - `text'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "gfGetOutlinePoints error, the 3rd param should be a float - `density'\n");
        return NULL;
    }
    font = (gdifont_font *)PyLong_AsUnsignedLong(pyArg1);
    text = (const wchar_t *)PyUnicode_AsUnicode(pyArg2);
    density = PyFloat_AsDouble(pyArg3);
    _gdifont_text_outline_points(font, text, density, &xBuf, &yBuf, &aBuf, &count);
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
}

/* Get Text extent */
static void _gdifont_text_extent(gdifont_font *font, const wchar_t *text, SIZE *ps) {
    SIZE size;
    int i, count, offset, spacing;
    offset = 0;
    spacing = (int)(font->spacing * SCALE_FACTOR + 0.5);
    count = wcslen(text);
    for (i = 0; i < count; i++) {
        GetTextExtentPoint32W(font->hDC, text + i, 1, &size);
        if (' ' == text[i] || '　' == text[i])
            size.cx = (long)(size.cx * font->spaceScale + 0.5);
        offset += size.cx + spacing;
    }
    ps->cx = offset - spacing;
}

static PyObject *gdifont_get_extent(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    gdifont_font *font;
    const wchar_t *text;
    SIZE size;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "gfGetExtent error, too less parameters - `(gdi_font, text)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyLong_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "gfGetExtent error, the 1st param should be an integer - `gdi_font'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyUnicode_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "gfGetExtent error, the 2nd param should be a unicode string - `text'\n");
        return NULL;
    }
    font = (gdifont_font *)PyLong_AsUnsignedLong(pyArg1);
    text = (const wchar_t *)PyUnicode_AsUnicode(pyArg2);
    _gdifont_text_extent(font, text, &size);
    return PyFloat_FromDouble(size.cx / (double)SCALE_FACTOR);
}

static PyMethodDef gdiFont_Methods[] = {
    { "gfInitFont"             ,  gdifont_init_font                        ,  METH_VARARGS ,  "gfInitFont(font_face_name, font_size, spacing, space_scale, color, bord, is_outline)" }, 
    { "gfFinFont"              ,  gdifont_fin_font                         ,  METH_VARARGS ,  "gfFinFont(gdi_font)" }, 
    { "gfGetPoints"            ,  gdifont_get_points                       ,  METH_VARARGS ,  "gfGetPoints(gdi_font, text)" }, 
    { "gfGetOutline"           ,  gdifont_get_outline                      ,  METH_VARARGS ,  "gfGetOutline(gdi_font, text, x, y)" }, 
    { "gfGetOutlinePoints"     ,  gdifont_get_outline_points               ,  METH_VARARGS ,  "gfGetOutlinePoints(gdi_font, text, density)" },
    { "gfGetExtent"            ,  gdifont_get_extent                       ,  METH_VARARGS ,  "gfGetExtent(gdi_font, text)" }, 
    { NULL                     ,  NULL                                     ,  0            ,  NULL }
};

static struct PyModuleDef gdiFont_Module = {
    PyModuleDef_HEAD_INIT, 
    "gdiFont", 
    "Module gdiFont is a component of tcax - developed by milkyjing", 
    -1, 
    gdiFont_Methods
};

PyMODINIT_FUNC PyInit_gdiFont() {
    return PyModule_Create(&gdiFont_Module);
}

