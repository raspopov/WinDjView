//	WinDjView
//	Copyright (C) 2004-2015 Andrew Zhezherun
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
//	You should have received a copy of the GNU General Public License along
//	with this program; if not, write to the Free Software Foundation, Inc.,
//	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//	http://www.gnu.org/copyleft/gpl.html

#include "stdafx.h"
#include "WinDjView.h"
#include "SearchResultsView.h"


// CSearchResultsView

IMPLEMENT_DYNAMIC(CSearchResultsView, CTreeView)

BEGIN_MESSAGE_MAP(CSearchResultsView, CTreeView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChanged)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeyDown)
	ON_NOTIFY_REFLECT(TVN_GETINFOTIP, OnGetInfoTip)
	ON_WM_MOUSEACTIVATE()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

CSearchResultsView::CSearchResultsView()
	: m_bChangeInternal(false)
{
}

CSearchResultsView::~CSearchResultsView()
{
}


// CSearchResultsView diagnostics

#ifdef _DEBUG
void CSearchResultsView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CSearchResultsView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG


// CSearchResultsView message handlers

void CSearchResultsView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	m_imageList.Create(16, 16, ILC_COLOR24 | ILC_MASK, 0, 1);
	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_BOOKMARKS);
	m_imageList.Add(&bitmap, RGB(192, 64, 32));
	GetTreeCtrl().SetImageList(&m_imageList, TVSIL_NORMAL);

	GetTreeCtrl().SetItemHeight(20);
}

void CSearchResultsView::AddString(const CString& strResult, int nPage, int nSelStart, int nSelEnd)
{
	ResultData data;
	data.nPage = nPage;
	data.nSelStart = nSelStart;
	data.nSelEnd = nSelEnd;
	m_results.push_back(data);

	m_bChangeInternal = true;

	HTREEITEM hItem = GetTreeCtrl().InsertItem(strResult, 2, 2);
	GetTreeCtrl().SetItemData(hItem, (DWORD_PTR)&m_results.back());

	m_bChangeInternal = false;
}

void CSearchResultsView::Reset()
{
	m_bChangeInternal = true;

	GetTreeCtrl().DeleteAllItems();
	m_results.clear();

	m_bChangeInternal = false;
}

void CSearchResultsView::HilightResult(HTREEITEM hItem)
{
	ResultData* data = (ResultData*)GetTreeCtrl().GetItemData(hItem);
	UpdateObservers(SearchResultClicked(data->nPage, data->nSelStart, data->nSelEnd));
}

void CSearchResultsView::OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if (pNMTreeView->action != TVC_BYKEYBOARD && !m_bChangeInternal)
	{
		HTREEITEM hItem = pNMTreeView->itemNew.hItem;
		if (hItem != NULL)
			HilightResult(hItem);
	}

	*pResult = 0;
}

void CSearchResultsView::OnKeyDown(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);

	if (pTVKeyDown->wVKey == VK_RETURN || pTVKeyDown->wVKey == VK_SPACE)
	{
		HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();
		if (hItem != NULL)
			HilightResult(hItem);

		*pResult = 0;
	}
	else
		*pResult = 1;
}

void CSearchResultsView::OnGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTVGETINFOTIP pNMInfoTip = reinterpret_cast<LPNMTVGETINFOTIP>(pNMHDR);

	HTREEITEM hItem = pNMInfoTip->hItem;
	ResultData* data = (ResultData*)GetTreeCtrl().GetItemData(hItem);

	CString strText;
	strText.Format(IDS_PAGE_NO_TOOLTIP, data->nPage + 1);

	pNMInfoTip->pszText[pNMInfoTip->cchTextMax - 1] = '\0';
	_tcsnccpy(pNMInfoTip->pszText, strText, pNMInfoTip->cchTextMax - 1);

	*pResult = 0;
}

int CSearchResultsView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
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

BOOL CSearchResultsView::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	CWnd* pWnd = WindowFromPoint(point);
	if (pWnd != this && !IsChild(pWnd) && IsFromCurrentProcess(pWnd) &&
			pWnd->SendMessage(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(point.x, point.y)) != 0)
		return true;

	return CTreeView::OnMouseWheel(nFlags, zDelta, point);
}
