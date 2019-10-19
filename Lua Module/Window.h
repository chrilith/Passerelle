#pragma once

int luaF_RegisterWindow(lua_State *L);
HWND *luaF_WindowPush(lua_State *L, HWND pVar);
