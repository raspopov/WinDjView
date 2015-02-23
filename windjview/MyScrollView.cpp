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
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()
};

const int s_nAnchorSize = 32;

HCURSOR CMyAnchorWnd::s_anchorCursors[11] = {0};


// CMyScrollView

IMPLEMENT_DYNAMIC(CMyScrollView, CView)

CMyScrollView::CMyScrollView()
	: m_szContent(0, 0), m_szPage(0, 0), m_szLine(0, 0), m_szViewport(0, 0),
	  m_szScrollLimit(0, 0), m_ptScrollPos(0, 0), m_bHorzScroll(false),
	  m_bVertScroll(false), m_bShowScrollBars(true), m_bPanning(false),
	  m_pAnchorWnd(NULL)
{
	m_szScrollBars.cx = ::GetSystemMetrics(SM_CXVSCROLL);
	m_szScrollBars.cy = ::GetSystemMetrics(SM_CYHSCROLL);
}

CMyScrollView::~CMyScrollView()
{
}


BEGIN_MESSAGE_MAP(CMyScrollView, CView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_MESSAGE(WM_MBUTTONDOWN, OnMButtonDown)
	ON_WM_CANCELMODE()
END_MESSAGE_MAP()


// CMyScrollView message handlers

BOOL CMyScrollView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CView::PreCreateWindow(cs))
		return false;

	cs.style |= WS_CLIPCHILDREN;
	cs.style &= ~(WS_HSCROLL | WS_VSCROLL);

	return true;
}

int CMyScrollView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_horzScrollBar.Create(SBS_HORZ | WS_CHILD, CRect(0, 0, 0, 0), this, AFX_IDW_HSCROLL_FIRST);
	m_vertScrollBar.Create(SBS_VERT | WS_CHILD, CRect(0, 0, 0, 0), this, AFX_IDW_VSCROLL_FIRST);

	return 0;
}

void CMyScrollView::OnDestroy()
{
	if (m_pAnchorWnd != NULL)
	{
		m_pAnchorWnd->DestroyWindow();
		m_pAnchorWnd = NULL;
	}

	CView::OnDestroy();
}

void CMyScrollView::OnPaint()
{
	CPaintDC dcPaint(this);
	DoPaint(&dcPaint);
}

LRESULT CMyScrollView::OnPrintClient(WPARAM wParam, LPARAM lParam)
{
	CDC dc;
	dc.Attach((HDC) wParam);
	DoPaint(&dc);
	dc.Detach();
	return true;
}

void CMyScrollView::DoPaint(CDC* pDC)
{
	CRect rcViewport(CPoint(0, 0), m_szViewport);
	if (m_bShowScrollBars && m_bHorzScroll && m_bVertScroll)
	{
		pDC->FillSolidRect(CRect(rcViewport.BottomRight(), m_szScrollBars),
				::GetSysColor(COLOR_BTNFACE));
	}

	pDC->IntersectClipRect(rcViewport);
	OnPrepareDC(pDC);
	OnDraw(pDC);
}

void CMyScrollView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID(pDC);
	ASSERT(m_szContent.cx >= 0 && m_szContent.cy >= 0);

	pDC->SetMapMode(MM_TEXT);

	// CScrollView sets viewport to minus scroll position, so that
	// all drawing can be done in natural coordinates. However,
	// this does not work in Windows 98, because in this OS coordinates
	// cannot be larger than 32767. So we do not do that.

	CView::OnPrepareDC(pDC, pInfo);
}

void CMyScrollView::InvalidateViewport()
{
	InvalidateRect(CRect(CPoint(0, 0), m_szViewport), false);
}

void CMyScrollView::ShowScrollBars(bool bShow)
{
	if (m_bShowScrollBars != bShow)
	{
		m_bShowScrollBars = bShow;
		UpdateBars();
	}
}

void CMyScrollView::SetScrollSizes(const CSize& szContent,
		const CSize& szPage, const CSize& szLine, bool bRepaint)
{
	ASSERT(szContent.cx >= 0 && szContent.cy >= 0);
	ASSERT(szPage.cx >= 0 && szPage.cy >= 0);
	ASSERT(szLine.cx >= 0 && szLine.cy >= 0);

	m_szContent = szContent;
	m_szPage = szPage;
	m_szLine = szLine;

	if (m_hWnd != NULL)
		UpdateBars(bRepaint);
}

void CMyScrollView::RepositionScrollBars(bool bHorzScroll, bool bVertScroll)
{
	CRect rcClient = ::GetClientRect(this);
	m_szViewport = rcClient.Size();

	if (m_bShowScrollBars)
	{
		if (!bHorzScroll)
		{
			m_horzScrollBar.ShowWindow(SW_HIDE);
		}
		else
		{
			CRect rcHorz = rcClient;
			rcHorz.top = rcHorz.bottom - m_szScrollBars.cy;
			if (bVertScroll)
				rcHorz.right -= m_szScrollBars.cx;

			m_horzScrollBar.MoveWindow(rcHorz);
			m_horzScrollBar.ShowWindow(SW_SHOW);

			m_szViewport.cy -= m_szScrollBars.cy;
		}

		if (!bVertScroll)
		{
			m_vertScrollBar.ShowWindow(SW_HIDE);
		}
		else
		{
			CRect rcVert = rcClient;
			rcVert.left = rcVert.right - m_szScrollBars.cx;
			if (bHorzScroll)
				rcVert.bottom -= m_szScrollBars.cy;

			m_vertScrollBar.MoveWindow(rcVert);
			m_vertScrollBar.ShowWindow(SW_SHOW);

			m_szViewport.cx -= m_szScrollBars.cx;
		}

		if (bHorzScroll && bVertScroll)
		{
			CRect rect(CPoint(m_szViewport), m_szScrollBars);
			InvalidateRect(rect, false);
		}
	}
	else
	{
		m_horzScrollBar.ShowWindow(SW_HIDE);
		m_vertScrollBar.ShowWindow(SW_HIDE);
	}

	m_bHorzScroll = bHorzScroll;
	m_bVertScroll = bVertScroll;
}

void CMyScrollView::SetScrollPosition(const CPoint& pos)
{
	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;

	si.nPos = pos.x;
	::SetScrollInfo(m_horzScrollBar.m_hWnd, SB_CTL, &si, true);

	si.nPos = pos.y;
	::SetScrollInfo(m_vertScrollBar.m_hWnd, SB_CTL, &si, true);

	m_ptScrollPos = pos;
}

void CMyScrollView::ScrollToPosition(CPoint pt, bool bRepaint)
{
	CPoint ptScroll = GetScrollPosition();

	pt.x = max(0, min(pt.x, m_szScrollLimit.cx));
	pt.y = max(0, min(pt.y, m_szScrollLimit.cy));
	SetScrollPosition(pt);

	if (bRepaint)
	{
		CRect rcViewport(CPoint(0, 0), m_szViewport);
		ScrollWindowEx(ptScroll.x - pt.x, ptScroll.y - pt.y,
				rcViewport, rcViewport, NULL, NULL, SW_INVALIDATE | SW_SCROLLCHILDREN);
	}
	else
	{
		InvalidateViewport();
	}
}

void CMyScrollView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	UpdateBars();
}

void CMyScrollView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return;

	if (pScrollBar != NULL && pScrollBar != &m_horzScrollBar)
		return;

	OnScroll(MAKEWORD(nSBCode, 0xff), nPos);
}

void CMyScrollView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return;

	if (pScrollBar != NULL && pScrollBar != &m_vertScrollBar)
		return;

	OnScroll(MAKEWORD(0xff, nSBCode), nPos);
}

BOOL CMyScrollView::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	// We don't handle anything but scrolling.
	if ((nFlags & MK_CONTROL) != 0)
		return false;

	if (!m_bHorzScroll && !m_bVertScroll)
		return false;

	UINT uWheelScrollLines = GetMouseScrollLines();
	int nToScroll = ::MulDiv(-zDelta, uWheelScrollLines, WHEEL_DELTA);
	int nDisplacement;

	bool bResult;
	if (m_bVertScroll && (!m_bHorzScroll || (nFlags & MK_SHIFT) == 0))
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
	else if (m_bHorzScroll)
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
	CPoint ptScroll = m_ptScrollPos;

	switch (LOBYTE(nScrollCode))
	{
	case SB_TOP:
		ptScroll.x = 0;
		break;
	case SB_BOTTOM:
		ptScroll.x = m_szScrollLimit.cx;
		break;
	case SB_LINEUP:
		ptScroll.x -= m_szLine.cx;
		break;
	case SB_LINEDOWN:
		ptScroll.x += m_szLine.cx;
		break;
	case SB_PAGEUP:
		ptScroll.x -= m_szPage.cx;
		break;
	case SB_PAGEDOWN:
		ptScroll.x += m_szPage.cx;
		break;
	case SB_THUMBTRACK:
		{
			SCROLLINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cbSize = sizeof(si);
			si.fMask = SIF_TRACKPOS;

			::GetScrollInfo(m_horzScrollBar.m_hWnd, SB_CTL, &si);
			ptScroll.x = si.nTrackPos;
		}
		break;
	}

	switch (HIBYTE(nScrollCode))
	{
	case SB_TOP:
		ptScroll.y = 0;
		break;
	case SB_BOTTOM:
		ptScroll.y = m_szScrollLimit.cy;
		break;
	case SB_LINEUP:
		ptScroll.y -= m_szLine.cy;
		break;
	case SB_LINEDOWN:
		ptScroll.y += m_szLine.cy;
		break;
	case SB_PAGEUP:
		ptScroll.y -= m_szPage.cy;
		break;
	case SB_PAGEDOWN:
		ptScroll.y += m_szPage.cy;
		break;
	case SB_THUMBTRACK:
		{
			SCROLLINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cbSize = sizeof(si);
			si.fMask = SIF_TRACKPOS;

			::GetScrollInfo(m_vertScrollBar.m_hWnd, SB_CTL, &si);
			ptScroll.y = si.nTrackPos;
		}
		break;
	}

	bool bResult = OnScrollBy(ptScroll - m_ptScrollPos, bDoScroll);
	if (bResult && bDoScroll)
		UpdateWindow();

	return bResult;
}

bool CMyScrollView::OnScrollBy(CSize szScrollBy, bool bDoScroll)
{
	if (!m_bHorzScroll)
		szScrollBy.cx = 0;
	if (!m_bVertScroll)
		szScrollBy.cy = 0;

	// Adjust current position.
	CPoint ptScroll = m_ptScrollPos;
	ptScroll.x = max(0, min(ptScroll.x + szScrollBy.cx, m_szScrollLimit.cx));
	ptScroll.y = max(0, min(ptScroll.y + szScrollBy.cy, m_szScrollLimit.cy));

	if (ptScroll == m_ptScrollPos)
		return false;

	if (bDoScroll)
	{
		// Do the scrolling and update scroll positions.
		CRect rcViewport(CPoint(0, 0), m_szViewport);
		ScrollWindowEx(-(ptScroll.x - m_ptScrollPos.x), -(ptScroll.y - m_ptScrollPos.y),
				rcViewport, rcViewport, NULL, NULL, SW_INVALIDATE | SW_SCROLLCHILDREN);

		SetScrollPosition(ptScroll);
	}

	return true;
}

bool CMyScrollView::AdjustViewportSize(const CSize& szContent, CSize& szViewport,
		bool& bHScroll, bool& bVScroll) const
{
	if (szViewport.cx <= m_szScrollBars.cx || szViewport.cy <= m_szScrollBars.cy
			|| !m_bShowScrollBars)
		return false;

	// Show scrollbars one at a time, because showing one may make the
	// other one unnecessary due to layout changes.

	// Test for vertical scrollbar first. This works best for all
	// zoom types, including Fit Width, Fit Height, Fit Page and Stretch.
	if (szContent.cy > szViewport.cy && !bVScroll)
	{
		bVScroll = true;
		szViewport.cx -= m_szScrollBars.cx;
		return true;
	}

	if (szContent.cx > szViewport.cx && !bHScroll)
	{
		bHScroll = true;
		szViewport.cy -= m_szScrollBars.cy;

		if (bVScroll && szViewport.cy <= m_szScrollBars.cy)
		{
			bVScroll = false;
			szViewport.cx += m_szScrollBars.cx;
		}

		return true;
	}

	return false;
}

void CMyScrollView::CalcScrollBarState(bool& bNeedHorz, bool& bNeedVert,
		CSize& szRange, CPoint& ptMove) const
{
	CSize szClient = ::GetClientSize(this);

	szRange = m_szContent - szClient;
	bNeedHorz = bNeedVert = false;
	ptMove = GetScrollPosition();

	if (m_bShowScrollBars)
	{
		while (AdjustViewportSize(m_szContent, szClient, bNeedHorz, bNeedVert))
			EMPTY_LOOP;

		if (bNeedHorz)
			szRange.cy += m_szScrollBars.cy;
		if (bNeedVert)
			szRange.cx += m_szScrollBars.cx;
	}
	else
	{
		bNeedHorz = (szRange.cx > 0);
		bNeedVert = (szRange.cy > 0);
	}

	szRange.cx = max(0, szRange.cx);
	szRange.cy = max(0, szRange.cy);

	ptMove.x = max(0, min(ptMove.x, szRange.cx));
	ptMove.y = max(0, min(ptMove.y, szRange.cy));
}

void CMyScrollView::UpdateBars(bool bRepaint)
{
	bool bNeedHorz, bNeedVert;
	CSize szRange;
	CPoint ptMove;
	CalcScrollBarState(bNeedHorz, bNeedVert, szRange, ptMove);

	m_szScrollLimit = szRange;
	RepositionScrollBars(bNeedHorz, bNeedVert);

	// Update the scrollbar page and range.
	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE | SIF_RANGE;
	si.nMin = 0;

	si.nPage = m_szViewport.cx;
	si.nMax = max(0, m_szContent.cx - 1);
	::SetScrollInfo(m_horzScrollBar.m_hWnd, SB_CTL, &si, false);

	si.nPage = m_szViewport.cy;
	si.nMax = max(0, m_szContent.cy - 1);
	::SetScrollInfo(m_vertScrollBar.m_hWnd, SB_CTL, &si, false);

	ScrollToPosition(ptMove, bRepaint);
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
		CSize szRange = m_szContent - szClient;

		// Adjust the window size based on the state
		if (szRange.cy > 0 && m_bShowScrollBars)
			lpClientRect->right += m_szScrollBars.cx;
		if (szRange.cx > 0 && m_bShowScrollBars)
			lpClientRect->bottom += m_szScrollBars.cy;
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

	if (!m_bHorzScroll && !m_bVertScroll)
		return Default();

	if (!OnStartPan())
		return false;

	if (m_pAnchorWnd == NULL)
	{
		m_pAnchorWnd = new CMyAnchorWnd();
		m_pAnchorWnd->Create(this);
	}

	if (!m_pAnchorWnd->IsWindowVisible())
		m_pAnchorWnd->Show(point, m_bHorzScroll, m_bVertScroll);
	else
		m_pAnchorWnd->Hide();

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
	: m_nDefaultCursor(4), m_pView(NULL), m_bQuitTracking(false),
	  m_bVertScroll(false), m_bHorzScroll(false),
	  m_pSetLayeredWindowAttributes(NULL), m_hUser32(NULL)
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

void CMyAnchorWnd::OnTimer(UINT_PTR nIDEvent)
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
		nScrollX = static_cast<int>(fabs(1.0*szDistance.cx) / 15) + 1;
		if (fabs(1.0*szDistance.cx) >= 150)
			nScrollX += static_cast<int>(pow(fabs(1.0*szDistance.cx) - 150, 2) / 40);

		nScrollY = static_cast<int>(fabs(1.0*szDistance.cy) / 15) + 1;
		if (fabs(1.0*szDistance.cy) >= 150)
			nScrollY += static_cast<int>(pow(fabs(1.0*szDistance.cy) - 150, 2) / 40);

		CSize szScroll(nSignX * nScrollX, nSignY * nScrollY);
		m_pView->OnPan(szScroll);
	}
}

BOOL CMyAnchorWnd::Create(CMyScrollView* pView)
{
	static CString strWndClass = AfxRegisterWndClass(CS_DBLCLKS,
			::LoadCursor(NULL, IDC_ARROW));

	m_pView = pView;

	DWORD dwExStyle = WS_EX_TOOLWINDOW;
	if (m_pSetLayeredWindowAttributes != NULL)
		dwExStyle |= WS_EX_LAYERED;
	if (!CreateEx(dwExStyle, strWndClass, NULL, WS_POPUP,
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
	m_pView->m_bPanning = true;

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
	ShowWindow(SW_SHOWNA);
	Invalidate();
	UpdateWindow();

	m_bQuitTracking = false;
	SetCapture();
	SetTimer(1, 20, NULL);

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

	m_pView->m_bPanning = false;
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
