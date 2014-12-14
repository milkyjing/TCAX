#include "fontColor.h"


// function
BYTE ToByte(int i)
{
    if (i < 0)
    {
        return 0;
    }
    else if (i > 255)
    {
        return 255;
    }
    return i;
}

int HexToDec(CString strHex)
{
    int iLen = strHex.GetLength();
    if (iLen > 2 || iLen < 1)
    {
        return 0;
    }
    else if (iLen == 1)
    {
        if (strHex == _T("A") || strHex == _T("a"))
        {
            return 10;
        }
        else if (strHex == _T("B") || strHex == _T("b"))
        {
            return 11;
        }
        else if (strHex == _T("C") || strHex == _T("c"))
        {
            return 12;
        }
        else if (strHex == _T("D") || strHex == _T("d"))
        {
            return 13;
        }
        else if (strHex == _T("E") || strHex == _T("e"))
        {
            return 14;
        }
        else if (strHex == _T("F") || strHex == _T("f"))
        {
            return 15;
        }
        else
        {
            return ::_ttoi(strHex);
        }
    }
    else
    {
        strHex.MakeReverse();
        return HexToDec(strHex.Mid(1, 1)) * 16 + HexToDec(strHex.Mid(0, 1));
    }
}

CString TcHexColor(DWORD dwColor)
{
    CString strHex = _T("");
    while (dwColor != 0)
    {
        int iFlag = dwColor % 16;
        switch (iFlag)
        {
        case 10:
            strHex += _T("A");
            break;
        case 11:
            strHex += _T("B");
            break;
        case 12:
            strHex += _T("C");
            break;
        case 13:
            strHex += _T("D");
            break;
        case 14:
            strHex += _T("E");
            break;
        case 15:
            strHex += _T("F");
            break;
        default:
            CString strTemp;
            strTemp.Format(_T("%i"), dwColor % 16);
            strHex += strTemp;
            break;
        }
        dwColor /= 16;
    }
    strHex.MakeReverse();
    switch (strHex.GetLength())
    {
    case 0:
        strHex = _T("000000");
        break;
    case 1:
        strHex = _T("00000") + strHex;
        break;
    case 2:
        strHex = _T("0000") + strHex;
        break;
    case 3:
        strHex = _T("000") + strHex;
        break;
    case 4:
        strHex = _T("00") + strHex;
        break;
    case 5:
        strHex = _T("0") + strHex;
        break;
    }
    return strHex;
}

COLORREF TcDecColor(CString strColor)
{
    return RGB(    ::ToByte(::HexToDec(strColor.Mid(4, 2))), 
                ::ToByte(::HexToDec(strColor.Mid(2, 2))),
                ::ToByte(::HexToDec(strColor.Mid(0, 2))));
}

BOOL IsHexNumber(LPCTSTR str)
{
    int nLen = ::_tcslen(str);
    for (int i = 0; i < nLen; i ++)
    {
        if (str[i] < _T('0') || (str[i] > L'9' && str[i] < _T('A')) || (str[i] > _T('H') && str[i] < _T('a')) || str[i] > _T('h'))
        {
            return FALSE;
        }
    }
    return TRUE;
}


// TCCFrm
TCCFrm *TCCFrm::m_pFrm = NULL;

BOOL TCCFrm::FrmInit()
{
    m_hDesktopDC = ::GetDC(NULL);
    if (!m_hDesktopDC)
    {
        return FALSE;
    }
    m_hDlg = ::CreateDialog(m_hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, (DLGPROC)FrmDlgProc);
    m_hDC = ::GetDC(m_hDlg);
    if (!m_hDC)
    {
        return FALSE;
    }
    ::SendMessage(m_hDlg, WM_SETICON, ICON_SMALL, (LPARAM)::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_ICON_MAIN)));
    m_hColorDlg = ::CreateDialog(m_hInst, MAKEINTRESOURCE(IDD_DIALOG_COLOR), NULL, (DLGPROC)FrmColorDlgProc);
    m_hColorDC = ::GetDC(m_hColorDlg);
    if (!m_hColorDC)
    {
        return FALSE;
    }
    // ---
    RECT rc;
    ::GetWindowRect(m_hColorDlg, &rc);
    m_iClrDlgWd = rc.right - rc.left;
    m_iClrDlgHt = rc.bottom - rc.top;
    // ---
    if (::IsWindow(::FindWindowEx(NULL, NULL, WNDCLASS_NAME, NULL)))
    {
        ::SetWindowPos(m_hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
    }
    m_hDll = ::LoadLibrary(_T("fontColor_dll.dll"));
    if (!m_hDll)
    {
        ::MessageBox(m_hDlg, _T("Error: can't load fontColor's component - fontColor_dll.dll"), 
                        _T("fontColor - info"), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }
    m_pTccStartHook = (pTcc_StartHookFunc)::GetProcAddress(m_hDll, "StartMouseHook");
    if (!m_pTccStartHook)
    {
        return FALSE;
    }
    m_pTccEndHook = (pTcc_EndHookFunc)::GetProcAddress(m_hDll, "EndMouseHook");
    if (!m_pTccEndHook)
    {
        return FALSE;
    }
    m_hComboFN = ::GetDlgItem(m_hDlg, IDC_COMBO_FONT);
    m_hEditFaceID = ::GetDlgItem(m_hDlg, IDC_EDIT_FACE_ID);
    m_hEditFileName = ::GetDlgItem(m_hDlg, IDC_EDIT_FILE_NAME);
    m_hEditFaceName = ::GetDlgItem(m_hDlg, IDC_EDIT_FACE_NAME);
    m_hEditColor = ::GetDlgItem(m_hDlg, IDC_EDIT_COLOR);
    m_hCheckHideDlg = ::GetDlgItem(m_hDlg, IDC_CHECK_HIDE_DLG);
    m_hBtnCapClr = ::GetDlgItem(m_hDlg, IDC_BTN_CAPTURE_COLOR);
    this->FrmInitCc();
    if (!this->FrmListFonts())
    {
        ::MessageBox(m_hDlg, _T("Couldn't load fonts"), _T("fontColor - info"), MB_OK | MB_ICONINFORMATION);
        //return FALSE;    // we can still use color-features
    }
    ::SendMessage(m_hEditColor, WM_SETFONT, (WPARAM)::GetStockObject(SYSTEM_FIXED_FONT), FALSE);
    ::SetWindowText(m_hEditColor, _T("FFFFFF"));
    ::SetWindowText(m_hEditFaceID, _T("1"));
    ::CheckDlgButton(m_hDlg, IDC_CHECK_HIDE_DLG, BST_CHECKED);
    ::SetFocus(m_hComboFN);
    ::ShowWindow(m_hDlg, SW_SHOWNORMAL);
    return TRUE;
}

BOOL TCCFrm::FrmListFonts()
{
    HKEY  hKey;
    LONG  lRes;
    DWORD cbValues;
    DWORD cbSize = 256;
    TCHAR szFnBuf[256];
    CString strFilter;
    lRes = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts\\"), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS)
    {
        return FALSE;
    }
    ::RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &cbValues, NULL, NULL, NULL, NULL);
    for (UINT i = 0; i < cbValues; i ++)
    {
        cbSize = 256;
        ::RegEnumValue(hKey, i, szFnBuf, &cbSize, NULL, NULL, NULL, NULL);
        strFilter = szFnBuf;
        if (strFilter.Find(_T("(TrueType)")) != -1 || strFilter.Find(_T("(OpenType)")) != -1)
        {
            ::SendMessage(m_hComboFN, CB_ADDSTRING, 0, (LPARAM)szFnBuf);
        }
    }
    ::SendMessage(m_hComboFN, CB_SETCURSEL, 0, 0);
    return TRUE;
}

BOOL TCCFrm::FrmShowFont()
{
    // --- Get Face ID
    TCHAR szFCID[3];
    ::GetWindowText(m_hEditFaceID, szFCID, 3);
    // --- Get Font Name
    TCHAR szFontName[512];
    ::SendMessage(m_hComboFN, CB_GETLBTEXT, ::SendMessage(m_hComboFN, CB_GETCURSEL, 0, 0), (LPARAM)szFontName);
    // --- Set Font File
    LONG lResult;
    HKEY hKey;
    DWORD dwType;
    DWORD cbSize = 256;
    TCHAR szFileName[256];
    lResult = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts\\"), 0, KEY_READ, &hKey);
    if (lResult != ERROR_SUCCESS)
    {
        return FALSE;
    }
    ::RegQueryValueEx(hKey, szFontName, NULL, &dwType, (BYTE *)szFileName, &cbSize);
    ::SetWindowText(m_hEditFileName, szFileName);
    // --- Get Face Name
    UINT uID = ::_ttoi(szFCID);
    CString Str(szFontName);
    int iIdx = 0;
    vector<int> vIdx;
    vector<CString> vStr;
    int iFlag  = 0;
    int iFlag1 = Str.Find(_T("("), 0);  // Cut such as (TrueType) from the font name
    int iFlag2 = Str.Find(_T(")"), 0);  // Ensure safety: ie. 华康少女文字W5(P) (TrueType)
    while (iFlag1 != -1 && iFlag2 != -1 && iFlag2 - iFlag1 < 9)   // len "(TrueType)") = 10, len "(OpenType)") = 10, so 9 is safe
    {
        iFlag1 = Str.Find(_T("("), iFlag1 + 1);
        iFlag2 = Str.Find(_T(")"), iFlag2 + 1);
    }
    if (iFlag1 != -1)
    {
        Str = Str.Mid(0, iFlag1 - 1);
    }
    vIdx.push_back(-1);
    while (TRUE)
    {
        iFlag = Str.Find(_T("&"), vIdx[iIdx] + 1);
        if (iFlag == -1)
        {
            vIdx.push_back(Str.GetLength() + 1);
            if (iIdx == 0)
            {
                vStr.push_back(Str.Mid(vIdx[iIdx] + 1, vIdx[iIdx + 1] - vIdx[iIdx] - 2));
            }
            else
            {
                vStr.push_back(Str.Mid(vIdx[iIdx] + 2, vIdx[iIdx + 1] - vIdx[iIdx] - 3));
            }
            iIdx ++;
            break;
        }
        vIdx.push_back(Str.Find(_T("&"), vIdx[iIdx] + 1));
        if (iIdx == 0)
        {
            vStr.push_back(Str.Mid(vIdx[iIdx] + 1, vIdx[iIdx + 1] - vIdx[iIdx] - 2));
        }
        else
        {
            vStr.push_back(Str.Mid(vIdx[iIdx] + 2, vIdx[iIdx + 1] - vIdx[iIdx] - 3));
        }
        iIdx ++;
    }
    if (uID < 1)
    {
        if (::GetWindowTextLength(m_hEditFaceID) > 0)
        {
            ::SetWindowText(m_hEditFaceID, _T("1"));
            ::SetFocus(m_hEditFaceID);
        }
        uID = 1;
    }
    if (uID > vStr.size())
    {
        CString strTemp;
        strTemp.Format(_T("%i"), vStr.size());
        ::SetWindowText(m_hEditFaceID, strTemp);
        ::SetFocus(m_hEditFaceID);
        uID = vStr.size();
    }
    // ---
    RECT    rect;
    LOGFONT lf;
    HDC     hDC       = NULL;
    HFONT   hFont     = NULL;
    HFONT   hOldFont  = NULL;
    HBRUSH  hBrush    = NULL;
    HBRUSH  hOldBrush = NULL;
    ZeroMemory(&rect, sizeof(RECT));
    ZeroMemory(&lf, sizeof(LOGFONT));
    lf.lfHeight  = 40;
    lf.lfCharSet = DEFAULT_CHARSET;
    ::_tcscpy_s(lf.lfFaceName, -1, vStr[uID - 1]);
    ::SetWindowText(m_hEditFaceName, vStr[uID - 1]);    // set face name
    // --- Erase the last image
    hBrush = (HBRUSH)::GetStockObject(DC_BRUSH);
    hOldBrush = (HBRUSH)::SelectObject(m_hDC, hBrush);
    ::SetDCBrushColor(m_hDC, RGB(240, 240, 240));
    ::GetClientRect(m_hDlg, &rect);
    ::Rectangle(m_hDC, 30, 20, rect.right - 30, 80);
    ::SelectObject(m_hDC, hOldBrush);
    ::DeleteObject(hBrush);
    // --- Show the current font
    hFont = ::CreateFontIndirect(&lf);
    hOldFont = (HFONT)::SelectObject(m_hDC, hFont);
    ::SetBkColor(m_hDC, RGB(240, 240, 240));
    ::SetTextColor(m_hDC, RGB(0, 0, 0));
    ::TextOut(m_hDC, 40, 30, vStr[uID - 1], vStr[uID - 1].GetLength());
    ::SelectObject(m_hDC, hOldFont);
    ::DeleteObject(hFont);
    return TRUE;
}

CString TCCFrm::FrmGetFormatedColorFromEdit()
{
    TCHAR szColor[7];
    ::GetWindowText(m_hEditColor, szColor, 7);
    if (!IsHexNumber(szColor))
    {
        ::_tcscpy_s(szColor, -1, _T("FFFFFF"));
    }
    int iLen = ::_tcslen(szColor);
    CString strColor(szColor);
    strColor.MakeUpper();
    for (int i = 0; i < 6 - iLen; i ++)
    {
        strColor = _T("0") + strColor;
    }
    return strColor;
}

void TCCFrm::FrmInitCc()
{
    HKEY     hKey;
    DWORD    dwDisp;
    DWORD    dwType = 0;
    DWORD    cbData = 32;
    BYTE     Data[32];
    CString  strTemp;
    ZeroMemory(Data, 32 * sizeof(BYTE));
    if (::RegCreateKeyEx(    HKEY_CURRENT_USER, _T("Software\\tcax\\tccHelper\\fontColor\\CustomColors\\"), 
                            0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
                            NULL, &hKey, &dwDisp) == ERROR_SUCCESS)
    {
        if (REG_CREATED_NEW_KEY == dwDisp)
        {
            for (int i = 0; i < 16; i ++)
            {
                strTemp.Format(_T("Custom_Color_%02i"), i + 1);
                ::RegSetValueEx(hKey, strTemp, 0, REG_SZ, (const BYTE *)_T("FFFFFF"), 14);
            }
        }
        for (int i = 0; i < 16; i ++)
        {
            dwType = 0;
            cbData = 32;
            strTemp.Format(_T("Custom_Color_%02i"), i + 1);
            if (::RegQueryValueEx(hKey, strTemp, NULL, &dwType, Data, &cbData) == ERROR_SUCCESS)
            {
                m_crCustColors[i] = TcDecColor((LPCTSTR)Data);
                ZeroMemory(Data, 32 * sizeof(BYTE));
            }
            else
            {
                m_crCustColors[i] = 0xFFFFFF;
                ::RegSetValueEx(hKey, strTemp, 0, REG_SZ, (const BYTE *)_T("FFFFFF"), 14);
            }
        }
        ::RegCloseKey(hKey);
    }
    else
    {
        for (int i = 0; i < 16; i ++)
        {
            m_crCustColors[i] = 0xFFFFFF;
        }
    }
    // ---
    ZeroMemory(&m_cc, sizeof(CHOOSECOLOR));
    m_cc.lStructSize = sizeof(CHOOSECOLOR);
    m_cc.hwndOwner = m_hDlg;
    m_cc.hInstance = NULL;
    m_cc.rgbResult = m_crColor;
    m_cc.lpCustColors = m_crCustColors;
    m_cc.Flags = CC_RGBINIT | CC_FULLOPEN;
    m_cc.lCustData = 0;
    m_cc.lpfnHook = NULL;
    m_cc.lpTemplateName = NULL;
}

BOOL TCCFrm::FrmWriteCustColorsToReg()
{
    HKEY    hKey;
    DWORD   dwDisp;
    CString strTemp;
    if (::RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\tcax\\tccHelper\\fontColor\\CustomColors\\"), 
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisp) != ERROR_SUCCESS)
    {
        return FALSE;
    }
    for (int i = 0; i < 16; i ++)
    {
        strTemp.Format(_T("Custom_Color_%02i"), i + 1);
        ::RegSetValueEx(hKey, strTemp, 0, REG_SZ, (const BYTE *)(LPCTSTR)TcHexColor(m_crCustColors[i]), 14);
    }
    ::RegCloseKey(hKey);
    return TRUE;
}

void TCCFrm::FrmChooseColor()
{
    CString strColor = this->FrmGetFormatedColorFromEdit();
    ::SetWindowText(m_hEditColor, strColor);
    m_cc.rgbResult = TcDecColor(strColor);
    if (::ChooseColor(&m_cc))
    {
        m_crColor = m_cc.rgbResult;
        ::SetWindowText(m_hEditColor, TcHexColor(m_cc.rgbResult));
        this->FrmShowColor();
    }
}

void TCCFrm::FrmCopyFileToClipboard()
{
    HGLOBAL hGblFiles;
    unsigned char *gblBuf;
    DROPFILES dropFiles;
    WCHAR szWinDir[MAX_PATH];
    WCHAR filename[1024];
    int lenWinDir, flen, len;
    dropFiles.pFiles = sizeof(DROPFILES);
    dropFiles.pt.x = 0;
    dropFiles.pt.y = 0;
    dropFiles.fNC = FALSE;
    dropFiles.fWide = TRUE;
    GetWindowsDirectoryW(szWinDir, MAX_PATH);
    lenWinDir = wcslen(szWinDir);
    memset(filename, 0, 1024 * sizeof(WCHAR));
    memcpy(filename, szWinDir, lenWinDir * sizeof(WCHAR));
    memcpy(filename + lenWinDir, L"\\Fonts\\", 7 * sizeof(WCHAR));
    GetWindowTextW(m_hEditFileName, filename + lenWinDir + 7, 512);
    flen = wcslen(filename) + 2;
    len = sizeof(DROPFILES) + flen * sizeof(WCHAR) + 2;
    hGblFiles = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, len);
    gblBuf = (unsigned char *)GlobalLock(hGblFiles);
    memcpy(gblBuf, &dropFiles, sizeof(DROPFILES));
    memcpy(gblBuf + sizeof(DROPFILES), filename, flen * sizeof(WCHAR));
    GlobalUnlock(hGblFiles);
    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_HDROP, hGblFiles);
    CloseClipboard();
}

LRESULT CALLBACK TCCFrm::FrmColorDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return 0;
}

LRESULT CALLBACK TCCFrm::FrmDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            ::BeginPaint(hDlg, &ps);
            m_pFrm->FrmShowFont();
            m_pFrm->FrmShowColor();
            ::EndPaint(hDlg, &ps);
        }
        break;
    case WM_SYSCOMMAND:
        switch (wParam)
        {
        case SC_CLOSE:
            ::PostMessage(hDlg, TCCM_EXIT, 0, 0);
            break;
        default:
            return ::DefWindowProc(hDlg, uMsg, wParam, lParam);
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_COMBO_FONT:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                m_pFrm->FrmShowFont();
            }
            break;
        case IDC_EDIT_FACE_ID:
            if (HIWORD(wParam) == EN_CHANGE)
            {
                m_pFrm->FrmShowFont();
            }
            break;
        case IDC_EDIT_COLOR:
            if (HIWORD(wParam) == EN_CHANGE)
            {
                m_pFrm->FrmUpdateColor();
            }
            break;
        case IDC_BTN_COPY_FILE_NAME:
            ::PostMessage(hDlg, TCCM_COPY_FILE_NAME, 0, 0);
            break;
        case IDC_BTN_COPY_FACE_NAME:
            ::PostMessage(hDlg, TCCM_COPY_FACE_NAME, 0, 0);
            break;
        case IDC_BTN_CAPTURE_COLOR:
            ::PostMessage(hDlg, TCCM_CAPTURE_COLOR, 0, TCC_COLOR_CAPTURE_INIT);
            break;
        case IDC_BTN_SELECT_COLOR:
            ::PostMessage(hDlg, TCCM_SELECT_COLOR, 0, 0);
            break;
        case IDC_BTN_COPY_COLOR:
            ::PostMessage(hDlg, TCCM_COPY_COLOR, 0, 0);
            break;
        case IDC_BTN_COPY_FILE:
            ::PostMessage(hDlg, TCCM_COPY_FILE, 0, 0);
            break;
        case IDC_BTN_EXIT:
            ::PostMessage(hDlg, TCCM_EXIT, 0, 0);
            break;
        default:
            return ::DefWindowProc(hDlg, uMsg, wParam, lParam);
        }
        break;
    }
    return 0;
}
