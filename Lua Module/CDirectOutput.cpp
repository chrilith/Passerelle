#include <Windows.h>
#include <tchar.h>
#include "CDirectOutput.h"

static bool SearchLibraryFilename(LPTSTR key, LPTSTR value, LPTSTR filename, DWORD length) {
	HKEY hk;
	bool result(false);
	// Read the Full Path to DirectOutput.dll from the registry
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_READ, &hk);

	if (lRet == ERROR_SUCCESS) {
		DWORD size(length * sizeof(filename[0]));

		// Note: this DirectOutput entry will point to the correct version on x86 or x64 systems
		lRet = RegQueryValueEx(hk, value, 0, 0, (LPBYTE)filename, &size);
		if (lRet == ERROR_SUCCESS) {
			result = true;
		}
		RegCloseKey(hk);
	}

	return result;
}

bool CDirectOutput::GetDirectOutputFilename(LPTSTR filename, DWORD length) {
	const LPTSTR kv[] = {
		_T("SOFTWARE\\Logitech\\DirectOutput"),	_T("DirectOutput"),
		_T("SOFTWARE\\Saitek\\DirectOutput"),	_T("DirectOutput_Saitek"),
		_T("SOFTWARE\\Saitek\\DirectOutput"),	_T("DirectOutput"),
		_T("SOFTWARE\\Saitek\\DirectOutput"),	_T("DirectOutputX86")
	};

	for (int i = 0; i < ARRAYSIZE(kv) / 2; i += 2) {
		if (SearchLibraryFilename(kv[i + 0], kv[i + 1], filename, length)) {
			return true;
		}
	}
	return false;
}

CDirectOutput::CDirectOutput() :
	DO_INIT(module),
	DO_INIT(Initialize),
	DO_INIT(Deinitialize),
	DO_INIT(RegisterDeviceCallback),
	DO_INIT(Enumerate),
	DO_INIT(RegisterPageCallback),
	DO_INIT(RegisterSoftButtonCallback),
	DO_INIT(GetDeviceType),
	DO_INIT(GetDeviceInstance),
	DO_INIT(SetProfile),
	DO_INIT(AddPage),
	DO_INIT(RemovePage),
	DO_INIT(SetLed),
	DO_INIT(SetString),
	DO_INIT(SetImage),
	DO_INIT(SetImageFromFile),
	DO_INIT(StartServer),
	DO_INIT(CloseServer),
	DO_INIT(SendServerFile),
	DO_INIT(SaveFile),
	DO_INIT(DisplayFile),
	DO_INIT(DeleteFile) {

	TCHAR filename[2048] = { 0 };

	if (GetDirectOutputFilename(filename, ARRAYSIZE(filename))) {
		m_module = LoadLibrary(filename);
		if (m_module) {
			DO_PROC(Initialize);
			DO_PROC(Deinitialize);
			DO_PROC(RegisterDeviceCallback);
			DO_PROC(Enumerate);
			DO_PROC(RegisterPageCallback);
			DO_PROC(RegisterSoftButtonCallback);
			DO_PROC(GetDeviceType);
			DO_PROC(GetDeviceInstance);
			DO_PROC(SetProfile);
			DO_PROC(AddPage);
			DO_PROC(RemovePage);
			DO_PROC(SetLed);
			DO_PROC(SetString);
			DO_PROC(SetImage);
			DO_PROC(SetImageFromFile);
			DO_PROC(StartServer);
			DO_PROC(CloseServer);
			DO_PROC(SendServerFile);
			DO_PROC(SendServerMsg);
			DO_PROC(SaveFile);
			DO_PROC(DisplayFile);
			DO_PROC(DeleteFile);
		}
	}
}

CDirectOutput::~CDirectOutput() {
	if (m_module) {
		FreeLibrary(m_module);
	}
}

HRESULT CDirectOutput::Initialize(const wchar_t* wszPluginName) {
	DO_IMPL(Initialize, wszPluginName);
}

HRESULT CDirectOutput::Deinitialize() {
	DO_IMPL(Deinitialize);
}

HRESULT CDirectOutput::RegisterDeviceCallback(Pfn_DirectOutput_DeviceChange pfnCb, void* pCtxt) {
	DO_IMPL(RegisterDeviceCallback, pfnCb, pCtxt);
}

HRESULT CDirectOutput::Enumerate(Pfn_DirectOutput_EnumerateCallback pfnCb, void* pCtxt) {
	DO_IMPL(Enumerate, pfnCb, pCtxt);
}

HRESULT CDirectOutput::RegisterPageCallback(void* hDevice, Pfn_DirectOutput_PageChange pfnCb, void* pCtxt) {
	DO_IMPL(RegisterPageCallback, hDevice, pfnCb, pCtxt);
}

HRESULT CDirectOutput::RegisterSoftButtonCallback(void* hDevice, Pfn_DirectOutput_SoftButtonChange pfnCb, void* pCtxt) {
	DO_IMPL(RegisterSoftButtonCallback, hDevice, pfnCb, pCtxt);
}

HRESULT CDirectOutput::GetDeviceType(void* hDevice, LPGUID pGuid) {
	DO_IMPL(GetDeviceType, hDevice, pGuid);
}

HRESULT CDirectOutput::GetDeviceInstance(void* hDevice, LPGUID pGuid) {
	DO_IMPL(GetDeviceInstance, hDevice, pGuid);
}

HRESULT CDirectOutput::SetProfile(void* hDevice, DWORD cchProfile, const wchar_t* wszProfile) {
	DO_IMPL(SetProfile, hDevice, cchProfile, wszProfile);
}

HRESULT CDirectOutput::AddPage(void* hDevice, DWORD dwPage, const wchar_t* wszDebugName, DWORD dwFlags) {
	DO_IMPL(AddPage, hDevice, dwPage, dwFlags);
}

HRESULT CDirectOutput::RemovePage(void* hDevice, DWORD dwPage) {
	DO_IMPL(RemovePage, hDevice, dwPage);
}

HRESULT CDirectOutput::SetLed(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD dwValue) {
	DO_IMPL(SetLed, hDevice, dwPage, dwIndex, dwValue);
}

HRESULT CDirectOutput::SetString(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD cchValue, const wchar_t* wszValue) {
	DO_IMPL(SetString, hDevice, dwPage, dwIndex, cchValue, wszValue);
}
HRESULT CDirectOutput::SetImage(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD cbValue, const void* pvValue) {
	DO_IMPL(SetImage, hDevice, dwPage, dwIndex, cbValue, pvValue);
}

HRESULT CDirectOutput::SetImageFromFile(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD cchFilename, const wchar_t* wszFilename) {
	DO_IMPL(SetImageFromFile, hDevice, dwPage, dwIndex, cchFilename, wszFilename);
}

HRESULT CDirectOutput::StartServer(void* hDevice, DWORD cchFilename, const wchar_t* wszFilename, LPDWORD pdwServerId, PSRequestStatus psStatus) {
	DO_IMPL(StartServer, hDevice, cchFilename, wszFilename, pdwServerId, psStatus);
}

HRESULT CDirectOutput::CloseServer(void* hDevice, DWORD dwServerId, PSRequestStatus psStatus) {
	DO_IMPL(CloseServer, hDevice, dwServerId, psStatus);
}

HRESULT CDirectOutput::SendServerMsg(void* hDevice, DWORD dwServerId, DWORD dwRequest, DWORD dwPage, DWORD cbIn, const void* pvIn, DWORD cbOut, void* pvOut, PSRequestStatus psStatus) {
	DO_IMPL(SendServerMsg, hDevice, dwServerId, dwRequest, dwPage, cbIn, pvIn, cbOut, pvOut, psStatus);
}

HRESULT CDirectOutput::SendServerFile(void* hDevice, DWORD dwServerId, DWORD dwRequest, DWORD dwPage, DWORD cbInHdr, const void* pvInHdr, DWORD cchFile, const wchar_t* wszFile, DWORD cbOut, void* pvOut, PSRequestStatus psStatus) {
	DO_IMPL(SendServerFile, hDevice, dwServerId, dwRequest, dwPage, cbInHdr, pvInHdr, cchFile, wszFile, cbOut, pvOut, psStatus);
}

HRESULT CDirectOutput::SaveFile(void* hDevice, DWORD dwPage, DWORD dwFile, DWORD cchFilename, const wchar_t* wszFilename, PSRequestStatus psStatus) {
	DO_IMPL(SaveFile, hDevice, dwPage, dwFile, cchFilename, wszFilename, psStatus);
}

HRESULT CDirectOutput::DisplayFile(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD dwFile, PSRequestStatus psStatus) {
	DO_IMPL(DisplayFile, hDevice, dwPage, dwIndex, dwFile, psStatus);
}

HRESULT CDirectOutput::DeleteFile(void* hDevice, DWORD dwPage, DWORD dwFile, PSRequestStatus psStatus) {
	DO_IMPL(DeleteFile, hDevice, dwPage, dwFile, psStatus);
}
