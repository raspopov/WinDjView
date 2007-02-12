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
#include "MyScrollView.h"
#include "FullscreenWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// From MFC's _AFX_MOUSEANCHORWND
class CMyAnchorWnd : public CWnd
{
private:
	using CWnd::Create;

public:
	CMyAnchorWnd();
	~CMyAnchorWnd();

	BOOL Create(CMyScrollView* pView);
	void Show(const CPoint& ptAnchor, bool bVertScroll, bool bHorzScroll);
	void Hide();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	CPoint m_ptAnchor;
	int m_nDefaultCursor;
	bool m_bVertScroll, m_bHorzScroll;
	bool m_bQuitTracking;
	CRect m_rectDrag;
	CMyScrollView* m_pView;
	CBitmap m_bitmap;

	static void InitCursors();
	static HCURSOR s_anchorCursors[11];

	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()
};

const int s_nAnchorSize = 32;

HCURSOR CMyAnchorWnd::s_anchorCursors[11] = {0};


// CMyScrollView

IMPLEMENT_DYNAMIC(CMyScrollView, CScrollView)

BEGIN_MESSAGE_MAP(CMyScrollView, CScrollView)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_MBUTTONDOWN, OnMButtonDown)
	ON_WM_CANCELMODE()
END_MESSAGE_MAP()

// CMyScrollView construction/destruction

CMyScrollView::CMyScrollView()
	: m_pAnchorWnd(NULL)
{
	m_nMapMode = MM_TEXT;
}

CMyScrollView::~CMyScrollView()
{
}

void CMyScrollView::OnDestroy()
{
	if (m_pAnchorWnd != NULL)
	{
		m_pAnchorWnd->DestroyWindow();
		m_pAnchorWnd = NULL;
	}

	CScrollView::OnDestroy();
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

LRESULT CMyScrollView::OnMButtonDown(WPARAM wParam, LPARAM lParam)
{
	// From MFC's CScrollView::HandleMButtonDown

	UINT nFlags = static_cast<UINT>(wParam);
	CPoint point(lParam);

	// If the user has CTRL or SHIFT down, we do not handle the message
	if (nFlags & (MK_SHIFT | MK_CONTROL))
	{
		CScrollView::OnMButtonDown(nFlags, point);
		return false;
	}

	if (m_pAnchorWnd == NULL)
	{
		m_pAnchorWnd = new CMyAnchorWnd();
		m_pAnchorWnd->Create(this);
	}

	if (!m_pAnchorWnd->IsWindowVisible())
	{
		BOOL bVertBar;
		BOOL bHorzBar;
		CheckScrollBars(bHorzBar, bVertBar);

		if (bHorzBar || bVertBar)
			m_pAnchorWnd->Show(point, !!bVertBar, !!bHorzBar);
	}
	else
	{
		m_pAnchorWnd->Hide();
	}
	
	return true;
}

BOOL CMyScrollView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		if (m_pAnchorWnd != NULL && m_pAnchorWnd->IsWindowVisible())
		{
			m_pAnchorWnd->Hide();
			return true;
		}
	}

	return CScrollView::PreTranslateMessage(pMsg);
}

void CMyScrollView::OnStartPan()
{
}

void CMyScrollView::OnPan(CSize szScroll)
{
	OnScrollBy(szScroll, true);
}

void CMyScrollView::OnEndPan()
{
}

void CMyScrollView::OnCancelMode()
{
	CScrollView::OnCancelMode();

	if (m_pAnchorWnd != NULL && m_pAnchorWnd->IsWindowVisible())
		m_pAnchorWnd->Hide();
}


// CMyAnchorWnd

BEGIN_MESSAGE_MAP(CMyAnchorWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP()

CMyAnchorWnd::CMyAnchorWnd()
	: m_pView(NULL), m_bQuitTracking(false), m_bVertScroll(false),
	  m_bHorzScroll(false)
{
	m_bitmap.LoadBitmap(IDB_PAN_ANCHOR);
}

CMyAnchorWnd::~CMyAnchorWnd()
{
}

BOOL CMyAnchorWnd::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	// Any of these messages cause us to quit scrolling
	case WM_MOUSEWHEEL:
	case WM_KEYDOWN:
	case WM_CHAR:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
		m_bQuitTracking = true;
		return true;

	// Button up message depend on the position of cursor
	// This enables the user to click and drag for a quick pan.
	case WM_MBUTTONUP:
		{
			CPoint pt(pMsg->lParam);
			ClientToScreen(&pt);
			if (!m_rectDrag.PtInRect(pt))
				m_bQuitTracking = true;
		}
		return true;
	}

	return false;
}

void CMyAnchorWnd::InitCursors()
{
	if (s_anchorCursors[0] != NULL)
		return;

	s_anchorCursors[0] = AfxGetApp()->LoadCursor(IDC_CURSOR_PAN_UL);
	s_anchorCursors[1] = AfxGetApp()->LoadCursor(IDC_CURSOR_PAN_U);
	s_anchorCursors[2] = AfxGetApp()->LoadCursor(IDC_CURSOR_PAN_UR);
	s_anchorCursors[3] = AfxGetApp()->LoadCursor(IDC_CURSOR_PAN_L);
	s_anchorCursors[4] = AfxGetApp()->LoadCursor(IDC_CURSOR_PAN_ALL);
	s_anchorCursors[5] = AfxGetApp()->LoadCursor(IDC_CURSOR_PAN_R);
	s_anchorCursors[6] = AfxGetApp()->LoadCursor(IDC_CURSOR_PAN_DL);
	s_anchorCursors[7] = AfxGetApp()->LoadCursor(IDC_CURSOR_PAN_D);
	s_anchorCursors[8] = AfxGetApp()->LoadCursor(IDC_CURSOR_PAN_DR);
	s_anchorCursors[9] = AfxGetApp()->LoadCursor(IDC_CURSOR_PAN_UD);
	s_anchorCursors[10] = AfxGetApp()->LoadCursor(IDC_CURSOR_PAN_LR);
}

void CMyAnchorWnd::OnTimer(UINT nIDEvent)
{
	if (nIDEvent != 1)
	{
		CWnd::OnTimer(nIDEvent);
		return;
	}

	if (m_pView == NULL)
	{
		Hide();
		return;
	}

	CPoint ptNow;
	GetCursorPos(&ptNow);

	CRect rectClient;
	GetWindowRect(rectClient);

	// Decide where the relative position of the cursor is and
	// pick a cursor that points where we're going
	int nCursor = 4;
	InitCursors();

	if (m_bVertScroll)
	{
		if (ptNow.y < rectClient.top)
			nCursor = 1;
		else if (ptNow.y > rectClient.bottom)
			nCursor = 7;
	}

	if (m_bHorzScroll)
	{
		if (ptNow.x < rectClient.left)
			nCursor -= 1;
		else if (ptNow.x > rectClient.right)
			nCursor += 1;
	}

	if (nCursor == 4)
		nCursor = m_nDefaultCursor;

	if (m_bQuitTracking)
	{
		Hide();
	}
	else if (nCursor == m_nDefaultCursor)
	{
		// The cursor is over the anchor window; use a cursor that
		// looks like the anchor bitmap.
		SetCursor(s_anchorCursors[nCursor]);
	}
	else
	{
		// Set cursor and do the tracking
		SetCursor(s_anchorCursors[nCursor]);

		// Calculate scroll distance
		CSize szDistance(0, 0);

		if (ptNow.x > rectClient.right)
			szDistance.cx = ptNow.x - rectClient.right;
		else if (ptNow.x < rectClient.left)
			szDistance.cx = ptNow.x - rectClient.left;

		if (ptNow.y > rectClient.bottom)
			szDistance.cy = ptNow.y - rectClient.bottom;
		else if (ptNow.y < rectClient.top)
			szDistance.cy = ptNow.y - rectClient.top;

		if (!m_bHorzScroll)
			szDistance.cx = 0;
		if (!m_bVertScroll)
			szDistance.cy = 0;

		int nSignX = szDistance.cx > 0 ? 1 : szDistance.cx < 0 ? -1 : 0;
		int nSignY = szDistance.cy > 0 ? 1 : szDistance.cy < 0 ? -1 : 0;

		int nScrollX, nScrollY;
		if (fabs(1.0*szDistance.cx) < 160)
			nScrollX = static_cast<int>(fabs(1.0*szDistance.cx) / 8);
		else
			nScrollX = 20 + static_cast<int>(pow(fabs(1.0*szDistance.cx) - 160, 2) / 16);

		if (fabs(1.0*szDistance.cy) < 160)
			nScrollY = static_cast<int>(fabs(1.0*szDistance.cy) / 8);
		else
			nScrollY = 20 + static_cast<int>(pow(fabs(1.0*szDistance.cy) - 160, 2) / 16);

		CSize szScroll(nSignX * nScrollX, nSignY * nScrollY);
		m_pView->OnPan(szScroll);
	}
}

BOOL CMyAnchorWnd::Create(CMyScrollView* pView)
{
	static CString strWndClass = AfxRegisterWndClass(CS_DBLCLKS,
			::LoadCursor(NULL, IDC_ARROW));

	m_pView = pView;

	BOOL bRetVal = CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, strWndClass, NULL, WS_POPUP,
		CRect(0, 0, s_nAnchorSize, s_nAnchorSize), m_pView, 0);

	if (bRetVal)
	{
		CDC dcScreen;
		dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

		CDC dc;
		dc.CreateCompatibleDC(&dcScreen);
		CBitmap* pOldBitmap = dc.SelectObject(&m_bitmap);

		CRgn rgn;
		rgn.CreateRectRgn(0, 0, 0, 0);

		// Create window region from anchor bitmap
		for (int y = 0; y < s_nAnchorSize; ++y)
		{
			int x = 0;
			while (x < s_nAnchorSize && dc.GetPixel(x, y) == RGB(192, 192, 192))
				++x;

			if (x == s_nAnchorSize)
				continue;

			int x2 = x + 1;
			while (x2 < s_nAnchorSize && dc.GetPixel(x2, y) != RGB(192, 192, 192))
				++x2;

			CRgn line;
			line.CreateRectRgn(x, y, x2, y + 1);
			rgn.CombineRgn(&rgn, &line, RGN_OR);
		}

		dc.SelectObject(pOldBitmap);

		SetWindowRgn(rgn, false);
	}

	return bRetVal;
}

void CMyAnchorWnd::Show(const CPoint& ptAnchor, bool bVertScroll, bool bHorzScroll)
{
	ASSERT(m_pView != NULL);

	m_ptAnchor = ptAnchor;
	m_pView->ClientToScreen(&m_ptAnchor);

	ASSERT(bHorzScroll || bVertScroll);
	m_bVertScroll = bVertScroll;
	m_bHorzScroll = bHorzScroll;

	if (m_bVertScroll && m_bHorzScroll)
		m_nDefaultCursor = 4;
	else if (m_bVertScroll)
		m_nDefaultCursor = 9;
	else
		m_nDefaultCursor = 10;

	m_rectDrag.top = m_ptAnchor.y - GetSystemMetrics(SM_CYDOUBLECLK);
	m_rectDrag.bottom = m_ptAnchor.y + GetSystemMetrics(SM_CYDOUBLECLK);
	m_rectDrag.left = m_ptAnchor.x - GetSystemMetrics(SM_CXDOUBLECLK);
	m_rectDrag.right = m_ptAnchor.x + GetSystemMetrics(SM_CXDOUBLECLK);

	SetWindowPos(&wndTop, m_ptAnchor.x - s_nAnchorSize / 2,
		m_ptAnchor.y - s_nAnchorSize / 2, 0, 0,
		SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	m_pView->OnStartPan();

	m_bQuitTracking = false;
	SetCapture();
	SetTimer(1, 50, NULL);

	InitCursors();
	SetCursor(s_anchorCursors[m_nDefaultCursor]);
}

void CMyAnchorWnd::Hide()
{
	if (m_pView == NULL)
		return;

	KillTimer(1);
	ReleaseCapture();
	ShowWindow(SW_HIDE);

	m_pView->GetTopLevelParent()->UpdateWindow();

	m_pView->OnEndPan();
}

void CMyAnchorWnd::OnPaint()
{
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(&rect);

	CDC dcBitmap;
	dcBitmap.CreateCompatibleDC(&dc);
	CBitmap* pOldBitmap = dcBitmap.SelectObject(&m_bitmap);

	dc.BitBlt(0, 0, s_nAnchorSize, s_nAnchorSize, &dcBitmap, 0, 0, SRCCOPY);

	dcBitmap.SelectObject(pOldBitmap);

	// draw anchor shape
	InitCursors();
	HCURSOR hCursor = s_anchorCursors[m_nDefaultCursor];
	::DrawIconEx(dc.m_hDC, 0, 0, hCursor, 0, 0, 0, NULL, DI_NORMAL);
}

void CMyAnchorWnd::PostNcDestroy()
{
	// Should be created on heap
	delete this;
}
