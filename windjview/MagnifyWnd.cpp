//	WinDjView
//	Copyright (C) 2004-2009 Andrew Zhezherun
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
#include "MagnifyWnd.h"
#include "DjVuView.h"


// CMagnifyWnd

IMPLEMENT_DYNAMIC(CMagnifyWnd, CWnd)
CMagnifyWnd::CMagnifyWnd()
	: m_pOwner(NULL), m_pView(NULL), m_hUser32(NULL)
{
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

CMagnifyWnd::~CMagnifyWnd()
{
	if (m_hUser32 != NULL)
		::FreeLibrary(m_hUser32);
}

BEGIN_MESSAGE_MAP(CMagnifyWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CMagnifyWnd message handlers

BOOL CMagnifyWnd::Create()
{
	static CString strWndClass = AfxRegisterWndClass(CS_DBLCLKS,
			::LoadCursor(NULL, IDC_ARROW));

	m_nWidth = 480;
	m_nHeight = 320;

	DWORD dwExStyle = WS_EX_TOOLWINDOW;
	if (m_pSetLayeredWindowAttributes != NULL)
		dwExStyle |= WS_EX_LAYERED;
	if (!CreateEx(dwExStyle, strWndClass, NULL, WS_POPUP,
			CRect(0, 0, m_nWidth, m_nHeight), NULL, 0))
		return false;

	if (m_pSetLayeredWindowAttributes != NULL)
		m_pSetLayeredWindowAttributes(m_hWnd, 0, 0, LWA_ALPHA);

	return true;
}

void CMagnifyWnd::Show(CDjVuView* pOwner, CDjVuView* pContents, const CPoint& ptCenter)
{
	ASSERT(m_pOwner == NULL);

	m_pOwner = pOwner;
	m_pView = pContents;

	m_pView->MoveWindow(1, 1, m_nWidth - 2, m_nHeight - 2);
	CenterOnPoint(ptCenter);

	SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
	Invalidate();

	// To keep the layered window from briefly flashing its previous state, show
	// it initially as completely transparent, and then change transparency when
	// everything is ready.
	if (m_pSetLayeredWindowAttributes != NULL)
		m_pSetLayeredWindowAttributes(m_hWnd, 0, 0, LWA_ALPHA);

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

void CMagnifyWnd::Update()
{
	UpdateWindow();

	if (m_pSetLayeredWindowAttributes != NULL)
		m_pSetLayeredWindowAttributes(m_hWnd, 0, 220, LWA_ALPHA);
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
