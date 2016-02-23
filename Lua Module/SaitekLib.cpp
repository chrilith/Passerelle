#include "Common.h"

#include <atlimage.h>
#include <tchar.h>

extern "C" {
#include <hidsdi.h>
#include <SetupAPI.h>
#include <devpkey.h>
#include <dinput.h>
}

#include "lua.hpp"
#include "SaitekLib.h"
#include "Import/Source/RawImage.h"
#include "Import/Source/DirectOutputImpl.h"

#include "Device.h"
#include "Script.h"
#include "Api.h"

// callback ref: http://stackoverflow.com/questions/2688040/how-to-callback-a-lua-function-from-a-c-function

#ifdef DEBUG
void print(lua_State* L, const char *s) {
		
	lua_getfield(L, LUA_GLOBALSINDEX, "print");
	lua_pushstring(L, s);
	lua_call(L, 1, 0);
}
#endif

void CALLBACK DO_Enumerate(void* hDevice, void* pCtxt) {
	DevMan->HandleDeviceChange(hDevice, true);
}

void CALLBACK DO_DeviceChange(void* hDevice, bool bAdded, void* pCtxt) {
	int index = DevMan->HandleDeviceChange(hDevice, bAdded);
	LuaMan->CallDeviceChangeCallbacks(index, bAdded);
}

void CALLBACK DO_PageChange(void* hDevice, DWORD dwPage, bool bSetActive, void* pCtxt) {
	int index = DevMan->LookupByHandle(hDevice);
	if (index == HID_NOTFOUND || !HID[index].isActive)
		return;
	LuaMan->CallPageChangeCallbacks(index, dwPage, bSetActive);
}

void CALLBACK DO_SoftButtonChange(void* hDevice, DWORD dwButtons, void* pCtxt) {
	int index = DevMan->LookupByHandle(hDevice);
	if (index == HID_NOTFOUND || !HID[index].isActive)
		return;
	LuaMan->CallSoftButtonCallbacks(index, dwButtons);
}

int charToWideConverter(const char *s, wchar_t **d) {
	size_t baseSize = strlen(s) + 1;
	size_t convertedChars = 0;
	*d = (wchar_t *)malloc(baseSize * 2);
	mbstowcs_s(&convertedChars, *d, baseSize, s, _TRUNCATE);

	return convertedChars;
}

void s_RenderImage(HDC hdc, LPCTSTR tsz) {
	CImage image;
	HRESULT hr = image.Load(tsz);
	if (SUCCEEDED(hr)) {
		int old = SetStretchBltMode(hdc, COLORONCOLOR);
		image.StretchBlt(hdc, 0, 0, 320, 240, SRCCOPY);
		SetStretchBltMode(hdc, old);
	} 
}

#include <direct.h>

int luaF_Finalizer(lua_State* L) {
	LuaMan->ReleaseSlot(L);
	DevMan->Release();
	return 0;
}

static const Const Errors[] = {
	{ "ERR_NONE", 0 },
	{ "ERR_HANDLE", 1 },
	{ "ERR_NOTIMPL", 2 },
	{ "ERR_INVALIGARG", 3 },
	{ "ERR_PAGENOTACTIVE", 4 },
	{ "ERR_OUTOFMEMORY", 5 },
	{ "ERR_UNKNOWN", -1 },
	{ NULL, 0 }
};

static const Const Buttons[] =  {
	{ "kRightRotaryCW", 0x00000002 },
	{ "kRightRotaryCCW", 0x00000004 },
	{ "kLeftRotaryCW", 0x00000008 },
	{ "kLeftRotaryCCW", 0x00000010 },
	{ "kS1Button", 0 },
	{ "kS2Button", 1 },
	{ "kS3Button", 2 },
	{ "kS4Button", 3 },
	{ "kS5Button", 4 },
	{ "kS6Button", 5 },
	{ NULL, 0 }
};

static void luaF_RegisterConst(lua_State* L, const Const *list) {
	for (int i = 0; list[i].name != NULL; i++) {
		lua_pushinteger(L, list[i].value);
		lua_setfield(L, 1, list[i].name);
	}
	lua_pushvalue(L, 1);
	lua_replace(L, LUA_ENVIRONINDEX);
}

extern "C" LUALIB_OPEN() {
	// Initialize
	DevMan->Initialize();

	// Activate
	LUA_START(API)
		LUA_ENTRY(GetVersion)
		LUA_ENTRY(Listen)
		LUA_ENTRY(GetNumDevices)
		LUA_ENTRY(AddPage)
		LUA_ENTRY(RemovePage)
		LUA_ENTRY(SetLed)
		LUA_ENTRY(SetImageFromFile)
		LUA_ENTRY(SetImage)
		LUA_ENTRY(SetString)
		LUA_ENTRY(SetProfile)
		LUA_ENTRY(RegisterDeviceChangeCallback)
		LUA_ENTRY(RegisterPageCallback)
		LUA_ENTRY(RegisterSoftButtonCallback)
		LUA_ENTRY(RegisterSoftButtonUpCallback)
		LUA_ENTRY(RegisterSoftButtonDownCallback)
		// Obsolete
		LUA_ENTRY(Initialize)
		LUA_ENTRY(Release)
	LUA_END()

	// Register methods
	luaL_register(L, LUALIB_NAME, API);

	// Set constants
	luaF_RegisterConst(L, Errors);
	luaF_RegisterConst(L, Buttons);

	// Create a meta table for the script data to be collected upon script interruption
	luaL_newmetatable(L, LUALIB_TABLE);
	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, luaF_Finalizer);
	lua_settable(L, -3);

	// Create the associated userdata
	ScriptInfo **dataP = (ScriptInfo **)lua_newuserdata(L, sizeof(ScriptInfo **));
	luaL_getmetatable(L, LUALIB_TABLE);
	lua_setmetatable(L, -2);

	// Save it so that it is collected only at the end of the execution
	int luaRef = luaL_ref(L, LUA_REGISTRYINDEX);

	// Saving...
	*dataP = LuaMan->GetFreeSlot(L, luaRef);

	return 0;
}
