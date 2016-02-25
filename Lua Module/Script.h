#ifndef SCRIPT_H
#define SCRIPT_H

#include "Device.h"
#include "Import/Source/ThreadLock.h"

struct lua_State;

typedef struct _CallbackList {
	int pageButtonCallbackRef;
	int softButtonCallbackRef;
	int softButDownCallbackRef;
	int softButUpCallbackRef;
} CallbackList;

typedef struct _ScriptInfo {
	lua_State *luaState;
	int luaRef;

	int deviceChangeCallbackRef;
	CallbackList HID[HID_COUNT];

} ScriptInfo;

class ScriptManager {

private:
	CThreadLock _lock;

	ScriptManager() {
		_scriptCount = HID_EMPTY;
		for (int i = 0; i < LUA_COUNT; i++) {
			_luaScript[i].luaState = NULL;
		}
	}

	ScriptInfo _luaScript[LUA_COUNT];
	int _scriptCount;

	int FindFreeSlot() {
		for (int i = 0; i < LUA_COUNT; i++)
			if (_luaScript[i].luaState == NULL)
				return i;
		return HID_NOTFOUND;
	}

public:
	static ScriptManager *GetInstance() {
		static ScriptManager *instance = new ScriptManager();
		return instance;
	}

	ScriptInfo *GetFreeSlot(lua_State *L, int ref);
	ScriptInfo *GetSlot(lua_State *L);
	int ReleaseSlot(lua_State *L);

	void CallSoftButtonCallbacks(int index, DWORD dwButtons);
	void CallPageChangeCallbacks(int index, DWORD dwPage, bool bSetActive);

	void CallDeviceChangeCallbacks(int index, bool bAdded);
};

extern ScriptManager *LuaMan;

#endif