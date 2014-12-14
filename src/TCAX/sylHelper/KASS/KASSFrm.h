#ifndef _TCASSFRM_H_
#define _TCASSFRM_H_
#pragma once

#include "stdafx.h"

#define DIALOG_MAIN_OPEN      (WM_USER + 101)
#define DIALOG_MAIN_SAVEAS    (WM_USER + 102)
#define DIALOG_MAIN_CLEAR     (WM_USER + 103)
#define DIALOG_MAIN_EXIT      (WM_USER + 104)

class KassFrm
{
private:
	HINSTANCE      m_hInst;
	HWND           m_hWnd;
	OPENFILENAME   m_ofn;
	BOOL           m_bActive;
	BOOL           m_bHasOpenFile;
	UINT           m_nTxtPerKar;   // how many texts in a karaoke timing
public:
	KassFrm(HINSTANCE hInstance);
	~KassFrm();
	VOID           FrmMainDlg();
	VOID           FrmSetOfn(WCHAR szFile[], WCHAR szFileTitle[]);
	OPENFILENAME   FrmGetOfn() const;
	VOID           FrmSetActiveFlag(BOOL bFlag);
	BOOL           FrmGetActiveFlag() const;
	VOID           FrmSetOpenFileFlag(BOOL bFlag);
	BOOL           FrmGetOpenFileFlag() const;
	VOID           FrmSetnTxtPerKar(UINT nTxtPerKar = 1);
	UINT           FrmGetnTxtPerKar() const;
	HWND           FrmGetWnd() const;
	BOOL           FrmFileDlg(BOOL bFlag);
	static LRESULT CALLBACK FrmMainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
#endif
