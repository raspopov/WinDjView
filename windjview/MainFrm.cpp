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
//  http://www.gnu.org/copyleft/gpl.html

// $Id$

#include "stdafx.h"
#include "WinDjView.h"
#include "DjVuView.h"
#include "DjVuDoc.h"

#include "MainFrm.h"
#include "AppSettings.h"

#include <dde.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
	: m_pFindDlg(NULL), m_bFirstShow(true)
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

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
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

	int nComboZoom = m_wndToolBar.CommandToIndex(ID_ZOOM_100) - 1;
	m_wndToolBar.SetButtonInfo(nComboZoom, IDC_ZOOM, TBBS_SEPARATOR, 120);

	m_wndToolBar.GetItemRect(nComboZoom, rcCombo);
	rcCombo.DeflateRect(8, 0, 3, 0);
	rcCombo.bottom += 400;

	m_cboZoom.Create(WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_VSCROLL | CBS_DROPDOWN,
		rcCombo, &m_wndToolBar, IDC_ZOOM);
	m_cboZoom.SetFont(&m_font);
	m_cboZoom.SetItemHeight(-1, 16);
	m_cboPage.GetEditCtrl().SetReal();
	m_cboZoom.GetEditCtrl().SetPercent();

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIFrameWnd::PreCreateWindow(cs))
		return false;

	return true;
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
	CMDIChildWnd* pFrame = MDIGetActive();
	if (pFrame == NULL)
		return;

	CDjVuView* pView = (CDjVuView*)pFrame->GetActiveView();

	int nPage = m_cboPage.GetCurSel();
	if (nPage >= 0 && nPage < pView->GetPageCount())
		pView->RenderPage(nPage);

	pView->SetFocus();
}

void CMainFrame::OnChangePageEdit()
{
	CMDIChildWnd* pFrame = MDIGetActive();
	if (pFrame == NULL)
		return;

	CDjVuView* pView = (CDjVuView*)pFrame->GetActiveView();

	CString strPage;
	m_cboPage.GetWindowText(strPage);

	int nPage;
	if (_stscanf(strPage, _T("%d"), &nPage) == 1)
	{
		if (nPage >= 1 && nPage <= pView->GetPageCount())
			pView->RenderPage(nPage - 1);
	}

	pView->SetFocus();
}

void CMainFrame::OnCancelChangePageZoom()
{
	CMDIChildWnd* pFrame = MDIGetActive();
	if (pFrame == NULL)
		return;

	CDjVuView* pView = (CDjVuView*)pFrame->GetActiveView();
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

void CMainFrame::UpdatePageCombo(int nPage, int nPages)
{
	if (nPages != -1)
	{
		m_cboPage.ResetContent();
		CString strTmp;
		for (int i = 1; i <= nPages; ++i)
		{
			strTmp.Format(_T("%d"), i);
			m_cboPage.AddString(strTmp);
		}
	}

	if (m_cboPage.GetCurSel() != nPage)
	{
		m_cboPage.SetCurSel(nPage);
	}
}

void CMainFrame::OnChangeZoom()
{
	CMDIChildWnd* pFrame = MDIGetActive();
	if (pFrame == NULL)
		return;

	CDjVuView* pView = (CDjVuView*)pFrame->GetActiveView();

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
	CMDIChildWnd* pFrame = MDIGetActive();
	if (pFrame == NULL)
		return;

	CDjVuView* pView = (CDjVuView*)pFrame->GetActiveView();

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

void CMainFrame::OnUpdateViewFind(CCmdUI *pCmdUI)
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
