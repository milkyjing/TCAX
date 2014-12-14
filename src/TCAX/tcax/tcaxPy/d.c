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

#include "py.h"

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif    /* _MSC_VER */

/* Convert between unicode big endian and unicode little endian */
static void _tcxpy_convert_endian(wchar_t *buffer, int count) {
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

static PY_Error_Code _tcxpy_read_file_to_unicode(const wchar_t *filename, wchar_t **pBuffer, int *pCount) {
    FILE *fp;
    int size, count;
    unsigned char *rawBuffer;
    wchar_t *buffer;
    fp = _wfopen(filename, L"rb");
    if (!fp) return py_error_file_cant_open;
    size = _filelength(fp->_file);
    rawBuffer = (unsigned char *)malloc(size + 2);    /* note that `+2' is intending to add a null terminator */
    rawBuffer[size] = 0;
    rawBuffer[size + 1] = 0;
    if (fread(rawBuffer, sizeof(unsigned char), size, fp) != size) {
        free(rawBuffer);
        fclose(fp);
        return py_error_file_while_reading;
    }
    fclose(fp);
    if (size >= 2 && 0xFF == rawBuffer[0] && 0xFE == rawBuffer[1]) {    /* unicode little endian */
        *pBuffer = (wchar_t *)(rawBuffer + 2);    /* `+2' is intending to remove the BOM-header */
        *pCount = (size - 2) / 2;        /* reduce count for the BOM-header */
    } else if (size >= 2 && 0xFE == rawBuffer[0] && 0xFF == rawBuffer[1]) {    /* unicode big endian */
        _tcxpy_convert_endian((wchar_t *)rawBuffer, size / 2);
        *pBuffer = (wchar_t *)(rawBuffer + 2);    /* `+2' is intending to remove the BOM-header */
        *pCount = (size - 2) / 2;        /* reduce count for the BOM-header */
    } else if (size >= 3 && 0xEF == rawBuffer[0] && 0xBB == rawBuffer[1] && 0xBF == rawBuffer[2]) {    /* utf-8 */
        count = MultiByteToWideChar(CP_UTF8, 0, (const char *)(rawBuffer + 3), -1, NULL, 0);    /* `+3' is intending to remove the BOM-header */
        buffer = (wchar_t *)malloc(count * sizeof(wchar_t));
        MultiByteToWideChar(CP_UTF8, 0, (const char *)(rawBuffer + 3), -1, buffer, count);    /* `+3' is intending to remove the BOM-header */
        free(rawBuffer);
        *pBuffer = buffer;
        *pCount = count - 1;    /* reduce count for the null terminator */
    } else {    /* ansi */
        count = MultiByteToWideChar(CP_ACP, 0, (const char *)rawBuffer, -1, NULL, 0);
        buffer = (wchar_t *)malloc(count * sizeof(wchar_t));
        MultiByteToWideChar(CP_ACP, 0, (const char *)rawBuffer, -1, buffer, count);
        free(rawBuffer);
        *pBuffer = buffer;
        *pCount = count - 1;    /* reduce count for the null terminator */
    }
    return py_error_success;
}

PY_Error_Code tcxpy_convert_file_to_utf8(const wchar_t *filename) {
    FILE *fp;
    unsigned char sig[3];
    if (!filename) return py_error_null_pointer;
    fp = _wfopen(filename, L"rb");
    if (!fp) return py_error_file_cant_open;
    if (fread(sig, sizeof(unsigned char), 3, fp) != 3) {
        fclose(fp);
        return py_error_file_while_reading;
    }
    fclose(fp);
    if (!(0xEF == sig[0] && 0xBB == sig[1] && 0xBF == sig[2])) {
        int count, size;
        wchar_t *uni;
        char *buffer;
        if (_tcxpy_read_file_to_unicode(filename, &uni, &count) == py_error_success) {
            size = WideCharToMultiByte(CP_UTF8, 0, uni, count, NULL, 0, NULL, NULL);
            buffer = (char *)malloc(size * sizeof(char));
            WideCharToMultiByte(CP_UTF8, 0, uni, count, buffer, size, NULL, NULL);
            free(uni);
        }
        fp = _wfopen(filename, L"wb");
        if (!fp) {
            free(buffer);
            return py_error_file_cant_open;
        }
        sig[0] = 0xEF;
        sig[1] = 0xBB;
        sig[2] = 0xBF;
        if (fwrite(sig, sizeof(unsigned char), 3, fp) != 3) {
            free(buffer);
            fclose(fp);
            return py_error_file_while_writing;
        }
        if (fwrite(buffer, sizeof(char), size, fp) != size) {
            free(buffer);
            fclose(fp);
            return py_error_file_while_writing;
        }
        free(buffer);
        fclose(fp);
    }
    return py_error_success;
}

PY_Error_Code tcxpy_init_python() {
    Py_Initialize();
    if (Py_IsInitialized() == 0) return py_error_init_fail;
    return py_error_success;
}

PY_Error_Code tcxpy_fin_python() {
    Py_Finalize();
    if (Py_IsInitialized() != 0) return py_error_fin_fail;
    return py_error_success;
}

static void _tcxpy_sz_unicode_to_ansi(const wchar_t *uni, char **ansi) {
    int size;
    char *sz;
    size = WideCharToMultiByte(CP_ACP, 0, uni, -1, NULL, 0, NULL, NULL);
    sz = (char *)malloc(size * sizeof(char));
    WideCharToMultiByte(CP_ACP, 0, uni, -1, sz, size, NULL, NULL);
    *ansi = sz;
}

PY_Error_Code tcxpy_exec_py_script(const wchar_t *filename) {
    int len, size;
    char *buf;
    char *ansiFilename;
    _tcxpy_sz_unicode_to_ansi(filename, &ansiFilename);
    len = strlen(ansiFilename);
    size = 11 + len + 11;
    buf = (char *)malloc(size * sizeof(char));
    memcpy(buf, "exec(open('", 11 * sizeof(char));
    memcpy(buf + 11, ansiFilename, len * sizeof(char));
    memcpy(buf + 11 + len, "').read())\0", 11 * sizeof(char));
    free(ansiFilename);
    PyRun_SimpleString(buf);
    free(buf);
    return py_error_success;
}

/*
PY_Error_Code tcxpy_exec_py_script(const wchar_t *filename) {
    FILE *fp;
    char *ansiFilename;
    if (!filename) return py_error_null_pointer;
    fp = _wfopen(filename, L"r");
    if (!fp) return py_error_file_cant_open;
    _tcxpy_sz_unicode_to_ansi(filename, &ansiFilename);
    if (!PyRun_SimpleFile(fp, ansiFilename)) {
        free(ansiFilename);
        fclose(fp);
        return py_error_file_invalid_script;
    }
    free(ansiFilename);
    fclose(fp);
    return py_error_success;
}

PY_Error_Code tcxpy_exec_py_script(const wchar_t *filename) {
    PY_Error_Code error;
    wchar_t *uni;
    char *ansi;
    int count;
    error = _tcxpy_read_file_to_unicode(filename, &uni, &count);
    if (py_error_success != error) return error;
    _tcxpy_sz_unicode_to_ansi(uni, &ansi);
    free(uni);
    if (!PyRun_SimpleString(ansi)) {
        free(ansi);
        return py_error_exec_fail;
    }
    free(ansi);
    return py_error_success;
}*/
/* tcx py token file functions */

const wchar_t *tcxpy_make_py_token_filename(const wchar_t *filename, wchar_t **pTokenFilename) {
    int count;
    wchar_t *tokenFilename;
    if (!filename) return NULL;
    count = wcslen(filename);
    tokenFilename = (wchar_t *)malloc((count + 7) * sizeof(wchar_t));
    memcpy(tokenFilename, filename, count * sizeof(wchar_t));
    memcpy(tokenFilename + count, L".token\0", 7 * sizeof(wchar_t));
    *pTokenFilename = tokenFilename;
    return (const wchar_t *)tokenFilename;
}

PY_Error_Code tcxpy_create_py_token(const wchar_t *filename) {
    FILE *fp;
    if (!filename) return py_error_null_pointer;
    fp = _wfopen(filename, L"w");
    if (!fp) py_error_file_cant_create;
    fclose(fp);
    return py_error_success;
}

PY_Error_Code tcxpy_synchronize_py_token(const wchar_t *filename, const wchar_t *tokenFilename) {
    HANDLE   hPyFile;
    HANDLE   hTokenFile;
    FILETIME pyFileTime;
    if (!filename) return py_error_null_pointer;
    if (!tokenFilename || GetFileAttributesW(tokenFilename) == INVALID_FILE_ATTRIBUTES) {
        wchar_t *buffer;
        tcxpy_make_py_token_filename(filename, &buffer);
        if (tcxpy_create_py_token(buffer) != py_error_success) {
            free(buffer);
            return py_error_file_cant_create;
        }
        hTokenFile = CreateFileW(buffer, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        free(buffer);
    } else hTokenFile = CreateFileW(tokenFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hTokenFile) return py_error_file_cant_open;
    hPyFile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hPyFile) {
        CloseHandle(hTokenFile);
        return py_error_file_cant_open;
    }
    GetFileTime(hPyFile, NULL, NULL, &pyFileTime);
    SetFileTime(hTokenFile, NULL, NULL, &pyFileTime);
    CloseHandle(hPyFile);
    CloseHandle(hTokenFile);
    return py_error_success;
}

PY_Error_Code tcxpy_is_py_modified(const wchar_t *filename, const wchar_t *tokenFilename) {
    HANDLE   hPyFile;
    HANDLE   hTokenFile;
    FILETIME pyFileTime;
    FILETIME tokenFileTime;
    if (!filename) return py_error_null_pointer;
    if (GetFileAttributesW(filename) == INVALID_FILE_ATTRIBUTES) return py_error_file_not_found;     /* if the py script file dosen't exists then it means there is only a pyc file which we can't modify */
    if (!tokenFilename || GetFileAttributesW(tokenFilename) == INVALID_FILE_ATTRIBUTES) {
        wchar_t *buffer;
        tcxpy_make_py_token_filename(filename, &buffer);
        if (tcxpy_create_py_token(buffer) != py_error_success) {
            free(buffer);
            return py_error_file_cant_create;
        }
        tcxpy_synchronize_py_token(filename, buffer);
        free(buffer);
        return py_error_file_modified;
    }
    hPyFile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hPyFile) return py_error_file_cant_open;
    hTokenFile = CreateFileW(tokenFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hTokenFile) {
        CloseHandle(hPyFile);
        return py_error_file_cant_open;
    }
    GetFileTime(hPyFile, NULL, NULL, &pyFileTime);
    GetFileTime(hTokenFile, NULL, NULL, &tokenFileTime);
    if (pyFileTime.dwHighDateTime != tokenFileTime.dwHighDateTime || pyFileTime.dwLowDateTime != tokenFileTime.dwLowDateTime) {
        SetFileTime(hTokenFile, NULL, NULL, &pyFileTime);
        CloseHandle(hPyFile);
        CloseHandle(hTokenFile);
        return py_error_file_modified;
    }
    CloseHandle(hPyFile);
    CloseHandle(hTokenFile);
    return py_error_success;
}

static PY_Error_Code _tcxpy_write_unicode_to_utf8_file(const wchar_t *filename, const wchar_t *uni, int count) {
    FILE *fp;
    int size;
    char *utf8String;
    unsigned char BOM[3];
    fp = _wfopen(filename, L"wb");
    if (!fp) return py_error_file_cant_create;
    BOM[0] = 0xEF;
    BOM[1] = 0xBB;
    BOM[2] = 0xBF;
    if (fwrite(BOM, sizeof(unsigned char), 3, fp) != 3) return py_error_file_while_writing;
    size = WideCharToMultiByte(CP_UTF8, 0, uni, count, NULL, 0, NULL, NULL);
    utf8String = (char *)malloc(size);
    WideCharToMultiByte(CP_UTF8, 0, uni, count, utf8String, size, NULL, NULL);
    if (fwrite(utf8String, sizeof(char), size, fp) != size) {
        free(utf8String);
        return py_error_file_while_writing;
    }
    free(utf8String);
    return py_error_success;
}

PY_Error_Code tcxpy_create_py_template(const wchar_t *filename) {
    const wchar_t *py_file_buf = L"from tcxPy import *\r\n\
\r\n\
\r\n\
def tcxPy_Init():\r\n\
    print('Tips: you can initialize your global variables here.')\r\n\
    print('This function will be executed once if you set `< tcxpy init = true >\'')\r\n\
    print(tcxGetValueIDsInfo())\r\n\
\r\n\
\r\n\
def tcxPy_User():\r\n\
    print('User defined function.')\r\n\
    print('This function will be executed once if you set `< tcxpy user = true >\'')\r\n\
    print('Otherwise, tcxPy_Main will be executed (many times)')\r\n\
\r\n\
\r\n\
def tcxPy_Fin():\r\n\
    print('Tips: you can finalize your global variables here.')\r\n\
    print('This function will be executed once if you set `< tcxpy fin = true >\'')\r\n\
    print('Note: you do not need to finalize the global variables got from function GetVal()')\r\n\
\r\n\
\r\n\
def tcxPy_Main(_I, _J, _N, _BT, _ET, _SK, _KT, _X, _Y, _A, _TXT):\r\n\
\r\n\
    ASS_BUF = []        # used for saving ass FX lines\r\n\
    TCS_BUF = []        # used for saving tcs FX raw data\r\n\
\r\n\
    #############################\r\n\
    # TODO: write your codes here #\r\n\
\r\n\
    ass_main(ASS_BUF, SubL(_BT, _ET), pos(_X, _Y) + K(_SK) + K(_KT), _TXT)\r\n\
\r\n\
    #############################\r\n\
\r\n\
    return (ASS_BUF, TCS_BUF)\r\n\
\r\n\
\r\n\
\r\n\
\r\n\
\r\n";
    return _tcxpy_write_unicode_to_utf8_file(filename, py_file_buf, wcslen(py_file_buf));
}

/* main functions */

static void _tcxpy_make_base_py_module_path(const wchar_t *directory, wchar_t **pPyModulePath) {
    int count;
    wchar_t *pyModulePath;
    count = wcslen(directory);
    pyModulePath = (wchar_t *)malloc((count + 1 + count + 5) * sizeof(wchar_t));
    memcpy(pyModulePath, directory, count * sizeof(wchar_t));
    pyModulePath[count] = L';';
    memcpy(pyModulePath + count + 1, directory, count * sizeof(wchar_t));
    memcpy(pyModulePath + count + 1 + count, L"\\Lib\0", 5 * sizeof(wchar_t));
    *pPyModulePath = pyModulePath;
}

PY_Error_Code tcxpy_init_base_py_module(PY_pTcxPy pTcxPy, const PY_pInitData pInitData) {
    int i, j, count;
    wchar_t *pyModulePath;
    PyObject *pyArgs;
    PyObject *pyData;
    PyObject *pyDataFunc;
    PyObject *pyTemp;
    /* syl info */
    PyObject *py_line1D;
    PyObject *py_begTime1D;
    PyObject *py_endTime1D;
    PyObject *py_count1D;
    PyObject *py_time2D;
    PyObject *py_timeDiff2D;
    PyObject *py_text2D;
    /* tm info */
    PyObject *py_width2D;
    PyObject *py_height2D;
    PyObject *py_kerning2D;
    PyObject *py_advance2D;
    PyObject *py_advanceDiff2D;
    PyObject *py_length1D;
    PyObject *py_initX2D;
    PyObject *py_initY2D;
    PyObject *py_bearingY2D;
    /***/
    if (!pTcxPy || !pInitData) return py_error_null_pointer;
    _tcxpy_make_base_py_module_path(pInitData->directory, &pyModulePath);
    PySys_SetPath(pyModulePath);  /* set the module's directory */
    free(pyModulePath);
    pTcxPy->pyBaseModule = PyImport_ImportModuleNoBlock("tcxPy");
    if (!pTcxPy->pyBaseModule) {
        PyErr_Print();
        /* PyErr_Clear(); */
        return py_error_init_fail;
    }
    pyArgs = PyTuple_New(43);
    PyTuple_SET_ITEM(pyArgs,  0, PyUnicode_FromUnicode(pInitData->assHeader, wcslen(pInitData->assHeader)));
    PyTuple_SET_ITEM(pyArgs,  1, PyUnicode_FromUnicode(pInitData->outFilename, wcslen(pInitData->outFilename)));
    /* tcc info */
    PyTuple_SET_ITEM(pyArgs,  2, PyUnicode_FromUnicode(pInitData->tccData.font_file, wcslen(pInitData->tccData.font_file)));
    PyTuple_SET_ITEM(pyArgs,  3, PyLong_FromLong(pInitData->tccData.font_face_id));
    PyTuple_SET_ITEM(pyArgs,  4, PyLong_FromLong(pInitData->tccData.font_size));
    PyTuple_SET_ITEM(pyArgs,  5, PyLong_FromLong(pInitData->tccData.fx_width));
    PyTuple_SET_ITEM(pyArgs,  6, PyLong_FromLong(pInitData->tccData.fx_height));
    PyTuple_SET_ITEM(pyArgs,  7, PyFloat_FromDouble(pInitData->tccData.fx_fps));
    PyTuple_SET_ITEM(pyArgs,  8, PyLong_FromLong(pInitData->tccData.alignment));
    PyTuple_SET_ITEM(pyArgs,  9, PyLong_FromLong(pInitData->tccData.x_offset));
    PyTuple_SET_ITEM(pyArgs, 10, PyLong_FromLong(pInitData->tccData.y_offset));
    PyTuple_SET_ITEM(pyArgs, 11, PyLong_FromLong(pInitData->tccData.spacing));
    PyTuple_SET_ITEM(pyArgs, 12, PyFloat_FromDouble(pInitData->tccData.space_scale));
    PyTuple_SET_ITEM(pyArgs, 13, PyLong_FromLong(pInitData->tccData.bord));
    PyTuple_SET_ITEM(pyArgs, 14, PyLong_FromLong(pInitData->tccData.shad));
    PyTuple_SET_ITEM(pyArgs, 15, PyUnicode_FromUnicode(pInitData->tccData.primary_color, wcslen(pInitData->tccData.primary_color)));
    PyTuple_SET_ITEM(pyArgs, 16, PyUnicode_FromUnicode(pInitData->tccData.secondary_color, wcslen(pInitData->tccData.secondary_color)));
    PyTuple_SET_ITEM(pyArgs, 17, PyUnicode_FromUnicode(pInitData->tccData.outline_color, wcslen(pInitData->tccData.outline_color)));
    PyTuple_SET_ITEM(pyArgs, 18, PyUnicode_FromUnicode(pInitData->tccData.back_color, wcslen(pInitData->tccData.back_color)));
    PyTuple_SET_ITEM(pyArgs, 19, PyLong_FromLong(pInitData->tccData.primary_alpha));
    PyTuple_SET_ITEM(pyArgs, 20, PyLong_FromLong(pInitData->tccData.secondary_alpha));
    PyTuple_SET_ITEM(pyArgs, 21, PyLong_FromLong(pInitData->tccData.outline_alpha));
    PyTuple_SET_ITEM(pyArgs, 22, PyLong_FromLong(pInitData->tccData.back_alpha));
    PyTuple_SET_ITEM(pyArgs, 23, PyFloat_FromDouble(pInitData->tccData.blur));
    /* syl info */
    PyTuple_SET_ITEM(pyArgs, 24, PyLong_FromLong(pInitData->sylData.lines));
    py_line1D = PyTuple_New(pInitData->sylData.lines);
    for (i = 0; i < pInitData->sylData.lines; i ++)
        PyTuple_SET_ITEM(py_line1D, i, PyUnicode_FromUnicode(pInitData->sylData.line1D[i], wcslen(pInitData->sylData.line1D[i])));
    PyTuple_SET_ITEM(pyArgs, 25, py_line1D);
    py_begTime1D = PyTuple_New(pInitData->sylData.lines);
    for (i = 0; i < pInitData->sylData.lines; i ++)
        PyTuple_SET_ITEM(py_begTime1D, i, PyLong_FromLong(pInitData->sylData.begTime1D[i]));
    PyTuple_SET_ITEM(pyArgs, 26, py_begTime1D);
    py_endTime1D = PyTuple_New(pInitData->sylData.lines);
    for (i = 0; i < pInitData->sylData.lines; i ++)
        PyTuple_SET_ITEM(py_endTime1D, i, PyLong_FromLong(pInitData->sylData.endTime1D[i]));
    PyTuple_SET_ITEM(pyArgs, 27, py_endTime1D);
    py_count1D = PyTuple_New(pInitData->sylData.lines);
    for (i = 0; i < pInitData->sylData.lines; i ++)
        PyTuple_SET_ITEM(py_count1D, i, PyLong_FromLong(pInitData->sylData.count1D[i]));
    PyTuple_SET_ITEM(pyArgs, 28, py_count1D);
    py_time2D = PyTuple_New(pInitData->sylData.lines);
    count = 0;
    for (i = 0; i < pInitData->sylData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->sylData.count1D[i]);
        for (j = 0; j < pInitData->sylData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->sylData.time2D[count ++]));
        PyTuple_SET_ITEM(py_time2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 29, py_time2D);
    py_timeDiff2D = PyTuple_New(pInitData->sylData.lines);
    count = 0;
    for (i = 0; i < pInitData->sylData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->sylData.count1D[i]);
        for (j = 0; j < pInitData->sylData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->sylData.timeDiff2D[count ++]));
        PyTuple_SET_ITEM(py_timeDiff2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 30, py_timeDiff2D);
    py_text2D = PyTuple_New(pInitData->sylData.lines);
    count = 0;
    for (i = 0; i < pInitData->sylData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->sylData.count1D[i]);
        for (j = 0; j < pInitData->sylData.count1D[i]; j ++) {
            PyTuple_SET_ITEM(pyTemp, j, PyUnicode_FromUnicode(pInitData->sylData.text2D[count], wcslen(pInitData->sylData.text2D[count])));
            count ++;
        }
        PyTuple_SET_ITEM(py_text2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 31, py_text2D);
    /* tm info */
    PyTuple_SET_ITEM(pyArgs, 32, PyLong_FromLong(pInitData->tmData.ascender));
    PyTuple_SET_ITEM(pyArgs, 33, PyLong_FromLong(pInitData->tmData.descender));
    py_width2D = PyTuple_New(pInitData->tmData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmData.count1D[i]);
        for (j = 0; j < pInitData->tmData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmData.width2D[count ++]));
        PyTuple_SET_ITEM(py_width2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 34, py_width2D);
    py_height2D = PyTuple_New(pInitData->tmData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmData.count1D[i]);
        for (j = 0; j < pInitData->tmData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmData.height2D[count ++]));
        PyTuple_SET_ITEM(py_height2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 35, py_height2D);
    py_kerning2D = PyTuple_New(pInitData->tmData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmData.count1D[i]);
        for (j = 0; j < pInitData->tmData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmData.kerning2D[count ++]));
        PyTuple_SET_ITEM(py_kerning2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 36, py_kerning2D);
    py_advance2D = PyTuple_New(pInitData->tmData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmData.count1D[i]);
        for (j = 0; j < pInitData->tmData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmData.advance2D[count ++]));
        PyTuple_SET_ITEM(py_advance2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 37, py_advance2D);
    py_advanceDiff2D = PyTuple_New(pInitData->tmData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmData.count1D[i]);
        for (j = 0; j < pInitData->tmData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmData.advanceDiff2D[count ++]));
        PyTuple_SET_ITEM(py_advanceDiff2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 38, py_advanceDiff2D);
    py_length1D = PyTuple_New(pInitData->tmData.lines);
    for (i = 0; i < pInitData->tmData.lines; i ++)
        PyTuple_SET_ITEM(py_length1D, i, PyLong_FromLong(pInitData->tmData.length1D[i]));
    PyTuple_SET_ITEM(pyArgs, 39, py_length1D);
    py_initX2D = PyTuple_New(pInitData->tmData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmData.count1D[i]);
        for (j = 0; j < pInitData->tmData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmData.initX2D[count ++]));
        PyTuple_SET_ITEM(py_initX2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 40, py_initX2D);
    py_initY2D = PyTuple_New(pInitData->tmData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmData.count1D[i]);
        for (j = 0; j < pInitData->tmData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmData.initY2D[count ++]));
        PyTuple_SET_ITEM(py_initY2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 41, py_initY2D);
    py_bearingY2D = PyTuple_New(pInitData->tmData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmData.count1D[i]);
        for (j = 0; j < pInitData->tmData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmData.bearingY2D[count ++]));
        PyTuple_SET_ITEM(py_bearingY2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 42, py_bearingY2D);
    pyDataFunc = PyObject_GetAttrString(pTcxPy->pyBaseModule, "tcxInitData");
    if (!pyDataFunc) {
        Py_CLEAR(pyArgs);
        Py_CLEAR(pTcxPy->pyBaseModule);
        PyErr_Print();
        /* PyErr_Clear(); */
        return py_error_init_fail;
    } else if (!PyCallable_Check(pyDataFunc)) {
        Py_CLEAR(pyArgs);
        Py_CLEAR(pyDataFunc);
        Py_CLEAR(pTcxPy->pyBaseModule);
        return py_error_init_fail;
    }
    pyData = PyTuple_New(1);
    PyTuple_SET_ITEM(pyData, 0, pyArgs);
    if (PyObject_CallObject(pyDataFunc, pyData) == NULL) {
        Py_CLEAR(pyArgs);
        Py_CLEAR(pyData);
        Py_CLEAR(pyDataFunc);
        Py_CLEAR(pTcxPy->pyBaseModule);
        PyErr_Print();
        /* PyErr_Clear(); */
        return py_error_init_fail;
    }
    Py_CLEAR(pyData);
    Py_CLEAR(pyDataFunc);
    pTcxPy->pyUserModule = NULL;
    pTcxPy->pyInitFunc = NULL;
    pTcxPy->pyUserFunc = NULL;
    pTcxPy->pyMainFunc = NULL;
    pTcxPy->pyFinFunc = NULL;
    return py_error_success;
}

static void _tcxpy_get_py_module_name_and_dir(const wchar_t *pyFilename, wchar_t **pPyModuleName, wchar_t **pPyModuleDir) {
    int i, indicator, len, count, flag;
    wchar_t *pyModuleName;
    wchar_t *pyModuleDir;
    flag = 0;
    count = 0;
    indicator = 0;
    len = wcslen(pyFilename);
    for (i = len - 1; i >= 0; i --) {
        if (0 == flag && L'.' == pyFilename[i]) flag = 1;
        else if (1 == flag) {
            if (L'\\' == pyFilename[i] || L'/' == pyFilename[i]) {
                indicator = i;
                break;
            } else count ++;
        }
    }
    if (0 == count) {
        *pPyModuleName = NULL;
        *pPyModuleDir = NULL;
    } else if (0 == indicator) {
        pyModuleName = (wchar_t *)malloc((count + 1) * sizeof(wchar_t));
        memcpy(pyModuleName, pyFilename, count * sizeof(wchar_t));
        pyModuleName[count] = L'\0';
        *pPyModuleName = pyModuleName;
        *pPyModuleDir = NULL;
    } else {
        pyModuleName = (wchar_t *)malloc((count + 1) * sizeof(wchar_t));
        memcpy(pyModuleName, pyFilename + indicator + 1, count * sizeof(wchar_t));
        pyModuleName[count] = L'\0';
        *pPyModuleName = pyModuleName;
        count = indicator;
        pyModuleDir = (wchar_t *)malloc((count + 1) * sizeof(wchar_t));
        memcpy(pyModuleDir, pyFilename, count * sizeof(wchar_t));
        pyModuleDir[count] = L'\0';
        *pPyModuleDir = pyModuleDir;
    }
}

static int _tcxpy_is_specified_py_module_existed(const wchar_t *filename) {
    int len;
    wchar_t *pycFilename;
    len = wcslen(filename);
    pycFilename = (wchar_t *)malloc((len + 2) * sizeof(wchar_t));
    memcpy(pycFilename, filename, len * sizeof(wchar_t));
    pycFilename[len] = L'c';
    pycFilename[len + 1] = L'\0';
    if (GetFileAttributesW(filename) == INVALID_FILE_ATTRIBUTES && GetFileAttributesW(pycFilename) == INVALID_FILE_ATTRIBUTES) {
        free(pycFilename);
        return 0;
    }
    free(pycFilename);
    return 1;
}

static int _tcxpy_is_py_module_existed(const wchar_t *userPyFilename, const wchar_t *directory, const wchar_t *pyModuleName, wchar_t **pPyFilename) {
    int      dir_len, name_len, count, existed;
    wchar_t *pyFilename;
    existed = _tcxpy_is_specified_py_module_existed(userPyFilename);  /* checks both py and pyc (if one exists then returns 1) */
    if (existed) {
        count = wcslen(userPyFilename) + 1;
        pyFilename = (wchar_t *)malloc(count * sizeof(wchar_t));
        memcpy(pyFilename, userPyFilename, count * sizeof(wchar_t));
        *pPyFilename = pyFilename;
        return 1;
    }
    dir_len = wcslen(directory);
    name_len = wcslen(pyModuleName);
    count = dir_len + 1 + name_len + 4;
    pyFilename = (wchar_t *)malloc(count * sizeof(wchar_t));
    memcpy(pyFilename, directory, dir_len * sizeof(wchar_t));
    pyFilename[dir_len] = L'\\';
    memcpy(pyFilename + dir_len + 1, pyModuleName, name_len * sizeof(wchar_t));
    memcpy(pyFilename + dir_len + 1 + name_len, L".py\0", 4 * sizeof(wchar_t));
    existed = _tcxpy_is_specified_py_module_existed(pyFilename);    /* if the py module is not found in the tcc file's directory then check Program's root dir */
    if (existed) {
        *pPyFilename = pyFilename;
        return 1;
    }
    free(pyFilename);
    count = dir_len + 5 + name_len + 4;
    pyFilename = (wchar_t *)malloc(count * sizeof(wchar_t));
    memcpy(pyFilename, directory, dir_len * sizeof(wchar_t));
    memcpy(pyFilename + dir_len, L"\\Lib\\", 5 * sizeof(wchar_t));
    memcpy(pyFilename + dir_len + 5, pyModuleName, name_len * sizeof(wchar_t));
    memcpy(pyFilename + dir_len + 5 + name_len, L".py\0", 4 * sizeof(wchar_t));
    existed = _tcxpy_is_specified_py_module_existed(pyFilename);    /* if the py module is not found in the tcc file's directory then check Program's root\Lib dir */
    if (existed) {
        *pPyFilename = pyFilename;
        return 1;
    }
    free(pyFilename);
    *pPyFilename = NULL;
    return 0;
}

static void _tcxpy_make_user_py_module_path(const wchar_t *pyModuleDir, const wchar_t *directory, wchar_t **pPyModulePath) {
    int mod_len, dir_len, count;
    wchar_t *pyModulePath;
    mod_len = wcslen(pyModuleDir);
    dir_len = wcslen(directory);
    count = mod_len + 1 + dir_len + 1 + dir_len + 5;
    pyModulePath = (wchar_t *)malloc(count * sizeof(wchar_t));
    memcpy(pyModulePath, pyModuleDir, mod_len * sizeof(wchar_t));
    pyModulePath[mod_len] = L';';
    memcpy(pyModulePath + mod_len + 1, directory, dir_len * sizeof(wchar_t));
    pyModulePath[mod_len + 1 + dir_len] = L';';
    memcpy(pyModulePath + mod_len + 1 + dir_len + 1, directory, dir_len * sizeof(wchar_t));
    memcpy(pyModulePath + mod_len + 1 + dir_len + 1 + dir_len, L"\\Lib\0", 5 * sizeof(wchar_t));
    *pPyModulePath = pyModulePath;
}

static void _tcxpy_fin_user_py_module(const PY_pTcxPy pTcxPy) {
    Py_CLEAR(pTcxPy->pyInitFunc);
    Py_CLEAR(pTcxPy->pyUserFunc);
    Py_CLEAR(pTcxPy->pyMainFunc);
    Py_CLEAR(pTcxPy->pyFinFunc);
    Py_CLEAR(pTcxPy->pyUserModule);
    pTcxPy->pyInitFunc = NULL;
    pTcxPy->pyUserFunc = NULL;
    pTcxPy->pyMainFunc = NULL;
    pTcxPy->pyFinFunc = NULL;
    pTcxPy->pyUserModule = NULL;
}

PY_Error_Code tcxpy_init_user_py_module(PY_pTcxPy pTcxPy, const wchar_t *directory, const wchar_t *userPyFilename, int use_tcxpy_init_func, int use_tcxpy_user_func, int use_tcxpy_fin_func) {
    PyObject *pyModule;
    wchar_t *pyModuleName;
    wchar_t *pyModuleDir;
    wchar_t *pyFilename;
    wchar_t *tokenFilename;
    wchar_t *pyModulePath;
    char    *ansiPyModuleName;
    if (!pTcxPy || !directory || !userPyFilename) return py_error_null_pointer;
    _tcxpy_fin_user_py_module(pTcxPy);
    _tcxpy_get_py_module_name_and_dir(userPyFilename, &pyModuleName, &pyModuleDir);
    if (!pyModuleName) return py_error_null_pointer;
    if (!_tcxpy_is_py_module_existed(userPyFilename, directory, pyModuleName, &pyFilename)) {
        free(pyModuleName);
        free(pyModuleDir);
        return py_error_file_not_found;
    }
    if (tcxpy_convert_file_to_utf8(pyFilename) != py_error_success) {
        free(pyModuleName);
        free(pyModuleDir);
        free(pyFilename);
        return py_error_init_fail;
    }
    _tcxpy_make_user_py_module_path(pyModuleDir, directory, &pyModulePath);
    free(pyModuleDir);
    PySys_SetPath(pyModulePath);
    free(pyModulePath);
    _tcxpy_sz_unicode_to_ansi(pyModuleName, &ansiPyModuleName);
    free(pyModuleName);
    pyModule = PyImport_ImportModuleNoBlock(ansiPyModuleName);
    free(ansiPyModuleName);
    if (!pyModule) {
        PyErr_Print();
        /* PyErr_Clear(); */
        return py_error_init_fail;
    }
    tcxpy_make_py_token_filename(pyFilename, &tokenFilename);
    if (tcxpy_is_py_modified(pyFilename, tokenFilename) == py_error_file_modified) {    /* check if we need to reload the module */
        free(pyFilename);
        free(tokenFilename);
        pTcxPy->pyUserModule = PyImport_ReloadModule(pyModule);  // refresh the module
        Py_CLEAR(pyModule);
        if (!pTcxPy->pyUserModule) {
            PyErr_Print();
            /* PyErr_Clear(); */
            return py_error_init_fail;
        }
    } else {
        free(pyFilename);
        free(tokenFilename);
        pTcxPy->pyUserModule = pyModule;
    }
    /* tcxPy_Init (Alternative) */
    if (use_tcxpy_init_func) {
        pTcxPy->pyInitFunc = PyObject_GetAttrString(pTcxPy->pyUserModule, "tcxPy_Init");
        if (!pTcxPy->pyInitFunc) {
            _tcxpy_fin_user_py_module(pTcxPy);
            PyErr_Print();
            /* PyErr_Clear(); */
            return py_error_init_fail;
        } else if (!PyCallable_Check(pTcxPy->pyInitFunc)) {
            _tcxpy_fin_user_py_module(pTcxPy);
            return py_error_init_fail;
        }
    }
    /* tcxPy_Main or tcxPy_User */
    if (use_tcxpy_user_func) {
        pTcxPy->pyUserFunc = PyObject_GetAttrString(pTcxPy->pyUserModule, "tcxPy_User");
        if (!pTcxPy->pyUserFunc) {
            _tcxpy_fin_user_py_module(pTcxPy);
            PyErr_Print();
            /* PyErr_Clear(); */
            return py_error_init_fail;
        } else if (!PyCallable_Check(pTcxPy->pyUserFunc)) {
            _tcxpy_fin_user_py_module(pTcxPy);
            return py_error_init_fail;
        }
    } else {
        pTcxPy->pyMainFunc = PyObject_GetAttrString(pTcxPy->pyUserModule, "tcxPy_Main");
        if (!pTcxPy->pyMainFunc) {
            _tcxpy_fin_user_py_module(pTcxPy);
            PyErr_Print();
            /* PyErr_Clear(); */
            return py_error_init_fail;
        } else if (!PyCallable_Check(pTcxPy->pyMainFunc)) {
            _tcxpy_fin_user_py_module(pTcxPy);
            return py_error_init_fail;
        }
    }
    /* tcxPy_Fin (Alternative) */
    if (use_tcxpy_fin_func) {
        pTcxPy->pyFinFunc = PyObject_GetAttrString(pTcxPy->pyUserModule, "tcxPy_Fin");
        if (!pTcxPy->pyFinFunc) {
            _tcxpy_fin_user_py_module(pTcxPy);
            PyErr_Print();
            /* PyErr_Clear(); */
            return py_error_init_fail;
        } else if (!PyCallable_Check(pTcxPy->pyFinFunc)) {
            _tcxpy_fin_user_py_module(pTcxPy);
            return py_error_init_fail;
        }
    }
    return py_error_success;
}

PY_Error_Code tcxpy_script_func_init(const PY_pTcxPy pTcxPy) {
    if (PyObject_CallObject(pTcxPy->pyInitFunc, NULL) == NULL) {
        _tcxpy_fin_user_py_module(pTcxPy);
        PyErr_Print();
        /* PyErr_Clear(); */
        return py_error_exec_fail;
    }
    return py_error_success;
}

PY_Error_Code tcxpy_script_func_user(const PY_pTcxPy pTcxPy) {
    if (PyObject_CallObject(pTcxPy->pyUserFunc, NULL) == NULL) {
        _tcxpy_fin_user_py_module(pTcxPy);
        PyErr_Print();
        /* PyErr_Clear(); */
        return py_error_exec_fail;
    }
    return py_error_success;
}

void **tcxpy_script_func_main(const PY_pTcxPy pTcxPy, int iLine, int iText, int nTexts, int start, int end, int timeDiff, int time, int x, int y, int advance, const wchar_t *text) {
    PyObject *pyArgs;          /* params to user tcx py script's tcxPy_Main function */
    PyObject *pyReturnedBuf;   /* the value returned from tcxPy_Main function */
    void    **pBufToReturn;    /* value return to tcx module */
    PyObject *pyAssList;
    PyObject *pyTcsList;
    pyArgs = PyTuple_New(11);
    PyTuple_SetItem(pyArgs, 0, PyLong_FromLong(iLine));
    PyTuple_SetItem(pyArgs, 1, PyLong_FromLong(iText));
    PyTuple_SetItem(pyArgs, 2, PyLong_FromLong(nTexts));
    PyTuple_SetItem(pyArgs, 3, PyLong_FromLong(start));
    PyTuple_SetItem(pyArgs, 4, PyLong_FromLong(end));
    PyTuple_SetItem(pyArgs, 5, PyLong_FromLong(timeDiff));
    PyTuple_SetItem(pyArgs, 6, PyLong_FromLong(time));
    PyTuple_SetItem(pyArgs, 7, PyLong_FromLong(x));
    PyTuple_SetItem(pyArgs, 8, PyLong_FromLong(y));
    PyTuple_SetItem(pyArgs, 9, PyLong_FromLong(advance));
    PyTuple_SetItem(pyArgs, 10, PyUnicode_FromUnicode(text, wcslen(text)));
    pyReturnedBuf = PyObject_CallObject(pTcxPy->pyMainFunc, pyArgs);
    Py_CLEAR(pyArgs);
    if (!pyReturnedBuf) {
        PyErr_Print();
        /* PyErr_Clear(); */
        return NULL;
    }
    if (!PyTuple_Check(pyReturnedBuf)) {
        Py_CLEAR(pyReturnedBuf);
        /* fwprintf_s(stderr, L"Error: return value from py script is invalid\n"); */
        return NULL;
    }
    pBufToReturn = (void **)malloc(4 * sizeof(void *));
    pyAssList = PyTuple_GET_ITEM(pyReturnedBuf, 0);
    if (PyList_Check(pyAssList)) {
        int i, count, size, offset;
        wchar_t *assBuf;
        wchar_t **pAssLine;
        int     *assLineSize;
        count = PyList_GET_SIZE(pyAssList);
        if (0 == count) {
            pBufToReturn[0] = (void *)0;
            pBufToReturn[1] = NULL;
        } else {
            assLineSize = (int *)malloc(count * sizeof(int));
            pAssLine = (wchar_t **)malloc(count * sizeof(wchar_t *));
            size = 0;
            for (i = 0; i < count; i ++) {
                pAssLine[i] = PyUnicode_AS_UNICODE(PyList_GET_ITEM(pyAssList, i));
                assLineSize[i] = wcslen(pAssLine[i]);
                size += assLineSize[i];
            }
            assBuf = (wchar_t *)malloc((size + 1) * sizeof(wchar_t));
            offset = 0;
            for (i = 0; i < count; i ++) {
                memcpy(assBuf + offset, pAssLine[i], assLineSize[i] * sizeof(wchar_t));
                offset += assLineSize[i];
            }
            assBuf[size] = L'\0';
            free(pAssLine);
            free(assLineSize);
            pBufToReturn[0] = (void *)size;
            pBufToReturn[1] = (void *)assBuf;
        }
    } else {
        pBufToReturn[0] = (void *)0;
        pBufToReturn[1] = NULL;
    }
    pyTcsList = PyTuple_GET_ITEM(pyReturnedBuf, 1);
    if (PyList_Check(pyTcsList)) {
        int i, count;
        PyObject *pyTcsItem;
        unsigned short x, y;
        unsigned long RGB;
        unsigned char alpha;
        unsigned long *tcsBuf;
        count = PyList_GET_SIZE(pyTcsList);
        if (0 == count) {
            pBufToReturn[2] = (void *)0;
            pBufToReturn[3] = NULL;
        }
        else {
            /* tcs buffer from user tcx py script: (Start, End, Layer, PosX, PosY, RGB, Alpha) */
            /* a raw tcs chunk defined in TCC file format specification: (Start, End, Layer, Pos, RGBA) */
            tcsBuf = (unsigned long *)malloc(count * 5 * sizeof(unsigned long));
            for (i = 0; i < count; i ++) {
                pyTcsItem = PyList_GET_ITEM(pyTcsList, i);
                x = (unsigned short)PyLong_AS_LONG(PyTuple_GET_ITEM(pyTcsItem, 3));
                y = (unsigned short)PyLong_AS_LONG(PyTuple_GET_ITEM(pyTcsItem, 4));
                RGB = (unsigned long)PyLong_AS_LONG(PyTuple_GET_ITEM(pyTcsItem, 5));
                alpha = (unsigned char)PyLong_AS_LONG(PyTuple_GET_ITEM(pyTcsItem, 6));
                tcsBuf[5 * i + 0] = PyLong_AS_LONG(PyTuple_GET_ITEM(pyTcsItem, 0));    /* Start */
                tcsBuf[5 * i + 1] = PyLong_AS_LONG(PyTuple_GET_ITEM(pyTcsItem, 1));    /* End */
                tcsBuf[5 * i + 2] = PyLong_AS_LONG(PyTuple_GET_ITEM(pyTcsItem, 2));    /* Layer */
                tcsBuf[5 * i + 3] = (x | ((unsigned long)y << 16));
                tcsBuf[5 * i + 4] = (RGB | ((unsigned long)alpha << 24));
            }
            pBufToReturn[2] = (void *)count;
            pBufToReturn[3] = tcsBuf;
        }
    } else {
        pBufToReturn[2] = (void *)0;
        pBufToReturn[3] = NULL;
    }
    Py_CLEAR(pyReturnedBuf);
    return pBufToReturn;
}

PY_Error_Code tcxpy_script_func_fin(const PY_pTcxPy pTcxPy) {
    if (PyObject_CallObject(pTcxPy->pyFinFunc, NULL) == NULL) {
        _tcxpy_fin_user_py_module(pTcxPy);
        PyErr_Print();
        /* PyErr_Clear(); */
        return py_error_exec_fail;
    }
    return py_error_success;
}

PY_Error_Code tcxpy_fin_tcxpy(PY_pTcxPy pTcxPy) {
    if (!pTcxPy) return py_error_null_pointer;
    Py_CLEAR(pTcxPy->pyInitFunc);
    Py_CLEAR(pTcxPy->pyUserFunc);
    Py_CLEAR(pTcxPy->pyMainFunc);
    Py_CLEAR(pTcxPy->pyFinFunc);
    Py_CLEAR(pTcxPy->pyUserModule);
    Py_CLEAR(pTcxPy->pyBaseModule);
    return py_error_success;
}

