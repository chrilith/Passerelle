#pragma once

class Image {

public:
	Image() : Image(320, 240) { }
	Image(int width, int height) : Image(NULL, width, height) { }
	Image(HDC hdc, int width, int height);
	Image(const char *ptr);

	virtual ~Image();

	HDC BeginPaint();
	void Blit(HDC hdc);
	void Blit(Image *rgb);
	void EndPaint(BOOL refresh = true);
	const char *ToGD2(int *size);

	const HDC DC() const { return _hDC;  }
	const int Width() const { return _iWidth; }
	const int Height() const { return _iHeight; }
	const void *Bits() const { return _pBits;  }

private:
	HDC _hDC;
	HBITMAP _hBmp;
	int _iWidth, _iHeight, _iDepth;

	HDC _hOld;

	void *_pBits;

};

