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

#include "py.h"


#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif    /* _MSC_VER */

#if defined WIN32
/* Convert between unicode big endian and unicode little endian */
static void _tcaxpy_convert_endian(wchar_t *buffer, int count) {
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

static PY_Error_Code _tcaxpy_read_file_to_unicode(const char *filename, wchar_t **pBuffer, int *pCount) {
    FILE *fp;
    int size, count;
    unsigned char *rawBuffer;
    wchar_t *buffer;
    fp = fopen(filename, "rb");
    if (!fp) return py_error_file_cant_open;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
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
        count = (size - 2) / 2;        /* reduce count for the BOM-header */
        buffer = (wchar_t *)malloc((count + 1) * sizeof(wchar_t));
        memcpy(buffer, rawBuffer + 2, (count + 1) * sizeof(wchar_t));    /* `+2' is intending to remove the BOM-header */
        free(rawBuffer);
        *pBuffer = buffer;
        *pCount = count;
    } else if (size >= 2 && 0xFE == rawBuffer[0] && 0xFF == rawBuffer[1]) {    /* unicode big endian */
        _tcaxpy_convert_endian((wchar_t *)rawBuffer, size / 2);
        count = (size - 2) / 2;        /* reduce count for the BOM-header */
        buffer = (wchar_t *)malloc((count + 1) * sizeof(wchar_t));
        memcpy(buffer, rawBuffer + 2, (count + 1) * sizeof(wchar_t));    /* `+2' is intending to remove the BOM-header */
        free(rawBuffer);
        *pBuffer = buffer;
        *pCount = count;
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

PY_Error_Code tcaxpy_convert_file_to_utf8(const char *filename) {
    FILE *fp;
    unsigned char sig[3];
    fp = fopen(filename, "rb");
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
        if (_tcaxpy_read_file_to_unicode(filename, &uni, &count) != py_error_success) return py_error_file_while_reading;
        size = WideCharToMultiByte(CP_UTF8, 0, uni, count, NULL, 0, NULL, NULL);
        buffer = (char *)malloc(size * sizeof(char));
        WideCharToMultiByte(CP_UTF8, 0, uni, count, buffer, size, NULL, NULL);
        free(uni);
        fp = fopen(filename, "wb");
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
#else
static PY_Error_Code _tcaxpy_read_file_to_unicode(const char *filename, wchar_t **pBuffer, int *pCount) {
    FILE *fp;
    int size, count;
    unsigned char *rawBuffer;
    wchar_t *buffer;
    fp = fopen(filename, "rb");
    if (!fp) return py_error_file_cant_open;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    rawBuffer = (unsigned char *)malloc(size + 2);    /* note that `+2' is intending to add a null terminator */
    rawBuffer[size] = 0;
    rawBuffer[size + 1] = 0;
    if (fread(rawBuffer, sizeof(unsigned char), size, fp) != size) {
        free(rawBuffer);
        fclose(fp);
        return py_error_file_while_reading;
    }
    fclose(fp);
    if (size >= 3 && 0xEF == rawBuffer[0] && 0xBB == rawBuffer[1] && 0xBF == rawBuffer[2]) {    /* utf-8 */
        setlocale(LC_CTYPE, "");
        count = mbstowcs(NULL, (const char *)(rawBuffer + 3), 0) + 1;    /* `+3' is intending to remove the BOM-header */
        buffer = (wchar_t *)malloc(count * sizeof(wchar_t));
        mbstowcs(buffer, (const char *)(rawBuffer + 3), count);    /* `+3' is intending to remove the BOM-header */
        free(rawBuffer);
        *pBuffer = buffer;
        *pCount = count - 1;    /* reduce count for the null terminator */
    } else return py_error_file_while_reading;
    return py_error_success;
}

PY_Error_Code tcaxpy_convert_file_to_utf8(const char *filename) {
    FILE *fp;
    unsigned char sig[3];
    fp = fopen(filename, "rb");
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
        if (_tcaxpy_read_file_to_unicode(filename, &uni, &count) != py_error_success) return py_error_file_while_reading;
        setlocale(LC_CTYPE, "");
        size = wcstombs(NULL, uni, 0) + 1;
        buffer = (char *)malloc(size * sizeof(char));
        wcstombs(buffer, uni, size);
        free(uni);
        fp = fopen(filename, "wb");
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
#endif  /* WIN32 */

PY_Error_Code tcaxpy_init_python() {
    Py_Initialize();
    if (Py_IsInitialized() == 0) return py_error_init_fail;
    return py_error_success;
}

PY_Error_Code tcaxpy_fin_python() {
    Py_Finalize();
    if (Py_IsInitialized() != 0) return py_error_fin_fail;
    return py_error_success;
}

#if defined WIN32
static void _tcaxpy_sz_unicode_to_ansi(const wchar_t *uni, char **ansi) {
    int size;
    char *sz;
    size = WideCharToMultiByte(CP_ACP, 0, uni, -1, NULL, 0, NULL, NULL);
    sz = (char *)malloc(size * sizeof(char));
    WideCharToMultiByte(CP_ACP, 0, uni, -1, sz, size, NULL, NULL);
    *ansi = sz;
}

static void _tcaxpy_sz_ansi_to_unicode(const char *ansi, wchar_t **uni) {
    int count;
    wchar_t *sz;
    count = MultiByteToWideChar(CP_ACP, 0, ansi, -1, NULL, 0);
    sz = (wchar_t *)malloc(count * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, ansi, -1, sz, count);
    *uni = sz;
}
#else
static void _tcaxpy_sz_unicode_to_ansi(const wchar_t *uni, char **ansi) {
    int size;
    char *sz;
    setlocale(LC_CTYPE, "");
    size = wcstombs(NULL, uni, 0) + 1;
    sz = (char *)malloc(size * sizeof(char));
    wcstombs(sz, uni, size);
    *ansi = sz;
}

static void _tcaxpy_sz_ansi_to_unicode(const char *ansi, wchar_t **uni) {
    int count;
    wchar_t *sz;
    setlocale(LC_CTYPE, "");
    count = mbstowcs(NULL, ansi, 0) + 1;
    sz = (wchar_t *)malloc(count * sizeof(wchar_t));
    mbstowcs(sz, ansi, count);
    *uni = sz;
}
#endif  /* WIN32 */

PY_Error_Code tcaxpy_exec_py_script(const char *filename) {
    int len, size;
    char *paramBuf;
    len = strlen(filename);
    size = 11 + len + 11;    /* "exec(open('<filename>').read())" */
    paramBuf = (char *)malloc(size * sizeof(char));
    memcpy(paramBuf, "exec(open('", 11 * sizeof(char));
    memcpy(paramBuf + 11, filename, len * sizeof(char));
    memcpy(paramBuf + 11 + len, "').read())\0", 11 * sizeof(char));
    if (PyRun_SimpleString(paramBuf) != 0) {
        free(paramBuf);
        return py_error_exec_fail;
    }
    free(paramBuf);
    return py_error_success;
}

/* tcax py token file functions */

const char *tcaxpy_make_py_token_filename(const char *filename, char **pTokenFilename) {
    int count;
    char *tokenFilename;
    count = strlen(filename);
    tokenFilename = (char *)malloc((count + 7) * sizeof(char));
    memcpy(tokenFilename, filename, count * sizeof(char));
    memcpy(tokenFilename + count, ".token\0", 7 * sizeof(char));
    *pTokenFilename = tokenFilename;
    return (const char *)tokenFilename;
}

#if defined WIN32
PY_Error_Code tcaxpy_create_py_token(const char *filename) {
    FILE *fp;
    fp = fopen(filename, "w");
    if (!fp) py_error_file_cant_create;
    fclose(fp);
    return py_error_success;
}

PY_Error_Code tcaxpy_synchronize_py_token(const char *filename, const char *tokenFilename) {
    HANDLE   hPyFile;
    HANDLE   hTokenFile;
    FILETIME pyFileTime;
    if (!tokenFilename || GetFileAttributesA(tokenFilename) == INVALID_FILE_ATTRIBUTES) {
        char *buffer;
        tcaxpy_make_py_token_filename(filename, &buffer);
        if (tcaxpy_create_py_token(buffer) != py_error_success) {
            free(buffer);
            return py_error_file_cant_create;
        }
        hTokenFile = CreateFileA(buffer, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        free(buffer);
    } else hTokenFile = CreateFileA(tokenFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hTokenFile) return py_error_file_cant_open;
    hPyFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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

PY_Error_Code tcaxpy_is_py_modified(const char *filename, const char *tokenFilename) {
    HANDLE   hPyFile;
    HANDLE   hTokenFile;
    FILETIME pyFileTime;
    FILETIME tokenFileTime;
    if (!filename) return py_error_null_pointer;
    if (GetFileAttributesA(filename) == INVALID_FILE_ATTRIBUTES) return py_error_file_not_found;     /* if the py script file dosen't exists then it means there is only a pyc file which we can't modify */
    if (!tokenFilename || GetFileAttributesA(tokenFilename) == INVALID_FILE_ATTRIBUTES) {
        char *buffer;
        tcaxpy_make_py_token_filename(filename, &buffer);
        if (tcaxpy_create_py_token(buffer) != py_error_success) {
            free(buffer);
            return py_error_file_cant_create;
        }
        tcaxpy_synchronize_py_token(filename, buffer);
        free(buffer);
        return py_error_file_modified;
    }
    hPyFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hPyFile) return py_error_file_cant_open;
    hTokenFile = CreateFileA(tokenFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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
#else
PY_Error_Code tcaxpy_create_py_token(const char *filename) {
    return py_error_file_cant_create;
}

PY_Error_Code tcaxpy_synchronize_py_token(const char *filename, const char *tokenFilename) {
    return py_error_file_cant_create;
}
PY_Error_Code tcaxpy_is_py_modified(const char *filename, const char *tokenFilename) {
    return py_error_file_modified;
}
#endif  /* WIN32 */

static PY_Error_Code _tcaxpy_write_unicode_to_utf8_file(const char *filename, const wchar_t *uni, int count) {
    FILE *fp;
    int size;
    char *utf8String;
    unsigned char BOM[3];
    fp = fopen(filename, "wb");
    if (!fp) return py_error_file_cant_create;
    BOM[0] = 0xEF;
    BOM[1] = 0xBB;
    BOM[2] = 0xBF;
    if (fwrite(BOM, sizeof(unsigned char), 3, fp) != 3) {
        fclose(fp);
        return py_error_file_while_writing;
    }
#if defined WIN32
    size = WideCharToMultiByte(CP_UTF8, 0, uni, count, NULL, 0, NULL, NULL);
    utf8String = (char *)malloc(size);
    WideCharToMultiByte(CP_UTF8, 0, uni, count, utf8String, size, NULL, NULL);
#else
    setlocale(LC_CTYPE, "");
    size = wcstombs(NULL, uni, 0) + 1;
    utf8String = (char *)malloc(size * sizeof(char));
    wcstombs(utf8String, uni, size);
#endif  /* WIN32 */
    if (fwrite(utf8String, sizeof(char), size, fp) != size) {
        free(utf8String);
        fclose(fp);
        return py_error_file_while_writing;
    }
    free(utf8String);
    fclose(fp);
    return py_error_success;
}

PY_Error_Code tcaxpy_create_py_template(const char *filename) {
    const wchar_t *py_file_buf = L"from tcaxPy import *\r\n\
\r\n\
\r\n\
def tcaxPy_Init():\r\n\
    print('Tips: you can initialize your global variables here.')\r\n\
    print('This function will be executed once if you set `< tcaxpy init = true >\\'')\r\n\
    print(GetHelp())\r\n\
\r\n\
\r\n\
def tcaxPy_User():\r\n\
    print('User defined function.')\r\n\
    print('This function will be executed once if you set `< tcaxpy user = true >\\'')\r\n\
    print('Otherwise, tcaxPy_Main will be executed (many times)')\r\n\
\r\n\
\r\n\
def tcaxPy_Fin():\r\n\
    print('Tips: you can finalize your global variables here.')\r\n\
    print('This function will be executed once if you set `< tcaxpy fin = true >\\'')\r\n\
    print('Note: you do not need to finalize the global variables got from function GetVal()')\r\n\
\r\n\
\r\n\
def tcaxPy_Main(_i, _j, _n, _start, _end, _elapk, _k, _x, _y, _a, _txt):\r\n\
\r\n\
    ASS_BUF  = []        # used for saving ASS FX lines\r\n\
    TCAS_BUF = []        # used for saving TCAS FX raw data\r\n\
\r\n\
    #############################\r\n\
    # TODO: write your codes here #\r\n\
\r\n\
    ass_main(ASS_BUF, SubL(_start, _end), pos(_x, _y) + K(_elapk) + K(_k), _txt)\r\n\
\r\n\
    #############################\r\n\
\r\n\
    return (ASS_BUF, TCAS_BUF)\r\n\
\r\n\
\r\n\
\r\n\
\r\n\
\r\n";
    return _tcaxpy_write_unicode_to_utf8_file(filename, py_file_buf, wcslen(py_file_buf));
}

/* main functions */

static void _tcaxpy_make_base_py_module_path(const char *directory, char **pPyModulePath) {
    int len, count;
    char *pyModulePath;
    len = strlen(TCAXPY_PY_LIB_FOLDER);
    count = strlen(directory);
    pyModulePath = (char *)malloc((count + 1 + count + 1 + len + 1) * sizeof(char));
    memcpy(pyModulePath, directory, count * sizeof(char));
    pyModulePath[count] = ';';
    memcpy(pyModulePath + count + 1, directory, count * sizeof(char));
    pyModulePath[count + 1 + count] = '\\';
    memcpy(pyModulePath + count + 1 + count + 1, TCAXPY_PY_LIB_FOLDER, len * sizeof(char));
    pyModulePath[count + 1 + count + 1 + len] = '\0';
    *pPyModulePath = pyModulePath;
}

PY_Error_Code tcaxpy_init_base_py_module(PY_pTcaxPy pTcaxPy, const PY_pInitData pInitData) {
    int i, j, count;
    char *pyModulePath;
    wchar_t *wcsPyModulePath;
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
    /* horizontal */
    PyObject *py_widthH2D;
    PyObject *py_heightH2D;
    PyObject *py_kerningH2D;
    PyObject *py_advanceH2D;
    PyObject *py_advanceDiffH2D;
    PyObject *py_lengthH1D;
    PyObject *py_initXH2D;
    PyObject *py_initYH2D;
    PyObject *py_bearingYH2D;
    /* vertical */
    PyObject *py_widthV2D;
    PyObject *py_heightV2D;
    PyObject *py_kerningV2D;
    PyObject *py_advanceV2D;
    PyObject *py_advanceDiffV2D;
    PyObject *py_lengthV1D;
    PyObject *py_initXV2D;
    PyObject *py_initYV2D;
    PyObject *py_bearingXV2D;
    /***/
    _tcaxpy_make_base_py_module_path(pInitData->directory, &pyModulePath);
    _tcaxpy_sz_ansi_to_unicode(pyModulePath, &wcsPyModulePath);
    free(pyModulePath);
    Py_SetPath(wcsPyModulePath);  /* set the module's directory */
    free(wcsPyModulePath);
    pTcaxPy->pyBaseModule = PyImport_ImportModuleNoBlock(TCAXPY_PY_MODULE_NAME);
    if (!pTcaxPy->pyBaseModule) {
        PyErr_Print();
        /* PyErr_Clear(); */
        return py_error_init_fail;
    }
    pyArgs = PyTuple_New(53);
    PyTuple_SET_ITEM(pyArgs,  0, PyUnicode_FromUnicode((const Py_UNICODE *)pInitData->assHeader, wcslen(pInitData->assHeader)));
    PyTuple_SET_ITEM(pyArgs,  1, PyUnicode_FromUnicode((const Py_UNICODE *)pInitData->outFilename, wcslen(pInitData->outFilename)));
    /* tcc info */
    PyTuple_SET_ITEM(pyArgs,  2, PyUnicode_FromUnicode((const Py_UNICODE *)pInitData->tccData.font_file, wcslen(pInitData->tccData.font_file)));
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
    PyTuple_SET_ITEM(pyArgs, 13, PyUnicode_FromUnicode((const Py_UNICODE *)pInitData->tccData.font_face_name, wcslen(pInitData->tccData.font_face_name)));
    PyTuple_SET_ITEM(pyArgs, 14, PyLong_FromLong(pInitData->tccData.bord));
    PyTuple_SET_ITEM(pyArgs, 15, PyLong_FromLong(pInitData->tccData.shad));
    PyTuple_SET_ITEM(pyArgs, 16, PyUnicode_FromUnicode((const Py_UNICODE *)pInitData->tccData.primary_color, wcslen(pInitData->tccData.primary_color)));
    PyTuple_SET_ITEM(pyArgs, 17, PyUnicode_FromUnicode((const Py_UNICODE *)pInitData->tccData.secondary_color, wcslen(pInitData->tccData.secondary_color)));
    PyTuple_SET_ITEM(pyArgs, 18, PyUnicode_FromUnicode((const Py_UNICODE *)pInitData->tccData.outline_color, wcslen(pInitData->tccData.outline_color)));
    PyTuple_SET_ITEM(pyArgs, 19, PyUnicode_FromUnicode((const Py_UNICODE *)pInitData->tccData.back_color, wcslen(pInitData->tccData.back_color)));
    PyTuple_SET_ITEM(pyArgs, 20, PyLong_FromLong(pInitData->tccData.primary_alpha));
    PyTuple_SET_ITEM(pyArgs, 21, PyLong_FromLong(pInitData->tccData.secondary_alpha));
    PyTuple_SET_ITEM(pyArgs, 22, PyLong_FromLong(pInitData->tccData.outline_alpha));
    PyTuple_SET_ITEM(pyArgs, 23, PyLong_FromLong(pInitData->tccData.back_alpha));
    PyTuple_SET_ITEM(pyArgs, 24, PyFloat_FromDouble(pInitData->tccData.blur));
    /* syl info */
    PyTuple_SET_ITEM(pyArgs, 25, PyLong_FromLong(pInitData->sylData.lines));
    py_line1D = PyTuple_New(pInitData->sylData.lines);
    for (i = 0; i < pInitData->sylData.lines; i ++)
        PyTuple_SET_ITEM(py_line1D, i, PyUnicode_FromUnicode((const Py_UNICODE *)pInitData->sylData.line1D[i], wcslen(pInitData->sylData.line1D[i])));
    PyTuple_SET_ITEM(pyArgs, 26, py_line1D);
    py_begTime1D = PyTuple_New(pInitData->sylData.lines);
    for (i = 0; i < pInitData->sylData.lines; i ++)
        PyTuple_SET_ITEM(py_begTime1D, i, PyLong_FromLong(pInitData->sylData.begTime1D[i]));
    PyTuple_SET_ITEM(pyArgs, 27, py_begTime1D);
    py_endTime1D = PyTuple_New(pInitData->sylData.lines);
    for (i = 0; i < pInitData->sylData.lines; i ++)
        PyTuple_SET_ITEM(py_endTime1D, i, PyLong_FromLong(pInitData->sylData.endTime1D[i]));
    PyTuple_SET_ITEM(pyArgs, 28, py_endTime1D);
    py_count1D = PyTuple_New(pInitData->sylData.lines);
    for (i = 0; i < pInitData->sylData.lines; i ++)
        PyTuple_SET_ITEM(py_count1D, i, PyLong_FromLong(pInitData->sylData.count1D[i]));
    PyTuple_SET_ITEM(pyArgs, 29, py_count1D);
    py_time2D = PyTuple_New(pInitData->sylData.lines);
    count = 0;
    for (i = 0; i < pInitData->sylData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->sylData.count1D[i]);
        for (j = 0; j < pInitData->sylData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->sylData.time2D[count ++]));
        PyTuple_SET_ITEM(py_time2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 30, py_time2D);
    py_timeDiff2D = PyTuple_New(pInitData->sylData.lines);
    count = 0;
    for (i = 0; i < pInitData->sylData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->sylData.count1D[i]);
        for (j = 0; j < pInitData->sylData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->sylData.timeDiff2D[count ++]));
        PyTuple_SET_ITEM(py_timeDiff2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 31, py_timeDiff2D);
    py_text2D = PyTuple_New(pInitData->sylData.lines);
    count = 0;
    for (i = 0; i < pInitData->sylData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->sylData.count1D[i]);
        for (j = 0; j < pInitData->sylData.count1D[i]; j ++) {
            PyTuple_SET_ITEM(pyTemp, j, PyUnicode_FromUnicode((const Py_UNICODE *)pInitData->sylData.text2D[count], wcslen(pInitData->sylData.text2D[count])));
            count ++;
        }
        PyTuple_SET_ITEM(py_text2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 32, py_text2D);
    /* tm info */
    PyTuple_SET_ITEM(pyArgs, 33, PyLong_FromLong(pInitData->tmHoriData.ascender));
    PyTuple_SET_ITEM(pyArgs, 34, PyLong_FromLong(pInitData->tmHoriData.descender));
    /* horizontal */
    py_widthH2D = PyTuple_New(pInitData->tmHoriData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmHoriData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmHoriData.count1D[i]);
        for (j = 0; j < pInitData->tmHoriData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmHoriData.width2D[count ++]));
        PyTuple_SET_ITEM(py_widthH2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 35, py_widthH2D);
    py_heightH2D = PyTuple_New(pInitData->tmHoriData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmHoriData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmHoriData.count1D[i]);
        for (j = 0; j < pInitData->tmHoriData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmHoriData.height2D[count ++]));
        PyTuple_SET_ITEM(py_heightH2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 36, py_heightH2D);
    py_kerningH2D = PyTuple_New(pInitData->tmHoriData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmHoriData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmHoriData.count1D[i]);
        for (j = 0; j < pInitData->tmHoriData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmHoriData.kerning2D[count ++]));
        PyTuple_SET_ITEM(py_kerningH2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 37, py_kerningH2D);
    py_advanceH2D = PyTuple_New(pInitData->tmHoriData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmHoriData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmHoriData.count1D[i]);
        for (j = 0; j < pInitData->tmHoriData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmHoriData.advance2D[count ++]));
        PyTuple_SET_ITEM(py_advanceH2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 38, py_advanceH2D);
    py_advanceDiffH2D = PyTuple_New(pInitData->tmHoriData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmHoriData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmHoriData.count1D[i]);
        for (j = 0; j < pInitData->tmHoriData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmHoriData.advanceDiff2D[count ++]));
        PyTuple_SET_ITEM(py_advanceDiffH2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 39, py_advanceDiffH2D);
    py_lengthH1D = PyTuple_New(pInitData->tmHoriData.lines);
    for (i = 0; i < pInitData->tmHoriData.lines; i ++)
        PyTuple_SET_ITEM(py_lengthH1D, i, PyLong_FromLong(pInitData->tmHoriData.length1D[i]));
    PyTuple_SET_ITEM(pyArgs, 40, py_lengthH1D);
    py_initXH2D = PyTuple_New(pInitData->tmHoriData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmHoriData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmHoriData.count1D[i]);
        for (j = 0; j < pInitData->tmHoriData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmHoriData.initX2D[count ++]));
        PyTuple_SET_ITEM(py_initXH2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 41, py_initXH2D);
    py_initYH2D = PyTuple_New(pInitData->tmHoriData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmHoriData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmHoriData.count1D[i]);
        for (j = 0; j < pInitData->tmHoriData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmHoriData.initY2D[count ++]));
        PyTuple_SET_ITEM(py_initYH2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 42, py_initYH2D);
    py_bearingYH2D = PyTuple_New(pInitData->tmHoriData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmHoriData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmHoriData.count1D[i]);
        for (j = 0; j < pInitData->tmHoriData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmHoriData.bearingY2D[count ++]));
        PyTuple_SET_ITEM(py_bearingYH2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 43, py_bearingYH2D);
    /* vertical */
    py_widthV2D = PyTuple_New(pInitData->tmVertData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmVertData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmVertData.count1D[i]);
        for (j = 0; j < pInitData->tmVertData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmVertData.width2D[count ++]));
        PyTuple_SET_ITEM(py_widthV2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 44, py_widthV2D);
    py_heightV2D = PyTuple_New(pInitData->tmVertData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmVertData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmVertData.count1D[i]);
        for (j = 0; j < pInitData->tmVertData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmVertData.height2D[count ++]));
        PyTuple_SET_ITEM(py_heightV2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 45, py_heightV2D);
    py_kerningV2D = PyTuple_New(pInitData->tmVertData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmVertData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmVertData.count1D[i]);
        for (j = 0; j < pInitData->tmVertData.count1D[i]; j ++) {
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(0));
            count ++;
        }
        PyTuple_SET_ITEM(py_kerningV2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 46, py_kerningV2D);
    py_advanceV2D = PyTuple_New(pInitData->tmVertData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmVertData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmVertData.count1D[i]);
        for (j = 0; j < pInitData->tmVertData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmVertData.advance2D[count ++]));
        PyTuple_SET_ITEM(py_advanceV2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 47, py_advanceV2D);
    py_advanceDiffV2D = PyTuple_New(pInitData->tmVertData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmVertData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmVertData.count1D[i]);
        for (j = 0; j < pInitData->tmVertData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmVertData.advanceDiff2D[count ++]));
        PyTuple_SET_ITEM(py_advanceDiffV2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 48, py_advanceDiffV2D);
    py_lengthV1D = PyTuple_New(pInitData->tmVertData.lines);
    for (i = 0; i < pInitData->tmVertData.lines; i ++)
        PyTuple_SET_ITEM(py_lengthV1D, i, PyLong_FromLong(pInitData->tmVertData.length1D[i]));
    PyTuple_SET_ITEM(pyArgs, 49, py_lengthV1D);
    py_initXV2D = PyTuple_New(pInitData->tmVertData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmVertData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmVertData.count1D[i]);
        for (j = 0; j < pInitData->tmVertData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmVertData.initX2D[count ++]));
        PyTuple_SET_ITEM(py_initXV2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 50, py_initXV2D);
    py_initYV2D = PyTuple_New(pInitData->tmVertData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmVertData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmVertData.count1D[i]);
        for (j = 0; j < pInitData->tmVertData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmVertData.initY2D[count ++]));
        PyTuple_SET_ITEM(py_initYV2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 51, py_initYV2D);
    py_bearingXV2D = PyTuple_New(pInitData->tmVertData.lines);
    count = 0;
    for (i = 0; i < pInitData->tmVertData.lines; i ++) {
        pyTemp = PyTuple_New(pInitData->tmVertData.count1D[i]);
        for (j = 0; j < pInitData->tmVertData.count1D[i]; j ++)
            PyTuple_SET_ITEM(pyTemp, j, PyLong_FromLong(pInitData->tmVertData.bearingX2D[count ++]));
        PyTuple_SET_ITEM(py_bearingXV2D, i, pyTemp);
    }
    PyTuple_SET_ITEM(pyArgs, 52, py_bearingXV2D);
    /*****/
    pyDataFunc = PyObject_GetAttrString(pTcaxPy->pyBaseModule, TCAXPY_PY_INIT_DATA);
    if (!pyDataFunc) {
        Py_CLEAR(pyArgs);
        Py_CLEAR(pTcaxPy->pyBaseModule);
        PyErr_Print();
        /* PyErr_Clear(); */
        return py_error_init_fail;
    } else if (!PyCallable_Check(pyDataFunc)) {
        Py_CLEAR(pyArgs);
        Py_CLEAR(pyDataFunc);
        Py_CLEAR(pTcaxPy->pyBaseModule);
        return py_error_init_fail;
    }
    pyData = PyTuple_New(1);
    PyTuple_SET_ITEM(pyData, 0, pyArgs);
    if (PyObject_CallObject(pyDataFunc, pyData) == NULL) {
        Py_CLEAR(pyArgs);
        Py_CLEAR(pyData);
        Py_CLEAR(pyDataFunc);
        Py_CLEAR(pTcaxPy->pyBaseModule);
        PyErr_Print();
        /* PyErr_Clear(); */
        return py_error_init_fail;
    }
    Py_CLEAR(pyData);
    Py_CLEAR(pyDataFunc);
    pTcaxPy->pyUserModule = NULL;
    pTcaxPy->pyInitFunc = NULL;
    pTcaxPy->pyUserFunc = NULL;
    pTcaxPy->pyMainFunc = NULL;
    pTcaxPy->pyFinFunc = NULL;
    return py_error_success;
}

static void _tcaxpy_get_py_module_name_and_dir(const char *pyFilename, char **pPyModuleName, char **pPyModuleDir) {
    int i, indicator, len, count, flag;
    char *pyModuleName;
    char *pyModuleDir;
    flag = 0;
    count = 0;
    indicator = 0;
    len = strlen(pyFilename);
    for (i = len - 1; i >= 0; i --) {
        if (0 == flag && '.' == pyFilename[i]) flag = 1;
        else if (1 == flag) {
            if ('\\' == pyFilename[i] || '/' == pyFilename[i]) {
                indicator = i;
                break;
            } else count ++;
        }
    }
    if (0 == count) {
        *pPyModuleName = NULL;
        *pPyModuleDir = NULL;
    } else if (0 == indicator) {
        pyModuleName = (char *)malloc((count + 1) * sizeof(char));
        memcpy(pyModuleName, pyFilename, count * sizeof(char));
        pyModuleName[count] = '\0';
        *pPyModuleName = pyModuleName;
        *pPyModuleDir = NULL;
    } else {
        pyModuleName = (char *)malloc((count + 1) * sizeof(char));
        memcpy(pyModuleName, pyFilename + indicator + 1, count * sizeof(char));
        pyModuleName[count] = '\0';
        *pPyModuleName = pyModuleName;
        count = indicator;
        pyModuleDir = (char *)malloc((count + 1) * sizeof(char));
        memcpy(pyModuleDir, pyFilename, count * sizeof(char));
        pyModuleDir[count] = '\0';
        *pPyModuleDir = pyModuleDir;
    }
}

static int _tcaxpy_is_specified_py_module_existed(const char *filename) {
    int len;
    char *pycFilename;
    len = strlen(filename);
    pycFilename = (char *)malloc((len + 2) * sizeof(char));
    memcpy(pycFilename, filename, len * sizeof(char));
    pycFilename[len] = 'c';
    pycFilename[len + 1] = '\0';
#if defined WIN32
    if (GetFileAttributesA(filename) == INVALID_FILE_ATTRIBUTES && GetFileAttributesA(pycFilename) == INVALID_FILE_ATTRIBUTES) {
        free(pycFilename);
        return 0;
    }
#endif  /* WIN32 */
    free(pycFilename);
    return 1;
}

static int _tcaxpy_is_py_module_existed(const char *userPyFilename, const char *directory, const char *pyModuleName, char **pPyFilename) {
    int   dir_len, name_len, lib_len, count, existed;
    char *pyFilename;
    existed = _tcaxpy_is_specified_py_module_existed(userPyFilename);  /* checks both py and pyc (if one exists then returns 1) */
    if (existed) {
        count = strlen(userPyFilename) + 1;
        pyFilename = (char *)malloc(count * sizeof(char));
        memcpy(pyFilename, userPyFilename, count * sizeof(char));
        *pPyFilename = pyFilename;
        return 1;
    }
    dir_len = strlen(directory);
    name_len = strlen(pyModuleName);
    count = dir_len + 1 + name_len + 4;
    pyFilename = (char *)malloc(count * sizeof(char));
    memcpy(pyFilename, directory, dir_len * sizeof(char));
    pyFilename[dir_len] = '\\';
    memcpy(pyFilename + dir_len + 1, pyModuleName, name_len * sizeof(char));
    memcpy(pyFilename + dir_len + 1 + name_len, ".py\0", 4 * sizeof(char));
    existed = _tcaxpy_is_specified_py_module_existed(pyFilename);    /* if the py module is not found in the tcc file's directory then check Program's root dir */
    if (existed) {
        *pPyFilename = pyFilename;
        return 1;
    }
    free(pyFilename);
    lib_len = strlen(TCAXPY_PY_LIB_FOLDER);
    count = dir_len + 1 + lib_len + 1 + name_len + 4;
    pyFilename = (char *)malloc(count * sizeof(char));
    memcpy(pyFilename, directory, dir_len * sizeof(char));
    pyFilename[dir_len] = '\\';
    memcpy(pyFilename + dir_len + 1, TCAXPY_PY_LIB_FOLDER, lib_len * sizeof(char));
    pyFilename[dir_len + 1 + lib_len] = '\\';
    memcpy(pyFilename + dir_len + 1 + lib_len + 1, pyModuleName, name_len * sizeof(char));
    memcpy(pyFilename + dir_len + 1 + lib_len + 1 + name_len, ".py\0", 4 * sizeof(char));
    existed = _tcaxpy_is_specified_py_module_existed(pyFilename);    /* if the py module is not found in the tcc file's directory then check Program's root\Lib dir */
    if (existed) {
        *pPyFilename = pyFilename;
        return 1;
    }
    free(pyFilename);
    *pPyFilename = NULL;
    return 0;
}

/* The search path of tcaxPy base module is the TCAX's root directory, 
   The search paths of user's tcaxPy scripts are the TCAX's root directory and the specified tcaxPy scripts' directories */
static void _tcaxpy_make_user_py_module_path(const char *pyModuleDir, const char *directory, char **pPyModulePath) {
    int mod_len, dir_len, lib_len, count;
    char *pyModulePath;
    mod_len = strlen(pyModuleDir);
    dir_len = strlen(directory);
    lib_len = strlen(TCAXPY_PY_LIB_FOLDER);
    count = mod_len + 1 + dir_len + 1 + dir_len + 1 + lib_len + 1;
    pyModulePath = (char *)malloc(count * sizeof(char));
    memcpy(pyModulePath, pyModuleDir, mod_len * sizeof(char));
    pyModulePath[mod_len] = ';';
    memcpy(pyModulePath + mod_len + 1, directory, dir_len * sizeof(char));
    pyModulePath[mod_len + 1 + dir_len] = ';';
    memcpy(pyModulePath + mod_len + 1 + dir_len + 1, directory, dir_len * sizeof(char));
    pyModulePath[mod_len + 1 + dir_len + 1 + dir_len] = '\\';
    memcpy(pyModulePath + mod_len + 1 + dir_len + 1 + dir_len + 1, TCAXPY_PY_LIB_FOLDER, lib_len * sizeof(char));
    pyModulePath[mod_len + 1 + dir_len + 1 + dir_len + 1 + lib_len] = '\0';
    *pPyModulePath = pyModulePath;
}

static void _tcaxpy_fin_user_py_module(const PY_pTcaxPy pTcaxPy) {
    Py_CLEAR(pTcaxPy->pyInitFunc);
    Py_CLEAR(pTcaxPy->pyUserFunc);
    Py_CLEAR(pTcaxPy->pyMainFunc);
    Py_CLEAR(pTcaxPy->pyFinFunc);
    Py_CLEAR(pTcaxPy->pyUserModule);
    pTcaxPy->pyInitFunc = NULL;
    pTcaxPy->pyUserFunc = NULL;
    pTcaxPy->pyMainFunc = NULL;
    pTcaxPy->pyFinFunc = NULL;
    pTcaxPy->pyUserModule = NULL;
}

PY_Error_Code tcaxpy_init_user_py_module(PY_pTcaxPy pTcaxPy, const char *directory, const char *userPyFilename, int use_tcaxpy_init_func, int use_tcaxpy_user_func, int use_tcaxpy_fin_func) {
    PyObject *pyModule;
    char *pyModuleName;
    char *pyModuleDir;
    char *pyFilename;
    char *tokenFilename;
    char *pyModulePath;
    wchar_t *wcsPyModulePath;
    _tcaxpy_fin_user_py_module(pTcaxPy);
    _tcaxpy_get_py_module_name_and_dir(userPyFilename, &pyModuleName, &pyModuleDir);
    if (!pyModuleName) return py_error_null_pointer;
    if (!_tcaxpy_is_py_module_existed(userPyFilename, directory, pyModuleName, &pyFilename)) {
        free(pyModuleName);
        free(pyModuleDir);
        return py_error_file_not_found;
    }
    tcaxpy_convert_file_to_utf8(pyFilename);
    _tcaxpy_make_user_py_module_path(pyModuleDir, directory, &pyModulePath);
    free(pyModuleDir);
    _tcaxpy_sz_ansi_to_unicode(pyModulePath, &wcsPyModulePath);
    free(pyModulePath);
    PySys_SetPath(wcsPyModulePath);
    free(wcsPyModulePath);
    pyModule = PyImport_ImportModuleNoBlock(pyModuleName);
    free(pyModuleName);
    if (!pyModule) {
        PyErr_Print();
        /* PyErr_Clear(); */
        return py_error_init_fail;
    }
    tcaxpy_make_py_token_filename(pyFilename, &tokenFilename);
    if (tcaxpy_is_py_modified(pyFilename, tokenFilename) == py_error_file_modified) {    /* check if we need to reload the module */
        free(pyFilename);
        free(tokenFilename);
        pTcaxPy->pyUserModule = PyImport_ReloadModule(pyModule);  // refresh the module
        Py_CLEAR(pyModule);
        if (!pTcaxPy->pyUserModule) {
            PyErr_Print();
            /* PyErr_Clear(); */
            return py_error_init_fail;
        }
    } else {
        free(pyFilename);
        free(tokenFilename);
        pTcaxPy->pyUserModule = pyModule;
    }
    /* tcaxPy_Init (Alternative) */
    if (use_tcaxpy_init_func) {
        pTcaxPy->pyInitFunc = PyObject_GetAttrString(pTcaxPy->pyUserModule, TCAXPY_PY_FUNC_INIT);
        if (!pTcaxPy->pyInitFunc) {
            _tcaxpy_fin_user_py_module(pTcaxPy);
            PyErr_Print();
            /* PyErr_Clear(); */
            return py_error_init_fail;
        } else if (!PyCallable_Check(pTcaxPy->pyInitFunc)) {
            _tcaxpy_fin_user_py_module(pTcaxPy);
            return py_error_init_fail;
        }
    }
    /* tcaxPy_Main or tcaxPy_User */
    if (use_tcaxpy_user_func) {
        pTcaxPy->pyUserFunc = PyObject_GetAttrString(pTcaxPy->pyUserModule, TCAXPY_PY_FUNC_USER);
        if (!pTcaxPy->pyUserFunc) {
            _tcaxpy_fin_user_py_module(pTcaxPy);
            PyErr_Print();
            /* PyErr_Clear(); */
            return py_error_init_fail;
        } else if (!PyCallable_Check(pTcaxPy->pyUserFunc)) {
            _tcaxpy_fin_user_py_module(pTcaxPy);
            return py_error_init_fail;
        }
    } else {
        pTcaxPy->pyMainFunc = PyObject_GetAttrString(pTcaxPy->pyUserModule, TCAXPY_PY_FUNC_MAIN);
        if (!pTcaxPy->pyMainFunc) {
            _tcaxpy_fin_user_py_module(pTcaxPy);
            PyErr_Print();
            /* PyErr_Clear(); */
            return py_error_init_fail;
        } else if (!PyCallable_Check(pTcaxPy->pyMainFunc)) {
            _tcaxpy_fin_user_py_module(pTcaxPy);
            return py_error_init_fail;
        }
    }
    /* tcaxPy_Fin (Alternative) */
    if (use_tcaxpy_fin_func) {
        pTcaxPy->pyFinFunc = PyObject_GetAttrString(pTcaxPy->pyUserModule, TCAXPY_PY_FUNC_FIN);
        if (!pTcaxPy->pyFinFunc) {
            _tcaxpy_fin_user_py_module(pTcaxPy);
            PyErr_Print();
            /* PyErr_Clear(); */
            return py_error_init_fail;
        } else if (!PyCallable_Check(pTcaxPy->pyFinFunc)) {
            _tcaxpy_fin_user_py_module(pTcaxPy);
            return py_error_init_fail;
        }
    }
    return py_error_success;
}

PY_Error_Code tcaxpy_script_func_init(const PY_pTcaxPy pTcaxPy) {
    if (PyObject_CallObject(pTcaxPy->pyInitFunc, NULL) == NULL) {
        _tcaxpy_fin_user_py_module(pTcaxPy);
        PyErr_Print();
        /* PyErr_Clear(); */
        return py_error_exec_fail;
    }
    return py_error_success;
}

PY_Error_Code tcaxpy_script_func_user(const PY_pTcaxPy pTcaxPy) {
    if (PyObject_CallObject(pTcaxPy->pyUserFunc, NULL) == NULL) {
        _tcaxpy_fin_user_py_module(pTcaxPy);
        PyErr_Print();
        /* PyErr_Clear(); */
        return py_error_exec_fail;
    }
    return py_error_success;
}

void **tcaxpy_script_func_main(const PY_pTcaxPy pTcaxPy, int iLine, int iText, int nTexts, int start, int end, int timeDiff, int time, int x, int y, int advance, const wchar_t *text) {
    PyObject *pyArgs;          /* params to user tcax py script's tcaxPy_Main function */
    PyObject *pyReturnedBuf;   /* the value returned from tcaxPy_Main function */
    void    **pBufToReturn;    /* value return to tcax module */
    PyObject *pyAssList;
    PyObject *pyTcasList;
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
    PyTuple_SetItem(pyArgs, 10, PyUnicode_FromUnicode((const Py_UNICODE *)text, wcslen(text)));
    pyReturnedBuf = PyObject_CallObject(pTcaxPy->pyMainFunc, pyArgs);
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
                pAssLine[i] = (wchar_t *)PyUnicode_AS_UNICODE(PyList_GET_ITEM(pyAssList, i));
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
    pyTcasList = PyTuple_GET_ITEM(pyReturnedBuf, 1);
    if (PyList_Check(pyTcasList)) {
        int i, count;
        PyObject *pyTcasItem;
        short x, y;
        unsigned long RGB;
        unsigned char alpha;
        unsigned long *tcasBuf;
        count = PyList_GET_SIZE(pyTcasList);
        if (0 == count) {
            pBufToReturn[2] = (void *)0;
            pBufToReturn[3] = NULL;
        }
        else {
            /* tcas buffer from user tcax py script: (Start, End, Layer_Type_Pair, PosX, PosY, RGB, Alpha) */
            /* a raw tcas chunk defined in TCAS file format specification: (Start, End, Layer_Type_Pair, Pos, RGBA) */
            tcasBuf = (unsigned long *)malloc(count * 5 * sizeof(unsigned long));
            for (i = 0; i < count; i ++) {
                pyTcasItem = PyList_GET_ITEM(pyTcasList, i);
                x = (short)PyLong_AsLong(PyTuple_GET_ITEM(pyTcasItem, 3));
                y = (short)PyLong_AsLong(PyTuple_GET_ITEM(pyTcasItem, 4));
                RGB = (unsigned long)PyLong_AsUnsignedLong(PyTuple_GET_ITEM(pyTcasItem, 5));
                alpha = (unsigned char)PyLong_AsLong(PyTuple_GET_ITEM(pyTcasItem, 6));
                tcasBuf[5 * i + 0] = (long)PyLong_AsLong(PyTuple_GET_ITEM(pyTcasItem, 0));    /* Start */
                tcasBuf[5 * i + 1] = (long)PyLong_AsLong(PyTuple_GET_ITEM(pyTcasItem, 1));    /* End */
                tcasBuf[5 * i + 2] = (unsigned long)PyLong_AsUnsignedLong(PyTuple_GET_ITEM(pyTcasItem, 2));    /* Layer_Type_Pair */
                tcasBuf[5 * i + 3] = (x | ((unsigned long)y << 16));
                tcasBuf[5 * i + 4] = (RGB | ((unsigned long)alpha << 24));
            }
            pBufToReturn[2] = (void *)count;
            pBufToReturn[3] = tcasBuf;
        }
    } else {
        pBufToReturn[2] = (void *)0;
        pBufToReturn[3] = NULL;
    }
    Py_CLEAR(pyReturnedBuf);
    return pBufToReturn;
}

PY_Error_Code tcaxpy_script_func_fin(const PY_pTcaxPy pTcaxPy) {
    if (PyObject_CallObject(pTcaxPy->pyFinFunc, NULL) == NULL) {
        _tcaxpy_fin_user_py_module(pTcaxPy);
        PyErr_Print();
        /* PyErr_Clear(); */
        return py_error_exec_fail;
    }
    return py_error_success;
}

void tcaxpy_fin_tcaxpy(PY_pTcaxPy pTcaxPy) {
    Py_CLEAR(pTcaxPy->pyInitFunc);
    Py_CLEAR(pTcaxPy->pyUserFunc);
    Py_CLEAR(pTcaxPy->pyMainFunc);
    Py_CLEAR(pTcaxPy->pyFinFunc);
    Py_CLEAR(pTcaxPy->pyUserModule);
    Py_CLEAR(pTcaxPy->pyBaseModule);
}

