//	WinDjView
//	Copyright (C) 2004-2005 Andrew Zhezherun
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
#include "BookmarksView.h"
#include "DjVuDoc.h"
#include "ChildFrm.h"
#include "DjVuView.h"
#include "MainFrm.h"


// CBookmarksView

IMPLEMENT_DYNAMIC(CBookmarksView, CTreeView)

BEGIN_MESSAGE_MAP(CBookmarksView, CTreeView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChanged)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeyDown)
	ON_WM_MOUSEACTIVATE()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

CBookmarksView::CBookmarksView()
	: m_pDoc(NULL)
{
}

CBookmarksView::~CBookmarksView()
{
}


// CBookmarksView diagnostics

#ifdef _DEBUG
void CBookmarksView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CBookmarksView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG


// CBookmarksView message handlers

void CBookmarksView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	m_imageList.Create(16, 16, ILC_COLOR24 | ILC_MASK, 0, 1);
	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_BOOKMARKS);
	m_imageList.Add(&bitmap, RGB(192, 64, 32));
	GetTreeCtrl().SetImageList(&m_imageList, TVSIL_NORMAL);

	GetTreeCtrl().SetItemHeight(20);

	InitBookmarks(GetDocument()->GetBookmarks(), TVI_ROOT, 0);
}

int CBookmarksView::InitBookmarks(GP<DjVmNav> bookmarks, HTREEITEM hParent, int nPos, int nCount)
{
	if (nCount == -1)
		nCount = bookmarks->getBookMarkCount();

	for (int i = 0; i < nCount && nPos < bookmarks->getBookMarkCount(); ++i)
	{
		GP<DjVmNav::DjVuBookMark> bm;
		bookmarks->getBookMark(bm, nPos);

		GNativeString strTitle = bm->displayname.UTF8ToNative();
		HTREEITEM hItem = GetTreeCtrl().InsertItem(strTitle, 0, 1, hParent);

		m_links.push_back(bm->url);
		GetTreeCtrl().SetItemData(hItem, (DWORD_PTR)&m_links.back());

		nPos = InitBookmarks(bookmarks, hItem, nPos + 1, bm->count);
	}

	return nPos;
}

void CBookmarksView::ActivateBookmark(HTREEITEM hItem)
{
	GUTF8String* url = (GUTF8String*)GetTreeCtrl().GetItemData(hItem);

	if (url->length() > 0)
	{
		CDjVuView* pView = ((CChildFrame*)GetParentFrame())->GetDjVuView();
		pView->GoToURL(*url, pView->GetCurrentPage());
	}
}

void CBookmarksView::OnSelChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if (pNMTreeView->action == TVC_BYMOUSE)
	{
		HTREEITEM hItem = pNMTreeView->itemNew.hItem;
		if (hItem != NULL)
			ActivateBookmark(hItem);
	}

	*pResult = 0;
}

void CBookmarksView::OnKeyDown(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);

	if (pTVKeyDown->wVKey == VK_RETURN || pTVKeyDown->wVKey == VK_SPACE)
	{
		HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();
		if (hItem != NULL)
			ActivateBookmark(hItem);

		*pResult = 0;
	}
	else
		*pResult = 1;
}

int CBookmarksView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	// From MFC: CView::OnMouseActivate
	// Don't call CFrameWnd::SetActiveView

	int nResult = CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
	if (nResult == MA_NOACTIVATE || nResult == MA_NOACTIVATEANDEAT)
		return nResult;

	// set focus to this view, but don't notify the parent frame
	OnActivateView(TRUE, this, this);
	return nResult;
}

BOOL CBookmarksView::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	CWnd* pWnd = WindowFromPoint(point);
	if (pWnd != this && !IsChild(pWnd) && GetMainFrame()->IsChild(pWnd) &&
			pWnd->SendMessage(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(point.x, point.y)) != 0)
		return true;

	return CTreeView::OnMouseWheel(nFlags, zDelta, point);
}
