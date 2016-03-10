#include "CallbackHandler.h"
#include "CallbackEvent.h"
#include "CallbackLua.h"
#include "CallbackFSUIPC.h"

#include "Common.h"
#include "lua.hpp"

void CallbackHandler::Process(Event * event) {
	_lock.Acquire();

	switch (event->type) {
	case EVT_BUTTONCHANGE:
		ButtonChange((ButtonChangeEvent *)event);
		break;
	case EVT_DEVICECHANGE:
		DeviceChange((DeviceChangeEvent *)event);
		break;
	case EVT_PAGECHANGE:
		PageChange((PageChangeEvent *)event);
		break;
	}

	_lock.Release();
	delete event;
}

void CallbackHandler::ButtonChange(ButtonChangeEvent *event) {
	lua_State *L = event->luaState;

	lua_rawgeti(L, LUA_REGISTRYINDEX, event->funcRef);
	lua_pushstring(L, event->devMod);
	lua_pushnumber(L, event->devIdx);
	lua_pushnumber(L, event->dwButtons);
	lua_call(L, 3, 0);
}

void CallbackHandler::DeviceChange(DeviceChangeEvent *event) {
	lua_State *L = event->luaState;

	lua_rawgeti(L, LUA_REGISTRYINDEX, event->funcRef);
	lua_pushstring(L, event->devMod);
	lua_pushnumber(L, event->devIdx);
	lua_pushboolean(L, event->bAdded);
	lua_call(L, 3, 0);
}

void CallbackHandler::PageChange(PageChangeEvent *event) {
	lua_State *L = event->luaState;

	lua_rawgeti(L, LUA_REGISTRYINDEX, event->funcRef);
	lua_pushstring(L, event->devMod);
	lua_pushnumber(L, event->devIdx);
	lua_pushnumber(L, event->dwPage);
	lua_pushboolean(L, event->bSetActive);
	lua_call(L, 4, 0);
}

CallbackHandler *CallbackHandler::Factory(int mode) {
	switch (mode) {
	case CBM_EVENT:
		return new CallbackEvent();
#ifdef WITH_FSUIPC
	case CBM_FSUIPC:
		return new CallbackFSUIPC();
#endif
	}
	// Default mode
	return new CallbackLua();
}
