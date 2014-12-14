#ifndef _STDAFX_H_
#define _STDAFX_H_
#pragma once
#ifndef WINVER
#define WINVER 0x0501
#endif

#define LEN_READ      0x00007800             // Max bytes read from an ass script, 30KB
#define LEN_LINES     0x00000078             // Max lines of dialogues allowed in an ass script, 120 lines
#define LEN_WORDS     0x00000064             // Max characters in a dialogue line, 100 characters

#include <windows.h>
#include <atlstr.h>
#include <vector>
using std::vector;


VOID  Message(int i, HWND hWnd = NULL);
VOID  Message(LPTSTR lpstr, HWND hWnd = NULL);
VOID  Message(CString str, HWND hWnd = NULL);
#endif
