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

class CMyScrollView : public CScrollView
{
	DECLARE_DYNAMIC(CMyScrollView)
public:
	CMyScrollView();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	virtual void OnStartPan();
	virtual void OnPan(CSize szScroll);
	virtual void OnEndPan();

	bool AdjustClientSize(const CSize& szContent, CSize& szClient,
		bool& bHScroll, bool& bVScroll) const;

// Overrides:
public:
	virtual void CalcWindowRect(LPRECT lpClientRect,
		UINT nAdjustType = adjustBorder);

	void CheckScrollBars(BOOL& bHasHorzBar, BOOL& bHasVertBar) const;

protected:
	~CMyScrollView();
	CMyAnchorWnd* m_pAnchorWnd;

	void SetScrollSizesNoRepaint(const CSize& szTotal,
		const CSize& szPage, const CSize& szLine);
	void SetScrollSizes(const CSize& szTotal,
		const CSize& szPage, const CSize& szLine);
	void UpdateBars(bool bRepaint);
	void ScrollToPositionNoRepaint(CPoint pt);
	void GetScrollBarSizes(CSize& sizeSb) const;
	bool GetTrueClientSize(CSize& size, CSize& sizeSb) const;
	void GetScrollBarState(CSize sizeClient, CSize& needSb,
		CSize& sizeRange, CPoint& ptMove, bool bInsideClient) const;

protected:
	// Generated message map functions
	afx_msg void OnDestroy();
	afx_msg void OnCancelMode();
	afx_msg LRESULT OnMButtonDown(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
