#include "Common.h"
#include "lua.hpp"
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

// TODO: move this in Utils

#include <atlimage.h>
#include <tchar.h>

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

int luaF_Finalizer(lua_State* L) {
	LuaMan->ReleaseSlot(L);
	DevMan->Release();
	return 0;
}

static void luaF_RegisterConst(lua_State* L) {
	const struct {
		const char *name;
		int value;

	} list[] = {
		// Errors
		{ "ERR_NONE", 0 },
		{ "ERR_HANDLE", 1 },
		{ "ERR_NOTIMPL", 2 },
		{ "ERR_INVALIGARG", 3 },
		{ "ERR_PAGENOTACTIVE", 4 },
		{ "ERR_OUTOFMEMORY", 5 },
		{ "ERR_UNKNOWN", -1 },

		// X52 Pro
		{ "BIT_SCROLLWHEEL_CLICK", SoftButton_Select },
		{ "BIT_SCROLLWHEEL_UP", SoftButton_Up },
		{ "BIT_SCROLLWHEEL_DOWN", SoftButton_Down },

		// FIP
		{ "BIT_RIGHTROTARY_CW", SoftButton_Up },
		{ "BIT_RIGHTROTARY_CCW", SoftButton_Down },
		{ "BIT_LEFTROTARY_CW", SoftButton_Right },
		{ "BIT_LEFTROTARY_CCW", SoftButton_Left },
		{ "BIT_S1BUTTON", SoftButton_1 },
		{ "BIT_S2BUTTON", SoftButton_2 },
		{ "BIT_S3BUTTON", SoftButton_3 },
		{ "BIT_S4BUTTON", SoftButton_4 },
		{ "BIT_S5BUTTON", SoftButton_5 },
		{ "BIT_S6BUTTON", SoftButton_6 },

		{ NULL, 0 }
	};

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
	luaF_RegisterConst(L);

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
