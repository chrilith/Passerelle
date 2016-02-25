#pragma once

#include <atlimage.h>

namespace Utils {

	int CharToWideConverter(const char *s, wchar_t **d);
	void RenderStretchedImage(HDC hdc, LPCTSTR tsz);

}
