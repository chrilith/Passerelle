#pragma once
#include "Include\DirectOutput.h"

#define DO_VAR(name)	m_##name
#define DO_DEF(name)	Pfn_DirectOutput_##name DO_VAR(name)
#define DO_INIT(name)	DO_VAR(name)(0)
#define DO_PROC(name)	DO_VAR(name) = (Pfn_DirectOutput_##name)GetProcAddress(m_module, "DirectOutput_" #name)

#define DO_IMPL(name, ...) \
	if (m_module && DO_VAR(name)) {	\
		return DO_VAR(name)(__VA_ARGS__); \
	} \
	return E_NOTIMPL


class CDirectOutput {
public:
	 CDirectOutput();
	~CDirectOutput();

	HRESULT Initialize(const wchar_t* wszPluginName);
	HRESULT Deinitialize();
	HRESULT RegisterDeviceCallback(Pfn_DirectOutput_DeviceChange pfnCb, void* pCtxt);
	HRESULT Enumerate(Pfn_DirectOutput_EnumerateCallback pfnCb, void* pCtxt);
	HRESULT RegisterPageCallback(void* hDevice, Pfn_DirectOutput_PageChange pfnCb, void* pCtxt);
	HRESULT RegisterSoftButtonCallback(void* hDevice, Pfn_DirectOutput_SoftButtonChange pfnCb, void* pCtxt);
	HRESULT GetDeviceType(void* hDevice, LPGUID pGuid);
	HRESULT GetDeviceInstance(void* hDevice, LPGUID pGuid);
	HRESULT SetProfile(void* hDevice, DWORD cchProfile, const wchar_t* wszProfile);
	HRESULT AddPage(void* hDevice, DWORD dwPage, const wchar_t* wszDebugName, DWORD dwFlags);
	HRESULT RemovePage(void* hDevice, DWORD dwPage);
	HRESULT SetLed(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD dwValue);
	HRESULT SetString(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD cchValue, const wchar_t* wszValue);
	HRESULT SetImage(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD cbValue, const void* pvValue);
	HRESULT SetImageFromFile(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD cchFilename, const wchar_t* wszFilename);
	HRESULT StartServer(void* hDevice, DWORD cchFilename, const wchar_t* wszFilename, LPDWORD pdwServerId, PSRequestStatus psStatus);
	HRESULT CloseServer(void* hDevice, DWORD dwServerId, PSRequestStatus psStatus);
	HRESULT SendServerMsg(void* hDevice, DWORD dwServerId, DWORD dwRequest, DWORD dwPage, DWORD cbIn, const void* pvIn, DWORD cbOut, void* pvOut, PSRequestStatus psStatus);
	HRESULT SendServerFile(void* hDevice, DWORD dwServerId, DWORD dwRequest, DWORD dwPage, DWORD cbInHdr, const void* pvInHdr, DWORD cchFile, const wchar_t* wszFile, DWORD cbOut, void* pvOut, PSRequestStatus psStatus);
	HRESULT SaveFile(void* hDevice, DWORD dwPage, DWORD dwFile, DWORD cchFilename, const wchar_t* wszFilename, PSRequestStatus psStatus);
	HRESULT DisplayFile(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD dwFile, PSRequestStatus psStatus);
	HRESULT DeleteFile(void* hDevice, DWORD dwPage, DWORD dwFile, PSRequestStatus psStatus);

private:
	HMODULE	m_module;

	DO_DEF(Initialize);
	DO_DEF(Deinitialize);
	DO_DEF(RegisterDeviceCallback);
	DO_DEF(Enumerate);
	DO_DEF(RegisterPageCallback);
	DO_DEF(RegisterSoftButtonCallback);
	DO_DEF(GetDeviceType);
	DO_DEF(GetDeviceInstance);
	DO_DEF(SetProfile);
	DO_DEF(AddPage);
	DO_DEF(RemovePage);
	DO_DEF(SetLed);
	DO_DEF(SetString);
	DO_DEF(SetImage);
	DO_DEF(SetImageFromFile);
	DO_DEF(StartServer);
	DO_DEF(CloseServer);
	DO_DEF(SendServerFile);
	DO_DEF(SendServerMsg);
	DO_DEF(SaveFile);
	DO_DEF(DisplayFile);
	DO_DEF(DeleteFile);

	bool GetDirectOutputFilename(LPTSTR filename, DWORD length);
};
