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
	void Show(const CPoint& ptAnchor, bool bHorzScroll, bool bVertScroll);
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

	typedef BOOL (WINAPI* pfnSetLayeredWindowAttributes)(HWND hwnd,
			COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
	pfnSetLayeredWindowAttributes m_pSetLayeredWindowAttributes;
	HMODULE m_hUser32;

	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()
};

const int s_nAnchorSize = 32;

HCURSOR CMyAnchorWnd::s_anchorCursors[11] = {0};


// CMyScrollView

IMPLEMENT_DYNAMIC(CMyScrollView, CView)

CMyScrollView::CMyScrollView()
	: m_szContent(0, 0), m_bInsideUpdate(false), m_pAnchorWnd(NULL)
{
	m_szScrollBars.cx = ::GetSystemMetrics(SM_CXVSCROLL);
	m_szScrollBars.cy = ::GetSystemMetrics(SM_CYHSCROLL);
}

CMyScrollView::~CMyScrollView()
{
}


BEGIN_MESSAGE_MAP(CMyScrollView, CView)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_MBUTTONDOWN, OnMButtonDown)
	ON_WM_CANCELMODE()
END_MESSAGE_MAP()


// CMyScrollView message handlers

void CMyScrollView::OnDestroy()
{
	if (m_pAnchorWnd != NULL)
	{
		m_pAnchorWnd->DestroyWindow();
		m_pAnchorWnd = NULL;
	}

	CView::OnDestroy();
}

void CMyScrollView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID(pDC);
	ASSERT(m_szContent.cx >= 0 && m_szContent.cy >= 0);

	pDC->SetMapMode(MM_TEXT);

	// CScrollView sets viewport to minus scroll position, so that
	// all drawing can be done in natural coordinates. However,
	// this does not work in Win98, because in this OS coordinates
	// cannot be larger than 32767.
	// Therefore, we set the viewport to (0, 0) and leave the
	// coordinate translation to the subclasses.
	pDC->SetViewportOrg(CPoint(0, 0));

	CView::OnPrepareDC(pDC, pInfo);
}

void CMyScrollView::SetScrollSizes(const CSize& szContent,
		const CSize& szPage, const CSize& szLine, bool bRepaint)
{
	ASSERT(szContent.cx >= 0 && szContent.cy >= 0);

	m_szContent = szContent;
	m_szPage = szPage;
	m_szLine = szLine;

	if (m_hWnd != NULL)
		UpdateBars(bRepaint);
}

CPoint CMyScrollView::GetScrollPosition() const
{
	CPoint pt(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
	ASSERT(pt.x >= 0 && pt.y >= 0);
	return pt;
}

void CMyScrollView::ScrollToPosition(CPoint pt, bool bRepaint)
{
	bool bHasHorzBar, bHasVertBar;
	CheckScrollBars(bHasHorzBar, bHasVertBar);
	int xMax = bHasHorzBar ? GetScrollLimit(SB_HORZ) : 0;
	int yMax = bHasVertBar ? GetScrollLimit(SB_VERT) : 0;
	pt.x = max(0, min(xMax, pt.x));
	pt.y = max(0, min(yMax, pt.y));

	int xOrig = GetScrollPos(SB_HORZ);
	int yOrig = GetScrollPos(SB_VERT);
	SetScrollPos(SB_HORZ, pt.x);
	SetScrollPos(SB_VERT, pt.y);

	if (bRepaint)
		ScrollWindow(xOrig - pt.x, yOrig - pt.y);
}

void CMyScrollView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	UpdateBars(true);
}

void CMyScrollView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return;

	if (pScrollBar != GetScrollBarCtrl(SB_HORZ))
		return;

	OnScroll(MAKEWORD(nSBCode, 0xff), nPos);
}

void CMyScrollView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return;

	if (pScrollBar != GetScrollBarCtrl(SB_VERT))
		return;

	OnScroll(MAKEWORD(0xff, nSBCode), nPos);
}

BOOL CMyScrollView::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	// we don't handle anything but scrolling
	if ((nFlags & MK_CONTROL) != 0)
		return false;

	bool bHasHorzBar, bHasVertBar;
	CheckScrollBars(bHasHorzBar, bHasVertBar);
	if (!bHasVertBar && !bHasHorzBar)
		return false;

	UINT uWheelScrollLines = GetMouseScrollLines();
	int nToScroll = ::MulDiv(-zDelta, uWheelScrollLines, WHEEL_DELTA);
	int nDisplacement;

	bool bResult;
	if (bHasVertBar && (!bHasHorzBar || (nFlags & MK_SHIFT) == 0))
	{
		if (uWheelScrollLines == WHEEL_PAGESCROLL)
		{
			nDisplacement = m_szPage.cy;
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
		}
		else
		{
			nDisplacement = nToScroll * m_szLine.cy;
			nDisplacement = min(nDisplacement, m_szPage.cy);
		}

		bResult = OnScrollBy(CSize(0, nDisplacement));
	}
	else if (bHasHorzBar)
	{
		if (uWheelScrollLines == WHEEL_PAGESCROLL)
		{
			nDisplacement = m_szPage.cx;
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
		}
		else
		{
			nDisplacement = nToScroll * m_szLine.cx;
			nDisplacement = min(nDisplacement, m_szPage.cx);
		}

		bResult = OnScrollBy(CSize(nDisplacement, 0));
	}

	if (bResult)
		UpdateWindow();

	return bResult;
}

bool CMyScrollView::OnScroll(UINT nScrollCode, UINT nPos, bool bDoScroll)
{
	int x = GetScrollPos(SB_HORZ);
	int xOrig = x;

	switch (LOBYTE(nScrollCode))
	{
	case SB_TOP:
		x = 0;
		break;
	case SB_BOTTOM:
		x = GetScrollLimit(SB_HORZ);
		break;
	case SB_LINEUP:
		x -= m_szLine.cx;
		break;
	case SB_LINEDOWN:
		x += m_szLine.cx;
		break;
	case SB_PAGEUP:
		x -= m_szPage.cx;
		break;
	case SB_PAGEDOWN:
		x += m_szPage.cx;
		break;
	case SB_THUMBTRACK:
		{
			SCROLLINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cbSize = sizeof(si);

			if (!GetScrollInfo(SB_HORZ, &si, SIF_TRACKPOS))
				return false;

			x = si.nTrackPos;
		}
		break;
	}

	int y = GetScrollPos(SB_VERT);
	int yOrig = y;

	switch (HIBYTE(nScrollCode))
	{
	case SB_TOP:
		y = 0;
		break;
	case SB_BOTTOM:
		y = GetScrollLimit(SB_VERT);
		break;
	case SB_LINEUP:
		y -= m_szLine.cy;
		break;
	case SB_LINEDOWN:
		y += m_szLine.cy;
		break;
	case SB_PAGEUP:
		y -= m_szPage.cy;
		break;
	case SB_PAGEDOWN:
		y += m_szPage.cy;
		break;
	case SB_THUMBTRACK:
		{
			SCROLLINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cbSize = sizeof(si);

			if (!GetScrollInfo(SB_VERT, &si, SIF_TRACKPOS))
				return false;

			y = si.nTrackPos;
		}
		break;
	}

	bool bResult = OnScrollBy(CSize(x - xOrig, y - yOrig), bDoScroll);
	if (bResult && bDoScroll)
		UpdateWindow();

	return bResult;
}

bool CMyScrollView::OnScrollBy(CSize szScrollBy, bool bDoScroll)
{
	bool bHasHorzBar, bHasVertBar;
	CheckScrollBars(bHasHorzBar, bHasVertBar);
	if (!bHasHorzBar)
		szScrollBy.cx = 0;
	if (!bHasVertBar)
		szScrollBy.cy = 0;

	// adjust current x position
	int xOrig = GetScrollPos(SB_HORZ);
	int xMax = GetScrollLimit(SB_HORZ);
	int x = max(0, min(xMax, xOrig + szScrollBy.cx));

	// adjust current y position
	int yOrig = GetScrollPos(SB_VERT);
	int yMax = GetScrollLimit(SB_VERT);
	int y = max(0, min(yMax, yOrig + szScrollBy.cy));

	if (x == xOrig && y == yOrig)
		return false;

	if (bDoScroll)
	{
		// do scroll and update scroll positions
		ScrollWindow(-(x - xOrig), -(y - yOrig));
		if (x != xOrig)
			SetScrollPos(SB_HORZ, x);
		if (y != yOrig)
			SetScrollPos(SB_VERT, y);
	}

	return true;
}

void CMyScrollView::GetScrollBarSizes(CSize& szScrollBars) const
{
	szScrollBars.cx = 0;
	if (GetScrollBarCtrl(SB_VERT) == NULL)
		szScrollBars.cx = m_szScrollBars.cx;

	szScrollBars.cy = 0;
	if (GetScrollBarCtrl(SB_HORZ) == NULL)
		szScrollBars.cy = m_szScrollBars.cy;
}

bool CMyScrollView::GetTrueClientSize(CSize& szTrueClient, CSize& szScrollBars) const
{
	szTrueClient = ::GetClientSize(this);
	GetScrollBarSizes(szScrollBars);

	DWORD dwStyle = GetStyle();

	// Check if the vertical scrollbar is impacting client area
	if (szScrollBars.cx != 0 && (dwStyle & WS_VSCROLL) != 0)
		szTrueClient.cx += szScrollBars.cx;

	// Check if the horizontal scrollbar is impacting client area
	if (szScrollBars.cy != 0 && (dwStyle & WS_HSCROLL) != 0)
		szTrueClient.cy += szScrollBars.cy;

	// return true if enough room
	return (szTrueClient.cx > szScrollBars.cx && szTrueClient.cy > szScrollBars.cy);
}

bool CMyScrollView::AdjustClientSize(const CSize& szContent, CSize& szClient,
		bool& bHScroll, bool& bVScroll) const
{
	CSize sizeSb;
	GetScrollBarSizes(sizeSb);

	// Show scrollbars one at a time, because showing one may make the
	// other one unnecessary due to layout changes.

	// Test for vertical scrollbar first. This works best for all
	// zoom types, including Fit Width, Fit Height, Fit Page and Stretch.
	if (szContent.cy > szClient.cy && sizeSb.cx != 0 && szClient.cx > sizeSb.cx && !bVScroll)
	{
		bVScroll = true;
		szClient.cx -= sizeSb.cx;
		return true;
	}

	if (szContent.cx > szClient.cx && sizeSb.cy != 0 && szClient.cy > sizeSb.cy && !bHScroll)
	{
		bHScroll = true;
		szClient.cy -= sizeSb.cy;
		return true;
	}

	return false;
}

void CMyScrollView::CalcScrollBarState(const CSize& szClient,
	CSize& needScrollBars, CSize& szRange, CPoint& ptMove, bool bInsideClient) const
{
	// From MFC:  CScrollView::GetScrollBarState

	CSize szScrollBars;
	GetScrollBarSizes(szScrollBars);

	szRange = m_szContent - szClient;  // > 0 => need to scroll
	ptMove = GetScrollPosition();

	bool bNeedHorz = szRange.cx > 0;
	if (bNeedHorz && bInsideClient)
		szRange.cy += szScrollBars.cy;  // need room for a scroll bar

	bool bNeedVert = szRange.cy > 0;
	if (bNeedVert && bInsideClient)
		szRange.cx += szScrollBars.cx;  // need room for a scroll bar

	if (bNeedVert && !bNeedHorz && szRange.cx > 0)
	{
		ASSERT(bInsideClient);
		// need a horizontal scrollbar after all
		bNeedHorz = true;
		szRange.cy += szScrollBars.cy;
	}

	szRange.cx = max(0, szRange.cx);
	szRange.cy = max(0, szRange.cy);

	ptMove.x = max(0, min(szRange.cx, ptMove.x));
	ptMove.y = max(0, min(szRange.cy, ptMove.y));

	needScrollBars.cx = bNeedHorz;
	needScrollBars.cy = bNeedVert;
}

void CMyScrollView::UpdateBars(bool bRepaint)
{
	// From MFC:  CScrollView::UpdateBars

	// UpdateBars may cause window to be resized - ignore those resizings
	if (m_bInsideUpdate)
		return;

	m_bInsideUpdate = true;

	// update the horizontal to reflect reality
	// NOTE: turning on/off the scrollbars will cause 'OnSize' callbacks
	ASSERT(m_szContent.cx >= 0 && m_szContent.cy >= 0);

	CRect rcClient;
	bool bCalcClient = true;

	// Allow parent to do inside-out layout first.
	// If parent window responds to this message, use rcClient
	// instead of GetTrueClientSize for client size calculation.
	CWnd* pParentWnd = GetParent();
	if (pParentWnd != NULL && pParentWnd->SendMessage(
			WM_RECALCPARENT, 0, (LPARAM)(LPCRECT) &rcClient) != 0)
		bCalcClient = false;

	CSize szClient;
	CSize szScrollBars;

	if (bCalcClient)
	{
		if (!GetTrueClientSize(szClient, szScrollBars))
		{
			// No room for scroll bars (common for zero sized elements).
			EnableScrollBarCtrl(SB_BOTH, false);
			m_bInsideUpdate = false;
			return;
		}
	}
	else
	{
		// Let parent window determine the client rect.
		GetScrollBarSizes(szScrollBars);
		szClient.cx = rcClient.Width();
		szClient.cy = rcClient.Height();
	}

	// Enough room to add scrollbars.
	CSize szRange;
	CPoint ptMove;
	CSize needScrollBars;

	// Get the current scroll bar state given the true client area.
	CalcScrollBarState(szClient, needScrollBars, szRange, ptMove, bCalcClient);
	if (needScrollBars.cx)
		szClient.cy -= szScrollBars.cy;
	if (needScrollBars.cy)
		szClient.cx -= szScrollBars.cx;

	// First hide scrollbars if needed, to avoid extra redrawing.
	if (!needScrollBars.cx)
		EnableScrollBarCtrl(SB_HORZ, false);
	if (!needScrollBars.cy)
		EnableScrollBarCtrl(SB_VERT, false);

	ScrollToPosition(ptMove, bRepaint);

	SCROLLINFO info;
	info.fMask = SIF_PAGE | SIF_RANGE;
	info.nMin = 0;

	// Update the scrollbar page range.
	if (needScrollBars.cx)
	{
		EnableScrollBarCtrl(SB_HORZ, true);
		info.nPage = szClient.cx;
		info.nMax = m_szContent.cx - 1;
		if (!SetScrollInfo(SB_HORZ, &info, true))
			SetScrollRange(SB_HORZ, 0, szRange.cx, true);
	}

	if (needScrollBars.cy)
	{
		EnableScrollBarCtrl(SB_VERT, true);
		info.nPage = szClient.cy;
		info.nMax = m_szContent.cy - 1;
		if (!SetScrollInfo(SB_VERT, &info, true))
			SetScrollRange(SB_VERT, 0, szRange.cy, true);
	}

	// remove recursion lockout
	m_bInsideUpdate = false;
}

void CMyScrollView::CheckScrollBars(bool& bHasHorzBar, bool& bHasVertBar) const
{
	DWORD dwStyle = GetStyle();
	CScrollBar* pHBar = GetScrollBarCtrl(SB_HORZ);
	CScrollBar* pVBar = GetScrollBarCtrl(SB_VERT);

	bHasHorzBar = (pHBar != NULL && pHBar->IsWindowEnabled())
			|| (dwStyle & WS_HSCROLL) != 0;
	bHasVertBar = (pVBar != NULL && pVBar->IsWindowEnabled())
			|| (dwStyle & WS_VSCROLL) != 0;
}

void CMyScrollView::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType)
{
	// From MFC:  CScrollView::CalcWindowRect

	if (nAdjustType == adjustOutside)
	{
		// Allow for special client-edge style.
		::AdjustWindowRectEx(lpClientRect, 0, false, GetExStyle());

		// If the view is being used in-place, add scrollbar sizes
		// (scollbars should appear on the outside when in-place editing).
		CSize szClient(lpClientRect->right - lpClientRect->left,
				lpClientRect->bottom - lpClientRect->top);

		CSize szScrollBars;
		GetScrollBarSizes(szScrollBars);
		CSize szRange = m_szContent - szClient;

		// Adjust the window size based on the state
		if (szRange.cy > 0)
			lpClientRect->right += szScrollBars.cx;
		if (szRange.cx > 0)
			lpClientRect->bottom += szScrollBars.cy;
		return;
	}

	// Call default to handle other non-client areas.
	::AdjustWindowRectEx(lpClientRect, GetStyle(), false,
		GetExStyle() & ~(WS_EX_CLIENTEDGE));
}

LRESULT CMyScrollView::OnMButtonDown(WPARAM wParam, LPARAM lParam)
{
	// From MFC:  CScrollView::HandleMButtonDown

	UINT nFlags = static_cast<UINT>(wParam);
	CPoint point(lParam);

	// If the user has CTRL or SHIFT down, we do not handle the message
	if (nFlags & (MK_SHIFT | MK_CONTROL))
		return Default();

	if (!OnStartPan())
		return false;

	if (m_pAnchorWnd == NULL)
	{
		m_pAnchorWnd = new CMyAnchorWnd();
		m_pAnchorWnd->Create(this);
	}

	if (!m_pAnchorWnd->IsWindowVisible())
	{
		bool bHasHorzBar, bHasVertBar;
		CheckScrollBars(bHasHorzBar, bHasVertBar);
		if (bHasHorzBar || bHasVertBar)
			m_pAnchorWnd->Show(point, bHasHorzBar, bHasVertBar);
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

	return CView::PreTranslateMessage(pMsg);
}

bool CMyScrollView::OnStartPan()
{
	return true;
}

void CMyScrollView::OnPan(CSize szScroll)
{
	OnScrollBy(szScroll, true);
	UpdateWindow();
}

void CMyScrollView::OnEndPan()
{
}

void CMyScrollView::OnCancelMode()
{
	CView::OnCancelMode();

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
	  m_bHorzScroll(false), m_pSetLayeredWindowAttributes(NULL),
	  m_hUser32(NULL)
{
	m_bitmap.LoadBitmap(IDB_PAN_ANCHOR);

	m_hUser32 = ::LoadLibrary(_T("user32.dll"));
	if (m_hUser32 != NULL)
	{
		m_pSetLayeredWindowAttributes =
				(pfnSetLayeredWindowAttributes) ::GetProcAddress(m_hUser32, "SetLayeredWindowAttributes");
		if (m_pSetLayeredWindowAttributes == NULL)
		{
			::FreeLibrary(m_hUser32);
			m_hUser32 = NULL;
		}
	}
}

CMyAnchorWnd::~CMyAnchorWnd()
{
	if (m_hUser32 != NULL)
		::FreeLibrary(m_hUser32);
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

	DWORD dwStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
	if (m_pSetLayeredWindowAttributes != NULL)
		dwStyle |= WS_EX_LAYERED;
	if (!CreateEx(dwStyle, strWndClass, NULL, WS_POPUP,
			CRect(0, 0, s_nAnchorSize, s_nAnchorSize), m_pView, 0))
		return false;

	if (m_pSetLayeredWindowAttributes != NULL)
	{
		m_pSetLayeredWindowAttributes(m_hWnd, RGB(192, 192, 192), 170, LWA_COLORKEY | LWA_ALPHA);
	}
	else
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

	return true;
}

void CMyAnchorWnd::Show(const CPoint& ptAnchor, bool bHorzScroll, bool bVertScroll)
{
	ASSERT(m_pView != NULL);

	m_ptAnchor = ptAnchor;
	m_pView->ClientToScreen(&m_ptAnchor);

	ASSERT(bHorzScroll || bVertScroll);
	m_bHorzScroll = bHorzScroll;
	m_bVertScroll = bVertScroll;

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
		m_ptAnchor.y - s_nAnchorSize / 2, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
	ShowWindow(SW_SHOW);

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
