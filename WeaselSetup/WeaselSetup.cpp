// WeaselSetup.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "WeaselSetup.h"
#include "InstallOptionsDialog.h"

CAppModule _Module;

static int Run(LPTSTR lpCmdLine);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	HRESULT hRes = ::CoInitialize(NULL);
	// If you are running on NT 4.0 or higher you can use the following call instead to
	// make the EXE free threaded. This means that calls come in on a random RPC thread.
	//HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int ret = Run(lpCmdLine);

	_Module.Term();
	::CoUninitialize();

	return ret;
}

int install(InstallRegion region, bool silent);
int uninstall(bool silent);
bool has_installed();

static int CustomInstall(bool installing)
{
	bool hant = false;
	bool silent = false;
	std::wstring user_dir;
	InstallRegion region = InstallRegion::CN;

	const WCHAR KEY[] = L"Software\\Rime\\Weasel";
	HKEY hKey;
	LSTATUS ret = RegOpenKey(HKEY_CURRENT_USER, KEY, &hKey);
	if (ret == ERROR_SUCCESS)
	{
		WCHAR value[MAX_PATH];
		DWORD len = sizeof(value);
		DWORD type = 0;
		DWORD data = 0;
		ret = RegQueryValueEx(hKey, L"RimeUserDir", NULL, &type, (LPBYTE)value, &len);
		if (ret == ERROR_SUCCESS && type == REG_SZ)
		{
			user_dir = value;
		}
		len = sizeof(data);
		// legacy: old installation on Hant
		ret = RegQueryValueEx(hKey, L"Hant", NULL, &type, (LPBYTE)&data, &len);
		if (ret == ERROR_SUCCESS && type == REG_DWORD)
		{
			hant = true;
			region = data != 0 ? InstallRegion::TW : InstallRegion::CN;
			if (installing)
				silent = true;
		}
		ret = RegQueryValueEx(hKey, L"Region", NULL, &type, (LPBYTE)&data, &len);
		if (ret == ERROR_SUCCESS && type == REG_DWORD)
		{
			region = (InstallRegion)data;
		}
		RegCloseKey(hKey);
	}

	if (!silent)
	{
		InstallOptionsDialog dlg;
		dlg.installed = has_installed();
		dlg.region = region;
		//dlg.hant = hant;
		dlg.user_dir = user_dir;
		if (IDOK != dlg.DoModal()) {
			if (!installing)
				return 1;  // aborted by user
		}
		else {
			region = dlg.region;
			user_dir = dlg.user_dir;
		}
	}
	if (0 != install(region, silent))
		return 1;

	ret = RegCreateKeyEx(HKEY_CURRENT_USER, KEY,
		                 0, NULL, 0, KEY_ALL_ACCESS, 0, &hKey, NULL);
	if (FAILED(HRESULT_FROM_WIN32(ret)))
	{
		MessageBox(NULL, KEY, L"安裝失敗", MB_ICONERROR | MB_OK);
		return 1;
	}

	ret = RegSetValueEx(hKey, L"RimeUserDir", 0, REG_SZ,
		                (const BYTE*)user_dir.c_str(),
						(user_dir.length() + 1) * sizeof(WCHAR));
	if (FAILED(HRESULT_FROM_WIN32(ret)))
	{
		MessageBox(NULL, L"無法寫入 RimeUserDir", L"安裝失敗", MB_ICONERROR | MB_OK);
		return 1;
	}

	//DWORD data = hant ? 1 : 0;
	//ret = RegSetValueEx(hKey, L"Hant", 0, REG_DWORD, (const BYTE*)&data, sizeof(DWORD));
	//if (FAILED(HRESULT_FROM_WIN32(ret)))
	//{
	//	MessageBox(NULL, L"無法寫入 Hant", L"安裝失敗", MB_ICONERROR | MB_OK);
	//	return 1;
	//}
	if (hant) {
		RegDeleteValueW(hKey, L"Hant");
	}
	DWORD data = (DWORD)region;
	ret = RegSetValueEx(hKey, L"Region", 0, REG_DWORD, (const BYTE*)&data, sizeof(DWORD));
	if (FAILED(HRESULT_FROM_WIN32(ret)))
	{
		MessageBox(NULL, L"無法寫入安裝語言區域", L"安裝失敗", MB_ICONERROR | MB_OK);
		return 1;
	}
	return 0;
}

static int Run(LPTSTR lpCmdLine)
{
	const bool silent = true;
	bool uninstalling = !wcscmp(L"/u", lpCmdLine);
	if (uninstalling)
		return uninstall(silent);
	if (!wcscmp(L"/CN", lpCmdLine)) {
		return install(InstallRegion::CN, silent);
	}
	if (!wcscmp(L"/TW", lpCmdLine)) {
		return install(InstallRegion::TW, silent);
	}
	if (!wcscmp(L"/SG", lpCmdLine)) {
		return install(InstallRegion::SG, silent);
	}
	if (!wcscmp(L"/MO", lpCmdLine)) {
		return install(InstallRegion::MO, silent);
	}
	if (!wcscmp(L"/HK", lpCmdLine)) {
		return install(InstallRegion::HK, silent);
	}

	bool installing = !wcscmp(L"/i", lpCmdLine);
	return CustomInstall(installing);
}
