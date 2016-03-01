#ifndef API_H
#define API_H

#define ERR_NONE			0
#define ERR_HANDLE			1
#define ERR_NOTIMPL			2
#define ERR_INVALIDARG		3
#define ERR_PAGENOTACTIVE	4
#define ERR_OUTOFMEMORY		5
#define ERR_UNKNOWN			-1

#define LUA_CONST(x)	{ #x, x }

#define LUA_FUNC(x)		int luaX_##x(lua_State* L)
#define LUA_OBSOLETE(x)	int luaX_##x(lua_State* L) { \
							DebugL(L, "The method '" #x "' is obsolete."); \
							return 0; \
						}
#define LUA_START(x)	static const luaL_reg x[] = {
#define LUA_ENTRY(x)		{ #x, luaX_##x },
#define LUA_END( )			{ NULL, NULL } \
						};

LUA_FUNC(Initialize);
LUA_FUNC(Release);

LUA_FUNC(listen);
LUA_FUNC(getVersion);
LUA_FUNC(getNumDevices);

LUA_FUNC(addPage);
LUA_FUNC(removePage);
LUA_FUNC(registerPageCallback);
LUA_FUNC(registerDeviceChangeCallback);
LUA_FUNC(setLed);
LUA_FUNC(registerSoftButtonCallback);
LUA_FUNC(registerSoftButtonUpCallback);
LUA_FUNC(registerSoftButtonDownCallback);
LUA_FUNC(setImageFromFile);
LUA_FUNC(setImage);
LUA_FUNC(setString);
LUA_FUNC(setProfile);

#endif
