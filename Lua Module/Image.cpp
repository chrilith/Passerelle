#include "Common.h"
#include "Image.h"
#include "Data.h"
#include "GD2.h"

Image::Image(const char *ptr) {
	_hDC = CreateCompatibleDC(NULL);

	_iDepth = 32;
	_iWidth = Read16(GD2H_WIDTH(ptr));
	_iHeight = Read16(GD2H_HEIGHT(ptr));
	_hBmp = CreateBitmap(_iWidth, _iHeight, 1, _iDepth, (void *)GD2H_BITS(ptr));

	// Get the _pBits address
	BITMAP bm;
	GetObject(_hBmp, sizeof(BITMAP), &bm);
	_pBits = bm.bmBits;
}

Image::Image(HDC hdc, int width, int height) {
	_hDC = CreateCompatibleDC(hdc);
	_iDepth = 24;

	BITMAPINFO bmpInfo = { 0 };
	bmpInfo.bmiHeader.biBitCount = _iDepth;
	bmpInfo.bmiHeader.biHeight = height;
	bmpInfo.bmiHeader.biWidth = width;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	_iWidth = width;
	_iHeight = height;
	_hBmp = CreateDIBSection(_hDC, &bmpInfo, DIB_RGB_COLORS, &_pBits, NULL, 0);
}

HDC Image::BeginPaint() {
	_hOld = (HDC)SelectObject(_hDC, _hBmp);
	return _hDC;
}

void Image::Blit(Image *rgb) {
	HDC hdc = rgb->BeginPaint();
	Blit(hdc);
	rgb->EndPaint(false);
}

void Image::Blit(HDC hdc) {
	BITMAP sbm;

	BeginPaint();

	HGDIOBJ hBmp = GetCurrentObject(hdc, OBJ_BITMAP);
	GetObject(hBmp, sizeof(BITMAP), &sbm);

	if (_iWidth == sbm.bmWidth && _iHeight == sbm.bmHeight) {
		BitBlt(_hDC, 0, 0, _iWidth, _iHeight, hdc, 0, 0, SRCCOPY);
	} else {
		int old = SetStretchBltMode(_hDC, STRETCH_HALFTONE);
		StretchBlt(_hDC, 0, 0, _iWidth, _iHeight, hdc, 0, 0, sbm.bmWidth, sbm.bmHeight, SRCCOPY);
		SetStretchBltMode(_hDC, old);
	}

	EndPaint();
}

void Image::EndPaint(BOOL refresh) {
	if (!_hOld) {
		return;
	}
	if (refresh) {
		GdiFlush();
	}
	SelectObject(_hDC, _hOld);
	_hOld = NULL;
}

const char *Image::ToGD2(int *size) {
	*size = GD_HEADER_SIZE + _iWidth *_iHeight * (32 / 8);
	char *ptr = new char[*size];

	// GD2_CHUNKSIZE_MIN(64) >= Chunk size <= GD2_CHUNKSIZE_MAX(4096)
	int chunk = _iWidth;
	// Format = GD2_FMT_RAW in truecolor (+2)
	int format = 1 + 2;
	// Number of chunks
	int ncx = _iWidth  / chunk + 1;
	int ncy = _iHeight / chunk + 1;

	/* Write GD header */

	Write32(GD2H_MAGIC(ptr),	 GD_MAGIC);		// GD2_ID
	Write16(GD2H_VER(ptr),		 2);			// GD2_VERS
	Write16(GD2H_WIDTH(ptr),	 _iWidth);
	Write16(GD2H_HEIGHT(ptr),	 _iHeight);
	Write16(GD2H_CHUNK(ptr),	 chunk);
	Write16(GD2H_FORMAT(ptr),	 format);
	Write16(GD2H_NCX(ptr),		 ncx);
	Write16(GD2H_NCY(ptr),		 ncy);
	Write8 (GD2H_TRUECOLOR(ptr), 1);			// truecolor flag
	Write8 (GD2H_TRANS(ptr),	 0);			// transparent flag

	/* Copy Image data */

	Image out(ptr);
	out.Blit(this);
	GetBitmapBits(out._hBmp, *size, GD2H_BITS(ptr));

	return ptr;
}

Image::~Image() {
	DeleteObject(_hBmp);
	DeleteDC(_hDC);
}
