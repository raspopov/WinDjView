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
struct Bookmark;


// CBookmarksWnd window

class CBookmarksWnd : public CMyTreeCtrl, public Observer, public Observable
{
	DECLARE_DYNAMIC(CBookmarksWnd)

public:
	CBookmarksWnd(DjVuSource* pSource);
	virtual ~CBookmarksWnd();

	void InitBookmarks();
	void InitCustomBookmarks();
	void AddBookmark(Bookmark& bookmark);
	void EnableEditing(bool bEnable = true) { m_bEnableEditing = bEnable; }

	virtual void OnUpdate(const Observable* source, const Message* message);

// Implementation
protected:
	DjVuSource* m_pSource;
	CImageList m_imageList;
	bool m_bEnableEditing;

	void GoToBookmark(HTREEITEM hItem);
	void InitBookmarks(const GPList<DjVmNav::DjVuBookMark>& bookmarks,
		HTREEITEM hParent, GPosition& pos, int nCount);
	HTREEITEM AddBookmark(Bookmark& bookmark, HTREEITEM hParent);
	void DeleteBookmark(TreeNode* pNode);
	void RenameBookmark(TreeNode* pNode);
	void SetBookmarkDestination(TreeNode* pNode);
	void MoveBookmark(TreeNode* pNode, bool bUp);

	struct BookmarkInfo
	{
		GUTF8String strURL;
		Bookmark* pBookmark;
	};
	list<BookmarkInfo> m_links;

	// Message map functions
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemClicked(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()
};
