/* 
   KASS
   by milkyjing
   05.15.2009 ~ 05.16.2009
*/

#include "MainFrm.h"
#include "KASSFrm.h"
#include "KASS.h"
#include "resource.h"

int WINAPI wWinMain(	HINSTANCE hInstance,
						HINSTANCE hPrevInstance,
						LPWSTR lpCmdLine,
						int nShowCmd)
{
	// ---
	INT     Argc = 1;
	LPWSTR *Argv = NULL;
	Argv = ::CommandLineToArgvW(::GetCommandLine(), &Argc);
	// ---
	MSG Msg;
	ZeroMemory(&Msg, sizeof(MSG));
	static WCHAR szFile1[1024]     = L"*.ass";
	static WCHAR szFileTitle1[256] = L"";
	static WCHAR szFile2[1024]     = L"";
	static WCHAR szFileTitle2[256] = L"";
	KassFrm myFrm(hInstance);
	Kass    myTC(&myFrm);
	myFrm.FrmMainDlg();
	if (Argc > 1)
	{
		myFrm.FrmSetOpenFileFlag(TRUE);
		myTC.OpenFile(Argv[1]);
	}
	while (myFrm.FrmGetActiveFlag())
	{
		if (::GetMessage(&Msg, NULL, 0, 0))
		{
			if (Msg.message == DIALOG_MAIN_OPEN)
			{
				myFrm.FrmSetOfn(szFile1, szFileTitle1);
				if (myFrm.FrmFileDlg(TRUE))
				{
					Argc = 1;
					myFrm.FrmSetOpenFileFlag(TRUE);
					myTC.OpenFile(myFrm.FrmGetOfn().lpstrFile);
				}
			}
			else if (Msg.message == DIALOG_MAIN_SAVEAS)
			{
				WCHAR szData[2];
				ZeroMemory(szData, 2 * sizeof(WCHAR));
				HWND hComboWnd1 = ::GetDlgItem(myFrm.FrmGetWnd(), IDC_MAIN_COMBO_NTXTPERKAR);
				::SendMessage(	hComboWnd1, CB_GETLBTEXT, 
								::SendMessage(hComboWnd1, CB_GETCURSEL, 0, 0),
								(LPARAM)szData);
				myFrm.FrmSetnTxtPerKar(_wtoi(szData));
				myFrm.FrmSetOfn(szFile2, szFileTitle2);
				if (myTC.TcConvert(myFrm.FrmGetnTxtPerKar()))
				{
					if (myFrm.FrmFileDlg(FALSE))
					{
						myTC.TcSaveASS(myFrm.FrmGetOfn().lpstrFile);
					}
				}
			}
			else if (Msg.message == DIALOG_MAIN_CLEAR)
			{
				myFrm.FrmSetOpenFileFlag(FALSE);
				myTC.TcClear();
			}
			else if (Msg.message == DIALOG_MAIN_EXIT)
			{
				myFrm.FrmSetActiveFlag(FALSE);
			}
			else
			{
				::TranslateMessage(&Msg);
				::DispatchMessage(&Msg);
			}
		}
	}
	return Msg.wParam;
}
