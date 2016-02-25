#pragma once

#include <atlimage.h>

namespace Utils {

	extern __inline int CharToWideConverter(const char *s, wchar_t **d);
	extern __inline void RenderStretchedImage(HDC hdc, LPCTSTR tsz);

}
