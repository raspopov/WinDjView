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

#include "Global.h"
#include "MyTreeCtrl.h"
class DjVuSource;


// CBookmarksWnd view

class CBookmarksWnd : public CMyTreeCtrl, public Observable
{
	DECLARE_DYNAMIC(CBookmarksWnd)

public:
	CBookmarksWnd();
	virtual ~CBookmarksWnd();

	void InitBookmarks(DjVuSource* pSource);
	void OnSettingsChanged();

// Implementation
protected:
	CImageList m_imageList;

	void GoToBookmark(HTREEITEM hItem);
	void InitBookmarks(const GPList<DjVmNav::DjVuBookMark>& bookmarks,
		HTREEITEM hParent, GPosition& pos, int nCount);

	list<GUTF8String> m_links;

// Generated message map functions
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSelChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnItemClicked(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyDown(NMHDR *pNMHDR, LRESULT *pResult);
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()
};
