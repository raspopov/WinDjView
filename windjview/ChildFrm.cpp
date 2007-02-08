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

#include "ChildFrm.h"
#include "MainFrm.h"
#include "DjVuView.h"
#include "ThumbnailsView.h"
#include "BookmarksWnd.h"
#include "SearchResultsView.h"
#include "PageIndexWnd.h"
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
	ON_WM_NCPAINT()
END_MESSAGE_MAP()


// CChildFrame construction/destruction

CChildFrame::CChildFrame()
	: m_bCreated(false), m_bActivating(false), m_pThumbnailsView(NULL),
	  m_pBookmarksWnd(NULL), m_pResultsView(NULL), m_bFirstShow(true),
	  m_nStartupPage(-1)
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

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIChildWnd::PreCreateWindow(cs))
		return false;

	if (theApp.GetAppSettings()->bChildMaximized)
		cs.style |= WS_MAXIMIZE;

	return true;
}

void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	if (bActivate)
	{
		CDjVuView* pView = GetDjVuView();

		if (!m_bActivating)
			pView->UpdateVisiblePages();

		GetMainFrame()->OnUpdate(pView, &ViewActivated());
	}
	else if (pActivateWnd == NULL)
	{
		GetMainFrame()->OnUpdate(NULL, &ViewActivated());
	}
}

void CChildFrame::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	BOOL bMaximized = false;
	CMDIChildWnd* pActive = GetMainFrame()->MDIGetActive(&bMaximized);

	if (!m_bActivating && bMaximized && theApp.m_bInitialized)
		GetMainFrame()->LockWindowUpdate();

	CMDIChildWnd::OnWindowPosChanged(lpwndpos);

	if (!m_bActivating && bMaximized && theApp.m_bInitialized)
		GetMainFrame()->UnlockWindowUpdate();

	if (pActive == this && IsWindowVisible() && !IsIconic())
		theApp.GetAppSettings()->bChildMaximized = !!IsZoomed();

	if (m_bCreated)
		m_wndSplitter.UpdateNavPane();

	OnUpdateFrameTitle(TRUE);
}

void CChildFrame::ActivateFrame(int nCmdShow)
{
	m_bActivating = true;

	CDjVuView* pView = GetDjVuView();

	if (theApp.GetAppSettings()->bChildMaximized)
	{
		nCmdShow = SW_SHOWMAXIMIZED;

		if (theApp.m_bInitialized)
		{
			GetMainFrame()->SetRedraw(false);
			GetMainFrame()->LockWindowUpdate();
		}
	}

	CMDIChildWnd::ActivateFrame(nCmdShow);

	if (m_bFirstShow)
	{
		// Startup page is displayed AFTER ActivateFrame(), not during the
		// OnInitialUpdate() call. In the latter, the client rectangle is INCORRECT
		// (not maximized). Child frame is maximized only inside the ActivateFrame(),
		// which is called after OnInitialUpdate() during the InitialUpdateFrame().
		// So we restore the startup page here, to ensure correct positioning.

		// This is also a good place to do it, because the main window is
		// still locked here so there won't be any unnecessary redraws

		if (theApp.GetAppSettings()->bRestoreView && m_nStartupPage >= 0 && m_nStartupPage < pView->GetPageCount())
			pView->ScrollToPage(m_nStartupPage, m_ptStartupOffset);
	}

	pView->UpdateVisiblePages();

	if (theApp.GetAppSettings()->bChildMaximized && theApp.m_bInitialized)
	{
		GetMainFrame()->UnlockWindowUpdate();
		GetMainFrame()->SetRedraw(true);

		GetMainFrame()->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ALLCHILDREN); 
	}

	m_bActivating = false;

	if (m_bFirstShow)
		GetThumbnailsView()->Start();

	m_bFirstShow = false;
}

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	m_wndSplitter.CreateStatic(this, 1, 2);

	m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CNavPaneWnd),
		CSize(100, 0), pContext);
/*
	m_wndDynSplitter.Create(&m_wndSplitter, 2, 2, CSize(10, 10), pContext,
		WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | SPLS_DYNAMIC_SPLIT,
		m_wndSplitter.IdFromRowCol(0, 1));
*/
	m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CDjVuView),
		CSize(0, 0), pContext);

	m_bCreated = true;

	SetActiveView(GetDjVuView());
	GetDjVuView()->SetFocus();
	m_wndSplitter.UpdateNavPane();

	theApp.AddObserver(this);

	return TRUE;
}

void CChildFrame::SaveStartupPage()
{
	DjVuSource* pSource = GetDjVuView()->GetDocument()->GetSource();
	m_nStartupPage = pSource->GetUserData()->nPage;
	m_ptStartupOffset = pSource->GetUserData()->ptOffset;
}

void CChildFrame::CreateNavPanes()
{
	CNavPaneWnd* pNavPane = GetNavPane();
	DjVuSource* pSource = ((CDjVuDoc*)GetActiveDocument())->GetSource();
	CDjVuView* pDjVuView = GetDjVuView();

	if (pSource->GetBookmarks() != NULL)
	{
		m_pBookmarksWnd = new CBookmarksWnd();
		m_pBookmarksWnd->Create(NULL, NULL, WS_VISIBLE | WS_TABSTOP | WS_CHILD
			| TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP
			| TVS_SHOWSELALWAYS | TVS_TRACKSELECT, CRect(), pNavPane, 1);
		pNavPane->AddTab(LoadString(IDS_BOOKMARKS_TAB), m_pBookmarksWnd);
		m_pBookmarksWnd->InitBookmarks(pSource);
		m_pBookmarksWnd->AddObserver(pDjVuView);
	}

	if (pSource->GetPageIndex().length() > 0)
	{
		m_pPageIndexWnd = new CPageIndexWnd();
		m_pPageIndexWnd->Create(NULL, NULL, WS_VISIBLE | WS_TABSTOP | WS_CHILD,
			CRect(), pNavPane, 2);
		if (m_pPageIndexWnd->InitPageIndex(pSource))
		{
			pNavPane->AddTab(LoadString(IDS_PAGE_INDEX_TAB), m_pPageIndexWnd);
			pNavPane->SetTabBorder(m_pPageIndexWnd, false);
			m_pPageIndexWnd->AddObserver(pDjVuView);
		}
		else
		{
			m_pPageIndexWnd->DestroyWindow();
			m_pPageIndexWnd = NULL;
		}
	}

	m_pThumbnailsView = new CThumbnailsView();
	m_pThumbnailsView->Create(NULL, NULL, WS_VISIBLE | WS_TABSTOP | WS_CHILD
		| WS_HSCROLL | WS_VSCROLL, CRect(), pNavPane, 3);
	pNavPane->AddTab(LoadString(IDS_THUMBNAILS_TAB), m_pThumbnailsView);
	m_pThumbnailsView->SetSource(pSource);
	m_pThumbnailsView->AddObserver(pDjVuView);
	pDjVuView->AddObserver(m_pThumbnailsView);
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
//	return static_cast<CDjVuView*>(static_cast<CSplitterWnd*>(m_wndSplitter.GetPane(0, 1))->GetPane(0, 0));
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
		if (theApp.GetAppSettings()->bLocalized)
			hMenuAlt = theApp.GetAppSettings()->hDjVuMenu;
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
		if (theApp.GetAppSettings()->bLocalized)
			hMainMenu = theApp.GetAppSettings()->hDefaultMenu;

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
	if (GetMainFrame()->IsFullscreenMode())
		return;

	// Begin process of stopping all threads simultaneously for faster closing
	GetDjVuView()->StopDecoding();
	GetThumbnailsView()->StopDecoding();

	CMDIChildWnd::OnClose();

	theApp.SaveSettings();
}

CSearchResultsView* CChildFrame::GetResultsView()
{
	CNavPaneWnd* pNavPane = GetNavPane();

	if (m_pResultsView == NULL)
	{
		CDjVuView* pDjVuView = GetDjVuView();

		m_pResultsView = new CSearchResultsView();
		m_pResultsView->Create(NULL, NULL, WS_VISIBLE | WS_TABSTOP | WS_CHILD
			| TVS_HASBUTTONS | TVS_DISABLEDRAGDROP | TVS_INFOTIP
			| TVS_SHOWSELALWAYS | TVS_TRACKSELECT, CRect(), pNavPane, 4);
		pNavPane->AddTab(LoadString(IDS_SEARCH_RESULTS_TAB), m_pResultsView);
		m_pResultsView->OnInitialUpdate();
		m_pResultsView->AddObserver(pDjVuView);
	}

	pNavPane->ActivateTab(m_pResultsView);
	pNavPane->UpdateWindow();

	return m_pResultsView;
}

void CChildFrame::OnUpdate(const Observable* source, const Message* message)
{
	if (message->code == APP_LANGUAGE_CHANGED)
	{
		CNavPaneWnd* pNavPane = GetNavPane();

		if (m_pThumbnailsView != NULL)
			pNavPane->SetTabName(m_pThumbnailsView, LoadString(IDS_THUMBNAILS_TAB));

		if (m_pBookmarksWnd != NULL)
			pNavPane->SetTabName(m_pBookmarksWnd, LoadString(IDS_BOOKMARKS_TAB));

		if (m_pResultsView != NULL)
			pNavPane->SetTabName(m_pResultsView, LoadString(IDS_SEARCH_RESULTS_TAB));
	}
}

void CChildFrame::OnNcPaint()
{
	BOOL bMaximized = false;
	GetMainFrame()->MDIGetActive(&bMaximized);
	if (bMaximized)
		return;

	CMDIChildWnd::OnNcPaint();
}
