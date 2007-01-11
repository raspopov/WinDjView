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

class CDjVuDoc;


// CSearchResultsView view

class CSearchResultsView : public CTreeView
{
	DECLARE_DYNAMIC(CSearchResultsView)

public:
	CSearchResultsView();           // protected constructor used by dynamic creation
	virtual ~CSearchResultsView();

	void AddString(const CString& strResult, int nPage, int nSelStart, int nSelEnd);
	void Reset();

// Attributes
public:
	CDjVuDoc* GetDocument() const { return m_pDoc; }
	void SetDocument(CDjVuDoc* pDoc) { m_pDoc = pDoc; }

// Overrides
public:
	virtual void OnInitialUpdate();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CDjVuDoc* m_pDoc;
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
	afx_msg void OnSelChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyDown(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	DECLARE_MESSAGE_MAP()
};
