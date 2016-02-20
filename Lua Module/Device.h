#ifndef DEVICE_H
#define DEVICE_H

#include <Windows.h>
#include <dinput.h>
#include "DirectOutputImpl.h"
#include "lua.hpp"
#include "ThreadLock.h"

extern void CALLBACK DO_PageChange(void* hDevice, DWORD dwPage, bool bSetActive, void* pCtxt);
extern void CALLBACK DO_SoftButtonChange(void* hDevice, DWORD dwButtons, void* pCtxt);
extern void CALLBACK DO_Enumerate(void* hDevice, void* pCtxt);
extern void CALLBACK DO_DeviceChange(void* hDevice, bool bAdded, void* pCtxt);

#define HID_COUNT		10
#define HID_EMPTY		0
#define HID_NOTFOUND	-1
#define LUA_COUNT		10

class ScriptManager;
class DeviceManager;

extern DeviceManager *DevMan;
extern ScriptManager *LuaMan;

typedef struct _SaitekDevice {
	void *hDevice;
	GUID type;
	GUID instance;
	bool isActive;
	DWORD oldButtonState;

	char name[32];
	char instanceID[64];
	char serialNumber[32];
} SaitekDevice;

extern SaitekDevice HID[HID_COUNT];
extern int HIDCount;

int ToDeviceShortName(const char *type);
int GetDeviceShortName(GUID type);
const char *GetDeviceStringName(GUID type);

int HIDLookupByGUID(void* hDevice, bool shouldAdd);
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

	CThreadLock _lockDI;
	CThreadLock _lockDO;
	CThreadLock _lockInit;

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

public:
	static DeviceManager *GetInstance() {
		static DeviceManager *instance = new DeviceManager();
		return instance;
	}

	CDirectOutput *DO() { return _do; }
	LPDIRECTINPUT8 DI() { return _di; }
	bool IsInitialized() { return (_initializedCounter > 0); }

	void GetDeviceInfo(void* hDevice, DeviceData &dd);
	void GetDeviceInfo(const GUID &iid, DeviceData &dd);
	
	int HandleDeviceChange(void* hDevice, bool bAdded);
	int LookupByHandle(void* hDevice);

	void Initialize();
	void Release();
};

#endif