#ifndef DEVICE_H
#define DEVICE_H

#include <Windows.h>
#include <dinput.h>

#include "Import/Source/DirectOutputImpl.h"
#include "Import/Source/ThreadLock.h"

#define HID_COUNT		10
#define HID_EMPTY		0
#define HID_NOTFOUND	-1

typedef struct _SaitekDevice {
	void *hDevice;
	GUID type;
	GUID instance;
	bool isActive;
	DWORD oldButtonState;

	// TODO: use DeviceData
	char name[32];
	char instanceID[64];
	char serialNumber[32];
} SaitekDevice;

extern SaitekDevice HID[HID_COUNT];
extern int HIDCount;

int ToDeviceShortName(const char *type);
int GetDeviceShortName(GUID type);
const char *GetDeviceStringName(GUID type);

int HIDLookupByType(const char *type, int index);
int HIDLookupByIndex(int index);

typedef struct _DeviceData {
	char name[32];
	char instanceID[64];
	char serialNumber[32];
} DeviceData;

class DeviceManager {

private:
	CDirectOutput *_do;
	LPDIRECTINPUT8 _di;

	CThreadLock _lockInit;
	CThreadLock _lockHID;

	int _initializedCounter;

	DeviceManager() {
		_initializedCounter = 0;
	}

	~DeviceManager() {
		_initializedCounter = 1;
		Release();
	}

	int LookupByDeviceInfo(GUID &type, DeviceData &dd, bool isActive);
	int LookupByDeviceInfo(void *hDevice, bool isActive);
	int Prepare(void* hDevice);
	void Set(int index);

	void GetDeviceInfo(void* hDevice, DeviceData &dd);
	void GetDeviceInfo(const GUID &iid, DeviceData &dd);

	LPDIRECTINPUT8 DI() { return _di; }

public:
	static DeviceManager *GetInstance() {
		static DeviceManager *instance = new DeviceManager();
		return instance;
	}

	CDirectOutput *DO() { return _do; }
	bool IsInitialized() { return (_initializedCounter > 0); }

	int HandleDeviceChange(void* hDevice, bool bAdded);
	int LookupByHandle(void* hDevice);

	void Initialize();
	void Release();
};

extern DeviceManager *DevMan;

#endif
