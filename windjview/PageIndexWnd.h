//	WinDjView
//	Copyright (C) 2004-2006 Andrew Zhezherun
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

#include "MyTreeCtrl.h"
class CDjVuDoc;


// CPageIndexWnd view

class CPageIndexWnd : public CWnd
{
	DECLARE_DYNAMIC(CPageIndexWnd)

public:
	CPageIndexWnd();
	virtual ~CPageIndexWnd();

	bool InitPageIndex(CDjVuDoc* pDoc);

// Implementation
protected:
	CDjVuDoc* m_pDoc;
	CImageList m_imageList;
	CFont m_font;
	CRect m_rcText, m_rcGap, m_rcList;

	class CIndexEdit : public CEdit
	{
	public:
		CIndexEdit() {}
		void SetParent(CPageIndexWnd* pParent) { m_pParent = pParent; }

	protected:
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		CPageIndexWnd* m_pParent;
	};
	friend class CIndexEdit;

	CMyTreeCtrl m_list;
	CIndexEdit m_edtText;

	enum
	{
		ID_TEXT = 1,
		ID_LIST = 2
	};

	void GoToItem(HTREEITEM hItem);
	void UpdateControls();

	struct IndexEntry
	{
		wstring strFirst, strLast, strLink;
		HTREEITEM hItem;
	};

	inline static bool CompareEntries(IndexEntry* lhs, IndexEntry* rhs)
		{ return lhs->strFirst.compare(rhs->strFirst) < 0; }

	vector<IndexEntry> m_entries;
	vector<IndexEntry*> m_sorted;

// Overrides
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Generated message map functions
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSelChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnItemClicked(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyDownList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyDownText(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnChangeText();
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()
};
