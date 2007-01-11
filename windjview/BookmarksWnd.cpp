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

#include "stdafx.h"
#include "WinDjView.h"
#include "BookmarksWnd.h"
#include "DjVuDoc.h"
#include "ChildFrm.h"
#include "DjVuView.h"
#include "MainFrm.h"
#include "AppSettings.h"

// CBookmarksWnd

IMPLEMENT_DYNAMIC(CBookmarksWnd, CMyTreeCtrl)

BEGIN_MESSAGE_MAP(CBookmarksWnd, CMyTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChanged)
	ON_NOTIFY_REFLECT(TVN_ITEMCLICKED, OnItemClicked)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeyDown)
	ON_WM_CREATE()
END_MESSAGE_MAP()

CBookmarksWnd::CBookmarksWnd()
	: m_pDoc(NULL)
{
}

CBookmarksWnd::~CBookmarksWnd()
{
}


// CBookmarksWnd message handlers

int CBookmarksWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMyTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_imageList.Create(16, 16, ILC_COLOR24 | ILC_MASK, 0, 1);
	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_BOOKMARKS);
	m_imageList.Add(&bitmap, RGB(192, 64, 32));
	SetImageList(&m_imageList, TVSIL_NORMAL);

	SetItemHeight(20);
	SetWrapLabels(CAppSettings::bWrapLongBookmarks);

	return 0;
}

void CBookmarksWnd::InitBookmarks(CDjVuDoc* pDoc)
{
	m_pDoc = pDoc;
	InitBookmarks(pDoc->GetBookmarks(), TVI_ROOT, 0);
}

int CBookmarksWnd::InitBookmarks(GP<DjVmNav> bookmarks, HTREEITEM hParent, int nPos, int nCount)
{
	if (nCount == -1)
		nCount = bookmarks->getBookMarkCount();

	for (int i = 0; i < nCount && nPos < bookmarks->getBookMarkCount(); ++i)
	{
		GP<DjVmNav::DjVuBookMark> bm;
		bookmarks->getBookMark(bm, nPos);

		CString strTitle = MakeCString(bm->displayname);
		HTREEITEM hItem = InsertItem(strTitle, 0, 1, hParent);

		m_links.push_back(bm->url);
		SetItemData(hItem, (DWORD_PTR)&m_links.back());

		nPos = InitBookmarks(bookmarks, hItem, nPos + 1, bm->count);
	}

	return nPos;
}

void CBookmarksWnd::GoToBookmark(HTREEITEM hItem)
{
	GUTF8String* url = (GUTF8String*)GetItemData(hItem);

	if (url->length() > 0)
	{
		CDjVuView* pView = ((CChildFrame*)GetParentFrame())->GetDjVuView();
		pView->GoToURL(*url);
	}
}

void CBookmarksWnd::OnSelChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if (pNMTreeView->action == TVC_BYMOUSE)
	{
		HTREEITEM hItem = pNMTreeView->itemNew.hItem;
		if (hItem != NULL)
			GoToBookmark(hItem);
	}

	*pResult = 0;
}

void CBookmarksWnd::OnItemClicked(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if (pNMTreeView->action == TVC_BYMOUSE)
	{
		HTREEITEM hItem = pNMTreeView->itemNew.hItem;
		if (hItem != NULL)
			GoToBookmark(hItem);
	}

	*pResult = 0;
}

void CBookmarksWnd::OnKeyDown(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);

	if (pTVKeyDown->wVKey == VK_RETURN || pTVKeyDown->wVKey == VK_SPACE)
	{
		HTREEITEM hItem = GetSelectedItem();
		if (hItem != NULL)
			GoToBookmark(hItem);

		*pResult = 0;
	}
	else
		*pResult = 1;
}

void CBookmarksWnd::PostNcDestroy()
{
	// Should be created on heap
	delete this;
}

void CBookmarksWnd::OnSettingsChanged()
{
	SetWrapLabels(CAppSettings::bWrapLongBookmarks);
}