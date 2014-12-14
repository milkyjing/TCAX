/*
 * tccAssist
 * milkyjing
 * 01.02.2010 ~ 07.02.2010
 * milkyjing@foxmail.com
 * http://www.tcsub.com
 */

#include "fontColor.h"


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
{
    MSG Msg;
    ZeroMemory(&Msg, sizeof(MSG));
    TCCFrm myFrm(hInstance);
    if (!myFrm.FrmInit())
    {
        return FALSE;
    }
    while (myFrm.FrmGetActiveFlag())
    {
        if (::GetMessage(&Msg, NULL, 0, 0))
        {
            if (TCCM_EXIT == Msg.message)
            {
                myFrm.FrmSetActiveFlag(FALSE);
            }
            else if (TCCM_COPY_FILE_NAME == Msg.message)
            {
                myFrm.FrmCopyFileName();
            }
            else if (TCCM_COPY_FACE_NAME == Msg.message)
            {
                myFrm.FrmCopyFaceName();
            }
            else if (TCCM_CAPTURE_COLOR == Msg.message)
            {
                if (TCC_COLOR_CAPTURE_INIT == Msg.lParam)
                {
                    myFrm.FrmMakePrevCur();
                    myFrm.FrmInitHideDlgFlag();
                    if (myFrm.FrmStartHook())
                    {
                        if (myFrm.FrmGetHideDlgFlag())
                        {
                            myFrm.FrmShowDlg(FALSE);
                            myFrm.FrmShowColorDlg(TRUE);
                        }
                        else
                        {
                            myFrm.FrmEnableButton(FALSE);
                        }
                    }
                }
                else if (TCC_COLOR_CAPTURING == Msg.lParam)
                {
                    myFrm.FrmCaptureColor();
                    if (myFrm.FrmGetHideDlgFlag())
                    {
                        myFrm.FrmShowClrDlgColor();
                    }
                    else
                    {
                        myFrm.FrmUpdateColorEdit();
                        myFrm.FrmShowColor();
                    }
                }
                else if (TCC_COLOR_CAPTURE_FIN == Msg.lParam)
                {
                    if (myFrm.FrmEndHook())
                    {
                        if (myFrm.FrmGetHideDlgFlag())
                        {
                            myFrm.FrmUpdateColorEdit();
                            myFrm.FrmShowColorDlg(FALSE);
                            myFrm.FrmShowDlg(TRUE);
                        }
                        else
                        {
                            myFrm.FrmEnableButton(TRUE);
                        }
                    }
                }
                else //if (TCC_COLOR_CAPTURE_CANCEL == Msg.lParam)
                {
                    if (myFrm.FrmEndHook())
                    {
                        myFrm.FrmMakeCurPrev();
                        if (myFrm.FrmGetHideDlgFlag())
                        {
                            myFrm.FrmShowColorDlg(FALSE);
                            myFrm.FrmShowDlg(TRUE);
                        }
                        else
                        {
                            myFrm.FrmUpdateColorEdit();
                            myFrm.FrmShowColor();
                            myFrm.FrmEnableButton(TRUE);
                        }
                    }
                }
            }
            else if (TCCM_SELECT_COLOR == Msg.message)
            {
                myFrm.FrmChooseColor();
            }
            else if (TCCM_COPY_COLOR == Msg.message)
            {
                myFrm.FrmCopyColor();
            }
            else if (TCCM_COPY_FILE == Msg.message)
            {
                myFrm.FrmCopyFileToClipboard();
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
