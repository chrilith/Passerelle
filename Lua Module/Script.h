#ifndef SCRIPT_H
#define SCRIPT_H

#include "Device.h"
#include "Import/Source/ThreadLock.h"

#define SCRIPT_COUNT		10

struct lua_State;
class CallbackHandler;
class CallbackLua;

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
	CallbackHandler *events;
	CallbackList HID[HID_COUNT];

} ScriptInfo;

class ScriptManager {

private:
	CThreadLock _lock;
	ScriptInfo _luaScript[SCRIPT_COUNT];
	int _scriptCount;

	ScriptManager();
	int FindFreeSlot();

public:
	static ScriptManager *GetInstance() {
		static ScriptManager *instance = new ScriptManager();
		return instance;
	}

	ScriptInfo *GetFreeSlot(lua_State *L);
	ScriptInfo *GetSlot(lua_State *L);
	int ReleaseSlot(lua_State *L);

	bool HaveSlot(lua_State *L) { return GetSlot(L) != NULL; }

	void CallSoftButtonCallbacks(int index, DWORD dwButtons);
	void CallPageChangeCallbacks(int index, DWORD dwPage, bool bSetActive);

	void CallDeviceChangeCallbacks(int index, bool bAdded);
};

extern ScriptManager *LuaMan;

#endif
