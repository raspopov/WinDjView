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

const int nThumbnailWidth = 109;
const int nThumbnailHeight = 75;


CStringW ReadPageText(GP<DjVuDocument> pDoc, int nPage)
{
	ATLASSERT(nPage >= 0 && nPage < pDoc->get_pages_num());
	CStringW strText;

	try
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
	catch (...)
	{
	}

	return strText;
}

FIBITMAP* RenderThumbnail(GPixmap& pm)
{
	FIBITMAP* pBitmap = FreeImage_AllocateT(FIT_BITMAP, nThumbnailWidth, nThumbnailHeight, 24);

	LPBYTE pBits = FreeImage_GetBits(pBitmap);
	BITMAPINFO* pBMI = FreeImage_GetInfo(pBitmap);

	pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pBMI->bmiHeader.biWidth = nThumbnailWidth;
	pBMI->bmiHeader.biHeight = nThumbnailHeight;
	pBMI->bmiHeader.biBitCount = 24;
	pBMI->bmiHeader.biCompression = BI_RGB;
	pBMI->bmiHeader.biClrUsed = 0;
	pBMI->bmiHeader.biPlanes = 1;
	pBMI->bmiHeader.biSizeImage = 0;
	pBMI->bmiHeader.biXPelsPerMeter = 0;
	pBMI->bmiHeader.biYPelsPerMeter = 0;
	pBMI->bmiHeader.biClrImportant = 0;

	size_t nLeft = (nThumbnailWidth - pm.columns()) / 2;
	size_t nTop = (nThumbnailHeight - pm.rows()) / 2;

	LPBYTE pNextBit = pBits;
	for (size_t y = 0; y < nThumbnailHeight; ++y)
	{
		if (y >= nTop && y < nTop + pm.rows())
		{
			memset(pNextBit, 0xff, nLeft*3);
			memcpy(pNextBit + nLeft*3, pm[y - nTop], pm.columns()*3);
			memset(pNextBit + (nLeft + pm.columns())*3, 0xff, (nThumbnailWidth - nLeft - pm.columns())*3);
		}
		else
			memset(pNextBit, 0xff, nThumbnailWidth*3);

		pNextBit += nThumbnailWidth*3;
		while ((pNextBit - pBits) % 4 != 0)
			++pNextBit;
	}

	return pBitmap;
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

	GRect rect(0, 0, nImageWidth, nImageHeight);
	GP<GPixmap> pGPixmap = pImage->get_pixmap(rect, rect);
	if (pGPixmap == NULL)
	{
		GP<GBitmap> pGBitmap = pImage->get_bitmap(rect, rect, 4);
		if (pGBitmap != NULL)
			pGPixmap = GPixmap::create(*pGBitmap);
	}

	if (pGPixmap == NULL)
		return;

	FreeImage_Initialise(true);
	FIMEMORY* pMem = FreeImage_OpenMemory();

	FIBITMAP* pBitmap = RenderThumbnail(*pGPixmap);
	if (FreeImage_SaveToMemory(FIF_PNG, pBitmap, pMem, 0))
	{
		LPBYTE pBytes;
		DWORD nLength;
		if (FreeImage_AcquireMemory(pMem, &pBytes, &nLength))
		{
			pThumbnailData = new BYTE[nLength];
			nThumbnailSize = nLength;
			memcpy(pThumbnailData, pBytes, nLength);
			strThumbnailFormat = L"image/png";
		}
	}

	FreeImage_CloseMemory(pMem);

	FreeImage_Unload(pBitmap);
	FreeImage_DeInitialise();
}

void ReadDjVuFile(BSTR fullPath, CStringW& strText,
		LPBYTE& pThumbnailData, int& nThumbnailSize, CStringW& strThumbnailFormat)
{
	CString strFileName;
	int nSize = ::WideCharToMultiByte(CP_UTF8, 0, fullPath, -1, NULL, 0, NULL, NULL);
	::WideCharToMultiByte(CP_UTF8, 0, fullPath, -1, CStrBufA(strFileName, nSize + 1), nSize, NULL, NULL);

	GP<DjVuDocument> pDoc = NULL;
	try
	{
		pDoc = DjVuDocument::create("file://" + GUTF8String(strFileName));
		pDoc->wait_for_complete_init();
	}
	catch (...)
	{
		_com_issue_error(E_FAIL);
	}

	int nPageCount = pDoc->get_pages_num();
	if (nPageCount == 0)
		_com_issue_error(E_FAIL);

	strText = "";
	for (int nPage = 0; nPage < nPageCount; ++nPage)
		strText += ReadPageText(pDoc, nPage);

	try
	{
		GP<DjVuImage> pImage = pDoc->get_page(0);
		CreateImageThumbnail(pImage, pThumbnailData, nThumbnailSize, strThumbnailFormat);
	}
	catch (...)
	{
		delete[] pThumbnailData;
		pThumbnailData = NULL;
	}
}


// CDjVuIndexerObj

STDMETHODIMP CDjVuIndexerObj::HandleFile(BSTR fullPath, IDispatch* pFactory)
{
	ATLTRACE("File: %S\n", fullPath);

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
