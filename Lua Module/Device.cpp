#include "Common.h"
#include "Script.h"

#include <atlimage.h>
#include <hidsdi.h>
#include <SetupAPI.h>

#define HID_SFIP		'SFIP'
#define HID_X52P		'X52P'
#define HID_UNKN		'UNKN'

// CHECKME : Do we still need all this code since we now have a GetSerialNumber() in the DO API?

DeviceManager *DevMan = DeviceManager::GetInstance();

SaitekDevice HID[HID_COUNT];
int HIDCount = HID_EMPTY;

int ToDeviceShortName(const char *type) {
	if (strcmp(type, "SFIP") == 0)
		return HID_SFIP;
	if (strcmp(type, "X52P") == 0)
		return HID_X52P;
	return HID_UNKN;
}

int GetDeviceShortName(GUID type) {
	if (type == DeviceType_Fip)
		return HID_SFIP;
	if (type == DeviceType_X52Pro)
		return HID_X52P;
	return HID_UNKN;
}

const char *GetDeviceStringName(GUID type) {
	if (type == DeviceType_Fip)
		return "SFIP";
	if (type == DeviceType_X52Pro)
		return "X52P";
	return "UNKN";
}

int HIDLookupByType(const char *type, int index) {
	int count = 1;	// Index starts at 1 in Lua
	int dev = ToDeviceShortName(type);

	for (int i = 0; i < HIDCount; i++) {
		if (GetDeviceShortName(HID[i].type) == dev/* && HID[i].isActive*/) {
			if (count++ == index)
				return i;
		}
	}

	return HID_NOTFOUND;
}

int HIDLookupByIndex(int index) {
	int count = 1;	// Index starts at 1 in Lua
	int dev = GetDeviceShortName(HID[index].type);

	for (int i = 0; i < HIDCount; i++) {
		if (GetDeviceShortName(HID[i].type) == dev/* && HID[i].isActive*/) {
			if (index == i)
				return count;
			count++;
		}
	}

	return HID_NOTFOUND;
}

static void CALLBACK DO_Enumerate(void* hDevice, void* pCtxt) {
	DevMan->HandleDeviceChange(hDevice, true);
}

static void CALLBACK DO_DeviceChange(void* hDevice, bool bAdded, void* pCtxt) {
	int index = DevMan->HandleDeviceChange(hDevice, bAdded);
	LuaMan->CallDeviceChangeCallbacks(index, bAdded);
}

void DeviceManager::Initialize() {
	_lockInit.Acquire();
	if (_initializedCounter > 0) {
		_initializedCounter++;
		_lockInit.Release();
		return;
	}

	// Initialize...
	memset(HID, 0, sizeof(SaitekDevice) * HID_COUNT);
	HIDCount = HID_EMPTY;

	//Initialize DirectInput
	HRESULT hdi = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID *)&this->_di, NULL);
	if (!SUCCEEDED(hdi)) {
		_lockInit.Release();
		return;
	}

	//Initialize Saitek DirectOutput
	_do = new CDirectOutput();
	HRESULT hdo = _do->Initialize(L"" LUALIB_IDENT);
	if (!SUCCEEDED(hdo)) {
		_di->Release();
		_lockInit.Release();
		return;
	}

	// Register callbacks
	HRESULT h1 = _do->Enumerate((Pfn_DirectOutput_EnumerateCallback)DO_Enumerate, NULL);
	HRESULT h2 = _do->RegisterDeviceCallback((Pfn_DirectOutput_DeviceChange)DO_DeviceChange, NULL);

	// Everything OK
	_initializedCounter = 1;
	_lockInit.Release();
}

void DeviceManager::Release() {
	_lockInit.Acquire();
	if (_initializedCounter-- > 1) {
		_lockInit.Release();
		return;
	}

	_do->Deinitialize();
	_di->Release();
	_initializedCounter = 0;
	_lockInit.Release();
}

void DeviceManager::GetDeviceInfo(void *hDevice, DeviceData &dd) {
	GUID gi;
	HRESULT hr = DO()->GetDeviceInstance(hDevice, &gi);
	GetDeviceInfo(gi, dd);
}

void DeviceManager::GetDeviceInfo(const GUID &iid, DeviceData &dd) {
	SP_DEVINFO_DATA DeviceInfoData;
	SP_DEVICE_INTERFACE_DATA did;
	struct { DWORD cbSize; TCHAR DevicePath[256]; } ciddd;

	TCHAR s[64];
	GUID HidGuid;

	//Empty
	dd.instanceID[0] = 0;
	dd.name[0] = 0;
	dd.serialNumber[0] = 0;

	// Try to create a device
	LPDIRECTINPUTDEVICE8 pDevice;
	HRESULT hd = DevMan->DI()->CreateDevice(iid, &pDevice, NULL);
	if (FAILED(hd)) {
		return;
	}
	// Get the GUID and Path
	DIPROPGUIDANDPATH h;
	h.diph.dwSize = sizeof(DIPROPGUIDANDPATH);
	h.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	h.diph.dwObj = 0;
	h.diph.dwHow = DIPH_DEVICE;
	HRESULT hp = pDevice->GetProperty(DIPROP_GUIDANDPATH, (LPDIPROPHEADER)&h);
	if (FAILED(hd))
		return;

	// Change # to \ to match structure of instance ID
	for (size_t i = 0; i < wcslen(h.wszPath); i++) {
		if (h.wszPath[i] == L'#') {
			h.wszPath[i] = L'\\';
		}
	}

	// Prepare enumeration
	HidD_GetHidGuid(&HidGuid);
	HDEVINFO hdi = SetupDiGetClassDevs(&HidGuid, NULL, NULL, DIGCF_PRESENT|DIGCF_DEVICEINTERFACE);

	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	did.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	ciddd.cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

	for (int i = 0; SetupDiEnumDeviceInterfaces(hdi, 0, &HidGuid, i, &did); i++) {
		if (!SetupDiGetDeviceInterfaceDetail(hdi, &did, PSP_INTERFACE_DEVICE_DETAIL_DATA(&ciddd), sizeof(ciddd.DevicePath), 0, &DeviceInfoData))
			continue;
		if (!SetupDiGetDeviceInstanceId(hdi, &DeviceInfoData, s, sizeof(s), 0))
			continue;
		_wcslwr_s(s);
		if(!wcsstr(h.wszPath, s))
			continue;
		strncpy_s(dd.instanceID, CT2A(s), sizeof(dd.instanceID) - 1);

		HANDLE h = CreateFile(ciddd.DevicePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		if (HidD_GetProductString(h, s, sizeof(s)))
			strncpy_s(dd.name, CT2A(s), sizeof(dd.name) - 1);
		if (HidD_GetSerialNumberString(h, s, sizeof(s)))
			strncpy_s(dd.serialNumber, CT2A(s), sizeof(dd.serialNumber) - 1);
		CloseHandle(h);
	}

	SetupDiDestroyDeviceInfoList(hdi);
}

int DeviceManager::Prepare(void *hDevice) {
	if (HIDCount == HID_COUNT)
		return HID_NOTFOUND;

	int index = HIDCount++;
	HID[index].hDevice = hDevice;
	return index;
}

void DeviceManager::Set(int index) {
	void *hDevice = HID[index].hDevice;

	GUID gt, gi;
	DeviceData dd;

	GetDeviceInfo(hDevice, dd);

	DO()->GetDeviceType(hDevice, &gt);
	DO()->GetDeviceInstance(hDevice, &gi);

	HID[index].type = gt;
	HID[index].instance = gi;
	
	strcpy_s(HID[index].instanceID, dd.instanceID);
	strcpy_s(HID[index].name, dd.name);
	strcpy_s(HID[index].serialNumber, dd.serialNumber);
}

int DeviceManager::HandleDeviceChange(void *hDevice, bool bAdded) {
	int index = LookupByHandle(hDevice);

	_lockHID.Acquire();
	if (bAdded) {
		if (index == HID_NOTFOUND)
			index = LookupByDeviceInfo(hDevice, false);
		if (index == HID_NOTFOUND)
			index = Prepare(hDevice);
	}

	if (index != HID_NOTFOUND) {
		HID[index].isActive = bAdded;
		if (bAdded) {
			HID[index].hDevice = hDevice;
			Set(index);
		}
	}
	_lockHID.Release();

	return index;
}


int DeviceManager::LookupByHandle(void* hDevice) {
	_lockHID.Acquire();

	for (int i = 0; i < HIDCount; i++) {
		if (hDevice == HID[i].hDevice) {
			_lockHID.Release();
			return i;
		}
	}

	_lockHID.Release();
	return HID_NOTFOUND;
}

int DeviceManager::LookupByDeviceInfo(void *hDevice, bool isActive) {
	GUID gt;
	DeviceData dd;

	DO()->GetDeviceType(hDevice, &gt);
	GetDeviceInfo(hDevice, dd);

	return LookupByDeviceInfo(gt, dd, isActive);
}

int DeviceManager::LookupByDeviceInfo(GUID &type, DeviceData &dd, bool isActive) {
	for (int i = 0; i < HIDCount; i++) {
		if (HID[i].isActive == isActive && HID[i].type == type &&
			strcmp(dd.instanceID, HID[i].instanceID) == 0 &&
			strcmp(dd.serialNumber, HID[i].serialNumber) == 0) {
			return i;
		}
	}
	return HID_NOTFOUND;
}
