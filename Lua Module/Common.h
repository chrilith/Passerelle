#ifndef COMMON_H
#define COMMON_H

#include <Windows.h>

#if LEGACY_MODE == 1
#	define LUALIB_NAME	"saitek"
#else
#	define LUALIB_NAME	"passerelle"
#endif

#define LUALIB_IDENT	LUALIB_NAME "LuaLib"
#define LUALIB_TABLE	LUALIB_IDENT "Data"
#define LUALIB_OPEN()	int __declspec(dllexport) __cdecl luaopen_ ## LUALIB_NAME(lua_State* L)

#endif
