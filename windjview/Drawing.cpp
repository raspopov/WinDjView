//	WinDjView
//	Copyright (C) 2004-2006 Andrew Zhezherun
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
//	http://www.gnu.org/copyleft/gpl.html

// $Id$

#include "stdafx.h"
#include "WinDjView.h"
#include "Drawing.h"

#include "PrintDlg.h"
#include "ProgressDlg.h"
#include "DjVuDoc.h"
#include "DjVuView.h"
#include "AppSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


void BuildBrightnessTable(int nBrightness, BYTE* table)
{
	for (int i = 0; i < 256; ++i)
	{
		double color = table[i] * (100.0 + nBrightness) / 100.0;
		color = max(0.0, min(color, 255.0));
		table[i] = static_cast<BYTE>(floor(color + 0.5));
	}
}

void BuildContrastTable(int nContrast, BYTE* table)
{
	for (int i = 0; i < 256; ++i)
	{
		double color = 128.0 + (table[i] - 128.0) * (100 + nContrast) / 100.0;
		color = max(0.0, min(color, 255.0));
		table[i] = static_cast<BYTE>(floor(color + 0.5));
	}
}

void BuildGammaTable(double fGamma, BYTE* table)
{
	double exponent = 1 / fGamma;
	for (int i = 0; i < 256; ++i)
	{
		double color = pow(table[i] / 255.0, exponent) * 255.0;
		color = max(0.0, min(color, 255.0));
		table[i] = static_cast<BYTE>(floor(color + 0.5));
	}
}

CDIB* RenderPixmap(GPixmap& pm, const CDisplaySettings& displaySettings)
{
	return RenderPixmap(pm, CRect(0, 0, pm.columns(), pm.rows()), displaySettings);
}

CDIB* RenderPixmap(GPixmap& pm, const CRect& rcClip, const CDisplaySettings& displaySettings)
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

	int nRowLength = rcClip.Width()*3;
	while (nRowLength % 4 != 0)
		++nRowLength;

	LPBYTE pBits = pBitmap->GetBits();
	for (int y = rcClip.top; y < rcClip.bottom; ++y, pBits += nRowLength)
		memcpy(pBits, pm[y] + rcClip.left, rcClip.Width()*3);

	int nBrightness = displaySettings.GetBrightness();
	int nContrast = displaySettings.GetContrast();
	double fGamma = displaySettings.GetGamma();
	if (fGamma != 1.0 || nBrightness != 0 || nContrast != 0)
	{
		// Adjust gamma
		BYTE table[256];
		for (int i = 0; i < 256; ++i)
			table[i] = i;

		if (nBrightness != 0)
			BuildBrightnessTable(nBrightness, table);
		if (nContrast != 0)
			BuildContrastTable(nContrast, table);
		if (fGamma != 1.0)
			BuildGammaTable(fGamma, table);

		pBits = pBitmap->GetBits();
		for (int y = rcClip.top; y < rcClip.bottom; ++y, pBits += nRowLength)
		{
			LPBYTE pCurBit = pBits;
			LPBYTE pEndBit = pCurBit + rcClip.Width()*3;
			while (pCurBit != pEndBit)
			{
				*pCurBit = table[*pCurBit];
				++pCurBit;
			}
		}
	}

	return pBitmap;
}

CDIB* RenderBitmap(GBitmap& bm, const CDisplaySettings& displaySettings)
{
	return RenderBitmap(bm, CRect(0, 0, bm.columns(), bm.rows()), displaySettings);
}

CDIB* RenderBitmap(GBitmap& bm, const CRect& rcClip, const CDisplaySettings& displaySettings)
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

	int nBrightness = displaySettings.GetBrightness();
	int nContrast = displaySettings.GetContrast();
	double fGamma = displaySettings.GetGamma();
	if (fGamma != 1.0 || nBrightness != 0 || nContrast != 0)
	{
		// Adjust gamma
		BYTE table[256];
		for (int i = 0; i < 256; ++i)
			table[i] = i;

		if (nBrightness != 0)
			BuildBrightnessTable(nBrightness, table);
		if (nContrast != 0)
			BuildContrastTable(nContrast, table);
		if (fGamma != 1.0)
			BuildGammaTable(fGamma, table);

		for (int j = 0; j < nPaletteEntries; ++j)
		{
			pBMI->bmiColors[j].rgbBlue = table[pBMI->bmiColors[j].rgbBlue];
			pBMI->bmiColors[j].rgbGreen = table[pBMI->bmiColors[j].rgbGreen];
			pBMI->bmiColors[j].rgbRed = table[pBMI->bmiColors[j].rgbRed];
		}
	}

	CDIB* pBitmap = CDIB::CreateDIB(pBMI);
	free(pBMI);

	if (pBitmap->m_hObject == NULL)
		return pBitmap;

	int nRowLength = rcClip.Width();
	while (nRowLength % 4 != 0)
		++nRowLength;

	LPBYTE pBits = pBitmap->GetBits();
	for (int y = rcClip.top; y < rcClip.bottom; ++y, pBits += nRowLength)
		memcpy(pBits, bm[y] + rcClip.left, rcClip.Width());

	return pBitmap;
}

CDIB* RenderEmpty(const CSize& szBitmap, const CDisplaySettings& displaySettings)
{
	LPBITMAPINFO pBMI = (LPBITMAPINFO)malloc(
		sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD));
	BITMAPINFOHEADER& bmih = pBMI->bmiHeader;

	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = szBitmap.cx;
	bmih.biHeight = szBitmap.cy;
	bmih.biBitCount = 1;
	bmih.biCompression = BI_RGB;
	bmih.biClrUsed = 1;

	// Create palette for the bitmap
	pBMI->bmiColors[0].rgbBlue = 0xff;
	pBMI->bmiColors[0].rgbGreen = 0xff;
	pBMI->bmiColors[0].rgbRed = 0xff;

	int nBrightness = displaySettings.GetBrightness();
	int nContrast = displaySettings.GetContrast();
	double fGamma = displaySettings.GetGamma();
	if (fGamma != 1.0 || nBrightness != 0 || nContrast != 0)
	{
		// Adjust gamma
		BYTE table[256];
		for (int i = 0; i < 256; ++i)
			table[i] = i;

		if (nBrightness != 0)
			BuildBrightnessTable(nBrightness, table);
		if (nContrast != 0)
			BuildContrastTable(nContrast, table);
		if (fGamma != 1.0)
			BuildGammaTable(fGamma, table);

		pBMI->bmiColors[0].rgbBlue = table[pBMI->bmiColors[0].rgbBlue];
		pBMI->bmiColors[0].rgbGreen = table[pBMI->bmiColors[0].rgbGreen];
		pBMI->bmiColors[0].rgbRed = table[pBMI->bmiColors[0].rgbRed];
	}

	CDIB* pBitmap = CDIB::CreateDIB(pBMI);
	free(pBMI);

	if (pBitmap->m_hObject == NULL)
		return pBitmap;

	int nRowLength = (szBitmap.cx - 1) / 8 + 1;
	while (nRowLength % 4 != 0)
		++nRowLength;

	LPBYTE pBits = pBitmap->GetBits();
	memset(pBits, 0, nRowLength*szBitmap.cy);

	return pBitmap;
}

CDIB::~CDIB()
{
	DeleteObject();
	free(m_pBMI);

	if (m_hSection != NULL)
		CloseHandle(m_hSection);
	if (m_hFile != NULL)
		CloseHandle(m_hFile);
}

void CDIB::Create(const BITMAPINFO* pBMI)
{
	ASSERT(m_hObject == NULL);

	UINT nSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*pBMI->bmiHeader.biClrUsed;
	m_pBMI = (LPBITMAPINFO)malloc(nSize);
	memcpy(m_pBMI, pBMI, nSize);

	BITMAPINFOHEADER& bmih = m_pBMI->bmiHeader;
	bmih.biPlanes = 1;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrImportant = 0;

	HBITMAP hBitmap = ::CreateDIBSection(NULL,
		m_pBMI, DIB_RGB_COLORS, (VOID**)&m_pBits, NULL, 0);

	if (hBitmap == NULL)
	{
		CString strTempPath;
		if (::GetTempPath(_MAX_PATH, strTempPath.GetBuffer(_MAX_PATH)) != 0)
			strTempPath.ReleaseBuffer();
		else
			strTempPath = _T(".");

		CString strTempFile;
		if (::GetTempFileName(strTempPath, _T("DJV"), 0, strTempFile.GetBuffer(_MAX_PATH)) != 0)
		{
			DWORD nLineLength = static_cast<DWORD>(ceil(1.0*pBMI->bmiHeader.biWidth*pBMI->bmiHeader.biBitCount/8.0));
			while ((nLineLength % 4) != 0)
				++nLineLength;

			DWORD nBitsSize = nLineLength*pBMI->bmiHeader.biHeight;

			// Try to create a mapped file section
			m_hFile = ::CreateFile(strTempFile, GENERIC_READ | GENERIC_WRITE,
				0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
			m_hSection = ::CreateFileMapping(m_hFile, NULL,
				PAGE_READWRITE, 0, nBitsSize, NULL);

			hBitmap = ::CreateDIBSection(NULL, m_pBMI, DIB_RGB_COLORS,
				(VOID**)&m_pBits, m_hSection, 0);
			if (hBitmap == NULL)
			{
				::CloseHandle(m_hSection);
				::CloseHandle(m_hFile);
				m_hSection = NULL;
				m_hFile = NULL;
			}
		}
	}

	Attach(hBitmap);
}

CDIB* CDIB::CreateDIB(const BITMAPINFO* pBMI)
{
	CDIB* pDIB = new CDIB();
	pDIB->Create(pBMI);
	return pDIB;
}

void CDIB::Create(CDIB* pSource, int nBitCount)
{
	ASSERT(pSource->m_pBits != NULL);
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

	Create((BITMAPINFO*)&bmih);
	if (m_hObject == NULL)
		return;

	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	CDC dcDest;
	dcDest.CreateCompatibleDC(&dcScreen);
	CBitmap* pOldBmp = dcDest.SelectObject(this);

	pSource->DrawDC(&dcDest, CPoint(0, 0));

	dcDest.SelectObject(pOldBmp);
}

CDIB* CDIB::CreateDIB(CDIB* pSource, int nBitCount)
{
	CDIB* pDIB = new CDIB();
	pDIB->Create(pSource, nBitCount);
	return pDIB;
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
	ASSERT(pDC != NULL && m_pBits != NULL);
	::StretchDIBits(pDC->m_hDC,
		ptOffset.x, ptOffset.y, m_pBMI->bmiHeader.biWidth, m_pBMI->bmiHeader.biHeight,
		0, 0, m_pBMI->bmiHeader.biWidth, m_pBMI->bmiHeader.biHeight,
		m_pBits, m_pBMI, DIB_RGB_COLORS, SRCCOPY);
}

void CDIB::Draw(CDC* pDC, const CPoint& ptOffset, const CSize& szScaled)
{
	ASSERT(pDC != NULL && m_pBits != NULL);
	pDC->SetStretchBltMode(COLORONCOLOR);
	::StretchDIBits(pDC->m_hDC,
		ptOffset.x, ptOffset.y, szScaled.cx, szScaled.cy,
		0, 0, m_pBMI->bmiHeader.biWidth, m_pBMI->bmiHeader.biHeight,
		m_pBits, m_pBMI, DIB_RGB_COLORS, SRCCOPY);
}

void CDIB::DrawDC(CDC* pDC, const CPoint& ptOffset)
{
	ASSERT(pDC != NULL && m_hObject != NULL);

	CDC dcSrc;
	dcSrc.CreateCompatibleDC(pDC);
	CBitmap* pOldBmpSrc = dcSrc.SelectObject(this);

	pDC->BitBlt(ptOffset.x, ptOffset.y,
		m_pBMI->bmiHeader.biWidth, m_pBMI->bmiHeader.biHeight,
        &dcSrc, 0, 0, SRCCOPY);

	dcSrc.SelectObject(pOldBmpSrc);
}

void CDIB::DrawDC(CDC* pDC, const CPoint& ptOffset, const CRect& rcPart)
{
	ASSERT(pDC != NULL && m_hObject != NULL);
	ASSERT(rcPart.left >= 0 && rcPart.top >= 0 &&
		   rcPart.right <= m_pBMI->bmiHeader.biWidth &&
		   rcPart.bottom <= m_pBMI->bmiHeader.biHeight);

	CDC dcSrc;
	dcSrc.CreateCompatibleDC(pDC);
	CBitmap* pOldBmpSrc = dcSrc.SelectObject(this);

	pDC->BitBlt(ptOffset.x, ptOffset.y, rcPart.Width(), rcPart.Height(),
		&dcSrc, rcPart.left, rcPart.top, SRCCOPY);

	dcSrc.SelectObject(pOldBmpSrc);
}

void CDIB::Save(LPCTSTR pszPathName) const
{
	CFile file;
	if (!file.Open(pszPathName, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive))
		return;

	BITMAPFILEHEADER hdr;
	hdr.bfType = 'MB';
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;

	DWORD dwHeaderSize = sizeof(BITMAPINFOHEADER) +
		sizeof(RGBQUAD)*m_pBMI->bmiHeader.biClrUsed;
	DWORD dwBitCount = m_pBMI->bmiHeader.biWidth * m_pBMI->bmiHeader.biBitCount;
	dwBitCount = (((dwBitCount + 31) / 32) * 4) * m_pBMI->bmiHeader.biHeight;

	hdr.bfOffBits = sizeof(BITMAPFILEHEADER) + dwHeaderSize;
	hdr.bfSize = hdr.bfOffBits + dwBitCount;

	file.Write(&hdr, sizeof(hdr));
	file.Write(m_pBMI, dwHeaderSize);
	file.Write(m_pBits, dwBitCount);

	file.Close();
}

CLightweightDIB* CLightweightDIB::Create(CDIB* pSrc)
{
	CLightweightDIB* pDIB = new CLightweightDIB();

	ASSERT(pSrc != NULL);
	pDIB->CDIB::Create(pSrc, 24);

	if (pDIB->m_hObject == NULL)
	{
		pDIB->m_pBits = NULL;
		return pDIB;
	}

	DWORD dwByteCount = pDIB->m_pBMI->bmiHeader.biWidth * pDIB->m_pBMI->bmiHeader.biBitCount;
	dwByteCount = (((dwByteCount + 31) / 32) * 4) * pDIB->m_pBMI->bmiHeader.biHeight;

	LPBYTE pBits = new BYTE[dwByteCount];
	memcpy(pBits, pDIB->m_pBits, dwByteCount);

	pDIB->DeleteObject();
	pDIB->m_pBits = pBits;

	return pDIB;
}

CLightweightDIB::~CLightweightDIB()
{
	delete[] m_pBits;
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
		pBmpPage = RenderPixmap(*pm, CDisplaySettings());
		ASSERT(pBmpPage->GetBitsPerPixel() == 24);
	}
	else if (pImage->is_legal_bilevel())
	{
		GP<GBitmap> bm = pImage->get_bitmap(rect, rect, 4);
		pBmpPage = RenderBitmap(*bm, CDisplaySettings());

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
	int y;
	for (y = 0; y < szImage.cy; ++y)
	{
		LPBYTE pPixel = pBits + y*nRowLength;
		LPBYTE pEndPixel = pPixel + nPixelSize*rcResult.left;

		while (pPixel < pEndPixel && *pPixel == nWhite)
			++pPixel;

		int nPixel = (pPixel - pBits - y*nRowLength)/nPixelSize;
		rcResult.left = min(rcResult.left, nPixel);
	}

	// Right margin
	for (y = 0; y < szImage.cy; ++y)
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

void PrintPage(CDC* pDC, GP<DjVuImage> pImage, int nMode, const CRect& rcFullPage,
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
	if (szDjVuPage.cx <= 0 || szDjVuPage.cy <= 0)
		return;

	if (settings.bClipContent)
	{
		CRect rcContent = FindContentRect(pImage);
		ptSrcOffset = rcContent.TopLeft();
		szDjVuPage = rcContent.Size();
	}

	double fSourceMM = pImage->get_dpi() / 25.4;

	CSize szScaled;
	CPoint ptOffset;

	if (!settings.bCenterImage)
	{
		ptOffset.x = static_cast<int>(settings.fPosLeft * fPrinterMMx);
		ptOffset.y = static_cast<int>(settings.fPosTop * fPrinterMMy);
	}

	if (!settings.bScaleToFit)
	{
		szScaled.cx = static_cast<int>(szDjVuPage.cx * fPrinterMMx * settings.fScale * 0.01 / fSourceMM);
		szScaled.cy = static_cast<int>(szDjVuPage.cy * fPrinterMMy * settings.fScale * 0.01 / fSourceMM);

		if (settings.bCenterImage)
		{
			ptOffset.x = (rcPage.Width() - szScaled.cx) / 2;
			ptOffset.y = (rcPage.Height() - szScaled.cy) / 2;
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

		if (settings.bCenterImage)
		{
			ptOffset.x = (rcPage.Width() - szScaled.cx) / 2;
			ptOffset.y = (rcPage.Height() - szScaled.cy) / 2;
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
	GP<GPixmap> pm;
	GP<GBitmap> bm;

	try
	{
		switch (nMode)
		{
		case CDjVuView::BlackAndWhite:
			bm = pImage->get_bitmap(rect, rectAll, 4);
			break;

		case CDjVuView::Foreground:
			pm = pImage->get_fg_pixmap(rect, rectAll);
			if (pm == NULL)
				bm = pImage->get_bitmap(rect, rectAll, 4);
			break;

		case CDjVuView::Background:
			pm = pImage->get_bg_pixmap(rect, rectAll);
			break;

		case CDjVuView::Color:
		default:
			pm = pImage->get_pixmap(rect, rectAll);
			if (pm == NULL)
				bm = pImage->get_bitmap(rect, rectAll, 4);
		}
	}
	catch (GException&)
	{
	}
	catch (...)
	{
		ReportFatalError();
	}

	if (pm != NULL)
	{
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

		pBmpPage = RenderPixmap(*pm, CAppSettings::displaySettings);
	}
	else if (bm != NULL)
	{
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

		pBmpPage = RenderBitmap(*bm, CAppSettings::displaySettings);

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

unsigned int __stdcall PrintThreadProc(void* pvData)
{
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	IProgressInfo* pProgress = reinterpret_cast<IProgressInfo*>(pvData);
	CPrintDlg& dlg = *reinterpret_cast<CPrintDlg*>(pProgress->GetUserData());

	CDjVuDoc* pDoc = dlg.GetDocument();

	size_t nPages = dlg.m_arrPages.size();
	if (dlg.m_nCopies > 1 && dlg.m_bCollate && !dlg.m_bPrinterCanCollate)
	{
		// We will do collation ourselves
		nPages *= dlg.m_nCopies;
		dlg.m_pPrinter->pDevMode->dmCopies = 1;
	}

	pProgress->SetRange(0, nPages);
	pProgress->SetStatus(LoadString(IDS_PRINTING));

	// Printing
	CDC print_dc;
	print_dc.CreateDC(dlg.m_pPrinter->pDriverName, dlg.m_pPrinter->pPrinterName, NULL, dlg.m_pPrinter->pDevMode);

	if (print_dc.m_hDC == NULL)
	{
		pProgress->StopProgress(1);
		::CoUninitialize();
		return 1;
	}

	double fPrinterMMx = print_dc.GetDeviceCaps(LOGPIXELSX) / 25.4;
	double fPrinterMMy = print_dc.GetDeviceCaps(LOGPIXELSY) / 25.4;

	int nPageWidth, nPageHeight;
	if (dlg.m_settings.bIgnorePrinterMargins)
	{
		nPageWidth = print_dc.GetDeviceCaps(PHYSICALWIDTH);
		nPageHeight = print_dc.GetDeviceCaps(PHYSICALHEIGHT);

		int nOffsetX = print_dc.GetDeviceCaps(PHYSICALOFFSETX);
		int nOffsetY = print_dc.GetDeviceCaps(PHYSICALOFFSETY);
		print_dc.SetViewportOrg(-nOffsetX, -nOffsetY);
	}
	else
	{
		nPageWidth = print_dc.GetDeviceCaps(HORZRES);
		nPageHeight = print_dc.GetDeviceCaps(VERTRES);
	}

	CSize szPaper(nPageWidth, nPageHeight);

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
	int nMode = dlg.GetMode();

	DOCINFO di;
	di.cbSize = sizeof(DOCINFO);
	di.fwType = 0;
	di.lpszDocName = dlg.GetDocument()->GetTitle();
	di.lpszDatatype = NULL;
	di.lpszOutput = (dlg.m_bPrintToFile ? _T("C:\\Output.prn") : NULL);

	if (print_dc.StartDoc(&di) <= 0)
	{
		pProgress->StopProgress(2);
		::CoUninitialize();
		return 2;
	}

	for (size_t i = 0; i < nPages; ++i)
	{
		CString strText;
		strText.Format(IDS_PRINTING_PAGE, i + 1, nPages);
		pProgress->SetStatus(strText);
		pProgress->SetPos(i);

		if (pProgress->IsCancelled())
		{
			print_dc.AbortDoc();

			pProgress->StopProgress(0);
			::CoUninitialize();
			return 0;
		}

		int nPage = dlg.m_arrPages[i % dlg.m_pages.size()].first - 1;
		int nSecondPage = dlg.m_arrPages[i % dlg.m_pages.size()].second - 1;

		if ((nPage < 0 || nPage >= pDoc->GetPageCount()) &&
			(!dlg.m_bTwoPages || nSecondPage < 0 || nSecondPage >= pDoc->GetPageCount()))
			continue;

		if (print_dc.StartPage() <= 0)
		{
			pProgress->StopProgress(3);
			::CoUninitialize();
			return 3;
		}

		GP<DjVuImage> pImage;
		if (nPage >= 0 && nPage < pDoc->GetPageCount())
			pImage = pDoc->GetPage(nPage, false);
		if (pImage != NULL)
		{
			RotateImage(pImage, nRotate);
			PrintPage(&print_dc, pImage, nMode, rcOddPage, fPrinterMMx, fPrinterMMy, dlg.m_settings);
		}

		pImage = NULL;
		if (dlg.m_bTwoPages && nSecondPage >= 0 && nSecondPage < pDoc->GetPageCount())
			pImage = pDoc->GetPage(nSecondPage, false);
		if (pImage != NULL)
		{
			RotateImage(pImage, nRotate);
			PrintPage(&print_dc, pImage, nMode, rcEvenPage, fPrinterMMx, fPrinterMMy, dlg.m_settings);
		}

		if (print_dc.EndPage() <= 0)
		{
			pProgress->StopProgress(4);
			::CoUninitialize();
			return 4;
		}
	}

	print_dc.EndDoc();

	pProgress->StopProgress(0);

	::CoUninitialize();
	return 0;
}

void FrameRect(CDC* pDC, const CRect& rect, COLORREF color)
{
	CRect rcHorzLine(rect.TopLeft(), CSize(rect.Width(), 1));
	pDC->FillSolidRect(rcHorzLine, color);
	rcHorzLine.OffsetRect(0, rect.Height() - 1);
	pDC->FillSolidRect(rcHorzLine, color);

	CRect rcVertLine(rect.TopLeft(), CSize(1, rect.Height()));
	pDC->FillSolidRect(rcVertLine, color);
	rcVertLine.OffsetRect(rect.Width() - 1, 0);
	pDC->FillSolidRect(rcVertLine, color);
}

void DrawDottedLine(CDC* pDC, COLORREF color, CPoint ptStart, CPoint ptEnd)
{
	if (ptStart.x == ptEnd.x)
	{
		ASSERT(ptStart.y <= ptEnd.y);
		for (int y = ptStart.y; y < ptEnd.y; y += 2)
			pDC->SetPixel(ptStart.x, y, color);
	}
	else if (ptStart.y == ptEnd.y)
	{
		ASSERT(ptStart.x <= ptEnd.x);
		for (int x = ptStart.x; x < ptEnd.x; x += 2)
			pDC->SetPixel(x, ptStart.y, color);
	}
	else
	{
		ASSERT(false);
	}
}
