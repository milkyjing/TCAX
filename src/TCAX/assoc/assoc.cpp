#include <windows.h>
#include <atlstr.h>
#include <shlobj.h>


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	int      argc;
	LPWSTR  *argv;
	HKEY     hKey;
	BOOL     bShowMsg = TRUE;
	DWORD    dwDisp;
	argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
	CStringW wcsPath = argv[0];
	WCHAR    szSysDir[256];
	CStringW wcsSysDir;
	::LocalFree(argv);
	wcsPath = wcsPath.Mid(0, wcsPath.ReverseFind(L'\\') + 1);    // note: '\\' is included in the Path
	::GetSystemDirectoryW(szSysDir, 256);
	wcsSysDir = szSysDir;

	// --- for TCC file
	// --- Reg ".tcc" in "HKEY_CLASSES_ROOT"
	if (::RegCreateKeyExW(	HKEY_CLASSES_ROOT, L".tcc", 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_WRITE, 
							NULL, &hKey, &dwDisp) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	::RegSetValueW(hKey, NULL, REG_SZ, L"TCC_File_Type", 0);
	::RegCloseKey(hKey);
	// --- Reg "shell\open\command" of "TCC_File_Type" in "HKEY_CLASSES_ROOT"
	if (::RegCreateKeyExW(	HKEY_CLASSES_ROOT, L"TCC_File_Type\\shell\\open\\command", 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_WRITE, 
							NULL, &hKey, &dwDisp) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	::RegSetValueW(hKey, NULL, REG_SZ, L"\"" + wcsSysDir + L"\\notepad.exe\" \"%1\"", 0);
	::RegCloseKey(hKey);
	// --- Reg "shell\edit\command" of "TCC_File_Type" in "HKEY_CLASSES_ROOT"
	if (::RegCreateKeyExW(	HKEY_CLASSES_ROOT, L"TCC_File_Type\\shell\\edit\\command", 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_WRITE, 
							NULL, &hKey, &dwDisp) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	::RegSetValueW(hKey, NULL, REG_SZ, L"\"" + wcsSysDir + L"\\notepad.exe\" \"%1\"", 0);
	::RegCloseKey(hKey);
	// --- Reg "shell\parse" of "TCC_File_Type" in "HKEY_CLASSES_ROOT"
	if (::RegCreateKeyExW(	HKEY_CLASSES_ROOT, L"TCC_File_Type\\shell\\parse", 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_WRITE, 
							NULL, &hKey, &dwDisp) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	::RegSetValueW(hKey, NULL, REG_SZ, L"Parse", 0);
	::RegCloseKey(hKey);
	// --- Reg "shell\parse\command" of "TCC_File_Type" in "HKEY_CLASSES_ROOT"
	if (::RegCreateKeyExW(	HKEY_CLASSES_ROOT, L"TCC_File_Type\\shell\\parse\\command", 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_WRITE, 
							NULL, &hKey, &dwDisp) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	::RegSetValueW(hKey, NULL, REG_SZ, L"\"" + wcsPath + L"tcax.exe\" \"%L\"", 0);
	::RegCloseKey(hKey);
	// --- Reg "DefaultIcon" of "TCC_File_Type" in "HKEY_CLASSES_ROOT"
	if (::RegCreateKeyExW(	HKEY_CLASSES_ROOT, L"TCC_File_Type\\DefaultIcon", 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_WRITE, 
							NULL, &hKey, &dwDisp) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	::RegSetValueW(hKey, NULL, REG_SZ, wcsPath + L"icons\\tcc.ico", 0);
	::RegCloseKey(hKey);

	// --- for ASS file
	// --- Reg ".ass" in "HKEY_CLASSES_ROOT"
	if (::RegCreateKeyExW(	HKEY_CLASSES_ROOT, L".ass", 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_WRITE, 
							NULL, &hKey, &dwDisp) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	::RegSetValueW(hKey, NULL, REG_SZ, L"ASS_File_Type", 0);
	::RegCloseKey(hKey);
	// --- Reg "DefaultIcon" of "ASS_File_Type" in "HKEY_CLASSES_ROOT"
	if (::RegCreateKeyExW(	HKEY_CLASSES_ROOT, L"ASS_File_Type\\DefaultIcon", 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_WRITE, 
							NULL, &hKey, &dwDisp) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	::RegSetValueW(hKey, NULL, REG_SZ, wcsPath + L"icons\\ass.ico", 0);
	::RegCloseKey(hKey);

	// --- for tcas file
	// --- Reg ".tcas" in "HKEY_CLASSES_ROOT"
	if (::RegCreateKeyExW(	HKEY_CLASSES_ROOT, L".tcas", 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_WRITE, 
							NULL, &hKey, &dwDisp) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	::RegSetValueW(hKey, NULL, REG_SZ, L"TCAS_File_Type", 0);
	::RegCloseKey(hKey);
	// --- Reg "shell\open\command" of "TCAS_File_Type" in "HKEY_CLASSES_ROOT"
	if (::RegCreateKeyExW(	HKEY_CLASSES_ROOT, L"TCAS_File_Type\\shell\\open\\command", 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_WRITE, 
							NULL, &hKey, &dwDisp) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	::RegSetValueW(hKey, NULL, REG_SZ, L"\"" + wcsPath + L"tools\\timeShift.exe\" \"%L\"", 0);
	::RegCloseKey(hKey);
	// --- Reg "DefaultIcon" of "TCAS_File_Type" in "HKEY_CLASSES_ROOT"
	if (::RegCreateKeyExW(	HKEY_CLASSES_ROOT, L"TCAS_File_Type\\DefaultIcon", 0, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_WRITE, 
							NULL, &hKey, &dwDisp) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	::RegSetValueW(hKey, NULL, REG_SZ, wcsPath + L"icons\\tcas.ico", 0);
	::RegCloseKey(hKey);
	::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT, 0, 0);
    ::MessageBoxW(NULL, L"TCAX file association successfully executed.\n\nTCAX程序文件关联成功执行, 请点确定.", L"TCAX - Info", MB_OK | MB_ICONINFORMATION);
	return TRUE;
}

