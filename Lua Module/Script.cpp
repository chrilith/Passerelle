#include "Common.h"
#include "Script.h"
#include "lua.hpp"

ScriptManager *LuaMan = ScriptManager::GetInstance();

ScriptInfo Script[SCRIPT_COUNT];
int ScriptCount = HID_EMPTY;

ScriptManager::ScriptManager() {
	_scriptCount = HID_EMPTY;
	for (int i = 0; i < SCRIPT_COUNT; i++) {
		_luaScript[i].luaState = NULL;
	}
}

int ScriptManager::FindFreeSlot() {
	for (int i = 0; i < SCRIPT_COUNT; i++)
		if (_luaScript[i].luaState == NULL)
			return i;
	return HID_NOTFOUND;
}

int ScriptManager::ReleaseSlot(lua_State *L) {
	_lock.Acquire();

	ScriptInfo *nfo = (*(ScriptInfo **)lua_touserdata(L, 1));
	if (nfo) {
		lua_unref(L, nfo->luaRef);
		nfo->luaState = NULL;

		_scriptCount--;
		DebugC(L"Dec: %d", _scriptCount);
	}

	_lock.Release();
	return 0;
}

ScriptInfo *ScriptManager::GetSlot(lua_State *L) {
	_lock.Acquire();

	for (int i = 0; i < SCRIPT_COUNT; i++) {
		if (_luaScript[i].luaState == L) {				
			_lock.Release();
			return &_luaScript[i];
		}
	}

	_lock.Release();
	return NULL;
}

void ScriptManager::CallPageChangeCallbacks(int index, DWORD dwPage, bool bSetActive) {
	int devIdx = HIDLookupByIndex(index);
	const char *devMod = GetDeviceStringName(HID[index].type);

	for (int i = 0; i < SCRIPT_COUNT; i++) {
		lua_State *L = _luaScript[i].luaState;
		if (!L) continue;
		CallbackList *hid = &_luaScript[i].HID[index];

		if (hid->pageButtonCallbackRef != LUA_REFNIL) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, hid->pageButtonCallbackRef);
			lua_pushstring(L, devMod);
			lua_pushnumber(L, devIdx);
			lua_pushnumber(L, dwPage);
			lua_pushboolean(L, bSetActive);

			lua_pcall(L, 4, 0, 0); // TODO: error function
		}
	}
}

void ScriptManager::CallSoftButtonCallbacks(int index, DWORD dwButtons) {
	DWORD changedDown = ~HID[index].oldButtonState & dwButtons;
	DWORD changedUp = HID[index].oldButtonState & ~dwButtons;
	HID[index].oldButtonState = dwButtons;

	int devIdx = HIDLookupByIndex(index);
	const char *devMod = GetDeviceStringName(HID[index].type);

	for (int i = 0; i < SCRIPT_COUNT; i++) {
		lua_State *L = _luaScript[i].luaState;
		if (!L) continue;
		CallbackList *hid = &_luaScript[i].HID[index];

		if (hid->softButtonCallbackRef != LUA_REFNIL) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, hid->softButtonCallbackRef);
			lua_pushstring(L, devMod);
			lua_pushnumber(L, devIdx);
			lua_pushnumber(L, dwButtons);
			lua_pcall(L, 3, 0, 0); // TODO: error function
		}

		if (hid->softButDownCallbackRef != LUA_REFNIL && changedDown != 0) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, hid->softButDownCallbackRef);
			lua_pushstring(L, devMod);
			lua_pushnumber(L, devIdx);
			lua_pushnumber(L, changedDown);
			lua_pcall(L, 3, 0, 0); // TODO: error function
		}

		if (hid->softButUpCallbackRef != LUA_REFNIL && changedUp != 0) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, hid->softButUpCallbackRef);
			lua_pushstring(L, devMod);
			lua_pushnumber(L, devIdx);
			lua_pushnumber(L, changedUp);
			lua_pcall(L, 3, 0, 0); // TODO: error function
		}
	}
}

void ScriptManager::CallDeviceChangeCallbacks(int index, bool bAdded) {
	int devIdx = HIDLookupByIndex(index);
	const char *devMod = GetDeviceStringName(HID[index].type);

	for (int i = 0; i < SCRIPT_COUNT; i++) {
		lua_State *L = _luaScript[i].luaState;
		if (!L) continue;
		ScriptInfo *lua = &_luaScript[i];

		if (lua->deviceChangeCallbackRef != LUA_REFNIL) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, lua->deviceChangeCallbackRef);
			lua_pushstring(L, devMod);
			lua_pushnumber(L, devIdx);
			lua_pushboolean(L, bAdded);

			lua_pcall(L, 3, 0, 0); // TODO: error function
		}
	}
}

ScriptInfo *ScriptManager::GetFreeSlot(lua_State *L) {
	_lock.Acquire();
	int slot = FindFreeSlot();
	if (slot == HID_NOTFOUND) {
		_lock.Release();
		return NULL;
	}

	for (int i = 0; i < HID_COUNT; i++) {
		_luaScript[slot].HID[i].pageButtonCallbackRef = LUA_REFNIL;
		_luaScript[slot].HID[i].softButtonCallbackRef = LUA_REFNIL;
		_luaScript[slot].HID[i].softButDownCallbackRef = LUA_REFNIL;
		_luaScript[slot].HID[i].softButUpCallbackRef = LUA_REFNIL;
	}
	_luaScript[slot].deviceChangeCallbackRef = LUA_REFNIL;
	_luaScript[slot].luaState = L;

	_scriptCount++;

	DebugC(L"Inc: %d", _scriptCount);
	_lock.Release();

	return &_luaScript[slot];
}
