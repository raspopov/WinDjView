//	Google Desktop Search DjVu Indexer Plugin
//	Copyright (C) 2005 Andrew Zhezherun
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
#include "DjVuIndexerObj.h"


DEFINE_GUID(ImageFormatPNG, 0xb96b3caf,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);

const int nThumbnailWidth = 109;
const int nThumbnailHeight = 75;


CStringW ReadPageText(GP<DjVuDocument> pDoc, int nPage)
{
	ATLASSERT(nPage >= 0 && nPage < pDoc->get_pages_num());
	CStringW strText;

	G_TRY
	{
		// Get raw data from the document and decode only page info chunk
		GP<DjVuFile> file(pDoc->get_djvu_file(nPage));
		GP<DataPool> pool = file->get_init_data_pool();
		GP<ByteStream> stream = pool->get_stream();
		GP<IFFByteStream> iff(IFFByteStream::create(stream));

		// Check file format
		GUTF8String chkid;
		if (!iff->get_chunk(chkid) ||
			(chkid != "FORM:DJVI" && chkid != "FORM:DJVU" &&
			chkid != "FORM:PM44" && chkid != "FORM:BM44"))
		{
			return L"";
		}

		// Find chunk with page info
		while (iff->get_chunk(chkid) != 0)
		{
			GP<ByteStream> chunk_stream = iff->get_bytestream();

			if (chkid == "TXTa" || chkid == "TXTz")
			{
				GP<DjVuTXT> txt = DjVuTXT::create();
				if (chkid == "TXTa")
				{
					txt->decode(chunk_stream);
				}
				else
				{
					GP<ByteStream> text_iff = BSByteStream::create(chunk_stream);
					txt->decode(text_iff);
				}

				CStringW strPageText;
				int nSize = ::MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)txt->textUTF8, -1, NULL, 0);
				::MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)txt->textUTF8, -1,
						CStrBufW(strPageText, nSize + 1), nSize);

				strText += strPageText;
			}

			// Close chunk
			iff->seek_close_chunk();
		}
	}
	G_CATCH(ex)
	{
		ex;
	}
	G_ENDCATCH;

	return strText;
}

HBITMAP RenderPixmap(GPixmap& pm)
{
	BITMAPINFOHEADER bmih;

	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = pm.columns();
	bmih.biHeight = pm.rows();
	bmih.biBitCount = 24;
	bmih.biCompression = BI_RGB;
	bmih.biClrUsed = 0;
	bmih.biPlanes = 1;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrImportant = 0;

	LPBYTE pBits;
	HBITMAP hBitmap = ::CreateDIBSection(NULL, (LPBITMAPINFO)&bmih,
		DIB_RGB_COLORS, (VOID**)&pBits, NULL, 0);

	if (hBitmap == NULL)
		return NULL;

	LPBYTE pNextBit = pBits;
	for (size_t y = 0; y < pm.rows(); ++y)
	{
		memcpy(pNextBit, pm[y], pm.columns()*3);

		pNextBit += pm.columns()*3;
		while ((pNextBit - pBits) % 4 != 0)
			++pNextBit;
	}

	return hBitmap;
}

HBITMAP RenderBitmap(GBitmap& bm)
{
	LPBITMAPINFO pBMI;

	int nPaletteEntries = bm.get_grays();

	pBMI = (LPBITMAPINFO)malloc(
		sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*nPaletteEntries);
	BITMAPINFOHEADER& bmih = pBMI->bmiHeader;

	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = bm.columns();
	bmih.biHeight = bm.rows();
	bmih.biBitCount = 8;
	bmih.biCompression = BI_RGB;
	bmih.biClrUsed = nPaletteEntries;
	bmih.biPlanes = 1;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrImportant = 0;

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

	LPBYTE pBits;
	HBITMAP hBitmap = ::CreateDIBSection(NULL, pBMI, DIB_RGB_COLORS, (VOID**)&pBits, NULL, 0);
	free(pBMI);

	if (hBitmap == NULL)
		return NULL;

	LPBYTE pNextBit = pBits;
	for (size_t y = 0; y < bm.rows(); ++y)
	{
		memcpy(pNextBit, bm[y], bm.columns());

		pNextBit += bm.columns();
		while ((pNextBit - pBits) % 4 != 0)
			++pNextBit;
	}

	return hBitmap;
}

void CreateImageThumbnail(GP<DjVuImage> pImage,
		LPBYTE& pThumbnailData, int& nThumbnailSize, CStringW& strThumbnailFormat)
{
	if (pImage == NULL)
		return;

	int nPageWidth = pImage->get_width();
	int nPageHeight = pImage->get_height();

	int nImageWidth = nThumbnailWidth;
	int nImageHeight = nImageWidth * nPageHeight / nPageWidth;

	if (nImageHeight > nThumbnailHeight)
	{
		nImageHeight = nThumbnailHeight;
		nImageWidth = nImageHeight * nPageWidth / nPageHeight;
	}

	GP<GBitmap> pGBitmap;
	GP<GPixmap> pGPixmap;

	GRect rect(0, 0, nImageWidth, nImageHeight);
	pGPixmap = pImage->get_pixmap(rect, rect);
	if (pGPixmap == NULL)
		pGBitmap = pImage->get_bitmap(rect, rect, 4);

	HBITMAP hBitmap = NULL;

	if (pGPixmap != NULL)
		hBitmap = RenderPixmap(*pGPixmap);
	else if (pGBitmap != NULL)
		hBitmap = RenderBitmap(*pGBitmap);

	if (hBitmap == NULL)
		return;

	CImage thumbnail;
	thumbnail.Create(nThumbnailWidth, nThumbnailHeight, 24);

	HDC dcThumb = thumbnail.GetDC();
	::SetBkColor(dcThumb, RGB(255, 255, 255));
	CRect rcAll(0, 0, nThumbnailWidth, nThumbnailHeight);
	::ExtTextOut(dcThumb, 0, 0, ETO_OPAQUE, &rcAll, NULL, 0, NULL);

	CImage image;
	image.Attach(hBitmap, CImage::DIBOR_BOTTOMUP);
	image.Draw(dcThumb, (nThumbnailWidth - nImageWidth) / 2, (nThumbnailHeight - nImageHeight) / 2);
	thumbnail.ReleaseDC();

	IStream* pStream = NULL;
	HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &pStream);
	if (FAILED(hr))
		return;

	strThumbnailFormat = L"image/png";
	hr = thumbnail.Save(pStream, ImageFormatPNG);
	if (FAILED(hr))
		return;

	try
	{
		IStreamPtr pStreamPtr = pStream;
		pStream->Release();

		STATSTG stat;
		pStreamPtr->Stat(&stat, STATFLAG_NONAME);

		ULARGE_INTEGER nSize, nPos;
		LARGE_INTEGER nZero = { 0 };
		pStreamPtr->Seek(nZero, STREAM_SEEK_CUR, &nSize);
		pStreamPtr->Seek(nZero, STREAM_SEEK_SET, &nPos);

		nThumbnailSize = static_cast<int>(nSize.QuadPart);
		pThumbnailData = new BYTE[nThumbnailSize];

		ULONG nRead;
		pStreamPtr->Read(pThumbnailData, nThumbnailSize, &nRead);
	}
	catch (_com_error& e)
	{
		e;
	}
}

void ReadDjVuFile(BSTR fullPath, CStringW& strText,
		LPBYTE& pThumbnailData, int& nThumbnailSize, CStringW& strThumbnailFormat)
{
	CString strFileName;
	int nSize = ::WideCharToMultiByte(CP_UTF8, 0, fullPath, -1, NULL, 0, NULL, NULL);
	::WideCharToMultiByte(CP_UTF8, 0, fullPath, -1, CStrBufA(strFileName, nSize + 1), nSize, NULL, NULL);

	GP<DjVuDocument> pDoc = NULL;
	G_TRY
	{
		pDoc = DjVuDocument::create("file://" + GUTF8String(strFileName));
		pDoc->wait_for_complete_init();
	}
	G_CATCH(ex)
	{
		ex;
		_com_issue_error(E_FAIL);
	}
	G_ENDCATCH;

	int nPageCount = pDoc->get_pages_num();
	if (nPageCount == 0)
		_com_issue_error(E_FAIL);

	strText = "";
	for (int nPage = 0; nPage < nPageCount; ++nPage)
		strText += ReadPageText(pDoc, nPage);

	G_TRY
	{
		GP<DjVuImage> pImage = pDoc->get_page(0);
		CreateImageThumbnail(pImage, pThumbnailData, nThumbnailSize, strThumbnailFormat);
	}
	G_CATCH(ex)
	{
		ex;
		delete[] pThumbnailData;
		pThumbnailData = NULL;
	}
	G_ENDCATCH;
}


// CDjVuIndexerObj

STDMETHODIMP CDjVuIndexerObj::HandleFile(BSTR fullPath, IDispatch* pFactory)
{
	try
	{
		IGoogleDesktopSearchEventFactoryPtr spEventFactory(pFactory);

		IGoogleDesktopSearchEventPtr spEvent = spEventFactory->__CreateEvent(
				(BSTR)CComBSTR(__uuidof(CDjVuIndexerObj)), L"Google.Desktop.File");

		WIN32_FILE_ATTRIBUTE_DATA data;
		if (!::GetFileAttributesExW(fullPath, GetFileExInfoStandard, &data))
			return HRESULT_FROM_WIN32(::GetLastError());

		UINT64 nSize = ((UINT64)data.nFileSizeHigh << 32) | (UINT64)data.nFileSizeLow;

		// convert the date to the variant format
		SYSTEMTIME systemTime;
		::FileTimeToSystemTime(&data.ftCreationTime, &systemTime);
		double varDate;
		::SystemTimeToVariantTime(&systemTime, &varDate);

		CStringW strContent;
		CStringW strThumbnailFormat;
		LPBYTE pThumbnailData = NULL;
		int nThumbnailSize;
		ReadDjVuFile(fullPath, strContent, pThumbnailData, nThumbnailSize, strThumbnailFormat);

		spEvent->AddProperty(L"uri", fullPath);
		spEvent->AddProperty(L"last_modified_time", _variant_t(varDate, VT_DATE));
		spEvent->AddProperty(L"native_size", nSize);
		spEvent->AddProperty(L"format", L"text/plain");
		spEvent->AddProperty(L"content", (const wchar_t*)strContent);

		if (pThumbnailData != NULL)
		{
			CComSafeArray<BYTE> arr_thumb(nThumbnailSize);
			for (int i = 0; i < nThumbnailSize; ++i)
				arr_thumb.SetAt(i, pThumbnailData[i]);

			try
			{
				spEvent->AddProperty(L"thumbnail_format", (const wchar_t*)strThumbnailFormat);
				spEvent->AddProperty(L"thumbnail", CComVariant(arr_thumb.m_psa));
			}
			catch (_com_error& e)
			{
				e;
			}

			delete pThumbnailData;
		}

		spEvent->Send(1);
		return S_OK;
	}
	catch (_com_error& e)
	{
		return Error(L"Failed to index file", __uuidof(IDjVuIndexerObj), e.Error());
	}
}
