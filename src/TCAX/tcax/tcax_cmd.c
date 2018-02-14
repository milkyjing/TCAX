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

#include "tcax.h"

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif    /* _MSC_VER */

/* Added at 2012-07-06 22:35 to solve the R6034 issue brought in by the site-packages of Python 
 * e.g., PyOpenGL which requires msvcr90.dll to run
 */
#pragma comment(linker, "/manifestdependency:\"type='win32' "                           \
                                              "name='Microsoft.VC90.CRT' "              \
                                              "version='9.0.21022.8' "                  \
                                              "processorArchitecture='x86' "            \
                                              "publicKeyToken='1fc8b3b9a1e18e3b'\"")

#define TCAX_CMD_EXIT         'e'
#define TCAX_CMD_PARSE_TCC    '1'
#define TCAX_CMD_DEFAULT_PY   '2'
#define TCAX_CMD_DEFAULT_TCC  '3'
#define TCAX_CMD_EXEC_PY      'i'
#define TCAX_CMD_MENU         'm'


void printMenu() {
    printf("\n\n");
    printf("----------------- MENU -----------------\n");
    printf("1 - parse a TCC file\n");
    printf("2 - create a default tcaxPy script template\n");
    printf("3 - create a default TCC file\n");
    printf("m - show the menu again\n");
    printf("e - exit TCAX\n");
    printf("\n\n");
}


int main(int argc, char *argv[]) {
    DWORD executionTime;
    int len, tcaxCmd;
    char szFilename[1024];
    char *outFilename;
    printf("TCAX version 1.2.0 [2012-08-03] [Anniversary]\n");
    printf("(C) Copyright 2009-2012 milkyjing\n\n");
    if (tcaxpy_init_python() != py_error_success) return -1;
    if (2 == argc) {
        executionTime = GetTickCount();
        tcax_make_out_filename_from_tcc(argv[1], &outFilename);
        if (tcax_entry(argv[1], outFilename) != 0) {
            free(outFilename);
            printf("INFO: Task failed!\n");
            printf("\n\n温馨提醒: 如果不清楚造成本错误的原因, 请保留本窗口截图以及当前特效工程, \n访问 http://tcax.rhacg.com/forum.php?mod=forumdisplay&fid=38 发帖以获取帮助.\n\n");
        } else {
            free(outFilename);
            printf("INFO: Task has been completed!\n");
            printf("INFO: execution duration is %i seconds\n", (GetTickCount() - executionTime) / 1000);
        }
        tcaxpy_fin_python();
        system("PAUSE");
        return 0;
    }
    printMenu();
    while (1) {
        printf("Please enter a number: ");
        tcaxCmd = getchar();
        if ('\n' != tcaxCmd)
            while ('\n' != getchar()) continue;    /* read the '\n' character brought in by the enter key to avoid the gets function to read it */
        if (TCAX_CMD_EXIT == tcaxCmd) {
            printf("Bye, Bye!\n");
            break;
        } else if (TCAX_CMD_PARSE_TCC == tcaxCmd) {
            do {
                printf("Please enter the TCC filename: ");
                fgets(szFilename, sizeof(szFilename), stdin);
                len = strlen(szFilename);
                szFilename[len - 1] = '\0';
                len--;
            }
            while (!(len > 4 && __str_ignore_case_cmp(szFilename + len - 4, ".tcc") == 0));
            executionTime = GetTickCount();
            tcax_make_out_filename_from_tcc(szFilename, &outFilename);
            if (tcax_entry(szFilename, outFilename) != 0) {
                free(outFilename);
                printf("INFO: Task failed!\n\n");
                printf("\n\n温馨提醒: 如果不清楚造成本错误的原因, 请保留本窗口截图以及当前特效工程, \n访问 http://tcax.rhacg.com/forum.php?mod=forumdisplay&fid=38 发帖以获取帮助.\n\n");
            } else {
                free(outFilename);
                printf("INFO: Task has been completed!\n");
                printf("INFO: execution duration is %i seconds\n\n", (GetTickCount() - executionTime) / 1000);
            }
        } else if (TCAX_CMD_DEFAULT_PY == tcaxCmd) {
            do {
                printf("Please enter the tcaxPy script filename: ");
                fgets(szFilename, sizeof(szFilename), stdin);
                len = strlen(szFilename);
                szFilename[len - 1] = '\0';
                len--;
            }
            while (!(len > 3 && __str_ignore_case_cmp(szFilename + len - 3, ".py") == 0));
            if (tcaxpy_create_py_template(szFilename) != tcc_error_success)
                printf("INFO: Task failed!\n\n");
            else
                printf("INFO: Task has been completed!\n\n");
        } else if (TCAX_CMD_DEFAULT_TCC == tcaxCmd) {
            do {
                printf("Please enter the TCC filename: ");
                fgets(szFilename, sizeof(szFilename), stdin);
                len = strlen(szFilename);
                szFilename[len - 1] = '\0';
                len--;
            }
            while (!(len > 4 && __str_ignore_case_cmp(szFilename + len - 4, ".tcc") == 0));
            if (libtcc_create_default_tcc_file(szFilename) != tcc_error_success)
                printf("INFO: Task failed!\n\n");
            else
                printf("INFO: Task has been completed!\n\n");
        } else if (TCAX_CMD_EXEC_PY == tcaxCmd) {
            do {
                printf("Please enter the PY filename: ");
                fgets(szFilename, sizeof(szFilename), stdin);
                len = strlen(szFilename);
                szFilename[len - 1] = '\0';
                len--;
            }
            while (!(len > 3 && __str_ignore_case_cmp(szFilename + len - 3, ".py") == 0));
            if (tcaxpy_exec_py_script(szFilename) != tcc_error_success)
                printf("INFO: Task failed!\n\n");
            else
                printf("INFO: Task has been done!\n\n");
        } else if (TCAX_CMD_MENU == tcaxCmd) {
            printMenu();
        } else {
            printf("INFO: Invalid command!\n\n");
        }
    }
    tcaxpy_fin_python();
    return 0;
}

