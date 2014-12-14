#include "stdafx.h"


VOID Message(int i, HWND hWnd)
{
	CString str;
	str.Format(_T("%i"), i);
	::MessageBox(hWnd, str, _T("Info."), MB_OK);
}

VOID Message(LPTSTR lpstr, HWND hWnd)
{
	CString str;
	str.Format(_T("%s"), lpstr);
	::MessageBox(hWnd, str, _T("Info."), MB_OK);
}

VOID Message(CString str, HWND hWnd)
{
	::MessageBox(hWnd, str, _T("Info."), MB_OK);
}
