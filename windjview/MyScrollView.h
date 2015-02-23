//	WinDjView
//	Copyright (C) 2004-2015 Andrew Zhezherun
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
//	You should have received a copy of the GNU General Public License along
//	with this program; if not, write to the Free Software Foundation, Inc.,
//	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//	http://www.gnu.org/copyleft/gpl.html

#pragma once


class CMyAnchorWnd;

// CMyScrollView

class CMyScrollView : public CView
{
	DECLARE_DYNAMIC(CMyScrollView)
public:
	CMyScrollView();
	virtual ~CMyScrollView();

// Attributes
public:
	const CPoint& GetScrollPosition() const { return m_ptScrollPos; }
	const CSize& GetScrollLimit() const { return m_szScrollLimit; }
	const CSize& GetViewportSize() const { return m_szViewport; }
	bool HasHorzScrollBar() const { return m_bHorzScroll; }
	bool HasVertScrollBar() const { return m_bVertScroll; }
	bool HasScrollBars() const { return m_bHorzScroll || m_bVertScroll; }

// Operations
public:
	void ShowScrollBars(bool bShow);
	void SetScrollSizes(const CSize& szContent, const CSize& szPage,
			const CSize& szLine, bool bRepaint = true);
	bool AdjustViewportSize(const CSize& szContent, CSize& szViewport,
		bool& bHScroll, bool& bVScroll) const;

	void InvalidateViewport();

	virtual void ScrollToPosition(CPoint pt, bool bRepaint = true);
	virtual bool OnScroll(UINT nScrollCode, UINT nPos, bool bDoScroll = true);
	virtual bool OnScrollBy(CSize szScrollBy, bool bDoScroll = true);

	virtual bool OnStartPan();
	virtual void OnPan(CSize szScroll);
	virtual void OnEndPan();

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	virtual void CalcWindowRect(LPRECT lpClientRect,
			UINT nAdjustType = adjustBorder);

// Implementation
protected:
	CSize m_szContent;
	CSize m_szPage, m_szLine;
	CSize m_szScrollBars;
	CSize m_szViewport, m_szScrollLimit;
	CPoint m_ptScrollPos;
	CScrollBar m_horzScrollBar, m_vertScrollBar;
	bool m_bHorzScroll, m_bVertScroll;
	bool m_bShowScrollBars;
	bool m_bPanning;
	CMyAnchorWnd* m_pAnchorWnd;

	void UpdateBars(bool bRepaint = true);
	void SetScrollPosition(const CPoint& pos);
	void RepositionScrollBars(bool bHorzScroll, bool bVertScroll);
	void CalcScrollBarState(bool& bNeedHorz, bool& bNeedVert,
			CSize& szRange, CPoint& ptMove) const;
	void DoPaint(CDC* pDC);

	// Generated message map functions
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg LRESULT OnPrintClient(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	afx_msg LRESULT OnMButtonDown(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCancelMode();
	DECLARE_MESSAGE_MAP()

	friend class CMyAnchorWnd;
};
