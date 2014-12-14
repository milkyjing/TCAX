#include "KASSFrm.h"
#include "resource.h"


KassFrm::KassFrm(HINSTANCE hInstance)
{
	m_hInst        = hInstance;
	m_hWnd         = NULL;
	m_bActive      = TRUE;
	m_bHasOpenFile = FALSE;
}

KassFrm::~KassFrm()
{
	if (m_hWnd != NULL)
	{
		::DestroyWindow(m_hWnd);
	}
}

VOID KassFrm::FrmMainDlg()
{
	m_hWnd = ::CreateDialog(m_hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, (DLGPROC)FrmMainDlgProc);
	::SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_ICON_MAIN)));
	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
	::ShowWindow(m_hWnd, SW_SHOW);
}

VOID KassFrm::FrmSetOfn(WCHAR szFile[], WCHAR szFileTitle[])
{
	ZeroMemory(&m_ofn, sizeof(OPENFILENAME));
	m_ofn.lStructSize       = sizeof(OPENFILENAME);
	m_ofn.hwndOwner         = m_hWnd;
	m_ofn.hInstance         = NULL;
	m_ofn.lpstrFilter       = NULL;
	m_ofn.lpstrCustomFilter = NULL;
	m_ofn.nMaxCustFilter    = 0;
	m_ofn.nFilterIndex      = 1;
	m_ofn.lpstrFile         = szFile;
	m_ofn.nMaxFile          = 1024;
	m_ofn.lpstrFileTitle    = szFileTitle;
	m_ofn.nMaxFileTitle     = 256;
	m_ofn.lpstrInitialDir   = NULL;
	m_ofn.lpstrTitle        = NULL;
	m_ofn.Flags             = 0;
	m_ofn.nFileOffset       = 0;
	m_ofn.nFileExtension    = 0;
	m_ofn.lpstrDefExt       = NULL;
	m_ofn.lCustData         = 0;
	m_ofn.lpfnHook          = NULL;
	m_ofn.lpTemplateName    = NULL;
	m_ofn.pvReserved        = NULL;
	m_ofn.dwReserved        = 0;
	m_ofn.FlagsEx           = 0;
}

OPENFILENAME KassFrm::FrmGetOfn() const
{
	return m_ofn;
}

VOID KassFrm::FrmSetActiveFlag(BOOL bFlag)
{
	m_bActive = bFlag;
}

BOOL KassFrm::FrmGetActiveFlag() const
{
	return m_bActive;
}

VOID KassFrm::FrmSetOpenFileFlag(BOOL bFlag)
{
	m_bHasOpenFile = bFlag;
}

BOOL KassFrm::FrmGetOpenFileFlag() const
{
	return m_bHasOpenFile;
}

VOID KassFrm::FrmSetnTxtPerKar(UINT nTxtPerKar)
{
	if (nTxtPerKar < 1)
	{
		m_nTxtPerKar = 1;
	}
	else if (nTxtPerKar > 5)
	{
		m_nTxtPerKar = 5;
	}
	else
	{
		m_nTxtPerKar = nTxtPerKar;
	}
}

UINT KassFrm::FrmGetnTxtPerKar() const
{
	return m_nTxtPerKar;
}

HWND KassFrm::FrmGetWnd() const
{
	return m_hWnd;
}

BOOL KassFrm::FrmFileDlg(BOOL bFlag)
{
	m_ofn.lpstrFilter = L"ASS文件(*.ass)\0*.ass\0\0";
	m_ofn.lpstrDefExt = L"ass";
	if (bFlag)
	{
		m_ofn.lpstrTitle = L"打开";
		m_ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
		if (::GetOpenFileName(&m_ofn))
		{
			return TRUE;
		}
	}
	else
	{
		m_ofn.lpstrTitle = L"保存";
		m_ofn.Flags = OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
		if (::GetSaveFileName(&m_ofn))
		{
			return TRUE;
		}
	}
	return FALSE;
}

LRESULT CALLBACK KassFrm::FrmMainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hComboWnd1;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		hComboWnd1 = ::GetDlgItem(hDlg, IDC_MAIN_COMBO_NTXTPERKAR);
		::SendMessage(hComboWnd1, CB_ADDSTRING, 0, (LPARAM)L"1");
		::SendMessage(hComboWnd1, CB_ADDSTRING, 0, (LPARAM)L"2");
		::SendMessage(hComboWnd1, CB_ADDSTRING, 0, (LPARAM)L"3");
		::SendMessage(hComboWnd1, CB_ADDSTRING, 0, (LPARAM)L"4");
		::SendMessage(hComboWnd1, CB_ADDSTRING, 0, (LPARAM)L"5");
		::SendMessage(hComboWnd1, CB_SELECTSTRING, 0, (LPARAM)L"1");
		break;
	case WM_SYSCOMMAND:
		switch (LOWORD(wParam))
		{
		case SC_CLOSE:
			::PostMessage(hDlg, DIALOG_MAIN_EXIT, 0, 0);
			break;
		default:
			return ::DefWindowProc(hDlg, uMsg, wParam, lParam);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_MAIN_BTM_OPEN:
			::PostMessage(hDlg, DIALOG_MAIN_OPEN, 0, 0);
			break;
		case IDC_MAIN_BTM_SAVEAS:
			::PostMessage(hDlg, DIALOG_MAIN_SAVEAS, 0, 0);
			break;
		case IDC_MAIN_BTM_CLEAR:
			::PostMessage(hDlg, DIALOG_MAIN_CLEAR, 0, 0);
			break;
		case IDC_MAIN_BTM_EXIT:
			::PostMessage(hDlg, DIALOG_MAIN_EXIT, 0, 0);
			break;
		default:
			return ::DefWindowProc(hDlg, uMsg, wParam, lParam);
		}
		break;
	/*default:
		return ::DefWindowProc(hDlg, uMsg, wParam, lParam);
		//Note: in dialog Proc you MUSN'T use this statement*/
	}
	return 0;
}
