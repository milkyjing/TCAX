#include "KASS.h"
#include "resource.h"


Kass::Kass(KassFrm *pmyFrm)
{
	m_pFrm         = pmyFrm;
	m_ASSIn        = NULL;
	m_dwASSInSize  = 0;
	m_ASSOut       = NULL;
	m_dwASSOutSize = 0;
	m_strHeader    = L"";
	m_nLines       = 0;
}

Kass::~Kass()
{
	this->TcClear();
}

VOID Kass::ChangeEndian(WCHAR *pwCh, DWORD dwSize)
{
	for (UINT i = 0; i < dwSize; i ++)
	{
		BYTE Temp = pwCh[i] >> 8;
		pwCh[i] = (((BYTE)pwCh[i]) << 8) | Temp;
	}
}

CString Kass::spFmtTime(LONG lTime)
{
	CString strTime;
	CString strH;
	CString strM;
	CString strS;
	CString strSs;
	int iH = lTime / 360000;
	int iM = (lTime / 6000) % 60;
	int iS = (lTime / 100) % 60;
	int iSs = lTime % 100;
	strH.Format(L"%i", iH);
	strM.Format(L"%i", iM);
	strS.Format(L"%i", iS);
	strSs.Format(L"%i", iSs);
	if (iM < 10)
	{
		strM = L"0" + strM;
	}
	if (iS < 10)
	{
		strS = L"0" + strS;
	}
	if (iSs < 10)
	{
		strSs = L"0" + strSs;
	}
	strTime.Format(L"%s:%s:%s.%s", strH, strM, strS, strSs);
	return strTime;
}

LONG Kass::spDeFmtTime(CString strTime)
{
	LONG lTime;
	lTime = _tstol(strTime.Mid(0, 1)) * 60 * 60 * 100 \
			+ _tstol(strTime.Mid(2, 2)) * 60 * 100 \
			+ _tstol(strTime.Mid(5, 2)) * 100 \
			+ _tstol(strTime.Mid(8, 2));
	return lTime;
}

BOOL Kass::OpenFile(LPCWSTR lpFileName)
{
	int   iFlag = 1;     // 1 - ANSI, 2 - UNICODE, 3 - UNICODE big endian, 4 - UTF-8
	DWORD dwSize;
	DWORD dwLen;
	WCHAR *lpBuffer = new WCHAR [LEN_READ];
	ZeroMemory(lpBuffer, LEN_READ * sizeof(WCHAR));
	HANDLE hFile = ::CreateFile(	lpFileName, GENERIC_READ, FILE_SHARE_READ, 
									NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	::ReadFile(hFile, lpBuffer, LEN_READ, &dwSize, NULL);
	::CloseHandle(hFile);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		::MessageBox(	m_pFrm->FrmGetWnd(), L"无法打开文件...      ",
						L"错误", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	if ((dwSize >= 2) && ((BYTE)(lpBuffer[0] >> 8) == 0xFE) && ((BYTE)lpBuffer[0] == 0xFF))
	{
		dwLen = dwSize / 2;
		iFlag = 2;
	}
	if ((dwSize >= 2) && ((BYTE)(lpBuffer[0] >> 8) == 0xFF) && ((BYTE)lpBuffer[0] == 0xFE))
	{
		dwLen = dwSize / 2;
		this->ChangeEndian(lpBuffer, dwSize);
		iFlag = 3;
	}
	if ((dwSize >= 3) && ((BYTE)lpBuffer[0] == 0xEF) && ((BYTE)(lpBuffer[0] >> 8) == 0xBB) && ((BYTE)lpBuffer[1] == 0xBF))
	{
		dwLen = ::MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)lpBuffer, -1, NULL, 0);
		LPWSTR lpWideCharStr = new WCHAR [dwLen];
		ZeroMemory(lpWideCharStr, dwLen * sizeof(WCHAR));
		::MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)lpBuffer, -1, lpWideCharStr, dwLen);
		delete[] lpBuffer;
		lpBuffer = lpWideCharStr;
		lpWideCharStr = NULL;
		iFlag = 4;
	}
	if (iFlag == 1)
	{
		dwLen = ::MultiByteToWideChar(CP_ACP, 0, (LPCSTR)lpBuffer, -1, NULL, 0);
		LPWSTR lpWideCharStr = new WCHAR [dwLen];
		ZeroMemory(lpWideCharStr, dwLen * sizeof(WCHAR));
		::MultiByteToWideChar(CP_ACP, 0, (LPCSTR)lpBuffer, -1, lpWideCharStr, dwLen);
		delete[] lpBuffer;
		lpBuffer = lpWideCharStr;
		lpWideCharStr = NULL;
	}
	::SetWindowText(::GetDlgItem(m_pFrm->FrmGetWnd(), IDC_MAIN_EDIT_ASS), lpBuffer);
	delete[] lpBuffer;
	return TRUE;
}

BOOL Kass::GetWindowTXT()
{
	m_dwASSInSize = ::GetWindowTextLength(::GetDlgItem(m_pFrm->FrmGetWnd(), IDC_MAIN_EDIT_ASS));
	if (m_dwASSInSize > LEN_READ)
	{
		::MessageBox(	m_pFrm->FrmGetWnd(), L"ASS文件的长度不能超过30KB...      ",
						L"错误", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	else if (m_dwASSInSize < 1)
	{
		::MessageBox(	m_pFrm->FrmGetWnd(), L"ASS文件不能为空...      ", 
						L"错误", MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	m_ASSIn = new WCHAR [m_dwASSInSize + 1];
	ZeroMemory(m_ASSIn, (m_dwASSInSize + 1) * sizeof(WCHAR));
	::GetWindowText(::GetDlgItem(m_pFrm->FrmGetWnd(), IDC_MAIN_EDIT_ASS), m_ASSIn, m_dwASSInSize + 1);
	return TRUE;
}

BOOL Kass::TcDlg()
{
	if (m_ASSIn == NULL || m_dwASSInSize == 0)
	{
		return FALSE;
	}
	int iFlag = 1;              // 1 - ansi, 2 - unicode, 3 - unicode big endian, 4 - utf8
	int iIdx = 0;
	int iDiff = 0;              // lines skipped
	int arrIdx[LEN_LINES + 1];  // Store chunk (one dialogue equals a chunk) info.
	arrIdx[0] = 0;
	CString strTemp(m_ASSIn);
	delete[] m_ASSIn;
	m_ASSIn = NULL;
	int iHead1 = strTemp.Find(L"[Events]");
	int iHead2 = strTemp.Find(L", Start, End, ");
	if (iHead1 == -1 && iHead2 == -1)
	{
		::MessageBox(	m_pFrm->FrmGetWnd(), L"此ASS文件无效...      ", 
						L"错误", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	iHead1 = strTemp.Find(L"\r\n", iHead1 + 10 + 1 + 60);
	iHead2 = strTemp.Find(L"\r\n", iHead2 + 1);
	int iHead = __max(iHead1, iHead2);        // Ensure safety
	m_nLines = 0;
	for (int i = 0; i < LEN_LINES; i ++)
	{
		m_Dlg[i] = L"";
	}
	m_strHeader = strTemp.Mid(0, iHead);
	strTemp = strTemp.Mid(iHead, m_dwASSInSize - iHead);
	while (TRUE)
	{
		iIdx ++;
		arrIdx[iIdx] = strTemp.Find(L"\r\n", arrIdx[iIdx - 1] + 2);
		if (arrIdx[iIdx] == -1)                          // if true: Reach the end of the file
		{
			if ((m_dwASSInSize - (iHead + 2 + arrIdx[iIdx - 1])) < 50)   // Skip non-dialogue lines
			{
				iDiff ++;
				break;
			}
			m_Dlg[iIdx - iDiff - 1] = strTemp.Mid(arrIdx[iIdx - 1] + 2, (m_dwASSInSize - (iHead + 2 + arrIdx[iIdx - 1])));
			break;
		}
		else if (arrIdx[iIdx] - arrIdx[iIdx - 1] < 50)   // Skip non-dialogue lines
		{
			iDiff ++;
		}
		else
		{
			m_Dlg[iIdx - iDiff - 1] = strTemp.Mid(arrIdx[iIdx - 1] + 2, arrIdx[iIdx] - arrIdx[iIdx - 1] - 2);
			if (iIdx - iDiff >= LEN_LINES)
			{
				::MessageBox(	m_pFrm->FrmGetWnd(), L"所处理ASS文件的Dialogue行数不超过120...      ", 
								L"警告", MB_OK | MB_ICONINFORMATION);
				m_nLines = 119;
				return TRUE;
			}
		}
	}
	m_nLines = iIdx - iDiff;
	if (m_nLines == 0)
	{
		return FALSE;
	}
	m_dwASSInSize = 0;
	return TRUE;
}

VOID Kass::TcTime()
{
	ZeroMemory(m_BTime, LEN_LINES * sizeof(INT));
	ZeroMemory(m_ETime, LEN_LINES * sizeof(INT));
	for (UINT i = 0; i < m_nLines; i ++)
	{
		int iFlag1 = m_Dlg[i].Find(L",", 0);
		int iFlag2 = m_Dlg[i].Find(L",", iFlag1 + 1);
		m_BTime[i] = this->spDeFmtTime(m_Dlg[i].Mid(iFlag1 + 1, 10));
		m_ETime[i] = this->spDeFmtTime(m_Dlg[i].Mid(iFlag2 + 1, 10));
		int temp = m_Dlg[i].Find(L",,", 0);
		int temp2 = m_Dlg[i].Find(L",,", temp + 2);
		if (temp2 != -1)
		{
		    temp = temp2;
		}
		m_SubDlg[i] = m_Dlg[i].Mid(0, 2 + temp);
	}
}

VOID Kass::TcText(UINT nTxtPerKar)
{
	ZeroMemory(m_nTxt, LEN_LINES * sizeof(UINT));
	UINT iLine = 0;
	while (iLine < m_nLines)
	{
		UINT iIdx = 0;
		UINT nTotalNbr;
		int temp = m_Dlg[iLine].Find(L",,", 0);
		int temp2 = m_Dlg[iLine].Find(L",,", temp + 2);
		if (temp2 != -1)
		{
		    temp = temp2;
		}
		int iInitPos = 2 + temp;
		int iPos = iInitPos;
		m_nTxt[iLine] = m_Dlg[iLine].GetLength() - iInitPos;
		nTotalNbr = m_nTxt[iLine] / nTxtPerKar;
		if (m_nTxt[iLine] % nTxtPerKar != 0)
		{
			nTotalNbr ++;
		}
		while (iIdx < nTotalNbr)
		{
			m_Txt[iLine][iIdx] = m_Dlg[iLine].Mid(iPos, nTxtPerKar);
			iPos += nTxtPerKar;
			iIdx ++;
		}
		iLine ++;
	}
}

BOOL Kass::TcConvert(UINT nTxtPerKar)
{
	if (!this->GetWindowTXT())
	{
		return FALSE;
	}
	if (!this->TcDlg())
	{
		return FALSE;
	}
	this->TcTime();
	this->TcText(nTxtPerKar);
	CString strASSOut = m_strHeader + L"\r\n";
	for (UINT i = 0; i < m_nLines; i ++)
	{
		int iKaraoke;
		UINT nTotalNbr;
		CString strKaraoke;
        CString strTemp = m_SubDlg[i] + L"{\\K4}";
		nTotalNbr = m_nTxt[i] / nTxtPerKar;
		if (m_nTxt[i] % nTxtPerKar != 0)
		{
			nTotalNbr ++;
		}
		for (UINT j = 0; j < nTotalNbr; j ++)
		{
			iKaraoke = nTxtPerKar * (m_ETime[i] - m_BTime[i]) / m_nTxt[i];
			if (nTxtPerKar * (m_ETime[i] - m_BTime[i]) % m_nTxt[i] >= j)
			{
				iKaraoke ++;
			}
			strKaraoke.Format(L"%i", iKaraoke);
			strTemp += L"{\\K" + strKaraoke + L"}" + m_Txt[i][j];
		}
		strASSOut += strTemp + L"\r\n";
	}
	m_ASSOut = new WCHAR [LEN_READ];
	m_ASSOut[0] = 0xFEFF;
	::wcscpy_s(m_ASSOut + 1, -1, strASSOut);
	m_dwASSOutSize = strASSOut.GetLength();
	return TRUE;
}

VOID Kass::TcClear()
{
	if (m_ASSIn != NULL)
	{
		delete[] m_ASSIn;
		m_ASSIn = NULL;
		m_dwASSInSize = 0;
	}
	if (m_ASSOut != NULL)
	{
		delete[] m_ASSOut;
		m_ASSOut = NULL;
		m_dwASSOutSize = 0;
	}
	m_strHeader = L"";
	m_nLines = 0;
	::SetWindowText(::GetDlgItem(m_pFrm->FrmGetWnd(), IDC_MAIN_EDIT_ASS), L"");
}

BOOL Kass::TcSaveASS(LPWSTR lpFileName)
{
	if (m_ASSOut == NULL || m_dwASSOutSize == 0)
	{
		return FALSE;
	}
	DWORD dwSize;
	HANDLE hFile = ::CreateFile(	lpFileName, GENERIC_WRITE, FILE_SHARE_WRITE, 
									NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		::MessageBox(	m_pFrm->FrmGetWnd(), L"无法保存文件...      ",
						L"错误", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	::WriteFile(hFile, m_ASSOut, 2 * m_dwASSOutSize + 2, &dwSize, NULL);
	::CloseHandle(hFile);
	delete[] m_ASSOut;
	m_ASSOut = NULL;
	m_dwASSOutSize = 0;
	return TRUE;
}
