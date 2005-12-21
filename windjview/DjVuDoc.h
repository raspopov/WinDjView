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
//	http://www.gnu.org/copyleft/gpl.html

// $Id$

#pragma once

#define WM_PAGE_DECODED (WM_USER + 18)

class CDjVuView;


struct PageInfo
{
	PageInfo() : szPage(0, 0), nDPI(0) {}
	PageInfo(GP<DjVuImage> pImage) : szPage(0, 0), nDPI(0)
	{
		if (pImage != NULL)
		{
			RotateImage(pImage, 0);
			szPage = CSize(pImage->get_width(), pImage->get_height());
			nDPI = pImage->get_dpi();
			if (szPage.cx <= 0 || szPage.cy <= 0)
			{
				szPage.cx = 100;
				szPage.cy = 100;
				nDPI = 100;
			}

			G_TRY
			{
				pTextStream = pImage->get_text();
				pAnnoStream = pImage->get_anno();
			}
			G_CATCH(ex)
			{
				ex;
			}
			G_ENDCATCH;
		}
	}

	CSize szPage;
	int nDPI;
	GP<ByteStream> pTextStream;
	GP<ByteStream> pAnnoStream;
};

class CDjVuDoc : public CDocument
{
protected: // create from serialization only
	CDjVuDoc();
	DECLARE_DYNCREATE(CDjVuDoc)

// Operations
public:
	GP<DjVuImage> GetPage(int nPage, bool bAddToCache = true);
	PageInfo GetPageInfo(int nPage);
	void RemoveFromCache(int nPage);
	bool IsPageCached(int nPage);
	int GetPageCount() const { return m_nPageCount; }

	bool HasText() const { return m_bHasText; }
	int GetPageFromId(const GUTF8String& strPageId) const;

	GP<DjVmNav> GetBookmarks() { return m_pDjVuDoc->get_djvm_nav(); }

	CDjVuView* GetDjVuView();

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
	struct PageRequest
	{
		HANDLE hEvent;
		GP<DjVuImage> pImage;
	};
	stack<HANDLE> m_eventCache;
	CCriticalSection m_eventLock;

	struct PageData
	{
		PageData() : pImage(NULL), bHasInfo(false),
					 bFullInfo(false), hDecodingThread(NULL) {}

		GP<DjVuImage> pImage;
		PageInfo info;
		bool bHasInfo;
		bool bFullInfo;

		HANDLE hDecodingThread;
		int nOrigThreadPriority;
		vector<PageRequest*> requests;
	};


	PageInfo ReadPageInfo(int nPage);
	GP<DjVuDocument> m_pDjVuDoc;
	vector<PageData> m_pages;
	int m_nPageCount;
	CCriticalSection m_lock;
	bool m_bHasText;

// Generated message map functions
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	afx_msg void OnSaveCopyAs();
	DECLARE_MESSAGE_MAP()
};
