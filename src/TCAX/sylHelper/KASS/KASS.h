#ifndef _KASS_H_
#define _KASS_H_
#pragma once

#include "stdafx.h"
#include "KASSFrm.h"

class Kass
{
private:
	KassFrm   *m_pFrm;
	WCHAR     *m_ASSIn;
	DWORD      m_dwASSInSize;
	WCHAR     *m_ASSOut;
	DWORD      m_dwASSOutSize;
	CString    m_strHeader;
	UINT       m_nLines;
	CString    m_Dlg[LEN_LINES];
	CString    m_SubDlg[LEN_LINES];
	CString    m_Txt[LEN_LINES][LEN_WORDS];
	UINT       m_nTxt[LEN_LINES];
	INT        m_BTime[LEN_LINES];
	INT        m_ETime[LEN_LINES];
private:
	VOID       ChangeEndian(WCHAR *pwCh, DWORD dwSize);
	CString    spFmtTime(LONG lTime);
	LONG       spDeFmtTime(CString strTime);
	BOOL       GetWindowTXT();                // Get text from the edit-control
	BOOL       TcDlg();
	VOID       TcTime();
	VOID       TcText(UINT nTxtPerKar);
public:
	Kass(KassFrm *pmyFrm);
	~Kass();
	BOOL       OpenFile(LPCWSTR lpFileName);
	BOOL       TcConvert(UINT nTxtPerKar = 1);  // put the above in one function
	VOID       TcClear();                       // Clean the edit-control and memory buffers
	BOOL       TcSaveASS(LPWSTR lpFileName);
	
};
#endif  // _KASS_H_
