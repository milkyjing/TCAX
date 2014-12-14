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

#include "file.h"


static void _tcaxlib_sz_unicode_to_utf8(const wchar_t *uni, char **utf8, int *pSize) {
    int size;
    char *sz;
    size = WideCharToMultiByte(CP_UTF8, 0, uni, -1, NULL, 0, NULL, NULL);
    sz = (char *)malloc(size * sizeof(char));
    WideCharToMultiByte(CP_UTF8, 0, uni, -1, sz, size, NULL, NULL);
    *utf8 = sz;
    *pSize = size - 1;
}

static void _tcaxlib_sz_unicode_to_ansi(const wchar_t *uni, char **ansi) {
    int size;
    char *sz;
    size = WideCharToMultiByte(CP_ACP, 0, uni, -1, NULL, 0, NULL, NULL);
    sz = (char *)malloc(size * sizeof(char));
    WideCharToMultiByte(CP_ACP, 0, uni, -1, sz, size, NULL, NULL);
    *ansi = sz;
}

TCAX_PyAssFile tcaxlib_create_ass_file(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    unsigned char BOM[3];
    const wchar_t *filename;
    const wchar_t *header;
    int size;
    char *utf8;
    TCAX_pAssFile pAssFile;
    TCAX_PyAssFile pyAssFile;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "CreateAssFile error, too less parameters - `(ass_file, ass_header)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyUnicode_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "CreateAssFile error, the 1st param should be a unicode string - `ass_file'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyUnicode_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "CreateAssFile error, the 2nd param should be a unicode string - `ass_header'\n");
        return NULL;
    }
    filename = (const wchar_t *)PyUnicode_AsUnicode(pyArg1);
    header = (const wchar_t *)PyUnicode_AsUnicode(pyArg2);
    pAssFile = (TCAX_pAssFile)malloc(sizeof(TCAX_AssFile));
    pAssFile->fp = _wfopen(filename, L"wb");
    if (!pAssFile->fp) {
        free(pAssFile);
        PyErr_SetString(PyExc_RuntimeError, "CreateAssFile error, failed to create the file!\n");
        return NULL;
    }
    BOM[0] = 0xEF;
    BOM[1] = 0xBB;
    BOM[2] = 0xBF;
    fwrite(BOM, sizeof(unsigned char), 3, pAssFile->fp);
    _tcaxlib_sz_unicode_to_utf8(header, &utf8, &size);
    fwrite(utf8, sizeof(char), size, pAssFile->fp);
    free(utf8);
    pyAssFile = PyTuple_New(1);
    PyTuple_SET_ITEM(pyAssFile, 0, PyLong_FromUnsignedLong((unsigned long)pAssFile));
    return pyAssFile;
}

int _tcaxlib_check_if_file_exists(const wchar_t *filename) {
    FILE *fp;
    fp = _wfopen(filename, L"rb");
    if (fp) {
        fclose(fp);
        return 1;
    }
    return 0;
}

TCAX_PyAssFile tcaxlib_append_ass_file(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    const wchar_t *filename;
    TCAX_pAssFile pAssFile;
    TCAX_PyAssFile pyAssFile;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "AppendAssFile error, too less parameters - `(ass_file)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyUnicode_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "AppendAssFile error, the 1st param should be a unicode string - `ass_file'\n");
        return NULL;
    }
    filename = (const wchar_t *)PyUnicode_AsUnicode(pyArg1);
    if (!_tcaxlib_check_if_file_exists(filename)) {
        PyErr_SetString(PyExc_RuntimeError, "AppendAssFile error, failed to open the file!\n");
        return NULL;
    }
    pAssFile = (TCAX_pAssFile)malloc(sizeof(TCAX_AssFile));
    pAssFile->fp = _wfopen(filename, L"ab");
    if (!pAssFile->fp) {
        free(pAssFile);
        PyErr_SetString(PyExc_RuntimeError, "AppendAssFile error, failed to open the file!\n");
        return NULL;
    }
    pyAssFile = PyTuple_New(1);
    PyTuple_SET_ITEM(pyAssFile, 0, PyLong_FromUnsignedLong((unsigned long)pAssFile));
    return pyAssFile;
}

static void _tcaxlib_convert_ass_list_to_string(PyObject *assList, wchar_t **pAssString, int *pCount) {
    int i, count, size, offset;
    wchar_t *assString;
    wchar_t **pAssLine;
    int     *assLineSize;
    count = PyList_GET_SIZE(assList);
    assLineSize = (int *)malloc(count * sizeof(int));
    pAssLine = (wchar_t **)malloc(count * sizeof(wchar_t *));
    size = 0;
    for (i = 0; i < count; i++) {
        pAssLine[i] = (wchar_t *)PyUnicode_AsUnicode(PyList_GET_ITEM(assList, i));
        assLineSize[i] = wcslen(pAssLine[i]);
        size += assLineSize[i];
    }
    assString = (wchar_t *)malloc((size + 1) * sizeof(wchar_t));
    offset = 0;
    for (i = 0; i < count; i++) {
        memcpy(assString + offset, pAssLine[i], assLineSize[i] * sizeof(wchar_t));
        offset += assLineSize[i];
    }
    assString[size] = L'\0';
    free(pAssLine);
    free(assLineSize);
    *pAssString = assString;
    *pCount = size;
}

TCAX_Py_Error_Code tcaxlib_write_ass_file(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    TCAX_pAssFile pAssFile;
    TCAX_PyAssFile pyAssFile;
    PyObject *assList;
    wchar_t *assString;
    char *utf8;
    int size;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "WriteAssFile error, too less parameters - `(pyAssFile, ASS_BUF)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "WriteAssFile error, the 1st param should be a tuple - `pyAssFile'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyList_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "WriteAssFile error, the 2nd param should be a list - `ASS_BUF'\n");
        return NULL;
    }
    pyAssFile = (TCAX_PyAssFile)pyArg1;
    assList = pyArg2;
    pAssFile = (TCAX_pAssFile)PyLong_AsUnsignedLong(PyTuple_GET_ITEM(pyAssFile, 0));
    _tcaxlib_convert_ass_list_to_string(assList, &assString, &size);
    _tcaxlib_sz_unicode_to_utf8(assString, &utf8, &size);
    free(assString);
    fwrite(utf8, sizeof(char), size, pAssFile->fp);
    free(utf8);
    return PyLong_FromLong(0);
}

TCAX_Py_Error_Code tcaxlib_fin_ass_file(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    TCAX_pAssFile pAssFile;
    TCAX_PyAssFile pyAssFile;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "FinAssFile error, too less parameters - `(pyAssFile)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "FinAssFile error, the 1st param should be a tuple - `pyAssFile'\n");
        return NULL;
    }
    pyAssFile = (TCAX_PyAssFile)pyArg1;
    pAssFile = (TCAX_pAssFile)PyLong_AsUnsignedLong(PyTuple_GET_ITEM(pyAssFile, 0));
    fclose(pAssFile->fp);
    free(pAssFile);
    Py_CLEAR(pyAssFile);
    return PyLong_FromLong(0);
}

static void _tcaxlib_get_numerator_and_denominator(double fps, unsigned long *pNumerator, unsigned long *pDenominator) {
    unsigned long numerator, denominator, x, y, t;
    denominator = 100000;
    numerator   = (unsigned long)(fps * denominator);
    x = numerator;
    y = denominator;
    while (y) {   // find gcd
        t = x % y;
        x = y;
        y = t;
    }
    numerator /= x;
    denominator /= x;
    *pNumerator = numerator;
    *pDenominator = denominator;
}

TCAX_PyTcasFile tcaxlib_create_tcas_file(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2, *pyArg3, *pyArg4;
    const wchar_t *filename;
    char *ansiFilename;
    int fx_width;
    int fx_height;
    double fx_fps;
    TCAX_pTcasFile pTcasFile;
    TCAX_PyTcasFile pyTcasFile;
    if (PyTuple_GET_SIZE(args) < 4) {
        PyErr_SetString(PyExc_RuntimeError, "CreateTcasFile error, too less parameters - `(tcas_file, fx_width, fx_height, fx_fps)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyUnicode_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "CreateTcasFile error, the 1st param should be a unicode string - `tcas_file'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyLong_Check(pyArg2) && !PyFloat_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "CreateTcasFile error, the 2nd param should be an integer - `fx_width'\n");
        return NULL;
    }
    pyArg3 = PyTuple_GET_ITEM(args, 2);
    if (!PyLong_Check(pyArg3) && !PyFloat_Check(pyArg3)) {
        PyErr_SetString(PyExc_RuntimeError, "CreateTcasFile error, the 3rd param should be an integer - `fx_height'\n");
        return NULL;
    }
    pyArg4 = PyTuple_GET_ITEM(args, 3);
    if (!PyLong_Check(pyArg4) && !PyFloat_Check(pyArg4)) {
        PyErr_SetString(PyExc_RuntimeError, "CreateTcasFile error, the 4th param should be a float - `fx_fps'\n");
        return NULL;
    }
    filename = (const wchar_t *)PyUnicode_AsUnicode(pyArg1);
    fx_width = (int)PyLong_AsLong(pyArg2);
    fx_height = (int)PyLong_AsLong(pyArg3);
    fx_fps = PyFloat_AsDouble(pyArg4);
    pTcasFile = (TCAX_pTcasFile)malloc(sizeof(TCAX_TcasFile));
    _tcaxlib_sz_unicode_to_ansi(filename, &ansiFilename);
    if (libtcas_open_file(&pTcasFile->file, ansiFilename, tcas_file_create_new) != tcas_error_success) {
        free(ansiFilename);
        free(pTcasFile);
        PyErr_SetString(PyExc_RuntimeError, "CreateTcasFile error, failed to create the file!\n");
        return NULL;
    }
    free(ansiFilename);
    _tcaxlib_get_numerator_and_denominator(fx_fps, &pTcasFile->header.fpsNumerator, &pTcasFile->header.fpsDenominator);
    //libtcas_set_header(&pTcasFile->header, TCAS_FILE_TYPE_COMPRESSED, TCAS_KEY_FRAMING_ENABLED, fx_width, fx_height, TCAS_INIT_MIN_TIME, TCAS_INIT_MAX_TIME, 0, pTcasFile->header.fpsNumerator, pTcasFile->header.fpsDenominator);
    libtcas_set_header(&pTcasFile->header, TCAS_FILE_TYPE_COMPRESSED_Z, TCAS_KEY_FRAMING_ENABLED, fx_width, fx_height, TCAS_INIT_MIN_TIME, TCAS_INIT_MAX_TIME, 0, pTcasFile->header.fpsNumerator, pTcasFile->header.fpsDenominator);
    //libtcas_set_file_position_indicator(&pTcasFile->file, tcas_fpi_header);    // use libtcas_write_header instead, in order to make use of the incompleted TCAS file
    libtcas_write_header(&pTcasFile->file, &pTcasFile->header, 0);
    pyTcasFile = PyTuple_New(1);
    PyTuple_SET_ITEM(pyTcasFile, 0, PyLong_FromUnsignedLong((unsigned long)pTcasFile));
    return pyTcasFile;
}

TCAX_Py_Error_Code tcaxlib_write_tcas_file(PyObject *self, PyObject *args) {
    PyObject *pyArg1, *pyArg2;
    TCAX_pTcasFile pTcasFile;
    TCAX_PyTcasFile pyTcasFile;
    PyObject *tcasList;
    tcas_unit *tcasBuf;
    tcas_unit *compTcasBuf;
    TCAS_pChunk pChunks;
    tcas_u32 count, chunks, units;
    if (PyTuple_GET_SIZE(args) < 2) {
        PyErr_SetString(PyExc_RuntimeError, "WriteTcasFile error, too less parameters - `(pyTcasFile, TCAS_BUF)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "WriteTcasFile error, the 1st param should be a tuple - `pyTcasFile'\n");
        return NULL;
    }
    pyArg2 = PyTuple_GET_ITEM(args, 1);
    if (!PyList_Check(pyArg2)) {
        PyErr_SetString(PyExc_RuntimeError, "WriteTcasFile error, the 2nd param should be a list - `TCAS_BUF'\n");
        return NULL;
    }
    if (PyList_GET_SIZE(pyArg2) == 0) {
        PyErr_SetString(PyExc_RuntimeWarning, "WriteTcasFile warning, empty list `TCAS_BUF'\n");
        return PyLong_FromLong(-1);
    }
    pyTcasFile = (TCAX_PyTcasFile)pyArg1;
    tcasList = pyArg2;
    pTcasFile = (TCAX_pTcasFile)PyLong_AsUnsignedLong(PyTuple_GET_ITEM(pyTcasFile, 0));
    tcaxlib_convert_tcas_list_to_buf(tcasList, &tcasBuf, &count);
    libtcas_get_raw_chunks_min_max_time((const TCAS_pRawChunk)tcasBuf, count, &pTcasFile->header);
    libtcas_compress_raw_chunks((const TCAS_pRawChunk)tcasBuf, count, TCAS_FALSE, &compTcasBuf, &chunks, &units);
    free(tcasBuf);
    pTcasFile->header.chunks += chunks;
    /*libtcas_write(&pTcasFile->file, compTcasBuf, units);
    free(compTcasBuf);*/
    libtcas_compress_chunks_z(compTcasBuf, chunks, &pChunks);
    free(compTcasBuf);
    libtcas_write_chunks_z(&pTcasFile->file, pChunks, chunks, TCAS_TRUE);
    free(pChunks);
    return PyLong_FromLong(0);
}

TCAX_Py_Error_Code tcaxlib_fin_tcas_file(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    TCAX_pTcasFile pTcasFile;
    TCAX_PyTcasFile pyTcasFile;
    if (PyTuple_GET_SIZE(args) < 1) {
        PyErr_SetString(PyExc_RuntimeError, "FinTcasFile error, too less parameters - `(pyTcasFile)'\n");
        return NULL;
    }
    pyArg1 = PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(pyArg1)) {
        PyErr_SetString(PyExc_RuntimeError, "FinTcasFile error, the 1st param should be a tuple - `pyTcasFile'\n");
        return NULL;
    }
    pyTcasFile = (TCAX_PyTcasFile)pyArg1;
    pTcasFile = (TCAX_pTcasFile)PyLong_AsUnsignedLong(PyTuple_GET_ITEM(pyTcasFile, 0));
    libtcas_write_header(&pTcasFile->file, &pTcasFile->header, 0);
    libtcas_close_file(&pTcasFile->file);
    free(pTcasFile);
    Py_CLEAR(pyTcasFile);
    return PyLong_FromLong(0);
}

