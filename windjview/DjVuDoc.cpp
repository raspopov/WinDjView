//	WinDjView
//	Copyright (C) 2004-2005 Andrew Zhezherun
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
	m_bHasText = false;
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
	if (!file.Open(pszName, CFile::modeRead | CFile::shareDenyWrite))
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
	m_pages.resize(m_pDjVuDoc->get_pages_num());

	m_pThread = new CDecodeThread(this);
	GetPage(0);

	return true;
}

GP<DjVuImage> CDjVuDoc::GetPage(int nPage)
{
	ASSERT(nPage >= 0 && nPage < m_pDjVuDoc->get_pages_num());

	m_lock.Lock();
	const PageData& data = m_pages[nPage];
	GP<DjVuImage> pImage = data.pImage;
	if (pImage != NULL)
	{
		m_lock.Unlock();
		return pImage;
	}

	::InterlockedExchange(&m_nPagePending, nPage);
	m_lock.Unlock();

	m_pThread->StartDecodePage(nPage);

	::WaitForSingleObject(m_pageReady.m_hObject, INFINITE);

	m_lock.Lock();
	pImage = m_pages[nPage].pImage;
	m_lock.Unlock();
/*
	GP<DjVuAnno> pAnno = pImage->get_decoded_anno();
	GP<DjVuANT> pAnt = NULL;
	if (pAnno != NULL)
		pAnt = pAnno->ant;

	if (pAnt != NULL)
	{
		GMap<GUTF8String, GUTF8String>& meta = pAnt->metadata;
		GPosition pos = meta.firstpos();
		while (pos)
		{
			const GUTF8String* pKey = meta.next(pos);
			GUTF8String value = meta[*pKey];
			TRACE("%s=%s\n", (const char*)(*pKey), (const char*)value);
		}
	}
*/
	ASSERT(pImage != NULL);
	return pImage;
}

void CDjVuDoc::PageDecoded(int nPage, GP<DjVuImage> pImage, const PageInfo& info)
{
	ASSERT(nPage >= 0 && nPage < m_pDjVuDoc->get_pages_num());

	m_lock.Lock();
	if (pImage != NULL)
	{
		ASSERT(m_pages[nPage].pImage == NULL);
		m_pages[nPage].pImage = pImage;
	}

	m_pages[nPage].info = info;
	m_pages[nPage].bHasInfo = true;
	m_lock.Unlock();

	if (info.pTextStream != NULL)
		m_bHasText = true;

	if (::InterlockedCompareExchange(&m_nPagePending, nPage, nPage) == nPage)
	{
		// If pImage is NULL, this means that when StartDecodePage executed,
		// the page was already being decoded, so it added a new entry to the
		// job list, and we must wait for the next job finish.
		if (pImage != NULL)
		{
			::InterlockedExchange(&m_nPagePending, -1);
			m_pageReady.SetEvent();
		}
	}

	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		pView->PostMessage(WM_PAGE_DECODED, nPage);
	}
}

PageInfo CDjVuDoc::GetPageInfo(int nPage)
{
	m_lock.Lock();

	if (m_pages[nPage].bHasInfo)
	{
		PageInfo info = m_pages[nPage].info;
		m_lock.Unlock();
		return info;
	}

	m_lock.Unlock();

	PageInfo info = CDecodeThread::ReadPageInfo(m_pDjVuDoc, nPage);
	m_lock.Lock();
	m_pages[nPage].info = info;
	m_pages[nPage].bHasInfo = true;
	m_lock.Unlock();

	return info;
}

void CDjVuDoc::RemoveFromCache(int nPage)
{
	m_lock.Lock();
	m_pages[nPage].pImage = NULL;
	m_lock.Unlock();
}
