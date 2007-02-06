//	WinDjView
//	Copyright (C) 2004-2007 Andrew Zhezherun
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License version 2
//	as published by the Free Software Foundation.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//	http://www.gnu.org/copyleft/gpl.html

// $Id$

#pragma once


struct CDisplaySettings;

class CDIB : public CBitmap
{
public:
	virtual ~CDIB();
	void Draw(CDC* pDC, const CPoint& ptOffset);
	void Draw(CDC* pDC, const CPoint& ptOffset, const CSize& szScale);
	void DrawDC(CDC* pDC, const CPoint& ptOffset);
	void DrawDC(CDC* pDC, const CPoint& ptOffset, const CRect& rcPart);

	int GetWidth() const { return m_pBMI->bmiHeader.biWidth; }
	int GetHeight() const { return m_pBMI->bmiHeader.biHeight; }
	CSize GetSize() const { return CSize(GetWidth(), GetHeight()); }
	LPBYTE GetBits() { return m_pBits; }
	int GetBitsPerPixel() const { return m_pBMI->bmiHeader.biBitCount; }
	int GetColorCount() const { return m_pBMI->bmiHeader.biClrUsed; }
	RGBQUAD* GetPalette() { return &m_pBMI->bmiColors[0]; }
	bool IsValid() const { return m_hObject != NULL; }

	static CDIB* CreateDIB(const BITMAPINFO* pBMI);
	static CDIB* CreateDIB(CDIB* pSource, int nBitCount = -1);
	static CDIB* CreateDIB(int nWidth, int nHeight, int nBitCount);

	CDIB* ReduceColors();
	void Save(LPCTSTR pszPathName) const;

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
class CLightweightDIB : protected CDIB
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

struct CPrintSettings
{
	CPrintSettings() :
		fMarginLeft(0.0), fMarginTop(0.0), fMarginRight(0.0), fMarginBottom(0.0),
		fPosLeft(0.0), fPosTop(0.0), bCenterImage(true), bClipContent(false),
		fScale(100.0), bShrinkOversized(true), bScaleToFit(false),
		bIgnorePrinterMargins(false) {}

	double fMarginLeft;
	double fMarginTop;
	double fMarginRight;
	double fMarginBottom;

	double fPosLeft;
	double fPosTop;
	BOOL bCenterImage;
	BOOL bClipContent;

	double fScale;
	BOOL bShrinkOversized;
	BOOL bScaleToFit;
	BOOL bIgnorePrinterMargins;
};

CDIB* RenderPixmap(GPixmap& pm, const CDisplaySettings& displaySettings);
CDIB* RenderBitmap(GBitmap& bm, const CDisplaySettings& displaySettings);
CDIB* RenderPixmap(GPixmap& pm, const CRect& rcClip, const CDisplaySettings& displaySettings);
CDIB* RenderBitmap(GBitmap& bm, const CRect& rcClip, const CDisplaySettings& displaySettings);
CDIB* RenderEmpty(const CSize& szBitmap, const CDisplaySettings& displaySettings);

void PrintPage(CDC* pDC, GP<DjVuImage> pImage, int nRotate, int nMode, const CRect& rcPage,
	double fPrinterMMx, double fPrinterMMy, CPrintSettings& settings, bool bPreview = false);

unsigned int __stdcall PrintThreadProc(void* pvData);

void FrameRect(CDC* pDC, const CRect& rect, COLORREF color);
void DrawDottedLine(CDC* pDC, COLORREF color, CPoint ptStart, CPoint ptEnd);
