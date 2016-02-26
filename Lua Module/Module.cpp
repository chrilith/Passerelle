#include "Common.h"
#include "lua.hpp"
#include "Script.h"
#include "Api.h"

static void luaF_RegisterConst(lua_State* L) {
	static const struct {
		const char *name;
		int value;

	} list[] = {
		// Errors
		LUA_CONST(ERR_NONE),
		LUA_CONST(ERR_HANDLE),
		LUA_CONST(ERR_NOTIMPL),
		LUA_CONST(ERR_INVALIDARG),
		LUA_CONST(ERR_PAGENOTACTIVE),
		LUA_CONST(ERR_OUTOFMEMORY),
		LUA_CONST(ERR_UNKNOWN),

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

int luaF_Finalizer(lua_State* L) {
	LuaMan->ReleaseSlot(L);
	DevMan->Release();
	return 0;
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

	// Find a slot...
	ScriptInfo *slotP = LuaMan->GetFreeSlot(L);
	if (slotP) {
		// Create the associated userdata
		ScriptInfo **dataP = (ScriptInfo **)lua_newuserdata(L, sizeof(ScriptInfo **));
		luaL_getmetatable(L, LUALIB_TABLE);
		lua_setmetatable(L, -2);

		// Save it so that it is collected only at the end of the execution
		slotP->luaRef = luaL_ref(L, LUA_REGISTRYINDEX);

		// Save the script data
		*dataP = slotP;
	}

	return 1;
}
