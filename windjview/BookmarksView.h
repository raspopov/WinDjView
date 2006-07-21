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


// CBookmarksView view

class CBookmarksView : public CMyTreeCtrl
{
	DECLARE_DYNAMIC(CBookmarksView)

public:
	CBookmarksView();
	virtual ~CBookmarksView();

	void InitBookmarks(CDjVuDoc* pDoc);

// Implementation
protected:
	CDjVuDoc* m_pDoc;
	CImageList m_imageList;

	void GoToBookmark(HTREEITEM hItem);
	int InitBookmarks(GP<DjVmNav> bookmarks, HTREEITEM hParent, int nPos, int nCount = -1);

	list<GUTF8String> m_links;

// Generated message map functions
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSelChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyDown(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()
};
