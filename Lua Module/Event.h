#pragma once

#define EVT_BUTTONCHANGE	1
#define EVT_PAGECHANGE		2
#define EVT_DEVICECHANGE	3

struct lua_State;

struct Event {
	Event(int type, lua_State *L) {
		this->type = type;
		luaState = L;
	}
	int type;
	lua_State *luaState;
	int funcRef;
};
