//	WinDjView
//	Copyright (C) 2004-2006 Andrew Zhezherun
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

#include "ChildFrm.h"
#include "MainFrm.h"
#include "DjVuView.h"
#include "ThumbnailsView.h"
#include "BookmarksView.h"
#include "SearchResultsView.h"
#include "NavPane.h"
#include "AppSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	ON_WM_MDIACTIVATE()
	ON_WM_WINDOWPOSCHANGED()
	ON_MESSAGE_VOID(ID_EXPAND_PANE, OnExpandPane)
	ON_MESSAGE_VOID(ID_COLLAPSE_PANE, OnCollapsePane)
	ON_WM_ERASEBKGND()
	ON_WM_CLOSE()
	ON_MESSAGE_VOID(WM_LANGUAGE_CHANGED, OnLanguageChanged)
	ON_WM_NCPAINT()
END_MESSAGE_MAP()


// CChildFrame construction/destruction

CChildFrame::CChildFrame()
	: m_bCreated(false), m_bActivating(false), m_pThumbnailsView(NULL),
	  m_pBookmarksView(NULL), m_pResultsView(NULL)
{
}

CChildFrame::~CChildFrame()
{
}


// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG


// CChildFrame message handlers

void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	CComboBox& cboPage = GetMainFrame()->m_cboPage;
	CComboBox& cboZoom = GetMainFrame()->m_cboZoom;

	if (bActivate)
	{
		CDjVuView* pView = GetDjVuView();

		if (!m_bActivating)
			pView->UpdateVisiblePages();

		cboPage.EnableWindow(true);
		int nPage = pView->GetCurrentPage();
		GetMainFrame()->UpdatePageCombo(pView);

		cboZoom.EnableWindow(true);
		int nZoomType = pView->GetZoomType();
		double fZoom = pView->GetZoom();
		GetMainFrame()->UpdateZoomCombo(nZoomType, fZoom);
	}
	else if (pActivateWnd == NULL)
	{
		cboPage.ResetContent();
		cboPage.EnableWindow(false);

		cboZoom.SetWindowText(_T("100%"));
		cboZoom.EnableWindow(false);
	}
}

void CChildFrame::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	BOOL bMaximized = false;
	CMDIChildWnd* pActive = GetMainFrame()->MDIGetActive(&bMaximized);

	if (bMaximized)
		GetMainFrame()->LockWindowUpdate();

	CMDIChildWnd::OnWindowPosChanged(lpwndpos);

	if (bMaximized)
		GetMainFrame()->UnlockWindowUpdate();

	if (pActive == this && IsWindowVisible() && !IsIconic())
		CAppSettings::bChildMaximized = !!IsZoomed();

	if (m_bCreated)
		m_wndSplitter.UpdateNavPane();

	OnUpdateFrameTitle(TRUE);
}

void CChildFrame::ActivateFrame(int nCmdShow)
{
	m_bActivating = true;

	if (CAppSettings::bChildMaximized)
		nCmdShow = SW_SHOWMAXIMIZED;

	CMDIChildWnd::ActivateFrame(nCmdShow);

	m_bActivating = false;
	GetDjVuView()->UpdateVisiblePages();
}

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	m_wndSplitter.CreateStatic(this, 1, 2);

	m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CNavPaneWnd),
		CSize(100, 0), pContext);
	m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CDjVuView),
		CSize(0, 0), pContext);

	m_bCreated = true;

	SetActiveView(GetDjVuView());
	GetDjVuView()->SetFocus();
	m_wndSplitter.UpdateNavPane();

	return TRUE;
}

void CChildFrame::CreateNavPanes()
{
	CNavPaneWnd* pNavPane = GetNavPane();
	CDjVuDoc* pDoc = (CDjVuDoc*)GetActiveDocument();

	if (pDoc->GetBookmarks() != NULL)
	{
		m_pBookmarksView = new CBookmarksView();
		m_pBookmarksView->Create(NULL, NULL, WS_VISIBLE | WS_TABSTOP | WS_CHILD
			| TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP
			| TVS_SHOWSELALWAYS | TVS_TRACKSELECT, CRect(), pNavPane, 1);
		pNavPane->AddTab(LoadString(IDS_BOOKMARKS_TAB), m_pBookmarksView);
		m_pBookmarksView->SetDocument(pDoc);
	}

	m_pThumbnailsView = new CThumbnailsView();
	m_pThumbnailsView->Create(NULL, NULL, WS_VISIBLE | WS_TABSTOP | WS_CHILD
		| WS_HSCROLL | WS_VSCROLL, CRect(), pNavPane, 2);
	pNavPane->AddTab(LoadString(IDS_THUMBNAILS_TAB), m_pThumbnailsView);
	m_pThumbnailsView->SetDocument(pDoc);
}

void CChildFrame::OnCollapsePane()
{
	m_wndSplitter.CollapseNavPane(true);
	GetDjVuView()->SetFocus();
}

void CChildFrame::OnExpandPane()
{
	m_wndSplitter.CollapseNavPane(false);
}

CDjVuView* CChildFrame::GetDjVuView()
{
	if (!m_bCreated)
		return NULL;

	return static_cast<CDjVuView*>(m_wndSplitter.GetPane(0, 1));
}

CNavPaneWnd* CChildFrame::GetNavPane()
{
	return m_wndSplitter.GetNavPane();
}

void CChildFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// update our parent window first
	GetMDIFrame()->OnUpdateFrameTitle(bAddToTitle);

	if (IsZoomed())
	{
		// Fool Windows so that it will not add child frame title
		// in square brackets to the main window title
		AfxSetWindowText(m_hWnd, _T(""));
		return;
	}

	CDocument* pDocument = GetActiveDocument();
	if (bAddToTitle)
	{
		TCHAR szText[256 + _MAX_PATH];
		if (pDocument == NULL)
			_tcsncpy(szText, m_strTitle, _countof(szText));
		else
			_tcsncpy(szText, pDocument->GetTitle(), _countof(szText));

		// set title if changed, but don't remove completely
		AfxSetWindowText(m_hWnd, szText);
	}
}

void CChildFrame::OnUpdateFrameMenu(
		BOOL bActivate, CWnd* pActivateWnd, HMENU hMenuAlt)
{
	// Fixed MFC's CMDIChildWnd::OnUpdateFrameMenu
	// Do not pass our Windows menu to Win32, we will build window list ourselves
	// Takes into account localized menus

	CMDIFrameWnd* pFrame = GetMDIFrame();
	if (hMenuAlt == NULL && bActivate)
	{
		// attempt to get default menu from document
		CDocument* pDoc = GetActiveDocument();
		if (pDoc != NULL)
			hMenuAlt = pDoc->GetDefaultMenu();
	}

	// use default menu stored in frame if none from document
	// use localized menu if localization is enabled
	if (hMenuAlt == NULL)
	{
		hMenuAlt = m_hMenuShared;
		if (CAppSettings::bLocalized)
			hMenuAlt = CAppSettings::hDjVuMenu;
	}

	if (hMenuAlt != NULL && bActivate)
	{
		ASSERT(pActivateWnd == this);

		// activating child, set parent menu
		::SendMessage(pFrame->m_hWndMDIClient, WM_MDISETMENU, (WPARAM)hMenuAlt, NULL);
	}
	else if (hMenuAlt != NULL && !bActivate && pActivateWnd == NULL)
	{
		HMENU hMainMenu = pFrame->m_hMenuDefault;
		if (CAppSettings::bLocalized)
			hMainMenu = CAppSettings::hDefaultMenu;

		// destroying last child
		::SendMessage(pFrame->m_hWndMDIClient, WM_MDISETMENU, (WPARAM)hMainMenu, NULL);
	}
	else
	{
		// refresh MDI Window menu (even if non-shared menu)
		::SendMessage(pFrame->m_hWndMDIClient, WM_MDIREFRESHMENU, 0, 0);
	}
}

CDocument* CChildFrame::GetActiveDocument()
{
	CDjVuView* pView = GetDjVuView();
	return (pView != NULL ? pView->GetDocument() : NULL);
}

BOOL CChildFrame::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void CChildFrame::OnClose()
{
	if (GetMainFrame()->m_pFullscreenWnd != NULL)
		return;

	// Begin process of stopping all threads simultaneously for faster closing
	GetDjVuView()->StopDecoding();
	GetThumbnailsView()->StopDecoding();

	CMDIChildWnd::OnClose();
}

CSearchResultsView* CChildFrame::GetResultsView()
{
	CNavPaneWnd* pNavPane = GetNavPane();

	if (m_pResultsView == NULL)
	{
		CDjVuDoc* pDoc = (CDjVuDoc*)GetActiveDocument();

		m_pResultsView = new CSearchResultsView();
		m_pResultsView->Create(NULL, NULL, WS_VISIBLE | WS_TABSTOP | WS_CHILD
			| TVS_HASBUTTONS | TVS_DISABLEDRAGDROP | TVS_INFOTIP
			| TVS_SHOWSELALWAYS | TVS_TRACKSELECT, CRect(), pNavPane, 3);
		pNavPane->AddTab(LoadString(IDS_SEARCH_RESULTS_TAB), m_pResultsView);
		m_pResultsView->SetDocument(pDoc);
		m_pResultsView->OnInitialUpdate();
	}

	pNavPane->ActivateTab(m_pResultsView);
	pNavPane->UpdateWindow();

	return m_pResultsView;
}

void CChildFrame::OnLanguageChanged()
{
	CNavPaneWnd* pNavPane = GetNavPane();

	if (m_pThumbnailsView != NULL)
		pNavPane->SetTabName(m_pThumbnailsView, LoadString(IDS_THUMBNAILS_TAB));

	if (m_pBookmarksView != NULL)
		pNavPane->SetTabName(m_pBookmarksView, LoadString(IDS_BOOKMARKS_TAB));

	if (m_pResultsView != NULL)
		pNavPane->SetTabName(m_pResultsView, LoadString(IDS_SEARCH_RESULTS_TAB));

	if (GetMainFrame()->MDIGetActive() == this)
		OnMDIActivate(true, this, NULL);
}

void CChildFrame::OnNcPaint()
{
	BOOL bMaximized = false;
	GetMainFrame()->MDIGetActive(&bMaximized);
	if (bMaximized)
		return;

	CMDIChildWnd::OnNcPaint();
}
