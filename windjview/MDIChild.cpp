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
#include "MDIChild.h"

#include "DjVuDoc.h"
#include "DjVuSource.h"
#include "BookmarksView.h"
#include "ThumbnailsView.h"
#include "PageIndexWnd.h"
#include "SearchResultsView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static int s_nSplitterWidth = 6;
static HCURSOR s_hCursorSplitter = NULL;


// CMDIChild

IMPLEMENT_DYNCREATE(CMDIChild, CWnd)

CMDIChild::CMDIChild()
	: m_pContentWnd(NULL), m_pDocument(NULL), m_bError(false), m_pThumbnailsView(NULL),
	  m_pContentsTree(NULL), m_pResultsView(NULL), m_pBookmarksTree(NULL),
	  m_pPageIndexWnd(NULL), m_nSplitterPos(0), m_bDragging(false)
{
	UpdateMetrics();

	m_nExpandedNavWidth = max(theApp.GetAppSettings()->nNavPaneWidth, CNavPaneWnd::s_nMinExpandedWidth);
	m_bNavCollapsed = theApp.GetAppSettings()->bNavPaneCollapsed;
	m_bNavHidden = theApp.GetAppSettings()->bNavPaneHidden;
}

CMDIChild::~CMDIChild()
{
}


BEGIN_MESSAGE_MAP(CMDIChild, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_MESSAGE_VOID(WM_INITIALUPDATE, OnInitialUpdate)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_MESSAGE_VOID(WM_EXPAND_NAV, OnExpandNav)
	ON_MESSAGE_VOID(WM_COLLAPSE_NAV, OnCollapseNav)
	ON_MESSAGE(WM_CLICKED_NAV_TAB, OnClickedNavTab)
	ON_WM_SHOWWINDOW()
	ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()


// CMDIChild message handlers

int CMDIChild::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CCreateContext* pContext = (CCreateContext*) lpCreateStruct->lpCreateParams;
	if (pContext->m_pCurrentDoc == NULL)
	{
		// Creating an empty MDIChild without a document
		m_bNavHidden = true;
		return 0;
	}

	if (!m_navPane.Create(NULL, NULL, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			CRect(0, 0, 0, 0), this, 1))
		return -1;

	// Note: can be a CWnd with PostNcDestroy self cleanup
	ASSERT(pContext != NULL && pContext->m_pNewViewClass != NULL);
	m_pContentWnd = (CWnd*)pContext->m_pNewViewClass->CreateObject();
	if (m_pContentWnd == NULL)
	{
		TRACE(_T("Warning: Dynamic create of content window %hs failed.\n"),
			pContext->m_pNewViewClass->m_lpszClassName);
		return -1;
	}
	ASSERT_KINDOF(CWnd, m_pContentWnd);

	if (!m_pContentWnd->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, 2, pContext))
	{
		TRACE(_T("Warning: could not create content window for MDI child.\n"));
		return -1;
	}

	m_pDocument = (CDjVuDoc*) pContext->m_pCurrentDoc;

	m_navPane.ShowWindow(m_bNavHidden ? SW_HIDE : SW_SHOW);

	m_nSplitterPos = CNavPaneWnd::s_nTabsWidth;
	if (!m_bNavCollapsed)
		m_nSplitterPos = m_nExpandedNavWidth;

	theApp.AddObserver(this);

	return 0;
}

void CMDIChild::OnDestroy()
{
	if (m_pDocument != NULL && m_pDocument->GetSource() != NULL)
		m_pDocument->GetSource()->GetSettings()->RemoveObserver(this);

	theApp.RemoveObserver(this);

	CWnd::OnDestroy();
}

void CMDIChild::OnInitialUpdate()
{
	if (m_pDocument == NULL)
		return;

	DjVuSource* pSource = m_pDocument->GetSource();
	pSource->GetSettings()->AddObserver(this);

	if (pSource->GetContents() != NULL)
	{
		m_pContentsTree = new CBookmarksView(pSource);
		m_pContentsTree->Create(NULL, NULL, WS_CHILD | WS_TABSTOP | TVS_HASLINES
			| TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP
			| TVS_SHOWSELALWAYS | TVS_TRACKSELECT, CRect(0, 0, 0, 0), &m_navPane, 1);
		m_navPane.AddTab(LoadString(IDS_CONTENTS_TAB), m_pContentsTree);
		m_navPane.SetTabSettings(m_pContentsTree, true);
		m_pContentsTree->LoadContents();
	}

	if (pSource->IsDictionary())
	{
		m_pPageIndexWnd = new CPageIndexWnd();
		m_pPageIndexWnd->Create(NULL, NULL, WS_CHILD | WS_TABSTOP,
				CRect(0, 0, 0, 0), &m_navPane, 2);
		if (m_pPageIndexWnd->InitPageIndex(pSource))
		{
			m_navPane.AddTab(LoadString(IDS_PAGE_INDEX_TAB), m_pPageIndexWnd);
			m_navPane.SetTabBorder(m_pPageIndexWnd, false);
		}
		else
		{
			m_pPageIndexWnd->DestroyWindow();
			m_pPageIndexWnd = NULL;
		}
	}

	m_pThumbnailsView = new CThumbnailsView(pSource);
	m_pThumbnailsView->Create(NULL, NULL, WS_CHILD | WS_TABSTOP,
			CRect(0, 0, 0, 0), &m_navPane, 3);
	m_navPane.AddTab(LoadString(IDS_THUMBNAILS_TAB), m_pThumbnailsView);
	m_navPane.SetTabSettings(m_pThumbnailsView, true);

	if (!pSource->GetSettings()->bookmarks.empty())
	{
		CBookmarksView* pBookmarks = GetBookmarksTree(false);
		pBookmarks->LoadUserBookmarks();
	}

	int nOpenTab = pSource->GetSettings()->nOpenSidebarTab;
	if (nOpenTab == DocSettings::Thumbnails)
		m_navPane.ActivateTab(m_pThumbnailsView, false);
	else if (nOpenTab == DocSettings::Contents && m_pContentsTree != NULL)
		m_navPane.ActivateTab(m_pContentsTree, false);
	else if (nOpenTab == DocSettings::Bookmarks && m_pBookmarksTree != NULL)
		m_navPane.ActivateTab(m_pBookmarksTree, false);
	else if (nOpenTab == DocSettings::PageIndex && m_pPageIndexWnd != NULL)
		m_navPane.ActivateTab(m_pPageIndexWnd, false);

	m_navPane.AddObserver(this);
	m_navPane.UpdateObservers(SIDEBAR_TAB_CHANGED);
}

CSearchResultsView* CMDIChild::GetSearchResults(bool bActivate)
{
	VERIFY(m_pDocument != NULL);

	if (m_pResultsView == NULL)
	{
		m_pResultsView = new CSearchResultsView();
		m_pResultsView->Create(NULL, NULL, WS_CHILD | WS_TABSTOP
			| TVS_HASBUTTONS | TVS_DISABLEDRAGDROP | TVS_INFOTIP
			| TVS_SHOWSELALWAYS | TVS_TRACKSELECT, CRect(0, 0, 0, 0), &m_navPane, 4);
		m_navPane.AddTab(LoadString(IDS_SEARCH_RESULTS_TAB), m_pResultsView);
		m_pResultsView->OnInitialUpdate();
	}

	if (bActivate)
	{
		m_navPane.ActivateTab(m_pResultsView);
		m_navPane.UpdateWindow();
	}

	return m_pResultsView;
}

CBookmarksView* CMDIChild::GetBookmarksTree(bool bActivate)
{
	VERIFY(m_pDocument != NULL);

	if (m_pBookmarksTree == NULL)
	{
		DjVuSource* pSource = m_pDocument->GetSource();

		m_pBookmarksTree = new CBookmarksView(pSource);
		m_pBookmarksTree->Create(NULL, NULL, WS_CHILD | WS_TABSTOP | TVS_HASLINES
			| TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP
			| TVS_SHOWSELALWAYS | TVS_TRACKSELECT, CRect(0, 0, 0, 0), &m_navPane, 5);
		m_navPane.AddTab(LoadString(IDS_BOOKMARKS_TAB), m_pBookmarksTree);
		m_navPane.SetTabSettings(m_pBookmarksTree, true);
		m_pBookmarksTree->EnableEditing();
	}

	if (bActivate)
	{
		m_navPane.ActivateTab(m_pBookmarksTree);
		m_navPane.UpdateWindow();
	}

	return m_pBookmarksTree;
}

void CMDIChild::OnSize(UINT nType, int cx, int cy)
{
	if (cx > 0 && cy > 0)
		RecalcLayout();

	CWnd::OnSize(nType, cx, cy);
}

void CMDIChild::RecalcLayout()
{
	CSize szClient = ::GetClientSize(this);
	int nWidth = szClient.cx - s_nSplitterWidth;

	InvalidateRect(CRect(0, 0, szClient.cx, 1));

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
			if (!m_bDragging)
			{
				m_nExpandedNavWidth = m_nSplitterPos;
				theApp.GetAppSettings()->nNavPaneWidth = m_nExpandedNavWidth;
			}
		}

		m_rcNavPane.left = 0;
		m_rcNavPane.top = 1;
		m_rcNavPane.right = m_nSplitterPos;
		m_rcNavPane.bottom = max(m_rcNavPane.top, szClient.cy);

		m_rcSplitter.left = m_nSplitterPos;
		m_rcSplitter.top = 0;
		m_rcSplitter.right = m_nSplitterPos + s_nSplitterWidth;
		m_rcSplitter.bottom = szClient.cy;

		m_rcContent.left = m_nSplitterPos + s_nSplitterWidth;
		m_rcContent.top = 1;
		m_rcContent.right = max(m_rcContent.left, szClient.cx);
		m_rcContent.bottom = max(m_rcContent.top, szClient.cy);

		m_navPane.MoveWindow(m_rcNavPane);
		InvalidateRect(m_rcSplitter);
	}
	else
	{
		m_rcContent.left = 0;
		m_rcContent.top = 1;
		m_rcContent.right = szClient.cx;
		m_rcContent.bottom = max(m_rcContent.top, szClient.cy);
	}

	if (m_pContentWnd != NULL)
		m_pContentWnd->MoveWindow(m_rcContent);
}

void CMDIChild::HideNavPane(bool bHide)
{
	if (bHide == m_bNavHidden)
		return;

	m_bNavHidden = bHide;
	RecalcLayout();

	m_navPane.ShowWindow(bHide ? SW_HIDE : SW_SHOW);
	UpdateWindow();

	if (m_bNavHidden)
		m_pContentWnd->SetFocus();
}

void CMDIChild::OnPaint()
{
	CPaintDC paintDC(this);

	CRect rcClient = ::GetClientRect(this);

	CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNSHADOW));
	CPen* pOldPen = paintDC.SelectObject(&pen);

	if (!m_bNavHidden)
	{
		paintDC.MoveTo(0, 0);
		paintDC.LineTo(m_rcSplitter.left, 0);
		paintDC.MoveTo(m_rcSplitter.right, 0);
		paintDC.LineTo(rcClient.right, 0);

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
		paintDC.MoveTo(0, 0);
		paintDC.LineTo(rcClient.right, 0);
	}

	if (!m_pDocument)
	{
		CRect rcContent = rcClient;
		rcContent.DeflateRect(0, 1, 0, 0);
		paintDC.FillSolidRect(rcContent, ::GetSysColor(COLOR_BTNFACE));

		if (m_bError)
		{
			CString strText = LoadString(IDS_FAILED_TO_OPEN) + m_strPathName;
			CFont* pOldFont = paintDC.SelectObject(&m_font);
			paintDC.SetTextColor(GetSysColor(COLOR_BTNTEXT));
			paintDC.SetBkMode(TRANSPARENT);
			rcContent.DeflateRect(0, 16, 0, 0);
			paintDC.DrawText(strText, rcContent, DT_CENTER | DT_NOPREFIX | DT_WORDBREAK);
			paintDC.SelectObject(pOldFont);
		}
	}

	paintDC.SelectObject(pOldPen);
}

BOOL CMDIChild::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

CNavPaneWnd* CMDIChild::GetNavPane()
{
	return &m_navPane;
}

CWnd* CMDIChild::GetContent()
{
	return m_pContentWnd;
}

bool CMDIChild::IsPlaceholder()
{
	return m_pDocument != NULL || !m_bError;
}

void CMDIChild::SetError(const CString& strPathName)
{
	m_bError = true;
	m_strPathName = strPathName;
	Invalidate();
}

void CMDIChild::CollapseNavPane(bool bCollapse)
{
	m_bNavCollapsed = bCollapse;
	theApp.GetAppSettings()->bNavPaneCollapsed = m_bNavCollapsed;

	m_nSplitterPos = m_bNavCollapsed ? CNavPaneWnd::s_nTabsWidth : m_nExpandedNavWidth;
	RecalcLayout();

	if (m_bNavCollapsed)
		m_pContentWnd->SetFocus();
}

void CMDIChild::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bDragging)
	{
		m_nSplitterPos = m_nOrigSplitterPos + point.x - m_ptDragStart.x;
		RecalcLayout();
		::RedrawWindow(m_hWnd, NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN);
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CMDIChild::OnLButtonDown(UINT nFlags, CPoint point)
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

void CMDIChild::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDragging)
		StopDragging();

	CWnd::OnLButtonUp(nFlags, point);
}

void CMDIChild::StopDragging()
{
	m_bDragging = false;
	ReleaseCapture();

	if (!m_bNavCollapsed)
	{
		m_nExpandedNavWidth = m_nSplitterPos;
		theApp.GetAppSettings()->nNavPaneWidth = m_nExpandedNavWidth;
	}
}

BOOL CMDIChild::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (s_hCursorSplitter == NULL)
		s_hCursorSplitter = theApp.LoadCursor(IDC_CURSOR_SPLIT_HORZ);

	if (m_bDragging)
	{
		SetCursor(s_hCursorSplitter);
		return true;
	}

	if (nHitTest == HTCLIENT)
	{
		CPoint ptCursor;
		::GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);

		if (!m_bNavHidden && m_rcSplitter.PtInRect(ptCursor))
		{
			SetCursor(s_hCursorSplitter);
			return true;
		}
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CMDIChild::OnExpandNav()
{
	HideNavPane(false);
	CollapseNavPane(false);
}

void CMDIChild::OnCollapseNav()
{
	CollapseNavPane(true);
}

LRESULT CMDIChild::OnClickedNavTab(WPARAM wparam, LPARAM lParam)
{
	CWnd* pTab = (CWnd*) lParam;
	if (pTab == m_navPane.GetActiveTab() && !IsNavPaneCollapsed())
	{
		OnCollapseNav();
		return 0;
	}

	return 1;
}

void CMDIChild::PostNcDestroy()
{
	CWnd::PostNcDestroy();

	delete this;
}

void CMDIChild::OnUpdate(const Observable* source, const Message* message)
{
	if (message->code == APP_LANGUAGE_CHANGED)
	{
		if (m_pThumbnailsView != NULL)
			m_navPane.SetTabName(m_pThumbnailsView, LoadString(IDS_THUMBNAILS_TAB));

		if (m_pContentsTree != NULL)
			m_navPane.SetTabName(m_pContentsTree, LoadString(IDS_CONTENTS_TAB));

		if (m_pBookmarksTree != NULL)
			m_navPane.SetTabName(m_pBookmarksTree, LoadString(IDS_BOOKMARKS_TAB));

		if (m_pResultsView != NULL)
			m_navPane.SetTabName(m_pResultsView, LoadString(IDS_SEARCH_RESULTS_TAB));

		if (m_pPageIndexWnd != NULL)
			m_navPane.SetTabName(m_pPageIndexWnd, LoadString(IDS_PAGE_INDEX_TAB));
	}
	else if (message->code == SIDEBAR_TAB_CHANGED)
	{
		CWnd* pActiveTab = m_navPane.GetActiveTab();
		DocSettings* pSettings = m_pDocument->GetSource()->GetSettings();

		if (pActiveTab != NULL)
		{
			if (pActiveTab == m_pThumbnailsView)
				pSettings->nOpenSidebarTab = DocSettings::Thumbnails;
			else if (pActiveTab == m_pContentsTree)
				pSettings->nOpenSidebarTab = DocSettings::Contents;
			else if (pActiveTab == m_pBookmarksTree)
				pSettings->nOpenSidebarTab = DocSettings::Bookmarks;
			else if (pActiveTab == m_pPageIndexWnd)
				pSettings->nOpenSidebarTab = DocSettings::PageIndex;
			else
				pSettings->nOpenSidebarTab = -1;
		}
		else
			pSettings->nOpenSidebarTab = -1;
	}
}

void CMDIChild::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CWnd::OnShowWindow(bShow, nStatus);

	CWnd* pParent = GetParent();
	if (pParent == NULL || pParent != NULL && pParent->IsWindowVisible())
		SendMessageToVisibleDescendants(m_hWnd, WM_SHOWPARENT, bShow, nStatus);
}

void CMDIChild::UpdateMetrics()
{
	CreateSystemDialogFont(m_font);
}

void CMDIChild::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	UpdateMetrics();

	CSize szClient = ::GetClientSize(this);
	OnSize(0, szClient.cx, szClient.cy);
}
