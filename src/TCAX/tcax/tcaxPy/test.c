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

int main() {
    //FILE *fp;
    const char *filename = "part1.py";
    //const wchar_t *tokenFilename = L"test.py.token";
    if (tcaxpy_init_python() != py_error_success) return -1;
    /*fp = fopen("tttttttt.py", "r");
    if (PyRun_SimpleFile(fp, "tttttttt.py") == 0) {
        PyErr_Print();
    }*/
    //if (tcaxpy_convert_file_to_utf8(filename) != py_error_success) return -1;
    if (tcaxpy_exec_py_script(filename) != py_error_success) return -1;
    /*if (tcaxpy_create_py_token(tcaxpy_make_py_token_filename(filename, &tokenFilename)) != py_error_success) {
        free(tokenFilename);
        return -1;
    }
    /if (tcaxpy_synchronize_py_token(filename, tokenFilename) != py_error_success) return -1;
    if (tcaxpy_is_py_modified(filename, tokenFilename) == py_error_file_modified) {
        printf("Haha");
    }*/
    if (tcaxpy_fin_python() != py_error_success) return -1;
    printf("Good!");
    return 0;
}

