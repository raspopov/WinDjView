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

#include "stdafx.h"
#include "WinDjView.h"
#include "Drawing.h"

#include "PrintDlg.h"
#include "ProgressDlg.h"
#include "DjVuDoc.h"
#include "DjVuView.h"
#include "MyGdiPlus.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


void BuildInvertTable(BYTE* table)
{
	for (int i = 0; i < 256; ++i)
		table[i] = 255 - table[i];
}

void BuildBrightnessTable(int nBrightness, BYTE* table)
{
	for (int i = 0; i < 256; ++i)
	{
		double color = table[i] * (100.0 + nBrightness) / 100.0;
		table[i] = max(0, min(255, static_cast<int>(color + 0.5)));
	}
}

void BuildContrastTable(int nContrast, BYTE* table)
{
	for (int i = 0; i < 256; ++i)
	{
		double color = 128.0 + (table[i] - 128.0) * (100 + nContrast) / 100.0;
		table[i] = max(0, min(255, static_cast<int>(color + 0.5)));
	}
}

void BuildGammaTable(double fGamma, BYTE* table)
{
	double exponent = 1 / fGamma;
	for (int i = 0; i < 256; ++i)
	{
		double color = pow(table[i] / 255.0, exponent) * 255.0;
		table[i] = max(0, min(255, static_cast<int>(color + 0.5)));
	}
}

void BuildTransparentcyTable(double fTransparency, BYTE c, BYTE* table)
{
	DWORD nAlpha = max(0, min(255, static_cast<DWORD>(fTransparency * 255.0 + 0.5)));
	for (int i = 0; i < 256; ++i)
	{
		BYTE color = static_cast<BYTE>((table[i] * nAlpha + c * (255 - nAlpha)) >> 8);
		table[i] = max(0, min(255, color));
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
	ZeroMemory(&bmih, sizeof(bmih));

	bmih.biSize = sizeof(bmih);
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
	if (fGamma != 1.0 || nBrightness != 0 || nContrast != 0 || displaySettings.bInvertColors)
	{
		// Adjust gamma
		BYTE table[256];
		for (int i = 0; i < 256; ++i)
			table[i] = i;

		if (displaySettings.bInvertColors)
			BuildInvertTable(table);
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
	int nSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*nPaletteEntries;

	pBMI = (LPBITMAPINFO) malloc(nSize);
	ZeroMemory(pBMI, nSize);

	BITMAPINFOHEADER& bmih = pBMI->bmiHeader;
	bmih.biSize = sizeof(bmih);
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
	if (fGamma != 1.0 || nBrightness != 0 || nContrast != 0 || displaySettings.bInvertColors)
	{
		// Adjust gamma
		BYTE table[256];
		for (int i = 0; i < 256; ++i)
			table[i] = i;

		if (displaySettings.bInvertColors)
			BuildInvertTable(table);
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
	int nSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD);
	LPBITMAPINFO pBMI = (LPBITMAPINFO) malloc(nSize);
	ZeroMemory(pBMI, nSize);

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
	if (fGamma != 1.0 || nBrightness != 0 || nContrast != 0 || displaySettings.bInvertColors)
	{
		// Adjust gamma
		BYTE table[256];
		for (int i = 0; i < 256; ++i)
			table[i] = i;

		if (displaySettings.bInvertColors)
			BuildInvertTable(table);
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
	ZeroMemory(pBits, nRowLength*szBitmap.cy);

	return pBitmap;
}


// CDIB

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
			if (m_hFile != NULL && m_hFile != INVALID_HANDLE_VALUE)
			{
				m_hSection = ::CreateFileMapping(m_hFile, NULL,
					PAGE_READWRITE, 0, nBitsSize, NULL);
				if (m_hSection != NULL)
				{
					hBitmap = ::CreateDIBSection(NULL, m_pBMI, DIB_RGB_COLORS,
							(VOID**)&m_pBits, m_hSection, 0);
				}
			}

			if (hBitmap == NULL)
			{
				if (m_hSection != NULL)
					::CloseHandle(m_hSection);
				if (m_hFile != NULL && m_hFile != INVALID_HANDLE_VALUE)
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

	int nSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*nPaletteEntries;
	LPBITMAPINFO pBMI = (LPBITMAPINFO) malloc(nSize);
	ZeroMemory(pBMI, nSize);

	BITMAPINFOHEADER& bmih = pBMI->bmiHeader;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = pSource->m_pBMI->bmiHeader.biWidth;
	bmih.biHeight = pSource->m_pBMI->bmiHeader.biHeight;
	bmih.biBitCount = nBitCount;
	bmih.biCompression = BI_RGB;
	bmih.biClrUsed = nPaletteEntries;
	bmih.biXPelsPerMeter = pSource->m_pBMI->bmiHeader.biXPelsPerMeter;
	bmih.biYPelsPerMeter = pSource->m_pBMI->bmiHeader.biYPelsPerMeter;

	memcpy(&pBMI->bmiColors[0], &pSource->m_pBMI->bmiColors[0], nPaletteEntries*sizeof(RGBQUAD));

	Create(pBMI);
	free(pBMI);

	if (m_hObject == NULL)
		return;

	CScreenDC dcScreen;

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
	ZeroMemory(&bmih, sizeof(bmih));

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

CDIB* CDIB::Crop(const CRect& rcCrop)
{
	ASSERT(m_pBits != NULL);

	UINT nSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*m_pBMI->bmiHeader.biClrUsed;
	LPBITMAPINFO pBMI = (LPBITMAPINFO) malloc(nSize);
	memcpy(pBMI, m_pBMI, nSize);

	BITMAPINFOHEADER& bmih = pBMI->bmiHeader;
	bmih.biWidth = rcCrop.Width();
	bmih.biHeight = rcCrop.Height();

	CDIB* pDIB = CreateDIB(pBMI);
	free(pBMI);

	if (pDIB->m_hObject == NULL)
		return pDIB;

	CScreenDC dcScreen;

	CDC dcDest;
	dcDest.CreateCompatibleDC(&dcScreen);
	CBitmap* pOldBmp = dcDest.SelectObject(pDIB);

	DrawDC(&dcDest, CPoint(0, 0), rcCrop);

	dcDest.SelectObject(pOldBmp);
	return pDIB;
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
	if (szScaled.cx == m_pBMI->bmiHeader.biWidth && szScaled.cy == m_pBMI->bmiHeader.biHeight)
	{
		Draw(pDC, ptOffset);
	}
	else
	{
		ASSERT(pDC != NULL && m_pBits != NULL);
		pDC->SetStretchBltMode(COLORONCOLOR);
		::StretchDIBits(pDC->m_hDC,
			ptOffset.x, ptOffset.y, szScaled.cx, szScaled.cy,
			0, 0, m_pBMI->bmiHeader.biWidth, m_pBMI->bmiHeader.biHeight,
			m_pBits, m_pBMI, DIB_RGB_COLORS, SRCCOPY);
	}
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

void CDIB::SetDPI(int nDPI)
{
	ASSERT(m_pBMI != NULL);
	m_pBMI->bmiHeader.biXPelsPerMeter = static_cast<int>(nDPI/0.0254);
	m_pBMI->bmiHeader.biYPelsPerMeter = static_cast<int>(nDPI/0.0254);
}

bool CDIB::Save(LPCTSTR pszPathName, ImageFormat nFormat) const
{
	if (nFormat >= FormatPNG)
	{
		if (!Gdip::IsLoaded())
			return false;

		Gdip::Bitmap* pGdipBitmap = NULL;
		if (Gdip::CreateBitmapFromGdiDib(m_pBMI, m_pBits, &pGdipBitmap) != Gdip::Ok)
			return false;

		Gdip::BitmapSetResolution(pGdipBitmap,
				static_cast<float>(m_pBMI->bmiHeader.biXPelsPerMeter*0.0254),
				static_cast<float>(m_pBMI->bmiHeader.biYPelsPerMeter*0.0254));

		const wchar_t* szEncoder = L"image/bmp";
		switch (nFormat)
		{
		case FormatPNG:
			szEncoder = L"image/png";
			break;
		case FormatGIF:
			szEncoder = L"image/gif";
			break;
		case FormatTIF:
			szEncoder = L"image/tiff";
			break;
		case FormatJPG:
			szEncoder = L"image/jpeg";
			break;
		}

		CLSID clsid;
		if (!Gdip::GetEncoderClsid(szEncoder, &clsid))
		{
			Gdip::DisposeImage(pGdipBitmap);
			return false;
		}

		vector<char> buf(sizeof(Gdip::EncoderParameters) + 2*sizeof(Gdip::EncoderParameter));
		Gdip::EncoderParameters* parameters = (Gdip::EncoderParameters*) &buf[0];
		parameters->Count = 0;

		LONG nTiffEncoding = Gdip::EncoderValueCompressionLZW;
		LONG nTiffColorDepth = 24;
		LONG nJpegQuality = 80;
		switch (nFormat)
		{
		case FormatTIF:
			parameters->Count = 2;
			parameters->Parameter[0].Guid = Gdip::EncoderCompression;
			parameters->Parameter[0].NumberOfValues = 1;
			parameters->Parameter[0].Type = Gdip::EncoderParameterValueTypeLong;
			parameters->Parameter[0].Value = &nTiffEncoding;
			parameters->Parameter[1].Guid = Gdip::EncoderColorDepth;
			parameters->Parameter[1].NumberOfValues = 1;
			parameters->Parameter[1].Type = Gdip::EncoderParameterValueTypeLong;
			parameters->Parameter[1].Value = &nTiffColorDepth;

			if (GetBitsPerPixel() <= 8)
			{
				if (GetColorCount() <= 2)
				{
					nTiffEncoding = Gdip::EncoderValueCompressionCCITT4;
					nTiffColorDepth = 1;

					// Invert image
					DWORD nLineLength = static_cast<DWORD>((GetWidth()*GetBitsPerPixel() - 1)/8 + 1);
					while ((nLineLength % 4) != 0)
						++nLineLength;

					for (DWORD* pData = (DWORD*)m_pBits;
						 pData < (DWORD*)m_pBits + GetHeight()*nLineLength/4;
						 ++pData)
					{
						*pData = ~*pData;
					}
				}
				else
					nTiffColorDepth = 8;
			}
			break;

		case FormatJPG:
			parameters->Count = 1;
			parameters->Parameter[0].Guid = Gdip::EncoderQuality;
			parameters->Parameter[0].NumberOfValues = 1;
			parameters->Parameter[0].Type = Gdip::EncoderParameterValueTypeLong;
			parameters->Parameter[0].Value = &nJpegQuality;
			break;
		}

		wstring wstrFileName;
		MakeWString(CString(pszPathName), wstrFileName);
		bool bResult = (Gdip::SaveImageToFile(pGdipBitmap, wstrFileName.c_str(), &clsid,
				parameters->Count > 0 ? parameters : NULL) == Gdip::Ok);

		Gdip::DisposeImage(pGdipBitmap);
		return bResult;
	}
	else
	{
		CFile file;
		if (!file.Open(pszPathName, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive))
			return false;

		BITMAPFILEHEADER hdr;
		hdr.bfType = 'MB';
		hdr.bfReserved1 = 0;
		hdr.bfReserved2 = 0;

		DWORD dwHeaderSize = sizeof(BITMAPINFOHEADER) +
			sizeof(RGBQUAD)*m_pBMI->bmiHeader.biClrUsed;
		DWORD dwBitCount = m_pBMI->bmiHeader.biWidth * m_pBMI->bmiHeader.biBitCount;
		DWORD dwDataSize = (((dwBitCount + 31) / 32) * 4) * m_pBMI->bmiHeader.biHeight;

		hdr.bfOffBits = sizeof(BITMAPFILEHEADER) + dwHeaderSize;
		hdr.bfSize = hdr.bfOffBits + dwDataSize;

		file.Write(&hdr, sizeof(hdr));
		file.Write(m_pBMI, dwHeaderSize);
		file.Write(m_pBits, dwDataSize);

		file.Close();
		return true;
	}
}

HGLOBAL CDIB::SaveToMemory() const
{
	DWORD dwHeaderSize = sizeof(BITMAPINFOHEADER) +
		sizeof(RGBQUAD)*m_pBMI->bmiHeader.biClrUsed;
	DWORD dwBitCount = m_pBMI->bmiHeader.biWidth * m_pBMI->bmiHeader.biBitCount;
	DWORD dwDataSize = (((dwBitCount + 31) / 32) * 4) * m_pBMI->bmiHeader.biHeight;

	HGLOBAL hData = ::GlobalAlloc(GMEM_MOVEABLE, dwHeaderSize + dwDataSize);
	if (hData != NULL)
	{
		LPBYTE pData = (LPBYTE) ::GlobalLock(hData);
		memmove(pData, m_pBMI, dwHeaderSize);
		memmove(pData + dwHeaderSize, m_pBits, dwDataSize);
		::GlobalUnlock(hData);
	}

	return hData;
}

// CLightweightDIB

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


// Drawing

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

void InvertFrame(CDC* pDC, const CRect& rect)
{
	CRect rcHorzLine(rect.TopLeft(), CSize(rect.Width(), 1));
	pDC->InvertRect(rcHorzLine);
	rcHorzLine.OffsetRect(0, rect.Height() - 1);
	pDC->InvertRect(rcHorzLine);

	CRect rcVertLine(CPoint(rect.left, rect.top + 1), CSize(1, rect.Height() - 2));
	pDC->InvertRect(rcVertLine);
	rcVertLine.OffsetRect(rect.Width() - 1, 0);
	pDC->InvertRect(rcVertLine);
}

void FrameOval(CDC* pDC, const CRect& bounds, COLORREF color)
{
	CPen penSolid(PS_SOLID, 1, color);
	CPen* pOldPen = pDC->SelectObject(&penSolid);

	CPoint ptLeft(bounds.left, bounds.CenterPoint().y);
	CPoint ptRight(bounds.left, bounds.CenterPoint().y);
	pDC->Arc(bounds, ptLeft, ptRight);
	pDC->Arc(bounds, ptRight, ptLeft);

	pDC->SelectObject(pOldPen);
}

void InvertOvalFrame(CDC* pDC, const CRect& bounds)
{
	CRect rcClip;
	pDC->GetClipBox(rcClip);
	if (!rcClip.IntersectRect(CRect(rcClip), bounds))
		return;

	static COffscreenDC offscreenDC;

	offscreenDC.Create(pDC, rcClip.Size());
	CDIB* pDIB = offscreenDC.GetDIB();
	if (pDIB == NULL || pDIB->m_hObject == NULL)
	{
		offscreenDC.Release();
		return;
	}

	offscreenDC.SetViewportOrg(-rcClip.TopLeft());
	offscreenDC.FillSolidRect(rcClip, RGB(0, 0, 0));
	FrameOval(&offscreenDC, bounds, RGB(255, 255, 255));

	pDC->BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(),
			&offscreenDC, rcClip.left, rcClip.top, SRCINVERT);
	offscreenDC.Release();
}

void FramePoly(CDC* pDC, const POINT* points, int nCount, COLORREF color)
{
	CPen penSolid(PS_SOLID, 1, color);
	CPen* pOldPen = pDC->SelectObject(&penSolid);

	pDC->Polyline((POINT*) points, nCount);

	pDC->SelectObject(pOldPen);
}

void InvertPolyFrame(CDC* pDC, const POINT* points, int nCount)
{
	if (nCount < 2)
		return;

	CRect rcBounds = CRect(points[0], CSize(1, 1));
	for (int i = 0; i < nCount; ++i)
		rcBounds.UnionRect(CRect(rcBounds), CRect(points[i], CSize(1, 1)));

	CRect rcClip;
	pDC->GetClipBox(rcClip);
	if (!rcClip.IntersectRect(CRect(rcClip), rcBounds))
		return;

	static COffscreenDC offscreenDC;

	offscreenDC.Create(pDC, rcClip.Size());
	CDIB* pDIB = offscreenDC.GetDIB();
	if (pDIB == NULL || pDIB->m_hObject == NULL)
	{
		offscreenDC.Release();
		return;
	}

	offscreenDC.SetViewportOrg(-rcClip.TopLeft());
	offscreenDC.FillSolidRect(rcClip, RGB(0, 0, 0));
	FramePoly(&offscreenDC, points, nCount, RGB(255, 255, 255));

	pDC->BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(),
			&offscreenDC, rcClip.left, rcClip.top, SRCINVERT);
	offscreenDC.Release();
}

void DrawDottedLine(CDC* pDC, const CPoint& ptStart, const CPoint& ptEnd, COLORREF color)
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

void DrawDottedRect(CDC* pDC, const CRect& rect, COLORREF color)
{
	DrawDottedLine(pDC, rect.TopLeft(), CPoint(rect.right, rect.top), color);
	DrawDottedLine(pDC, CPoint(rect.right - 1, rect.top + (rect.Width() + 1) % 2),
		CPoint(rect.right - 1, rect.bottom), color);
	DrawDottedLine(pDC, rect.TopLeft(), CPoint(rect.left, rect.bottom), color);
	DrawDottedLine(pDC, CPoint(rect.left + (rect.Height() + 1) % 2, rect.bottom - 1),
		CPoint(rect.right, rect.bottom - 1), color);
}

void HighlightRect(CDC* pDC, const CRect& rect, COLORREF color, double fTransparency)
{
	CRect rcClip;
	pDC->GetClipBox(rcClip);
	if (!rcClip.IntersectRect(CRect(rcClip), rect))
		return;

	static COffscreenDC offscreenDC;

	offscreenDC.Create(pDC, rcClip.Size());
	CDIB* pDIB = offscreenDC.GetDIB();
	if (pDIB == NULL || pDIB->m_hObject == NULL)
	{
		offscreenDC.Release();
		return;
	}

	ASSERT(pDIB->GetBitsPerPixel() == 24);

	offscreenDC.BitBlt(0, 0, rcClip.Width(), rcClip.Height(), pDC, rcClip.left, rcClip.top, SRCCOPY);

	int nRowLength = pDIB->GetWidth()*3;
	while (nRowLength % 4 != 0)
		++nRowLength;

	LPBYTE pBits = pDIB->GetBits();

	BYTE tableRed[256], tableGreen[256], tableBlue[256];
	for (int i = 0; i < 256; ++i)
		tableRed[i] = tableGreen[i] = tableBlue[i] = i;

	BuildTransparentcyTable(fTransparency, GetRValue(color), tableRed);
	BuildTransparentcyTable(fTransparency, GetGValue(color), tableGreen);
	BuildTransparentcyTable(fTransparency, GetBValue(color), tableBlue);

	for (int y = 0; y < rcClip.Height(); ++y, pBits += nRowLength)
	{
		LPBYTE pPixel = pBits;
		for (int x = 0; x < rcClip.Width(); ++x)
		{
			*pPixel = tableBlue[*pPixel];
			++pPixel;
			*pPixel = tableGreen[*pPixel];
			++pPixel;
			*pPixel = tableRed[*pPixel];
			++pPixel;
		}
	}

	pDC->BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), &offscreenDC, 0, 0, SRCCOPY);
	offscreenDC.Release();
}

void HighlightPolygon(CDC* pDC, const POINT* points, int nCount, COLORREF color, double fTransparency)
{
	if (nCount < 2)
		return;

	CRect rcBounds = CRect(points[0], CSize(1, 1));
	for (int nPoint = 0; nPoint < nCount; ++nPoint)
		rcBounds.UnionRect(CRect(rcBounds), CRect(points[nPoint], CSize(1, 1)));

	CRect rcClip;
	pDC->GetClipBox(rcClip);
	if (!rcClip.IntersectRect(CRect(rcClip), rcBounds))
		return;

	static COffscreenDC offscreenDC, maskDC;

	offscreenDC.Create(pDC, rcClip.Size());
	maskDC.Create(pDC, rcClip.Size());
	CDIB* pDIB = offscreenDC.GetDIB();
	CDIB* pMask = maskDC.GetDIB();
	if (pDIB == NULL || pDIB->m_hObject == NULL ||
			pMask == NULL || pMask->m_hObject == NULL)
	{
		offscreenDC.Release();
		maskDC.Release();
		return;
	}

	ASSERT(pDIB->GetBitsPerPixel() == 24);
	ASSERT(pMask->GetBitsPerPixel() == 24);
	offscreenDC.BitBlt(0, 0, rcClip.Width(), rcClip.Height(), pDC, rcClip.left, rcClip.top, SRCCOPY);

	maskDC.SetViewportOrg(-rcClip.TopLeft());
	maskDC.FillSolidRect(rcClip, RGB(255, 255, 255));

	CBrush brushBlack(RGB(0, 0, 0));
	CPen penBlack(PS_SOLID, 1, RGB(0, 0, 0));
	CBrush* pOldBrush = maskDC.SelectObject(&brushBlack);
	CPen* pOldPen = maskDC.SelectObject(&penBlack);
	maskDC.SetPolyFillMode(WINDING);

	maskDC.Polygon((POINT*) points, nCount);

	maskDC.SelectObject(pOldPen);
	maskDC.SelectObject(pOldBrush);

	int nRowLength = pDIB->GetWidth()*3;
	while (nRowLength % 4 != 0)
		++nRowLength;

	LPBYTE pBits = pDIB->GetBits();
	LPBYTE pMaskBits = pMask->GetBits();

	BYTE tableRed[256], tableGreen[256], tableBlue[256];
	for (int i = 0; i < 256; ++i)
		tableRed[i] = tableGreen[i] = tableBlue[i] = i;

	BuildTransparentcyTable(fTransparency, GetRValue(color), tableRed);
	BuildTransparentcyTable(fTransparency, GetGValue(color), tableGreen);
	BuildTransparentcyTable(fTransparency, GetBValue(color), tableBlue);

	for (int y = 0; y < rcClip.Height(); ++y, pBits += nRowLength, pMaskBits += nRowLength)
	{
		LPBYTE pPixel = pBits;
		LPBYTE pMaskPixel = pMaskBits;
		for (int x = 0; x < rcClip.Width(); ++x, pMaskPixel += 3)
		{
			if (*pMaskPixel == 0)
			{
				*pPixel = tableBlue[*pPixel];
				++pPixel;
				*pPixel = tableGreen[*pPixel];
				++pPixel;
				*pPixel = tableRed[*pPixel];
				++pPixel;
			}
			else
				pPixel += 3;
		}
	}

	pDC->BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), &offscreenDC, 0, 0, SRCCOPY);
	maskDC.Release();
	offscreenDC.Release();
}

void DrawArrow(CDC* pDC, int nArrowType, const CRect& rect, COLORREF color)
{
	POINT points[3];

	switch (nArrowType)
	{
	case ARR_LEFT:
		points[0].x = rect.left;
		points[0].y = rect.CenterPoint().y;
		points[1].x = rect.right;
		points[1].y = rect.top;
		points[2].x = rect.right;
		points[2].y = rect.bottom;
		break;

	case ARR_RIGHT:
		points[0].x = rect.left;
		points[0].y = rect.top;
		points[1].x = rect.right;
		points[1].y = rect.CenterPoint().y;
		points[2].x = rect.left;
		points[2].y = rect.bottom;
		break;

	case ARR_UP:
		points[0].x = rect.CenterPoint().x;
		points[0].y = rect.top;
		points[1].x = rect.right;
		points[1].y = rect.bottom;
		points[2].x = rect.left;
		points[2].y = rect.bottom;
		break;

	case ARR_DOWN:
	default:
		points[0].x = rect.left;
		points[0].y = rect.top;
		points[1].x = rect.right;
		points[1].y = rect.top;
		points[2].x = rect.CenterPoint().x;
		points[2].y = rect.bottom;
		break;
	}
	
	CBrush brush(color);
	CPen pen(PS_SOLID, 1, color);

	CBrush* pOldBrush = pDC->SelectObject(&brush);
	CPen* pOldPen = pDC->SelectObject(&pen);
	pDC->SetPolyFillMode(WINDING);

	pDC->Polygon(points, 3);

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
}


// COffscreenDC

COffscreenDC::COffscreenDC()
	: m_pBitmap(NULL), m_pOldBitmap(NULL), m_szBitmap(0, 0)
{
}

COffscreenDC::~COffscreenDC()
{
	if (m_hDC != NULL)
		Release();

	delete m_pBitmap;
}

bool COffscreenDC::Create(CDC* pDC, CSize size)
{
	if (m_pBitmap == NULL || m_szBitmap.cx < size.cx || m_szBitmap.cy < size.cy)
	{
		m_szBitmap.cx = max(m_szBitmap.cx, static_cast<int>(size.cx*1.1 + 0.5));
		m_szBitmap.cy = max(m_szBitmap.cy, static_cast<int>(size.cy*1.1 + 0.5));

		delete m_pBitmap;
		m_pBitmap = CDIB::CreateDIB(m_szBitmap.cx, -m_szBitmap.cy, 24); // Top-down DIB
	}

	ASSERT(m_hDC == NULL);
	if (m_hDC != NULL)
		Release();

	if (!CreateCompatibleDC(pDC))
		return false;

	SetViewportOrg(CPoint(0, 0));

	m_pOldBitmap = SelectObject(m_pBitmap);
	return true;
}

void COffscreenDC::Release()
{
	ASSERT(m_hDC != NULL);

	if (m_hDC != NULL)
	{
		SelectObject(m_pOldBitmap);
		m_pOldBitmap = NULL;

		DeleteDC();
	}
}


// CScreenDC

CScreenDC::CScreenDC()
{
	CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
}


// Printing

CRect FindContentRect(GP<DjVuImage> pImage)
{
	CRect rcResult(0, 0, 0, 0);
	if (pImage == NULL)
		return rcResult;

	CSize szImage(pImage->get_width(), pImage->get_height());

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
			int nPixel = static_cast<int>(pPixel - pBits - rcResult.top*nRowLength)/nPixelSize;
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
			int nPixel = static_cast<int>(pPixel - pBits - (rcResult.bottom - 1)*nRowLength) / nPixelSize;
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

		int nPixel = static_cast<int>(pPixel - pBits - y*nRowLength) / nPixelSize;
		rcResult.left = min(rcResult.left, nPixel);
	}

	// Right margin
	for (y = 0; y < szImage.cy; ++y)
	{
		LPBYTE pPixel = pBits + y*nRowLength + nPixelSize*szImage.cx - 1;
		LPBYTE pEndPixel = pBits + y*nRowLength + nPixelSize*rcResult.right;

		while (pPixel >= pEndPixel && *pPixel == nWhite)
			--pPixel;

		int nPixel = static_cast<int>(pPixel - pBits - y*nRowLength) / nPixelSize;
		rcResult.right = max(rcResult.right, nPixel + 1);
	}

	delete pBmpPage;
	return rcResult;
}

void PrintPage(CDC* pDC, GP<DjVuImage> pImage, int nRotate, int nMode, const CRect& rcFullPage,
	double fPrinterMMx, double fPrinterMMy, CPrintSettings& settings, GRect* pSelRect, bool bPreview)
{
	if (pImage == NULL)
		return;

	CSize szImage(pImage->get_width(), pImage->get_height());
	if (szImage.cx <= 0 || szImage.cy <= 0)
		return;

	CPoint ptSrcOffset(0, 0);
	CSize szDjVuPage(szImage);

	if (pSelRect != NULL)
	{
		ptSrcOffset = CPoint(pSelRect->xmin, pSelRect->ymin);
		szDjVuPage = CSize(pSelRect->width(), pSelRect->height());
	}
	else if (settings.bClipContent)
	{
		CRect rcContent = FindContentRect(pImage);
		ptSrcOffset = rcContent.TopLeft();

		szDjVuPage = rcContent.Size();
	}

	int nTotalRotate = GetTotalRotate(pImage, nRotate);
	if (nTotalRotate % 2 != 0)
		swap(szDjVuPage.cx, szDjVuPage.cy);

	CRect rcPage = rcFullPage;

	// Subtract margins
	rcPage.DeflateRect(static_cast<int>(settings.fMarginLeft*fPrinterMMx),
					   static_cast<int>(settings.fMarginTop*fPrinterMMy),
					   static_cast<int>(settings.fMarginRight*fPrinterMMx),
					   static_cast<int>(settings.fMarginBottom*fPrinterMMy));

	if (settings.bAutoRotate && (rcPage.Width() - rcPage.Height())*(szDjVuPage.cx - szDjVuPage.cy) < 0)
	{
		nTotalRotate = (nTotalRotate + 3) % 4;
		swap(szDjVuPage.cx, szDjVuPage.cy);
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
	if (settings.bClipContent || pSelRect != NULL || !bPreview)
	{
		if (nTotalRotate % 2 == 0)
			rect = GRect(ptSrcOffset.x, ptSrcOffset.y, szDjVuPage.cx, szDjVuPage.cy);
		else
			rect = GRect(ptSrcOffset.x, ptSrcOffset.y, szDjVuPage.cy, szDjVuPage.cx);

		rectAll = GRect(0, 0, szImage.cx, szImage.cy);
	}
	else // if (bPreview)
	{
		rect = GRect(0, 0, szScaled.cx, szScaled.cy);
		if (nTotalRotate % 2 != 0)
			swap(rect.xmax, rect.ymax);

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
		theApp.ReportFatalError();
	}

	CDisplaySettings defaultSettings;
	CDisplaySettings& displaySettings = (settings.bAdjustPrinting ? *theApp.GetDisplaySettings() : defaultSettings);

	if (pm != NULL)
	{
		if (nTotalRotate != 0)
			pm = pm->rotate(nTotalRotate);

		if ((settings.bClipContent || pSelRect != NULL) && bPreview)
		{
			// Scale the pixmap to needed size
			GRect rcDest(0, 0, szScaled.cx, szScaled.cy);
			GRect rcSrc(0, 0, szDjVuPage.cx, szDjVuPage.cy);

			GP<GPixmapScaler> ps = GPixmapScaler::create();
			ps->set_input_size(szDjVuPage.cx, szDjVuPage.cy);
			ps->set_output_size(szScaled.cx, szScaled.cy);
			ps->set_horz_ratio(szScaled.cx, szDjVuPage.cx);
			ps->set_vert_ratio(szScaled.cy, szDjVuPage.cy);

			GP<GPixmap> pmStretched = GPixmap::create();
			ps->scale(rcSrc, *pm, rcDest, *pmStretched);
			pm = pmStretched;
		}

		pBmpPage = RenderPixmap(*pm, displaySettings);
	}
	else if (bm != NULL)
	{
		if (nTotalRotate != 0)
			bm = bm->rotate(nTotalRotate);

		if ((settings.bClipContent || pSelRect != NULL) && bPreview)
		{
			// Scale the bitmap to needed size
			GRect rcDest(0, 0, szScaled.cx, szScaled.cy);
			GRect rcSrc(0, 0, szDjVuPage.cx, szDjVuPage.cy);

			GP<GBitmapScaler> bs = GBitmapScaler::create();
			bs->set_input_size(szDjVuPage.cx, szDjVuPage.cy);
			bs->set_output_size(szScaled.cx, szScaled.cy);
			bs->set_horz_ratio(szScaled.cx, szDjVuPage.cx);
			bs->set_vert_ratio(szScaled.cy, szDjVuPage.cy);

			GP<GBitmap> bmStretched = GBitmap::create();
			bs->scale(rcSrc, *bm, rcDest, *bmStretched);
			bm = bmStretched;
		}

		pBmpPage = RenderBitmap(*bm, displaySettings);

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

	DjVuSource* pSource = dlg.GetSource();

	CPrintSettings& printSettings = dlg.m_settings;

	int nPages = (int)dlg.m_arrPages.size();
	if (printSettings.nCopies > 1 && printSettings.bCollate && !dlg.m_pPrinter->bCanCollate)
	{
		// We will do collation ourselves
		nPages *= printSettings.nCopies;
		dlg.m_pDevMode->dmCopies = 1;
	}

	pProgress->SetRange(0, nPages);
	pProgress->SetStatus(LoadString(IDS_PRINTING));

	// Printing
	CDC dcPrint;
	dcPrint.CreateDC(dlg.m_pPrinter->strDriverName, dlg.m_pPrinter->strPrinterName, NULL, dlg.m_pDevMode);

	if (dcPrint.m_hDC == NULL)
	{
		pProgress->StopProgress(1);
		::CoUninitialize();
		return 1;
	}

	dcPrint.SetMapMode(MM_TEXT);

	double fPrinterMMx = dcPrint.GetDeviceCaps(LOGPIXELSX) / 25.4;
	double fPrinterMMy = dcPrint.GetDeviceCaps(LOGPIXELSY) / 25.4;

	int nPageWidth, nPageHeight;
	if (printSettings.bIgnorePrinterMargins)
	{
		nPageWidth = dcPrint.GetDeviceCaps(PHYSICALWIDTH);
		nPageHeight = dcPrint.GetDeviceCaps(PHYSICALHEIGHT);

		int nOffsetX = dcPrint.GetDeviceCaps(PHYSICALOFFSETX);
		int nOffsetY = dcPrint.GetDeviceCaps(PHYSICALOFFSETY);
		dcPrint.SetViewportOrg(-nOffsetX, -nOffsetY);
	}
	else
	{
		nPageWidth = dcPrint.GetDeviceCaps(HORZRES);
		nPageHeight = dcPrint.GetDeviceCaps(VERTRES);
	}

	CSize szPaper(nPageWidth, nPageHeight);

	CRect rcOddPage = CRect(CPoint(0, 0), szPaper), rcEvenPage = rcOddPage;
	if (printSettings.bTwoPages && !dlg.IsPrintSelection())
	{
		if (printSettings.bLandscape)
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
	di.lpszDocName = PathFindFileName(dlg.GetSource()->GetFileName());
	di.lpszDatatype = NULL;
	di.lpszOutput = (dlg.m_bPrintToFile ? _T("C:\\Output.prn") : NULL);

	if (dcPrint.StartDoc(&di) <= 0)
	{
		pProgress->StopProgress(2);
		::CoUninitialize();
		return 2;
	}

	for (int i = 0; i < nPages; ++i)
	{
		pProgress->SetStatus(FormatString(IDS_PRINTING_PAGE, i + 1, nPages));
		pProgress->SetPos(i);

		if (pProgress->IsCancelled())
		{
			dcPrint.AbortDoc();

			pProgress->StopProgress(0);
			::CoUninitialize();
			return 0;
		}

		int nPage = dlg.m_arrPages[i].first - 1;
		int nSecondPage = dlg.m_arrPages[i].second - 1;

		if ((nPage < 0 || nPage >= pSource->GetPageCount()) &&
				(!printSettings.bTwoPages || nSecondPage < 0 || nSecondPage >= pSource->GetPageCount()))
			continue;

		if (dcPrint.StartPage() <= 0)
		{
			pProgress->StopProgress(3);
			::CoUninitialize();
			return 3;
		}

		GP<DjVuImage> pImage;
		if (nPage >= 0 && nPage < pSource->GetPageCount())
			pImage = pSource->GetPage(nPage, NULL);
		if (pImage != NULL)
		{
			GRect* pSelRect = NULL;
			if (dlg.IsPrintSelection())
				pSelRect = &dlg.m_rcSelection;

			PrintPage(&dcPrint, pImage, nRotate, nMode, rcOddPage, fPrinterMMx, fPrinterMMy, printSettings, pSelRect);
		}

		pImage = NULL;
		if (printSettings.bTwoPages && nSecondPage >= 0 && nSecondPage < pSource->GetPageCount())
			pImage = pSource->GetPage(nSecondPage, NULL);
		if (pImage != NULL)
		{
			PrintPage(&dcPrint, pImage, nRotate, nMode, rcEvenPage, fPrinterMMx, fPrinterMMy, printSettings);
		}

		if (dcPrint.EndPage() <= 0)
		{
			pProgress->StopProgress(4);
			::CoUninitialize();
			return 4;
		}
	}

	dcPrint.EndDoc();

	pProgress->SetPos(nPages);
	pProgress->StopProgress(0);

	::CoUninitialize();
	return 0;
}
