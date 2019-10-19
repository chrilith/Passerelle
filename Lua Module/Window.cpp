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
	pt2.x = rect.right;
	pt2.y = rect.bottom;

	int ww = (pt2.x - pt.x + 1);
	int wh = (pt2.y - pt.y + 1);

	int cx = rect.left + (LONG)(x * ww / 100.0);
	int cy = rect.top + (LONG)(y * wh / 100.0);
	int cw = (LONG)(width * ww / 100.0);
	int ch = (LONG)(height * wh / 100.0);

	return luaF_BitmapCaptureFromWindow(L, winH, cx, cy, cw, ch);
}


LUA_SFUNC(setPosition) {
	HWND winH = luaF_WindowCheck(L, 1);
	if (!winH) { return 0; }

	int p = lua_gettop(L);

	if (/* Base signature */
		!(p == 3 &&
			lua_isnumber(L, 2) &&		// x position
			lua_isnumber(L, 3))			// y position
		) {
		return 0;
	}

	double x = lua_tonumber(L, 2);
	double y = lua_tonumber(L, 3);

	SetWindowPos(winH, 0, (int)x, (int)y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	return 0;
}

LUA_SFUNC(setSize) {
	HWND winH = luaF_WindowCheck(L, 1);
	if (!winH) { return 0; }

	int p = lua_gettop(L);

	if (/* Base signature */
		!(p == 3 &&
			lua_isnumber(L, 2) &&		// width
			lua_isnumber(L, 3))			//height
		) {
		return 0;
	}

	double w = lua_tonumber(L, 2);
	double h = lua_tonumber(L, 3);

	RECT rw, rc;

	GetClientRect(winH, &rc);
	int cw = w - (rc.right - rc.left + 1);
	int ch = h - (rc.bottom - rc.top + 1);

	GetWindowRect(winH, &rw);
	int ww = rw.right + rw.left + 1;
	int wh = rw.bottom + rw.top + 1;

	ww += cw - 1;
	wh += ch - 1;
	// If the window menu is very long, it may be displayed on multiple lines
	// when reducing the size of the window. Use may have to call this function twice
	// to properly size the window.
	SetWindowPos(winH, 0, 0, 0, ww, wh, SWP_NOMOVE | SWP_NOZORDER);

	return 0;
}

LUA_SFUNC(isValid) {
	HWND winH = luaF_WindowCheck(L, 1);

	lua_pushboolean(L, winH && IsWindow(winH));
	return 1;
}

LUA_SFUNC(isMinimized) {
	HWND winH = luaF_WindowCheck(L, 1);

	lua_pushboolean(L, winH && IsIconic(winH));
	return 1;
}

LUA_SFUNC(isMaximized) {
	HWND winH = luaF_WindowCheck(L, 1);

	lua_pushboolean(L, winH && IsZoomed(winH));
	return 1;
}

LUA_SFUNC(minimize) {
	HWND winH = luaF_WindowCheck(L, 1);
	if (!winH) { return 0; }
	ShowWindow(winH, SW_FORCEMINIMIZE);
	return 0;
}

LUA_SFUNC(maximize) {
	HWND winH = luaF_WindowCheck(L, 1);
	if (!winH) { return 0; }
	ShowWindow(winH, SW_MAXIMIZE);
	return 0;
}

LUA_SFUNC(restore) {
	HWND winH = luaF_WindowCheck(L, 1);
	if (!winH) { return 0; }
	ShowWindow(winH, SW_RESTORE);
	return 0;
}

LUA_SFUNC(getSize) {
	HWND winH = luaF_WindowCheck(L, 1);

	int w = 0;
	int h = 0;

	if (winH) {
		RECT r;
		GetClientRect(winH, &r);
		w = r.right - r.left + 1;
		h = r.bottom - r.top + 1;
	}

	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 2;
}


int luaF_RegisterWindow(lua_State *L) {
	LUA_START(METHODS, luaL_reg)
		LUA_ENTRY(capture)
		LUA_ENTRY(setPosition)
		LUA_ENTRY(setSize)
		LUA_ENTRY(isValid)
		LUA_ENTRY(isMinimized)
		LUA_ENTRY(isMaximized)
		LUA_ENTRY(minimize)
		LUA_ENTRY(maximize)
		LUA_ENTRY(restore)
		LUA_ENTRY(getSize)
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
