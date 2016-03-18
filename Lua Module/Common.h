#ifndef COMMON_H
#define COMMON_H

#include <Windows.h>
#include "Debug.h"
#include "Config.h"

#define SCREEN_WIDTH	320
#define SCREEN_HEIGHT	240
#define SCREEN_BUFSIZE	(SCREEN_WIDTH * SCREEN_HEIGHT * 3)

#define _LIB_NAME	"passerelle"
#define _LIB_OPEN	luaopen_passerelle

#define LUALIB_NAME		_LIB_NAME
#define LUALIB_IDENT	LUALIB_NAME "LuaLib"
#define LUALIB_TABLE	LUALIB_IDENT "Data"
#define LUALIB_OPEN( )	int __declspec(dllexport) __cdecl _LIB_OPEN (lua_State* L)

#define META_BITMAP	_LIB_NAME "BITMAP"

#define DIRECTINPUT_VERSION 0x0800

#endif
