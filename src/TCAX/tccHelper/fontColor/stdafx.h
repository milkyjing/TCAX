#ifndef __STDAFX_H__
#define __STDAFX_H__
#pragma once
#ifndef WINVER
#define WINVER 0x0501
#endif

#include <Windows.h>
#include <ShlObj.h>
#include <atlstr.h>
#include <vector>
using std::vector;


void Message(int i);
void Message(LPCWSTR wcs);
void Message(LPCSTR str);

#endif    // __STDAFX_H__
