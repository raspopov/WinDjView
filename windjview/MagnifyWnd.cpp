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
		m_pUpdateLayeredWindow =
				(pfnUpdateLayeredWindow) ::GetProcAddress(m_hUser32, "UpdateLayeredWindow");
		if (m_pUpdateLayeredWindow == NULL)
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

	m_rcPos = CRect(0, 0, 480, 320);

	DWORD dwExStyle = WS_EX_TOOLWINDOW;
	if (m_pUpdateLayeredWindow != NULL)
		dwExStyle |= WS_EX_LAYERED;
	if (!CreateEx(dwExStyle, strWndClass, NULL, WS_POPUP | WS_CLIPCHILDREN,
			m_rcPos, NULL, 0))
		return false;

	return true;
}

void CMagnifyWnd::Init(CDjVuView* pOwner, CDjVuView* pContents)
{
	ASSERT(m_pOwner == NULL);

	m_pOwner = pOwner;

	m_pView = pContents;
	m_pView->MoveWindow(1, 1, m_rcPos.Width() - 2, m_rcPos.Height() - 2);
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

void CMagnifyWnd::RepaintContents()
{
	ASSERT(m_pUpdateLayeredWindow != NULL);
	ASSERT(m_pView != NULL);

	CScreenDC dcScreen;
	m_offscreenDC.Create(&dcScreen, m_rcPos.Size());
	FrameRect(&m_offscreenDC, m_rcPos - m_rcPos.TopLeft(),
			::GetSysColor(COLOR_WINDOWFRAME));

	m_offscreenDC.SetViewportOrg(1, 1);
	int nSaveDC = m_offscreenDC.SaveDC();
	m_offscreenDC.IntersectClipRect(0, 0, m_rcPos.Width() - 2, m_rcPos.Height() - 2);
	m_pView->SendMessage(WM_PRINT, (WPARAM) m_offscreenDC.m_hDC, PRF_CLIENT);
	m_offscreenDC.RestoreDC(nSaveDC);
	m_offscreenDC.SetViewportOrg(0, 0);

	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = 225;
	bf.AlphaFormat = 0;

	m_pUpdateLayeredWindow(m_hWnd, NULL, &m_rcPos.TopLeft(), &m_rcPos.Size(),
			m_offscreenDC.m_hDC, &CPoint(0, 0), 0, &bf, ULW_ALPHA);
	m_offscreenDC.Release();
}

void CMagnifyWnd::PostNcDestroy()
{
	// Should be created on heap
	delete this;
}

void CMagnifyWnd::CenterOnPoint(const CPoint& pt)
{
	CPoint ptOffset(pt.x - m_rcPos.Width() / 2, pt.y - m_rcPos.Height() / 2);
	m_rcPos += ptOffset - m_rcPos.TopLeft();

	if (m_pUpdateLayeredWindow != NULL)
	{
		RepaintContents();
	}
	else
	{
		Invalidate();
		MoveWindow(m_rcPos.left, m_rcPos.top, m_rcPos.Width(), m_rcPos.Height());
		m_pOwner->GetTopLevelParent()->UpdateWindow();

		m_pView->Invalidate();
		UpdateWindow();
	}

	if (!IsWindowVisible())
	{
		ShowWindow(SW_SHOWNA);
		SendMessageToVisibleDescendants(m_hWnd, WM_SHOWPARENT, true);
	}
}

void CMagnifyWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	FrameRect(&dc, m_rcPos - m_rcPos.TopLeft(),
			::GetSysColor(COLOR_WINDOWFRAME));
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
