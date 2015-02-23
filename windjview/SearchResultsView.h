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

#include "Global.h"


// CSearchResultsView view

class CSearchResultsView : public CTreeView, public Observable
{
	DECLARE_DYNAMIC(CSearchResultsView)

public:
	CSearchResultsView();
	virtual ~CSearchResultsView();

	void AddString(const CString& strResult, int nPage, int nSelStart, int nSelEnd);
	void Reset();

// Overrides
public:
	virtual void OnInitialUpdate();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CImageList m_imageList;

	struct ResultData
	{
		int nPage;
		int nSelStart, nSelEnd;
	};
	list<ResultData> m_results;
	bool m_bChangeInternal;

	void HilightResult(HTREEITEM hItem);

	// Generated message map functions
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	DECLARE_MESSAGE_MAP()
};
