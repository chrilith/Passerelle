#pragma once

struct lua_State;
class Image;

int luaF_RegisterBitmap(lua_State *L);
Image *luaF_BitmapCheck(lua_State *L, int index);
int luaF_BitmapCaptureFromWindow(lua_State *L, HWND winH, LONG x, LONG y, LONG width, LONG height);
