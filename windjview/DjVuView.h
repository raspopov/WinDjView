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

#include "Drawing.h"
#include "DjVuDoc.h"

class CPrintDlg;
class CRenderThread;

inline bool IsStandardZoom(int nZoomType, double fZoom)
{
	return (nZoomType < 0 || fZoom == 50.0 || fZoom == 75.0 ||
			fZoom == 100.0 || fZoom == 200.0 || fZoom == 400.0);
}

typedef GList<DjVuTXT::Zone*> DjVuSelection;


class CDjVuView : public CScrollView
{
protected: // create from serialization only
	CDjVuView();
	DECLARE_DYNCREATE(CDjVuView)

// Attributes
public:
	CDjVuDoc* GetDocument() const;

// Operations
public:
	void RenderPage(int nPage, int nTimeout = -1);
	int GetPageCount() const { return m_nPageCount; }
	int GetCurrentPage() const;
	int GetZoomType() const { return m_nZoomType; }
	double GetZoom() const;
	void ZoomTo(int nZoomType, double fZoom = 100.0);
	int GetLayout() const { return m_nLayout; }
	int GetRotate() const { return m_nRotate; }
	void GoToURL(const GUTF8String& url, int nLinkPage, bool bAddToHistory = true);

	enum ZoomType
	{
		ZoomPercent = 0,
		ZoomFitWidth = -1,
		ZoomFitHeight = -2,
		ZoomFitPage = -3,
		ZoomActualSize = -4,
		ZoomStretch = -5
	};

	enum LayoutType
	{
		SinglePage = 0,
		Continuous = 1
	};

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CDjVuView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CToolTipCtrl m_toolTip;
	CRenderThread* m_pRenderThread;
	CEvent m_evtRendered;

	int m_nPage, m_nPageCount;
	CSize m_szDisplay;
	void CalcTopPage();

	void DrawPage(CDC* pDC, int nPage);
	void DrawWhite(CDC* pDC, int nPage);
	void DrawOffscreen(CDC* pDC, int nPage);
	void DrawStretch(CDC* pDC, int nPage);
	void InvalidatePage(int nPage);
	void DrawMapArea(CDC* pDC, GP<GMapArea> pArea, int nPage, bool bActive);

	int m_nZoomType;
	double m_fZoom;
	int m_nLayout;
	int m_nRotate;

	struct Page
	{
		Page() :
			bInfoLoaded(false), szDisplay(0, 0), ptOffset(0, 0),
			pBitmap(NULL), bTextDecoded(false), bAnnoDecoded(false) {}
		~Page() { delete pBitmap; }

		CSize GetSize(int nRotate) const
		{
			CSize sz(info.szPage);
			if ((nRotate % 2) == 1)
				swap(sz.cx, sz.cy);
			return sz;
		}

		void Init(const PageInfo& info)
		{
			this->info = info;
			bInfoLoaded = true;
			DecodeAnno();
		}

		bool bInfoLoaded;
		PageInfo info;

		CPoint ptOffset;
		CSize szDisplay;
		CRect rcDisplay;
		CDIB* pBitmap;

		bool bTextDecoded;
		GP<DjVuTXT> pText;
		DjVuSelection selection;
		int nSelStart, nSelEnd;

		bool bAnnoDecoded;
		GP<DjVuANT> pAnt;

		void DecodeText()
		{
			if (info.pTextStream != NULL && !bTextDecoded)
			{
				info.pTextStream->seek(0);
				GP<DjVuText> pDjVuText = DjVuText::create();
				pDjVuText->decode(info.pTextStream);
				pText = pDjVuText->txt;
				bTextDecoded = true;
			}
		}

		void DecodeAnno()
		{
			if (info.pAnnoStream != NULL && !bAnnoDecoded)
			{
				info.pAnnoStream->seek(0);
				GP<DjVuAnno> pDjVuAnno = DjVuAnno::create();
				pDjVuAnno->decode(info.pAnnoStream);
				pAnt = pDjVuAnno->ant;
				bAnnoDecoded = true;
			}
		}

		void DeleteBitmap()
		{
			delete pBitmap;
			pBitmap = NULL;
		}
	};
	vector<Page> m_pages;

	void UpdatePageSize(int nPage);
	CSize CalcPageSize(const CSize& szPage, int nDPI);
	void UpdatePageSizes(int nTop, int nScroll = 0);
	bool UpdatePagesFromTop(int nTop, int nBottom);
	void UpdatePagesFromBottom(int nTop, int nBottom);
	void DeleteBitmaps();
	int GetPageFromPoint(CPoint point);

	void SetScrollSizesNoRepaint(const CSize& szTotal,
		const CSize& szPage, const CSize& szLine);
	void SetScrollSizes(const CSize& szTotal,
		const CSize& szPage, const CSize& szLine);
	void UpdateBarsNoRepaint();
	void ScrollToPositionNoRepaint(CPoint pt);

	enum UpdateType
	{
		TOP = 0,
		BOTTOM = 1,
		RECALC = 2
	};
	void UpdateView(UpdateType updateType = TOP);
	void UpdateVisiblePages();
	void UpdatePagesCache();
	void UpdatePageCache(int nPage, const CRect& rcClient);
	void UpdatePageCacheSingle(int nPage);
	void ClearSelection();
	bool IsSelectionBelowTop(int nPage, const DjVuSelection& selection);
	bool IsSelectionVisible(int nPage, const DjVuSelection& selection);
	void EnsureSelectionVisible(int nPage, const DjVuSelection& selection);
	CRect GetSelectionRect(int nPage, const DjVuSelection& selection) const;
	CRect TranslatePageRect(int nPage, GRect rect) const;
	bool m_bInsideUpdateView;

	GP<GMapArea> m_pActiveLink;
	int m_nLinkPage;
	GP<GMapArea> GetHyperlinkFromPoint(CPoint point, int* pnPage = NULL);
	void UpdateActiveHyperlink(CPoint point);
	void GoToPage(int nPage, int nLinkPage, bool bAddToHistory = true);

	struct View
	{
		int nPage;
		int nTopOffset, nLeftOffset;

		bool operator==(const View& rhs) const { return nPage == rhs.nPage; }
		bool operator!=(const View& rhs) const { return !(*this == rhs); }
	};
	list<View> m_history;
	list<View>::iterator m_historyPos;

	int m_nClickedPage;
	bool m_bDragging;
	CPoint m_ptStart, m_ptStartPos;
	int m_nStartPage;
	bool m_bClick;
	CPoint m_ptClick;
	int m_nClickCount;
	static HCURSOR hCursorHand;
	static HCURSOR hCursorDrag;
	static HCURSOR hCursorLink;

public:
	int m_nPendingPage;
	afx_msg LRESULT OnRenderFinished(WPARAM wParam, LPARAM lParam);

	// Generated message map functions
protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPageInformation();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnViewZoom(UINT nID);
	afx_msg void OnUpdateViewZoom(CCmdUI *pCmdUI);
	afx_msg void OnViewNextpage();
	afx_msg void OnUpdateViewNextpage(CCmdUI *pCmdUI);
	afx_msg void OnViewPreviouspage();
	afx_msg void OnUpdateViewPreviouspage(CCmdUI *pCmdUI);
	afx_msg void OnViewBack();
	afx_msg void OnUpdateViewBack(CCmdUI *pCmdUI);
	afx_msg void OnViewForward();
	afx_msg void OnUpdateViewForward(CCmdUI *pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnRotateLeft();
	afx_msg void OnRotateRight();
	afx_msg void OnRotate180();
	afx_msg void OnViewFirstpage();
	afx_msg void OnViewLastpage();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnFilePrint();
	afx_msg void OnViewLayout(UINT nID);
	afx_msg void OnUpdateViewLayout(CCmdUI* pCmdUI);
	afx_msg LRESULT OnPageDecoded(WPARAM wParam, LPARAM lParam = 0);
	afx_msg void OnDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnExportPage();
	afx_msg void OnFindString();
	afx_msg BOOL OnToolTipNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};


#ifndef _DEBUG  // debug version in DjVuView.cpp
inline CDjVuDoc* CDjVuView::GetDocument() const
   { return reinterpret_cast<CDjVuDoc*>(m_pDocument); }
#endif

