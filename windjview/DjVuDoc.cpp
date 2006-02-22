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

#include "ChildFrm.h"
#include "DjVuDoc.h"
#include "DjVuView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDjVuDoc

IMPLEMENT_DYNCREATE(CDjVuDoc, CDocument)

BEGIN_MESSAGE_MAP(CDjVuDoc, CDocument)
	ON_COMMAND(ID_FILE_SAVE_COPY_AS, OnSaveCopyAs)
END_MESSAGE_MAP()


// CDjVuDoc construction/destruction

CDjVuDoc::CDjVuDoc()
	: m_nPageCount(0), m_bHasText(false)
{
}

CDjVuDoc::~CDjVuDoc()
{
	while (!m_eventCache.empty())
	{
		HANDLE hEvent = m_eventCache.top();
		m_eventCache.pop();

		::CloseHandle(hEvent);
	}
}

BOOL CDjVuDoc::OnNewDocument()
{
	return false;
}

BOOL CDjVuDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	return false;
}

void CDjVuDoc::Serialize(CArchive& ar)
{
	ASSERT(false);
}


// CDjVuDoc diagnostics

#ifdef _DEBUG
void CDjVuDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDjVuDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

GURL MakeFilenameURL(const CString& strFilename)
{
#ifdef _UNICODE
	int nSize = ::WideCharToMultiByte(CP_UTF8, 0, strFilename, -1, NULL, 0, NULL, NULL);
	LPSTR pszText = new CHAR[nSize];
	::WideCharToMultiByte(CP_UTF8, 0, strFilename, -1, pszText, nSize, NULL, NULL);

	GURL url = GURL::Filename::UTF8(pszText);
	delete[] pszText;

	return url;
#else
	return GURL::Filename::Native((LPCSTR)strFilename);
#endif
}


BOOL CDjVuDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	DeleteContents();

	TCHAR pszName[MAX_PATH] = { 0 };
	LPTSTR pszFileName;
	GetFullPathName(lpszPathName, MAX_PATH, pszName, &pszFileName);

	CFile file;
	if (!file.Open(pszName, CFile::modeRead | CFile::shareDenyWrite))
	{
		AfxMessageBox(LoadString(IDS_FAILED_TO_OPEN) + pszName);
		return false;
	}
	file.Close();

	try
	{
		m_pDjVuDoc = DjVuDocument::create(MakeFilenameURL(pszName));
		m_pDjVuDoc->wait_get_pages_num();
	}
	catch (GException&)
	{
		AfxMessageBox(LoadString(IDS_FAILED_TO_OPEN) + pszName + LoadString(IDS_NOT_VALID_DOCUMENT));
		return false;
	}
	catch (...)
	{
		ReportFatalError();
		return false;
	}

	m_nPageCount = m_pDjVuDoc->get_pages_num();
	if (m_nPageCount == 0)
	{
		AfxMessageBox(LoadString(IDS_FAILED_TO_OPEN) + pszName + LoadString(IDS_NOT_VALID_DOCUMENT));
		return false;
	}

	m_pages.clear();
	m_pages.resize(m_nPageCount);

	GetPage(0);

	// Read bookmarks
	GP<DataPool> pDataPool = m_pDjVuDoc->get_init_data_pool();
	GP<ByteStream> pStream = pDataPool->get_stream();
	GP<IFFByteStream> iff = IFFByteStream::create(pStream);

	CDjVuView* pView = GetDjVuView();
	CChildFrame* pFrame = (CChildFrame*)pView->GetParentFrame();
	pFrame->CreateNavPanes();

	return true;
}

bool CDjVuDoc::IsPageCached(int nPage)
{
	ASSERT(nPage >= 0 && nPage < m_pDjVuDoc->get_pages_num());
	const PageData& data = m_pages[nPage];

	m_lock.Lock();
	bool bCached = !!(data.pImage != NULL);
	m_lock.Unlock();

	return bCached;
}

GP<DjVuImage> CDjVuDoc::GetPage(int nPage, bool bAddToCache)
{
	ASSERT(nPage >= 0 && nPage < m_pDjVuDoc->get_pages_num());
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

	GP<DjVuFile> file;
	try
	{
		file = m_pDjVuDoc->get_djvu_file(nPage);
	}
	catch (GException&)
	{
	}
	catch (...)
	{
		ReportFatalError();
		m_lock.Unlock();
		return NULL;
	}

	m_lock.Unlock();

	if (file)
	{
		try
		{
			pImage = DjVuImage::create(file);
			file->resume_decode();
			if (pImage && THREADMODEL != NOTHREADS)
				pImage->wait_for_complete_decode();
		}
		catch (...)
		{
			ReportFatalError();
		}
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

	if (pImage != NULL && bAddToCache)
	{
		data.pImage = pImage;
		TRACE(_T("Cached: %d\n"), nPage);

		if (!data.bFullInfo)
		{
			PageInfo info(pImage);
			data.info = info;
			data.bHasInfo = true;
			data.bFullInfo = true;

			if (info.pTextStream != NULL)
				m_bHasText = true;
		}
	}
	m_lock.Unlock();

	return pImage;
}

PageInfo CDjVuDoc::GetPageInfo(int nPage)
{
	ASSERT(nPage >= 0 && nPage < m_pDjVuDoc->get_pages_num());
	PageData& data = m_pages[nPage];

	m_lock.Lock();

	if (data.bHasInfo)
	{
		PageInfo info = data.info;
		m_lock.Unlock();
		return info;
	}

	m_lock.Unlock();

	PageInfo info = ReadPageInfo(nPage);

	m_lock.Lock();
	data.info = info;
	data.bHasInfo = true;
	m_lock.Unlock();

	return info;
}

void CDjVuDoc::RemoveFromCache(int nPage)
{
	m_lock.Lock();
	GP<DjVuImage> pImage = m_pages[nPage].pImage;
	m_pages[nPage].pImage = NULL;
	m_lock.Unlock();

	// This will cause the destructor to be called
	pImage = NULL;
	TRACE(_T("Removed: %d\n"), nPage);
}

int CDjVuDoc::GetPageFromId(const GUTF8String& strPageId) const
{
	if (m_pDjVuDoc == NULL)
		return -1;
	
	return m_pDjVuDoc->id_to_page(strPageId);
}

PageInfo CDjVuDoc::ReadPageInfo(int nPage)
{
	ASSERT(nPage >= 0 && nPage < m_pDjVuDoc->get_pages_num());
	PageInfo pageInfo;
	pageInfo.szPage.cx = 100;
	pageInfo.szPage.cy = 100;
	pageInfo.nDPI = 100;

	GP<DjVuFile> file;

	m_lock.Lock();
	try
	{
		file = (m_pDjVuDoc->get_djvu_file(nPage));
	}
	catch (GException&)
	{
		m_lock.Unlock();
		return pageInfo;
	}
	catch (...)
	{
		m_lock.Unlock();
		ReportFatalError();
		return pageInfo;
	}
	m_lock.Unlock();

	try
	{
		// Get raw data from the document and decode only page info chunk
		GP<DataPool> pool = file->get_init_data_pool();
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
				if (pageInfo.pTextStream == NULL)
				{
					pageInfo.pTextStream = ByteStream::create();
				}
				else
				{
					pageInfo.pTextStream->seek(0, SEEK_END);
					pageInfo.pTextStream->write((const void*)"", 1);
				}

				const GP<IFFByteStream> iffout = IFFByteStream::create(pageInfo.pTextStream);
				iffout->put_chunk(chkid);
				iffout->copy(*chunk_stream);
				iffout->close_chunk();
			}

			// Close chunk
			iff->seek_close_chunk();
		}
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

CDjVuView* CDjVuDoc::GetDjVuView()
{
	POSITION pos = GetFirstViewPosition();
	ASSERT(pos != NULL);

	CDjVuView* pView = (CDjVuView*)GetNextView(pos);
	return pView;
}

void CDjVuDoc::OnSaveCopyAs()
{
	CString strFileName = GetTitle();

	CFileDialog dlg(false, _T("djvu"), strFileName, OFN_OVERWRITEPROMPT |
		OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST,
		LoadString(IDS_DJVU_FILTER));

	CString strTitle;
	strTitle.LoadString(IDS_SAVE_COPY_AS);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

	UINT nResult = dlg.DoModal();
	GetDjVuView()->SetFocus();
	if (nResult != IDOK)
		return;

	CWaitCursor wait;
	strFileName = dlg.GetPathName();

	if (AfxComparePath(strFileName, GetPathName()))
	{
		AfxMessageBox(IDS_CANNOT_SAVE_TO_ORIG, MB_ICONERROR | MB_OK);
		return;
	}

	try
	{
		m_pDjVuDoc->wait_for_complete_init();
		m_pDjVuDoc->save_as(MakeFilenameURL(strFileName), true);
	}
	catch (GException&)
	{
		AfxMessageBox(IDS_SAVE_ERROR, MB_ICONERROR | MB_OK);
	}
	catch (...)
	{
		ReportFatalError();
	}
}
