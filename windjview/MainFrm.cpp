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
#include "DjVuView.h"
#include "DjVuDoc.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "AppSettings.h"
#include "ThumbnailsView.h"
#include "FullscreenWnd.h"

#include <dde.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HHOOK CMainFrame::hHook;

void CreateSystemDialogFont(CFont& font)
{
	LOGFONT lf;

	HGDIOBJ hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	::GetObject(hFont, sizeof(LOGFONT), &lf);

	OSVERSIONINFO vi;
	if (::GetVersionEx(&vi) && vi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
		vi.dwMajorVersion >= 5)
	{
		strcpy(lf.lfFaceName, "MS Shell Dlg 2");
	}

	font.CreateFontIndirect(&lf);
}


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_TOOLBAR, OnViewToolbar)
	ON_COMMAND(ID_VIEW_STATUS_BAR, OnViewStatusBar)
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_SHOWWINDOW()
	ON_CBN_SELCHANGE(IDC_PAGENUM, OnChangePage)
	ON_CONTROL(CBN_FINISHEDIT, IDC_PAGENUM, OnChangePageEdit)
	ON_CONTROL(CBN_CANCELEDIT, IDC_PAGENUM, OnCancelChangePageZoom)
	ON_CBN_SELCHANGE(IDC_ZOOM, OnChangeZoom)
	ON_CONTROL(CBN_FINISHEDIT, IDC_ZOOM, OnChangeZoomEdit)
	ON_CONTROL(CBN_CANCELEDIT, IDC_ZOOM, OnCancelChangePageZoom)
	ON_MESSAGE(WM_DDE_EXECUTE, OnDDEExecute)
	ON_COMMAND(ID_VIEW_FIND, OnViewFind)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIND, OnUpdateViewFind)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_ACTIVATE_FIRST, OnUpdateWindowList)
	ON_COMMAND_RANGE(ID_WINDOW_ACTIVATE_FIRST, ID_WINDOW_ACTIVATE_LAST, OnActivateWindow)
	ON_COMMAND(ID_VIEW_BACK, OnViewBack)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BACK, OnUpdateViewBack)
	ON_COMMAND(ID_VIEW_FORWARD, OnViewForward)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FORWARD, OnUpdateViewForward)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_ADJUST, OnUpdateStatusAdjust)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_MODE, OnUpdateStatusMode)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_PAGE, OnUpdateStatusPage)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_SIZE, OnUpdateStatusSize)
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_SHOWALLLINKS, OnShowAllLinks)
#ifdef ELIBRA_READER
	ON_COMMAND(ID_HELP_CONTENTS, OnHelpContents)
	ON_COMMAND(IDC_STATIC_LINK, OnGoToHomepage)
#endif
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR
#ifndef ELIBRA_READER
	,
	ID_INDICATOR_ADJUST,
	ID_INDICATOR_MODE,
	ID_INDICATOR_PAGE,
	ID_INDICATOR_SIZE
#endif
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
	: m_pFindDlg(NULL), m_bFirstShow(true), m_historyPos(m_history.end()),
	  m_pFullscreenWnd(NULL)
{
}

CMainFrame::~CMainFrame()
{
	if (m_pFindDlg != NULL)
	{
		m_pFindDlg->DestroyWindow();
		m_pFindDlg = NULL;
	}
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_GRIPPER | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.CreateEx(this, SBT_TOOLTIPS) ||
		!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetHeight(30);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.SetWindowText(_T("Toolbar"));

	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	int nComboPage = m_wndToolBar.CommandToIndex(ID_VIEW_NEXTPAGE) - 1;
	m_wndToolBar.SetButtonInfo(nComboPage, IDC_PAGENUM, TBBS_SEPARATOR, 65);

	CRect rcCombo;
	m_wndToolBar.GetItemRect(nComboPage, rcCombo);
	rcCombo.DeflateRect(3, 0);
	rcCombo.bottom += 400;

	// Create bold font for combo boxes in toolbar
	CFont fnt;
	CreateSystemDialogFont(fnt);
	LOGFONT lf;
	fnt.GetLogFont(&lf);

	lf.lfWeight = FW_BOLD;
	m_font.CreateFontIndirect(&lf);

	m_cboPage.Create(WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_VSCROLL | CBS_DROPDOWN,
		rcCombo, &m_wndToolBar, IDC_PAGENUM);
	m_cboPage.SetFont(&m_font);
	m_cboPage.SetItemHeight(-1, 16);
	m_cboPage.GetEditCtrl().SetInteger();

	int nComboZoom = m_wndToolBar.CommandToIndex(ID_ZOOM_IN) - 1;
	m_wndToolBar.SetButtonInfo(nComboZoom, IDC_ZOOM, TBBS_SEPARATOR, 120);

	m_wndToolBar.GetItemRect(nComboZoom, rcCombo);
	rcCombo.DeflateRect(3, 0);
	rcCombo.bottom += 400;

	m_cboZoom.Create(WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_VSCROLL | CBS_DROPDOWN,
		rcCombo, &m_wndToolBar, IDC_ZOOM);
	m_cboZoom.SetFont(&m_font);
	m_cboZoom.SetItemHeight(-1, 16);
	m_cboPage.GetEditCtrl().SetReal();
	m_cboZoom.GetEditCtrl().SetPercent();

	hHook = ::SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, NULL, ::GetCurrentThreadId());

#ifdef ELIBRA_READER
	TBBUTTON btn;
	btn.fsStyle = TBSTYLE_SEP;
	m_wndToolBar.GetToolBarCtrl().AddButtons(1, &btn);

	int nButtonLink = m_wndToolBar.GetToolBarCtrl().GetButtonCount() - 1;
	m_wndToolBar.SetButtonInfo(nButtonLink, IDC_STATIC_LINK, TBBS_SEPARATOR, 84);

	CRect rcButton;
	m_wndToolBar.GetItemRect(nButtonLink, rcButton);
	rcButton.DeflateRect(8, 0, 4, 0);
	rcButton.bottom = rcButton.top + 22;

	m_btnLink.Create(_T(""), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | BS_PUSHBUTTON,
		rcButton, &m_wndToolBar, IDC_STATIC_LINK);
	m_btnLink.LoadBitmaps(IDB_TOOLBAR_LINK);
#endif

	return 0;
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

	CControlBar* pBar = GetControlBar(ID_VIEW_TOOLBAR);
	if (pBar)
		CAppSettings::bToolbar = (pBar->GetStyle() & WS_VISIBLE) != 0;
}

void CMainFrame::OnViewStatusBar()
{
	CMDIFrameWnd::OnBarCheck(ID_VIEW_STATUS_BAR);

	CControlBar* pBar = GetControlBar(ID_VIEW_STATUS_BAR);
	if (pBar)
		CAppSettings::bStatusBar = (pBar->GetStyle() & WS_VISIBLE) != 0;
}

void CMainFrame::UpdateSettings()
{
	CRect rect;
	GetWindowRect(rect);

	if (IsZoomed())
	{
		CAppSettings::bWindowMaximized = true;
	}
	else
	{
		CAppSettings::bWindowMaximized = false;
		CAppSettings::nWindowPosX = rect.left;
		CAppSettings::nWindowPosY = rect.top;
		CAppSettings::nWindowWidth = rect.right - rect.left;
		CAppSettings::nWindowHeight = rect.bottom - rect.top;
	}
}

void CMainFrame::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CMDIFrameWnd::OnWindowPosChanged(lpwndpos);

	if (IsWindowVisible() && !IsIconic() && !m_bFirstShow)
		UpdateSettings();
}

void CMainFrame::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if (bShow && m_bFirstShow)
	{
		CControlBar* pBar = GetControlBar(ID_VIEW_TOOLBAR);
		if (pBar)
			ShowControlBar(pBar, CAppSettings::bToolbar, FALSE);

		pBar = GetControlBar(ID_VIEW_STATUS_BAR);
		if (pBar)
			ShowControlBar(pBar, CAppSettings::bStatusBar, FALSE);
	}

	CMDIFrameWnd::OnShowWindow(bShow, nStatus);

	if (bShow && m_bFirstShow)
	{
		m_bFirstShow = false;

		WINDOWPLACEMENT wp;
		GetWindowPlacement(&wp);

		wp.rcNormalPosition.left = CAppSettings::nWindowPosX;
		wp.rcNormalPosition.top = CAppSettings::nWindowPosY;
		wp.rcNormalPosition.right = CAppSettings::nWindowPosX + CAppSettings::nWindowWidth;
		wp.rcNormalPosition.bottom = CAppSettings::nWindowPosY + CAppSettings::nWindowHeight;
		wp.showCmd = (CAppSettings::bWindowMaximized ? SW_SHOWMAXIMIZED : SW_SHOW);

		SetWindowPlacement(&wp);
	}
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

void CMainFrame::OnCancelChangePageZoom()
{
	CChildFrame* pFrame = (CChildFrame*)MDIGetActive();
	if (pFrame == NULL)
		return;

	CDjVuView* pView = pFrame->GetDjVuView();
	pView->SetFocus();
}

void CMainFrame::UpdateZoomCombo(int nZoomType, double fZoom)
{
	if (m_cboZoom.GetCount() == 0)
	{
		m_cboZoom.AddString(_T("50%"));
		m_cboZoom.AddString(_T("75%"));
		m_cboZoom.AddString(_T("100%"));
		m_cboZoom.AddString(_T("200%"));
		m_cboZoom.AddString(_T("400%"));
		m_cboZoom.AddString(_T("Fit Width"));
		m_cboZoom.AddString(_T("Fit Height"));
		m_cboZoom.AddString(_T("Fit Page"));
		m_cboZoom.AddString(_T("Actual Size"));
		m_cboZoom.AddString(_T("Stretch"));
	}

	if (nZoomType == CDjVuView::ZoomFitWidth)
		m_cboZoom.SelectString(-1, _T("Fit Width"));
	else if (nZoomType == CDjVuView::ZoomFitHeight)
		m_cboZoom.SelectString(-1, _T("Fit Height"));
	else if (nZoomType == CDjVuView::ZoomFitPage)
		m_cboZoom.SelectString(-1, _T("Fit Page"));
	else if (nZoomType == CDjVuView::ZoomActualSize)
		m_cboZoom.SelectString(-1, _T("Actual Size"));
	else if (nZoomType == CDjVuView::ZoomStretch)
		m_cboZoom.SelectString(-1, _T("Stretch"));
	else
		m_cboZoom.SetWindowText(FormatDouble(fZoom) + "%");
}

void CMainFrame::UpdatePageCombo(CDjVuView* pView)
{
	if (pView->GetMode() == CDjVuView::Fullscreen)
		return;

	if (pView->GetPageCount() != m_cboPage.GetCount())
	{
		m_cboPage.ResetContent();
		CString strTmp;
		for (int i = 1; i <= pView->GetPageCount(); ++i)
		{
			strTmp.Format(_T("%d"), i);
			m_cboPage.AddString(strTmp);
		}
	}

	int nPage = pView->GetCurrentPage();
	if (m_cboPage.GetCurSel() != nPage)
		m_cboPage.SetCurSel(nPage);

	CThumbnailsView* pThumbnails = ((CChildFrame*)pView->GetParentFrame())->GetThumbnailsView();
	if (pThumbnails != NULL)
	{
		if (pThumbnails->GetCurrentPage() != nPage)
		{
			pThumbnails->EnsureVisible(nPage);
			pThumbnails->SetCurrentPage(nPage);
		}

		if (pThumbnails->GetRotate() != pView->GetRotate())
			pThumbnails->SetRotate(pView->GetRotate());
	}
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

	if (strZoom == _T("Fit Width"))
		pView->ZoomTo(CDjVuView::ZoomFitWidth);
	else if (strZoom == _T("Fit Height"))
		pView->ZoomTo(CDjVuView::ZoomFitHeight);
	else if (strZoom == _T("Fit Page"))
		pView->ZoomTo(CDjVuView::ZoomFitPage);
	else if (strZoom == _T("Actual Size"))
		pView->ZoomTo(CDjVuView::ZoomActualSize);
	else if (strZoom == _T("Stretch"))
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

	if (strZoom == _T("Fit Width"))
		pView->ZoomTo(CDjVuView::ZoomFitWidth);
	else if (strZoom == _T("Fit Height"))
		pView->ZoomTo(CDjVuView::ZoomFitHeight);
	else if (strZoom == _T("Fit Page"))
		pView->ZoomTo(CDjVuView::ZoomFitPage);
	else if (strZoom == _T("Actual Size"))
		pView->ZoomTo(CDjVuView::ZoomActualSize);
	else if (strZoom == _T("Stretch"))
		pView->ZoomTo(CDjVuView::ZoomStretch);
	else
	{
		double fZoom;
		if (_stscanf(strZoom, _T("%lf"), &fZoom) == 1)
			pView->ZoomTo(CDjVuView::ZoomPercent, fZoom);
	}

	pView->SetFocus();
}

LRESULT CMainFrame::OnDDEExecute(WPARAM wParam, LPARAM lParam)
{
	// From CFrameWnd::OnDDEExecute

	// unpack the DDE message
	UINT_PTR unused;
	HGLOBAL hData;
	//IA64: Assume DDE LPARAMs are still 32-bit
	VERIFY(UnpackDDElParam(WM_DDE_EXECUTE, lParam, &unused, (UINT_PTR*)&hData));

	// get the command string
	TCHAR szCommand[_MAX_PATH * 2];
	LPCTSTR lpsz = (LPCTSTR)GlobalLock(hData);
	int commandLength = lstrlen(lpsz);
	if (commandLength >= _countof(szCommand))
	{
		// The command would be truncated. This could be a security problem
		TRACE0("Warning: Command was ignored because it was too long.\n");
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
		TRACE(traceAppMsg, 0, _T("Warning: DDE command '%s' ignored because window is disabled.\n"),
			szCommand);
		return 0;
	}

	// execute the command
	if (!AfxGetApp()->OnDDECommand(szCommand))
		TRACE(traceAppMsg, 0, _T("Error: failed to execute DDE command '%s'.\n"), szCommand);

	return 0L;
}

void CMainFrame::OnViewFind()
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

void CMainFrame::OnUpdateViewFind(CCmdUI* pCmdUI)
{
	CMDIChildWnd* pFrame = MDIGetActive();
	if (pFrame == NULL)
	{
		pCmdUI->Enable(false);
		return;
	}

	CDjVuDoc* pDoc = (CDjVuDoc*)pFrame->GetActiveDocument();
	pCmdUI->Enable(pDoc->HasText());
}

void CMainFrame::HilightStatusMessage(LPCTSTR pszMessage)
{
	CControlBar* pBar = GetControlBar(ID_VIEW_STATUS_BAR);
	if (pBar)
		ShowControlBar(pBar, TRUE, FALSE);

	CAppSettings::bStatusBar = true;

	m_wndStatusBar.SetHilightMessage(pszMessage);
}

void CMainFrame::OnGoToHomepage()
{
	::ShellExecute(NULL, "open", "http://www.starpath.com/elibra",
		NULL, NULL, SW_SHOWNORMAL);
}

void CMainFrame::OnHelpContents()
{
	CString strPathName;
	GetModuleFileName(theApp.m_hInstance, strPathName.GetBuffer(_MAX_PATH), _MAX_PATH);
	strPathName.ReleaseBuffer();

	TCHAR szDrive[_MAX_DRIVE + 1] = {0};
	TCHAR szDir[_MAX_DIR + 1] = {0};
	TCHAR szExt[_MAX_EXT + 1] = {0};
	_tsplitpath(strPathName, szDrive, szDir, NULL, NULL);

	strPathName = CString(szDrive) + CString(szDir) + "elibra-help.chm";
	::ShellExecute(NULL, "open", strPathName, NULL, NULL, SW_SHOWNORMAL);
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
				CFrameWnd* pFrame = pView->GetParentFrame();
				pFrame->ActivateFrame();
			}

			++nDoc;
		}
	}
}

void CMainFrame::GoToHistoryPos(const HistoryPos& pos)
{
	CDjVuDoc* pDoc = theApp.OpenDocument(pos.strFileName, "");
	if (pDoc == NULL)
	{
		AfxMessageBox(_T("Failed to open document\n") + pos.strFileName, MB_ICONERROR | MB_OK);
		return;
	}

	CDjVuView* pView = pDoc->GetDjVuView();
	pView->GoToPage(pos.nPage, -1, CDjVuView::DoNotAdd);
}

void CMainFrame::OnViewBack()
{
	if (m_history.empty() || m_historyPos == m_history.begin())
		return;

	const HistoryPos& pos = *(--m_historyPos);
	GoToHistoryPos(pos);
}

void CMainFrame::OnUpdateViewBack(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pFullscreenWnd == NULL && !m_history.empty()
		&& m_historyPos != m_history.begin());
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
	if (m_pFullscreenWnd != NULL || m_history.empty())
	{
		pCmdUI->Enable(false);
	}
	else
	{
		list<HistoryPos>::iterator it = m_history.end();
		pCmdUI->Enable(m_historyPos != m_history.end() && m_historyPos != --it);
	}
}

void CMainFrame::AddToHistory(CDjVuView* pView, int nPage)
{
	HistoryPos pos;
	pos.strFileName = pView->GetDocument()->GetPathName();
	pos.nPage = nPage;

	if (!m_history.empty() && pos == *m_historyPos)
		return;

	if (!m_history.empty())
	{
		++m_historyPos;
		m_history.erase(m_historyPos, m_history.end());
	}

	m_history.push_back(pos);

	m_historyPos = m_history.end();
	--m_historyPos;
}

void CMainFrame::OnUpdateStatusAdjust(CCmdUI* pCmdUI)
{
	static CString strMessage, strTooltip;
	CStatusBarCtrl& status = m_wndStatusBar.GetStatusBarCtrl();

	if (CAppSettings::displaySettings == CDisplaySettings() || MDIGetActive() == NULL)
	{
		m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_ADJUST, SBPS_DISABLED | SBPS_NOBORDERS, 0);
		pCmdUI->Enable(false);
		strMessage.Empty();
		return;
	}

	CString strNewMessage;
	strNewMessage.LoadString(ID_INDICATOR_ADJUST);

	if (strMessage != strNewMessage)
	{
		strMessage = strNewMessage;
		pCmdUI->SetText(strMessage + _T("          "));
		status.SetText(strMessage + _T("          "), 1, 0);

		CWindowDC dc(&status);
		CFont* pOldFont = dc.SelectObject(status.GetFont());
		m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_ADJUST, 0, dc.GetTextExtent(strMessage).cx + 2);
		dc.SelectObject(pOldFont);
	}

	CString strNewTooltip, strTemp;

	int nBrightness = CAppSettings::displaySettings.GetBrightness();
	if (nBrightness != 0)
	{
		strTemp.Format(_T("Brightness: %+d"), nBrightness);
		strNewTooltip += (!strNewTooltip.IsEmpty() ? _T(", ") : _T("")) + strTemp;
	}

	int nContrast = CAppSettings::displaySettings.GetContrast();
	if (nContrast != 0)
	{
		strTemp.Format(_T("Contrast: %+d"), nContrast);
		strNewTooltip += (!strNewTooltip.IsEmpty() ? _T(", ") : _T("")) + strTemp;
	}

	double fGamma = CAppSettings::displaySettings.GetGamma();
	if (fGamma != 1.0)
	{
		strTemp.Format(_T("Gamma: %1.1f"), fGamma);
		strNewTooltip += (!strNewTooltip.IsEmpty() ? _T(", ") : _T("")) + strTemp;
	}

	if (strTooltip != strNewTooltip)
	{
		strTooltip = strNewTooltip;
		status.SetTipText(1, strTooltip);
	}
}

void CMainFrame::OnUpdateStatusMode(CCmdUI* pCmdUI)
{
	static CString strMessage;

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
			strNewMessage = _T("  Black & White  ");
			break;

		case CDjVuView::Foreground:
			strNewMessage = _T("  Foreground  ");
			break;

		case CDjVuView::Background:
			strNewMessage = _T("  Background  ");
			break;
		}
	}

	if (strNewMessage.IsEmpty())
	{
		m_wndStatusBar.SetPaneInfo(2, ID_INDICATOR_MODE, SBPS_DISABLED | SBPS_NOBORDERS, 0);
		pCmdUI->Enable(false);
		strMessage.Empty();
	}
	else if (strMessage != strNewMessage)
	{
		CStatusBarCtrl& status = m_wndStatusBar.GetStatusBarCtrl();

		strMessage = strNewMessage;
		pCmdUI->SetText(strMessage);
		status.SetText(strMessage, 2, 0);

		CWindowDC dc(&status);
		CFont* pOldFont = dc.SelectObject(status.GetFont());
		m_wndStatusBar.SetPaneInfo(2, ID_INDICATOR_MODE, 0, dc.GetTextExtent(strMessage).cx + 2);
		dc.SelectObject(pOldFont);
	}
}

void CMainFrame::OnUpdateStatusPage(CCmdUI* pCmdUI)
{
	static CString strMessage;

	CMDIChildWnd* pActive = MDIGetActive();
	if (pActive == NULL)
	{
		m_wndStatusBar.SetPaneInfo(3, ID_INDICATOR_PAGE, SBPS_DISABLED | SBPS_NOBORDERS, 0);
		pCmdUI->Enable(false);
		strMessage.Empty();
		return;
	}

	CDjVuView* pView = (CDjVuView*)pActive->GetActiveView();
	ASSERT(pView);

	CString strNewMessage;
	strNewMessage.Format(ID_INDICATOR_PAGE, pView->GetCurrentPage() + 1, pView->GetPageCount());

	if (strMessage != strNewMessage)
	{
		CStatusBarCtrl& status = m_wndStatusBar.GetStatusBarCtrl();

		strMessage = strNewMessage;
		pCmdUI->SetText(strMessage);
		status.SetText(strMessage, 3, 0);

		CWindowDC dc(&status);
		CFont* pOldFont = dc.SelectObject(status.GetFont());
		m_wndStatusBar.SetPaneInfo(3, ID_INDICATOR_PAGE, 0, dc.GetTextExtent(strMessage).cx + 2);
		dc.SelectObject(pOldFont);
	}
}

void CMainFrame::OnUpdateStatusSize(CCmdUI* pCmdUI)
{
	static CString strMessage;

	CMDIChildWnd* pActive = MDIGetActive();
	if (pActive == NULL)
	{
		m_wndStatusBar.SetPaneInfo(4, ID_INDICATOR_SIZE, SBPS_DISABLED | SBPS_NOBORDERS, 0);
		pCmdUI->Enable(false);
		strMessage.Empty();
		return;
	}

	CDjVuView* pView = (CDjVuView*)pActive->GetActiveView();
	ASSERT(pView);
	int nCurrentPage = pView->GetCurrentPage();

	CString strNewMessage;
	CSize szPage = pView->GetPageSize(nCurrentPage);
	int nDPI = pView->GetPageDPI(nCurrentPage);
	double fWidth = static_cast<int>(25.4 * szPage.cx / nDPI) * 0.1;
	double fHeight = static_cast<int>(25.4 * szPage.cy / nDPI) * 0.1;

	strNewMessage.Format(ID_INDICATOR_SIZE, 
			(LPCTSTR)FormatDouble(fWidth), (LPCTSTR)FormatDouble(fHeight), "cm");

	if (strMessage != strNewMessage)
	{
		CStatusBarCtrl& status = m_wndStatusBar.GetStatusBarCtrl();

		strMessage = strNewMessage;
		pCmdUI->SetText(strMessage);
		status.SetText(strMessage, 4, 0);

		CWindowDC dc(&status);
		CFont* pOldFont = dc.SelectObject(status.GetFont());
		m_wndStatusBar.SetPaneInfo(4, ID_INDICATOR_SIZE, 0, dc.GetTextExtent(strMessage).cx + 2);
		dc.SelectObject(pOldFont);
	}
}

BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (m_pFullscreenWnd != NULL)
		return m_pFullscreenWnd->GetView()->SendMessage(WM_COMMAND, wParam, lParam);

	return CMDIFrameWnd::OnCommand(wParam, lParam);
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (m_pFullscreenWnd != NULL)
		return m_pFullscreenWnd->GetView()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);

	return CMDIFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
	if (m_pFullscreenWnd != NULL)
	{
		m_pFullscreenWnd->SetForegroundWindow();
		m_pFullscreenWnd->SetFocus();
		return;
	}

	CMDIFrameWnd::OnSetFocus(pOldWnd);
}

LRESULT CALLBACK CMainFrame::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION && wParam == VK_SHIFT)
	{
		static bool bWasPressed = false;
		bool bPressed = (lParam & 0x80000000) == 0;

		if (bPressed != bWasPressed)
		{
			bWasPressed = bPressed;
			GetMainFrame()->PostMessage(WM_SHOWALLLINKS, bPressed);
		}
	}

	return ::CallNextHookEx(hHook, nCode, wParam, lParam);
}

void CMainFrame::OnDestroy()
{
	::UnhookWindowsHookEx(hHook);

	CMDIFrameWnd::OnDestroy();
}

LRESULT CMainFrame::OnShowAllLinks(WPARAM wParam, LPARAM lParam)
{
	POSITION pos = theApp.GetFirstDocTemplatePosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = theApp.GetNextDocTemplate(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);

		POSITION posDoc = pTemplate->GetFirstDocPosition();
		while (posDoc != NULL)
		{
			CDocument* pDoc = pTemplate->GetNextDoc(posDoc);
			CDjVuView* pView = ((CDjVuDoc*)pDoc)->GetDjVuView();
			pView->ShowAllLinks(wParam != 0);
		}
	}

	return 0;
}
