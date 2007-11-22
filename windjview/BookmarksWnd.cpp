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

#include "stdafx.h"
#include "WinDjView.h"
#include "BookmarksWnd.h"
#include "DjVuSource.h"
#include "AppSettings.h"
#include "BookmarkDlg.h"
#include "ChildFrm.h"
#include "DjVuView.h"


// CBookmarksWnd

IMPLEMENT_DYNAMIC(CBookmarksWnd, CMyTreeCtrl)

BEGIN_MESSAGE_MAP(CBookmarksWnd, CMyTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChanged)
	ON_NOTIFY_REFLECT(TVN_ITEMCLICKED, OnItemClicked)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeyDown)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_WM_MENUSELECT()
	ON_WM_ENTERIDLE()
END_MESSAGE_MAP()

CBookmarksWnd::CBookmarksWnd(DjVuSource* pSource)
	: m_bEnableEditing(false), m_pSource(pSource)
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
	SetWrapLabels(theApp.GetAppSettings()->bWrapLongBookmarks);

	theApp.AddObserver(this);

	return 0;
}

void CBookmarksWnd::OnDestroy()
{
	theApp.RemoveObserver(this);

	CMyTreeCtrl::OnDestroy();
}

void CBookmarksWnd::LoadContents()
{
	BeginBatchUpdate();

	const GPList<DjVmNav::DjVuBookMark>& bookmarks = m_pSource->GetContents()->getBookMarkList();
	GPosition pos = bookmarks;
	AddBookmarks(bookmarks, TVI_ROOT, pos, bookmarks.size());

	EndBatchUpdate();
}

void CBookmarksWnd::LoadUserBookmarks()
{
	BeginBatchUpdate();

	DocSettings* pSettings = m_pSource->GetSettings();

	list<Bookmark>::iterator it;
	for (it = pSettings->bookmarks.begin(); it != pSettings->bookmarks.end(); ++it)
		AddBookmark(*it, TVI_ROOT);

	EndBatchUpdate();
}

void CBookmarksWnd::AddBookmarks(const GPList<DjVmNav::DjVuBookMark>& bookmarks,
	HTREEITEM hParent, GPosition& pos, int nCount)
{
	for (int i = 0; i < nCount && !!pos; ++i)
	{
		const GP<DjVmNav::DjVuBookMark> bm = bookmarks[pos];

		CString strTitle = MakeCString(bm->displayname);
		HTREEITEM hItem = InsertItem(strTitle, 0, 1, hParent);

		m_links.push_back(BookmarkInfo());
		BookmarkInfo& info = m_links.back();
		info.strURL = bm->url;
		info.pBookmark = NULL;

		SetItemData(hItem, (DWORD_PTR)&info);

		AddBookmarks(bookmarks, hItem, ++pos, bm->count);
	}
}

void CBookmarksWnd::AddBookmark(Bookmark& bookmark)
{
	HTREEITEM hItem = AddBookmark(bookmark, TVI_ROOT);
	SelectItem(hItem);
}

HTREEITEM CBookmarksWnd::AddBookmark(Bookmark& bookmark, HTREEITEM hParent)
{
	CString strTitle = MakeCString(bookmark.strTitle);
	HTREEITEM hItem = InsertItem(strTitle, 0, 1, hParent);

	m_links.push_back(BookmarkInfo());
	BookmarkInfo& info = m_links.back();
	info.strURL = bookmark.strURL;
	info.pBookmark = &bookmark;

	SetItemData(hItem, (DWORD_PTR)&info);

	list<Bookmark>::iterator it;
	for (it = bookmark.children.begin(); it != bookmark.children.end(); ++it)
		AddBookmark(*it, hItem);

	return hItem;
}

void CBookmarksWnd::GoToBookmark(HTREEITEM hItem)
{
	BookmarkInfo* pInfo = (BookmarkInfo*) GetItemData(hItem);
	if (pInfo->pBookmark != NULL)
	{
		if (pInfo->pBookmark->HasLink())
			UpdateObservers(BookmarkMsg(BOOKMARK_CLICKED, pInfo->pBookmark));
	}
	else
	{
		if (pInfo->strURL.length() > 0)
			UpdateObservers(LinkClicked(pInfo->strURL));
	}
}

void CBookmarksWnd::OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
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

void CBookmarksWnd::OnItemClicked(NMHDR* pNMHDR, LRESULT* pResult)
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

void CBookmarksWnd::OnKeyDown(NMHDR* pNMHDR, LRESULT* pResult)
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

void CBookmarksWnd::OnUpdate(const Observable* source, const Message* message)
{
	if (message->code == APP_SETTINGS_CHANGED)
	{
		SetWrapLabels(theApp.GetAppSettings()->bWrapLongBookmarks);
	}
}

void CBookmarksWnd::OnContextMenu(CWnd* pWnd, CPoint point)
{
	TreeNode* pNode = m_pSelection;
	if (!m_bEnableEditing || pNode == NULL || pNode == m_pRoot)
		return;

	CRect rcClient;
	GetClientRect(rcClient);
	ClientToScreen(rcClient);

	if (!rcClient.PtInRect(point))
	{
		point = CPoint(pNode->rcLabel.left + 2, pNode->rcLabel.bottom)
				+ rcClient.TopLeft() - GetScrollPosition();
	}

	CMenu menu;
	menu.LoadMenu(IDR_POPUP);

	CMenu* pPopup = menu.GetSubMenu(1);
	ASSERT(pPopup != NULL);

	if (pNode->pNext == NULL)
		pPopup->EnableMenuItem(ID_BOOKMARK_MOVEDOWN, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	if (pNode->pParent->pChild == pNode)
		pPopup->EnableMenuItem(ID_BOOKMARK_MOVEUP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	int nCommand = pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			point.x, point.y, this);

	switch (nCommand)
	{
	case ID_BOOKMARK_DELETE:
		DeleteBookmark(pNode);
		break;
	
	case ID_BOOKMARK_RENAME:
		RenameBookmark(pNode);
		break;

	case ID_BOOKMARK_SETDESTINATION:
		SetBookmarkDestination(pNode);
		break;

	case ID_BOOKMARK_MOVEUP:
	case ID_BOOKMARK_MOVEDOWN:
		MoveBookmark(pNode, nCommand == ID_BOOKMARK_MOVEUP);
		break;
	}
}

void CBookmarksWnd::DeleteBookmark(TreeNode* pNode)
{
	if (pNode == NULL)
		return;

	BookmarkInfo* pInfo = (BookmarkInfo*) pNode->dwUserData;
	if (pInfo->pBookmark == NULL)
		return;

	if (AfxMessageBox(IDS_PROMPT_BOOKMARK_DELETE, MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
	{
		m_pSource->GetSettings()->DeleteBookmark(pInfo->pBookmark);
		DeleteItem((HTREEITEM) pNode);
	}

	SetFocus();
}

void CBookmarksWnd::RenameBookmark(TreeNode* pNode)
{
	if (pNode == NULL)
		return;

	BookmarkInfo* pInfo = (BookmarkInfo*) pNode->dwUserData;
	if (pInfo->pBookmark == NULL)
		return;

	CBookmarkDlg dlg(IDS_RENAME_BOOKMARK);
	dlg.m_strTitle = pNode->strLabel;

	if (dlg.DoModal())
	{
		pNode->strLabel = dlg.m_strTitle;
		pInfo->pBookmark->strTitle = MakeUTF8String(dlg.m_strTitle);
		RecalcLayout();
	}

	SetFocus();
}

void CBookmarksWnd::SetBookmarkDestination(TreeNode* pNode)
{
	if (pNode == NULL)
		return;

	BookmarkInfo* pInfo = (BookmarkInfo*) pNode->dwUserData;
	if (pInfo->pBookmark == NULL)
		return;

	if (AfxMessageBox(IDS_PROMPT_BOOKMARK_DESTINATION, MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
	{
		CDjVuView* pView = ((CChildFrame*) GetParentFrame())->GetDjVuView();
		pView->CreateBookmarkFromView(*pInfo->pBookmark);
	}

	SetFocus();
}

void CBookmarksWnd::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	CMyTreeCtrl::OnMenuSelect(nItemID, nFlags, hSysMenu);

	GetTopLevelFrame()->SendMessage(WM_MENUSELECT, MAKEWPARAM(nItemID, nFlags),
			(LPARAM) hSysMenu);
}

void CBookmarksWnd::OnEnterIdle(UINT nWhy, CWnd* pWho)
{
	CMyTreeCtrl::OnEnterIdle(nWhy, pWho);

	GetTopLevelFrame()->SendMessage(WM_ENTERIDLE, nWhy, (LPARAM) pWho->GetSafeHwnd());
}

void CBookmarksWnd::MoveBookmark(TreeNode* pNode, bool bUp)
{
	if (pNode == m_pRoot)
		return;

	TreeNode* pSwapNode = pNode->pNext;
	if (bUp)
	{
		pSwapNode = pNode->pParent->pChild;
		while (pSwapNode != NULL && pSwapNode != pNode && pSwapNode->pNext != NULL && pSwapNode->pNext != pNode)
			pSwapNode = pSwapNode->pNext;
	}
	if (pSwapNode == NULL || pSwapNode == pNode)
		return;

	BookmarkInfo* pInfo = (BookmarkInfo*) pNode->dwUserData;
	BookmarkInfo* pSwapInfo = (BookmarkInfo*) pSwapNode->dwUserData;

	pInfo->pBookmark->swap(*pSwapInfo->pBookmark);
	swap(pNode->strLabel, pSwapNode->strLabel);

	RecalcLayout();
	SelectNode(pSwapNode);
}
