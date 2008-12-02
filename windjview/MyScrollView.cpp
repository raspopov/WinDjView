//	WinDjView
//	Copyright (C) 2004-2008 Andrew Zhezherun
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

// $Id$

#include "stdafx.h"
#include "WinDjView.h"
#include "MyScrollView.h"
#include "FullscreenWnd.h"
#include "Drawing.h"

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
		UpdateBars(false);
}

void CMyScrollView::SetScrollSizes(const CSize& szTotal, const CSize& szPage, const CSize& szLine)
{
	ASSERT(szTotal.cx >= 0 && szTotal.cy >= 0);

	m_nMapMode = MM_TEXT;
	m_totalLog = szTotal;

	m_totalDev = m_totalLog;
	m_pageDev = szPage;
	m_lineDev = szLine;
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);

	if (m_hWnd != NULL)
		UpdateBars(true);
}

void CMyScrollView::GetScrollBarSizes(CSize& sizeSb) const
{
	// MFC Fix:  CScrollView::GetTrueClientSize

	sizeSb.cx = sizeSb.cy = 0;
	DWORD dwStyle = GetStyle();

	if (GetScrollBarCtrl(SB_VERT) == NULL)
	{
		// vert scrollbars will impact client area of this window
		sizeSb.cx = afxData.cxVScroll - CX_BORDER;  // MFC Fix
	}
	if (GetScrollBarCtrl(SB_HORZ) == NULL)
	{
		// horz scrollbars will impact client area of this window
		sizeSb.cy = afxData.cyHScroll - CY_BORDER;  // MFC Fix
	}
}

bool CMyScrollView::GetTrueClientSize(CSize& size, CSize& sizeSb) const
{
	// From MFC:  CScrollView::GetTrueClientSize

	size = ::GetClientSize(this);
	GetScrollBarSizes(sizeSb);

	DWORD dwStyle = GetStyle();

	if (sizeSb.cx != 0 && (dwStyle & WS_VSCROLL))
	{
		// vert scrollbars will impact client area of this window
		size.cx += sizeSb.cx;
	}
	if (sizeSb.cy != 0 && (dwStyle & WS_HSCROLL))
	{
		// horz scrollbars will impact client area of this window
		size.cy += sizeSb.cy;
	}

	// return true if enough room
	return (size.cx > sizeSb.cx && size.cy > sizeSb.cy);
}

bool CMyScrollView::AdjustClientSize(const CSize& szContent, CSize& szClient,
		bool& bHScroll, bool& bVScroll) const
{
	CSize sizeSb;
	GetScrollBarSizes(sizeSb);

	bool bChanged = false;
	if (szContent.cx > szClient.cx && sizeSb.cy != 0 && szClient.cy > sizeSb.cy && !bHScroll)
	{
		bHScroll = true;
		bChanged = true;
		szClient.cy -= sizeSb.cy;
	}

	if (szContent.cy > szClient.cy && sizeSb.cx != 0 && szClient.cx > sizeSb.cx && !bVScroll)
	{
		bVScroll = true;
		bChanged = true;
		szClient.cx -= sizeSb.cx;
	}

	if (szContent.cx > szClient.cx && sizeSb.cy != 0 && szClient.cy > sizeSb.cy && !bHScroll)
	{
		bHScroll = true;
		bChanged = true;
		szClient.cy -= sizeSb.cy;
	}

	return bChanged;
}

void CMyScrollView::GetScrollBarState(CSize sizeClient, CSize& needSb,
	CSize& sizeRange, CPoint& ptMove, bool bInsideClient) const
{
	// From MFC:  CScrollView::GetScrollBarState

	// get scroll bar sizes (the part that is in the client area)
	CSize sizeSb;
	GetScrollBarSizes(sizeSb);

	// enough room to add scrollbars
	sizeRange = m_totalDev - sizeClient;  // > 0 => need to scroll
	ptMove = GetDeviceScrollPosition();  // point to move to (start at current scroll pos)

	BOOL bNeedH = sizeRange.cx > 0;
	if (!bNeedH)
		ptMove.x = 0;  // jump back to origin
	else if (bInsideClient)
		sizeRange.cy += sizeSb.cy;  // need room for a scroll bar

	BOOL bNeedV = sizeRange.cy > 0;
	if (!bNeedV)
		ptMove.y = 0;  // jump back to origin
	else if (bInsideClient)
		sizeRange.cx += sizeSb.cx;  // need room for a scroll bar

	if (bNeedV && !bNeedH && sizeRange.cx > 0)
	{
		ASSERT(bInsideClient);
		// need a horizontal scrollbar after all
		bNeedH = TRUE;
		sizeRange.cy += sizeSb.cy;
	}

	// if current scroll position will be past the limit, scroll to limit
	if (sizeRange.cx > 0 && ptMove.x >= sizeRange.cx)
		ptMove.x = sizeRange.cx;
	if (sizeRange.cy > 0 && ptMove.y >= sizeRange.cy)
		ptMove.y = sizeRange.cy;

	// now update the bars as appropriate
	needSb.cx = bNeedH;
	needSb.cy = bNeedV;
}

void CMyScrollView::UpdateBars(bool bRepaint)
{
	// From MFC:  CScrollView::UpdateBars

	// UpdateBars may cause window to be resized - ignore those resizings
	if (m_bInsideUpdate)
		return;         // Do not allow recursive calls

	// Lock out recursion
	m_bInsideUpdate = true;

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
			CSize szClient = ::GetClientSize(this);
			if (szClient.cx > 0 && szClient.cy > 0)
			{
				// if entire client area is not invisible, assume we have
				//  control over our scrollbars
				EnableScrollBarCtrl(SB_BOTH, false);
			}
			m_bInsideUpdate = false;
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

	// hide scrollbars first to avoid redrawing
	if (!needSb.cx)
		EnableScrollBarCtrl(SB_HORZ, false);
	if (!needSb.cy)
		EnableScrollBarCtrl(SB_VERT, false);

	// first scroll the window as needed
	if (bRepaint)
	{
		ScrollToDevicePosition(ptMove);
	}
	else
	{
		SetScrollPos(SB_HORZ, ptMove.x);
		SetScrollPos(SB_VERT, ptMove.y);
	}

	// this structure needed to update the scrollbar page range
	SCROLLINFO info;
	info.fMask = SIF_PAGE|SIF_RANGE;
	info.nMin = 0;

	// update the scrollbar page range
	if (needSb.cx)
	{
		EnableScrollBarCtrl(SB_HORZ, true);
		info.nPage = sizeClient.cx;
		info.nMax = m_totalDev.cx-1;
		if (!SetScrollInfo(SB_HORZ, &info, true))
			SetScrollRange(SB_HORZ, 0, sizeRange.cx, true);
	}

	if (needSb.cy)
	{
		EnableScrollBarCtrl(SB_VERT, true);
		info.nPage = sizeClient.cy;
		info.nMax = m_totalDev.cy-1;
		if (!SetScrollInfo(SB_VERT, &info, true))
			SetScrollRange(SB_VERT, 0, sizeRange.cy, true);
	}

	// remove recursion lockout
	m_bInsideUpdate = false;
}

void CMyScrollView::ScrollToPositionNoRepaint(CPoint pt)
{
	ASSERT(m_nMapMode == MM_TEXT);

	BOOL bHasHorzBar, bHasVertBar;
	CheckScrollBars(bHasHorzBar, bHasVertBar);

	int xMax = bHasHorzBar ? GetScrollLimit(SB_HORZ) : 0;
	int yMax = bHasVertBar ? GetScrollLimit(SB_VERT) : 0;

	pt.x = max(0, min(xMax, pt.x));
	pt.y = max(0, min(yMax, pt.y));

	SetScrollPos(SB_HORZ, pt.x);
	SetScrollPos(SB_VERT, pt.y);
}

void CMyScrollView::CheckScrollBars(BOOL& bHasHorzBar, BOOL& bHasVertBar) const
{
	// From MFC:  CScrollView::CheckScrollBars

	DWORD dwStyle = GetStyle();
	CScrollBar* pBar = GetScrollBarCtrl(SB_VERT);
	bHasVertBar = (pBar != NULL && pBar->IsWindowEnabled())
			|| (dwStyle & WS_VSCROLL) != 0;

	pBar = GetScrollBarCtrl(SB_HORZ);
	bHasHorzBar = (pBar != NULL && pBar->IsWindowEnabled())
			|| (dwStyle & WS_HSCROLL) != 0;
}

void CMyScrollView::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType)
{
	// From MFC:  CScrollView::CalcWindowRect

	if (nAdjustType == adjustOutside)
	{
		// allow for special client-edge style
		::AdjustWindowRectEx(lpClientRect, 0, FALSE, GetExStyle());

		// if the view is being used in-place, add scrollbar sizes
		//  (scollbars should appear on the outside when in-place editing)
		CSize sizeClient(
			lpClientRect->right - lpClientRect->left,
			lpClientRect->bottom - lpClientRect->top);

		CSize sizeRange = m_totalDev - sizeClient;

		CSize sizeSb;
		GetScrollBarSizes(sizeSb);

		// adjust the window size based on the state
		if (sizeRange.cy > 0)
			lpClientRect->right += sizeSb.cx;
		if (sizeRange.cx > 0)
			lpClientRect->bottom += sizeSb.cy;
	}
	else
	{
		// call default to handle other non-client areas
		::AdjustWindowRectEx(lpClientRect, GetStyle(), FALSE,
			GetExStyle() & ~(WS_EX_CLIENTEDGE));
	}
}

LRESULT CMyScrollView::OnMButtonDown(WPARAM wParam, LPARAM lParam)
{
	// From MFC:  CScrollView::HandleMButtonDown

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
		CScreenDC dcScreen;

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

		// SetWindowRgn now owns the region, so don't destroy it
		rgn.Detach();
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
