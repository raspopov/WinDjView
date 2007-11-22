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
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CChildFrame construction/destruction

CChildFrame::CChildFrame()
	: m_bCreated(false), m_bActivating(false), m_pThumbnailsView(NULL),
	  m_pContentsWnd(NULL), m_pResultsView(NULL), m_bFirstShow(true),
	  m_pBookmarksWnd(NULL), m_pPageIndexWnd(NULL), m_nStartupPage(-1),
	  m_bLockingUpdate(false)
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

		GetMainFrame()->OnUpdate(pView, &Message(VIEW_ACTIVATED));
	}
	else if (pActivateWnd == NULL)
	{
		GetMainFrame()->OnUpdate(NULL, &Message(VIEW_ACTIVATED));
	}
}

void CChildFrame::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	bool bLockingUpdate = false;

	BOOL bMaximized = false;
	CMDIFrameWnd* pFrame = GetMDIFrame();
	CMDIChildWnd* pActive = pFrame->MDIGetActive(&bMaximized);

	if (!m_bLockingUpdate && pActive == this && bMaximized)
	{
		bLockingUpdate = true;
		m_bLockingUpdate = true;
		::SendMessage(pFrame->m_hWndMDIClient, WM_SETREDRAW, 0, 0);
	}

	CMDIChildWnd::OnWindowPosChanged(lpwndpos);

	if (bLockingUpdate)
	{
		m_bLockingUpdate = false;
		::SendMessage(pFrame->m_hWndMDIClient, WM_SETREDRAW, 1, 0);
		::RedrawWindow(pFrame->m_hWndMDIClient, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ALLCHILDREN);
	}

	if (pActive == this && IsWindowVisible() && !IsIconic())
		theApp.GetAppSettings()->bChildMaximized = !!IsZoomed();

	OnUpdateFrameTitle(true);
}

void CChildFrame::ActivateFrame(int nCmdShow)
{
	bool bLockingUpdate = false;
	m_bActivating = true;

	CMDIFrameWnd* pFrame = GetMDIFrame();
	CDjVuView* pView = GetDjVuView();

	if (theApp.GetAppSettings()->bChildMaximized)
	{
		nCmdShow = SW_SHOWMAXIMIZED;

		if (!m_bLockingUpdate)
		{
			bLockingUpdate = true;
			m_bLockingUpdate = true;

			pFrame->UpdateWindow();
			::SendMessage(pFrame->m_hWndMDIClient, WM_SETREDRAW, 0, 0);
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

		// Another reason for doing it here is that the main window is
		// still locked so there won't be any unnecessary redraws

		if (theApp.GetAppSettings()->bRestoreView && m_nStartupPage >= 0 && m_nStartupPage < pView->GetPageCount())
			pView->ScrollToPage(m_nStartupPage, m_ptStartupOffset);
	}

	pView->UpdateVisiblePages();

	if (bLockingUpdate)
	{
		m_bLockingUpdate = false;
		::SendMessage(pFrame->m_hWndMDIClient, WM_SETREDRAW, 1, 0);
		::RedrawWindow(pFrame->m_hWndMDIClient, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ALLCHILDREN);
	}

	m_bActivating = false;
	m_bFirstShow = false;
}

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	m_wndSplitter.CreateStatic(this, 1, 2);

	m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CNavPaneWnd),
		CSize(100, 0), pContext);

//	m_wndDynSplitter.Create(&m_wndSplitter, 2, 2, CSize(10, 10), pContext,
//		WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | SPLS_DYNAMIC_SPLIT,
//		m_wndSplitter.IdFromRowCol(0, 1));

	m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CDjVuView),
		CSize(0, 0), pContext);

	m_bCreated = true;

	SetActiveView(GetDjVuView());
	GetDjVuView()->SetFocus();
	m_wndSplitter.UpdateNavPane();

	theApp.AddObserver(this);

	return TRUE;
}

void CChildFrame::OnDestroy()
{
	theApp.RemoveObserver(this);

	CFrameWnd::OnDestroy();
}

void CChildFrame::SaveStartupPage()
{
	DjVuSource* pSource = GetDjVuView()->GetDocument()->GetSource();
	m_nStartupPage = pSource->GetSettings()->nPage;
	m_ptStartupOffset = pSource->GetSettings()->ptOffset;
}

void CChildFrame::CreateNavPanes()
{
	CNavPaneWnd* pNavPane = GetNavPane();
	CDjVuView* pDjVuView = GetDjVuView();
	DjVuSource* pSource = pDjVuView->GetDocument()->GetSource();

	if (pSource->GetContents() != NULL)
	{
		m_pContentsWnd = new CBookmarksWnd(pSource);
		m_pContentsWnd->Create(NULL, NULL, WS_VISIBLE | WS_TABSTOP | WS_CHILD
			| TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP
			| TVS_SHOWSELALWAYS | TVS_TRACKSELECT, CRect(), pNavPane, 1);
		pNavPane->AddTab(LoadString(IDS_CONTENTS_TAB), m_pContentsWnd);
		m_pContentsWnd->LoadContents();
		m_pContentsWnd->AddObserver(pDjVuView);
	}

	if (pSource->IsDictionary())
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

	m_pThumbnailsView = new CThumbnailsView(pSource);
	m_pThumbnailsView->Create(NULL, NULL, WS_VISIBLE | WS_TABSTOP | WS_CHILD
		| WS_HSCROLL | WS_VSCROLL, CRect(), pNavPane, 3);
	pNavPane->AddTab(LoadString(IDS_THUMBNAILS_TAB), m_pThumbnailsView);
	m_pThumbnailsView->AddObserver(pDjVuView);
	pDjVuView->AddObserver(m_pThumbnailsView);

	if (!pSource->GetSettings()->bookmarks.empty())
	{
		CBookmarksWnd* pBookmarks = GetBookmarks(false);
		pBookmarks->LoadUserBookmarks();
	}
}

void CChildFrame::OnCollapsePane()
{
	m_wndSplitter.CollapseNavPane(true);
	GetDjVuView()->SetFocus();
}

void CChildFrame::OnExpandPane()
{
	m_wndSplitter.HideNavPane(false);
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

void CChildFrame::HideNavPane(bool bHide)
{
	m_wndSplitter.HideNavPane(bHide);
	GetNavPane()->ShowWindow(bHide ? SW_HIDE : SW_SHOWNA);
}

bool CChildFrame::IsNavPaneHidden() const
{
	return m_wndSplitter.IsNavPaneHidden();
}

bool CChildFrame::IsNavPaneCollapsed() const
{
	return m_wndSplitter.IsNavPaneCollapsed();
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

	CMDIChildWnd::OnClose();

	theApp.SaveSettings();
}

CSearchResultsView* CChildFrame::GetSearchResults(bool bActivate)
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

	if (bActivate)
	{
		pNavPane->ActivateTab(m_pResultsView);
		pNavPane->UpdateWindow();
	}

	return m_pResultsView;
}

CBookmarksWnd* CChildFrame::GetBookmarks(bool bActivate)
{
	CNavPaneWnd* pNavPane = GetNavPane();

	if (m_pBookmarksWnd == NULL)
	{
		CDjVuView* pDjVuView = GetDjVuView();
		DjVuSource* pSource = pDjVuView->GetDocument()->GetSource();

		m_pBookmarksWnd = new CBookmarksWnd(pSource);
		m_pBookmarksWnd->Create(NULL, NULL, WS_VISIBLE | WS_TABSTOP | WS_CHILD
			| TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP
			| TVS_SHOWSELALWAYS | TVS_TRACKSELECT, CRect(), pNavPane, 5);
		pNavPane->AddTab(LoadString(IDS_BOOKMARKS_TAB), m_pBookmarksWnd);
		m_pBookmarksWnd->AddObserver(pDjVuView);
		m_pBookmarksWnd->EnableEditing();
	}

	if (bActivate)
	{
		pNavPane->ActivateTab(m_pBookmarksWnd);
		pNavPane->UpdateWindow();
	}

	return m_pBookmarksWnd;
}

void CChildFrame::OnUpdate(const Observable* source, const Message* message)
{
	if (message->code == APP_LANGUAGE_CHANGED)
	{
		CNavPaneWnd* pNavPane = GetNavPane();

		if (m_pThumbnailsView != NULL)
			pNavPane->SetTabName(m_pThumbnailsView, LoadString(IDS_THUMBNAILS_TAB));

		if (m_pContentsWnd != NULL)
			pNavPane->SetTabName(m_pContentsWnd, LoadString(IDS_CONTENTS_TAB));

		if (m_pBookmarksWnd != NULL)
			pNavPane->SetTabName(m_pBookmarksWnd, LoadString(IDS_BOOKMARKS_TAB));

		if (m_pResultsView != NULL)
			pNavPane->SetTabName(m_pResultsView, LoadString(IDS_SEARCH_RESULTS_TAB));
	}
}

void CChildFrame::OnNcPaint()
{
	BOOL bMaximized = false;
	GetMDIFrame()->MDIGetActive(&bMaximized);
	if (bMaximized)
		return;

	CMDIChildWnd::OnNcPaint();
}

CMainFrame* CChildFrame::GetMainFrame()
{
	return (CMainFrame*) GetMDIFrame();
}
