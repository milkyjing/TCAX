#include "stdafx.h"


void Message(int i)
{
    WCHAR wcs[33];
    ::_itow_s(i, wcs, 32, 10);
    ::MessageBoxW(NULL, wcs, L"Info", MB_OK);
}

void Message(LPCWSTR wcs)
{
    ::MessageBoxW(NULL, wcs, L"Info", MB_OK);
}

void Message(LPCSTR str)
{
    ::MessageBoxA(NULL, str, "Info", MB_OK);
}
