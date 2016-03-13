#include "Common.h"
#include "lua.hpp"
#include <tchar.h>

void TraceL(lua_State* L, const char *format, ...) {
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

#ifdef _DEBUG

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

void DebugC(wchar_t *format, ...) {
	TCHAR buf[1024];

	va_list vargs;
	__crt_va_start(vargs, format);
	vswprintf(buf, sizeof(buf), format, vargs);
	__crt_va_end(vargs);

	OutputDebugString(buf);
}

void DebugF(wchar_t *format, ...) {
	TCHAR buf[1024];
	TCHAR path[_MAX_PATH];

	va_list vargs;
	__crt_va_start(vargs, format);
	vswprintf(buf, sizeof(buf), format, vargs);
	__crt_va_end(vargs);

	GetModuleFileName((HMODULE)&__ImageBase, path, _countof(path));
	wcscat_s(path, _T(".log"));

	HANDLE f = CreateFile(path, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (f != INVALID_HANDLE_VALUE) {
		WriteFile(f, buf, (DWORD)wcslen(buf) * sizeof(TCHAR), NULL, NULL);
		CloseHandle(f);
	}
}

#endif
