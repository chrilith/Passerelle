#pragma once

#ifndef _DEBUG
#	define DebugL(a, b, ...)
#	define DebugC(a, ...)
#else

struct lua_State;

void DebugL(lua_State* L, const char *format, ...);
void DebugC(wchar_t *format, ...);

#endif
