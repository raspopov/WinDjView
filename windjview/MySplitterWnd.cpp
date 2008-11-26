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
#include "MySplitterWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static int s_nSplitterWidth = 6;
static HCURSOR s_hCursorSplitter = NULL;


// CMySplitterWnd

IMPLEMENT_DYNAMIC(CMySplitterWnd, CWnd)

CMySplitterWnd::CMySplitterWnd()
	: m_pContentWnd(NULL), m_nSplitterPos(0), m_bChildMaximized(false), m_bDragging(false)
{
	m_nExpandedNavWidth = max(theApp.GetAppSettings()->nNavPaneWidth, CNavPaneWnd::s_nMinExpandedWidth);
	m_bNavCollapsed = theApp.GetAppSettings()->bNavPaneCollapsed;
	m_bNavHidden = theApp.GetAppSettings()->bNavPaneHidden;
}

CMySplitterWnd::~CMySplitterWnd()
{
}


BEGIN_MESSAGE_MAP(CMySplitterWnd, CWnd)
	ON_WM_NCCREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


// CMySplitterWnd message handlers

BOOL CMySplitterWnd::Create(CWnd* pParent, UINT nID)
{
	if (!CWnd::Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), pParent, nID, NULL))
		return false;

	if (!m_navPane.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, 1))
		return false;

	m_nSplitterPos = CNavPaneWnd::s_nTabsWidth;
	if (!m_bNavCollapsed)
		m_nSplitterPos = m_nExpandedNavWidth;

	m_navPane.ShowWindow(m_bNavHidden ? SW_HIDE : SW_SHOW);

	return true;
}

BOOL CMySplitterWnd::CreateContent(CRuntimeClass* pContentClass, CCreateContext* pContext)
{
	m_pContentWnd = (CWnd*) pContentClass->CreateObject();
	ASSERT_KINDOF(CWnd, m_pContentWnd);
	ASSERT(m_pContentWnd->m_hWnd == NULL);

	if (!m_pContentWnd->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, 2, pContext))
		return false;

	return true;
}

BOOL CMySplitterWnd::OnNcCreate(LPCREATESTRUCT lpcs)
{
	if (!CWnd::OnNcCreate(lpcs))
		return false;

	// remove WS_EX_CLIENTEDGE style from parent window
	CWnd* pParent = GetParent();
	ASSERT_VALID(pParent);
	pParent->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME);

	return true;
}

void CMySplitterWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd* pFrame = GetParent();
	while (pFrame != NULL && !pFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)))
		pFrame = pFrame->GetParent();

	if (pFrame != NULL)
	{
		BOOL bMaximized = false;
		CMDIFrameWnd* pMDIFrame = ((CMDIChildWnd*) pFrame)->GetMDIFrame();
		CMDIChildWnd* pActive = pMDIFrame->MDIGetActive(&bMaximized);
		if (pFrame != pActive && theApp.GetAppSettings()->bChildMaximized && !pFrame->IsZoomed())
		{
			CWnd::OnSize(nType, cx, cy);
			return;
		}
	}

	if (cx > 0 && cy > 0)
		RecalcLayout();

	CWnd::OnSize(nType, cx, cy);
}

void CMySplitterWnd::RecalcLayout()
{
	CRect rcClient;
	GetClientRect(rcClient);
	int nWidth = rcClient.Width() - s_nSplitterWidth;

	CWnd* pFrame = GetParent();
	while (pFrame != NULL && !pFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)))
		pFrame = pFrame->GetParent();
	m_bChildMaximized = (pFrame != NULL ? !!pFrame->IsZoomed() : false);

	int nTopOffset = (m_bChildMaximized ? 1 : 0);
	if (m_bChildMaximized)
		InvalidateRect(CRect(0, 0, rcClient.right, nTopOffset));

	if (!m_bNavHidden)
	{
		m_nSplitterPos = min(m_nSplitterPos, nWidth);
		if (m_nSplitterPos < CNavPaneWnd::s_nMinExpandedWidth)
		{
			m_bNavCollapsed = true;
			m_nSplitterPos = CNavPaneWnd::s_nTabsWidth;
		}
		else
		{
			m_bNavCollapsed = false;
			m_nExpandedNavWidth = m_nSplitterPos;
			theApp.GetAppSettings()->nNavPaneWidth = m_nExpandedNavWidth;
		}

		m_rcNavPane.left = 0;
		m_rcNavPane.top = nTopOffset;
		m_rcNavPane.right = m_nSplitterPos;
		m_rcNavPane.bottom = max(m_rcNavPane.top, rcClient.bottom);

		m_rcSplitter.left = m_nSplitterPos;
		m_rcSplitter.top = 0;
		m_rcSplitter.right = m_nSplitterPos + s_nSplitterWidth;
		m_rcSplitter.bottom = rcClient.bottom;

		m_rcContent.left = m_nSplitterPos + s_nSplitterWidth;
		m_rcContent.top = nTopOffset;
		m_rcContent.right = max(m_rcContent.left, rcClient.right);
		m_rcContent.bottom = max(m_rcContent.top, rcClient.bottom);

		m_navPane.MoveWindow(m_rcNavPane);
		InvalidateRect(m_rcSplitter);
	}
	else
	{
		m_rcContent.left = 0;
		m_rcContent.top = nTopOffset;
		m_rcContent.right = rcClient.right;
		m_rcContent.bottom = max(m_rcContent.top, rcClient.bottom);
	}

	if (m_pContentWnd != NULL)
		m_pContentWnd->MoveWindow(m_rcContent);
}

void CMySplitterWnd::HideNavPane(bool bHide)
{
	m_bNavHidden = bHide;
	RecalcLayout();

	m_navPane.ShowWindow(bHide ? SW_HIDE : SW_SHOW);
	UpdateWindow();
}

void CMySplitterWnd::OnPaint()
{
	CPaintDC paintDC(this);

	CRect rcClient;
	GetClientRect(rcClient);

	CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNSHADOW));
	CPen* pOldPen = paintDC.SelectObject(&pen);

	if (!m_bNavHidden)
	{
		if (m_bChildMaximized)
		{
			paintDC.MoveTo(0, 0);
			paintDC.LineTo(m_rcSplitter.left, 0);
			paintDC.MoveTo(m_rcSplitter.right, 0);
			paintDC.LineTo(rcClient.right, 0);
		}

		paintDC.MoveTo(m_rcSplitter.left, 0);
		paintDC.LineTo(m_rcSplitter.left, rcClient.bottom);
		paintDC.MoveTo(m_rcSplitter.right - 1, 0);
		paintDC.LineTo(m_rcSplitter.right - 1, rcClient.bottom);

		CRect rcSplitterBody = m_rcSplitter;
		rcSplitterBody.DeflateRect(1, 0, 1, 0);
		paintDC.FillSolidRect(rcSplitterBody, ::GetSysColor(COLOR_BTNFACE));
	}
	else
	{
		if (m_bChildMaximized)
		{
			paintDC.MoveTo(0, 0);
			paintDC.LineTo(rcClient.right, 0);
		}
	}

	paintDC.SelectObject(pOldPen);
}

BOOL CMySplitterWnd::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

CNavPaneWnd* CMySplitterWnd::GetNavPane()
{
	return &m_navPane;
}

CWnd* CMySplitterWnd::GetContent()
{
	return m_pContentWnd;
}

void CMySplitterWnd::CollapseNavPane(bool bCollapse)
{
	m_bNavCollapsed = bCollapse;
	theApp.GetAppSettings()->bNavPaneCollapsed = m_bNavCollapsed;

	m_nSplitterPos = m_bNavCollapsed ? CNavPaneWnd::s_nTabsWidth : m_nExpandedNavWidth;
	RecalcLayout();
}

void CMySplitterWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bDragging)
	{
		m_nSplitterPos = m_nOrigSplitterPos + point.x - m_ptDragStart.x;
		RecalcLayout();
		::RedrawWindow(m_hWnd, NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN);
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CMySplitterWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_bDragging)
		StopDragging();

	if (!m_bNavHidden && m_rcSplitter.PtInRect(point))
	{
		m_bDragging = true;
		m_ptDragStart = point;
		m_nOrigSplitterPos = m_nSplitterPos;
		SetCapture();
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CMySplitterWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDragging)
		StopDragging();

	CWnd::OnLButtonUp(nFlags, point);
}

void CMySplitterWnd::StopDragging()
{
	m_bDragging = false;
	ReleaseCapture();
}

BOOL CMySplitterWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (nHitTest == HTCLIENT)
	{
		CPoint ptCursor;
		::GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);

		if (!m_bNavHidden && m_rcSplitter.PtInRect(ptCursor))
		{
			if (s_hCursorSplitter == NULL)
				s_hCursorSplitter = theApp.LoadCursor(IDC_CURSOR_SPLIT_HORZ);
			SetCursor(s_hCursorSplitter);
			return true;
		}
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}
