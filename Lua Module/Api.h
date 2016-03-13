#ifndef API_H
#define API_H

// Errors
#define ERR_NONE			0
#define ERR_HANDLE			1
#define ERR_NOTIMPL			2
#define ERR_INVALIDARG		3
#define ERR_PAGENOTACTIVE	4
#define ERR_OUTOFMEMORY		5
#define ERR_UNKNOWN			-1

// X52 Pro
#define BIT_SCROLLWHEEL_CLICK	SoftButton_Select
#define BIT_SCROLLWHEEL_UP		SoftButton_Up
#define BIT_SCROLLWHEEL_DOWN	SoftButton_Down

// FIP
#define BIT_RIGHTROTARY_CW		SoftButton_Up
#define BIT_RIGHTROTARY_CCW		SoftButton_Down
#define BIT_LEFTROTARY_CW		SoftButton_Right
#define BIT_LEFTROTARY_CCW		SoftButton_Left
#define BIT_S1BUTTON			SoftButton_1
#define BIT_S2BUTTON			SoftButton_2
#define BIT_S3BUTTON			SoftButton_3
#define BIT_S4BUTTON			SoftButton_4
#define BIT_S5BUTTON			SoftButton_5
#define BIT_S6BUTTON			SoftButton_6

typedef struct {
	const char *name;
	int value;
} luaI_const;

typedef struct {
	const char *name;
	const char *value;
} luaI_literal;

#define LUA_CONST(x)	{ #x, x },

#define LUA_FUNC(x)		int luaX_##x(lua_State* L)
#define LUA_OBSOLETE(x)	int luaX_##x(lua_State* L) { \
							TraceL(L, LUALIB_NAME ": the method '" #x "' is obsolete"); \
							return 0; \
						}
#define LUA_START(x, T)	static const T x[] = {
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
LUA_FUNC(poll);
LUA_FUNC(sleep);
LUA_FUNC(setMode);

#endif
