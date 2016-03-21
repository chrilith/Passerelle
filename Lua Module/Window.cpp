#include "Common.h"
#include "Api.h"
#include "Bitmap.h"
#include "Window.h"
#include "Utils.h"
#include "lua.hpp"

HWND *luaF_WindowPush(lua_State *L, HWND pVar) {
	HWND *p = (HWND *)lua_newuserdata(L, sizeof(HWND));
	*p = pVar;
	luaL_getmetatable(L, META_WINDOW);
	lua_setmetatable(L, -2);
	DebugL(L, "Push Window: %x %x", p, pVar);
	return p;
}

static HWND luaF_WindowGet(lua_State *L, int index) {
	HWND *p = (HWND *)lua_touserdata(L, index);
	if (p == NULL) {
		luaL_typerror(L, index, META_WINDOW);
	}
	return *p;
}

static HWND luaF_WindowCheck(lua_State *L, int index) {

	luaL_checktype(L, index, LUA_TUSERDATA);
	HWND *p = (HWND *)luaL_checkudata(L, index, META_WINDOW);

	if (p == NULL) { luaL_typerror(L, index, META_WINDOW); }
	if (*p == NULL) { luaL_error(L, "Null " META_WINDOW); }

	return *p;
}

LUA_SFUNC(__tostring) {
	lua_pushfstring(L, "Window: %p", lua_touserdata(L, 1));
	return 1;
}

LUA_SFUNC(capture) {
	HWND winH = luaF_WindowCheck(L, 1);
	if (!winH) { return 0; }

	int p = lua_gettop(L);

	if (/* Base signature */
		!(p == 5 &&
			lua_isnumber(L, 2) &&		// x offset
			lua_isnumber(L, 3) &&		// y offset
			lua_isnumber(L, 4) &&		// width
			lua_isnumber(L, 5))			// height
		) {
		return 0;
	}

	// Copy size in %
	double x = lua_tonumber(L, 2);
	double y = lua_tonumber(L, 3);
	double width = lua_tonumber(L, 4);
	double height = lua_tonumber(L, 5);

	RECT rect;
	GetClientRect(winH, &rect);

	POINT pt, pt2;
	pt.x = rect.left;
	pt.y = rect.top;
	pt2.x = rect.right + 1;
	pt2.y = rect.bottom + 1;

	int ww = (pt2.x - pt.x + 1);
	int wh = (pt2.y - pt.y + 1);

	int cx = rect.left + (LONG)(x * ww / 100.0);
	int cy = rect.top + (LONG)(y * wh / 100.0);
	int cw = (LONG)(width * ww / 100.0);
	int ch = (LONG)(height * wh / 100.0);

	return luaF_BitmapCaptureFromWindow(L, winH, cx, cy, cw, ch);
}

LUA_SFUNC(isValid) {
	HWND winH = luaF_WindowCheck(L, 1);

	lua_pushboolean(L, winH && IsWindow(winH));
	return 1;
}

int luaF_RegisterWindow(lua_State *L) {
	LUA_START(METHODS, luaL_reg)
		LUA_ENTRY(capture)
		LUA_ENTRY(isValid)
	LUA_END()

	LUA_START(META, luaL_reg)
		// Nothing to gc
		LUA_ENTRY(__tostring)
	LUA_END()

	/* Hum... this seems to not put the METHODS in _G.
	Supposed to be into the table on the top of the stack
	but what is this table?
	*/
	luaL_openlib(L, TYPE_WINDOW, METHODS, 0);

	// http://lua-users.org/wiki/UserDataWithPointerExample

	luaL_newmetatable(L, META_WINDOW);
	luaL_openlib(L, 0, META, 0);		// fill metatable
	lua_pushliteral(L, "__index");
	lua_pushvalue(L, -3);				// dup methods table
	lua_rawset(L, -3);					// metatable.__index = methods
	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, -3);				// dup methods table (again?)
	lua_rawset(L, -3);                  /* hide metatable:
										metatable.__metatable = methods */
	lua_pop(L, 1);						// drop metatable
	return 1;							// return methods on the stack
}
