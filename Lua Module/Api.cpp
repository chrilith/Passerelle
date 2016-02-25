#include "Common.h"
#include "Script.h"
#include "resource.h"
#include "Api.h"

#include "Import/Source/RawImage.h"
#include "lua.hpp"

#ifdef DEBUG
extern void print(lua_State* L, const char *s);
#else
#	define print(a, b)
#endif

#define GD_HEADER_SIZE 24

extern int HIDCount;
extern SaitekDevice HID[HID_COUNT];

extern int charToWideConverter(const char *s, wchar_t **d);
extern void s_RenderImage(HDC hdc, LPCTSTR tsz);

static int GetAPIError(HRESULT hResult) {
	switch (hResult) {
	case S_OK:
		return 0;
	case E_HANDLE:
		return 1;
	case E_NOTIMPL:
		return 2;
	case E_INVALIDARG:
		return 3;
	case E_PAGENOTACTIVE:
		return 4;
	case E_OUTOFMEMORY:
		return 5;
	default:	// Unknown
		return -1;
	}
}

static void CALLBACK DO_PageChange(void* hDevice, DWORD dwPage, bool bSetActive, void* pCtxt) {
	int index = DevMan->LookupByHandle(hDevice);
	if (index == HID_NOTFOUND || !HID[index].isActive)
		return;
	LuaMan->CallPageChangeCallbacks(index, dwPage, bSetActive);
}

static void CALLBACK DO_SoftButtonChange(void* hDevice, DWORD dwButtons, void* pCtxt) {
	int index = DevMan->LookupByHandle(hDevice);
	if (index == HID_NOTFOUND || !HID[index].isActive)
		return;
	LuaMan->CallSoftButtonCallbacks(index, dwButtons);
}

LUA_FUNC(GetVersion) {
	lua_pushstring(L, LIB_VERSION);
	lua_pushnumber(L, LIB_VERSION_MAJOR);
	lua_pushnumber(L, LIB_VERSION_MINOR);
	lua_pushnumber(L, LIB_VERSION_BUILD);
	lua_pushnumber(L, LIB_VERSION_PATCH);
	return 5;
}

LUA_FUNC(Initialize) {
	return 0;
}

LUA_FUNC(Release) {
	return 0;
}

LUA_FUNC(Listen) {
	fgetc(stdin);
	return 0;
}

LUA_FUNC(GetNumDevices) {
	if (!DevMan->IsInitialized())
		return 0;
	int p = lua_gettop(L);

	if (/* Base signature */
			!(p == 0) &&
		/* Per device */
			!(p == 1 &&
			  lua_isstring(L, 1))
	) { return 0; }

	int count = 0;
	int dev = 0;

	if (p == 1) {
		const char *nameP = lua_tostring(L, 1);
		dev = ToDeviceShortName(nameP);
	}

	for (int i = 0; i < HIDCount; i++) {
		if (HID[i].isActive) {
			if (dev == 0 || GetDeviceShortName(HID[i].type) == dev) {
				count++;
			}
		}
	}

	lua_pushnumber(L, count);
	return 1;
}

LUA_FUNC(AddPage) {
	if (!DevMan->IsInitialized())
		return 0;
	int p = lua_gettop(L);

	if (/* Base signature */
		!(p == 4 &&
			lua_isstring(L, 1) &&	// Model Name
			lua_isnumber(L, 2) &&	// Model Index
			lua_isnumber(L, 3) &&	// Page ID
			lua_isboolean(L, 4))	// Set Active
	) { return 0; }

	const char *devMod = lua_tostring(L, 1);
	int devIdx = (int)lua_tonumber(L, 2);
	int pageID = (int)lua_tonumber(L, 3);
	int setActive = (lua_toboolean(L, 4) == 1);

	devIdx = HIDLookupByType(devMod, devIdx);
	if (!(devIdx >= 0 && devIdx < HIDCount && HID[devIdx].isActive))
		return 0;

	HRESULT h = DevMan->DO()->AddPage(HID[devIdx].hDevice, pageID, 0, setActive ? FLAG_SET_AS_ACTIVE : 0);
	lua_pushnumber(L, GetAPIError(h));
	return 1;
}

LUA_FUNC(RemovePage) {
	if (!DevMan->IsInitialized())
		return 0;
	int p = lua_gettop(L);

	if (/* Base signature */
		!(p == 3 &&
			lua_isstring(L, 1) &&	// Model Name
			lua_isnumber(L, 2) &&	// Model Index
			lua_isnumber(L, 3))		// Page ID
	) { return 0; }

	const char *devMod = lua_tostring(L, 1);
	int devIdx = (int)lua_tonumber(L, 2);
	int pageID = (int)lua_tonumber(L, 3);

	devIdx = HIDLookupByType(devMod, devIdx);
	if (!(devIdx >= 0 && devIdx < HIDCount && HID[devIdx].isActive))
		return 0;

	HRESULT h = DevMan->DO()->RemovePage(HID[devIdx].hDevice, pageID);
	lua_pushnumber(L, GetAPIError(h));
	return 1;
}

LUA_FUNC(RegisterDeviceChangeCallback) {
	if (!DevMan->IsInitialized())
		return 0;
	int p = lua_gettop(L);

	if (/* Base signature */
		!(p == 1 &&
		(lua_isstring(L, 1)
			|| lua_isnil(L, 1)
			|| lua_isfunction(L, 1)))	// Callback
	) { return 0; }

	int luaRef = LUA_REFNIL;

	if (lua_isstring(L, 1)) {
		const char *nameP = lua_tostring(L, 1);
		lua_getfield(L, LUA_GLOBALSINDEX, nameP);
		luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
	} else if (lua_isfunction(L, 1)) {
		luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
	}	

	ScriptInfo *luaScr = LuaMan->GetSlot(L);
	if (luaScr == NULL)
		return 0;
	if (luaScr->deviceChangeCallbackRef != LUA_REFNIL)
		luaL_unref(L, LUA_REGISTRYINDEX, luaScr->deviceChangeCallbackRef);
	luaScr->deviceChangeCallbackRef = luaRef;

	lua_pushnumber(L, GetAPIError(S_OK));
	return 1;
}

LUA_FUNC(RegisterPageCallback) {
	if (!DevMan->IsInitialized())
		return 0;
	int p = lua_gettop(L);

	if (/* Base signature */
		!(p == 3 &&
		lua_isstring(L, 1) &&						// Model Name
		lua_isnumber(L, 2) &&						// Model Index
		(lua_isstring(L, 3)
			|| lua_isnil(L, 3)
			|| lua_isfunction(L, 3)))	// Callback
	) { return 0; }

	const char *devMod = lua_tostring(L, 1);
	int devIdx = (int)lua_tonumber(L, 2);
	int luaRef = LUA_REFNIL;

	devIdx = HIDLookupByType(devMod, devIdx);
	if (!(devIdx >= 0 && devIdx < HIDCount && HID[devIdx].isActive))
		return 0;

	if (lua_isstring(L, 3)) {
		const char *nameP = lua_tostring(L, 3);
		lua_getfield(L, LUA_GLOBALSINDEX, nameP);
		luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
	} else if (lua_isfunction(L, 3)) {
		luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
	}	

	ScriptInfo *luaScr = LuaMan->GetSlot(L);
	if (luaScr == NULL)
		return 0;
	if (luaScr->HID[devIdx].pageButtonCallbackRef != LUA_REFNIL)
		luaL_unref(L, LUA_REGISTRYINDEX, luaScr->HID[devIdx].pageButtonCallbackRef);
	luaScr->HID[devIdx].pageButtonCallbackRef = luaRef;

	HRESULT h;
	if (luaRef != LUA_REFNIL) {
		h = DevMan->DO()->RegisterPageCallback(HID[devIdx].hDevice, (Pfn_DirectOutput_PageChange)DO_PageChange, NULL);
	} else {
		h = DevMan->DO()->RegisterPageCallback(HID[devIdx].hDevice, NULL, NULL);
	}
	lua_pushnumber(L, GetAPIError(h));
	return 1;
}

LUA_FUNC(RegisterSoftButtonCallback) {
	if (!DevMan->IsInitialized())
		return 0;
	int p = lua_gettop(L);

	if (/* Base signature */
			!(p == 3 &&
		lua_isstring(L, 1) &&			// Model Name
		lua_isnumber(L, 2) &&			// Model Index
		(lua_isstring(L, 3)
			|| lua_isnil(L, 3)
			|| lua_isfunction(L, 3)))	// Callback
	) { return 0; }

	const char *devMod = lua_tostring(L, 1);
	int devIdx = (int)lua_tonumber(L, 2);
	int luaRef = LUA_REFNIL;

	devIdx = HIDLookupByType(devMod, devIdx);
	if (!(devIdx >= 0 && devIdx < HIDCount && HID[devIdx].isActive))
		return 0;

	ScriptInfo *luaScr = LuaMan->GetSlot(L);
	if (luaScr == NULL)
		return 0;
	bool hasCallback = (luaScr->HID[devIdx].softButtonCallbackRef != LUA_REFNIL);
	bool hasOtherCallback = (luaScr->HID[devIdx].softButUpCallbackRef != LUA_REFNIL ||
							 luaScr->HID[devIdx].softButDownCallbackRef != LUA_REFNIL);

	if (lua_isstring(L, 3)) {
		const char *nameP = lua_tostring(L, 3);
		lua_getfield(L, LUA_GLOBALSINDEX, nameP);
		luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
	} else if (lua_isfunction(L, 3)) {
		luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
	}	

	if (hasCallback)
		luaL_unref(L, LUA_REGISTRYINDEX, luaScr->HID[devIdx].softButtonCallbackRef);
	luaScr->HID[devIdx].softButtonCallbackRef = luaRef;

	HRESULT h;
	if (luaRef != LUA_REFNIL) {
		h = hasCallback || hasOtherCallback ? S_OK :
			DevMan->DO()->RegisterSoftButtonCallback(HID[devIdx].hDevice, (Pfn_DirectOutput_SoftButtonChange)DO_SoftButtonChange, NULL);
	} else {
		h = hasOtherCallback ? S_OK :
			DevMan->DO()->RegisterSoftButtonCallback(HID[devIdx].hDevice, NULL, NULL);
	}
	lua_pushnumber(L, GetAPIError(h));
	return 1;
}

LUA_FUNC(RegisterSoftButtonUpCallback) {
	if (!DevMan->IsInitialized())
		return 0;
	int p = lua_gettop(L);

	if (/* Base signature */
			!(p == 3 &&
		lua_isstring(L, 1) &&						// Model Name
		lua_isnumber(L, 2) &&						// Model Index
		(lua_isstring(L, 3)
			|| lua_isnil(L, 3)
			|| lua_isfunction(L, 3)))	// Callback
	) { return 0; }

	const char *devMod = lua_tostring(L, 1);
	int devIdx = (int)lua_tonumber(L, 2);
	int luaRef = LUA_REFNIL;

	devIdx = HIDLookupByType(devMod, devIdx);
	if (!(devIdx >= 0 && devIdx < HIDCount && HID[devIdx].isActive))
		return 0;

	ScriptInfo *luaScr = LuaMan->GetSlot(L);
	if (luaScr == NULL)
		return 0;
	bool hasCallback = (luaScr->HID[devIdx].softButUpCallbackRef != LUA_REFNIL);
	bool hasOtherCallback = (luaScr->HID[devIdx].softButtonCallbackRef != LUA_REFNIL ||
							 luaScr->HID[devIdx].softButDownCallbackRef != LUA_REFNIL);

	if (lua_isstring(L, 3)) {
		const char *nameP = lua_tostring(L, 3);
		lua_getfield(L, LUA_GLOBALSINDEX, nameP);
		luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
	} else if (lua_isfunction(L, 3)) {
		luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
	}	
	if (hasCallback)
		luaL_unref(L, LUA_REGISTRYINDEX, luaScr->HID[devIdx].softButUpCallbackRef);
	luaScr->HID[devIdx].softButUpCallbackRef = luaRef;

	HRESULT h;
	if (luaRef != LUA_REFNIL) {
		h = hasCallback || hasOtherCallback ? S_OK :
			DevMan->DO()->RegisterSoftButtonCallback(HID[devIdx].hDevice, (Pfn_DirectOutput_SoftButtonChange)DO_SoftButtonChange, NULL);
	} else {
		h = hasOtherCallback ? S_OK :
			DevMan->DO()->RegisterSoftButtonCallback(HID[devIdx].hDevice, NULL, NULL);
	}
	lua_pushnumber(L, GetAPIError(h));
	return 1;
}

LUA_FUNC(RegisterSoftButtonDownCallback) {
	if (!DevMan->IsInitialized())
		return 0;
	int p = lua_gettop(L);

	if (/* Base signature */
			!(p == 3 &&
		lua_isstring(L, 1) &&						// Model Name
		lua_isnumber(L, 2) &&						// Model Index
		(lua_isstring(L, 3) || lua_isnil(L, 3)))	// Callback
	) { return 0; }

	const char *devMod = lua_tostring(L, 1);
	int devIdx = (int)lua_tonumber(L, 2);
	int luaRef = LUA_REFNIL;

	devIdx = HIDLookupByType(devMod, devIdx);
	if (!(devIdx >= 0 && devIdx < HIDCount && HID[devIdx].isActive))
		return 0;

	ScriptInfo *luaScr = LuaMan->GetSlot(L);
	if (luaScr == NULL)
		return 0;
	bool hasCallback = (luaScr->HID[devIdx].softButDownCallbackRef != LUA_REFNIL);
	bool hasOtherCallback = (luaScr->HID[devIdx].softButtonCallbackRef != LUA_REFNIL ||
							 luaScr->HID[devIdx].softButUpCallbackRef != LUA_REFNIL);

	if (lua_isstring(L, 3)) {
		const char *nameP = lua_tostring(L, 3);
		lua_getfield(L, LUA_GLOBALSINDEX, nameP);
		luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
	} else if (lua_isfunction(L, 3)) {
		luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
	}	
	if (hasCallback)
		luaL_unref(L, LUA_REGISTRYINDEX, luaScr->HID[devIdx].softButDownCallbackRef);
	luaScr->HID[devIdx].softButDownCallbackRef = luaRef;

	HRESULT h;
	if (luaRef != LUA_REFNIL) {
		h = hasCallback || hasOtherCallback ? S_OK :
			DevMan->DO()->RegisterSoftButtonCallback(HID[devIdx].hDevice, (Pfn_DirectOutput_SoftButtonChange)DO_SoftButtonChange, NULL);
	} else {
		h = hasOtherCallback ? S_OK :
			DevMan->DO()->RegisterSoftButtonCallback(HID[devIdx].hDevice, NULL, NULL);
	}
	lua_pushnumber(L, GetAPIError(h));
	return 1;
}

LUA_FUNC(SetLed) {
	if (!DevMan->IsInitialized())
		return 0;
	int p = lua_gettop(L);

	if (/* Base signature */
		!(p == 5 &&
			lua_isstring(L, 1) &&		// Model Name
			lua_isnumber(L, 2) &&		// Model Index
			lua_isnumber(L, 3) &&		// Page ID
			lua_isnumber(L, 4) &&		// Led Index
			lua_isboolean(L, 5))		// ON/OFF
	) { return 0; }

	const char *devMod = lua_tostring(L, 1);
	int devIdx = (int)lua_tonumber(L, 2);
	int pageID = (int)lua_tonumber(L, 3);
	int ledIdx = (int)lua_tonumber(L, 4);
	bool isOn = (lua_toboolean(L, 5) == 1);

	devIdx = HIDLookupByType(devMod, devIdx);
	if (!(devIdx >= 0 && devIdx < HIDCount && HID[devIdx].isActive))
		return 0;

	HRESULT h = DevMan->DO()->SetLed(HID[devIdx].hDevice, pageID, ledIdx, isOn);
	lua_pushnumber(L, GetAPIError(h));
	return 1;
}

LUA_FUNC(SetImageFromFile) {
	if (!DevMan->IsInitialized())
		return 0;
	int p = lua_gettop(L);

	if (/* Base signature */
		!(p == 5 &&
			lua_isstring(L, 1) &&		// Model Name
			lua_isnumber(L, 2) &&		// Model Index
			lua_isnumber(L, 3) &&		// Page ID
			lua_isnumber(L, 4) &&		// Image Index
			lua_isstring(L, 5)) &&		// Path
		/* With stretch */
		!(p == 6 &&
			lua_isstring(L, 1) &&		// Model Name
			lua_isnumber(L, 2) &&		// Model Index
			lua_isnumber(L, 3) &&		// Page ID
			lua_isnumber(L, 4) &&		// Image Index
			lua_isstring(L, 5) &&		// Path
			lua_isboolean(L, 6))		// Strech
	) { return 0; }

	const char *devMod = lua_tostring(L, 1);
	int devIdx = (int)lua_tonumber(L, 2);
	int pageID = (int)lua_tonumber(L, 3);
	int imgIdx = (int)lua_tonumber(L, 4) - 1;	// Index 1..n in Lua
	const char *nameP = lua_tostring(L, 5);
	bool shouldStrech = (p == 5) ? false : (lua_toboolean(L, 6) == 1);

	devIdx = HIDLookupByType(devMod, devIdx);
	if (!(devIdx >= 0 && devIdx < HIDCount && HID[devIdx].isActive))
		return 0;

	wchar_t *fileNameWideP;
	int len = charToWideConverter(nameP, &fileNameWideP);// TODO: pas de malloc, [MAXPATHLEN]

	HRESULT h;
	if (!shouldStrech) {
		h = DevMan->DO()->SetImageFromFile(HID[devIdx].hDevice, pageID, imgIdx, len, fileNameWideP);
	} else {
		CRawImage img;
		HDC hdc = img.BeginPaint();
		s_RenderImage(hdc, fileNameWideP);
		img.EndPaint();
		h = DevMan->DO()->SetImage(HID[devIdx].hDevice, pageID, imgIdx, 320*240*3, img.Buffer());
	}
	free(fileNameWideP);
	lua_pushnumber(L, GetAPIError(h));
	return 1;
}

LUA_FUNC(SetImage) {
	if (!DevMan->IsInitialized())
		return 0;
	int p = lua_gettop(L);

	if (/* Base signature */
		!(p == 5 &&
			lua_isstring(L, 1) &&		// Model Name
			lua_isnumber(L, 2) &&		// Model Index
			lua_isnumber(L, 3) &&		// Page ID
			lua_isnumber(L, 4) &&		// Image Index
			lua_isstring(L, 5))			// Image Data
	) { return 0; }

	const char *devMod = lua_tostring(L, 1);
	int devIdx = (int)lua_tonumber(L, 2);
	int pageID = (int)lua_tonumber(L, 3);
	int imgIdx = (int)lua_tonumber(L, 4) - 1;	// Index 1..n in Lua
	const char *dataP = lua_tostring(L, 5);

	devIdx = HIDLookupByType(devMod, devIdx);
	if (!(devIdx >= 0 && devIdx < HIDCount && HID[devIdx].isActive))
		return 0;
	if (!dataP)
		return 0;

	// GD only
	char img[320*240*3];
	dataP += GD_HEADER_SIZE;

	for (int y = 0; y < 240; y++) {
		for (int x = 0; x < 320; x++) {
			int a = (x + y * 320) * 3;
			int b = (x + ((240 - 1) - y) * 320) * 4;

			*(img + a + 0) = *(dataP + b + 2);
			*(img + a + 1) = *(dataP + b + 1);
			*(img + a + 2) = *(dataP + b + 0);
		}
	}
	HRESULT h = DevMan->DO()->SetImage(HID[devIdx].hDevice, pageID, imgIdx, sizeof(img), img);
	lua_pushnumber(L, GetAPIError(h));
	return 1;
}

LUA_FUNC(SetString) {
	if (!DevMan->IsInitialized())
		return 0;
	int p = lua_gettop(L);

	if (/* Base signature */
		!(p == 5 &&
			lua_isstring(L, 1) &&		// Model Name
			lua_isnumber(L, 2) &&		// Model Index
			lua_isnumber(L, 3) &&		// Page ID
			lua_isnumber(L, 4) &&		// String Index
			lua_isstring(L, 5))			// String Content
	) { return 0; }

	const char *devMod = lua_tostring(L, 1);
	int devIdx = (int)lua_tonumber(L, 2);
	int pageID = (int)lua_tonumber(L, 3);
	int strIdx = (int)lua_tonumber(L, 4) - 1;	// Index 1..n in Lua
	const char *dataP = lua_tostring(L, 5);

	devIdx = HIDLookupByType(devMod, devIdx);
	if (!(devIdx >= 0 && devIdx < HIDCount && HID[devIdx].isActive))
		return 0;
	if (!dataP)
		return 0;

	wchar_t *textWideP;
	int len = charToWideConverter(dataP, &textWideP);// TODO: pas de malloc, [MAXFILENAME]

	HRESULT h = DevMan->DO()->SetString(HID[devIdx].hDevice, pageID, strIdx, len, textWideP);
	free(textWideP);
	lua_pushnumber(L, GetAPIError(h));
	return 1;
}

LUA_FUNC(SetProfile) {
	if (!DevMan->IsInitialized())
		return 0;
	int p = lua_gettop(L);

	if (/* Base signature */
		!(p == 3 &&
			lua_isstring(L, 1) &&		// Model Name
			lua_isnumber(L, 2) &&		// Model Index
			lua_isstring(L, 3))			// Path
	) { return 0; }

	const char *devMod = lua_tostring(L, 1);
	int devIdx = (int)lua_tonumber(L, 2);
	const char *nameP = lua_tostring(L, 3);

	devIdx = HIDLookupByType(devMod, devIdx);
	if (!(devIdx >= 0 && devIdx < HIDCount && HID[devIdx].isActive))
		return 0;

	HRESULT h;
	if (!nameP) {
		h = DevMan->DO()->SetProfile(HID[devIdx].hDevice, 0, NULL);
	} else {
		wchar_t *fileNameWideP;
		int len = charToWideConverter(nameP, &fileNameWideP);// TODO: pas de malloc, [MAXPATHLEN]
		h = DevMan->DO()->SetProfile(HID[devIdx].hDevice, len, fileNameWideP);
		free(fileNameWideP);
	}

	lua_pushnumber(L, GetAPIError(h));
	return 1;
}
