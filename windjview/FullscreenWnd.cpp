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
#include "FullscreenWnd.h"
#include "DjVuView.h"
#include "MainFrm.h"


// CFullscreenWnd

IMPLEMENT_DYNAMIC(CFullscreenWnd, CWnd)
CFullscreenWnd::CFullscreenWnd(CDjVuView* pOwner)
	: m_pOwner(pOwner), m_pView(NULL)
{
}

CFullscreenWnd::~CFullscreenWnd()
{
}

BEGIN_MESSAGE_MAP(CFullscreenWnd, CWnd)
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()


// CFullscreenWnd message handlers

BOOL CFullscreenWnd::Create()
{
	static CString strWndClass = AfxRegisterWndClass(0);

	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	m_nWidth = dcScreen.GetDeviceCaps(HORZRES);
	m_nHeight = dcScreen.GetDeviceCaps(VERTRES);

	CWnd* pParent = NULL;
	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (::GetVersionEx(&vi) && vi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
			vi.dwMajorVersion >= 5)
	{
		pParent = GetMainFrame();
	}

	return CreateEx(WS_EX_LEFT | WS_EX_TOOLWINDOW, strWndClass, NULL,
		WS_VISIBLE | WS_POPUP, CRect(0, 0, m_nWidth, m_nHeight), pParent, 0);
}

void CFullscreenWnd::SetView(CDjVuView* pView)
{
	m_pView = pView;
	pView->MoveWindow(0, 0, m_nWidth, m_nHeight);
}

void CFullscreenWnd::OnDestroy()
{
	// Detach view from the document
	if (m_pView != NULL)
	{
		int nPage = m_pView->GetCurrentPage();
		m_pView->UpdatePageInfo(m_pOwner);

		m_pView->SetDocument(NULL);
		GetMainFrame()->SetFullscreenWnd(NULL);
		GetMainFrame()->SetFocus();

		m_pOwner->RestartThread();
		m_pOwner->GoToPage(nPage, -1, CDjVuView::DoNotAdd);
	}

	CWnd::OnDestroy();
}

BOOL CFullscreenWnd::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		DestroyWindow();
		return TRUE;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CFullscreenWnd::OnSetFocus(CWnd* pOldWnd)
{
	if (m_pView != NULL)
	{
		m_pView->SetFocus();
		return;
	}

	CWnd::OnSetFocus(pOldWnd);
}

void CFullscreenWnd::PostNcDestroy()
{
	// Should be created on heap
	delete this;
}
