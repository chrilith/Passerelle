#include "Common.h"
#include "lua.hpp"

#ifdef _DEBUG

void DebugL(lua_State* L, const char *format, ...) {
	CHAR buf[1024];

	va_list vargs;
	__crt_va_start(vargs, format);
	vsnprintf(buf, sizeof(buf), format, vargs);
	__crt_va_end(vargs);

	// Call lua "print" function
	lua_getfield(L, LUA_GLOBALSINDEX, "print");
	lua_pushstring(L, buf);
	lua_call(L, 1, 0);
}

void DebugC(wchar_t *format, ...) {
	WCHAR buf[1024];

	va_list vargs;
	__crt_va_start(vargs, format);
	vswprintf(buf, sizeof(buf), format, vargs);
	__crt_va_end(vargs);

	OutputDebugString(buf);
}

#endif