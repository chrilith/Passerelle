#include "Common.h"
#include "Bitmap.h"
#include "Api.h"
#include "lua.hpp"
#include "Image.h"

static Image **luaF_BitmapPush(lua_State *L, Image *pImg) {
	Image **p = (Image **)lua_newuserdata(L, sizeof(Image **));
	*p = pImg;
	luaL_getmetatable(L, META_BITMAP);
	lua_setmetatable(L, -2);
	return p;
}

static Image *luaF_BitmapGet(lua_State *L, int index) {
	Image **p = (Image **)lua_touserdata(L, index);
	if (p == NULL) {
		luaL_typerror(L, index, META_BITMAP);
	}
	return *p;
}

Image *luaF_BitmapCheck(lua_State *L, int index) {

	luaL_checktype(L, index, LUA_TUSERDATA);
	Image **p = (Image **)luaL_checkudata(L, index, META_BITMAP);

	if ( p == NULL)	{ luaL_typerror(L, index, META_BITMAP); }
	if (*p == NULL)	{ luaL_error(L, "null Capture"); }

	return *p;
}

int luaF_BitmapCaptureFromWindow(lua_State *L, HWND winH, LONG x, LONG y, LONG width, LONG height) {
	HDC winHdc = GetDC(winH);
	Image *img = new Image(winHdc, width, height);

	HDC selH = img->BeginPaint();
	BitBlt(selH, 0, 0, width, height, winHdc, x, y, SRCCOPY);
	img->EndPaint();

	ReleaseDC(winH, winHdc);

	luaF_BitmapPush(L, img);
	return 1;
}

LUA_FUNC(__gc) {
	Image *p = luaF_BitmapGet(L, 1);
	if (p) { delete p; }
	return 0;
}

LUA_FUNC(__tostring) {
	lua_pushfstring(L, "Image: %x", lua_touserdata(L, 1));
	return 1;
}

LUA_FUNC(getSize) {
	Image *p = luaF_BitmapCheck(L, 1);

	lua_pushnumber(L, p->Width());
	lua_pushnumber(L, p->Height());
	return 2;
}

LUA_FUNC(toGd2) {
	Image *p = luaF_BitmapCheck(L, 1);

	int size;
	const char *ptr = p->ToGD2(&size);
	lua_pushlstring(L, ptr, size);
	delete ptr;

	return 1;
}

int luaF_RegisterBitmap(lua_State *L) {
	LUA_START(METHODS, luaL_reg)
		LUA_ENTRY(getSize)
		LUA_ENTRY(toGd2)
	LUA_END()

	LUA_START(META, luaL_reg)
		LUA_ENTRY(__gc)
		LUA_ENTRY(__tostring)
	LUA_END()

	/* Hum... this seems to not put the METHODS in _G.
	Supposed to be into the table on the top of the stack
	but what is this table?
	*/
	luaL_openlib(L, NULL, METHODS, 0);

	// http://lua-users.org/wiki/UserDataWithPointerExample

	luaL_newmetatable(L, META_BITMAP);
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
