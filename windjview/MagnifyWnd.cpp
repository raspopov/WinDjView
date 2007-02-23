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
#include "MagnifyWnd.h"
#include "DjVuView.h"


// CFullscreenWnd

IMPLEMENT_DYNAMIC(CMagnifyWnd, CWnd)
CMagnifyWnd::CMagnifyWnd()
	: m_pOwner(NULL), m_pView(NULL)
{
}

CMagnifyWnd::~CMagnifyWnd()
{
}

BEGIN_MESSAGE_MAP(CMagnifyWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CFullscreenWnd message handlers

BOOL CMagnifyWnd::Create()
{
	static CString strWndClass = AfxRegisterWndClass(CS_DBLCLKS,
			::LoadCursor(NULL, IDC_ARROW));

	m_nWidth = 480;
	m_nHeight = 320;

	return CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, strWndClass, NULL, WS_POPUP,
		CRect(0, 0, m_nWidth, m_nHeight), NULL, 0);
}

void CMagnifyWnd::Show(CDjVuView* pOwner, CDjVuView* pContents, const CPoint& ptCenter)
{
	ASSERT(m_pOwner == NULL);

	m_pOwner = pOwner;
	m_pView = pContents;

	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	m_pView->MoveWindow(1, 1, m_nWidth - 2, m_nHeight - 2);
	CenterOnPoint(ptCenter);

	ShowWindow(SW_SHOWNA);
}

void CMagnifyWnd::Hide()
{
	ShowWindow(SW_HIDE);

	if (m_pOwner != NULL)
	{
		m_pOwner->GetTopLevelParent()->UpdateWindow();
	}

	// Detach view from the document before destroying
	if (m_pView != NULL)
	{
		m_pView->SetDocument(NULL);
		m_pView->DestroyWindow();
	}

	m_pView = NULL;
	m_pOwner = NULL;
}

void CMagnifyWnd::PostNcDestroy()
{
	// Should be created on heap
	delete this;
}

void CMagnifyWnd::CenterOnPoint(const CPoint& point)
{
	MoveWindow(point.x - m_nWidth / 2, point.y - m_nHeight / 2, m_nWidth, m_nHeight);
}

void CMagnifyWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	dc.FrameRect(CRect(0, 0, m_nWidth, m_nHeight), &CBrush(::GetSysColor(COLOR_WINDOWFRAME)));
}

BOOL CMagnifyWnd::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void CMagnifyWnd::OnDestroy()
{
	if (m_pView != NULL)
		m_pView->SetDocument(NULL);

	CWnd::OnDestroy();
}
