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

#include "DjVuDoc.h"
#include "ThumbnailsView.h"
#include "MainFrm.h"
#include "Drawing.h"
#include "ThumbnailsThread.h"
#include "ChildFrm.h"
#include "DjVuView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


const int nVertMargin = 3;
const int nHorzMargin = 3;
const int nFrameWidth = 1;
const int nPadding = 3;
const int nNumberSkip = 2;
const int nNumberHeight = 15;
const int nNumberWidth = 30;
const int nPageWidth = 100;
const int nPageHeight = 110;


// CThumbnailsView

IMPLEMENT_DYNAMIC(CThumbnailsView, CMyScrollView)

BEGIN_MESSAGE_MAP(CThumbnailsView, CMyScrollView)
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
	ON_MESSAGE(WM_RENDER_THUMB_FINISHED, OnRenderFinished)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// CThumbnailsView construction/destruction

CThumbnailsView::CThumbnailsView()
	: m_bInsideUpdateView(false), m_nPageCount(0), m_bVisible(false),
	  m_pThread(NULL), m_pIdleThread(NULL), m_nSelectedPage(-1), m_pDoc(NULL)
{
	CFont systemFont;
	CreateSystemDialogFont(systemFont);

	LOGFONT lf;
	::GetObject(systemFont.m_hObject, sizeof(LOGFONT), &lf);

	strcpy(lf.lfFaceName, "Arial");
	lf.lfHeight = -12;
	m_font.CreateFontIndirect(&lf);
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

	CRect rcClip;
	pDC->GetClipBox(rcClip);
	rcClip.OffsetRect(GetDeviceScrollPosition());

	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		Page& page = m_pages[nPage];

		if (page.rcDisplay.top < rcClip.bottom &&
			page.rcDisplay.bottom > rcClip.top)
		{
			DrawPage(pDC, nPage);
		}
	}
}

void CThumbnailsView::DrawPage(CDC* pDC, int nPage)
{
	Page& page = m_pages[nPage];

	COLORREF clrWindow = ::GetSysColor(COLOR_WINDOW);
	COLORREF clrFrame = ::GetSysColor(COLOR_WINDOWFRAME);
	COLORREF clrShadow = ::GetSysColor(COLOR_BTNSHADOW);
	COLORREF clrHilight = ::GetSysColor(COLOR_HIGHLIGHT);
	static CPen penFrame(PS_SOLID, 1, clrFrame);

	CPoint ptScrollPos = GetDeviceScrollPosition();

	CRect rcClip;
	pDC->GetClipBox(rcClip);
	rcClip.OffsetRect(ptScrollPos);
	rcClip.top = max(rcClip.top, page.rcDisplay.top);
	rcClip.bottom = min(rcClip.bottom, page.rcDisplay.bottom);

	if (rcClip.IsRectEmpty())
		return;

	CRect rcPage(page.rcPage);
	CRect rcCorner;

	if (page.pBitmap != NULL && page.pBitmap->m_hObject != NULL)
	{
		CPoint ptOffset = rcPage.Size() - page.pBitmap->GetSize();
		rcPage = CRect(rcPage.TopLeft() + CPoint(ptOffset.x / 2, ptOffset.y / 2),
				page.pBitmap->GetSize());

		page.pBitmap->DrawDC(pDC, rcPage.TopLeft() - ptScrollPos);
	}
	else
	{
		pDC->FillSolidRect(rcPage - ptScrollPos, clrWindow);

		// Draw corner
		rcCorner = CRect(rcPage.TopLeft(), CSize(15, 15));
		rcCorner.OffsetRect(-1, -1);
		rcCorner.OffsetRect(-ptScrollPos);

		CPoint points[] = {
				CPoint(rcCorner.left, rcCorner.bottom - 1),
				CPoint(rcCorner.right - 1, rcCorner.top),
				CPoint(rcCorner.right - 1, rcCorner.bottom - 1),
				CPoint(rcCorner.left, rcCorner.bottom - 1) };

		CPen* pOldPen = pDC->SelectObject(&penFrame);
		pDC->Polyline(points, 4);
		pDC->SelectObject(pOldPen);

		CRect rcWhiteBar(CPoint(rcCorner.left, rcCorner.top), CSize(1, rcCorner.Height() - 1));
		pDC->FillSolidRect(rcWhiteBar, clrWindow);
		rcWhiteBar = CRect(CPoint(rcCorner.left, rcCorner.top), CSize(rcCorner.Width() - 1, 1));
		pDC->FillSolidRect(rcWhiteBar, clrWindow);
	}

	// Page border and shadow
	CRect rcBorder(rcPage);

	if (nPage == m_nSelectedPage)
	{
		for (int i = 0; i < 4; i++)
		{
			rcBorder.InflateRect(1, 1);
			FrameRect(pDC, rcBorder - ptScrollPos, clrHilight);
		}
	}
	else
	{
		rcBorder.InflateRect(1, 1);

		if (page.pBitmap != NULL && page.pBitmap->m_hObject != NULL)
		{
			FrameRect(pDC, rcBorder - ptScrollPos, clrFrame);
		}
		else
		{
			int nSaveDC = pDC->SaveDC();
			pDC->ExcludeClipRect(rcCorner);
			FrameRect(pDC, rcBorder - ptScrollPos, clrFrame);
			pDC->RestoreDC(nSaveDC);
		}
		
		CRect rcWhiteBar(CPoint(rcBorder.left, rcBorder.bottom), CSize(3, 1));
		pDC->FillSolidRect(rcWhiteBar - ptScrollPos, clrWindow);
		rcWhiteBar = CRect(CPoint(rcBorder.right, rcBorder.top), CSize(1, 3));
		pDC->FillSolidRect(rcWhiteBar - ptScrollPos, clrWindow);

		CRect rcShadow(CPoint(rcBorder.left + 3, rcBorder.bottom), CSize(rcBorder.Width() - 2, 1));
		pDC->FillSolidRect(rcShadow - ptScrollPos, clrShadow);
		rcShadow = CRect(CPoint(rcBorder.right, rcBorder.top + 3), CSize(1, rcBorder.Height() - 2));
		pDC->FillSolidRect(rcShadow - ptScrollPos, clrShadow);

		rcBorder.InflateRect(0, 0, 1, 1);
	}

	// Page number
	CRect rcNumberFrame = CRect(CPoint(rcPage.CenterPoint().x - nNumberWidth / 2,
		rcPage.bottom + nFrameWidth + nNumberSkip), CSize(nNumberWidth, nNumberHeight));
	FrameRect(pDC, rcNumberFrame - ptScrollPos, clrFrame);

	CRect rcNumber(rcNumberFrame);
	rcNumber.DeflateRect(1, 1);
	pDC->FillSolidRect(rcNumber - ptScrollPos, clrWindow);

	CString strPageNumber;
	strPageNumber.Format(_T("%d"), nPage + 1);
	CFont* pOldFont = pDC->SelectObject(&m_font);
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(strPageNumber, rcNumber - ptScrollPos,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pDC->SelectObject(pOldFont);

	if (nPage == m_nSelectedPage)
	{
		for (int i = 0; i < 3; i++)
		{
			rcNumberFrame.InflateRect(1, 1);
			FrameRect(pDC, rcNumberFrame - ptScrollPos, clrHilight);
		}
	}

	// Fill everything else with window background color
	int nSaveDC = pDC->SaveDC();
	pDC->ExcludeClipRect(rcBorder - ptScrollPos);
	pDC->ExcludeClipRect(rcNumberFrame - ptScrollPos);
	pDC->FillSolidRect(page.rcDisplay - ptScrollPos, clrWindow);
	pDC->RestoreDC(nSaveDC);
}

// CThumbnailsView diagnostics

#ifdef _DEBUG
void CThumbnailsView::AssertValid() const
{
	CMyScrollView::AssertValid();
}

void CThumbnailsView::Dump(CDumpContext& dc) const
{
	CMyScrollView::Dump(dc);
}
#endif //_DEBUG


// CThumbnailsView message handlers

void CThumbnailsView::OnInitialUpdate()
{
	CMyScrollView::OnInitialUpdate();

	m_nPageCount = GetDocument()->GetPageCount();
	m_pages.resize(m_nPageCount);

	m_pThread = new CThumbnailsThread(GetDocument(), this);
	m_pThread->SetThumbnailSize(CSize(nPageWidth, nPageHeight));

	m_pIdleThread = new CThumbnailsThread(GetDocument(), this, true);
	m_pIdleThread->SetThumbnailSize(CSize(nPageWidth, nPageHeight));

	UpdateView(RECALC);
}

BOOL CThumbnailsView::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void CThumbnailsView::OnSize(UINT nType, int cx, int cy)
{
	if (m_nPageCount != 0)
	{
		UpdateView(TOP);
	}

	CMyScrollView::OnSize(nType, cx, cy);
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
	UpdateVisiblePages();

	CMyScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CThumbnailsView::OnSetFocus(CWnd* pOldWnd)
{
	CMyScrollView::OnSetFocus(pOldWnd);
}

void CThumbnailsView::OnKillFocus(CWnd* pNewWnd)
{
	CMyScrollView::OnKillFocus(pNewWnd);
}

void CThumbnailsView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rcClient;
	GetClientRect(rcClient);

	int nPage = GetPageFromPoint(point);
	if (nPage != m_nSelectedPage)
	{
		if (m_nSelectedPage != -1)
			InvalidatePage(m_nSelectedPage);

		m_nSelectedPage = nPage;

		if (m_nSelectedPage != -1)
		{
			InvalidatePage(m_nSelectedPage);
			CDjVuView* pView = ((CChildFrame*)GetParentFrame())->GetDjVuView();
			pView->GoToPage(nPage, pView->GetCurrentPage());
		}

		UpdateWindow();
	}

	CMyScrollView::OnLButtonDown(nFlags, point);
}

void CThumbnailsView::OnContextMenu(CWnd* pWnd, CPoint point)
{
#ifndef ELIBRA_READER
/*	if (point.x < 0 || point.y < 0)
		point = CPoint(0, 0);

	CRect rcClient;
	GetClientRect(rcClient);
	ClientToScreen(rcClient);

	if (!rcClient.PtInRect(point))
	{
		CMyScrollView::OnContextMenu(pWnd, point);
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
*/
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
				UpdateVisiblePages();
				UpdateWindow();
			}

			return bResult;
		}

	BOOL bResult = CMyScrollView::OnScroll(nScrollCode, nPos, bDoScroll);
	UpdateVisiblePages();

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

		UpdateVisiblePages();
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

bool CThumbnailsView::InvalidatePage(int nPage)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);

	CRect rect;
	GetClientRect(rect);

	CPoint ptScrollPos = GetDeviceScrollPosition();
	if (rect.IntersectRect(rect, m_pages[nPage].rcDisplay - ptScrollPos))
	{
		InvalidateRect(rect);
		return true;
	}

	return false;
}

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

void CThumbnailsView::UpdateView(UpdateType updateType)
{
	if (m_bInsideUpdateView)
		return;

	m_bInsideUpdateView = true;

	// Save page and offset to restore after changes
	int nAnchorPage;
	CPoint ptAnchorOffset;
	CPoint ptTop = GetDeviceScrollPosition();

	if (updateType == TOP)
	{
		int nTopPage = 0;
		while (nTopPage < m_nPageCount - 1 &&
				ptTop.y >= m_pages[nTopPage].rcDisplay.bottom)
			++nTopPage;

		nAnchorPage = nTopPage;
		ptAnchorOffset = ptTop - m_pages[nTopPage].rcPage.TopLeft();
	}

	int nThumbnailWidth = 2*nHorzMargin + 2*nFrameWidth + nPageWidth;
	int nThumbnailHeight = 2*nVertMargin + 2*nFrameWidth + nPageHeight
			+ nNumberSkip + nNumberHeight;

	for (int i = 0; i < 2; ++i)
	{
		CRect rcClient;
		GetClientRect(rcClient);

		int nPagesInRow = max(1, (rcClient.Width() - 2*nPadding) / nThumbnailWidth);
		nPagesInRow = min(nPagesInRow, m_nPageCount);
		int nRowCount = (m_nPageCount - 1) / nPagesInRow + 1;

		m_szDisplay = CSize(max(rcClient.Width(), nPagesInRow*nThumbnailWidth + 2*nPadding),
			max(rcClient.Height(), nThumbnailHeight*nRowCount + 2*nPadding));

		double nOffsetX = (rcClient.Width() - 2*nPadding - nThumbnailWidth*nPagesInRow) / (2.0*nPagesInRow);
		if (nOffsetX < 0)
			nOffsetX = 0;

		for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		{
			int nRow = nPage / nPagesInRow;
			int nCol = nPage % nPagesInRow;

			Page& page = m_pages[nPage];

			CPoint ptOffset(nPadding + nCol*nThumbnailWidth + static_cast<int>(2*nCol*nOffsetX),
					nPadding + nRow*nThumbnailHeight);
			int nWidthExtra = static_cast<int>(2*(nCol + 1)*nOffsetX) - static_cast<int>(2*nCol*nOffsetX);
			page.rcDisplay = CRect(ptOffset, CSize(nThumbnailWidth + nWidthExtra, nThumbnailHeight));

			ptOffset.Offset(static_cast<int>(nOffsetX), 0);

			page.rcPage = CRect(CPoint(ptOffset.x + nHorzMargin + nFrameWidth,
				ptOffset.y + nVertMargin + nFrameWidth), CSize(nPageWidth, nPageHeight));

			if (nCol == 0)
				page.rcDisplay.left = 0;
			if (nCol == nPagesInRow - 1)
				page.rcDisplay.right = m_szDisplay.cx;
			if (nRow == 0)
				page.rcDisplay.top = 0;
			if (nRow == nRowCount - 1)
			{
				page.rcDisplay.bottom = m_szDisplay.cy;
				if (nPage == m_nPageCount - 1 && nCol != nPagesInRow - 1)
					page.rcDisplay.right = m_szDisplay.cx;
			}
		}

		CSize szDevPage(rcClient.Width()*3/4, rcClient.Height()*3/4);
		CSize szDevLine(15, 15);

		SetScrollSizesNoRepaint(m_szDisplay, szDevPage, szDevLine);
	}

	if (updateType == TOP)
	{
		ScrollToPositionNoRepaint(m_pages[nAnchorPage].rcPage.TopLeft() + ptAnchorOffset);
	}

	m_bInsideUpdateView = false;
	UpdateVisiblePages();
}

void CThumbnailsView::OnDestroy()
{
	delete m_pThread;
	m_pThread = NULL;

	delete m_pIdleThread;
	m_pIdleThread = NULL;

	CMyScrollView::OnDestroy();
}

void CThumbnailsView::UpdateVisiblePages()
{
	if (m_pThread == NULL || m_pIdleThread == NULL)
		return;

	CRect rcClient;
	GetClientRect(rcClient);
	int nTop = GetScrollPos(SB_VERT);

	m_pThread->ClearQueue();
	m_pIdleThread->ClearQueue();

	if (m_bVisible)
	{
		m_pThread->PauseJobs();
		m_pIdleThread->PauseJobs();

		int nTopPage = 0;
		while (nTopPage < m_nPageCount - 1 &&
				nTop >= m_pages[nTopPage].rcDisplay.bottom)
			++nTopPage;

		int nBottomPage = nTopPage + 1;
		while (nBottomPage < m_nPageCount &&
				m_pages[nBottomPage].rcDisplay.top < nTop + rcClient.Height())
			++nBottomPage;

		for (int nDiff = m_nPageCount; nDiff >= 1; --nDiff)
		{
			if (nTopPage - nDiff >= 0)
				UpdatePage(nTopPage - nDiff, m_pIdleThread);
			if (nBottomPage + nDiff - 1 < m_nPageCount)
				UpdatePage(nBottomPage + nDiff - 1, m_pIdleThread);
		}

		for (int nPage = nBottomPage - 1; nPage >= nTopPage; --nPage)
			UpdatePage(nPage, m_pThread);

		m_pThread->ResumeJobs();
		m_pIdleThread->ResumeJobs();
	}
}

void CThumbnailsView::UpdatePage(int nPage, CThumbnailsThread* pThread)
{
	Page& page = m_pages[nPage];

	if (page.pBitmap == NULL)
	{
		pThread->AddJob(nPage, 0);
		InvalidatePage(nPage);
	}
}

LRESULT CThumbnailsView::OnRenderFinished(WPARAM wParam, LPARAM lParam)
{
	int nPage = (int)wParam;

	Page& page = m_pages[nPage];
	CDIB* pBitmap = reinterpret_cast<CDIB*>(lParam);

	if (page.pBitmap != NULL && page.rcPage.Size() == page.pBitmap->GetSize())
	{
		// Bitmap is too old, ignore it
		delete pBitmap;
		return 0;
	}

	page.DeleteBitmap();
	page.pBitmap = pBitmap;

	if (InvalidatePage(nPage))
		UpdateWindow();

	return 0;
}

void CThumbnailsView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CMyScrollView::OnShowWindow(bShow, nStatus);

	m_bVisible = !!bShow;
	UpdateVisiblePages();
}

int CThumbnailsView::GetPageFromPoint(CPoint point)
{
	point += GetScrollPosition();

	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		if (m_pages[nPage].rcPage.PtInRect(point))
			return nPage;
	}

	return -1;
}
