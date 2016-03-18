#include "Common.h"
#include "Script.h"
#include "resource.h"
#include "Api.h"
#include "Utils.h"
#include "Bitmap.h"
#include "Image.h"

#include "lua.hpp"

#include "CallbackHandler.h"

extern int HIDCount;
extern SaitekDevice HID[HID_COUNT];

static int GetAPIError(HRESULT hResult) {
	switch (hResult) {
	case S_OK:
		return ERR_NONE;
	case E_HANDLE:
		return ERR_HANDLE;
	case E_NOTIMPL:
		return ERR_NOTIMPL;
	case E_INVALIDARG:
		return ERR_INVALIDARG;
	case E_PAGENOTACTIVE:
		return ERR_PAGENOTACTIVE;
	case E_OUTOFMEMORY:
		return ERR_OUTOFMEMORY;
	default:	// Unknown
		return ERR_UNKNOWN;
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

LUA_FUNC(getVersion) {
	lua_pushstring(L, LIB_VERSION);
	lua_pushnumber(L, LIB_VERSION_MAJOR);
	lua_pushnumber(L, LIB_VERSION_MINOR);
	lua_pushnumber(L, LIB_VERSION_BUILD);
	lua_pushnumber(L, LIB_VERSION_PATCH);
	return 5;
}

LUA_OBSOLETE(Initialize)
LUA_OBSOLETE(Release)

LUA_FUNC(listen) {
	fgetc(stdin);
	return 0;
}

LUA_FUNC(sleep) {
	int p = lua_gettop(L);

	if (!(p == 1 &&
			lua_isnumber(L, 1))
		) {
		return 0;
	}
	int delay = (int)lua_tonumber(L, 1);

	Sleep(delay);
	return 0;
}

LUA_FUNC(poll) {
	ScriptInfo *luaScr = LuaMan->GetSlot(L);
	CallbackHandler *handler = luaScr->events;
	int processed = 0;

	if (handler)
	while (Event *e = handler->PopEvent()) {
		handler->Process(e);
		processed++;
	}

	lua_pushnumber(L, processed);
	return 1;
}

LUA_FUNC(setMode) {
	ScriptInfo *luaScr = LuaMan->GetSlot(L);
	int p = lua_gettop(L);

	if (!(p == 1 &&
		lua_isnumber(L, 1))
		) {
		return 0;
	}
	int mode = (int)lua_tonumber(L, 1);

	if (luaScr->events->GetMode() != mode) {
		delete luaScr->events;
		luaScr->events = CallbackHandler::Factory(mode);
	}

	return 0;
}

LUA_FUNC(getNumDevices) {
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

LUA_FUNC(addPage) {
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

LUA_FUNC(removePage) {
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

LUA_FUNC(registerDeviceChangeCallback) {
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

LUA_FUNC(registerPageCallback) {
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

LUA_FUNC(registerSoftButtonCallback) {
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

LUA_FUNC(registerSoftButtonUpCallback) {
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

LUA_FUNC(registerSoftButtonDownCallback) {
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

LUA_FUNC(setLed) {
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

LUA_FUNC(setImageFromFile) {
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
	int len = Utils::CharToWideConverter(nameP, &fileNameWideP);

	HRESULT h;
	if (!shouldStrech) {
		h = DevMan->DO()->SetImageFromFile(HID[devIdx].hDevice, pageID, imgIdx, len, fileNameWideP);
	} else {
		Image img;
		HDC hdc = img.BeginPaint();
		Utils::RenderStretchedImage(hdc, fileNameWideP);
		img.EndPaint();
		h = DevMan->DO()->SetImage(HID[devIdx].hDevice, pageID, imgIdx, SCREEN_BUFSIZE, img.Bits());
	}
	free(fileNameWideP);
	lua_pushnumber(L, GetAPIError(h));
	return 1;
}

LUA_FUNC(setImage) {
	if (!DevMan->IsInitialized())
		return 0;
	int p = lua_gettop(L);

	if (/* Base signature */
		!(p == 5 &&
			lua_isstring(L, 1) &&		// Model Name
			lua_isnumber(L, 2) &&		// Model Index
			lua_isnumber(L, 3) &&		// Page ID
			lua_isnumber(L, 4) &&		// Image Index
			(	lua_isstring(L, 5) ||
				lua_isuserdata(L, 5))	// Image Data
		)
	) { return 0; }

	const char *devMod = lua_tostring(L, 1);
	int devIdx = (int)lua_tonumber(L, 2);
	int pageID = (int)lua_tonumber(L, 3);
	int imgIdx = (int)lua_tonumber(L, 4) - 1;	// Index 1..n in Lua

	devIdx = HIDLookupByType(devMod, devIdx);
	if (!(devIdx >= 0 && devIdx < HIDCount && HID[devIdx].isActive))
		return 0;

	HRESULT h;
	Image *render;
	bool clean = true;

	if (lua_isstring(L, 5)) {
		const char *dataP = lua_tostring(L, 5);
		if (!dataP) {
			return 0;
		}
		// Convert from 32-bit to 24-bit
		Image srgb(dataP);
		render = new Image();
		render->Blit(&srgb);

	} else {
		Image *image = luaF_BitmapCheck(L, 5);
		if (!image) {
			return 0;
		}
		if (image->Width() == SCREEN_WIDTH && image->Height() == SCREEN_HEIGHT) {
			render = image;
			clean = false;
		} else {
			render = new Image();
			render->Blit(image);
		}
	}

	h = DevMan->DO()->SetImage(HID[devIdx].hDevice, pageID, imgIdx, SCREEN_BUFSIZE, render->Bits());
	if (clean) {
		delete render;
	}
	lua_pushnumber(L, GetAPIError(h));
	return 1;
}

LUA_FUNC(captureWindow) {
	int p = lua_gettop(L);
//	DebugL(L, "captureWindow %d", p);

	if (/* Base signature */
		!(p == 6 &&
			(	lua_isstring(L, 1) ||
				lua_isnil(L, 1)) &&		// class name
			(	lua_isstring(L, 2) ||
				lua_isnil(L, 2)) &&		// window name
			lua_isnumber(L, 3) &&		// x offset
			lua_isnumber(L, 4) &&		// y offset
			lua_isnumber(L, 5) &&		// width
			lua_isnumber(L, 6))			// height
		) {
		return 0;
	}

	wchar_t *lpClassName, *lpWindowName;
	const char *className = lua_tostring(L, 1);
	const char *winName = lua_tostring(L, 2);

	Utils::CharToWideConverter(className, &lpClassName);
	Utils::CharToWideConverter(winName, &lpWindowName);

	HWND winH = FindWindow(lpClassName, lpWindowName);
	if (!winH) {
		return 0;
	}

	// Copy size in %
	double x = lua_tonumber(L, 3);
	double y = lua_tonumber(L, 4);
	double width = lua_tonumber(L, 5);
	double height = lua_tonumber(L, 6);

	RECT rect;
	GetClientRect(winH, &rect);

	POINT pt, pt2;
	pt.x = rect.left;
	pt.y = rect.top;
	pt2.x = rect.right + 1;
	pt2.y = rect.bottom + 1;

	int ww = (pt2.x - pt.x + 1);
	int wh = (pt2.y - pt.y + 1);

	int cx = rect.left +(LONG)(x * ww / 100.0);
	int cy = rect.top +(LONG)(y * wh / 100.0);
	int cw = (LONG)(width * ww / 100.0);
	int ch = (LONG)(height * wh / 100.0);

	return luaF_BitmapCaptureFromWindow(L, winH, cx, cy, cw, ch);
}

LUA_FUNC(setString) {
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
	int len = Utils::CharToWideConverter(dataP, &textWideP);

	HRESULT h = DevMan->DO()->SetString(HID[devIdx].hDevice, pageID, strIdx, len, textWideP);
	free(textWideP);
	lua_pushnumber(L, GetAPIError(h));
	return 1;
}

LUA_FUNC(setProfile) {
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
		int len = Utils::CharToWideConverter(nameP, &fileNameWideP);
		h = DevMan->DO()->SetProfile(HID[devIdx].hDevice, len, fileNameWideP);
		free(fileNameWideP);
	}

	lua_pushnumber(L, GetAPIError(h));
	return 1;
}
