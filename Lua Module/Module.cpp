#include "Config.h"
#include "Common.h"
#include "Bitmap.h"
#include "Window.h"
#include "lua.hpp"
#include "Script.h"
#include "Api.h"
#include "CallbackHandler.h"
#include "resource.h"

#define _NAME		LUALIB_NAME
#define _VERSION	_NAME " " LIB_VERSION
#define _COPYRIGHT	_VERSION " (c) 2011-2016 Chris Apers"

static void luaF_RegisterConst(lua_State* L) {
	LUA_START(list, luaI_const)
		// Errors
		LUA_CONST(ERR_NONE)
		LUA_CONST(ERR_HANDLE)
		LUA_CONST(ERR_NOTIMPL)
		LUA_CONST(ERR_INVALIDARG)
		LUA_CONST(ERR_PAGENOTACTIVE)
		LUA_CONST(ERR_OUTOFMEMORY)
		LUA_CONST(ERR_UNKNOWN)
		// X52 Pro
		LUA_CONST(BIT_SCROLLWHEEL_CLICK)
		LUA_CONST(BIT_SCROLLWHEEL_UP)
		LUA_CONST(BIT_SCROLLWHEEL_DOWN)
		// FIP
		LUA_CONST(BIT_RIGHTROTARY_CW)
		LUA_CONST(BIT_RIGHTROTARY_CCW)
		LUA_CONST(BIT_LEFTROTARY_CW)
		LUA_CONST(BIT_LEFTROTARY_CCW)
		LUA_CONST(BIT_S1BUTTON)
		LUA_CONST(BIT_S2BUTTON)
		LUA_CONST(BIT_S3BUTTON)
		LUA_CONST(BIT_S4BUTTON)
		LUA_CONST(BIT_S5BUTTON)
		LUA_CONST(BIT_S6BUTTON)
		// Callback mode
		LUA_CONST(CBM_DIRECT)
		LUA_CONST(CBM_EVENT)
#ifdef WITH_FSUIPC
		LUA_CONST(CBM_FSUIPC)
#endif
	LUA_END()

	for (int i = 0; list[i].name != NULL; i++) {
		lua_pushinteger(L, list[i].value);
		lua_setfield(L, -2, list[i].name);
	}
}

static void luaF_RegisterLiteral(lua_State* L) {
	LUA_START(list, luaI_literal)
		LUA_CONST(_NAME)
		LUA_CONST(_VERSION)
		LUA_CONST(_COPYRIGHT)
	LUA_END()

	for (int i = 0; list[i].name != NULL; i++) {
		lua_pushlstring(L, list[i].value, strlen(list[i].value));
		lua_setfield(L, -2, list[i].name);
	}
}

int luaF_Finalizer(lua_State* L) {
	LuaMan->ReleaseSlot(L);
	DevMan->Release();
	return 0;
}

extern "C" LUALIB_OPEN() {
	// Activate
	LUA_START(API, luaL_reg)
		LUA_ENTRY(getVersion)
		LUA_ENTRY(listen)
		LUA_ENTRY(getNumDevices)
		LUA_ENTRY(addPage)
		LUA_ENTRY(removePage)
		LUA_ENTRY(setLed)
		LUA_ENTRY(setImageFromFile)
		LUA_ENTRY(setImage)
		LUA_ENTRY(setString)
		LUA_ENTRY(setProfile)
		LUA_ENTRY(registerDeviceChangeCallback)
		LUA_ENTRY(registerPageCallback)
		LUA_ENTRY(registerSoftButtonCallback)
		LUA_ENTRY(registerSoftButtonUpCallback)
		LUA_ENTRY(registerSoftButtonDownCallback)
		// New in v0.7
		LUA_ENTRY(setMode)
		LUA_ENTRY(poll)
		LUA_ENTRY(sleep)
		// New in v0.8
		LUA_ENTRY(findWindow)
		// Obsolete
		LUA_ENTRY(Initialize)
		LUA_ENTRY(Release)
	LUA_END()

	// We already have a slot for this context
	if (LuaMan->HaveSlot(L)) {
		return 0;
	}

	// Find a free slot...
	ScriptInfo *slotP = LuaMan->GetFreeSlot(L);
	if (!slotP) {
		TraceL(L, "Failed to initialize '" LUALIB_NAME "'");
		// Initialization failed
		return -1;
	}

	// Initialize
	DevMan->Initialize();

	// Register methods
	luaL_register(L, LUALIB_NAME, API);

	// Set constants
	luaF_RegisterConst(L);
	luaF_RegisterLiteral(L);

	// Create a meta table for the script data to be collected upon script interruption
	luaL_newmetatable(L, LUALIB_TABLE);
	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, luaF_Finalizer);
	lua_settable(L, -3);

	// Initialize the internal bitmap context
	luaF_RegisterBitmap(L);
	luaF_RegisterWindow(L);

	// Create the associated userdata
	ScriptInfo **dataP = (ScriptInfo **)lua_newuserdata(L, sizeof(ScriptInfo **));
	luaL_getmetatable(L, LUALIB_TABLE);
	lua_setmetatable(L, -2);

	// Save it so that it is collected only at the end of the execution
	slotP->luaRef = luaL_ref(L, LUA_REGISTRYINDEX);

	// Save the script data
	*dataP = slotP;

	return 1;
}
