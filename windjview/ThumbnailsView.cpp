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
//  http://www.gnu.org/copyleft/gpl.html

// $Id$

#include "stdafx.h"
#include "WinDjView.h"

#include "DjVuDoc.h"
#include "ThumbnailsView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int nVertMargin = 3;
const int nHorzMargin = 3;
const int nFrameWidth = 1;
const int nPadding = 3;
const int nNumberSkip = 1;
const int nNumberHeight = 8;
const int nNumberWidth = 25;
const int nPageWidth = 100;
const int nPageHeight = 110;


// CThumbnailsView

IMPLEMENT_DYNCREATE(CThumbnailsView, CScrollView)

BEGIN_MESSAGE_MAP(CThumbnailsView, CScrollView)
	ON_WM_ERASEBKGND()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEACTIVATE()
END_MESSAGE_MAP()

// CThumbnailsView construction/destruction

CThumbnailsView::CThumbnailsView()
	: m_bInsideUpdateView(false), m_nPageCount(0)
{
}

CThumbnailsView::~CThumbnailsView()
{
}

// CThumbnailsView drawing

void CThumbnailsView::OnDraw(CDC* pDC)
{
	// CScrollView sets viewport to minus scroll position, so that
	// all drawing can be done in natural coordinates. Unfortunately,
	// this does not work in Win98, because in this OS coordinates
	// cannot be larger than 32767. So we will subtract scroll position
	// explicitely.
	pDC->SetViewportOrg(CPoint(0, 0));
}


// CThumbnailsView diagnostics

#ifdef _DEBUG
void CThumbnailsView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CThumbnailsView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG


// CThumbnailsView message handlers

void CThumbnailsView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	m_nPageCount = GetDocument()->GetPageCount();
	m_pages.resize(m_nPageCount);

//	m_pThumbanilsThread = new CThumbnailsThread(GetDocument(), this);

	UpdateView();
//	RenderPage(0);
}

BOOL CThumbnailsView::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void CThumbnailsView::OnSize(UINT nType, int cx, int cy)
{
	if (m_nPageCount != 0)
	{
		UpdateView();
	}

	CScrollView::OnSize(nType, cx, cy);
}

void CThumbnailsView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	int y = GetScrollPos(SB_VERT);
	int nScroll = 0;

	switch (nChar)
	{
	case VK_RIGHT:
		OnScrollBy(CSize(3*m_lineDev.cx, 0));
		return;

	case VK_LEFT:
		OnScrollBy(CSize(-3*m_lineDev.cx, 0));
		return;

	case VK_DOWN:
		nScroll = 3*m_lineDev.cy;
		break;

	case VK_UP:
		nScroll = -3*m_lineDev.cy;
		break;

	case VK_NEXT:
	case VK_SPACE:
		nScroll = m_pageDev.cy;
		break;

	case VK_PRIOR:
	case VK_BACK:
		nScroll = -m_pageDev.cy;
		break;
	}

	OnScrollBy(CSize(0, nScroll));
//	UpdateVisiblePages();

	CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CThumbnailsView::OnSetFocus(CWnd* pOldWnd)
{
	CScrollView::OnSetFocus(pOldWnd);
}

void CThumbnailsView::OnKillFocus(CWnd* pNewWnd)
{
	CScrollView::OnKillFocus(pNewWnd);
}

void CThumbnailsView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rcClient;
	GetClientRect(rcClient);

	CScrollView::OnLButtonDown(nFlags, point);
}

void CThumbnailsView::OnContextMenu(CWnd* pWnd, CPoint point)
{
#ifndef ELIBRA_READER
	if (point.x < 0 || point.y < 0)
		point = CPoint(0, 0);

	CRect rcClient;
	GetClientRect(rcClient);
	ClientToScreen(rcClient);

	if (!rcClient.PtInRect(point))
	{
		CScrollView::OnContextMenu(pWnd, point);
		return;
	}

	ScreenToClient(&point);
//	m_nClickedPage = GetPageFromPoint(point);
//	if (m_nClickedPage == -1)
//		return;

	CMenu menu;
	menu.LoadMenu(IDR_POPUP);

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	ClientToScreen(&point);
	pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y,
		GetMainFrame());
#endif
}

BOOL CThumbnailsView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
		CRect rcClient;
		GetClientRect(rcClient);

		int nCode = HIBYTE(nScrollCode);
		if (nCode == SB_THUMBTRACK)
		{
			SCROLLINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cbSize = sizeof(si);

			if (!GetScrollInfo(SB_VERT, &si, SIF_TRACKPOS))
				return true;

			int yOrig = GetScrollPos(SB_VERT);

			BOOL bResult = OnScrollBy(CSize(0, si.nTrackPos - yOrig), bDoScroll);
			if (bResult && bDoScroll)
			{
//				UpdateVisiblePages();
				UpdateWindow();
			}

			return bResult;
		}

	BOOL bResult = CScrollView::OnScroll(nScrollCode, nPos, bDoScroll);
//	UpdateVisiblePages();

	return bResult;
}

BOOL CThumbnailsView::OnMouseWheel(UINT nFlags, short zDelta, CPoint /*point*/)
{
	// we don't handle anything but scrolling
	if ((nFlags & MK_CONTROL) != 0)
		return false;

	BOOL bHasHorzBar, bHasVertBar;
	CheckScrollBars(bHasHorzBar, bHasVertBar);

	BOOL bResult = false;
	UINT uWheelScrollLines = GetMouseScrollLines();
	int nToScroll = ::MulDiv(-zDelta, uWheelScrollLines, WHEEL_DELTA);
	int nDisplacement;

	bool bScrollPages = true;

	if (bHasVertBar && (!bHasHorzBar || (nFlags & MK_SHIFT) == 0))
	{
		if (uWheelScrollLines == WHEEL_PAGESCROLL)
		{
			nDisplacement = m_pageDev.cy;
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
		}
		else
		{
			nDisplacement = nToScroll * m_lineDev.cy;
			nDisplacement = min(nDisplacement, m_pageDev.cy);
		}

		bResult = OnScrollBy(CSize(0, nDisplacement));
		bScrollPages = false;

//		UpdateVisiblePages();
	}
	else if (bHasHorzBar)
	{
		if (uWheelScrollLines == WHEEL_PAGESCROLL)
		{
			nDisplacement = m_pageDev.cx;
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
		}
		else
		{
			nDisplacement = nToScroll * m_lineDev.cx;
			nDisplacement = min(nDisplacement, m_pageDev.cx);
		}

		bResult = OnScrollBy(CSize(nDisplacement, 0));
		bScrollPages = false;
	}

	if (bResult)
		UpdateWindow();

	return bResult;
}
/*
void CThumbnailsView::InvalidatePage(int nPage)
{
	ASSERT(nPage >= 0 && nPage <= m_nPageCount);

	CRect rect;
	GetClientRect(rect);

	int y = GetScrollPos(SB_VERT);
	rect.top = max(rect.top, m_pages[nPage].rcDisplay.top - y);
	rect.bottom = min(rect.bottom, m_pages[nPage].rcDisplay.bottom - y);

	if (!rect.IsRectEmpty())
		InvalidateRect(rect);
}
*/

int CThumbnailsView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
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

void CThumbnailsView::UpdateView(/*UpdateType updateType*/)
{
	if (m_bInsideUpdateView)
		return;

	m_bInsideUpdateView = true;

/*
	// Save page and offset to restore after changes
	int nAnchorPage;
	CPoint ptAnchorOffset;
	CPoint ptTop = GetDeviceScrollPosition();

	if (updateType == TOP)
	{
		nAnchorPage = m_nPage;
		ptAnchorOffset = ptTop - m_pages[m_nPage].ptOffset;
	}
	else if (updateType == BOTTOM)
	{
		CPoint ptBottom = ptTop + rcClient.Size();

		int nPage = m_nPage;
		while (nPage < m_nPageCount - 1 &&
			ptBottom.y > m_pages[nPage].rcDisplay.bottom)
			++nPage;

		nAnchorPage = nPage;
		ptAnchorOffset = ptBottom - m_pages[nPage].ptOffset;
	}
*/
	int nThumbnailWidth = 2*nHorzMargin + 2*nFrameWidth + nPageWidth;
	int nThumbnailHeight = 2*nVertMargin + 2*nFrameWidth + nPageHeight
			+ nNumberSkip + nNumberHeight;

	for (int i = 0; i < 2; ++i)
	{
		CRect rcClient;
		GetClientRect(rcClient);

		int nPagesInRow = max(1, (rcClient.Width() - 2*nPadding) / nThumbnailWidth);
		int nRowCount = (m_nPageCount - 1) / nPagesInRow + 1;

		m_szDisplay = CSize(max(rcClient.Width(), nPagesInRow*nThumbnailWidth + 2*nPadding),
			nThumbnailHeight*nRowCount + 2*nPadding);

		int nOffsetX = (rcClient.Width() - 2*nPadding - nThumbnailWidth*nRowCount) / nPagesInRow;
		if (nOffsetX < 0)
			nOffsetX = 0;

		for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		{
			int nRow = nPage / nPagesInRow;
			int nCol = nPage % nPagesInRow;

			Page& page = m_pages[nPage];
			page.ptOffset.x = nPadding + nCol*nThumbnailWidth + nOffsetX;
			page.ptOffset.y = nPadding + nRow*nThumbnailHeight;

			page.szTotal = CSize(nThumbnailWidth, nThumbnailHeight);
			page.rcPage = CRect(CPoint(page.ptOffset.x + nHorzMargin + nFrameWidth,
				page.ptOffset.y + nVertMargin + nFrameWidth), CSize(nPageWidth, nPageHeight));

			page.rcNumber = CRect(CPoint(page.rcPage.CenterPoint().x - nNumberWidth / 2,
				page.rcPage.bottom + nFrameWidth + nNumberSkip), CSize(nNumberWidth, nNumberHeight));
		}

		CSize szDevPage(rcClient.Width()*3/4, rcClient.Height()*3/4);
		CSize szDevLine(15, 15);

		SetScrollSizes(MM_TEXT, m_szDisplay, szDevPage, szDevLine);
	}

/*
	if (updateType == TOP)
	{
		ScrollToPositionNoRepaint(m_pages[nAnchorPage].ptOffset + ptAnchorOffset);
	}
	else if (updateType == BOTTOM)
	{
		ScrollToPositionNoRepaint(m_pages[nAnchorPage].ptOffset + ptAnchorOffset - rcClient.Size());
	}

	if (updateType != RECALC)
	{
		UpdateVisiblePages();
	}
*/
	m_bInsideUpdateView = false;
}
