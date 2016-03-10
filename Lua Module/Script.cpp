#include "Common.h"
#include "Script.h"
#include "CallbackLua.h"
#include "lua.hpp"

#include "CallbackEvent.h"

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
		ScriptInfo *lua = &_luaScript[i];
		CallbackList *hid = &_luaScript[i].HID[index];
		CallbackHandler *handler = lua->events;

		if (hid->pageButtonCallbackRef != LUA_REFNIL) {
			PageChangeEvent *e = new PageChangeEvent(EVT_PAGECHANGE, L);
			e->funcRef = hid->softButtonCallbackRef;
			e->devMod = devMod;
			e->devIdx = devIdx;
			e->dwPage = dwPage;
			e->bSetActive = bSetActive;

			handler->PushEvent(e);
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
		ScriptInfo *lua = &_luaScript[i];
		CallbackList *hid = &_luaScript[i].HID[index];
		CallbackHandler *handler = lua->events;

		if (hid->softButtonCallbackRef != LUA_REFNIL) {
			ButtonChangeEvent *e = new ButtonChangeEvent(EVT_BUTTONCHANGE, L);
			e->funcRef = hid->softButtonCallbackRef;
			e->devMod = devMod;
			e->devIdx = devIdx;
			e->dwButtons = dwButtons;

			handler->PushEvent(e);
		}

		if (hid->softButDownCallbackRef != LUA_REFNIL && changedDown != 0) {
			ButtonChangeEvent *e = new ButtonChangeEvent(EVT_BUTTONCHANGE, L);
			e->funcRef = hid->softButDownCallbackRef;
			e->devMod = devMod;
			e->devIdx = devIdx;
			e->dwButtons = changedDown;

			handler->PushEvent(e);
		}

		if (hid->softButUpCallbackRef != LUA_REFNIL && changedUp != 0) {
			ButtonChangeEvent *e = new ButtonChangeEvent(EVT_BUTTONCHANGE, L);
			e->funcRef = hid->softButUpCallbackRef;
			e->devMod = devMod;
			e->devIdx = devIdx;
			e->dwButtons = changedUp;

			handler->PushEvent(e);
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
		CallbackHandler *handler = lua->events;

		if (lua->deviceChangeCallbackRef != LUA_REFNIL) {
			DeviceChangeEvent *e = new DeviceChangeEvent(EVT_DEVICECHANGE, L);
			e->funcRef = lua->deviceChangeCallbackRef;
			e->devMod = devMod;
			e->devIdx = devIdx;
			e->bAdded = bAdded;

			handler->PushEvent(e);
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
	_luaScript[slot].events = new CallbackLua();

	_scriptCount++;

	DebugC(L"Inc: %d", _scriptCount);
	_lock.Release();

	return &_luaScript[slot];
}
