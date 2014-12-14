#ifndef WINVER
#define WINVER 0x0501
#endif

#include <windows.h>
#include <atlstr.h>

#pragma data_seg("TcxMouseHookData")
HINSTANCE g_hInst = NULL;
HHOOK     g_hHook = NULL;
HWND      g_hWnd  = NULL;
#pragma data_seg()
#pragma comment(linker, "/section:TcxMouseHookData,rws")


// messages
#define TCXM_CAPTURE_COLOR        (WM_USER + 104)
// macros
//#define TCX_COLOR_CAPTURE_INIT     1
#define TCX_COLOR_CAPTURING        2
#define TCX_COLOR_CAPTURE_FIN      3
#define TCX_COLOR_CAPTURE_CANCEL   4

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    if (!g_hHook)   // Note: we can't use DLL_PROCESS_ATTACH or something like that, otherwise SetWindowsHookEx function will fail and GetLastError will simply return 0
    {
        g_hInst = hinstDLL;    // There will be many Processes/Threads including this dll however g_hInst need to be eqaul to the tccAssist.exe's not any others'
    }
    return TRUE;
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
    {
        return ::CallNextHookEx(g_hHook, nCode, wParam, lParam);
    }
    switch (wParam)
    {
    case WM_MOUSEMOVE:
        ::PostMessage(g_hWnd, TCXM_CAPTURE_COLOR, 0, TCX_COLOR_CAPTURING);
        break;
    case WM_LBUTTONDOWN:
        ::PostMessage(g_hWnd, TCXM_CAPTURE_COLOR, 0, TCX_COLOR_CAPTURE_FIN);
        break;
    case WM_RBUTTONDOWN:
        ::PostMessage(g_hWnd, TCXM_CAPTURE_COLOR, 0, TCX_COLOR_CAPTURE_CANCEL);
        break;
    }
    return ::CallNextHookEx(g_hHook, nCode, wParam, lParam);
}


extern "C" __declspec(dllexport) BOOL StartMouseHook(HWND hWnd)
{
    g_hWnd = hWnd;
    g_hHook = ::SetWindowsHookEx(WH_MOUSE_LL, MouseProc, g_hInst, 0);
    if (!g_hHook)
    {
        return FALSE;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) BOOL EndMouseHook()
{
    if (g_hHook)
    {
        return ::UnhookWindowsHookEx(g_hHook);
    }
    return TRUE;
}
