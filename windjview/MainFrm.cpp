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
#include "MainFrm.h"

#include "MDIChild.h"
#include "DjVuView.h"
#include "DjVuDoc.h"
#include "ThumbnailsView.h"
#include "FullscreenWnd.h"
#include "MagnifyWnd.h"
#include "MyDocTemplate.h"
#include "FindDlg.h"

#include <dde.h>
#include <afxole.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDW_DICTIONARIES_BAR (AFX_IDW_CONTROLBAR_FIRST + 10)


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_ACTIVATE()
	ON_COMMAND(ID_VIEW_TOOLBAR, OnViewToolbar)
	ON_COMMAND(ID_VIEW_TAB_BAR, OnViewTabBar)
	ON_COMMAND(ID_VIEW_STATUS_BAR, OnViewStatusBar)
	ON_COMMAND(ID_VIEW_SIDEBAR, OnViewSidebar)
	ON_COMMAND(ID_VIEW_DICTBAR, OnViewDictBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, OnUpdateViewToolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TAB_BAR, OnUpdateViewTabBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SIDEBAR, OnUpdateViewSidebar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DICTBAR, OnUpdateViewDictBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STATUS_BAR, OnUpdateViewStatusBar)
	ON_COMMAND(ID_TOGGLE_NAV_PANE, OnToggleNavPane)
	ON_WM_WINDOWPOSCHANGED()
	ON_CBN_SELCHANGE(IDC_PAGENUM, OnChangePage)
	ON_CONTROL(CBN_FINISHEDIT, IDC_PAGENUM, OnChangePageEdit)
	ON_CONTROL(CBN_DROPDOWN, IDC_PAGENUM, OnDropDownPage)
	ON_CONTROL(CBN_CANCELEDIT, IDC_PAGENUM, OnCancelChange)
	ON_CBN_SELCHANGE(IDC_ZOOM, OnChangeZoom)
	ON_CONTROL(CBN_FINISHEDIT, IDC_ZOOM, OnChangeZoomEdit)
	ON_CONTROL(CBN_CANCELEDIT, IDC_ZOOM, OnCancelChange)
	ON_NOTIFY(CBN_MOUSEWHEEL, IDC_PAGENUM, OnMouseWheelPage)
	ON_MESSAGE(WM_DDE_EXECUTE, OnDDEExecute)
	ON_COMMAND(ID_EDIT_FIND, OnEditFind)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateEditFind)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_ACTIVATE_FIRST, OnUpdateWindowList)
	ON_COMMAND_RANGE(ID_WINDOW_ACTIVATE_FIRST, ID_WINDOW_ACTIVATE_LAST, OnActivateWindow)
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
	ON_UPDATE_COMMAND_UI(ID_WINDOW_CASCADE, OnUpdateWindowCascade)
	ON_COMMAND(ID_WINDOW_TILE_HORZ, OnWindowTileHorz)
	ON_COMMAND(ID_WINDOW_TILE_VERT, OnWindowTileVert)
	ON_COMMAND(ID_WINDOW_NEXT, OnWindowNext)
	ON_COMMAND(ID_WINDOW_PREV, OnWindowPrev)
	ON_WM_ERASEBKGND()
	ON_WM_NCACTIVATE()
	ON_MESSAGE_VOID(WM_NOTIFY_NEW_VERSION, OnNewVersion)
	ON_WM_NCDESTROY()
	ON_UPDATE_COMMAND_UI_RANGE(ID_LAYOUT_CONTINUOUS, ID_LAYOUT_FACING, OnUpdateDisable)
	ON_UPDATE_COMMAND_UI_RANGE(ID_MODE_DRAG, ID_MODE_ZOOM_RECT, OnUpdateDisable)
	ON_UPDATE_COMMAND_UI_RANGE(ID_ZOOM_50, ID_ZOOM_CUSTOM, OnUpdateDisable)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, OnUpdateFileClose)
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
	: m_bDontActivate(false), m_pFullscreenWnd(NULL),
	  m_pMagnifyWnd(NULL), m_nCurLang(CB_ERR), m_nCurDict(CB_ERR),
	  m_bTabActivating(false), m_bHadActiveView(false), m_hPrevMenu(NULL)
{
	m_appMenu.LoadMenu(IDR_MAINFRAME);
	m_docMenu.LoadMenu(IDR_DjVuTYPE);
	m_bMaximized = theApp.GetAppSettings()->bWindowMaximized;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT,
			WS_CHILD | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.GetToolBar().LoadToolBar(IDR_MAINFRAME))
	{
		TRACE(_T("Failed to create toolbar\n"));
		return -1;      // fail to create
	}

	if (!m_wndDictBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT,
			WS_CHILD | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_SIZE_DYNAMIC,
			CRect(0, 0, 0, 0), IDW_DICTIONARIES_BAR) ||
		!m_wndDictBar.GetToolBar().LoadToolBar(IDR_DICT_BAR))
	{
		TRACE(_T("Failed to create dictionaries bar\n"));
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
	m_tabbedMDI.AddObserver(this);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))
		return false;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;

	return true;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	m_tabbedMDI.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST);
	m_tabbedMDI.BringWindowToTop();
	return true;
}

void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	if (nState == WA_ACTIVE || nState == WA_CLICKACTIVE)
		theApp.ChangeMainWnd(this);

	CFrameWnd::OnActivate(nState, pWndOther, bMinimized);
}

void CMainFrame::InitToolBar()
{
	m_wndToolBar.GetToolBar().SetHeight(30);
	m_wndToolBar.GetToolBar().SetWindowText(LoadString(IDS_TOOLBAR_TITLE));
	m_wndToolBar.SetWindowText(LoadString(IDS_TOOLBAR_TITLE));

	HBITMAP hbm = (HBITMAP)::LoadImage(AfxGetInstanceHandle(),
			MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_BITMAP,
			0, 0, LR_CREATEDIBSECTION);
	CBitmap bm;
	bm.Attach(hbm);

	m_imageListBar.Create(16, 16, ILC_COLOR32, 0, 1);
	m_imageListBar.Add(&bm, RGB(192, 192, 192));
	m_wndToolBar.GetToolBar().GetToolBarCtrl().SetImageList(&m_imageListBar);

	HBITMAP hbm2 = (HBITMAP)::LoadImage(AfxGetInstanceHandle(),
			MAKEINTRESOURCE(IDR_TOOLBAR_GRAYED), IMAGE_BITMAP,
			0, 0, LR_CREATEDIBSECTION);
	CBitmap bm2;
	bm2.Attach(hbm2);

	m_imageListGrayed.Create(16, 16, ILC_COLOR32, 0, 1);
	m_imageListGrayed.Add(&bm2, RGB(192, 192, 192));
	m_wndToolBar.GetToolBar().GetToolBarCtrl().SetDisabledImageList(&m_imageListGrayed);

	int nComboPage = m_wndToolBar.GetToolBar().CommandToIndex(ID_VIEW_NEXTPAGE) - 1;
	m_wndToolBar.SetControl(nComboPage, IDC_PAGENUM, 65);

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
	m_wndToolBar.SetControl(nComboZoom, IDC_ZOOM, 110);

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

	HBITMAP hbm = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_DICT_BAR),
			IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	CBitmap bm;
	bm.Attach(hbm);

	m_imageListDict.Create(16, 16, ILC_COLOR32, 0, 1);
	m_imageListDict.Add(&bm, RGB(192, 192, 192));
	m_wndDictBar.GetToolBar().GetToolBarCtrl().SetImageList(&m_imageListDict);

	HBITMAP hbm2 = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_DICT_BAR_GRAYED),
			IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	CBitmap bm2;
	bm2.Attach(hbm2);

	m_imageListDictGrayed.Create(16, 16, ILC_COLOR32, 0, 1);
	m_imageListDictGrayed.Add(&bm2, RGB(192, 192, 192));
	m_wndDictBar.GetToolBar().GetToolBarCtrl().SetDisabledImageList(&m_imageListDictGrayed);

	m_wndDictBar.InsertLabel(0, IDC_LOOKUP_LABEL, &m_font);

	TBBUTTON btnSep;
	ZeroMemory(&btnSep, sizeof(btnSep));
	btnSep.iBitmap = 0;
	btnSep.idCommand = IDC_STATIC;
	btnSep.fsStyle = TBSTYLE_SEP;

	// Lookup combo
	m_wndDictBar.GetToolBarCtrl().InsertButton(1, &btnSep);
	m_wndDictBar.SetControl(1, IDC_LOOKUP, 135);

	CRect rcItem;
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

	m_wndDictBar.GetToolBarCtrl().InsertButton(3, &btnSep);
	m_wndDictBar.InsertLabel(4, IDC_LANGUAGE_LIST_LABEL, &m_font);

	// Language combo
	m_wndDictBar.GetToolBarCtrl().InsertButton(5, &btnSep);
	m_wndDictBar.SetControl(5, IDC_LANGUAGE_LIST, 250);

	m_wndDictBar.GetToolBar().GetItemRect(5, rcItem);
	rcItem.DeflateRect(3, 0);
	rcItem.bottom += 160;

	m_cboLangs.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL
			| CBS_DROPDOWNLIST | CBS_AUTOHSCROLL, rcItem, &m_wndDictBar.GetToolBar(), IDC_LANGUAGE_LIST);
	m_cboLangs.SetFont(&m_boldFont);
	m_cboLangs.SetItemHeight(-1, 16);

	m_wndDictBar.GetToolBarCtrl().InsertButton(6, &btnSep);
	m_wndDictBar.InsertLabel(7, IDC_DICTIONARY_LABEL, &m_font);

	// Dictionary combo
	m_wndDictBar.GetToolBarCtrl().InsertButton(8, &btnSep);
	m_wndDictBar.SetControl(8, IDC_DICTIONARY, 350);

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

void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	CString strTitle = m_strTitle;

	CDocument* pDoc = GetActiveDocument();
	if (pDoc != NULL)
		strTitle = pDoc->GetTitle() + _T(" - ") + strTitle;

	AfxSetWindowText(m_hWnd, strTitle);
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnViewToolbar()
{
	CFrameWnd::OnBarCheck(ID_VIEW_TOOLBAR);
	theApp.GetAppSettings()->bToolbar = !!m_wndToolBar.IsWindowVisible();
}

void CMainFrame::OnViewTabBar()
{
	if (theApp.m_bTopLevelDocs)
		return;

	m_tabbedMDI.ShowTabBar(!m_tabbedMDI.IsTabBarVisible());
	theApp.GetAppSettings()->bTabBar = m_tabbedMDI.IsTabBarVisible();
}

void CMainFrame::OnViewStatusBar()
{
	CFrameWnd::OnBarCheck(ID_VIEW_STATUS_BAR);
	theApp.GetAppSettings()->bStatusBar = !!m_wndStatusBar.IsWindowVisible();
}

void CMainFrame::OnViewSidebar()
{
	CMDIChild* pMDIChild = (CMDIChild*) m_tabbedMDI.GetActiveTab();
	if (pMDIChild == NULL)
		return;

	bool bHide = !pMDIChild->IsNavPaneHidden();
	theApp.GetAppSettings()->bNavPaneHidden = bHide;
	pMDIChild->HideNavPane(bHide);
}

void CMainFrame::OnViewDictBar()
{
	ShowControlBar(&m_wndDictBar, !m_wndDictBar.IsWindowVisible(), false);
	theApp.GetAppSettings()->bDictBar = !!m_wndDictBar.IsWindowVisible();
}

void CMainFrame::OnUpdateViewToolbar(CCmdUI* pCmdUI)
{
	OnUpdateControlBarMenu(pCmdUI);

	if (!theApp.m_bTopLevelDocs
			&& m_tabbedMDI.GetTabCount() > 0
			&& (!theApp.GetAppSettings()->bHideSingleTab || m_tabbedMDI.GetTabCount() > 1)
			&& pCmdUI->m_pMenu != NULL
			&& pCmdUI->m_pMenu->GetMenuItemID(pCmdUI->m_nIndex + 1) != ID_VIEW_TAB_BAR)
	{
		pCmdUI->m_pMenu->InsertMenu(pCmdUI->m_nIndex + 1, MF_BYPOSITION | MF_STRING,
				ID_VIEW_TAB_BAR, LoadString(IDS_TAB_BAR_MENU));
		pCmdUI->m_nIndexMax = pCmdUI->m_pMenu->GetMenuItemCount();
		pCmdUI->m_bEnableChanged = true;
	}
}

void CMainFrame::OnUpdateViewTabBar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_tabbedMDI.IsTabBarVisible());

	if (pCmdUI->m_pMenu != NULL && (theApp.m_bTopLevelDocs
			|| m_tabbedMDI.GetTabCount() == 0
			|| theApp.GetAppSettings()->bHideSingleTab && m_tabbedMDI.GetTabCount() == 1))
	{
		pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nIndex, MF_BYPOSITION);
		pCmdUI->m_nIndex--;
		pCmdUI->m_nIndexMax = pCmdUI->m_pMenu->GetMenuItemCount();
	}
}

void CMainFrame::OnUpdateViewSidebar(CCmdUI* pCmdUI)
{
	CMDIChild* pMDIChild = (CMDIChild*) m_tabbedMDI.GetActiveTab();

	pCmdUI->Enable(pMDIChild != NULL);
	pCmdUI->SetCheck(pMDIChild != NULL && !pMDIChild->IsNavPaneHidden());
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
				ID_VIEW_DICTBAR, LoadString(IDR_DICT_BAR));
		pCmdUI->m_nIndexMax = pCmdUI->m_pMenu->GetMenuItemCount();
		pCmdUI->m_bEnableChanged = true;
	}
}

void CMainFrame::OnToggleNavPane()
{
	CMDIChild* pMDIChild = (CMDIChild*) m_tabbedMDI.GetActiveTab();
	if (pMDIChild == NULL)
		return;

	bool bCollapsed = !pMDIChild->IsNavPaneCollapsed();
	theApp.GetAppSettings()->bNavPaneCollapsed = bCollapsed;
	pMDIChild->CollapseNavPane(bCollapsed);
}

void CMainFrame::UpdateToolbars()
{
	CAppSettings* pSettings = theApp.GetAppSettings();

	ShowControlBar(&m_wndToolBar, pSettings->bToolbar, false);
	ShowControlBar(&m_wndDictBar, pSettings->bDictBar && theApp.GetDictLangsCount() > 0, false);
	ShowControlBar(&m_wndStatusBar, pSettings->bStatusBar, false);

	if (!theApp.m_bTopLevelDocs)
	{
		m_tabbedMDI.ShowTabBar(pSettings->bTabBar &&
				(!theApp.GetAppSettings()->bHideSingleTab || m_tabbedMDI.GetTabCount() > 1));
	}
}

void CMainFrame::UpdateSettings()
{
	CRect rect;
	GetWindowRect(rect);

	CAppSettings* pSettings = theApp.GetAppSettings();
	m_bMaximized = !!IsZoomed();
	pSettings->bWindowMaximized = m_bMaximized;
	if (!m_bMaximized)
	{
		pSettings->nWindowPosX = rect.left;
		pSettings->nWindowPosY = rect.top;
		pSettings->nWindowWidth = rect.right - rect.left;
		pSettings->nWindowHeight = rect.bottom - rect.top;
	}
}

void CMainFrame::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CFrameWnd::OnWindowPosChanged(lpwndpos);

	if (IsWindowVisible() && !IsIconic() && theApp.m_bInitialized)
		UpdateSettings();

	// Note: When the window is first shown with SW_MAXIMIZE command,
	// the WM_SHOWWINDOW message is not sent! So we have to handle
	// WM_WINDOWPOSCHANGED instead.
	if ((lpwndpos->flags & (SWP_SHOWWINDOW | SWP_HIDEWINDOW)) != 0)
	{
		bool bShow = (lpwndpos->flags & SWP_SHOWWINDOW) != 0;
		CWnd* pParent = GetParent();
		if (pParent == NULL || pParent != NULL && pParent->IsWindowVisible())
			SendMessageToVisibleDescendants(m_hWnd, WM_SHOWPARENT, bShow);
	}
}

void CMainFrame::OnChangePage()
{
	CDjVuView* pView = (CDjVuView*) GetActiveView();
	if (pView == NULL)
		return;

	int nPage = m_cboPage.GetCurSel();
	if (nPage >= 0 && nPage < pView->GetPageCount())
		pView->GoToPage(nPage);

	pView->SetFocus();
}

void CMainFrame::OnChangePageEdit()
{
	CDjVuView* pView = (CDjVuView*) GetActiveView();
	if (pView == NULL)
		return;

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

void CMainFrame::OnDropDownPage()
{
	CDjVuView* pView = (CDjVuView*) GetActiveView();
	if (pView == NULL)
		return;

	int nPage = pView->GetCurrentPage();
	if (pView->GetPageCount() > m_cboPage.GetCount())
	{
		m_cboPage.InitStorage(pView->GetPageCount() - m_cboPage.GetCount(), 10);
		for (int i = m_cboPage.GetCount(); i < pView->GetPageCount(); ++i)
			m_cboPage.AddString(FormatString(_T("%d"), i + 1));
	}
	else if (pView->GetPageCount() < m_cboPage.GetCount())
	{
		for (int i = m_cboPage.GetCount() - 1; i >= pView->GetPageCount(); --i)
			m_cboPage.DeleteString(i);
	}

	m_cboPage.SetCurSel(nPage);
}

void CMainFrame::OnCancelChange()
{
	CDjVuView* pView = (CDjVuView*) GetActiveView();
	if (pView == NULL)
	{
		m_tabbedMDI.SetFocus();
		return;
	}

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

	int nPage = pView->GetCurrentPage();
	CString strPage = FormatString(_T("%d"), nPage + 1);
	CString strCurPage;
	m_cboPage.GetWindowText(strCurPage);
	if (strPage != strCurPage)
		m_cboPage.SetWindowText(strPage);
}

void CMainFrame::OnChangeZoom()
{
	CDjVuView* pView = (CDjVuView*) GetActiveView();
	if (pView == NULL)
		return;

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
	CDjVuView* pView = (CDjVuView*) GetActiveView();
	if (pView == NULL)
		return;

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
		pView = (CDjVuView*) GetActiveView();

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
	UINT_PTR unused;
	HGLOBAL hData;
	//IA64: Assume DDE LPARAMs are still 32-bit
	VERIFY(UnpackDDElParam(WM_DDE_EXECUTE, lParam, &unused, (PUINT_PTR)&hData));

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
	CFindDlg* pFindDlg = theApp.GetFindDlg();
	pFindDlg->SetOwner(this);
	pFindDlg->ShowWindow(SW_SHOW);
	pFindDlg->SetFocus();
	pFindDlg->GotoDlgCtrl(pFindDlg->GetDlgItem(IDC_FIND));
}

void CMainFrame::OnUpdateEditFind(CCmdUI* pCmdUI)
{
	CDjVuView* pView = (CDjVuView*) GetActiveView();
	if (IsFullscreenMode() || pView == NULL)
	{
		pCmdUI->Enable(false);
		return;
	}

	CDjVuDoc* pDoc = pView->GetDocument();
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
	CDocument* pActiveDoc = GetActiveDocument();

	const int nMaxWindows = ID_WINDOW_ACTIVATE_LAST - ID_WINDOW_ACTIVATE_FIRST + 1;
	for (int i = 0; i < nMaxWindows; ++i)
		pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nID + i, MF_BYCOMMAND);

	if (theApp.m_bTopLevelDocs)
	{
		int nDoc = 0;
		for (CDjViewApp::DocIterator it; it && nDoc < nMaxWindows; ++it, ++nDoc)
		{
			CDocument* pDoc = *it;

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
		}

		if (nDoc == 0)
		{
			// Delete the separator
			pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nIndex - 1, MF_BYPOSITION);
		}
	}
	else
	{
		for (int nTab = 0; nTab < m_tabbedMDI.GetTabCount(); ++nTab)
		{
			CString strText;
			if (nTab >= 10)
				strText.Format(_T("%d "), nTab + 1);
			else if (nTab == 9)
				strText = _T("1&0 ");
			else
				strText.Format(_T("&%d "), nTab + 1);

			CString strDocTitle = m_tabbedMDI.GetTabName(nTab);
			strDocTitle.Replace(_T("&"), _T("&&"));
			strText += strDocTitle;

			int nFlags = MF_STRING;
			if (m_tabbedMDI.GetActiveTabIndex() == nTab)
				nFlags |= MF_CHECKED;

			pCmdUI->m_pMenu->AppendMenu(nFlags, pCmdUI->m_nID + nTab, strText);
		}

		if (m_tabbedMDI.GetTabCount() == 0)
		{
			// Delete the separator. If there are no open documents, we are modifying
			// the IDR_MAINFRAME menu here which can never be used to display a non-empty
			// list of windows again. It can only be non-empty if some of the restored
			// tabs failed to open.
			pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nIndex - 1, MF_BYPOSITION);
		}
	}

	// update end menu count
	pCmdUI->m_nIndex = pCmdUI->m_pMenu->GetMenuItemCount();
	pCmdUI->m_nIndexMax = pCmdUI->m_pMenu->GetMenuItemCount();
	pCmdUI->m_bEnableChanged = true;
}

void CMainFrame::OnActivateWindow(UINT nID)
{
	int nActivateDoc = nID - ID_WINDOW_ACTIVATE_FIRST;

	if (theApp.m_bTopLevelDocs)
	{
		int nDoc = 0;
		for (CDjViewApp::DocIterator it; it; ++it, ++nDoc)
		{
			CDocument* pDoc = *it;
			if (nDoc == nActivateDoc)
			{
				ActivateDocument(pDoc);
				return;
			}
		}
	}
	else
	{
		m_tabbedMDI.ActivateTab(nActivateDoc);
	}
}

void CMainFrame::AddMDIChild(CWnd* pMDIChild, CDocument* pDocument)
{
	if (theApp.m_bTopLevelDocs
			|| theApp.GetAppSettings()->bHideSingleTab && m_tabbedMDI.GetTabCount() == 0)
		m_tabbedMDI.ShowTabBar(false);

	m_tabbedMDI.AddTab(pMDIChild, pDocument->GetTitle(), pDocument->GetPathName());

	if (!theApp.m_bTopLevelDocs
			&& (!theApp.GetAppSettings()->bHideSingleTab || m_tabbedMDI.GetTabCount() > 1))
		m_tabbedMDI.ShowTabBar(theApp.GetAppSettings()->bTabBar);
}

void CMainFrame::CloseMDIChild(CWnd* pMDIChild)
{
	m_tabbedMDI.CloseTab(pMDIChild);
}

void CMainFrame::ActivateDocument(CDocument* pDocument)
{
	if (pDocument == NULL)
	{
		ActivateFrame(m_bMaximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
		return;
	}

	CDjVuView* pView = ((CDjVuDoc*) pDocument)->GetDjVuView();
	CMainFrame* pMainFrame = pView->GetMainFrame();

	if (pMainFrame->IsFullscreenMode() &&
		pMainFrame->GetFullscreenWnd()->GetOwner()->GetDocument() == pDocument)
	{
		pMainFrame->ShowWindow(SW_HIDE);
		pMainFrame->GetFullscreenWnd()->SetForegroundWindow();
		pMainFrame->GetFullscreenWnd()->SetFocus();
	}
	else
	{
		if (pMainFrame->IsFullscreenMode())
			pMainFrame->GetFullscreenWnd()->Hide();

		pMainFrame->m_tabbedMDI.ActivateTab(pView->GetMDIChild());
		if (theApp.m_bInitialized)
			pMainFrame->ActivateFrame(m_bMaximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
	}
}

void CMainFrame::OnUpdateStatusAdjust(CCmdUI* pCmdUI)
{
	CDisplaySettings* pDisplaySettings = theApp.GetDisplaySettings();

	if (!pDisplaySettings->IsAdjusted() || GetActiveView() == NULL)
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
	CDjVuView* pView = (CDjVuView*) GetActiveView();
	if (pView != NULL)
	{
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
	CDjVuView* pView = (CDjVuView*) GetActiveView();
	if (pView == NULL)
	{
		m_wndStatusBar.SetPaneInfo(s_nIndicatorPage, ID_INDICATOR_PAGE, SBPS_DISABLED | SBPS_NOBORDERS, 0);
		pCmdUI->Enable(false);
		return;
	}

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
	CDjVuView* pView = (CDjVuView*) GetActiveView();
	if (pView == NULL)
	{
		m_wndStatusBar.SetPaneInfo(4, ID_INDICATOR_SIZE, SBPS_DISABLED | SBPS_NOBORDERS, 0);
		pCmdUI->Enable(false);
		return;
	}

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

	return CFrameWnd::OnCommand(wParam, lParam);
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (IsFullscreenMode())
	{
		if (m_pFullscreenWnd->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return true;

		return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	}

	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
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

	theApp.GetAppSettings()->nCurLang = m_nCurLang;
	theApp.GetAppSettings()->nCurDict = m_nCurDict;

	theApp.RemoveObserver(this);
	m_tabbedMDI.RemoveObserver(this);

	m_wndDictBar.GetToolBar().GetToolBarCtrl().SetImageList(NULL);
	m_wndDictBar.GetToolBar().GetToolBarCtrl().SetDisabledImageList(NULL);
	m_wndToolBar.GetToolBar().GetToolBarCtrl().SetImageList(NULL);
	m_wndToolBar.GetToolBar().GetToolBarCtrl().SetDisabledImageList(NULL);

	CFrameWnd::OnDestroy();
}

void CMainFrame::LanguageChanged()
{
	HMENU hOldAppMenu = m_appMenu.Detach();
	m_appMenu.LoadMenu(IDR_MAINFRAME);
	HMENU hOldDocMenu = m_docMenu.Detach();
	m_docMenu.LoadMenu(IDR_DjVuTYPE);

	OnUpdateFrameMenu(NULL);
	DrawMenuBar();

	::DestroyMenu(hOldAppMenu);
	::DestroyMenu(hOldDocMenu);

	UpdateLangAndDict(NULL, true);

	m_cboZoom.ResetContent();

	CDjVuView* pView = (CDjVuView*) GetActiveView();
	if (pView != NULL)
		UpdateZoomCombo(pView);

	SetMessageText(AFX_IDS_IDLEMESSAGE);
}

void CMainFrame::OnClose()
{
	if (theApp.m_bTopLevelDocs && theApp.GetDocumentCount() > 1)
	{
		// This is not the last top-level window.
		// Remove it from the application list before closing.
		theApp.RemoveMainFrame(this);
	}

	CFindDlg* pFindDlg = theApp.GetFindDlg(false);
	if (pFindDlg != NULL)
	{
		pFindDlg->UpdateData();
		theApp.GetAppSettings()->strFind = pFindDlg->m_strFind;
		theApp.GetAppSettings()->bMatchCase = !!pFindDlg->m_bMatchCase;
	}

	if (theApp.m_pMainWnd == this)
	{
		// attempt to save all documents
		if (!theApp.SaveAllModified())
			return;

		theApp.m_bClosing = true;

		// hide the application's windows before closing all the documents
		theApp.HideApplication();

		// close all documents first
		theApp.CloseAllDocuments(false);

		// don't exit if there are outstanding component objects
		if (!AfxOleCanExitApp())
		{
			// take user out of control of the app
			AfxOleSetUserCtrl(false);

			// don't destroy the main window and close down just yet
			//  (there are outstanding component (OLE) objects)
			return;
		}
	}

	ShowWindow(SW_HIDE);

	// close all documents that belong to this window
	CDocument* pDocument = GetActiveDocument();
	while (pDocument != NULL)
	{
		pDocument->OnCloseDocument();
		pDocument = GetActiveDocument();
	}

	DestroyWindow();
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

	return CFrameWnd::PreTranslateMessage(pMsg);
}

LRESULT CMainFrame::OnAppCommand(WPARAM wParam, LPARAM lParam)
{
	UINT nCommand = GET_APPCOMMAND_LPARAM(lParam);
	if (nCommand == APPCOMMAND_BROWSER_BACKWARD)
	{
		SendMessage(WM_COMMAND, ID_VIEW_BACK);
		return 1;
	}
	else if (nCommand == APPCOMMAND_BROWSER_FORWARD)
	{
		SendMessage(WM_COMMAND, ID_VIEW_FORWARD);
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
		if (pView == GetActiveView())
			UpdatePageCombo(pView);

		SendMessageToDescendants(WM_IDLEUPDATECMDUI, true, 0, true, true);
	}
	else if (message->code == ZOOM_CHANGED)
	{
		const CDjVuView* pView = static_cast<const CDjVuView*>(source);
		if (pView == GetActiveView())
			UpdateZoomCombo(pView);

		SendMessageToDescendants(WM_IDLEUPDATECMDUI, true, 0, true, true);
	}
	else if (message->code == VIEW_ACTIVATED || message->code == VIEW_INITIALIZED)
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
	else if (message->code == TAB_ACTIVATING)
	{
		const TabMsg* tabMsg = static_cast<const TabMsg*>(message);
		CMDIChild* pWnd = (CMDIChild*)tabMsg->pWnd;
		if (!m_bTabActivating && pWnd != NULL)
		{
			m_bTabActivating = true;
			if (pWnd->IsPlaceholder() && theApp.m_bInitialized && !theApp.m_bClosing)
			{
				CString strPathName = m_tabbedMDI.GetTabPathName(tabMsg->nTab);
				CDocument* pDocument = theApp.OpenDocumentFile(strPathName);
				if (!pDocument)
					pWnd->SetError(strPathName);
			}
			m_bTabActivating = false;
		}
	}
	else if (message->code == TAB_ACTIVATED)
	{
		const TabMsg* tabMsg = static_cast<const TabMsg*>(message);
		CMDIChild* pWnd = (CMDIChild*) tabMsg->pWnd;
		if (pWnd != NULL)
		{
			CDjVuView* pView = (CDjVuView*) pWnd->GetContent();
			SetActiveView(pView);
			OnViewActivated(pView);
			if (pView)
				pView->SetFocus();

			OnUpdateFrameMenu(NULL);
			OnUpdateFrameTitle(true);
			if (!m_bHadActiveView)
				DrawMenuBar();
			m_bHadActiveView = true;
		}
		else
		{
			SetActiveView(NULL);
			OnViewActivated(NULL);

			OnUpdateFrameMenu(NULL);
			OnUpdateFrameTitle(true);
			if (m_bHadActiveView)
				DrawMenuBar();
			m_bHadActiveView = false;
		}
	}
	else if (message->code == TAB_CLOSED)
	{
		theApp.SaveSettings();
		UpdateToolbars();

		CMDIChild* pWnd = (CMDIChild*) static_cast<const TabMsg*>(message)->pWnd;
		CDjVuView* pView = (CDjVuView*) pWnd->GetContent();
		if (pView == GetActiveView())
		{
			SetActiveView(NULL);
			OnViewActivated(NULL);

			OnUpdateFrameMenu(NULL);
			OnUpdateFrameTitle(true);
			DrawMenuBar();
			m_bHadActiveView = false;
		}
	}
	else if (message->code == APP_SETTINGS_CHANGED)
	{
		UpdateToolbars();
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
		return;

	CRect rcMonitor = GetMonitorWorkArea(this);
	CSize szWindow(rcMonitor.Width() * 2 / 3, rcMonitor.Height() * 2 / 3);
	if (rcMonitor.Width() > rcMonitor.Height())
		szWindow.cx = rcMonitor.Width() - rcMonitor.Height() / 3;
	else
		szWindow.cy = rcMonitor.Height() - rcMonitor.Width() / 3;
	CPoint ptTopLeft(0, 0);

	int nDoc = 0;
	for (CDjViewApp::DocIterator it; it; ++it, ++nDoc)
	{
		CDjVuDoc* pDoc = (CDjVuDoc*) *it;
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

	BringWindowToTop();
}

void CMainFrame::OnWindowTileHorz()
{
	if (!theApp.m_bTopLevelDocs)
		return;

	CRect rcMonitor = GetMonitorWorkArea(this);
	int nFrameCount = theApp.GetDocumentCount();

	int nDoc = 0;
	for (CDjViewApp::DocIterator it; it; ++it, ++nDoc)
	{
		CDjVuDoc* pDoc = (CDjVuDoc*) *it;
		CMainFrame* pFrame = pDoc->GetDjVuView()->GetMainFrame();

		// In workspace coordinates
		int nTop = rcMonitor.Height() * nDoc / nFrameCount;
		int nBottom = rcMonitor.Height() * (nDoc + 1) / nFrameCount;

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

	BringWindowToTop();
}

void CMainFrame::OnWindowTileVert()
{
	if (!theApp.m_bTopLevelDocs)
		return;

	CRect rcMonitor = GetMonitorWorkArea(this);
	int nFrameCount = theApp.GetDocumentCount();

	int nDoc = 0;
	for (CDjViewApp::DocIterator it; it; ++it, ++nDoc)
	{
		CDjVuDoc* pDoc = (CDjVuDoc*) *it;
		CMainFrame* pFrame = pDoc->GetDjVuView()->GetMainFrame();

		// In workspace coordinates
		int nLeft = rcMonitor.Width() * nDoc / nFrameCount;
		int nRight = rcMonitor.Width() * (nDoc + 1) / nFrameCount;

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

	BringWindowToTop();
}

void CMainFrame::OnUpdateWindowCascade(CCmdUI* pCmdUI)
{
	if (pCmdUI->m_pMenu == NULL || theApp.m_bTopLevelDocs)
		return;

	pCmdUI->m_pMenu->DeleteMenu(ID_WINDOW_CASCADE, MF_BYCOMMAND);
	pCmdUI->m_pMenu->DeleteMenu(ID_WINDOW_TILE_HORZ, MF_BYCOMMAND);
	pCmdUI->m_pMenu->DeleteMenu(ID_WINDOW_TILE_VERT, MF_BYCOMMAND);

	// Delete the separator
	pCmdUI->m_pMenu->DeleteMenu(0, MF_BYPOSITION);

	// update end menu count
	pCmdUI->m_nIndex = -1;
	pCmdUI->m_nIndexMax = pCmdUI->m_pMenu->GetMenuItemCount();
	pCmdUI->m_bEnableChanged = true;
}

void CMainFrame::OnWindowNext()
{
	if (theApp.m_bTopLevelDocs)
	{
		CDocument* pActiveDoc = GetActiveDocument();
		if (pActiveDoc == NULL)
			return;

		CDocument* pPrev = NULL;
		for (CDjViewApp::DocIterator it; it; ++it)
		{
			CDocument* pDoc = *it;
			if (pDoc == pActiveDoc && pPrev != NULL)
			{
				ActivateDocument(pPrev);
				return;
			}

			pPrev = pDoc;
		}

		ActivateDocument(pPrev);
	}
	else
	{
		m_tabbedMDI.ActivateNextTab();
	}
}

void CMainFrame::OnWindowPrev()
{
	if (theApp.m_bTopLevelDocs)
	{
		CDocument* pActiveDoc = GetActiveDocument();
		if (pActiveDoc == NULL)
			return;

		CDocument* pFirst = NULL;
		for (CDjViewApp::DocIterator it; it; ++it)
		{
			CDocument* pDoc = *it;
			if (pFirst == NULL)
				pFirst = pDoc;

			if (pDoc == pActiveDoc)
			{
				ActivateDocument(++it ? *it : pFirst);
				return;
			}
		}
	}
	else
	{
		m_tabbedMDI.ActivatePrevTab();
	}
}

void CMainFrame::OnUpdateFrameMenu(HMENU hMenuAlt)
{
	if (hMenuAlt == NULL)
	{
		CDocument* pDoc = GetActiveDocument();
		if (pDoc != NULL)
			hMenuAlt = m_docMenu.m_hMenu;

		if (hMenuAlt == NULL)
			hMenuAlt = m_appMenu.m_hMenu;
	}

	if (hMenuAlt != m_hPrevMenu)
		::SetMenu(m_hWnd, hMenuAlt);
	m_hPrevMenu = hMenuAlt;
}

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

BOOL CMainFrame::OnNcActivate(BOOL bActive)
{
	if (m_bDontActivate)
		return false;

	return CFrameWnd::OnNcActivate(bActive);
}

void CMainFrame::OnMouseWheelPage(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMCBWHEEL* pNMWheel = reinterpret_cast<NMCBWHEEL*>(pNMHDR);

	CDjVuView* pView = (CDjVuView*) GetActiveView();
	if (pView == NULL)
		return;

	// Parse the page displayed in the combo box instead of taking the current
	// page from the active view:  we want to go through all page numbers here,
	// but in Facing modes "current page + 1" may refer to the same view, and
	// there will be no scrolling at all.
	CString strPage;
	m_cboPage.GetWindowText(strPage);
	int nPage = 0;
	_stscanf(strPage, _T("%d"), &nPage);

	nPage = nPage - 1 + (pNMWheel->bUp ? -1 : 1);
	if (nPage >= 0 && nPage < pView->GetPageCount())
	{
		pView->GoToPage(nPage);
		CString strPage = FormatString(_T("%d"), nPage + 1);
		m_cboPage.SetWindowText(strPage);
	}

	*pResult = 1;
}

void CMainFrame::OnNewVersion()
{
	CPushRoutingFrame* pPushFrame = NULL;
	if (IsFullscreenMode())
		pPushFrame = new CPushRoutingFrame((CFrameWnd*) m_pFullscreenWnd);

	CDjViewApp::MessageBoxOptions mbo;
	mbo.strCheckBox = LoadString(IDS_CHECK_UPDATES);
	mbo.pCheckValue = &theApp.GetAppSettings()->bCheckUpdates;
	if (theApp.DoMessageBox(FormatString(IDS_NEW_VERSION_AVAILABLE, theApp.m_strNewVersion),
			MB_ICONQUESTION | MB_YESNO, 0, mbo) == IDYES)
	{
		::ShellExecute(NULL, _T("open"), LoadString(IDS_WEBSITE_URL),
				NULL, NULL, SW_SHOW);
	}

	if (pPushFrame != NULL)
		delete pPushFrame;
}

void CMainFrame::OnNcDestroy()
{
	// From MFC:  CWnd::OnNcDestroy
	// WM_NCDESTROY is the absolute LAST message sent.
	
	// cleanup main and active windows
	CWinThread* pThread = AfxGetThread();
	if (pThread != NULL)
	{
		if (pThread->m_pMainWnd == this)
		{
			if (!afxContextIsDLL)
			{
				// shut down current thread if possible
				if (pThread != AfxGetApp() || AfxOleCanExitApp())
					AfxPostQuitMessage(0);
			}

			if (pThread == AfxGetApp())
				theApp.ChangeMainWnd(NULL);
			else
				pThread->m_pMainWnd = NULL;
		}
		if (pThread->m_pActiveWnd == this)
			pThread->m_pActiveWnd = NULL;
	}

#ifndef _AFX_NO_OLE_SUPPORT
	// cleanup OLE drop target interface
	if (m_pDropTarget != NULL)
	{
		m_pDropTarget->Revoke();
		m_pDropTarget = NULL;
	}
#endif

#ifndef _AFX_NO_OCC_SUPPORT
	// cleanup control container
	delete m_pCtrlCont;
	m_pCtrlCont = NULL;
#endif

	// cleanup tooltip support
	if (m_nFlags & WF_TOOLTIPS)
	{
		CToolTipCtrl* pToolTip = AfxGetModuleThreadState()->m_pToolTip;
		if (pToolTip->GetSafeHwnd() != NULL)
		{
			TOOLINFO ti; memset(&ti, 0, sizeof(TOOLINFO));
			ti.cbSize = sizeof(AFX_OLDTOOLINFO);
			ti.uFlags = TTF_IDISHWND;
			ti.hwnd = m_hWnd;
			ti.uId = (UINT_PTR)m_hWnd;
			pToolTip->SendMessage(TTM_DELTOOL, 0, (LPARAM)&ti);
		}
	}

	// call default, unsubclass, and detach from the map
	WNDPROC pfnWndProc = WNDPROC(GetWindowLongPtr(m_hWnd, GWLP_WNDPROC));
	Default();
	if (WNDPROC(GetWindowLongPtr(m_hWnd, GWLP_WNDPROC)) == pfnWndProc)
	{
		WNDPROC pfnSuper = *GetSuperWndProcAddr();
		if (pfnSuper != NULL)
			SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, reinterpret_cast<INT_PTR>(pfnSuper));
	}
	Detach();
	ASSERT(m_hWnd == NULL);

	// call special post-cleanup routine
	PostNcDestroy();
}

void CMainFrame::OnUpdateDisable(CCmdUI* pCmdUI)
{
	// Default handler for view-handled commands. If no view is active
	// (i.e. to tabs open), we need to make sure that toolbar buttons
	// are unchecked so that they are drawn properly (see KB231893).
	pCmdUI->SetCheck(false);
	pCmdUI->Enable(false);
}

int CMainFrame::GetTabCount()
{
	return m_tabbedMDI.GetTabCount();
}

void CMainFrame::RestoreOpenTabs()
{
	CAppSettings* pSettings = theApp.GetAppSettings();
	for (int i = 0; i < (int)pSettings->openTabs.size(); ++i)
	{
		CWnd* pWnd = theApp.GetDocumentTemplate()->CreateEmptyMDIChild();
		// Set the tab title based on path name
		TCHAR szTitle[_MAX_FNAME] = { 0 };
		AfxGetFileTitle(pSettings->openTabs[i], szTitle, _MAX_FNAME);
		m_tabbedMDI.AddTab(pWnd, szTitle, pSettings->openTabs[i]);
	}

	if (pSettings->nStartupTab >= 0 && pSettings->nStartupTab < GetTabCount())
	{
		m_tabbedMDI.ActivateTab(pSettings->nStartupTab);
	}
}

void CMainFrame::LoadActiveTab()
{
	if (GetTabCount() == 0)
		return;

	CMDIChild* pChild = (CMDIChild*)m_tabbedMDI.GetActiveTab();
	if (pChild->IsPlaceholder()) {
		int nTab = m_tabbedMDI.GetActiveTabIndex();
		CString strPathName = m_tabbedMDI.GetTabPathName(nTab);
		CDocument* pDocument = theApp.OpenDocumentFile(strPathName);
		if (!pDocument)
			pChild->SetError(strPathName);
	}
}

void CMainFrame::SaveOpenTabs()
{
	CAppSettings* pSettings = theApp.GetAppSettings();
	pSettings->openTabs.clear();
	for (int i = 0; i < m_tabbedMDI.GetTabCount(); ++i)
	{
		const CString& strPathName = m_tabbedMDI.GetTabPathName(i);
		pSettings->openTabs.push_back(strPathName);
	}
	pSettings->nStartupTab = m_tabbedMDI.GetActiveTabIndex();
}

void CMainFrame::OnFileClose()
{
	CWnd* pActiveTab = m_tabbedMDI.GetActiveTab();
	if (pActiveTab)
		m_tabbedMDI.CloseTab(pActiveTab);
}

void CMainFrame::OnUpdateFileClose(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetTabCount() > 0);
}
