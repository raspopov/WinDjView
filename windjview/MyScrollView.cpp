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
#include "MyScrollView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMyScrollView

IMPLEMENT_DYNAMIC(CMyScrollView, CScrollView)

BEGIN_MESSAGE_MAP(CMyScrollView, CScrollView)
END_MESSAGE_MAP()

// CMyScrollView construction/destruction

CMyScrollView::CMyScrollView()
{
}

CMyScrollView::~CMyScrollView()
{
}

void CMyScrollView::SetScrollSizesNoRepaint(const CSize& szTotal,
	const CSize& szPage, const CSize& szLine)
{
	ASSERT(szTotal.cx >= 0 && szTotal.cy >= 0);

	m_nMapMode = MM_TEXT;
	m_totalLog = szTotal;

	m_totalDev = m_totalLog;
	m_pageDev = szPage;
	m_lineDev = szLine;
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);

	if (m_hWnd != NULL)
	{
		// window has been created, invalidate now
		UpdateBarsNoRepaint();
	}
}

void CMyScrollView::SetScrollSizes(const CSize& szTotal, const CSize& szPage, const CSize& szLine)
{
	SetScrollSizesNoRepaint(szTotal, szPage, szLine);

	if (m_hWnd != NULL)
	{
		// window has been created, invalidate now
		UpdateBars();
	}
}

void CMyScrollView::UpdateBarsNoRepaint()
{
	// update the horizontal to reflect reality
	// NOTE: turning on/off the scrollbars will cause 'OnSize' callbacks
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);

	CRect rectClient;
	bool bCalcClient = true;

	// allow parent to do inside-out layout first
	CWnd* pParentWnd = GetParent();
	if (pParentWnd != NULL)
	{
		// if parent window responds to this message, use just
		//  client area for scroll bar calc -- not "true" client area
		if (pParentWnd->SendMessage(WM_RECALCPARENT, 0,
			(LPARAM)(LPCRECT)&rectClient) != 0)
		{
			// use rectClient instead of GetTrueClientSize for
			//  client size calculation.
			bCalcClient = false;
		}
	}

	CSize sizeClient;
	CSize sizeSb;

	if (bCalcClient)
	{
		// get client rect
		if (!GetTrueClientSize(sizeClient, sizeSb))
		{
			// no room for scroll bars (common for zero sized elements)
			CRect rect;
			GetClientRect(&rect);
			if (rect.right > 0 && rect.bottom > 0)
			{
				// if entire client area is not invisible, assume we have
				//  control over our scrollbars
				EnableScrollBarCtrl(SB_BOTH, false);
			}
			return;
		}
	}
	else
	{
		// let parent window determine the "client" rect
		GetScrollBarSizes(sizeSb);
		sizeClient.cx = rectClient.right - rectClient.left;
		sizeClient.cy = rectClient.bottom - rectClient.top;
	}

	// enough room to add scrollbars
	CSize sizeRange;
	CPoint ptMove;
	CSize needSb;

	// get the current scroll bar state given the true client area
	GetScrollBarState(sizeClient, needSb, sizeRange, ptMove, bCalcClient);
	if (needSb.cx)
		sizeClient.cy -= sizeSb.cy;
	if (needSb.cy)
		sizeClient.cx -= sizeSb.cx;

	// first scroll the window as needed
	SetScrollPos(SB_HORZ, ptMove.x);
	SetScrollPos(SB_VERT, ptMove.y);

	// this structure needed to update the scrollbar page range
	SCROLLINFO info;
	info.fMask = SIF_PAGE|SIF_RANGE;
	info.nMin = 0;

	// now update the bars as appropriate
	EnableScrollBarCtrl(SB_HORZ, needSb.cx);
	if (needSb.cx)
	{
		info.nPage = sizeClient.cx;
		info.nMax = m_totalDev.cx-1;
		if (!SetScrollInfo(SB_HORZ, &info, true))
			SetScrollRange(SB_HORZ, 0, sizeRange.cx, true);
	}

	EnableScrollBarCtrl(SB_VERT, needSb.cy);
	if (needSb.cy)
	{
		info.nPage = sizeClient.cy;
		info.nMax = m_totalDev.cy-1;
		if (!SetScrollInfo(SB_VERT, &info, true))
			SetScrollRange(SB_VERT, 0, sizeRange.cy, true);
	}
}

void CMyScrollView::ScrollToPositionNoRepaint(CPoint pt)
{
	ASSERT(m_nMapMode > 0);     // not allowed for shrink to fit
	if (m_nMapMode != MM_TEXT)
	{
		CWindowDC dc(NULL);
		dc.SetMapMode(m_nMapMode);
		dc.LPtoDP((LPPOINT)&pt);
	}

	// now in device coordinates - limit if out of range
	int xMax = GetScrollLimit(SB_HORZ);
	int yMax = GetScrollLimit(SB_VERT);

	BOOL bHasHorzBar, bHasVertBar;
	CheckScrollBars(bHasHorzBar, bHasVertBar);

	if (!bHasHorzBar)
		xMax = 0;
	if (!bHasVertBar)
		yMax = 0;

	if (pt.x < 0)
		pt.x = 0;
	else if (pt.x > xMax)
		pt.x = xMax;
	if (pt.y < 0)
		pt.y = 0;
	else if (pt.y > yMax)
		pt.y = yMax;

	SetScrollPos(SB_HORZ, pt.x);
	SetScrollPos(SB_VERT, pt.y);
}

void CMyScrollView::CheckScrollBars(BOOL& bHasHorzBar, BOOL& bHasVertBar) const
{
	DWORD dwStyle = GetStyle();
	CScrollBar* pBar = GetScrollBarCtrl(SB_VERT);
	bHasVertBar = (pBar != NULL && pBar->IsWindowEnabled())
			|| (dwStyle & WS_VSCROLL) != 0;

	pBar = GetScrollBarCtrl(SB_HORZ);
	bHasHorzBar = (pBar != NULL && pBar->IsWindowEnabled())
			|| (dwStyle & WS_HSCROLL) != 0;
}
