#pragma once

struct lua_State;
void TraceL(lua_State* L, const char *format, ...);

#ifndef _DEBUG
#	define DebugL(a, b, ...)
#	define DebugC(a, ...)
#	define DebugF(a, ...)
#else

#define DebugL TraceL

void DebugC(wchar_t *format, ...);
void DebugF(wchar_t *format, ...);

#endif
