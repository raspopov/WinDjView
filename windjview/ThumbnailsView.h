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

#include "MyScrollView.h"
#include "Drawing.h"
class CDjVuDoc;
class CThumbnailsThread;


// CThumbnailsView

class CThumbnailsView : public CMyScrollView
{
	DECLARE_DYNAMIC(CThumbnailsView)

public:
	CThumbnailsView();
	virtual ~CThumbnailsView();

// Attributes
public:
	CDjVuDoc* GetDocument() const { return m_pDoc; }
	void SetDocument(CDjVuDoc* pDoc) { m_pDoc = pDoc; }

	int GetCurrentPage() const { return m_nCurrentPage; }

	void SetCurrentPage(int nPage);
	void SetSelectedPage(int nPage);
	void EnsureVisible(int nPage);

	void SetRotate(int nRotate);
	int GetRotate() const { return m_nRotate; }

	void StopDecoding();
	void OnSettingsChanged();

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);

// Implementation
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CThumbnailsThread* m_pThread;
	CThumbnailsThread* m_pIdleThread;
	int m_nPageCount;
	CDjVuDoc* m_pDoc;
	bool m_bInsideUpdateView;
	CFont m_font;
	int m_nSelectedPage, m_nCurrentPage;
	bool m_bVisible;
	int m_nRotate;
	int m_nPagesInRow;

	enum UpdateType
	{
		TOP = 0,
		RECALC = 1
	};
	void UpdateView(UpdateType updateType = TOP);
	void RecalcPageRects(int nPage);
	void UpdateVisiblePages();
	void UpdatePage(int nPage, CThumbnailsThread* pThread);
	bool InvalidatePage(int nPage);
	void DrawPage(CDC* pDC, int nPage);
	int GetPageFromPoint(CPoint point);

	struct Page
	{
		Page() : pBitmap(NULL) {}
		~Page() { delete pBitmap; }

		CRect rcDisplay, rcPage, rcBitmap, rcNumber;
		CDIB* pBitmap;

		void DeleteBitmap()
		{
			delete pBitmap;
			pBitmap = NULL;
		}
	};
	vector<Page> m_pages;
	CSize m_szDisplay;

	// Generated message map functions
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnRenderFinished(WPARAM wParam, LPARAM lParam);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	DECLARE_MESSAGE_MAP()
};
