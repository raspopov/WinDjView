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

#pragma once

#define WM_PAGE_DECODED (WM_USER + 18)

struct PageInfo
{
	PageInfo() : szPage(0, 0), nDPI(0) {}
	PageInfo(GP<DjVuImage> pImage)
	{
		pImage->set_rotate(0);
		szPage = CSize(pImage->get_width(), pImage->get_height());
		nDPI = pImage->get_dpi();
		pTextStream = pImage->get_text();
	}

	CSize szPage;
	int nDPI;
	GP<ByteStream> pTextStream;
};

class CDjVuDoc : public CDocument
{
protected: // create from serialization only
	CDjVuDoc();
	DECLARE_DYNCREATE(CDjVuDoc)

// Operations
public:
	int GetPageCount() { return m_pDjVuDoc->get_pages_num(); }
	GP<DjVuImage> GetPage(int nPage);
	void PageDecoded(int nPage, GP<DjVuImage> pImage, const PageInfo& info);
	PageInfo GetPageInfo(int nPage);
	void RemoveFromCache(int nPage);

	bool HasText() const { return m_bHasText; }

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CDjVuDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


protected:
	friend class CDecodeThread;
	CDecodeThread* m_pThread;

	struct PageData
	{
		PageData() : pImage(NULL), bHasInfo(false) {}

		GP<DjVuImage> pImage;
		PageInfo info;
		bool bHasInfo;
	};

	GP<DjVuDocument> m_pDjVuDoc;
	vector<PageData> m_pages;
	bool m_bHasText;

	CCriticalSection m_lock;
	CEvent m_pageReady;
	LONG m_nPagePending;

// Generated message map functions
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	DECLARE_MESSAGE_MAP()
};

