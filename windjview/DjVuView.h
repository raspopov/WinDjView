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

#pragma once

class CDjVuDoc;
class CPrintDlg;

inline bool IsStandardZoom(int nZoomType, double fZoom)
{
	return (nZoomType < 0 || fZoom == 50.0 || fZoom == 75.0 ||
			fZoom == 100.0 || fZoom == 200.0 || fZoom == 400.0);
}


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
	void RenderPage(int nPage);
	void UpdatePageSize();
	int GetPageCount() const { return m_nPageCount; }
	int GetCurrentPage() const { return m_nPage; }
	int GetZoomType() const { return m_nZoomType; }
	double GetZoom() const;
	void ZoomTo(int nZoomType, double fZoom = 100.0);

	enum ZoomType
	{
		ZoomPercent = 0,
		ZoomFitWidth = -1,
		ZoomFitHeight = -2,
		ZoomFitPage = -3,
		ZoomActualSize = -4,
		ZoomStretch = -5
	};

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

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
	int m_nPage, m_nPageCount;
	GP<DjVuImage> m_pImage;
	CSize m_szPage, m_szDisplay, m_szDisplayPage;
	CPoint m_ptPageOffset;

	GP<GPixmap> m_pGPixmap;
	GP<GBitmap> m_pGBitmap;
	GP<GPixmap> m_pGPixmapStretched;
	GP<GBitmap> m_pGBitmapStretched;
	CSize m_szStretched;

	CBitmap* m_pBmpDisplay;
	CBitmap* m_pBmpClient;
	CSize m_szBitmap, m_szClient;
	bool m_bRepaint;
	void DrawVisible(CDC* pDC);
	void DrawOffscreen(CDC* pDC);
	void StretchBitmap();

	int m_nZoomType;
	double m_fZoom;

	bool m_bDragging;
	CPoint m_ptStart, m_ptStartPos;
	static HCURSOR hCursorHand;
	static HCURSOR hCursorDrag;

// Generated message map functions
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnPageInformation();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnViewZoom(UINT nID);
	afx_msg void OnUpdateViewZoom(CCmdUI *pCmdUI);
	afx_msg void OnViewNextpage();
	afx_msg void OnUpdateViewNextpage(CCmdUI *pCmdUI);
	afx_msg void OnViewPreviouspage();
	afx_msg void OnUpdateViewPreviouspage(CCmdUI *pCmdUI);
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
	DECLARE_MESSAGE_MAP()
};


#ifndef _DEBUG  // debug version in DjVuView.cpp
inline CDjVuDoc* CDjVuView::GetDocument() const
   { return reinterpret_cast<CDjVuDoc*>(m_pDocument); }
#endif

