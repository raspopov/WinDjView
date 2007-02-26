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

// Implementation
protected:
	CImageList m_imageList;
	CFont m_font;
	CRect m_rcText, m_rcGap, m_rcList;
	bool m_bChangeInternal;

	CMyTreeCtrl m_list;
	CEdit m_edtText;

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
		CString strTextFirst;
		HTREEITEM hItem;
	};

	inline static bool CompareEntries(IndexEntry* lhs, IndexEntry* rhs)
		{ return wcscmp(lhs->strFirst.c_str(), rhs->strFirst.c_str()) < 0; }

	vector<IndexEntry> m_entries;
	vector<IndexEntry*> m_sorted;

	wstring& MapCharacters(wstring& str);
	bool InitCharacterMap(const GUTF8String& strCharMap);
	wchar_t m_charMap[65535];

// Overrides
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	// Message map functions
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
