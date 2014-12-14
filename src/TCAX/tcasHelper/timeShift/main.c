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

#include "std.h"
#include "resource.h"


static void _W2A(const wchar_t *uni, char **ansi) {
    int size;
    char *sz;
    size = WideCharToMultiByte(CP_ACP, 0, uni, -1, NULL, 0, NULL, NULL);
    sz = (char *)malloc(size * sizeof(char));
    WideCharToMultiByte(CP_ACP, 0, uni, -1, sz, size, NULL, NULL);
    *ansi = sz;
}

static void tcasShowInfo(HWND hDlg, WCHAR *filename) {
    char *ansiFilename;
    TCAS_File file;
    TCAS_Header header;
    WCHAR wcs[1024];
    WCHAR *fileType;
    WCHAR *fileTypeRaw = L"raw";
    WCHAR *fileTypeCompressed = L"compressed";
    WCHAR *fileTypeCompressedZ = L"compressed z";
    WCHAR *fileTypeParsed = L"parsed";
    WCHAR *fileTypeParsedZ = L"parsed z";
    WCHAR *fileTypeUnknown = L"unknown";
    _W2A(filename, &ansiFilename);
    if (libtcas_open_file(&file, ansiFilename, tcas_file_open_existing) != tcas_error_success) {
        free(ansiFilename);
        wsprintf(wcs, L"Cannot open the TCAS file - \"%s\"", filename);
        MessageBox(hDlg, wcs, L"timeShift ERROR", MB_OK | MB_ICONINFORMATION);
    }
    free(ansiFilename);
    libtcas_read_header(&file, &header, tcas_false);
    switch (GETHI16B(header.flag)) {
    case TCAS_FILE_TYPE_RAW:
        fileType = fileTypeRaw;
        break;
    case TCAS_FILE_TYPE_COMPRESSED:
        fileType = fileTypeCompressed;
        break;
    case TCAS_FILE_TYPE_COMPRESSED_Z:
        fileType = fileTypeCompressedZ;
        break;
    case TCAS_FILE_TYPE_PARSED:
        fileType = fileTypeParsed;
        break;
    case TCAS_FILE_TYPE_PARSED_Z:
        fileType = fileTypeParsedZ;
        break;
    default:
        fileType = fileTypeUnknown;
        break;
    }
    wsprintf(wcs, L"type:\t%s\nwidth:\t%i\nheight:\t%i\nminTime:\t%i\nmaxTime:\t%i\nchunks:\t%i\nfpsNumerator:\t%i\nfpsDenominator:\t%i\n", 
        fileType, GETPOSX(header.resolution), GETPOSY(header.resolution), header.minTime, header.maxTime, header.chunks, header.fpsNumerator, header.fpsDenominator);
    MessageBox(hDlg, wcs, L"timeShift info", MB_OK | MB_ICONINFORMATION);
    libtcas_close_file(&file);
}

static void tcasShiftTime(HWND hDlg, WCHAR *filename, int iShiftTime, int saveCopy) {
    char *ansiFilename;
    WCHAR wcs[1024];
    if (saveCopy) {
        WCHAR name[1024];
        int len;
        len = wcslen(filename) - 5;
        memcpy(name, filename, len * sizeof(WCHAR));
        memcpy(name + len, L"_old.tcas\0", 10 * sizeof(WCHAR));
        CopyFile(filename, name, 0);
    }
    _W2A(filename, &ansiFilename);
    switch (libtcas_file_shift_time(ansiFilename, iShiftTime)) {
    case tcas_error_success:
        MessageBox(hDlg, L"Shifting time done successfully!", L"timeShift info", MB_OK | MB_ICONINFORMATION);
        break;
    case tcas_error_file_type_not_support:
        MessageBox(hDlg, L"Unknown file type!", L"timeShift info", MB_OK | MB_ICONINFORMATION);
        break;
    default:
        wsprintf(wcs, L"Cannot open the TCAS file - \"%s\"", filename);
        MessageBox(hDlg, wcs, L"timeShift error", MB_OK | MB_ICONINFORMATION);
    }
    free(ansiFilename);
}


// -------------------------------------------------------------------------------

#define TCAS_FILE         (WM_USER + 101)
#define TCAS_INFO         (WM_USER + 102)
#define TCAS_SHIFTTIME    (WM_USER + 103)
#define TCAS_EXIT         (WM_USER + 104)


LRESULT CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        SetFocus(GetDlgItem(hDlg, IDC_EDIT_FILE));
        CheckDlgButton(hDlg, IDC_RADIO_FORWARD, BST_CHECKED);
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_HOUR), L"0");
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_MIN), L"00");
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_SEC), L"00");
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_SS), L"000");
        break;
    case WM_SYSCOMMAND:
        switch (wParam) {
        case SC_CLOSE:
            PostMessage(hDlg, TCAS_EXIT, 0, 0);
            break;
        default:
            return DefWindowProc(hDlg, uMsg, wParam, lParam);
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BTM_FILE:
            PostMessage(hDlg, TCAS_FILE, 0, 0);
            break;
        case IDC_BTM_TCASINFO:
            PostMessage(hDlg, TCAS_INFO, 0, 0);
            break;
        case IDC_BTM_SHIFTTIME:
            PostMessage(hDlg, TCAS_SHIFTTIME, 0, 0);
            break;
        case IDC_EDIT_MIN:
            if (HIWORD(wParam) == EN_CHANGE) {
                WCHAR szMin[3];
                int iMin;
                GetWindowText((HWND)lParam, szMin, 3);
                iMin = wcstol(szMin, NULL, 10);
                if (iMin > 59) {
                    HWND hEditHour;
                    WCHAR szHour[2];
                    int iHour;
                    wsprintf(szMin, L"%02i", iMin - 60);
                    SetWindowText((HWND)lParam, szMin);
                    hEditHour = GetDlgItem(hDlg, IDC_EDIT_HOUR);
                    GetWindowText(hEditHour, szHour, 2);
                    iHour = wcstol(szHour, NULL, 10);
                    if (iHour + 1 < 10) {
                        wsprintf(szHour, L"%i", iHour + 1);
                        SetWindowText(hEditHour, szHour);
                    }
                }
            }
            break;
        case IDC_EDIT_SEC:
            if (HIWORD(wParam) == EN_CHANGE) {
                WCHAR szSec[3];
                int iSec;
                GetWindowText((HWND)lParam, szSec, 3);
                iSec = wcstol(szSec, NULL, 10);
                if (iSec > 59) {
                    HWND hEditMin;
                    WCHAR szMin[3];
                    int iMin;
                    wsprintf(szSec, L"%02i", iSec - 60);
                    SetWindowText((HWND)lParam, szSec);
                    hEditMin = GetDlgItem(hDlg, IDC_EDIT_MIN);
                    GetWindowText(hEditMin, szMin, 3);
                    iMin = wcstol(szMin, NULL, 10);
                    if (iMin + 1 > 59) {
                        HWND hEditHour;
                        WCHAR szHour[2];
                        int iHour;
                        wprintf(szMin, L"%02i", iMin - 59);    // iMin + 1 - 60
                        SetWindowText(hEditMin, szMin);
                        hEditHour = GetDlgItem(hDlg, IDC_EDIT_HOUR);
                        GetWindowText(hEditHour, szHour, 2);
                        iHour = wcstol(szHour, NULL, 10);
                        if (iHour + 1 < 10) {  // if iHour + 1 == 10 then it means that Edit Hour already == 9
                            wprintf(szHour, L"%i", iHour + 1);
                            SetWindowText(hEditHour, szHour);
                        }
                    } else {
                        wsprintf(szMin, L"%02i", iMin + 1);
                        SetWindowText(hEditMin, szMin);
                    }
                }
            }
            break;
        default:
            return DefWindowProc(hDlg, uMsg, wParam, lParam);
        }
        break;
    }
    return 0;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
    int          Argc;
    WCHAR      **Argv;
    HWND         hDlg;
    BOOL         bActive;
    MSG          Msg;
    OPENFILENAME ofn;
    static WCHAR szFile[1024];
    static WCHAR szFileTitle[1024];
    Argv = CommandLineToArgvW(GetCommandLine(), &Argc);
    hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, (DLGPROC)DlgProc);
    if (Argc > 1) {
        wcscpy(szFile, Argv[1]);
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE), szFile);
    }
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize    = sizeof(OPENFILENAME);
    ofn.hwndOwner      = hDlg;
    ofn.lpstrFilter    = L"tcas file(*.tcas)\0*.tcas\0\0";
    ofn.nFilterIndex   = 1;
    ofn.lpstrFile      = szFile;
    ofn.nMaxFile       = 1024;
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle  = 256;
    ofn.lpstrTitle     = L"Open";
    ofn.Flags          = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    // ----------------------
    bActive = 1;
    ZeroMemory(&Msg, sizeof(Msg));
    SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON)));
    ShowWindow(hDlg, SW_SHOWNORMAL);
    while (bActive) {
        if (GetMessage(&Msg, hDlg, 0, 0)) {
            if (TCAS_EXIT == Msg.message) {
                DestroyWindow(hDlg);
                bActive = 0;
            } else if (TCAS_FILE == Msg.message) {
                if (GetOpenFileName(&ofn))
                    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE), ofn.lpstrFile);
            } else if (TCAS_INFO == Msg.message) {
                GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE), szFile, 1024);
                if (wcslen(szFile) == 0) continue;
                tcasShowInfo(hDlg, szFile);
            } else if (TCAS_SHIFTTIME == Msg.message) {
                WCHAR szHour[2];
                WCHAR szMin[3];
                WCHAR szSec[3];
                WCHAR szSS[4];
                BOOL  bSaveCopy = 0;
                int   iShiftTime = 0;
                int   iHour, iMinute, iSecond, iSemiSecond;
                GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE), szFile, 1024);
                if (wcslen(szFile) == 0) continue;
                GetWindowText(GetDlgItem(hDlg, IDC_EDIT_HOUR), szHour, 2);
                GetWindowText(GetDlgItem(hDlg, IDC_EDIT_MIN), szMin, 3);
                GetWindowText(GetDlgItem(hDlg, IDC_EDIT_SEC), szSec, 3);
                GetWindowText(GetDlgItem(hDlg, IDC_EDIT_SS), szSS, 4);
                iHour       = wcstol(szHour, NULL, 10);
                iMinute     = wcstol(szMin, NULL, 10);
                iSecond     = wcstol(szSec, NULL, 10);
                iSemiSecond = wcstol(szSS, NULL, 10);
                iShiftTime = iHour * 60 * 60 * 1000 + iMinute * 60 * 1000 + iSecond * 1000 + iSemiSecond;
                if (SendMessage(GetDlgItem(hDlg, IDC_CHECK_SAVECOPY), BM_GETCHECK, 0, 0) == BST_CHECKED)
                    bSaveCopy = 1;
                if (SendMessage(GetDlgItem(hDlg, IDC_RADIO_BACK), BM_GETCHECK, 0, 0) == BST_CHECKED)
                    iShiftTime = -iShiftTime;
                tcasShiftTime(hDlg, szFile, iShiftTime, bSaveCopy);
            } else {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        }
    }
    return Msg.wParam;
}

