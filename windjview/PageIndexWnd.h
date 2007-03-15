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

#include "BookmarksWnd.h"
#include "MyTreeCtrl.h"
#include "MyComboBox.h"
class DjVuSource;
struct XMLNode;


// CPageIndexWnd view

class CPageIndexWnd : public CWnd, public Observable
{
	DECLARE_DYNAMIC(CPageIndexWnd)

public:
	CPageIndexWnd();
	virtual ~CPageIndexWnd();

	bool InitPageIndex(DjVuSource* pSource);
	void Lookup(const CString& strLookup);

// Implementation
protected:
	CImageList m_imageList;
	CFont m_font;
	CRect m_rcText, m_rcGap, m_rcList;
	bool m_bChangeInternal;

	CMyTreeCtrl m_list;
	CMyComboBoxEx m_cboLookup;

	enum
	{
		ID_TEXT = 1,
		ID_LIST = 2
	};

	int AddEntries(const XMLNode& parent, HTREEITEM hParent);
	void GoToItem(HTREEITEM hItem);
	void UpdateControls();

	struct IndexEntry
	{
		wstring strFirst, strLast, strLink;
		CString strText;
		HTREEITEM hItem;
	};

	inline static bool CompareEntries(IndexEntry* lhs, IndexEntry* rhs)
		{ return lhs->strFirst < rhs->strFirst; }

	vector<IndexEntry> m_entries;
	vector<IndexEntry*> m_sorted;

	wstring& MapCharacters(wstring& str);
	bool InitCharacterMap(const GUTF8String& strCharMap);
	vector<pair<wstring, wstring> > m_charMap;

// Overrides
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	// Message map functions
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemClicked(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDownList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeText();
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLookupFocus();
	afx_msg void OnFinishEditText();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()
};
