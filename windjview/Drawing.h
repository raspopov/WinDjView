//	WinDjView 0.1
//	Copyright (C) 2004 Andrew Zhezherun
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
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  http://www.gnu.org/copyleft/gpl.html

// $Id$

#pragma once


class CDIB : public CBitmap
{
public:
	virtual ~CDIB() { free(m_pBMI); }
	void Draw(CDC* pDC, const CPoint& ptOffset);
	void Draw(CDC* pDC, const CPoint& ptOffset, const CSize& szScale);
	void DrawDC(CDC* pDC, const CPoint& ptOffset);

	int GetWidth() const { return m_pBMI->bmiHeader.biWidth; }
	int GetHeight() const { return m_pBMI->bmiHeader.biHeight; }
	LPBYTE GetBits() { return m_pBits; }
	int GetBitsPerPixel() const { return m_pBMI->bmiHeader.biBitCount; }
	int GetColorCount() const { return m_pBMI->bmiHeader.biClrUsed; }
	RGBQUAD* GetPalette() { return &m_pBMI->bmiColors[0]; }

	static CDIB* CreateDIB(const BITMAPINFO* pBMI);
	static CDIB* CreateDIB(CDIB* pSource, int nBitCount = -1);
	static CDIB* CreateDIB(int nWidth, int nHeight, int nBitCount);

	CDIB* ReduceColors();

protected:
	CDIB() : CBitmap(), m_pBits(NULL), m_pBMI(NULL) {}

	LPBYTE m_pBits;
	BITMAPINFO* m_pBMI;
};

struct CPrintSettings
{
	CPrintSettings() :
		fMarginLeft(0.0), fMarginTop(0.0), fMarginRight(0.0), fMarginBottom(0.0),
		fPosLeft(0.0), fPosTop(0.0), bCenterImage(false), bClipContent(false),
		fScale(100.0), bShrinkOversized(false), bScaleToFit(false) {}

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
};

CDIB* RenderPixmap(GPixmap& pm);
CDIB* RenderBitmap(GBitmap& bm);
CDIB* RenderPixmap(GPixmap& pm, const CRect& rcClip);
CDIB* RenderBitmap(GBitmap& bm, const CRect& rcClip);

void PrintPage(CDC* pDC, GP<DjVuImage> pImage, const CRect& rcPage,
	double fPrinterMMx, double fPrinterMMy, CPrintSettings& settings, bool bPreview = false);

DWORD WINAPI PrintThreadProc(LPVOID pvData);

