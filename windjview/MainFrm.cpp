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
#include "DjVuView.h"
#include "DjVuDoc.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "AppSettings.h"
#include "ThumbnailsView.h"
#include "FullscreenWnd.h"
#include "MagnifyWnd.h"
#include "MyDocTemplate.h"

#include <dde.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_ACTIVATE()
	ON_COMMAND(ID_VIEW_TOOLBAR, OnViewToolbar)
	ON_COMMAND(ID_VIEW_STATUS_BAR, OnViewStatusBar)
	ON_COMMAND(ID_VIEW_SIDEBAR, OnViewSidebar)
	ON_COMMAND(ID_VIEW_DICTBAR, OnViewDictBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SIDEBAR, OnUpdateViewSidebar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DICTBAR, OnUpdateViewDictBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STATUS_BAR, OnUpdateViewStatusBar)
	ON_WM_WINDOWPOSCHANGED()
	ON_CBN_SELCHANGE(IDC_PAGENUM, OnChangePage)
	ON_CONTROL(CBN_FINISHEDIT, IDC_PAGENUM, OnChangePageEdit)
	ON_CONTROL(CBN_CANCELEDIT, IDC_PAGENUM, OnCancelChange)
	ON_CBN_SELCHANGE(IDC_ZOOM, OnChangeZoom)
	ON_CONTROL(CBN_FINISHEDIT, IDC_ZOOM, OnChangeZoomEdit)
	ON_CONTROL(CBN_CANCELEDIT, IDC_ZOOM, OnCancelChange)
	ON_MESSAGE(WM_DDE_EXECUTE, OnDDEExecute)
	ON_COMMAND(ID_EDIT_FIND, OnEditFind)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateEditFind)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_ACTIVATE_FIRST, OnUpdateWindowList)
	ON_COMMAND_RANGE(ID_WINDOW_ACTIVATE_FIRST, ID_WINDOW_ACTIVATE_LAST, OnActivateWindow)
	ON_COMMAND(ID_VIEW_BACK, OnViewBack)
	ON_COMMAND(ID_VIEW_FORWARD, OnViewForward)
	ON_COMMAND(ID_VIEW_BACK_SHORTCUT, OnViewBack)
	ON_COMMAND(ID_VIEW_FORWARD_SHORTCUT, OnViewForward)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BACK, OnUpdateViewBack)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FORWARD, OnUpdateViewForward)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_ADJUST, OnUpdateStatusAdjust)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_MODE, OnUpdateStatusMode)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_PAGE, OnUpdateStatusPage)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_SIZE, OnUpdateStatusSize)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_APPCOMMAND, OnAppCommand)
	ON_CBN_SETFOCUS(IDC_LOOKUP, OnLookupSetFocus)
	ON_CONTROL(CBN_FINISHEDIT, IDC_LOOKUP, OnLookup)
	ON_CONTROL(CBN_CANCELEDIT, IDC_LOOKUP, OnCancelChange)
	ON_CBN_SELCHANGE(IDC_DICTIONARY, OnChangeDictionary)
	ON_CBN_SELCHANGE(IDC_LANGUAGE_LIST, OnChangeLanguage)
	ON_COMMAND(ID_DICTIONARY_INFO, OnDictionaryInfo)
	ON_COMMAND(ID_DICTIONARY_NEXT, OnDictionaryNext)
	ON_COMMAND(ID_DICTIONARY_PREV, OnDictionaryPrev)
	ON_COMMAND(ID_DICTIONARY_LOOKUP, OnLookup)
	ON_COMMAND(ID_FOCUS_TO_LOOKUP, OnFocusToLookup)
	ON_UPDATE_COMMAND_UI(ID_DICTIONARY_INFO, OnUpdateDictionaryInfo)
	ON_UPDATE_COMMAND_UI(ID_DICTIONARY_NEXT, OnUpdateDictionaryNext)
	ON_UPDATE_COMMAND_UI(ID_DICTIONARY_PREV, OnUpdateDictionaryPrev)
	ON_UPDATE_COMMAND_UI(ID_DICTIONARY_LOOKUP, OnUpdateDictionaryLookup)
	ON_COMMAND(ID_WINDOW_CASCADE, OnWindowCascade)
	ON_COMMAND(ID_WINDOW_TILE_HORZ, OnWindowTileHorz)
	ON_COMMAND(ID_WINDOW_TILE_VERT, OnWindowTileVert)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,
	ID_INDICATOR_ADJUST,
	ID_INDICATOR_MODE,
	ID_INDICATOR_PAGE,
	ID_INDICATOR_SIZE
};

const int s_nIndicatorPage = 3;


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
	: m_pFindDlg(NULL), m_historyPos(m_history.end()), m_pFullscreenWnd(NULL),
	  m_pMagnifyWnd(NULL), m_nCurLang(CB_ERR), m_nCurDict(CB_ERR)
{
	m_childMenu.LoadMenu(IDR_DjVuTYPE);
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT,
			WS_CHILD | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.GetToolBar().LoadToolBar(IDR_MAINFRAME))
	{
		TRACE(_T("Failed to create toolbar\n"));
		return -1;      // fail to create
	}

	if (!m_wndDictBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT,
			WS_CHILD | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_SIZE_DYNAMIC) ||
		!m_wndDictBar.GetToolBar().LoadToolBar(IDR_DICTIONARIES_BAR))
	{
		TRACE(_T("Failed to create dictionaries bar\n"));
		return -1;      // fail to create
	}

	if (!m_wndTabBar.Create(this, WS_CHILD | CBRS_TOP, IDR_TAB_BAR))
	{
		TRACE(_T("Failed to create tab bar\n"));
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.CreateEx(this, SBT_TOOLTIPS) ||
		!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
	{
		TRACE(_T("Failed to create status bar\n"));
		return -1;      // fail to create
	}

	// Create bold font for combo boxes in toolbar
	CreateSystemDialogFont(m_font);

	LOGFONT lf;
	m_font.GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	m_boldFont.CreateFontIndirect(&lf);

	InitToolBar();
	InitDictBar();

	theApp.AddObserver(this);
	m_wndTabBar.AddObserver(this);

	return 0;
}

void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	if (nState == WA_ACTIVE)
		theApp.ChangeMainWnd(this);

	CMDIFrameWnd::OnActivate(nState, pWndOther, bMinimized);
}

void CMainFrame::InitToolBar()
{
	m_wndToolBar.GetToolBar().SetHeight(30);
	m_wndToolBar.GetToolBar().SetWindowText(LoadString(IDS_TOOLBAR_TITLE));
	m_wndToolBar.SetWindowText(LoadString(IDS_TOOLBAR_TITLE));

	int nComboPage = m_wndToolBar.GetToolBar().CommandToIndex(ID_VIEW_NEXTPAGE) - 1;
	m_wndToolBar.GetToolBar().SetButtonInfo(nComboPage, IDC_PAGENUM, TBBS_SEPARATOR, 65);

	CRect rcItem;
	m_wndToolBar.GetToolBar().GetItemRect(nComboPage, rcItem);
	rcItem.DeflateRect(3, 0);
	rcItem.bottom += 400;

	m_cboPage.Create(WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_VSCROLL
			| CBS_DROPDOWN | CBS_AUTOHSCROLL, rcItem, &m_wndToolBar.GetToolBar(), IDC_PAGENUM);
	m_cboPage.SetFont(&m_boldFont);
	m_cboPage.SetItemHeight(-1, 16);
	m_cboPage.GetEditCtrl()->SetInteger();

	int nComboZoom = m_wndToolBar.GetToolBar().CommandToIndex(ID_ZOOM_IN) - 1;
	m_wndToolBar.GetToolBar().SetButtonInfo(nComboZoom, IDC_ZOOM, TBBS_SEPARATOR, 110);

	m_wndToolBar.GetToolBar().GetItemRect(nComboZoom, rcItem);
	rcItem.DeflateRect(3, 0);
	rcItem.bottom += 400;

	m_cboZoom.Create(WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_VSCROLL
			| CBS_DROPDOWN | CBS_AUTOHSCROLL, rcItem, &m_wndToolBar.GetToolBar(), IDC_ZOOM);
	m_cboZoom.SetFont(&m_boldFont);
	m_cboZoom.SetItemHeight(-1, 16);
	m_cboZoom.GetEditCtrl()->SetReal();
	m_cboZoom.GetEditCtrl()->SetPercent();
}

void CMainFrame::InitDictBar()
{
	m_wndDictBar.GetToolBar().SetHeight(30);
	m_wndDictBar.GetToolBar().SetWindowText(LoadString(IDS_DICTBAR_TITLE));
	m_wndDictBar.SetWindowText(LoadString(IDS_DICTBAR_TITLE));

	HBITMAP hbm = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_DICTIONARIES_BAR),
			IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	CBitmap bm;
	bm.Attach(hbm);

	m_imageListDict.Create(16, 15, ILC_COLOR24 | ILC_MASK, 0, 1);
	m_imageListDict.Add(&bm, RGB(192, 192, 192));
	m_wndDictBar.GetToolBar().GetToolBarCtrl().SetImageList(&m_imageListDict);

	m_wndDictBar.InsertLabel(0, IDC_LOOKUP_LABEL, &m_font);

	TBBUTTON btn;
	ZeroMemory(&btn, sizeof(btn));
	btn.iBitmap = 135;
	btn.idCommand = IDC_LOOKUP;
	btn.fsStyle = TBSTYLE_SEP;

	CRect rcItem;
	m_wndDictBar.GetToolBarCtrl().InsertButton(1, &btn);
	m_wndDictBar.GetToolBar().GetItemRect(1, rcItem);
	rcItem.DeflateRect(3, 0);
	rcItem.bottom += 160;

	m_cboLookup.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL
			| CBS_DROPDOWN | CBS_AUTOHSCROLL, rcItem, &m_wndDictBar.GetToolBar(), IDC_LOOKUP);
	m_cboLookup.SetExtendedStyle(CBES_EX_CASESENSITIVE | CBES_EX_NOEDITIMAGE,
			CBES_EX_CASESENSITIVE | CBES_EX_NOEDITIMAGE);
	m_cboLookup.GetComboBoxCtrl()->ModifyStyle(CBS_SORT | CBS_NOINTEGRALHEIGHT, CBS_AUTOHSCROLL);
	m_cboLookup.SetFont(&m_font);
	m_cboLookup.SetItemHeight(-1, 16);

	m_cboLookup.SetWindowText(theApp.GetAppSettings()->strFind);

	// Separator
	btn.iBitmap = 0;
	btn.idCommand = IDC_STATIC;
	m_wndDictBar.GetToolBarCtrl().InsertButton(3, &btn);

	m_wndDictBar.InsertLabel(4, IDC_LANGUAGE_LIST_LABEL, &m_font);

	// Language combo
	btn.iBitmap = 250;
	btn.idCommand = IDC_LANGUAGE_LIST;
	m_wndDictBar.GetToolBarCtrl().InsertButton(5, &btn);

	m_wndDictBar.GetToolBar().GetItemRect(5, rcItem);
	rcItem.DeflateRect(3, 0);
	rcItem.bottom += 160;

	m_cboLangs.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL
			| CBS_DROPDOWNLIST | CBS_AUTOHSCROLL, rcItem, &m_wndDictBar.GetToolBar(), IDC_LANGUAGE_LIST);
	m_cboLangs.SetFont(&m_boldFont);
	m_cboLangs.SetItemHeight(-1, 16);

	// Separator
	btn.iBitmap = 0;
	btn.idCommand = IDC_STATIC;
	m_wndDictBar.GetToolBarCtrl().InsertButton(6, &btn);

	m_wndDictBar.InsertLabel(7, IDC_DICTIONARY_LABEL, &m_font);

	// Dictionary combo
	btn.iBitmap = 350;
	btn.idCommand = IDC_DICTIONARY;
	m_wndDictBar.GetToolBarCtrl().InsertButton(8, &btn);

	m_wndDictBar.GetToolBar().GetItemRect(8, rcItem);
	rcItem.DeflateRect(3, 0);
	rcItem.bottom += 160;

	m_cboDict.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL
			| CBS_DROPDOWNLIST | CBS_AUTOHSCROLL, rcItem, &m_wndDictBar.GetToolBar(), IDC_DICTIONARY);
	m_cboDict.SetFont(&m_boldFont);
	m_cboDict.SetItemHeight(-1, 16);

	m_nCurLang = theApp.GetAppSettings()->nCurLang;
	m_nCurDict = theApp.GetAppSettings()->nCurDict;
	UpdateLangAndDict(NULL, true);
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIFrameWnd::PreCreateWindow(cs))
		return false;

	cs.style |= FWS_PREFIXTITLE | FWS_ADDTOTITLE;

	return true;
}

void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	if ((GetStyle() & FWS_ADDTOTITLE) == 0)
		return;

	CMDIChildWnd* pActiveChild = MDIGetActive();
	if (pActiveChild != NULL)
	{
		CDocument* pDocument = pActiveChild->GetActiveDocument();
		if (pDocument != NULL)
		{
			UpdateFrameTitleForDocument(pDocument->GetTitle());
			return;
		}
	}

	UpdateFrameTitleForDocument(NULL);
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers


void CMainFrame::OnViewToolbar()
{
	CMDIFrameWnd::OnBarCheck(ID_VIEW_TOOLBAR);
	theApp.GetAppSettings()->bToolbar = !!m_wndToolBar.IsWindowVisible();
}

void CMainFrame::OnViewStatusBar()
{
	CMDIFrameWnd::OnBarCheck(ID_VIEW_STATUS_BAR);
	theApp.GetAppSettings()->bStatusBar = !!m_wndStatusBar.IsWindowVisible();
}

void CMainFrame::OnViewSidebar()
{
	bool bHide = !theApp.GetAppSettings()->bNavPaneHidden;
	theApp.GetAppSettings()->bNavPaneHidden = bHide;

	CChildFrame* pFrame = (CChildFrame*) MDIGetActive();
	if (pFrame == NULL)
		return;

	pFrame->HideNavPane(bHide);
}

void CMainFrame::OnViewDictBar()
{
	ShowControlBar(&m_wndDictBar, !m_wndDictBar.IsWindowVisible(), false);
	theApp.GetAppSettings()->bDictBar = !!m_wndDictBar.IsWindowVisible();
}

void CMainFrame::OnUpdateViewSidebar(CCmdUI* pCmdUI)
{
	CChildFrame* pFrame = (CChildFrame*) MDIGetActive();

	pCmdUI->Enable(pFrame != NULL);
	pCmdUI->SetCheck(pFrame != NULL && !pFrame->IsNavPaneHidden());
}

void CMainFrame::OnUpdateViewDictBar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndDictBar.IsWindowVisible());

	if (theApp.GetDictLangsCount() == 0 && pCmdUI->m_pMenu != NULL)
	{
		pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nIndex, MF_BYPOSITION);
		pCmdUI->m_nIndex--;
		pCmdUI->m_nIndexMax = pCmdUI->m_pMenu->GetMenuItemCount();
	}
}

void CMainFrame::OnUpdateViewStatusBar(CCmdUI* pCmdUI)
{
	OnUpdateControlBarMenu(pCmdUI);

	if (theApp.GetDictLangsCount() > 0
			&& pCmdUI->m_pMenu != NULL
			&& pCmdUI->m_pMenu->GetMenuItemID(pCmdUI->m_nIndex + 1) != ID_VIEW_DICTBAR)
	{
		pCmdUI->m_pMenu->InsertMenu(pCmdUI->m_nIndex + 1, MF_BYPOSITION | MF_STRING,
				ID_VIEW_DICTBAR, LoadString(IDR_DICTIONARIES_BAR));
		pCmdUI->m_nIndexMax = pCmdUI->m_pMenu->GetMenuItemCount();
	}
}

void CMainFrame::UpdateToolbars()
{
	CAppSettings* pSettings = theApp.GetAppSettings();

	ShowControlBar(&m_wndToolBar, pSettings->bToolbar, false);
	ShowControlBar(&m_wndDictBar, pSettings->bDictBar && theApp.GetDictLangsCount() > 0, false);
	ShowControlBar(&m_wndStatusBar, pSettings->bStatusBar, false);
	ShowControlBar(&m_wndTabBar, !pSettings->bTopLevelDocs && m_wndTabBar.GetTabCount() > 0, false);
}

void CMainFrame::UpdateSettings()
{
	CRect rect;
	GetWindowRect(rect);

	CAppSettings* pSettings = theApp.GetAppSettings();
	if (IsZoomed())
	{
		pSettings->bWindowMaximized = true;
	}
	else
	{
		pSettings->bWindowMaximized = false;
		pSettings->nWindowPosX = rect.left;
		pSettings->nWindowPosY = rect.top;
		pSettings->nWindowWidth = rect.right - rect.left;
		pSettings->nWindowHeight = rect.bottom - rect.top;
	}
}

void CMainFrame::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CMDIFrameWnd::OnWindowPosChanged(lpwndpos);

	if (IsWindowVisible() && !IsIconic() && theApp.m_bInitialized)
		UpdateSettings();
}

void CMainFrame::OnChangePage()
{
	CChildFrame* pFrame = (CChildFrame*)MDIGetActive();
	if (pFrame == NULL)
		return;

	CDjVuView* pView = pFrame->GetDjVuView();

	int nPage = m_cboPage.GetCurSel();
	if (nPage >= 0 && nPage < pView->GetPageCount())
		pView->GoToPage(nPage);

	pView->SetFocus();
}

void CMainFrame::OnChangePageEdit()
{
	CChildFrame* pFrame = (CChildFrame*)MDIGetActive();
	if (pFrame == NULL)
		return;

	CDjVuView* pView = pFrame->GetDjVuView();

	CString strPage;
	m_cboPage.GetWindowText(strPage);

	int nPage;
	if (_stscanf(strPage, _T("%d"), &nPage) == 1)
	{
		if (nPage >= 1 && nPage <= pView->GetPageCount())
			pView->GoToPage(nPage - 1);
	}

	pView->SetFocus();
}

void CMainFrame::OnCancelChange()
{
	CChildFrame* pFrame = (CChildFrame*)MDIGetActive();
	if (pFrame == NULL)
	{
		::SetFocus(m_hWndMDIClient);
		return;
	}

	CDjVuView* pView = pFrame->GetDjVuView();
	pView->SetFocus();
}

void CMainFrame::UpdateZoomCombo(const CDjVuView* pView)
{
	if (pView == NULL)
		return;

	int nZoomType = pView->GetZoomType();
	double fZoom = pView->GetZoom();

	if (m_cboZoom.GetCount() == 0)
	{
		m_cboZoom.AddString(_T("50%"));
		m_cboZoom.AddString(_T("75%"));
		m_cboZoom.AddString(_T("100%"));
		m_cboZoom.AddString(_T("200%"));
		m_cboZoom.AddString(_T("400%"));
		m_cboZoom.AddString(LoadString(IDS_FIT_WIDTH));
		m_cboZoom.AddString(LoadString(IDS_FIT_HEIGHT));
		m_cboZoom.AddString(LoadString(IDS_FIT_PAGE));
		m_cboZoom.AddString(LoadString(IDS_ACTUAL_SIZE));
		m_cboZoom.AddString(LoadString(IDS_STRETCH));
	}

	if (nZoomType == CDjVuView::ZoomFitWidth)
		m_cboZoom.SelectString(-1, LoadString(IDS_FIT_WIDTH));
	else if (nZoomType == CDjVuView::ZoomFitHeight)
		m_cboZoom.SelectString(-1, LoadString(IDS_FIT_HEIGHT));
	else if (nZoomType == CDjVuView::ZoomFitPage)
		m_cboZoom.SelectString(-1, LoadString(IDS_FIT_PAGE));
	else if (nZoomType == CDjVuView::ZoomActualSize)
		m_cboZoom.SelectString(-1, LoadString(IDS_ACTUAL_SIZE));
	else if (nZoomType == CDjVuView::ZoomStretch)
		m_cboZoom.SelectString(-1, LoadString(IDS_STRETCH));
	else
		m_cboZoom.SetWindowText(FormatDouble(fZoom) + _T("%"));
}

void CMainFrame::UpdatePageCombo(const CDjVuView* pView)
{
	if (pView == NULL)
		return;

	if (pView->GetPageCount() != m_cboPage.GetCount())
	{
		m_cboPage.ResetContent();
		for (int i = 1; i <= pView->GetPageCount(); ++i)
			m_cboPage.AddString(FormatString(_T("%d"), i));
	}

	int nPage = pView->GetCurrentPage();
	if (m_cboPage.GetCurSel() != nPage)
		m_cboPage.SetCurSel(nPage);
}

void CMainFrame::OnChangeZoom()
{
	CChildFrame* pFrame = (CChildFrame*)MDIGetActive();
	if (pFrame == NULL)
		return;

	CDjVuView* pView = pFrame->GetDjVuView();

	int nSel = m_cboZoom.GetCurSel();
	CString strZoom;
	m_cboZoom.GetLBText(nSel, strZoom);

	if (strZoom == LoadString(IDS_FIT_WIDTH))
		pView->ZoomTo(CDjVuView::ZoomFitWidth);
	else if (strZoom == LoadString(IDS_FIT_HEIGHT))
		pView->ZoomTo(CDjVuView::ZoomFitHeight);
	else if (strZoom == LoadString(IDS_FIT_PAGE))
		pView->ZoomTo(CDjVuView::ZoomFitPage);
	else if (strZoom == LoadString(IDS_ACTUAL_SIZE))
		pView->ZoomTo(CDjVuView::ZoomActualSize);
	else if (strZoom == LoadString(IDS_STRETCH))
		pView->ZoomTo(CDjVuView::ZoomStretch);
	else
	{
		double fZoom;
		if (_stscanf(strZoom, _T("%lf"), &fZoom) == 1)
			pView->ZoomTo(CDjVuView::ZoomPercent, fZoom);
	}

	pView->SetFocus();
}

void CMainFrame::OnChangeZoomEdit()
{
	CChildFrame* pFrame = (CChildFrame*)MDIGetActive();
	if (pFrame == NULL)
		return;

	CDjVuView* pView = pFrame->GetDjVuView();

	CString strZoom;
	m_cboZoom.GetWindowText(strZoom);

	if (strZoom == LoadString(IDS_FIT_WIDTH))
		pView->ZoomTo(CDjVuView::ZoomFitWidth);
	else if (strZoom == LoadString(IDS_FIT_HEIGHT))
		pView->ZoomTo(CDjVuView::ZoomFitHeight);
	else if (strZoom == LoadString(IDS_FIT_PAGE))
		pView->ZoomTo(CDjVuView::ZoomFitPage);
	else if (strZoom == LoadString(IDS_ACTUAL_SIZE))
		pView->ZoomTo(CDjVuView::ZoomActualSize);
	else if (strZoom == LoadString(IDS_STRETCH))
		pView->ZoomTo(CDjVuView::ZoomStretch);
	else
	{
		double fZoom;
		if (_stscanf(strZoom, _T("%lf"), &fZoom) == 1)
			pView->ZoomTo(CDjVuView::ZoomPercent, fZoom);
	}

	pView->SetFocus();
}

void CMainFrame::OnLookupSetFocus()
{
	theApp.InitSearchHistory(m_cboLookup);
}

void CMainFrame::OnFocusToLookup()
{
	if (theApp.GetDictLangsCount() == 0)
		return;

	if (!m_wndDictBar.IsWindowVisible())
	{
		ShowControlBar(&m_wndDictBar, true, false);
		theApp.GetAppSettings()->bDictBar = true;
	}

	m_cboLookup.SetFocus();
}

void CMainFrame::OnLookup()
{
	if (m_nCurDict == CB_ERR)
		return;

	theApp.UpdateSearchHistory(m_cboLookup);

	CString strLookup;
	m_cboLookup.GetWindowText(strLookup);
	DictionaryInfo* pInfo = (DictionaryInfo*) m_cboDict.GetItemDataPtr(m_nCurDict);

	theApp.Lookup(strLookup, pInfo);
}

void CMainFrame::OnChangeDictionary()
{
	m_nCurDict = m_cboDict.GetCurSel();
}

void CMainFrame::OnChangeLanguage()
{
	if (m_nCurLang != m_cboLangs.GetCurSel())
	{
		m_nCurLang = m_cboLangs.GetCurSel();
		m_nCurDict = 0;
		UpdateDictCombo();
	}
}

void CMainFrame::OnDictionaryInfo()
{
	if (m_nCurDict == CB_ERR)
		return;

	DictionaryInfo* pInfo = (DictionaryInfo*) m_cboDict.GetItemDataPtr(m_nCurDict);
	theApp.OpenDocument(pInfo->strPathName, "#1");
}

void CMainFrame::OnDictionaryNext()
{
	if (m_nCurDict == CB_ERR || m_nCurDict >= m_cboDict.GetCount() - 1)
		return;
	m_cboDict.SetCurSel(++m_nCurDict);

	OnLookup();
}

void CMainFrame::OnDictionaryPrev()
{
	if (m_nCurDict == CB_ERR || m_nCurDict <= 0)
		return;
	m_cboDict.SetCurSel(--m_nCurDict);

	OnLookup();
}

void CMainFrame::OnUpdateDictionaryInfo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_nCurDict != CB_ERR);
}

void CMainFrame::OnUpdateDictionaryNext(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_nCurDict != CB_ERR && m_nCurDict < m_cboDict.GetCount() - 1);
}

void CMainFrame::OnUpdateDictionaryPrev(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_nCurDict != CB_ERR && m_nCurDict > 0);
}

void CMainFrame::OnUpdateDictionaryLookup(CCmdUI* pCmdUI)
{
	CString strLookup;
	m_cboLookup.GetWindowText(strLookup);

	pCmdUI->Enable(m_nCurDict != CB_ERR && !strLookup.IsEmpty());
}

void CMainFrame::UpdateLangAndDict(const CDjVuView* pView, bool bReset)
{
	if (pView == NULL)
	{
		CMDIChildWnd* pActive = MDIGetActive();
		pView = (pActive != NULL ? (CDjVuView*) pActive->GetActiveView() : NULL);
	}

	if (bReset)
	{
		m_cboLangs.ResetContent();
		m_cboDict.ResetContent();

		if (theApp.GetDictLangsCount() > 0)
		{
			for (int i = 0; i < theApp.GetDictLangsCount(); ++i)
				m_cboLangs.AddString(theApp.GetLangFrom(i) + _T(" -> ") + theApp.GetLangTo(i));

			if (m_nCurLang == CB_ERR || m_nCurLang < 0 || m_nCurLang >= theApp.GetDictLangsCount())
			{
				m_nCurLang = 0;
				m_nCurDict = 0;
			}
			if (m_nCurDict == CB_ERR || m_nCurDict < 0 || m_nCurDict >= theApp.GetDictionaryCount(m_nCurLang))
				m_nCurDict = 0;
		}
	}

	if (theApp.GetDictLangsCount() == 0)
	{
		m_nCurLang = CB_ERR;
		m_nCurDict = CB_ERR;
		return;
	}

	DictionaryInfo* pInfo = NULL;
	DjVuSource* pSource = NULL;
	if (pView != NULL)
	{
		pSource = const_cast<CDjVuView*>(pView)->GetDocument()->GetSource();
		pInfo = pSource->GetDictionaryInfo();
	}

	if (!bReset && (pView == NULL || !pInfo->bInstalled))
		return;

	if (pView != NULL && pInfo->bInstalled)
	{
		int nLang, nDict;
		if (FindAppDictionary(pSource->GetFileName(), nLang, nDict))
		{
			if (!bReset && nLang == m_nCurLang)
			{
				m_cboDict.SetCurSel(m_nCurDict = nDict);
				return;
			}

			m_nCurLang = nLang;
			m_nCurDict = nDict;
		}
	}

	m_cboLangs.SetCurSel(m_nCurLang);
	UpdateDictCombo();
}

bool CMainFrame::FindAppDictionary(const CString& strPathName, int& nLang, int& nDict)
{
	for (int i = 0; i < theApp.GetDictLangsCount(); ++i)
	{
		for (int j = 0; j < theApp.GetDictionaryCount(i); ++j)
		{
			if (AfxComparePath(strPathName, theApp.GetDictionaryInfo(i, j)->strPathName))
			{
				nLang = i;
				nDict = j;
				return true;
			}
		}
	}

	return false;
}

void CMainFrame::UpdateDictCombo()
{
	m_cboDict.ResetContent();
	for (int j = 0; j < theApp.GetDictionaryCount(m_nCurLang); ++j)
	{
		DictionaryInfo* pInfo = theApp.GetDictionaryInfo(m_nCurLang, j);
		int nIndex = m_cboDict.AddString(pInfo->strTitle);
		m_cboDict.SetItemDataPtr(nIndex, pInfo);
	}
	m_cboDict.SetCurSel(m_nCurDict);
}

LRESULT CMainFrame::OnDDEExecute(WPARAM wParam, LPARAM lParam)
{
	// From CFrameWnd::OnDDEExecute

	// unpack the DDE message
	unsigned int unused;
	HGLOBAL hData;
	//IA64: Assume DDE LPARAMs are still 32-bit
	VERIFY(UnpackDDElParam(WM_DDE_EXECUTE, lParam, &unused, (unsigned int*)&hData));

	// get the command string
	TCHAR szCommand[_MAX_PATH * 2];
	LPCTSTR lpsz = (LPCTSTR)GlobalLock(hData);
	int commandLength = lstrlen(lpsz);
	if (commandLength >= _countof(szCommand))
	{
		// The command would be truncated. This could be a security problem
		TRACE(_T("Warning: Command was ignored because it was too long.\n"));
		return 0;
	}
	// !!! MFC Bug Fix
	_tcsncpy(szCommand, lpsz, _countof(szCommand) - 1);
	szCommand[_countof(szCommand) - 1] = '\0';
	// !!!
	GlobalUnlock(hData);

	// acknowledge now - before attempting to execute
	::PostMessage((HWND)wParam, WM_DDE_ACK, (WPARAM)m_hWnd,
		//IA64: Assume DDE LPARAMs are still 32-bit
		ReuseDDElParam(lParam, WM_DDE_EXECUTE, WM_DDE_ACK,
		(UINT)0x8000, (UINT_PTR)hData));

	// don't execute the command when the window is disabled
	if (!IsWindowEnabled())
	{
		TRACE(_T("Warning: DDE command '%s' ignored because window is disabled.\n"), szCommand);
		return 0;
	}

	// execute the command
	if (!AfxGetApp()->OnDDECommand(szCommand))
		TRACE(_T("Error: failed to execute DDE command '%s'.\n"), szCommand);

	return 0L;
}

void CMainFrame::OnEditFind()
{
	if (m_pFindDlg == NULL)
	{
		m_pFindDlg = new CFindDlg();
		m_pFindDlg->Create(IDD_FIND, this);
		m_pFindDlg->CenterWindow();
	}

	m_pFindDlg->ShowWindow(SW_SHOW);
	m_pFindDlg->SetFocus();
	m_pFindDlg->GotoDlgCtrl(m_pFindDlg->GetDlgItem(IDC_FIND));
}

void CMainFrame::OnUpdateEditFind(CCmdUI* pCmdUI)
{
	CMDIChildWnd* pFrame = MDIGetActive();
	if (IsFullscreenMode() || pFrame == NULL)
	{
		pCmdUI->Enable(false);
		return;
	}

	CDjVuDoc* pDoc = (CDjVuDoc*)pFrame->GetActiveDocument();
	pCmdUI->Enable(pDoc->GetSource()->HasText());
}

void CMainFrame::HilightStatusMessage(LPCTSTR pszMessage)
{
	ShowControlBar(&m_wndStatusBar, true, false);
	theApp.GetAppSettings()->bStatusBar = true;

	m_wndStatusBar.SetHilightMessage(pszMessage);
}

void CMainFrame::OnUpdateWindowList(CCmdUI* pCmdUI)
{
	if (pCmdUI->m_pMenu == NULL)
		return;

	// Remove all window menu items
	CDocument* pActiveDoc = NULL;
	if (MDIGetActive() != NULL)
		pActiveDoc = MDIGetActive()->GetActiveDocument();

	const int nMaxWindows = 16;
	for (int i = 0; i < nMaxWindows; ++i)
		pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nID + i, MF_BYCOMMAND);

	int nDoc = 0;
	POSITION pos = theApp.GetFirstDocTemplatePosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = theApp.GetNextDocTemplate(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);

		POSITION posDoc = pTemplate->GetFirstDocPosition();
		while (posDoc != NULL)
		{
			CDocument* pDoc = pTemplate->GetNextDoc(posDoc);

			CString strText;
			if (nDoc >= 10)
				strText.Format(_T("%d "), nDoc + 1);
			else if (nDoc == 9)
				strText = _T("1&0 ");
			else
				strText.Format(_T("&%d "), nDoc + 1);

			CString strDocTitle = pDoc->GetTitle();
			strDocTitle.Replace(_T("&"), _T("&&"));
			strText += strDocTitle;

			int nFlags = MF_STRING;
			if (pDoc == pActiveDoc)
				nFlags |= MF_CHECKED;

			pCmdUI->m_pMenu->AppendMenu(nFlags, pCmdUI->m_nID + nDoc, strText);
			++nDoc;
		}
	}

	// update end menu count
	pCmdUI->m_nIndex = pCmdUI->m_pMenu->GetMenuItemCount();
	pCmdUI->m_nIndexMax = pCmdUI->m_pMenu->GetMenuItemCount();
	pCmdUI->m_bEnableChanged = TRUE;
}

void CMainFrame::OnActivateWindow(UINT nID)
{
	int nActivateDoc = nID - ID_WINDOW_ACTIVATE_FIRST;

	int nDoc = 0;
	POSITION pos = theApp.GetFirstDocTemplatePosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = theApp.GetNextDocTemplate(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);

		POSITION posDoc = pTemplate->GetFirstDocPosition();
		while (posDoc != NULL)
		{
			CDocument* pDoc = pTemplate->GetNextDoc(posDoc);
			if (nDoc == nActivateDoc)
			{
				CDjVuView* pView = ((CDjVuDoc*)pDoc)->GetDjVuView();
				CMainFrame* pMainFrame = pView->GetMainFrame();
				pMainFrame->ActivateFrame();
				CFrameWnd* pFrame = pView->GetParentFrame();
				pFrame->ActivateFrame();
				return;
			}

			++nDoc;
		}
	}
}

void CMainFrame::GoToHistoryPos(const HistoryPos& pos, const HistoryPos* pCurPos)
{
	CDjVuDoc* pDoc = theApp.FindOpenDocument(pos.strFileName);
	if (pDoc == NULL)
		pDoc = theApp.OpenDocument(pos.strFileName, "", false);
	if (pDoc == NULL)
		return;

	CDjVuView* pView = pDoc->GetDjVuView();
	if (IsFullscreenMode())
	{
		if (pView == m_pFullscreenWnd->GetOwner())
		{
			m_pFullscreenWnd->GetView()->GoToBookmark(pos.bookmark, CDjVuView::DoNotAdd);
			return;
		}
		else
		{
			m_pFullscreenWnd->Hide();
		}
	}

	pView->GetParentFrame()->ActivateFrame();

	if (pCurPos != NULL && pCurPos->bookmark.bZoom)
	{
		pView->ZoomTo(pCurPos->bookmark.nPrevZoomType, pCurPos->bookmark.fPrevZoom);
	}

	pView->GoToBookmark(pos.bookmark, CDjVuView::DoNotAdd);
}

void CMainFrame::OnViewBack()
{
	if (m_history.empty() || m_historyPos == m_history.begin())
		return;

	const HistoryPos* pCurPos = NULL;
	if (m_historyPos != m_history.end())
		pCurPos = &(*m_historyPos);

	const HistoryPos& pos = *(--m_historyPos);
	GoToHistoryPos(pos, pCurPos);
}

void CMainFrame::OnUpdateViewBack(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_history.empty() && m_historyPos != m_history.begin());
}

void CMainFrame::OnViewForward()
{
	if (m_history.empty())
		return;

	list<HistoryPos>::iterator it = m_history.end();
	if (m_historyPos == m_history.end() || m_historyPos == --it)
		return;

	const HistoryPos& pos = *(++m_historyPos);
	GoToHistoryPos(pos);
}

void CMainFrame::OnUpdateViewForward(CCmdUI* pCmdUI)
{
	if (m_history.empty())
	{
		pCmdUI->Enable(false);
	}
	else
	{
		list<HistoryPos>::iterator it = m_history.end();
		pCmdUI->Enable(m_historyPos != m_history.end() && m_historyPos != --it);
	}
}

bool CMainFrame::AddToHistory(CDjVuView* pView, bool bAlwaysEnableBack)
{
	HistoryPos pos;
	pos.strFileName = pView->GetDocument()->GetPathName();
	pView->CreateBookmarkFromView(pos.bmView);
	pos.bookmark = pos.bmView;

	if (AddToHistory(pos))
		return true;
	else if (!bAlwaysEnableBack)
		return false;
	else
	{
		++m_historyPos;
		return true;
	}
}

bool CMainFrame::AddToHistory(CDjVuView* pView, int nPage)
{
	HistoryPos pos;
	pos.strFileName = pView->GetDocument()->GetPathName();
	pView->CreateBookmarkFromView(pos.bmView);
	pView->CreateBookmarkFromPage(pos.bookmark, nPage);

	return AddToHistory(pos);
}

bool CMainFrame::AddToHistory(CDjVuView* pView, const Bookmark& bookmark, bool bForce)
{
	HistoryPos pos;
	pos.strFileName = pView->GetDocument()->GetPathName();
	pView->CreateBookmarkFromView(pos.bmView);
	pos.bookmark = bookmark;

	return AddToHistory(pos, bForce);
}

bool CMainFrame::AddToHistory(const HistoryPos& pos, bool bForce)
{
	ASSERT(pos.bmView.nLinkType == Bookmark::View);

	if (!m_history.empty())
	{
		if (m_historyPos == m_history.end())
			--m_historyPos;

		if (pos == *m_historyPos && !bForce)
			return false;

		++m_historyPos;
		m_history.erase(m_historyPos, m_history.end());
	}

	m_history.push_back(pos);

	m_historyPos = m_history.end();
	--m_historyPos;
	return true;
}

void CMainFrame::OnUpdateStatusAdjust(CCmdUI* pCmdUI)
{
	CDisplaySettings* pDisplaySettings = theApp.GetDisplaySettings();

	if (!pDisplaySettings->IsAdjusted() || MDIGetActive() == NULL)
	{
		m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_ADJUST, SBPS_DISABLED | SBPS_NOBORDERS, 0);
		pCmdUI->Enable(false);
		return;
	}

	CString strNewMessage;
	strNewMessage.LoadString(ID_INDICATOR_ADJUST);

	CStatusBarCtrl& status = m_wndStatusBar.GetStatusBarCtrl();
	if (status.GetText(1, 0) != strNewMessage)
	{
		pCmdUI->SetText(strNewMessage + _T("          "));
		status.SetText(strNewMessage + _T("          "), 1, 0);

		CWindowDC dc(&status);
		CFont* pOldFont = dc.SelectObject(status.GetFont());
		m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_ADJUST, 0, dc.GetTextExtent(strNewMessage).cx + 2);
		dc.SelectObject(pOldFont);
	}

	CString strNewTooltip, strTemp;

	int nBrightness = pDisplaySettings->GetBrightness();
	if (nBrightness != 0)
	{
		strTemp.Format(IDS_TOOLTIP_BRIGHTNESS, nBrightness);
		strNewTooltip += (!strNewTooltip.IsEmpty() ? _T(", ") : _T("")) + strTemp;
	}

	int nContrast = pDisplaySettings->GetContrast();
	if (nContrast != 0)
	{
		strTemp.Format(IDS_TOOLTIP_CONTRAST, nContrast);
		strNewTooltip += (!strNewTooltip.IsEmpty() ? _T(", ") : _T("")) + strTemp;
	}

	double fGamma = pDisplaySettings->GetGamma();
	if (fGamma != 1.0)
	{
		strTemp.Format(IDS_TOOLTIP_GAMMA, fGamma);
		strNewTooltip += (!strNewTooltip.IsEmpty() ? _T(", ") : _T("")) + strTemp;
	}

	if (status.GetTipText(1) != strNewTooltip)
		status.SetTipText(1, strNewTooltip);
}

void CMainFrame::OnUpdateStatusMode(CCmdUI* pCmdUI)
{
	CString strNewMessage;
	CMDIChildWnd* pActive = MDIGetActive();
	if (pActive != NULL)
	{
		CDjVuView* pView = (CDjVuView*)pActive->GetActiveView();
		ASSERT(pView);

		int nMode = pView->GetDisplayMode();
		switch (nMode)
		{
		case CDjVuView::BlackAndWhite:
			strNewMessage.LoadString(IDS_STATUS_BLACKANDWHITE);
			break;

		case CDjVuView::Foreground:
			strNewMessage.LoadString(IDS_STATUS_FOREGROUND);
			break;

		case CDjVuView::Background:
			strNewMessage.LoadString(IDS_STATUS_BACKGROUND);
			break;
		}
	}

	CStatusBarCtrl& status = m_wndStatusBar.GetStatusBarCtrl();
	if (strNewMessage.IsEmpty())
	{
		m_wndStatusBar.SetPaneInfo(2, ID_INDICATOR_MODE, SBPS_DISABLED | SBPS_NOBORDERS, 0);
		pCmdUI->Enable(false);
	}
	else if (status.GetText(2, 0) != strNewMessage)
	{
		pCmdUI->SetText(strNewMessage);
		status.SetText(strNewMessage, 2, 0);

		CWindowDC dc(&status);
		CFont* pOldFont = dc.SelectObject(status.GetFont());
		m_wndStatusBar.SetPaneInfo(2, ID_INDICATOR_MODE, 0, dc.GetTextExtent(strNewMessage).cx + 2);
		dc.SelectObject(pOldFont);
	}
}

void CMainFrame::OnUpdateStatusPage(CCmdUI* pCmdUI)
{
	CMDIChildWnd* pActive = MDIGetActive();
	if (pActive == NULL)
	{
		m_wndStatusBar.SetPaneInfo(s_nIndicatorPage, ID_INDICATOR_PAGE, SBPS_DISABLED | SBPS_NOBORDERS, 0);
		pCmdUI->Enable(false);
		return;
	}

	CDjVuView* pView = (CDjVuView*)pActive->GetActiveView();
	ASSERT(pView);

	CString strNewMessage;
	strNewMessage.Format(ID_INDICATOR_PAGE, pView->GetCurrentPage() + 1, pView->GetPageCount());

	CStatusBarCtrl& status = m_wndStatusBar.GetStatusBarCtrl();
	if (status.GetText(s_nIndicatorPage, 0) != strNewMessage)
	{
		pCmdUI->SetText(strNewMessage);
		status.SetText(strNewMessage, s_nIndicatorPage, 0);

		CWindowDC dc(&status);
		CFont* pOldFont = dc.SelectObject(status.GetFont());
		m_wndStatusBar.SetPaneInfo(s_nIndicatorPage, ID_INDICATOR_PAGE, 0, dc.GetTextExtent(strNewMessage).cx + 2);
		dc.SelectObject(pOldFont);
	}
}

void CMainFrame::OnUpdateStatusSize(CCmdUI* pCmdUI)
{
	CMDIChildWnd* pActive = MDIGetActive();
	if (pActive == NULL)
	{
		m_wndStatusBar.SetPaneInfo(4, ID_INDICATOR_SIZE, SBPS_DISABLED | SBPS_NOBORDERS, 0);
		pCmdUI->Enable(false);
		return;
	}

	CDjVuView* pView = (CDjVuView*)pActive->GetActiveView();
	ASSERT(pView);
	int nCurrentPage = pView->GetCurrentPage();

	int nUnits = theApp.GetAppSettings()->nUnits;
	double fUnitsPerInch = CAppSettings::unitsPerInch[nUnits];

	CString strNewMessage;
	CSize szPage = pView->GetPageSize(nCurrentPage);
	int nDPI = pView->GetPageDPI(nCurrentPage);
	double fWidth = static_cast<int>(fUnitsPerInch * 100.0 * szPage.cx / nDPI + 0.5) * 0.01;
	double fHeight = static_cast<int>(fUnitsPerInch * 100.0 * szPage.cy / nDPI + 0.5) * 0.01;

	CString strUnits;
	AfxExtractSubString(strUnits, LoadString(IDS_UNITS_SHORT), nUnits, ',');

	strNewMessage.Format(ID_INDICATOR_SIZE,
			(LPCTSTR)FormatDouble(fWidth), (LPCTSTR)FormatDouble(fHeight), strUnits);

	CStatusBarCtrl& status = m_wndStatusBar.GetStatusBarCtrl();
	if (status.GetText(4, 0) != strNewMessage)
	{
		pCmdUI->SetText(strNewMessage);
		status.SetText(strNewMessage, 4, 0);

		CWindowDC dc(&status);
		CFont* pOldFont = dc.SelectObject(status.GetFont());
		m_wndStatusBar.SetPaneInfo(4, ID_INDICATOR_SIZE, 0, dc.GetTextExtent(strNewMessage).cx + 2);
		dc.SelectObject(pOldFont);
	}
}

BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (IsFullscreenMode())
	{
		if (m_pFullscreenWnd->SendMessage(WM_COMMAND, wParam, lParam))
			return true;

		return CWnd::OnCommand(wParam, lParam);
	}

	return CMDIFrameWnd::OnCommand(wParam, lParam);
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (IsFullscreenMode())
	{
		if (m_pFullscreenWnd->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return true;

		return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	}

	return CMDIFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnDestroy()
{
	if (m_pFullscreenWnd != NULL)
	{
		m_pFullscreenWnd->DestroyWindow();
		m_pFullscreenWnd = NULL;
	}

	if (m_pMagnifyWnd != NULL)
	{
		m_pMagnifyWnd->DestroyWindow();
		m_pMagnifyWnd = NULL;
	}

	if (m_pFindDlg != NULL)
	{
		m_pFindDlg->DestroyWindow();
		delete m_pFindDlg;
		m_pFindDlg = NULL;
	}

	theApp.GetAppSettings()->nCurLang = m_nCurLang;
	theApp.GetAppSettings()->nCurDict = m_nCurDict;

	theApp.RemoveObserver(this);
	m_wndTabBar.RemoveObserver(this);

	CMDIFrameWnd::OnDestroy();
}

void CMainFrame::LanguageChanged()
{
	HMENU hOldMenuDefault = m_hMenuDefault;
	m_hMenuDefault = ::LoadMenu(AfxFindResourceHandle(MAKEINTRESOURCE(IDR_MAINFRAME), RT_MENU),
			MAKEINTRESOURCE(IDR_MAINFRAME));
	HMENU hOldChildMenu = m_childMenu.Detach();
	m_childMenu.LoadMenu(IDR_DjVuTYPE);

	OnUpdateFrameMenu(NULL);
	DrawMenuBar();

	::DestroyMenu(hOldMenuDefault);
	::DestroyMenu(hOldChildMenu);

	UpdateLangAndDict(NULL, true);

	m_cboZoom.ResetContent();

	CMDIChildWnd* pActive = MDIGetActive();
	if (pActive != NULL)
	{
		CDjVuView* pView = (CDjVuView*)pActive->GetActiveView();
		UpdateZoomCombo(pView);
	}

	if (m_pFindDlg != NULL)
	{
		m_pFindDlg->UpdateData();
		theApp.GetAppSettings()->strFind = m_pFindDlg->m_strFind;
		theApp.GetAppSettings()->bMatchCase = !!m_pFindDlg->m_bMatchCase;

		bool bVisible = !!m_pFindDlg->IsWindowVisible();

		CRect rcFindDlg;
		m_pFindDlg->GetWindowRect(rcFindDlg);
		m_pFindDlg->DestroyWindow();
		delete m_pFindDlg;

		m_pFindDlg = new CFindDlg();
		m_pFindDlg->Create(IDD_FIND, this);

		CRect rcNewFindDlg;
		m_pFindDlg->GetWindowRect(rcNewFindDlg);

		m_pFindDlg->MoveWindow(rcFindDlg.left, rcFindDlg.top,
			rcNewFindDlg.Width(), rcNewFindDlg.Height());
		m_pFindDlg->ShowWindow(bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
		SetFocus();
	}
}

void CMainFrame::OnClose()
{
	if (theApp.m_bTopLevelDocs)
	{
		if (theApp.m_frames.size() > 1)
		{
			// This is not the last top-level window.
			// Remove it from the application list before closing.
			theApp.RemoveMainFrame(this);
		}
	}

	if (m_pFindDlg != NULL)
	{
		m_pFindDlg->UpdateData();
		theApp.GetAppSettings()->strFind = m_pFindDlg->m_strFind;
		theApp.GetAppSettings()->bMatchCase = !!m_pFindDlg->m_bMatchCase;
	}

	CMDIFrameWnd::OnClose();
}

CFullscreenWnd* CMainFrame::GetFullscreenWnd()
{
	if (m_pFullscreenWnd == NULL)
	{
		m_pFullscreenWnd = new CFullscreenWnd();
		m_pFullscreenWnd->Create();
	}

	return m_pFullscreenWnd;
}

bool CMainFrame::IsFullscreenMode()
{
	return m_pFullscreenWnd != NULL && m_pFullscreenWnd->IsWindowVisible();
}

CMagnifyWnd* CMainFrame::GetMagnifyWnd()
{
	if (m_pMagnifyWnd == NULL)
	{
		m_pMagnifyWnd = new CMagnifyWnd();
		m_pMagnifyWnd->Create();
	}

	return m_pMagnifyWnd;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		if (IsFullscreenMode())
			m_pFullscreenWnd->Hide();
		else if (theApp.GetAppSettings()->bCloseOnEsc)
			SendMessage(WM_CLOSE);
		return true;
	}

	return CMDIFrameWnd::PreTranslateMessage(pMsg);
}

LRESULT CMainFrame::OnAppCommand(WPARAM wParam, LPARAM lParam)
{
	UINT nCommand = GET_APPCOMMAND_LPARAM(lParam);
	if (nCommand == APPCOMMAND_BROWSER_BACKWARD)
	{
		OnViewBack();
		return 1;
	}
	else if (nCommand == APPCOMMAND_BROWSER_FORWARD)
	{
		OnViewForward();
		return 1;
	}

	Default();
	return 0;
}

void CMainFrame::OnUpdate(const Observable* source, const Message* message)
{
	if (message->code == CURRENT_PAGE_CHANGED)
	{
		const CDjVuView* pView = static_cast<const CDjVuView*>(source);

		CMDIChildWnd* pActive = MDIGetActive();
		if (pActive == NULL)
			return;

		CDjVuView* pActiveView = (CDjVuView*)pActive->GetActiveView();
		if (pActiveView == pView)
			UpdatePageCombo(pView);

		SendMessageToDescendants(WM_IDLEUPDATECMDUI, TRUE, 0, TRUE, TRUE);
	}
	else if (message->code == ZOOM_CHANGED)
	{
		const CDjVuView* pView = static_cast<const CDjVuView*>(source);

		CMDIChildWnd* pActive = MDIGetActive();
		if (pActive == NULL)
			return;

		CDjVuView* pActiveView = (CDjVuView*)pActive->GetActiveView();
		if (pActiveView == pView)
			UpdateZoomCombo(pView);
	}
	else if (message->code == VIEW_ACTIVATED)
	{
		const CDjVuView* pView = static_cast<const CDjVuView*>(source);
		OnViewActivated(pView);
	}
	else if (message->code == APP_LANGUAGE_CHANGED)
	{
		LanguageChanged();
	}
	else if (message->code == DICT_LIST_CHANGED)
	{
		UpdateToolbars();
		UpdateLangAndDict(NULL, true);
	}
	else if (message->code == FRAME_CREATED)
	{
		CFrameWnd* pFrame = const_cast<CFrameWnd*>(static_cast<const FrameMsg*>(message)->pFrame);
		CDocument* pDocument = pFrame->GetActiveDocument();
		CString strName = pDocument != NULL ? pDocument->GetTitle() : _T("");
		m_wndTabBar.AddTab(pFrame, strName);
		UpdateToolbars();
	}
	else if (message->code == FRAME_ACTIVATED)
	{
		CFrameWnd* pFrame = const_cast<CFrameWnd*>(static_cast<const FrameMsg*>(message)->pFrame);
		if (pFrame != NULL)
			m_wndTabBar.ActivateTab(pFrame);
		else
			m_wndTabBar.ActivateTab((CFrameWnd*) NULL);

		CDjVuView* pView = (pFrame != NULL ? ((CChildFrame*) pFrame)->GetDjVuView() : NULL);
		OnViewActivated(pView);
	}
	else if (message->code == FRAME_CLOSED)
	{
		CFrameWnd* pFrame = const_cast<CFrameWnd*>(static_cast<const FrameMsg*>(message)->pFrame);
		if (pFrame != NULL)
		{
			m_wndTabBar.RemoveTab(pFrame);
			((CChildFrame*) pFrame)->RemoveObserver(this);
		}
		UpdateToolbars();
	}
	else if (message->code == TAB_SELECTED)
	{
		CFrameWnd* pFrame = const_cast<CFrameWnd*>(static_cast<const FrameMsg*>(message)->pFrame);
		if (pFrame != NULL)
			pFrame->ActivateFrame();
	}
}

void CMainFrame::OnViewActivated(const CDjVuView* pView)
{
	if (pView != NULL)
	{
		m_cboPage.EnableWindow(true);
		m_cboZoom.EnableWindow(true);

		UpdatePageCombo(pView);
		UpdateZoomCombo(pView);
		UpdateLangAndDict(pView);
	}
	else
	{
		m_cboPage.SetWindowText(_T(""));
		m_cboPage.ResetContent();
		m_cboPage.EnableWindow(false);

		m_cboZoom.SetWindowText(_T(""));
		m_cboZoom.EnableWindow(false);
	}
}

void CMainFrame::OnWindowCascade()
{
	if (!theApp.m_bTopLevelDocs)
	{
		OnMDIWindowCmd(ID_WINDOW_CASCADE);
		return;
	}

	CRect rcMonitor = GetMonitorWorkArea(this);
	CSize szWindow(rcMonitor.Width() * 2 / 3, rcMonitor.Height() * 2 / 3);
	if (rcMonitor.Width() > rcMonitor.Height())
		szWindow.cx = rcMonitor.Width() - rcMonitor.Height() / 3;
	else
		szWindow.cy = rcMonitor.Height() - rcMonitor.Width() / 3;
	CPoint ptTopLeft(0, 0);

	int nDoc = 0;
	POSITION pos = theApp.GetFirstDocTemplatePosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = theApp.GetNextDocTemplate(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);

		POSITION posDoc = pTemplate->GetFirstDocPosition();
		while (posDoc != NULL)
		{
			CDjVuDoc* pDoc = (CDjVuDoc*) pTemplate->GetNextDoc(posDoc);
			CMainFrame* pFrame = pDoc->GetDjVuView()->GetMainFrame();

			// In workspace coordinates
			if (ptTopLeft.x + szWindow.cx > rcMonitor.Width() ||
					ptTopLeft.y + szWindow.cy > rcMonitor.Height())
				ptTopLeft = CPoint(0, 0);

			WINDOWPLACEMENT wndpl;
			wndpl.length = sizeof(wndpl);
			pFrame->GetWindowPlacement(&wndpl);
			wndpl.rcNormalPosition.left = ptTopLeft.x;
			wndpl.rcNormalPosition.top = ptTopLeft.y;
			wndpl.rcNormalPosition.right = ptTopLeft.x + szWindow.cx;
			wndpl.rcNormalPosition.bottom = ptTopLeft.y + szWindow.cy;
			wndpl.showCmd = (pFrame->IsZoomed() || pFrame->IsIconic() ? SW_RESTORE : SW_SHOWNORMAL);
			pFrame->SetWindowPlacement(&wndpl);
			if (pFrame != this)
				pFrame->SetWindowPos(this, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);

			ptTopLeft += CPoint(20, 20);
		}
	}

	BringWindowToTop();
}

void CMainFrame::OnWindowTileHorz()
{
	if (!theApp.m_bTopLevelDocs)
	{
		OnMDIWindowCmd(ID_WINDOW_TILE_HORZ);
		return;
	}

	CRect rcMonitor = GetMonitorWorkArea(this);
	int nFrameCount = theApp.m_frames.size();

	int nDoc = 0;
	POSITION pos = theApp.GetFirstDocTemplatePosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = theApp.GetNextDocTemplate(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);

		POSITION posDoc = pTemplate->GetFirstDocPosition();
		while (posDoc != NULL)
		{
			CDjVuDoc* pDoc = (CDjVuDoc*) pTemplate->GetNextDoc(posDoc);
			CMainFrame* pFrame = pDoc->GetDjVuView()->GetMainFrame();

			// In workspace coordinates
			int nTop = rcMonitor.Height() * nDoc / nFrameCount;
			int nBottom = rcMonitor.Height() * (nDoc + 1) / nFrameCount;
			++nDoc;

			WINDOWPLACEMENT wndpl;
			wndpl.length = sizeof(wndpl);
			pFrame->GetWindowPlacement(&wndpl);
			wndpl.rcNormalPosition.left = 0;
			wndpl.rcNormalPosition.top = nTop;
			wndpl.rcNormalPosition.right = rcMonitor.Width();
			wndpl.rcNormalPosition.bottom = nBottom;
			wndpl.showCmd = (pFrame->IsZoomed() || pFrame->IsIconic() ? SW_RESTORE : SW_SHOWNORMAL);
			pFrame->SetWindowPlacement(&wndpl);
			if (pFrame != this)
				pFrame->SetWindowPos(this, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		}
	}

	BringWindowToTop();
}

void CMainFrame::OnWindowTileVert()
{
	if (!theApp.m_bTopLevelDocs)
	{
		OnMDIWindowCmd(ID_WINDOW_TILE_VERT);
		return;
	}

	CRect rcMonitor = GetMonitorWorkArea(this);
	int nFrameCount = theApp.m_frames.size();

	int nDoc = 0;
	POSITION pos = theApp.GetFirstDocTemplatePosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = theApp.GetNextDocTemplate(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);

		POSITION posDoc = pTemplate->GetFirstDocPosition();
		while (posDoc != NULL)
		{
			CDjVuDoc* pDoc = (CDjVuDoc*) pTemplate->GetNextDoc(posDoc);
			CMainFrame* pFrame = pDoc->GetDjVuView()->GetMainFrame();

			// In workspace coordinates
			int nLeft = rcMonitor.Width() * nDoc / nFrameCount;
			int nRight = rcMonitor.Width() * (nDoc + 1) / nFrameCount;
			++nDoc;

			WINDOWPLACEMENT wndpl;
			wndpl.length = sizeof(wndpl);
			pFrame->GetWindowPlacement(&wndpl);
			wndpl.rcNormalPosition.left = nLeft;
			wndpl.rcNormalPosition.top = 0;
			wndpl.rcNormalPosition.right = nRight;
			wndpl.rcNormalPosition.bottom = rcMonitor.Height();
			wndpl.showCmd = (pFrame->IsZoomed() || pFrame->IsIconic() ? SW_RESTORE : SW_SHOWNORMAL);
			pFrame->SetWindowPlacement(&wndpl);
			if (pFrame != this)
				pFrame->SetWindowPos(this, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		}
	}

	BringWindowToTop();
}
