#ifndef __FONTCOLOR_H__
#define __FONTCOLOR_H__
#pragma once

#include "stdafx.h"
#include "resource.h"

// messages
#define WNDCLASS_NAME             _T("tcax_by_milkyjing")

#define TCCM_EXIT                 (WM_USER + 101)
#define TCCM_COPY_FILE_NAME       (WM_USER + 102)
#define TCCM_COPY_FACE_NAME       (WM_USER + 103)
#define TCCM_CAPTURE_COLOR        (WM_USER + 104)
#define TCCM_SELECT_COLOR         (WM_USER + 105)
#define TCCM_COPY_COLOR           (WM_USER + 106)
#define TCCM_COPY_FILE            (WM_USER + 110)

// macros
#define TCC_COLOR_CAPTURE_INIT     1
#define TCC_COLOR_CAPTURING        2
#define TCC_COLOR_CAPTURE_FIN      3
#define TCC_COLOR_CAPTURE_CANCEL   4


// function
inline BYTE ToByte(int i);
inline int HexToDec(CString strHex);
inline CString TcHexColor(DWORD dwColor);
inline COLORREF TcDecColor(CString strColor);
inline BOOL IsHexNumber(LPCTSTR str);


typedef BOOL (*pTcc_StartHookFunc)(HWND);
typedef BOOL (*pTcc_EndHookFunc)();

class TCCFrm
{
private:
    HINSTANCE   m_hInst;
    HDC         m_hDesktopDC;
    HWND        m_hDlg;
    HDC         m_hDC;
    HWND        m_hColorDlg;
    HDC         m_hColorDC;
    HMODULE     m_hDll;
    // control
    HWND        m_hComboFN;
    HWND        m_hEditFaceID;
    HWND        m_hEditFileName;
    HWND        m_hEditFaceName;
    HWND        m_hEditColor;
    HWND        m_hCheckHideDlg;
    HWND        m_hBtnCapClr;
    // flag
    BOOL        m_bActive;
    BOOL        m_bHideDlg;
    // metrics
    int         m_iClrDlgWd;
    int         m_iClrDlgHt;
    // ---
    CHOOSECOLOR m_cc;
    COLORREF    m_crCustColors[16];
    COLORREF    m_crColor;
    COLORREF    m_crPrevColor;
    // ---
    static TCCFrm *m_pFrm;
private:
    pTcc_StartHookFunc m_pTccStartHook;
    pTcc_EndHookFunc   m_pTccEndHook;
    CString FrmGetFormatedColorFromEdit();
    void    FrmInitCc();
    BOOL    FrmWriteCustColorsToReg();
    BOOL    FrmListFonts();
    static  LRESULT CALLBACK FrmColorDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static  LRESULT CALLBACK FrmDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
    TCCFrm(HINSTANCE hInst) :    m_hInst(hInst), m_hDesktopDC(NULL), m_hDlg(NULL), m_hDC(NULL), m_hColorDlg(NULL), m_hColorDC(NULL), 
                                m_hDll(NULL), m_hComboFN(NULL), m_hEditFaceID(NULL), m_hEditFileName(NULL), m_hEditFaceName(NULL), 
                                m_hEditColor(NULL), m_hCheckHideDlg(NULL), m_hBtnCapClr(NULL), m_bActive(TRUE), m_bHideDlg(TRUE), 
                                m_iClrDlgWd(0), m_iClrDlgHt(0), m_crColor(0xFFFFFF), m_crPrevColor(0xFFFFFF), m_pTccStartHook(NULL), 
                                m_pTccEndHook(NULL) {
        m_pFrm = this;
    }
    ~TCCFrm() {
        this->FrmWriteCustColorsToReg();
        if (m_hDesktopDC)
        {
            ::ReleaseDC(NULL, m_hDesktopDC);
        }
        if (m_hDC)
        {
            ::ReleaseDC(m_hDlg, m_hDC);
        }
        if (m_hColorDC)
        {
            ::ReleaseDC(m_hColorDlg, m_hColorDC);
        }
        if (m_hDlg)
        {
            ::DestroyWindow(m_hDlg);
        }
        if (m_hColorDlg)
        {
            ::DestroyWindow(m_hColorDlg);
        }
    }
    BOOL FrmInit();
    inline BOOL FrmStartHook() { return m_pTccStartHook(m_hDlg); }
    inline BOOL FrmEndHook() { return m_pTccEndHook(); }
    inline void FrmShowDlg(BOOL bShow) {
        if (bShow)
        {
            ::ShowWindow(m_hDlg, SW_SHOW);
        }
        else
        {
            ::ShowWindow(m_hDlg, SW_HIDE);
        }
    }
    inline void FrmShowColorDlg(BOOL bShow) {
        if (bShow)
        {
            POINT pt;
            ::GetCursorPos(&pt);
            ::MoveWindow(m_hColorDlg, pt.x, pt.y, m_iClrDlgWd, m_iClrDlgHt, FALSE);
            ::ShowWindow(m_hColorDlg, SW_SHOW);
        }
        else
        {
            ::ShowWindow(m_hColorDlg, SW_HIDE);
        }
    }
    inline void FrmEnableButton(BOOL bEnable) { ::EnableWindow(m_hBtnCapClr, bEnable); }
    inline void FrmSetActiveFlag(BOOL bFlag) { m_bActive = bFlag; }
    inline BOOL FrmGetActiveFlag() const { return m_bActive; }
    inline void FrmInitHideDlgFlag() { m_bHideDlg = (::SendMessage(m_hCheckHideDlg, BM_GETCHECK, 0, 0) == BST_CHECKED); }
    inline BOOL FrmGetHideDlgFlag() { return m_bHideDlg; }
    // ---
    inline void FrmCopyFileName() {
        ::SendMessage(m_hEditFileName, EM_SETSEL, 0, -1);
        ::SendMessage(m_hEditFileName, WM_COPY, 0, 0);
    }
    void FrmCopyFileToClipboard();
    inline void FrmCopyFaceName() {
        ::SendMessage(m_hEditFaceName, EM_SETSEL, 0, -1);
        ::SendMessage(m_hEditFaceName, WM_COPY, 0, 0);
    }
    inline void FrmCopyColor() {
        ::SetWindowText(m_hEditColor, this->FrmGetFormatedColorFromEdit());
        ::SendMessage(m_hEditColor, EM_SETSEL, 0, -1);
        ::SendMessage(m_hEditColor, WM_COPY, 0, 0);
    }
    // ---
    BOOL FrmShowFont();
    void FrmChooseColor();
    inline void FrmShowColor() {
        HBRUSH hBrush = (HBRUSH)::GetStockObject(DC_BRUSH);
        HBRUSH hOldBrush = (HBRUSH)::SelectObject(m_hDC, hBrush);
        ::SetDCBrushColor(m_hDC, m_crColor);
        ::Rectangle(m_hDC, 210, 316, 270, 342);
        ::SelectObject(m_hDC, hOldBrush);
        ::DeleteObject(hBrush);
    }
    inline void FrmShowClrDlgColor() {
        POINT ptCur;
        ::GetCursorPos(&ptCur);
        if (ptCur.x + 50 + 10 + m_iClrDlgWd > ::GetSystemMetrics(SM_CXSCREEN))
        {
            ptCur.x -= (15 + m_iClrDlgWd);
        }
        if (ptCur.y + 50 + 30 + m_iClrDlgHt > ::GetSystemMetrics(SM_CYSCREEN))
        {
            ptCur.y -= (35 + m_iClrDlgHt);
        }
        ::MoveWindow(m_hColorDlg, ptCur.x + 10, ptCur.y + 30, m_iClrDlgWd, m_iClrDlgHt, TRUE);
        HBRUSH hBrush = (HBRUSH)::GetStockObject(DC_BRUSH);
        HBRUSH hOldBrush = (HBRUSH)::SelectObject(m_hColorDC, hBrush);
        ::SetDCBrushColor(m_hColorDC, m_crColor);
        ::Rectangle(m_hColorDC, 0, 0, m_iClrDlgWd, m_iClrDlgHt);
        ::SelectObject(m_hColorDC, hOldBrush);
        ::DeleteObject(hBrush);
    }
    inline void FrmCaptureColor() {
        POINT ptCur;
        ::GetCursorPos(&ptCur);
        m_crColor = ::GetPixel(m_hDesktopDC, ptCur.x, ptCur.y);
    }
    inline void FrmUpdateColor() {
        m_crColor = TcDecColor(this->FrmGetFormatedColorFromEdit());
        this->FrmShowColor();
    }
    inline void FrmUpdateColorEdit() { ::SetWindowText(m_hEditColor, TcHexColor(m_crColor)); }
    inline void FrmMakeCurPrev() { m_crColor = m_crPrevColor; }
    inline void FrmMakePrevCur() { m_crPrevColor = m_crColor; }
};

#endif    // __FONT_COLOR_H__
