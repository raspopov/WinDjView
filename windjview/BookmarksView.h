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

class CDjVuDoc;


// CBookmarksView view

class CBookmarksView : public CTreeView
{
	DECLARE_DYNAMIC(CBookmarksView)

public:
	CBookmarksView();           // protected constructor used by dynamic creation
	virtual ~CBookmarksView();

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

	void GoToBookmark(HTREEITEM hItem);
	int InitBookmarks(GP<DjVmNav> bookmarks, HTREEITEM hParent, int nPos, int nCount = -1);

	list<GUTF8String> m_links;

// Generated message map functions
	afx_msg void OnSelChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyDown(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	DECLARE_MESSAGE_MAP()
};
