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

#include "DjVuDoc.h"
#include "DecodeThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDjVuDoc

IMPLEMENT_DYNCREATE(CDjVuDoc, CDocument)

BEGIN_MESSAGE_MAP(CDjVuDoc, CDocument)
END_MESSAGE_MAP()


// CDjVuDoc construction/destruction

CDjVuDoc::CDjVuDoc()
{
	m_pThread = NULL;
}

CDjVuDoc::~CDjVuDoc()
{
	delete m_pThread;
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


BOOL CDjVuDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	DeleteContents();

	TCHAR pszName[MAX_PATH] = { 0 };
	LPTSTR pszFileName;
	GetFullPathName(lpszPathName, MAX_PATH, pszName, &pszFileName);

	CFile file;
	if (!file.Open(pszName, CFile::readOnly | CFile::shareDenyWrite))
	{
		AfxMessageBox("Failed to open file " + CString(pszName));
		return false;
	}
	file.Close();

	G_TRY
	{
		m_pDjVuDoc = DjVuDocument::create("file://" + GUTF8String(pszName));
		m_pDjVuDoc->wait_get_pages_num();
	}
	G_CATCH(ex) 
	{
		AfxMessageBox("Error opening file " + CString(pszName) + ":\n" + CString(ex.get_cause()));
		return false;
	}
	G_ENDCATCH;

	m_pages.clear();
	m_pages.resize(m_pDjVuDoc->get_pages_num(), NULL);

	m_pThread = new CDecodeThread(this);
	GetPage(0);

	return true;
}

GP<DjVuImage> CDjVuDoc::GetPage(int nPage)
{
	ASSERT(nPage >= 0 && nPage < m_pDjVuDoc->get_pages_num());

	m_lock.Lock();
	GP<DjVuImage> pImage = m_pages[nPage];
	if (pImage != NULL)
	{
		m_lock.Unlock();
		return pImage;
	}

	::InterlockedExchange(&m_nPagePending, nPage);
	m_lock.Unlock();

	m_pThread->MoveToFront(nPage);

	::WaitForSingleObject(m_pageReady.m_hObject, INFINITE);

	m_lock.Lock();
	pImage = m_pages[nPage];
	m_lock.Unlock();

	ASSERT(pImage != NULL);
	return pImage;
}

void CDjVuDoc::PageDecoded(int nPage, GP<DjVuImage> pImage)
{
	ASSERT(nPage >= 0 && nPage < m_pDjVuDoc->get_pages_num());

	m_lock.Lock();
	ASSERT(m_pages[nPage] == NULL);
	m_pages[nPage] = pImage;
	m_lock.Unlock();

	if (::InterlockedCompareExchange(&m_nPagePending, -1, nPage) == nPage)
	{
		TRACE("Pending page decoded: %d\n", nPage);
		m_pageReady.SetEvent();
	}

	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		pView->PostMessage(WM_PAGE_DECODED, nPage);
	}
}

bool CDjVuDoc::GetPageInfo(int nPage, CSize& szPage, int& nDPI)
{
	ASSERT(nPage >= 0 && nPage < m_pDjVuDoc->get_pages_num());

	G_TRY
	{
		// Get raw data from the document and decode only page info chunk
		GP<DjVuFile> file(m_pDjVuDoc->get_djvu_file(nPage));
		GP<DataPool> pool = file->get_init_data_pool();
		GP<ByteStream> stream = pool->get_stream();
		GP<IFFByteStream> iff(IFFByteStream::create(stream));

		// Check file format
		GUTF8String chkid;
		if (!iff->get_chunk(chkid) ||
			(chkid != "FORM:DJVI" && chkid != "FORM:DJVU" &&
			 chkid != "FORM:PM44" && chkid != "FORM:BM44"))
		{
			szPage.cx = szPage.cy = nDPI = 0;
			return false;
		}

		// Find chunk with page info
		while (iff->get_chunk(chkid) != 0)
		{
			// Decode and get chunk description
			if (chkid == "INFO")
			{
				GP<DjVuInfo> info = DjVuInfo::create();
				GP<ByteStream> chunk_stream = iff->get_bytestream();
				info->decode(*chunk_stream);

				// Check data for consistency
				szPage.cx = max(info->width, 0);
				szPage.cy = max(info->height, 0);
				nDPI = max(info->dpi, 0);
				return true;
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

	szPage.cx = szPage.cy = nDPI = 0;
	return false;
}
