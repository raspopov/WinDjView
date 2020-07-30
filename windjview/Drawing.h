//	WinDjView
//	Copyright (C) 2004-2015 Andrew Zhezherun
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along
//	with this program; if not, write to the Free Software Foundation, Inc.,
//	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//	http://www.gnu.org/copyleft/gpl.html

#pragma once


struct CDisplaySettings;
struct CPrintSettings;

class CDIB : public CBitmap
{
public:
	virtual ~CDIB();
	void Draw(CDC* pDC, const CPoint& ptOffset);
	void Draw(CDC* pDC, const CPoint& ptOffset, const CSize& szScale);
	void DrawDC(CDC* pDC, const CPoint& ptOffset);
	void DrawDC(CDC* pDC, const CPoint& ptOffset, const CRect& rcPart);

	int GetWidth() const { return m_pBMI->bmiHeader.biWidth; }
	int GetHeight() const { return abs(m_pBMI->bmiHeader.biHeight); }
	CSize GetSize() const { return CSize(GetWidth(), GetHeight()); }
	LPBYTE GetBits() { return m_pBits; }
	int GetBitsPerPixel() const { return m_pBMI->bmiHeader.biBitCount; }
	int GetColorCount() const { return m_pBMI->bmiHeader.biClrUsed; }
	RGBQUAD* GetPalette() { return &m_pBMI->bmiColors[0]; }
	bool IsValid() const { return m_hObject != NULL; }
	const BITMAPINFO* GetBitmapInfo() const { return m_pBMI; }

	static CDIB* CreateDIB(const BITMAPINFO* pBMI);
	static CDIB* CreateDIB(CDIB* pSource, int nBitCount = -1);
	static CDIB* CreateDIB(int nWidth, int nHeight, int nBitCount);

	enum ImageFormat
	{
		FormatBMP = 1,
		FormatPNG = 2,
		FormatGIF = 3,
		FormatTIF = 4,
		FormatJPG = 5
	};

	CDIB* ReduceColors();
	CDIB* Crop(const CRect& rcCrop);
	void SetDPI(int nDPI);
	bool Save(LPCTSTR pszPathName, ImageFormat nFormat = FormatBMP) const;
	HGLOBAL SaveToMemory() const;

protected:
	CDIB() : CBitmap(), m_pBits(NULL), m_pBMI(NULL),
			 m_hFile(NULL), m_hSection(NULL) {}

	void Create(const BITMAPINFO* pBMI);
	void Create(CDIB* pSource, int nBitCount = -1);

	LPBYTE m_pBits;
	BITMAPINFO* m_pBMI;

	HANDLE m_hFile;
	HANDLE m_hSection;
};

// A lightweight DIB does not use a bitmap handle.
// This allows to keep many such DIBs in memory
// on systems where there is a limit on the number
// of bitmap handles open (e.g., Windows 98).
// Thus, CLightweightDIBs are used to store thumbnails.
class CLightweightDIB : public CDIB
{
public:
	~CLightweightDIB();
	bool IsValid() const { return m_pBits != NULL; }

	using CDIB::Draw;
	using CDIB::GetWidth;
	using CDIB::GetHeight;
	using CDIB::GetSize;

	static CLightweightDIB* Create(CDIB* pSrc);

protected:
	CLightweightDIB() {}
};

class COffscreenDC : public CDC
{
public:
	COffscreenDC();
	virtual ~COffscreenDC();

	bool Create(CDC* pDC, CSize size);
	void Release();

	CDIB* GetDIB() { return m_pBitmap; }

private:
	CDIB* m_pBitmap;
	CBitmap* m_pOldBitmap;
	CSize m_szBitmap;
};

class CScreenDC : public CDC
{
public:
	CScreenDC();
};

CDIB* RenderPixmap(GPixmap& pm, const CDisplaySettings& displaySettings);
CDIB* RenderBitmap(GBitmap& bm, const CDisplaySettings& displaySettings);
CDIB* RenderPixmap(GPixmap& pm, const CRect& rcClip, const CDisplaySettings& displaySettings);
CDIB* RenderBitmap(GBitmap& bm, const CRect& rcClip, const CDisplaySettings& displaySettings);
CDIB* RenderEmpty(const CSize& szBitmap, const CDisplaySettings& displaySettings);

void PrintPage(CDC* pDC, GP<DjVuImage> pImage, int nRotate, int nMode, const CRect& rcPage,
	double fPrinterMMx, double fPrinterMMy, CPrintSettings& settings,
	GRect* pSelRect = NULL, bool bPreview = false);

unsigned int __stdcall PrintThreadProc(void* pvData);

void FrameRect(CDC* pDC, const CRect& rect, COLORREF color);
void InvertFrame(CDC* pDC, const CRect& rect);
void FrameOval(CDC* pDC, const CRect& bounds, COLORREF color);
void InvertOvalFrame(CDC* pDC, const CRect& bounds);
void FramePoly(CDC* pDC, const POINT* points, int nCount, COLORREF color);
void InvertPolyFrame(CDC* pDC, const POINT* points, int nCount);
void DrawDottedLine(CDC* pDC, const CPoint& ptStart, const CPoint& ptEnd, COLORREF color);
void DrawDottedRect(CDC* pDC, const CRect& rect, COLORREF color);
void HighlightRect(CDC* pDC, const CRect& rect, COLORREF color, double fTransparency);
void HighlightPolygon(CDC* pDC, const POINT* points, int nCount, COLORREF color, double fTransparency);

enum ArrowType
{
	ARR_LEFT = 1,
	ARR_RIGHT = 2,
	ARR_UP = 3,
	ARR_DOWN = 4
};

void DrawArrow(CDC* pDC, int nArrowType, const CRect& rect, COLORREF color);

inline COLORREF ChangeBrightness(COLORREF color, double fFactor)
{
	int nRed = min(static_cast<int>(GetRValue(color)*fFactor + 0.5), 255);
	int nGreen = min(static_cast<int>(GetGValue(color)*fFactor + 0.5), 255);
	int nBlue = min(static_cast<int>(GetBValue(color)*fFactor + 0.5), 255);
	return RGB(nRed, nGreen, nBlue);
}

inline COLORREF AlphaCombine(COLORREF crFirst, COLORREF crSecond, BYTE nAlpha)
{
	return RGB((GetRValue(crFirst) * (255L - nAlpha) + GetRValue(crSecond) * (0L + nAlpha)) >> 8,
			(GetGValue(crFirst) * (255L - nAlpha) + GetGValue(crSecond) * (0L + nAlpha)) >> 8,
			(GetBValue(crFirst) * (255L - nAlpha) + GetBValue(crSecond) * (0L + nAlpha)) >> 8);
}

inline COLORREF InvertColor(COLORREF color)
{
	return RGB(255 - GetRValue(color), 255 - GetGValue(color), 255 - GetBValue(color));
}
