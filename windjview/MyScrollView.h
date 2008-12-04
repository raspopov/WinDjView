//	WinDjView
//	Copyright (C) 2004-2008 Andrew Zhezherun
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

// $Id$

#pragma once


class CMyAnchorWnd;

// CMyScrollView

class CMyScrollView : public CView
{
	DECLARE_DYNAMIC(CMyScrollView)
public:
	CMyScrollView();
	virtual ~CMyScrollView();

	void SetScrollSizes(const CSize& szContent, const CSize& szPage,
			const CSize& szLine, bool bRepaint = true);

// Attributes
public:
	CPoint GetScrollPosition() const;
	void CheckScrollBars(bool& bHasHorzBar, bool& bHasVertBar) const;

// Operations
public:
	bool AdjustClientSize(const CSize& szContent, CSize& szClient,
		bool& bHScroll, bool& bVScroll) const;

	virtual void ScrollToPosition(CPoint pt, bool bRepaint = true);
	virtual bool OnScroll(UINT nScrollCode, UINT nPos, bool bDoScroll = true);
	virtual bool OnScrollBy(CSize szScrollBy, bool bDoScroll = true);

	virtual bool OnStartPan();
	virtual void OnPan(CSize szScroll);
	virtual void OnEndPan();

// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void CalcWindowRect(LPRECT lpClientRect,
		UINT nAdjustType = adjustBorder);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);

// Implementation
protected:
	CSize m_szContent;
	CSize m_szPage;
	CSize m_szLine;
	CSize m_szScrollBars;

	bool m_bInsideUpdate;
	CMyAnchorWnd* m_pAnchorWnd;

	void UpdateBars(bool bRepaint);
	void GetScrollBarSizes(CSize& szScrollBars) const;
	bool GetTrueClientSize(CSize& szTrueClient, CSize& szScrollBars) const;
	void CalcScrollBarState(const CSize& szClient, CSize& needScrollBars,
		CSize& szRange, CPoint& ptMove, bool bInsideClient) const;

	// Generated message map functions
	afx_msg void OnDestroy();
	afx_msg void OnCancelMode();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	afx_msg LRESULT OnMButtonDown(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
