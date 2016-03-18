#include <tchar.h>
#include "Common.h"
#include "Utils.h"

namespace Utils {

int CharToWideConverter(const char *s, wchar_t **d) {
	if (s == NULL) {
		*d = NULL;
		return 0;
	}
	size_t baseSize = strlen(s) + 1;
	size_t convertedChars = 0;

	*d = (wchar_t *)malloc(baseSize * 2);
	mbstowcs_s(&convertedChars, *d, baseSize, s, _TRUNCATE);

	return (int)convertedChars;
}

void RenderStretchedImage(HDC hdc, LPCTSTR tsz) {
	CImage image;
	HRESULT hr = image.Load(tsz);
	if (SUCCEEDED(hr)) {
		int old = SetStretchBltMode(hdc, COLORONCOLOR);
		image.StretchBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SRCCOPY);
		SetStretchBltMode(hdc, old);
	}
}

}
