#ifndef API_H
#define API_H

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
LUA_FUNC(Listen);
LUA_FUNC(Release);

LUA_FUNC(GetVersion);
LUA_FUNC(GetNumDevices);

LUA_FUNC(AddPage);
LUA_FUNC(RemovePage);
LUA_FUNC(RegisterPageCallback);
LUA_FUNC(RegisterDeviceChangeCallback);
LUA_FUNC(SetLed);
LUA_FUNC(RegisterSoftButtonCallback);
LUA_FUNC(RegisterSoftButtonUpCallback);
LUA_FUNC(RegisterSoftButtonDownCallback);
LUA_FUNC(SetImageFromFile);
LUA_FUNC(SetImage);
LUA_FUNC(SetString);
LUA_FUNC(SetProfile);

#endif
