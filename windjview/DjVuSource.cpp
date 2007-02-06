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

#include "stdafx.h"
#include "WinDjView.h"
#include "DjVuSource.h"
#include "AppSettings.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// DjVuSource

map<CString, DjVuSource*> DjVuSource::openDocuments;
map<MD5, DjVuUserData> DjVuSource::userData;

DjVuSource::DjVuSource(const CString& strFileName, GP<DjVuDocument> pDoc, DjVuUserData* pData)
	: m_strFileName(strFileName), m_pDjVuDoc(pDoc), m_nPageCount(0), m_bHasText(false), m_pUserData(pData)
{
	m_nPageCount = m_pDjVuDoc->get_pages_num();
	ASSERT(m_nPageCount > 0);

	m_pages.clear();
	m_pages.resize(m_nPageCount);

	PageInfo info = GetPageInfo(0, false, true);
	if (info.pAnt != NULL)
		m_strPageIndex = info.pAnt->metadata["page-index"];
}

DjVuSource::~DjVuSource()
{
	while (!m_eventCache.empty())
	{
		HANDLE hEvent = m_eventCache.top();
		m_eventCache.pop();

		::CloseHandle(hEvent);
	}

	openDocuments.erase(m_strFileName);
}

DjVuSource* DjVuSource::FromFile(const CString& strFileName)
{
	TCHAR pszName[MAX_PATH] = { 0 };
	LPTSTR pszFileName;
	GetFullPathName(strFileName, MAX_PATH, pszName, &pszFileName);

	DjVuSource* pSource = openDocuments[pszName];
	if (pSource != NULL)
	{
		pSource->AddRef();
		return pSource;
	}

	CFile file;
	if (!file.Open(pszName, CFile::modeRead | CFile::shareDenyWrite))
	{
		return NULL;
	}
	int nLength = min(file.GetLength(), 0x40000);
	LPBYTE pBuf = new BYTE[nLength];
	file.Read(pBuf, nLength);
	file.Close();

	MD5 digest(pBuf, nLength);
	delete[] pBuf;

	GP<DjVuDocument> pDoc = NULL;
	try
	{
		GURL url = GURL::Filename::UTF8(MakeUTF8String(CString(pszName)));
		pDoc = DjVuDocument::create(url);
		pDoc->wait_get_pages_num();
	}
	catch (GException&)
	{
		return NULL;
	}

	if (pDoc->get_pages_num() == 0)
	{
		return NULL;
	}

	map<MD5, DjVuUserData>::iterator it = userData.find(digest);
	bool bExisting = (it != userData.end());
	DjVuUserData* pData = &userData[digest];

	if (!bExisting)
	{
		theApp.LoadDjVuUserData(digest.ToString(), pData);
	}

	return new DjVuSource(pszName, pDoc, pData);
}

bool DjVuSource::IsPageCached(int nPage, Observer* observer)
{
	ASSERT(nPage >= 0 && nPage < m_pDjVuDoc->get_pages_num());
	const PageData& data = m_pages[nPage];

	m_lock.Lock();
	bool bCached = (data.pImage != NULL && data.IsObservedBy(observer));
	m_lock.Unlock();

	return bCached;
}

GP<DjVuImage> DjVuSource::GetPage(int nPage, Observer* observer)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);
	PageData& data = m_pages[nPage];

	m_lock.Lock();
	GP<DjVuImage> pImage = data.pImage;
	if (pImage != NULL)
	{
		m_lock.Unlock();
		return pImage;
	}

	if (data.hDecodingThread != NULL)
	{
		// Other thread is already decoding this page. Put
		// ourselves in a list of waiting threads
		PageRequest request;

		m_eventLock.Lock();
		if (m_eventCache.empty())
		{
			request.hEvent = ::CreateEvent(NULL, false, false, NULL);
		}
		else
		{
			request.hEvent = m_eventCache.top();
			::ResetEvent(request.hEvent);
			m_eventCache.pop();
		}
		m_eventLock.Unlock();

		// Temporarily increase priority of the decoding thread if needed
		HANDLE hOurThread = ::GetCurrentThread();
		int nOurPriority = ::GetThreadPriority(hOurThread);
		if (::GetThreadPriority(data.hDecodingThread) < nOurPriority)
			::SetThreadPriority(data.hDecodingThread, nOurPriority);

		data.requests.push_back(&request);
		
		if (observer != NULL)
			data.AddObserver(observer);

		m_lock.Unlock();

		::WaitForSingleObject(request.hEvent, INFINITE);
		pImage = request.pImage;

		m_eventLock.Lock();
		m_eventCache.push(request.hEvent);
		m_eventLock.Unlock();

		// Page will be put in cache by the thread which decoded it
		return pImage;
	}

	data.hDecodingThread = ::GetCurrentThread();
	data.nOrigThreadPriority = ::GetThreadPriority(data.hDecodingThread);

	m_lock.Unlock();

	try
	{
		GP<DjVuFile> file = m_pDjVuDoc->get_djvu_file(nPage);
		if (file)
		{
			pImage = DjVuImage::create(file);
			file->resume_decode();
			if (pImage && THREADMODEL != NOTHREADS)
				pImage->wait_for_complete_decode();
		}
	}
	catch (GException&)
	{
	}
	catch (...)
	{
		ReportFatalError();
	}

	m_lock.Lock();
	// Notify all waiting threads that image is ready
	for (size_t i = 0; i < data.requests.size(); ++i)
	{
		data.requests[i]->pImage = pImage;
		::SetEvent(data.requests[i]->hEvent);
	}

	ASSERT(data.hDecodingThread == ::GetCurrentThread());
	::SetThreadPriority(data.hDecodingThread, data.nOrigThreadPriority);
	data.hDecodingThread = NULL;
	data.requests.clear();

	if (pImage != NULL)
	{
		pImage->set_rotate(0);

		if (observer != NULL || data.HasObservers())
		{
			data.pImage = pImage;

			if (observer != NULL)
				data.AddObserver(observer);
		}

		data.info.Update(pImage);
		if (data.info.bHasText)
			m_bHasText = true;
	}
	m_lock.Unlock();

	return pImage;
}

PageInfo DjVuSource::GetPageInfo(int nPage, bool bNeedText, bool bNeedAnno)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);
	PageData& data = m_pages[nPage];

	m_lock.Lock();

	if (data.info.bDecoded && (data.info.bTextDecoded || !bNeedText)
			&& (data.info.bAnnoDecoded || !bNeedAnno))
	{
		PageInfo info = data.info;
		m_lock.Unlock();
		return info;
	}

	m_lock.Unlock();

	PageInfo info = ReadPageInfo(nPage, bNeedText, bNeedAnno);

	m_lock.Lock();
	data.info.Update(info);
	if (data.info.bHasText)
		m_bHasText = true;
	m_lock.Unlock();

	return info;
}

void DjVuSource::RemoveFromCache(int nPage, Observer* observer)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);
	PageData& data = m_pages[nPage];
	GP<DjVuImage> pImage = NULL;

	m_lock.Lock();
	data.RemoveObserver(observer);
	if (!data.HasObservers())
	{
		pImage = m_pages[nPage].pImage;
		m_pages[nPage].pImage = NULL;
	}
	m_lock.Unlock();

	// This will cause the destructor to be called, if page is removed from cache
	pImage = NULL;
}

int DjVuSource::GetPageFromId(const GUTF8String& strPageId) const
{
	if (m_pDjVuDoc == NULL)
		return -1;
	
	return m_pDjVuDoc->id_to_page(strPageId);
}

void DjVuSource::ReadAnnotations(GP<ByteStream> pInclStream, set<GUTF8String>& processed, GP<ByteStream> pAnnoStream)
{
	// Look for shared annotations
	GUTF8String strInclude;
	char buf[1024];
	int nLength;
	while ((nLength = pInclStream->read(buf, 1024)))
		strInclude += GUTF8String(buf, nLength);

	// Eat '\n' in the beginning and at the end
	while (strInclude.length() > 0 && strInclude[0] == '\n')
		strInclude = strInclude.substr(1, static_cast<unsigned int>(-1));

	while (strInclude.length() > 0 && strInclude[static_cast<int>(strInclude.length()) - 1] == '\n')
		strInclude.setat(strInclude.length() - 1, 0);

	if (strInclude.length() > 0 && processed.find(strInclude) == processed.end())
	{
		processed.insert(strInclude);

		GURL urlInclude = m_pDjVuDoc->id_to_url(strInclude);
		GP<DataPool> pool = m_pDjVuDoc->request_data(NULL, urlInclude);
		GP<ByteStream> stream = pool->get_stream();
		GP<IFFByteStream> iff(IFFByteStream::create(stream));

		// Check file format
		GUTF8String chkid;
		if (!iff->get_chunk(chkid) ||
			(chkid != "FORM:DJVI" && chkid != "FORM:DJVU" &&
			chkid != "FORM:PM44" && chkid != "FORM:BM44"))
		{
			return;
		}

		// Find chunk with page info
		while (iff->get_chunk(chkid) != 0)
		{
			GP<ByteStream> chunk_stream = iff->get_bytestream();

			if (chkid == "INCL")
			{
				ReadAnnotations(pInclStream, processed, pAnnoStream);
			}
			else if (chkid == "FORM:ANNO")
			{
				pAnnoStream->copy(*chunk_stream);
			}
			else if (chkid == "ANTa" || chkid == "ANTz")
			{
				const GP<IFFByteStream> iffout = IFFByteStream::create(pAnnoStream);
				iffout->put_chunk(chkid);
				iffout->copy(*chunk_stream);
				iffout->close_chunk();
			}

			iff->seek_close_chunk();
		}
	}
}

PageInfo DjVuSource::ReadPageInfo(int nPage, bool bNeedText, bool bNeedAnno)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);
	PageInfo pageInfo;
	pageInfo.szPage.cx = 100;
	pageInfo.szPage.cy = 100;
	pageInfo.nDPI = 100;
	pageInfo.bDecoded = true;

	GP<ByteStream> pAnnoStream;
	if (bNeedAnno)
		pAnnoStream = ByteStream::create();

	GP<ByteStream> pTextStream;
	if (bNeedText)
		pTextStream = ByteStream::create();

	try
	{
		// Get raw data from the document and decode only requested chunks
		// DjVuFile is not used to ensure that we do not wait for a lock
		// to be released and thus do not block the UI thread
		GURL url = m_pDjVuDoc->page_to_url(nPage);
		GP<DataPool> pool = m_pDjVuDoc->request_data(NULL, url);
		GP<ByteStream> stream = pool->get_stream();
		GP<IFFByteStream> iff(IFFByteStream::create(stream));

		// Check file format
		GUTF8String chkid;
		if (!iff->get_chunk(chkid) ||
			(chkid != "FORM:DJVI" && chkid != "FORM:DJVU" &&
			 chkid != "FORM:PM44" && chkid != "FORM:BM44"))
		{
			return pageInfo;
		}

		// Find chunk with page info
		while (iff->get_chunk(chkid) != 0)
		{
			GP<ByteStream> chunk_stream = iff->get_bytestream();

			if (chkid == "INFO")
			{
				// Get page dimensions and resolution from info chunk
				GP<DjVuInfo> pInfo = DjVuInfo::create();
				pInfo->decode(*chunk_stream);

				// Check data for consistency
				pageInfo.szPage.cx = max(pInfo->width, 0);
				pageInfo.szPage.cy = max(pInfo->height, 0);
				pageInfo.nInitialRotate = pInfo->orientation;
				pageInfo.nDPI = max(pInfo->dpi, 0);

				if ((pInfo->orientation & 1) != 0)
					swap(pageInfo.szPage.cx, pageInfo.szPage.cy);
			}
			else if (chkid == "PM44" || chkid == "BM44")
			{
				// Get image dimensions and resolution from bitmap chunk
				UINT serial = chunk_stream->read8();
				UINT slices = chunk_stream->read8();
				UINT major = chunk_stream->read8();
				UINT minor = chunk_stream->read8();

				UINT xhi = chunk_stream->read8();
				UINT xlo = chunk_stream->read8();
				UINT yhi = chunk_stream->read8();
				UINT ylo = chunk_stream->read8();

				pageInfo.szPage.cx = (xhi << 8) | xlo;
				pageInfo.szPage.cy = (yhi << 8) | ylo;
				pageInfo.nDPI = 100;
			}
			else if (chkid == "TXTa" || chkid == "TXTz")
			{
				pageInfo.bHasText = true;

				if (bNeedText)
				{
					const GP<IFFByteStream> iffout = IFFByteStream::create(pTextStream);
					iffout->put_chunk(chkid);
					iffout->copy(*chunk_stream);
					iffout->close_chunk();
				}
			}
			else if (bNeedAnno && chkid == "FORM:ANNO")
			{
				pAnnoStream->copy(*chunk_stream);
			}
			else if (bNeedAnno && (chkid == "ANTa" || chkid == "ANTz"))
			{
				const GP<IFFByteStream> iffout = IFFByteStream::create(pAnnoStream);
				iffout->put_chunk(chkid);
				iffout->copy(*chunk_stream);
				iffout->close_chunk();
			}
			else if (bNeedAnno && chkid == "INCL")
			{
				set<GUTF8String> processed;
				ReadAnnotations(chunk_stream, processed, pAnnoStream);
			}

			iff->seek_close_chunk();
		}

		if (bNeedText && pTextStream->tell())
			pageInfo.DecodeText(pTextStream);

		if (bNeedAnno && pAnnoStream->tell())
			pageInfo.DecodeAnno(pAnnoStream);
	}
	catch (GException&)
	{
	}
	catch (...)
	{
		ReportFatalError();
	}

	return pageInfo;
}

bool DjVuSource::SaveAs(const CString& strFileName)
{
	try
	{
		m_pDjVuDoc->wait_for_complete_init();
		GURL url = GURL::Filename::UTF8(MakeUTF8String(strFileName));
		m_pDjVuDoc->write(ByteStream::create(url, "wb"), true);
		return true;
	}
	catch (GException&)
	{
		return false;
	}
}

DjVuUserData::DjVuUserData()
	: nPage(-1), ptOffset(0, 0), nZoomType(-10), fZoom(100.0), nLayout(-1),
	  bFirstPageAlone(CAppSettings::bFirstPageAlone), nDisplayMode(-1)
{
}
