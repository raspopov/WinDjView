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

#include "stdafx.h"
#include "WinDjView.h"
#include "Drawing.h"

#include "PrintDlg.h"
#include "ProgressDlg.h"
#include "DjVuDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDIB* RenderPixmap(GPixmap& pm)
{
	return RenderPixmap(pm, CRect(0, 0, pm.columns(), pm.rows()));
}

CDIB* RenderPixmap(GPixmap& pm, const CRect& rcClip)
{
	ASSERT(0 <= rcClip.left && 0 <= rcClip.top);
	ASSERT((int)pm.columns() >= rcClip.right && (int)pm.rows() >= rcClip.bottom);

	BITMAPINFOHEADER bmih;

	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = rcClip.Width();
	bmih.biHeight = rcClip.Height();
	bmih.biBitCount = 24;
	bmih.biCompression = BI_RGB;
	bmih.biClrUsed = 0;

	CDIB* pBitmap = CDIB::CreateDIB((BITMAPINFO*)&bmih);
	if (pBitmap->m_hObject == NULL)
		return pBitmap;

	LPBYTE pBits = pBitmap->GetBits();

	LPBYTE pNextBit = pBits;
	for (int y = rcClip.top; y < rcClip.bottom; ++y)
	{
		memcpy(pNextBit, pm[y] + rcClip.left, rcClip.Width()*3);

		pNextBit += rcClip.Width()*3;
		while ((pNextBit - pBits) % 4 != 0)
			++pNextBit;
	}

	return pBitmap;
}

CDIB* RenderBitmap(GBitmap& bm)
{
	return RenderBitmap(bm, CRect(0, 0, bm.columns(), bm.rows()));
}

CDIB* RenderBitmap(GBitmap& bm, const CRect& rcClip)
{
	ASSERT(0 <= rcClip.left && 0 <= rcClip.top);
	ASSERT((int)bm.columns() >= rcClip.right && (int)bm.rows() >= rcClip.bottom);

	LPBITMAPINFO pBMI;

	int nPaletteEntries = bm.get_grays();

	pBMI = (LPBITMAPINFO)malloc(
		sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*nPaletteEntries);
	BITMAPINFOHEADER& bmih = pBMI->bmiHeader;

	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = rcClip.Width();
	bmih.biHeight = rcClip.Height();
	bmih.biBitCount = 8;
	bmih.biCompression = BI_RGB;
	bmih.biClrUsed = nPaletteEntries;

	// Create palette for the bitmap
	int color = 0xff0000;
	int decrement = color/(nPaletteEntries - 1);
	for (int i = 0; i < nPaletteEntries; ++i)
	{
		int level = color >> 16;
		pBMI->bmiColors[i].rgbBlue = level;
		pBMI->bmiColors[i].rgbGreen = level;
		pBMI->bmiColors[i].rgbRed = level;
		color -= decrement;
	}

	CDIB* pBitmap = CDIB::CreateDIB(pBMI);
	free(pBMI);

	if (pBitmap->m_hObject == NULL)
		return pBitmap;

	LPBYTE pBits = pBitmap->GetBits();

	LPBYTE pNextBit = pBits;
	for (int y = rcClip.top; y < rcClip.bottom; ++y)
	{
		memcpy(pNextBit, bm[y] + rcClip.left, rcClip.Width());

		pNextBit += rcClip.Width();
		while ((pNextBit - pBits) % 4 != 0)
			++pNextBit;
	}

	return pBitmap;
}

CDIB* CDIB::CreateDIB(const BITMAPINFO* pBMI)
{
	CDIB* pDIB = new CDIB();

	UINT nSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*pBMI->bmiHeader.biClrUsed;
	pDIB->m_pBMI = (LPBITMAPINFO)malloc(nSize);
	memcpy(pDIB->m_pBMI, pBMI, nSize);

	BITMAPINFOHEADER& bmih = pDIB->m_pBMI->bmiHeader;
	bmih.biPlanes = 1;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrImportant = 0;

	HBITMAP hBitmap = ::CreateDIBSection(NULL,
		pDIB->m_pBMI, DIB_RGB_COLORS, (VOID**)&pDIB->m_pBits, NULL, 0);

	pDIB->Attach(hBitmap);
	return pDIB;
}

CDIB* CDIB::CreateDIB(CDIB* pSource, int nBitCount)
{
	ASSERT(nBitCount == -1 || nBitCount == 1 || nBitCount == 4 || nBitCount == 8 ||
		   nBitCount == 16 || nBitCount == 24 || nBitCount == 32);
	if (nBitCount == -1)
		nBitCount = pSource->m_pBMI->bmiHeader.biBitCount;

	int nPaletteEntries = pSource->m_pBMI->bmiHeader.biClrUsed;
	if (nBitCount <= 8)
		nPaletteEntries = min(1 << nBitCount, nPaletteEntries);
	else
		nPaletteEntries = 0;

	LPBITMAPINFO pBMI = (LPBITMAPINFO)malloc(
		sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*nPaletteEntries);
	BITMAPINFOHEADER& bmih = pBMI->bmiHeader;

	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = pSource->m_pBMI->bmiHeader.biWidth;
	bmih.biHeight = pSource->m_pBMI->bmiHeader.biHeight;
	bmih.biBitCount = nBitCount;
	bmih.biCompression = BI_RGB;
	bmih.biClrUsed = nPaletteEntries;

	memcpy(&pBMI->bmiColors[0], &pSource->m_pBMI->bmiColors[0], nPaletteEntries*sizeof(RGBQUAD));

	CDIB* pBitmap = CDIB::CreateDIB((BITMAPINFO*)&bmih);
	if (pBitmap->m_hObject == NULL)
		return pBitmap;

	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	CDC dcDest;
	dcDest.CreateCompatibleDC(&dcScreen);
	CBitmap* pOldBmp = dcDest.SelectObject(pBitmap);

	pSource->DrawDC(&dcDest, CPoint(0, 0));

	dcDest.SelectObject(pOldBmp);

	return pBitmap;
}

CDIB* CDIB::CreateDIB(int nWidth, int nHeight, int nBitCount)
{
	BITMAPINFOHEADER bmih;

	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = nWidth;
	bmih.biHeight = nHeight;
	bmih.biBitCount = nBitCount;
	bmih.biCompression = BI_RGB;
	bmih.biClrUsed = 0;

	return CreateDIB((BITMAPINFO*)&bmih);
}

CDIB* CDIB::ReduceColors()
{
	if (m_pBMI->bmiHeader.biBitCount > 8)
		return NULL;

	if (m_pBMI->bmiHeader.biClrUsed <= 2)
		return CDIB::CreateDIB(this, 1);

	if (m_pBMI->bmiHeader.biClrUsed <= 16)
		return CDIB::CreateDIB(this, 4);

	return NULL;
}

void CDIB::Draw(CDC* pDC, const CPoint& ptOffset)
{
	ASSERT(pDC != NULL);
	::StretchDIBits(pDC->m_hDC,
		ptOffset.x, ptOffset.y, m_pBMI->bmiHeader.biWidth, m_pBMI->bmiHeader.biHeight,
		0, 0, m_pBMI->bmiHeader.biWidth, m_pBMI->bmiHeader.biHeight,
		m_pBits, m_pBMI, DIB_RGB_COLORS, SRCCOPY);
}

void CDIB::Draw(CDC* pDC, const CPoint& ptOffset, const CSize& szScaled)
{
	ASSERT(pDC != NULL);
	pDC->SetStretchBltMode(COLORONCOLOR);
	::StretchDIBits(pDC->m_hDC,
		ptOffset.x, ptOffset.y, szScaled.cx, szScaled.cy,
		0, 0, m_pBMI->bmiHeader.biWidth, m_pBMI->bmiHeader.biHeight,
		m_pBits, m_pBMI, DIB_RGB_COLORS, SRCCOPY);
}

void CDIB::DrawDC(CDC* pDC, const CPoint& ptOffset)
{
	CDC dcSrc;
	dcSrc.CreateCompatibleDC(pDC);
	CBitmap* pOldBmpSrc = dcSrc.SelectObject(this);

	pDC->BitBlt(ptOffset.x, ptOffset.y,
		m_pBMI->bmiHeader.biWidth, m_pBMI->bmiHeader.biHeight,
        &dcSrc, 0, 0, SRCCOPY);

	dcSrc.SelectObject(pOldBmpSrc);
}

CRect FindContentRect(GP<DjVuImage> pImage)
{
	CRect rcResult(0, 0, 0, 0);
	if (pImage == NULL)
		return rcResult;

	CSize szImage = CSize(pImage->get_width(), pImage->get_height());

	rcResult = CRect(CPoint(0, 0), szImage);
	GRect rect(0, 0, szImage.cx, szImage.cy);

	BYTE nWhite = 0xff;
	int nPixelSize = 3;

	CDIB* pBmpPage = NULL;
	if (pImage->is_legal_photo() || pImage->is_legal_compound())
	{
		GP<GPixmap> pm = pImage->get_pixmap(rect, rect);
		pBmpPage = RenderPixmap(*pm);
		ASSERT(pBmpPage->GetBitsPerPixel() == 24);
	}
	else if (pImage->is_legal_bilevel())
	{
		GP<GBitmap> bm = pImage->get_bitmap(rect, rect, 4);
		pBmpPage = RenderBitmap(*bm);

		nPixelSize = 1;

		int nColorCount = pBmpPage->GetColorCount();
		RGBQUAD* pColors = pBmpPage->GetPalette();

		bool bFound = false;
		for (int i = 0; i < nColorCount; ++i)
		{
			if (pColors[i].rgbRed == 0xff &&
				pColors[i].rgbGreen == 0xff &&
				pColors[i].rgbBlue == 0xff)
			{
				nWhite = i;
				bFound = true;
				break;
			}
		}

		if (!bFound)
			return rcResult; // No white color in the palette
	}

	if (pBmpPage == NULL || pBmpPage->m_hObject == NULL)
		return rcResult;

	LPBYTE pBits = pBmpPage->GetBits();
	int nRowLength = szImage.cx*nPixelSize;
	while ((nRowLength % 4) != 0)
		++nRowLength;

	// Top margin
	for (; rcResult.top < rcResult.bottom; ++rcResult.top)
	{
		LPBYTE pPixel = pBits + rcResult.top*nRowLength;
		LPBYTE pEndPixel = pPixel + nPixelSize*szImage.cx;

		while (pPixel < pEndPixel && *pPixel == nWhite)
			++pPixel;

		if (pPixel < pEndPixel)
		{
			int nPixel = (pPixel - pBits - rcResult.top*nRowLength)/nPixelSize;
			rcResult.left = nPixel;
			rcResult.right = nPixel + 1;
			break;
		}
	}

	// Bottom margin
	for (; rcResult.bottom > rcResult.top; --rcResult.bottom)
	{
		LPBYTE pPixel = pBits + (rcResult.bottom - 1)*nRowLength;
		LPBYTE pEndPixel = pPixel + nPixelSize*szImage.cx;

		while (pPixel < pEndPixel && *pPixel == nWhite)
			++pPixel;

		if (pPixel < pEndPixel)
		{
			int nPixel = (pPixel - pBits - (rcResult.bottom - 1)*nRowLength)/nPixelSize;
			rcResult.left = min(rcResult.left, nPixel);
			rcResult.right = max(rcResult.right, nPixel + 1);
			break;
		}
	}

	// Left margin
	for (int y = 0; y < szImage.cy; ++y)
	{
		LPBYTE pPixel = pBits + y*nRowLength;
		LPBYTE pEndPixel = pPixel + nPixelSize*rcResult.left;

		while (pPixel < pEndPixel && *pPixel == nWhite)
			++pPixel;

		int nPixel = (pPixel - pBits - y*nRowLength)/nPixelSize;
		rcResult.left = min(rcResult.left, nPixel);
	}

	// Right margin
	for (int y = 0; y < szImage.cy; ++y)
	{
		LPBYTE pPixel = pBits + y*nRowLength + nPixelSize*szImage.cx - 1;
		LPBYTE pEndPixel = pBits + y*nRowLength + nPixelSize*rcResult.right;

		while (pPixel >= pEndPixel && *pPixel == nWhite)
			--pPixel;

		int nPixel = (pPixel - pBits - y*nRowLength)/nPixelSize;
		rcResult.right = max(rcResult.right, nPixel + 1);
	}

	delete pBmpPage;
	return rcResult;
}

void PrintPage(CDC* pDC, GP<DjVuImage> pImage, const CRect& rcFullPage,
	double fPrinterMMx, double fPrinterMMy, CPrintSettings& settings, bool bPreview)
{
	if (pImage == NULL)
		return;

	CRect rcPage = rcFullPage;

	// Subtract margins
	rcPage.DeflateRect(static_cast<int>(settings.fMarginLeft*fPrinterMMx),
					   static_cast<int>(settings.fMarginTop*fPrinterMMy),
					   static_cast<int>(settings.fMarginRight*fPrinterMMx),
					   static_cast<int>(settings.fMarginBottom*fPrinterMMy));

	CPoint ptSrcOffset(0, 0);
	CSize szDjVuPage = CSize(pImage->get_width(), pImage->get_height());

	if (settings.bClipContent)
	{
		CRect rcContent = FindContentRect(pImage);
		ptSrcOffset = rcContent.TopLeft();
		szDjVuPage = rcContent.Size();
	}

	double fSourceMM = pImage->get_dpi() / 25.4;

	CSize szScaled;
	CPoint ptOffset;

	if (!settings.bScaleToFit)
	{
		szScaled.cx = static_cast<int>(szDjVuPage.cx * fPrinterMMx * settings.fScale * 0.01 / fSourceMM);
		szScaled.cy = static_cast<int>(szDjVuPage.cy * fPrinterMMy * settings.fScale * 0.01 / fSourceMM);

		if (settings.bCenterImage)
		{
			ptOffset.x = (rcPage.Width() - szScaled.cx) / 2;
			ptOffset.y = (rcPage.Height() - szScaled.cy) / 2;
		}
		else
		{
			ptOffset.x = static_cast<int>(settings.fPosLeft * fPrinterMMx);
			ptOffset.y = static_cast<int>(settings.fPosTop * fPrinterMMy);
		}
	}

	if (settings.bScaleToFit || settings.bShrinkOversized && (szScaled.cx > rcPage.Width() || szScaled.cy > rcPage.Height()))
	{
		szScaled.cx = rcPage.Width();
		szScaled.cy = szScaled.cx * szDjVuPage.cy / szDjVuPage.cx;
		if (szScaled.cy > rcPage.Height())
		{
			szScaled.cy = rcPage.Height();
			szScaled.cx = szScaled.cy * szDjVuPage.cx / szDjVuPage.cy;
		}

		if (settings.bScaleToFit)
		{
			ptOffset.x = (rcPage.Width() - szScaled.cx) / 2;
			ptOffset.y = (rcPage.Height() - szScaled.cy) / 2;
		}
		else
		{
			if (ptOffset.x < 0)
				ptOffset.x = 0;
			else if (ptOffset.x + szScaled.cx > rcPage.Width())
				ptOffset.x = rcPage.Width() - szScaled.cx;

			if (ptOffset.y < 0)
				ptOffset.y = 0;
			else if (ptOffset.y + szScaled.cy > rcPage.Height())
				ptOffset.y = rcPage.Height() - szScaled.cy;
		}
	}

	CRect rcResult(ptOffset + rcPage.TopLeft(), szScaled);
	rcResult &= rcFullPage;
	if (rcResult.IsRectEmpty())
		return;

	GRect rect, rectAll;
	if (settings.bClipContent || !bPreview)
	{
		rect = GRect(ptSrcOffset.x, ptSrcOffset.y, szDjVuPage.cx, szDjVuPage.cy);
		rectAll = GRect(0, 0, pImage->get_width(), pImage->get_height());
	}
	else // if (bPreview)
	{
		rect = GRect(0, 0, szScaled.cx, szScaled.cy);
		rectAll = rect;
	}

	CDIB* pBmpPage = NULL;
	if (pImage->is_legal_photo() || pImage->is_legal_compound())
	{
		GP<GPixmap> pm = pImage->get_pixmap(rect, rectAll);

		if (settings.bClipContent && bPreview)
		{
			// Scale the pixmap to needed size
			GRect rcDest(0, 0, szScaled.cx, szScaled.cy);
			GRect rcSrc(0, 0, rect.width(), rect.height());

			GP<GPixmapScaler> ps = GPixmapScaler::create();
			ps->set_input_size(rect.width(), rect.height());
			ps->set_output_size(szScaled.cx, szScaled.cy);
			ps->set_horz_ratio(szScaled.cx, rect.width());
			ps->set_vert_ratio(szScaled.cy, rect.height());

			GP<GPixmap> pmStretched = GPixmap::create();
			ps->scale(rcSrc, *pm, rcDest, *pmStretched);
			pm = pmStretched;
		}

		pBmpPage = RenderPixmap(*pm);
	}
	else if (pImage->is_legal_bilevel())
	{
		GP<GBitmap> bm = pImage->get_bitmap(rect, rectAll, 4);

		if (settings.bClipContent && bPreview)
		{
			// Scale the bitmap to needed size
			GRect rcDest(0, 0, szScaled.cx, szScaled.cy);
			GRect rcSrc(0, 0, rect.width(), rect.height());

			GP<GBitmapScaler> bs = GBitmapScaler::create();
			bs->set_input_size(rect.width(), rect.height());
			bs->set_output_size(szScaled.cx, szScaled.cy);
			bs->set_horz_ratio(szScaled.cx, rect.width());
			bs->set_vert_ratio(szScaled.cy, rect.height());

			GP<GBitmap> bmStretched = GBitmap::create();
			bs->scale(rcSrc, *bm, rcDest, *bmStretched);
			bm = bmStretched;
		}

		pBmpPage = RenderBitmap(*bm);

		if (!bPreview)
		{
			CDIB* pBmpReduced = pBmpPage->ReduceColors();
			if (pBmpReduced != NULL)
			{
				delete pBmpPage;
				pBmpPage = pBmpReduced;
			}
		}
	}

	if (pBmpPage == NULL)
		return;

	int nDC = pDC->SaveDC();
	pDC->IntersectClipRect(rcFullPage);

	if (bPreview)
		pBmpPage->Draw(pDC, ptOffset + rcPage.TopLeft());
	else
		pBmpPage->Draw(pDC, ptOffset + rcPage.TopLeft(), szScaled);

	pDC->RestoreDC(nDC);

	delete pBmpPage;
}

DWORD WINAPI PrintThreadProc(LPVOID pvData)
{
	IProgressInfo* pProgress = reinterpret_cast<IProgressInfo*>(pvData);
	CPrintDlg& dlg = *reinterpret_cast<CPrintDlg*>(pProgress->GetUserData());

	GP<DjVuDocument> pDoc = dlg.GetDocument()->m_pDjVuDoc;

	int nPages = dlg.m_arrPages.size();
	pProgress->SetRange(0, nPages);
	pProgress->SetStatus(_T("Printing, please wait..."));

	// Printing
	CDC print_dc;
	print_dc.CreateDC(dlg.m_pPrinter->pDriverName, dlg.m_pPrinter->pPrinterName, dlg.m_pPrinter->pPortName, dlg.m_pPrinter->pDevMode);

	if (print_dc.m_hDC == NULL)
	{
		pProgress->StopProgress(1);
		return 1;
	}

	double fPrinterMMx = print_dc.GetDeviceCaps(LOGPIXELSX) / 25.4;
	double fPrinterMMy = print_dc.GetDeviceCaps(LOGPIXELSY) / 25.4;

	int nPageWidth = print_dc.GetDeviceCaps(PHYSICALWIDTH);
	int nPageHeight = print_dc.GetDeviceCaps(PHYSICALHEIGHT);
	CSize szPaper(nPageWidth, nPageHeight);

	int nOffsetX = print_dc.GetDeviceCaps(PHYSICALOFFSETX);
	int nOffsetY = print_dc.GetDeviceCaps(PHYSICALOFFSETY);
	print_dc.SetViewportOrg(-nOffsetX, -nOffsetY);

	CRect rcOddPage = CRect(CPoint(0, 0), szPaper), rcEvenPage = rcOddPage;
	if (dlg.m_bTwoPages)
	{
		if (dlg.m_bLandscape)
		{
			szPaper.cx = szPaper.cx / 2;
			rcOddPage.right = szPaper.cx;
			rcEvenPage.left = rcOddPage.right;
		}
		else
		{
			szPaper.cy = szPaper.cy / 2;
			rcOddPage.bottom = szPaper.cy;
			rcEvenPage.top = rcOddPage.bottom;
		}
	}

	int nRotate = dlg.GetRotate();

	DOCINFO di;
	di.cbSize = sizeof(DOCINFO);
	di.fwType = 0;
	di.lpszDocName = dlg.GetDocument()->GetTitle();
	di.lpszDatatype = NULL;
	di.lpszOutput = (dlg.m_bPrintToFile ? _T("C:\\Output.prn") : NULL);

	if (print_dc.StartDoc(&di) <= 0)
	{
		pProgress->StopProgress(2);
		return 2;
	}

	for (size_t i = 0; i < dlg.m_arrPages.size(); ++i)
	{
		CString strText;
		strText.Format(_T("Printing page %d of %d, please wait..."), i + 1, nPages);
		pProgress->SetStatus(strText);
		pProgress->SetPos(i);

		if (pProgress->IsCancelled())
			break;

		int nPage = dlg.m_arrPages[i].first - 1;
		int nSecondPage = dlg.m_arrPages[i].second - 1;

		if ((nPage < 0 || nPage >= pDoc->get_pages_num()) &&
			(!dlg.m_bTwoPages || nSecondPage < 0 || nSecondPage >= pDoc->get_pages_num()))
			continue;

		if (print_dc.StartPage() <= 0)
		{
			pProgress->StopProgress(3);
			return 3;
		}

		GP<DjVuImage> pImage;
		if (nPage >= 0 && nPage < pDoc->get_pages_num())
			pImage = pDoc->get_page(nPage);
		if (pImage != NULL)
		{
			pImage->set_rotate(nRotate);
			PrintPage(&print_dc, pImage, rcOddPage, fPrinterMMx, fPrinterMMy, dlg.m_settings);
		}

		pImage = NULL;
		if (dlg.m_bTwoPages && nSecondPage >= 0 && nSecondPage < pDoc->get_pages_num())
			pImage = pDoc->get_page(nSecondPage);
		if (pImage != NULL)
		{
			pImage->set_rotate(nRotate);
			PrintPage(&print_dc, pImage, rcEvenPage, fPrinterMMx, fPrinterMMy, dlg.m_settings);
		}

		if (print_dc.EndPage() <= 0)
		{
			pProgress->StopProgress(4);
			return 4;
		}
	}

	print_dc.EndDoc();

	pProgress->StopProgress(0);
	return 0;
}

/*
//////////////////////////////////////////////////////////////////////
// Fast 2 pass scale

struct ContributionType // Contirbution information for a single pixel
{
	ContributionType() : Weights(0) {}
	~ContributionType() { delete[] Weights; }

	int Left, Right;   // Bounds of source pixels window
	float* Weights;  // Normalized weights of neighboring pixels (changed from double* in Eran's code)
};

struct LineContribType // Contribution information for an entire line (row or column)
{
	LineContribType(int uLineLength, int uWindowSize)
		: WindowSize(uWindowSize), LineLength(uLineLength)
	{
		// Allocate contributions
		ContribRow = new ContributionType[uLineLength];
		for (int u = 0; u < uLineLength; u++)
			ContribRow[u].Weights = new float[uWindowSize];
	}
	~LineContribType() { delete[] ContribRow; }

	ContributionType* ContribRow; // Row (or column) of contribution weights
	int WindowSize,               // Filter window size (of affecting source pixels)
		LineLength;               // Length of line (no. or rows / cols)
};

#define _PI (3.1415926535897932384626433832795)
#define _2PI (2.0*_PI)

LineContribType* CalcContributions(int uLineSize, int uSrcSize, float dScale)
{ 
	float dWidth;
	float dFScale = 1.0;
	float dFilterWidth = 1.0; //CurFilter.GetWidth();

	if (dScale < 1.0) 
	{    // Minification
		dWidth = dFilterWidth / dScale; 
		dFScale = dScale; 
	} 
	else
	{    // Magnification
		dWidth = dFilterWidth; 
	}

	// Window size is the number of sampled pixels
	int iWindowSize = 2 * (int)ceil(dWidth) + 1; 

	// Allocate a new line contributions strucutre
	LineContribType *res = new LineContribType(uLineSize, iWindowSize); 

	for (int u = 0; u < uLineSize; u++) 
	{   // Scan through line of contributions
		float dCenter = (float)u / dScale;   // Reverse mapping
		// Find the significant edge points that affect the pixel
		int iLeft = max (0, (int)floor (dCenter - dWidth)); 

		int iRight = min ((int)ceil (dCenter + dWidth), int(uSrcSize) - 1); 

		// Cut edge points to fit in filter window in case of spill-off
		if (iRight - iLeft + 1 > iWindowSize) 
		{
			if (iLeft < (int(uSrcSize) - 1 / 2)) 
			{
				iLeft++; 
			}
			else 
			{
				iRight--; 
			}
		}
		res->ContribRow[u].Left = iLeft; 
		res->ContribRow[u].Right = iRight;
		float dTotalWeight = 0.0;  // Zero sum of weights
		float dVal;
		float filtered;
		int iSrc;

		for (iSrc = iLeft; iSrc <= iRight; iSrc++)
		{   // Calculate weights
			dVal = dFScale * (dCenter - (float)iSrc);
// Bilinear
//			dVal = float(fabs(dVal)); 
//			filtered = (float)(dVal < 1.0 ? 1.0 - dVal : 0.0);
// Gaussian
//			filtered = (float)(dVal <= 1.0 ? exp(-dVal*dVal/2.0)/sqrt(_2PI) : 0.0);
// Blackman
//			filtered = (float)(fabs(dVal) <= 1.0 ? 0.42 + 0.5*cos(_PI*dVal) + 0.08*cos(_2PI*dVal) : 0.0);
// Hamming
			filtered = (float)(fabs(dVal) <= 1.0 ? (0.54 + 0.46*cos(_2PI*dVal))*(dVal == 0 ? 1.0 : sin(_PI*dVal)/(_PI*dVal)) : 0.0);

			dTotalWeight += (res->ContribRow[u].Weights[iSrc-iLeft] =  
				dFScale * filtered); 
		}

		ASSERT (dTotalWeight >= 0.0);   // An error in the filter function can cause this 
		if (dTotalWeight > 0.0)
		{   // Normalize weight of neighbouring points
			for (iSrc = iLeft; iSrc <= iRight; iSrc++)
			{   // Normalize point
				res->ContribRow[u].Weights[iSrc-iLeft] /= dTotalWeight; 
			}
		}
	} 

	return res;
} 

void Fast2PassScale(void* pSrc, int uSrcWidth, int uSrcHeight,
					void* pDst, int uResWidth, int uResHeight)
{
	ASSERT(uSrcWidth > 0);
	ASSERT(uSrcHeight > 0);
	ASSERT(uResWidth > 0);
	ASSERT(uResHeight > 0);
	ASSERT(pSrc != NULL);
	ASSERT(pDst != NULL);

	// If neither X or Y scaling is required, just memcopy from source to destination.
	if (uSrcWidth == uResWidth && uSrcHeight == uResHeight)
	{
		memcpy (pDst, pSrc, sizeof(DWORD)*uSrcWidth*uSrcHeight);
		return;
	}

	LineContribType* YContrib = CalcContributions(uResHeight, uSrcHeight, float(uResHeight) / float(uSrcHeight));
	LineContribType* XContrib = CalcContributions(uResWidth, uSrcWidth, float(uResWidth) / float(uSrcWidth));

    DWORD *ContribPtrX, *ContribTempPtr, *ContribPtrY;        // Compliments YContrib and XContrib above.

	void* Dest = pDst;					                      // Keeps track of where we are to put the 
											                  // next pixels in the destination bitmap.

	float* YWeightPtr;                                        // A pointer to our Y "Wieght" factors.
	float* RGBArray = new float[uSrcWidth * 3];               // Our "temporary" array of pixels 
	float* RGBArrPtr;

	int SourceWidth = 4 * uSrcWidth;                          // This number tells us how many bytes there are 
	                                                          // between 2 vertically adjacent pixels.

	DWORD BVal, GVal, RVal;					                  // Temporary holders for our pixel components
	BVal = GVal = RVal = 0;


	DWORD YDelta, YCounter, XCounter, ColumnCounter; // Some counters

	_asm{
		
			// This is the "Super Optimized" version of the CFast2PassScale scaling function
		    // It "brings the two passes together" (so to speak) reducing memory usage and instructions
		    // Improving on the original C++ source, the temporary bitmap is eliminated (and replaced with 
		    // an array whose size is [12 * uSrcWidth] bytes).  The function will produce identical
		    // bitmaps to the original (corrected) C++ source.  If there are any differences they
		    // should be due to rounding errors (rounding should actually be BETTER with this function
		    // since the float precision decimal numbers are preserved between the Y and X passes.
			//
			//
			// The code achieves speed increases over the original Horizontal and Vertical scaling
		    // functions by calculating the vertical scaling for one horizontal "strip" of the source bitmap
		    // and placing the resulting pixels in an array of float precision R, G, and B values.
			// Then the array is scaled in the "horizontal" direction and the final pixels are placed 
		    // directly into the Destination bitmap.  IOW, each iteration of the VerticalLoop loop 
		    // will produce one line of scaled destination pixels.  This has the advantage of avoiding
		    // "cache thrashing" on most large bitmaps since the bitmap is handled in strips, and the
		    // memory overhead associated with creating and destroying large temporary arrays is eliminated.
		    // Also, several costly "fistp" instructions are avoided (since float RGB values are preserved
		    // between Y and X scaling (as well as not having to parse out RGB values from temporary source 
		    // pixels on the second pass).
		    // 
		    
		
			//  Below is a diagram of how the bitmap is processed. It is mathematically the same 
		    //  as the (corrected) C++ source code.
		
		    //  There is still calculating to be done in both the X and Y direction, but instead of
		    //  doing it all at once, we Vertically Scale each pixel for one line of the source bitmap
		    //  (into an array of float precision R,G,B values) and then do the "Horizontal Scaling"
		    //  on that array (just like the temporary bitmap in the original without the memory waste
		    //  and cache thrashing.  The algorithm is done in these two basic steps for each vertical
		    //  line in the bitmap.


		    //    Bitmap Processing 1 (ColumnLoop):  * = pixel.
		    //
			//    -----------------------
			//   |
			//   | * 1  <- Process Pixels in Y direction to YDelta to get the RBG
			//   | * 2             values of our "Vertically Scaled" pixel.
			//	 | * 3             
		    //   |                 Then repeat for each column in the X direction ->
		    //   |                 building an array of "vertically scaled" pixels 
		    //   |                 for one horizontal line of our temporary bitmap.     	
		

			//    Bitmap Processing 2 (RowLoop):  * = pixel.
			//
			//    -----------------------
			//   |
			//   | ***
			//   | 123 ->    Weight the array values (generated by Part One above) in the 
		    //   |           "X direction" and plot the line of destination pixels. 
		    //   |
		    //   |                 Repeat Parts 1 and 2 for each vertical line in the bitmap.   


			//
			//   Here is a diagram of the loops:
			//
			//
			//   VerticalLoop:          // This loop goes once for each line down destination bitmap.
			//      ColumnLoop: (1)     
			//         YWeightingLoop:  // Does the Y-Weighting for each pixel on the source bitmap
			//
			//      RowLoop:    (2)     
			//         XWeightingLoop:  // Does the X-Weighting for each pixel on the temporary array
			//
			//   * - So VerticalLoop runs once for each line down the bitmap.
			//


		    //  "And now here's something we hope you'll really like......"
	
			mov eax, YContrib;      
			mov eax, [eax];
			sub eax, 12;
			mov ContribPtrY, eax;      // The YContrib (and XContrib) pointers point to arrays
			mov YCounter, 0;           // of scaling weight and Delta information for a row or
									   // column of the source bitmap
	
			ALIGN 16;
			VerticalLoop:              // This loop goes once for each "Y-Pixel" of the
									   // *destination* bitmap.
				mov ebx, XContrib;
				mov ebx, [ebx];
				sub ebx, 12;
				mov ContribPtrX, ebx;  // ContribPtrX and Y point to our weighting structures.
				add ContribPtrY, 12;
				mov edi, ContribPtrY;
				mov ecx, [edi];
				mov esi, [edi + 4];
				
			
				sub esi, ecx;
				inc esi;
				mov YDelta, esi;

				mov eax, SourceWidth;
				imul eax, ecx;
				add eax, pSrc;
				sub eax, 4;
				mov esi, eax;		   // esi is our starting point on any given line
				                       // We use it to hold the place inside the next loop.
				
				mov edi, [edi + 8];
				mov YWeightPtr, edi;   // Points to the pixel weights we will use for Y-Scaling
		
				mov eax, uSrcWidth;
				mov ecx, [RGBArray];
				mov ColumnCounter, eax;
				mov RGBArrPtr, ecx;
				
				ALIGN 16;
				ColumnLoop:			   // This loop goes for each X pixel of the Source bitmap. 
			                           // We calculate the weights of pixels vertically from esi  
				                       // to (esi + YDelta). Then later we scale this line of 
				                       // pixels. Horizintally to get our Destination pixel.
					mov ecx, YDelta;
					mov edi, YWeightPtr;
					add esi, 4;
					mov edx, esi;      // edx points to our source bitmap pixels.
				
					fldz;              // Load up some 0's in our ploat stack for the loop below.
					fldz;
					fldz;									
				

					ALIGN 16;
					YWeightingLoop:    // This loops once for each Pixel in the Y-direction 
					                   // (to YDelta). Just load up each pixel from ecx to 0, 
					                   // and multiply it by it's weighting (pointed to by edi).
	
						fld dword ptr[edi];               // Load up our weighting factor.

						movzx eax, byte ptr [edx];        // Get our RGB value into RVal, BVal, GVal 
						movzx ebx, byte ptr [edx + 1];

						mov BVal, eax;

						movzx eax, byte ptr [edx + 2];
						
						mov GVal, ebx;
						mov RVal, eax;	
						
						fild BVal;
						fmul st(0), st(1);
						fxch;
						add edx, SourceWidth;
						fild GVal;
						fmul st(0), st(1);
						fxch;
						add edi, 4;                      // Go to next weighting factor
						fild RVal;
						fmulp st(1), st(0);
						fxch st(2);                      // Now st(0) thru st(2) are B, G, R....


						faddp st(3), st(0);	
						faddp st(3), st(0);		
						faddp st(3), st(0);	
					
						
						dec ecx;                         // Our counter							
					
					jnz short YWeightingLoop;

				mov ecx, [RGBArrPtr];
									
				fstp dword ptr [ecx];       // B   Move our YScaled pixels into our 
				fstp dword ptr [ecx + 4];   // G   temporary array.
				fstp dword ptr [ecx + 8];   // R
			
				add RGBArrPtr, 12;
				dec ColumnCounter;
		
			jnz short ColumnLoop;			// Process the pixels for the next column...

											// Now we scale our "temporary bitmap" array and 
			                                // plot a line of pixels in our Destination bitmap.
			mov eax, uResWidth;
			mov XCounter, eax;

			mov edx, [Dest];
			mov eax, ContribPtrX;
			mov ContribTempPtr, eax;

			ALIGN 16;
			RowLoop:							 // This loop calculates each destination
												 // X pixel for one vertical line.

				add ContribTempPtr, 12; 
				mov eax, ContribTempPtr;         // Our array of X-weighting factors
				 
				mov ebx, [RGBArray];
				mov edi, eax;
				mov ecx, [eax];
				mov esi, [edi + 4];	
					
				sub esi, ecx;
				mov edi, [edi + 8];
				lea eax, [ecx * 8 + ebx];
				lea ebx, [eax + ecx * 4];
				inc esi;
				mov eax, 4;
		
				fldz;
				fldz;
				fldz;

				ALIGN 16;
				XWeightingLoop:
				
					fld dword ptr[edi];         // Load up weighting factor...
			
					fld dword ptr [ebx];
					fmul st(0), st(1);          // B
					fxch;
					add edi, eax;

					fld dword ptr [ebx + eax];
					fmul st(0), st(1);          // G
					add ebx, 12;
					fxch;

					fld dword ptr [ebx + 2 * eax - 12];
					fmulp st(1), st(0);         // R 
					fxch st(2);

					dec esi;

					faddp st(3), st(0);
					faddp st(3), st(0);		
					faddp st(3), st(0);	
									
				jnz short XWeightingLoop;  // Repeat until all pixels have been wieghted

										   //  Unload pixels into RGB ints and then make them
										   //  into the final RGB quad.

				fistp BVal;				   // Ouch!!!
				fistp GVal;
				fistp RVal;
							
				mov ebx, RVal;		       // We will make our pixel in the ebx register and
				rol ebx, 8;			       // then move it into the address pointed to by 
				or ebx, GVal;              // edx register.
					                 
				rol ebx,8;
				or ebx, BVal;
			
				mov dword ptr [edx], ebx;  // Place the value in Dest
										
				lea edx, [edx + 4];
				dec XCounter;


			jnz short RowLoop;		       // Continue across the array until you have 
								           // processed all horizintal pixels in this "line"
			mov Dest, edx;
			inc YCounter;
			mov eax, YCounter;
			cmp eax, uResHeight;

			
		jb VerticalLoop;			       // Jump down one line and repeat.....

	}

	delete[] RGBArray;

	delete YContrib;
	delete XContrib;
}
*/