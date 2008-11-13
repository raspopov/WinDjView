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
#include "FullscreenWnd.h"
#include "DjVuView.h"
#include "ThumbnailsView.h"
#include "ChildFrm.h"


// CFullscreenWnd

IMPLEMENT_DYNAMIC(CFullscreenWnd, CWnd)
CFullscreenWnd::CFullscreenWnd()
	: m_pOwner(NULL), m_pView(NULL)
{
}

CFullscreenWnd::~CFullscreenWnd()
{
	if (AfxGetThreadState()->m_pRoutingFrame == (CFrameWnd*) this)
		AfxGetThreadState()->m_pRoutingFrame = NULL;
}

BEGIN_MESSAGE_MAP(CFullscreenWnd, CWnd)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_ERASEBKGND()
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNeedText)
	ON_MESSAGE(WM_APPCOMMAND, OnAppCommand)
END_MESSAGE_MAP()


// CFullscreenWnd message handlers

BOOL CFullscreenWnd::Create()
{
	static CString strWndClass = AfxRegisterWndClass(CS_DBLCLKS,
			::LoadCursor(NULL, IDC_ARROW), NULL,
			::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME)));

	return CreateEx(0, strWndClass, NULL,
		WS_POPUP, CRect(0, 0, 0, 0), NULL, 0);
}

void CFullscreenWnd::Show(CDjVuView* pOwner, CDjVuView* pContents)
{
	ASSERT(m_pOwner == NULL);

	m_pOwner = pOwner;
	m_pView = pContents;

	CRect rcMonitor = GetMonitorRect(pOwner);
	int nWidth = rcMonitor.Width();
	int nHeight = rcMonitor.Height();

	MoveWindow(rcMonitor.left, rcMonitor.top, nWidth, nHeight);
	m_pView->MoveWindow(0, 0, nWidth, nHeight);

	CString strText;
	pOwner->GetTopLevelParent()->GetWindowText(strText);
	SetWindowText(strText);

	ShowWindow(SW_SHOW);
	pOwner->GetTopLevelParent()->ShowWindow(SW_HIDE);
}

void CFullscreenWnd::Hide()
{
	if (m_pOwner != NULL)
	{
		int nPage = m_pView->GetCurrentPage();
		m_pOwner->UpdatePageInfoFrom(m_pView);
		m_pOwner->CopyBitmapsFrom(m_pView, true);

		m_pOwner->GoToPage(nPage, CDjVuView::DoNotAdd);

		// Detach view from the document before destroying
		m_pView->SetDocument(NULL);
		m_pView->DestroyWindow();

		m_pOwner->ResumeDecoding();

		CThumbnailsView* pThumbnails = ((CChildFrame*) m_pOwner->GetParentFrame())->GetThumbnailsView();
		if (pThumbnails != NULL)
			pThumbnails->ResumeDecoding();

		m_pOwner->GetTopLevelParent()->ShowWindow(SW_SHOW);
	}

	ShowWindow(SW_HIDE);

	m_pView = NULL;
	m_pOwner = NULL;
}

BOOL CFullscreenWnd::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		Hide();
		return true;
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

BOOL CFullscreenWnd::OnToolTipNeedText(UINT nID, NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_pView == NULL)
		return false;

	return m_pView->SendMessage(WM_NOTIFY, nID, reinterpret_cast<LPARAM>(pNMHDR));
}

BOOL CFullscreenWnd::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void CFullscreenWnd::OnClose()
{
	if (m_pOwner != NULL)
	{
		// WM_CLOSE may cause a message box to pop up. Ensure that it gets a correct parent.
		CPushRoutingFrame push((CFrameWnd*) this);

		m_pOwner->GetTopLevelFrame()->SendMessage(WM_CLOSE);
	}
	else
	{
		Hide();
	}
}

void CFullscreenWnd::OnDestroy()
{
	if (m_pView != NULL)
		m_pView->SetDocument(NULL);

	CWnd::OnDestroy();
}

BOOL CFullscreenWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (IsWindowVisible())
	{
		// Trick to make dialogs and message boxes work properly
		CPushRoutingFrame push((CFrameWnd*) this);

		// Send commands to the view
		if (m_pView != NULL && ::IsWindow(m_pView->m_hWnd)
				&& m_pView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return true;

		if (CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return true;

		if (theApp.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return true;

		return false;
	}
	else
	{
		return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	}
}

LRESULT CFullscreenWnd::OnAppCommand(WPARAM wParam, LPARAM lParam)
{
	if (IsWindowVisible() && m_pView != NULL)
	{
		UINT nCommand = GET_APPCOMMAND_LPARAM(lParam);
		if (nCommand == APPCOMMAND_BROWSER_BACKWARD)
		{
			m_pOwner->GetTopLevelParent()->SendMessage(WM_COMMAND, ID_VIEW_BACK);
			return 1;
		}
		else if (nCommand == APPCOMMAND_BROWSER_FORWARD)
		{
			m_pOwner->GetTopLevelParent()->SendMessage(WM_COMMAND, ID_VIEW_FORWARD);
			return 1;
		}
	}

	Default();
	return 0;
}
