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
class CDjVuDoc;


class CThumbnailsView : public CScrollView
{
protected: // create from serialization only
	CThumbnailsView();
	DECLARE_DYNCREATE(CThumbnailsView)

// Attributes
public:
	CDjVuDoc* GetDocument() const { return m_pDoc; }
	void SetDocument(CDjVuDoc* pDoc) { m_pDoc = pDoc; }

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);

// Implementation
public:
	virtual ~CThumbnailsView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	int m_nPageCount;
	CDjVuDoc* m_pDoc;
	bool m_bInsideUpdateView;
	void UpdateView(/*UpdateType updateType*/);

	struct Page
	{
		Page() : pBitmap(NULL) {}
		~Page() { delete pBitmap; }

		CPoint ptOffset;
		CSize szTotal;
		CRect rcPage, rcNumber;
		CDIB* pBitmap;

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
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()
};
