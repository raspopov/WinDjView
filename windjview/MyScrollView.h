//	WinDjView
//	Copyright (C) 2004-2007 Andrew Zhezherun
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License version 2
//	as published by the Free Software Foundation.
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

protected:
	~CMyScrollView();
	CMyAnchorWnd* m_pAnchorWnd;

	void SetScrollSizesNoRepaint(const CSize& szTotal,
		const CSize& szPage, const CSize& szLine);
	void SetScrollSizes(const CSize& szTotal,
		const CSize& szPage, const CSize& szLine);
	void UpdateBarsNoRepaint();
	void ScrollToPositionNoRepaint(CPoint pt);
	void CheckScrollBars(BOOL& bHasHorzBar, BOOL& bHasVertBar) const;

protected:
	// Generated message map functions
	afx_msg void OnDestroy();
	afx_msg void OnCancelMode();
	afx_msg LRESULT OnMButtonDown(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
