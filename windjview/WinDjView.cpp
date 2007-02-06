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
#include "MainFrm.h"

#include "ChildFrm.h"
#include "MyBitmapButton.h"
#include "DjVuDoc.h"
#include "DjVuView.h"
#include "MyDocManager.h"
#include "MyDocTemplate.h"
#include "AppSettings.h"
#include "SettingsDlg.h"
#include "UpdateDlg.h"
#include "ThumbnailsView.h"
#include "BookmarksWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString CURRENT_VERSION;
BOOL s_bFatalError = false;


const TCHAR* s_pszDisplaySettings = _T("Display");
const TCHAR* s_pszPosX = _T("x");
const TCHAR* s_pszPosY = _T("y");
const TCHAR* s_pszWidth = _T("width");
const TCHAR* s_pszHeight = _T("height");
const TCHAR* s_pszMaximized = _T("maximized");
const TCHAR* s_pszChildMaximized = _T("child-maximized");
const TCHAR* s_pszToolbar = _T("toobar");
const TCHAR* s_pszStatusBar = _T("statusbar");
const TCHAR* s_pszZoom = _T("zoom");
const TCHAR* s_pszZoomPercent = _T("%");
const TCHAR* s_pszLayout = _T("layout");
const TCHAR* s_pszFirstPage = _T("first-page");
const TCHAR* s_pszMode = _T("mode");
const TCHAR* s_pszNavCollapsed = _T("nav-collapsed");
const TCHAR* s_pszNavWidth = _T("nav-width");
const TCHAR* s_pszAdjustDisplay = _T("adjust");
const TCHAR* s_pszGamma = _T("gamma");
const TCHAR* s_pszBrightness = _T("brightness");
const TCHAR* s_pszContrast = _T("contrast");
const TCHAR* s_pszScaleMethod = _T("scale-method");
const TCHAR* s_pszInvertColors = _T("invert");

const TCHAR* s_pszGlobalSettings = _T("Settings");
const TCHAR* s_pszRestoreAssocs = _T("assocs");
const TCHAR* s_pszGenAllThumbnails = _T("gen-all-thumbs");
const TCHAR* s_pszFullscreenClicks = _T("fullscreen-clicks");
const TCHAR* s_pszFullscreenHideScroll = _T("fullscreen-hide-scroll");
const TCHAR* s_pszWarnCloseMultiple = _T("warn-close-multiple");
const TCHAR* s_pszInvertWheelZoom = _T("invert-wheel-zoom");
const TCHAR* s_pszCloseOnEsc = _T("close-on-esc");
const TCHAR* s_pszWrapLongBookmarks = _T("wrap-long-bookmarks");
const TCHAR* s_pszRestoreView = _T("restore-view");
const TCHAR* s_pszVersion = _T("version");
const TCHAR* s_pszLanguage = _T("language");
const TCHAR* s_pszFind = _T("find");
const TCHAR* s_pszMatchCase = _T("match-case");

const TCHAR* s_pszPrintSettings = _T("Print");
const TCHAR* s_pszMarginLeft = _T("m-left");
const TCHAR* s_pszMarginTop = _T("m-top");
const TCHAR* s_pszMarginRight = _T("m-right");
const TCHAR* s_pszMarginBottom = _T("m-bottom");
const TCHAR* s_pszPosLeft = _T("left");
const TCHAR* s_pszPosTop = _T("top");
const TCHAR* s_pszCenterImage = _T("center");
const TCHAR* s_pszIgnoreMargins = _T("no-margins");
const TCHAR* s_pszShrinkOversized = _T("shrink");

const TCHAR* s_pszDocumentsSettings = _T("Documents");
const TCHAR* s_pszStartupPage = _T("page");
const TCHAR* s_pszDisplayMode = _T("display");


// CDjViewApp

BEGIN_MESSAGE_MAP(CDjViewApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	ON_COMMAND(ID_FILE_SETTINGS, OnFileSettings)
	ON_COMMAND(ID_CHECK_FOR_UPDATE, OnCheckForUpdate)
	ON_COMMAND_EX_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnOpenRecentFile)
END_MESSAGE_MAP()


// CDjViewApp construction

CDjViewApp::CDjViewApp()
	: m_bInitialized(false), m_pDjVuTemplate(NULL)
{
}


// The one and only CDjViewApp object

CDjViewApp theApp;

// CDjViewApp initialization

BOOL CDjViewApp::InitInstance()
{
	_tsetlocale(LC_ALL, _T(""));

	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	// Standard initialization

	// Change the registry key under which our settings are stored
	SetRegistryKey(_T("Andrew Zhezherun"));

	CURRENT_VERSION.LoadString(IDS_CURRENT_VERSION);
	LoadStdProfileSettings(10);  // Load standard INI file options (including MRU)
	LoadSettings();

	m_pDocManager = new CMyDocManager();

	// Register the application's document template
	m_pDjVuTemplate = new CMyDocTemplate(IDR_DjVuTYPE,
		RUNTIME_CLASS(CDjVuDoc),
		RUNTIME_CLASS(CChildFrame),
		RUNTIME_CLASS(CDjVuView));
	if (!m_pDjVuTemplate)
		return FALSE;
	AddDocTemplate(m_pDjVuTemplate);

	// Create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;
	pMainFrame->LockWindowUpdate();

	CControlBar* pBar = pMainFrame->GetControlBar(ID_VIEW_TOOLBAR);
	if (pBar)
		pMainFrame->ShowControlBar(pBar, CAppSettings::bToolbar, FALSE);

	pBar = pMainFrame->GetControlBar(ID_VIEW_STATUS_BAR);
	if (pBar)
		pMainFrame->ShowControlBar(pBar, CAppSettings::bStatusBar, FALSE);

	pMainFrame->SetWindowPos(NULL, CAppSettings::nWindowPosX, CAppSettings::nWindowPosY,
				CAppSettings::nWindowWidth, CAppSettings::nWindowHeight,
				SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();

	if (CAppSettings::bRestoreAssocs)
		RegisterShellFileTypes();

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	if (CAppSettings::bWindowMaximized && (m_nCmdShow == -1 || m_nCmdShow == SW_SHOWNORMAL))
		m_nCmdShow = SW_SHOWMAXIMIZED;

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->SetStartupLanguage();

	pMainFrame->UnlockWindowUpdate();
	pMainFrame->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

	m_bInitialized = true;
	CChildFrame* pChildFrame = (CChildFrame*)pMainFrame->MDIGetActive();
	if (pChildFrame != NULL)
		pChildFrame->GetDjVuView()->UpdateVisiblePages();

	if (CAppSettings::strVersion != CURRENT_VERSION)
		OnAppAbout();

	return TRUE;
}

void CDjViewApp::EnableShellOpen()
{
	ASSERT(m_atomApp == NULL && m_atomSystemTopic == NULL); // do once

	m_atomApp = ::GlobalAddAtom(_T("WinDjView"));
	m_atomSystemTopic = ::GlobalAddAtom(_T("System"));
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };
	CStatic m_weblinkLibrary;
	CStatic m_weblink;
	CMyBitmapButton m_btnDonate;

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
public:
	virtual BOOL OnInitDialog();

// Implementation
protected:
	CFont m_font;

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDonate();
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_LINK, m_weblink);
	DDX_Control(pDX, IDC_STATIC_LIB_LINK, m_weblinkLibrary);
	DDX_Control(pDX, IDC_DONATE, m_btnDonate);

	CString strVersion;
	strVersion.Format(IDS_VERSION_ABOUT, CURRENT_VERSION);
	DDX_Text(pDX, IDC_STATIC_VERSION, strVersion);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_DONATE, OnDonate)
END_MESSAGE_MAP()

// App command to run the dialog
void CDjViewApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH brush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetSafeHwnd() == m_weblink.m_hWnd
			|| pWnd->GetSafeHwnd() == m_weblinkLibrary.m_hWnd)
	{
		pDC->SetTextColor(RGB(0, 0, 204));
	}

	return brush;
}

BOOL CAboutDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	static HCURSOR hCursor = NULL;
	if (hCursor == NULL)
	{
		hCursor = ::LoadCursor(NULL, IDC_HAND);
		if (hCursor == NULL)
			hCursor = theApp.LoadCursor(IDC_CURSOR_LINK);
	}

	CPoint ptCursor;
	::GetCursorPos(&ptCursor);

	CRect rcWeblink;
	m_weblink.GetWindowRect(rcWeblink);

	if (rcWeblink.PtInRect(ptCursor))
	{
		::SetCursor(hCursor);
		return true;
	}

	CRect rcWeblinkLibrary;
	m_weblinkLibrary.GetWindowRect(rcWeblinkLibrary);

	if (rcWeblinkLibrary.PtInRect(ptCursor))
	{
		::SetCursor(hCursor);
		return true;
	}

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CAboutDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rcWeblink;
	m_weblink.GetWindowRect(rcWeblink);
	ScreenToClient(rcWeblink);

	if (rcWeblink.PtInRect(point))
	{
		::ShellExecute(NULL, _T("open"), LoadString(IDS_WEBSITE_URL),
			NULL, NULL, SW_SHOWNORMAL);
		return;
	}

	CRect rcWeblinkLibrary;
	m_weblinkLibrary.GetWindowRect(rcWeblinkLibrary);
	ScreenToClient(rcWeblinkLibrary);

	if (rcWeblinkLibrary.PtInRect(point))
	{
		::ShellExecute(NULL, _T("open"), LoadString(IDS_DJVULIBRE_URL),
			NULL, NULL, SW_SHOWNORMAL);
		return;
	}

	CDialog::OnLButtonDown(nFlags, point);
}

void CAboutDlg::OnDonate()
{
	::ShellExecute(NULL, _T("open"), LoadString(IDS_DONATE_URL),
		NULL, NULL, SW_SHOWNORMAL);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFont fnt;
	CreateSystemDialogFont(fnt);
	LOGFONT lf;
	fnt.GetLogFont(&lf);

	lf.lfUnderline = true;
	m_font.CreateFontIndirect(&lf);

	m_weblink.SetFont(&m_font);
	m_weblinkLibrary.SetFont(&m_font);

	m_btnDonate.LoadBitmaps(IDB_DONATE);

	return true;
}


// CDjViewApp message handlers

BOOL CDjViewApp::WriteProfileDouble(LPCTSTR pszSection, LPCTSTR pszEntry, double fValue)
{
	CString strLocale = _tsetlocale(LC_NUMERIC, NULL);

	_tsetlocale(LC_NUMERIC, _T("C"));
	BOOL result = WriteProfileString(pszSection, pszEntry, FormatDouble(fValue));
	_tsetlocale(LC_NUMERIC, strLocale);

	return result;
}

double CDjViewApp::GetProfileDouble(LPCTSTR pszSection, LPCTSTR pszEntry, double fDefault)
{
	CString strLocale = _tsetlocale(LC_NUMERIC, NULL);
	_tsetlocale(LC_NUMERIC, _T("C"));

	CString strValue = GetProfileString(pszSection, pszEntry);

	double fValue;
	int result = _stscanf(strValue, _T("%lf"), &fValue);

	_tsetlocale(LC_NUMERIC, strLocale);

	return (result != 1 ? fDefault : fValue);
}

void CDjViewApp::LoadSettings()
{
	CAppSettings::nWindowPosX = GetProfileInt(s_pszDisplaySettings, s_pszPosX, CAppSettings::nWindowPosX);
	CAppSettings::nWindowPosY = GetProfileInt(s_pszDisplaySettings, s_pszPosY, CAppSettings::nWindowPosY);
	CAppSettings::nWindowWidth = GetProfileInt(s_pszDisplaySettings, s_pszWidth, CAppSettings::nWindowWidth);
	CAppSettings::nWindowHeight = GetProfileInt(s_pszDisplaySettings, s_pszHeight, CAppSettings::nWindowHeight);
	CAppSettings::bWindowMaximized = !!GetProfileInt(s_pszDisplaySettings, s_pszMaximized, CAppSettings::bWindowMaximized);
	CAppSettings::bChildMaximized = !!GetProfileInt(s_pszDisplaySettings, s_pszChildMaximized, CAppSettings::bChildMaximized);
	CAppSettings::bToolbar = !!GetProfileInt(s_pszDisplaySettings, s_pszToolbar, CAppSettings::bToolbar);
	CAppSettings::bStatusBar = !!GetProfileInt(s_pszDisplaySettings, s_pszStatusBar, CAppSettings::bStatusBar);
	CAppSettings::nDefaultZoomType = GetProfileInt(s_pszDisplaySettings, s_pszZoom, CAppSettings::nDefaultZoomType);
	CAppSettings::fDefaultZoom = GetProfileDouble(s_pszDisplaySettings, s_pszZoomPercent, CAppSettings::fDefaultZoom);
	CAppSettings::nDefaultLayout = GetProfileInt(s_pszDisplaySettings, s_pszLayout, CAppSettings::nDefaultLayout);
	CAppSettings::bFirstPageAlone = !!GetProfileInt(s_pszDisplaySettings, s_pszFirstPage, CAppSettings::bFirstPageAlone);
	CAppSettings::nDefaultMode = GetProfileInt(s_pszDisplaySettings, s_pszMode, CAppSettings::nDefaultMode);
	CAppSettings::bNavPaneCollapsed = !!GetProfileInt(s_pszDisplaySettings, s_pszNavCollapsed, CAppSettings::bNavPaneCollapsed);
	CAppSettings::nNavPaneWidth = GetProfileInt(s_pszDisplaySettings, s_pszNavWidth, CAppSettings::nNavPaneWidth);

	CAppSettings::bRestoreAssocs = !!GetProfileInt(s_pszGlobalSettings, s_pszRestoreAssocs, CAppSettings::bRestoreAssocs);
	CAppSettings::bGenAllThumbnails = !!GetProfileInt(s_pszGlobalSettings, s_pszGenAllThumbnails, CAppSettings::bGenAllThumbnails);
	CAppSettings::bFullscreenClicks = !!GetProfileInt(s_pszGlobalSettings, s_pszFullscreenClicks, CAppSettings::bFullscreenClicks);
	CAppSettings::bFullscreenHideScroll = !!GetProfileInt(s_pszGlobalSettings, s_pszFullscreenHideScroll, CAppSettings::bFullscreenHideScroll);
	CAppSettings::bWarnCloseMultiple = !!GetProfileInt(s_pszGlobalSettings, s_pszWarnCloseMultiple, CAppSettings::bWarnCloseMultiple);
	CAppSettings::bInvertWheelZoom = !!GetProfileInt(s_pszGlobalSettings, s_pszInvertWheelZoom, CAppSettings::bInvertWheelZoom);
	CAppSettings::bCloseOnEsc = !!GetProfileInt(s_pszGlobalSettings, s_pszCloseOnEsc, CAppSettings::bCloseOnEsc);
	CAppSettings::bWrapLongBookmarks = !!GetProfileInt(s_pszGlobalSettings, s_pszWrapLongBookmarks, CAppSettings::bWrapLongBookmarks);
	CAppSettings::bRestoreView = !!GetProfileInt(s_pszGlobalSettings, s_pszRestoreView, CAppSettings::bRestoreView);
	CAppSettings::strVersion = GetProfileString(s_pszGlobalSettings, s_pszVersion, CAppSettings::strVersion);
	CAppSettings::strLanguage = GetProfileString(s_pszGlobalSettings, s_pszLanguage, CAppSettings::strLanguage);
	CAppSettings::strFind = GetProfileString(s_pszGlobalSettings, s_pszFind, CAppSettings::strFind);
	CAppSettings::bMatchCase = !!GetProfileInt(s_pszGlobalSettings, s_pszMatchCase, CAppSettings::bMatchCase);

	CDisplaySettings& ds = CAppSettings::displaySettings;
	ds.nScaleMethod = GetProfileInt(s_pszDisplaySettings, s_pszScaleMethod, ds.nScaleMethod);
	ds.bInvertColors = !!GetProfileInt(s_pszDisplaySettings, s_pszInvertColors, ds.bInvertColors);
	ds.bAdjustDisplay = !!GetProfileInt(s_pszDisplaySettings, s_pszAdjustDisplay, ds.bAdjustDisplay);
	ds.fGamma = GetProfileDouble(s_pszDisplaySettings, s_pszGamma, ds.fGamma);
	ds.nBrightness = GetProfileInt(s_pszDisplaySettings, s_pszBrightness, ds.nBrightness);
	ds.nContrast = GetProfileInt(s_pszDisplaySettings, s_pszContrast, ds.nContrast);
	ds.Fix();

	CPrintSettings& ps = CAppSettings::printSettings;
	ps.fMarginLeft = GetProfileDouble(s_pszPrintSettings, s_pszMarginLeft, ps.fMarginLeft);
	ps.fMarginTop = GetProfileDouble(s_pszPrintSettings, s_pszMarginTop, ps.fMarginTop);
	ps.fMarginRight = GetProfileDouble(s_pszPrintSettings, s_pszMarginRight, ps.fMarginRight);
	ps.fMarginBottom = GetProfileDouble(s_pszPrintSettings, s_pszMarginBottom, ps.fMarginBottom);
	ps.fPosLeft = GetProfileDouble(s_pszPrintSettings, s_pszPosLeft, ps.fPosLeft);
	ps.fPosTop = GetProfileDouble(s_pszPrintSettings, s_pszPosTop, ps.fPosTop);
	ps.bCenterImage = !!GetProfileInt(s_pszPrintSettings, s_pszCenterImage, ps.bCenterImage);
	ps.bIgnorePrinterMargins = !!GetProfileInt(s_pszPrintSettings, s_pszIgnoreMargins, ps.bIgnorePrinterMargins);
	ps.bShrinkOversized = !!GetProfileInt(s_pszPrintSettings, s_pszShrinkOversized, ps.bShrinkOversized);
}

void CDjViewApp::LoadDjVuUserData(const CString& strKey, DjVuUserData* pData)
{
	CString strDocumentKey = s_pszDocumentsSettings + CString(_T("\\")) + strKey;

	pData->nPage = GetProfileInt(strDocumentKey, s_pszStartupPage, pData->nPage);
	pData->nZoomType = GetProfileInt(strDocumentKey, s_pszZoom, pData->nZoomType);
	pData->fZoom = GetProfileDouble(strDocumentKey, s_pszZoomPercent, pData->fZoom);
	pData->ptOffset.x = GetProfileInt(strDocumentKey, s_pszPosX, pData->ptOffset.x);
	pData->ptOffset.y = GetProfileInt(strDocumentKey, s_pszPosY, pData->ptOffset.y);
	pData->nLayout = GetProfileInt(strDocumentKey, s_pszLayout, pData->nLayout);
	pData->bFirstPageAlone = !!GetProfileInt(strDocumentKey, s_pszFirstPage, pData->bFirstPageAlone);
	pData->nDisplayMode = GetProfileInt(strDocumentKey, s_pszDisplayMode, pData->nDisplayMode);
}

void CDjViewApp::SaveSettings()
{
	WriteProfileInt(s_pszDisplaySettings, s_pszPosX, CAppSettings::nWindowPosX);
	WriteProfileInt(s_pszDisplaySettings, s_pszPosY, CAppSettings::nWindowPosY);
	WriteProfileInt(s_pszDisplaySettings, s_pszWidth, CAppSettings::nWindowWidth);
	WriteProfileInt(s_pszDisplaySettings, s_pszHeight, CAppSettings::nWindowHeight);
	WriteProfileInt(s_pszDisplaySettings, s_pszMaximized, CAppSettings::bWindowMaximized);
	WriteProfileInt(s_pszDisplaySettings, s_pszChildMaximized, CAppSettings::bChildMaximized);
	WriteProfileInt(s_pszDisplaySettings, s_pszToolbar, CAppSettings::bToolbar);
	WriteProfileInt(s_pszDisplaySettings, s_pszStatusBar, CAppSettings::bStatusBar);
	WriteProfileInt(s_pszDisplaySettings, s_pszZoom, CAppSettings::nDefaultZoomType);
	WriteProfileDouble(s_pszDisplaySettings, s_pszZoomPercent, CAppSettings::fDefaultZoom);
	WriteProfileInt(s_pszDisplaySettings, s_pszLayout, CAppSettings::nDefaultLayout);
	WriteProfileInt(s_pszDisplaySettings, s_pszFirstPage, CAppSettings::bFirstPageAlone);
	WriteProfileInt(s_pszDisplaySettings, s_pszMode, CAppSettings::nDefaultMode);
	WriteProfileInt(s_pszDisplaySettings, s_pszNavCollapsed, CAppSettings::bNavPaneCollapsed);
	WriteProfileInt(s_pszDisplaySettings, s_pszNavWidth, CAppSettings::nNavPaneWidth);

	WriteProfileInt(s_pszGlobalSettings, s_pszRestoreAssocs, CAppSettings::bRestoreAssocs);
	WriteProfileInt(s_pszGlobalSettings, s_pszGenAllThumbnails, CAppSettings::bGenAllThumbnails);
	WriteProfileInt(s_pszGlobalSettings, s_pszFullscreenClicks, CAppSettings::bFullscreenClicks);
	WriteProfileInt(s_pszGlobalSettings, s_pszFullscreenHideScroll, CAppSettings::bFullscreenHideScroll);
	WriteProfileInt(s_pszGlobalSettings, s_pszWarnCloseMultiple, CAppSettings::bWarnCloseMultiple);
	WriteProfileInt(s_pszGlobalSettings, s_pszInvertWheelZoom, CAppSettings::bInvertWheelZoom);
	WriteProfileInt(s_pszGlobalSettings, s_pszCloseOnEsc, CAppSettings::bCloseOnEsc);
	WriteProfileInt(s_pszGlobalSettings, s_pszWrapLongBookmarks, CAppSettings::bWrapLongBookmarks);
	WriteProfileInt(s_pszGlobalSettings, s_pszRestoreView, CAppSettings::bRestoreView);
	WriteProfileString(s_pszGlobalSettings, s_pszVersion, CURRENT_VERSION);
	WriteProfileString(s_pszGlobalSettings, s_pszLanguage, CAppSettings::strLanguage);
	WriteProfileString(s_pszGlobalSettings, s_pszFind, CAppSettings::strFind);
	WriteProfileInt(s_pszGlobalSettings, s_pszMatchCase, CAppSettings::bMatchCase);

	CDisplaySettings& ds = CAppSettings::displaySettings;
	WriteProfileInt(s_pszDisplaySettings, s_pszScaleMethod, ds.nScaleMethod);
	WriteProfileInt(s_pszDisplaySettings, s_pszInvertColors, ds.bInvertColors);
	WriteProfileInt(s_pszDisplaySettings, s_pszAdjustDisplay, ds.bAdjustDisplay);
	WriteProfileDouble(s_pszDisplaySettings, s_pszGamma, ds.fGamma);
	WriteProfileInt(s_pszDisplaySettings, s_pszBrightness, ds.nBrightness);
	WriteProfileInt(s_pszDisplaySettings, s_pszContrast, ds.nContrast);

	CPrintSettings& ps = CAppSettings::printSettings;
	WriteProfileDouble(s_pszPrintSettings, s_pszMarginLeft, ps.fMarginLeft);
	WriteProfileDouble(s_pszPrintSettings, s_pszMarginTop, ps.fMarginTop);
	WriteProfileDouble(s_pszPrintSettings, s_pszMarginRight, ps.fMarginRight);
	WriteProfileDouble(s_pszPrintSettings, s_pszMarginBottom, ps.fMarginBottom);
	WriteProfileDouble(s_pszPrintSettings, s_pszPosLeft, ps.fPosLeft);
	WriteProfileDouble(s_pszPrintSettings, s_pszPosTop, ps.fPosTop);
	WriteProfileInt(s_pszPrintSettings, s_pszCenterImage, ps.bCenterImage);
	WriteProfileInt(s_pszPrintSettings, s_pszIgnoreMargins, ps.bIgnorePrinterMargins);
	WriteProfileInt(s_pszPrintSettings, s_pszShrinkOversized, ps.bShrinkOversized);

	const map<MD5, DjVuUserData>& userData = DjVuSource::GetAllUserData();
	for (map<MD5, DjVuUserData>::const_iterator it = userData.begin(); it != userData.end(); ++it)
	{
		CString strDocumentKey = s_pszDocumentsSettings + CString(_T("\\")) + (*it).first.ToString();
		const DjVuUserData& data = (*it).second;

		WriteProfileInt(strDocumentKey, s_pszStartupPage, data.nPage);
		WriteProfileInt(strDocumentKey, s_pszZoom, data.nZoomType);
		WriteProfileDouble(strDocumentKey, s_pszZoomPercent, data.fZoom);
		WriteProfileInt(strDocumentKey, s_pszPosX, data.ptOffset.x);
		WriteProfileInt(strDocumentKey, s_pszPosY, data.ptOffset.y);
		WriteProfileInt(strDocumentKey, s_pszLayout, data.nLayout);
		WriteProfileInt(strDocumentKey, s_pszFirstPage, data.bFirstPageAlone);
		WriteProfileInt(strDocumentKey, s_pszDisplayMode, data.nDisplayMode);
	}
}

int CDjViewApp::ExitInstance()
{
	SaveSettings();

	::CoUninitialize();

	return CWinApp::ExitInstance();
}

bool SetRegKey(LPCTSTR lpszKey, LPCTSTR lpszValue)
{
	if (::RegSetValue(HKEY_CLASSES_ROOT, lpszKey, REG_SZ,
		lpszValue, lstrlen(lpszValue) * sizeof(TCHAR)) != ERROR_SUCCESS)
	{
		TRACE(_T("Warning: registration database update failed for key '%s'.\n"), lpszKey);
		return false;
	}

	return true;
}

bool CDjViewApp::RegisterShellFileTypes()
{
	bool bSuccess = true;
	CString strPathName, strTemp;

	GetModuleFileName(m_hInstance, strPathName.GetBuffer(_MAX_PATH), _MAX_PATH);
	strPathName.ReleaseBuffer();

	POSITION pos = GetFirstDocTemplatePosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = GetNextDocTemplate(pos);

		CString strOpenCommandLine;
		strOpenCommandLine.Format(_T("\"%s\""), strPathName);

		CString strDefaultIconCommandLine;
		strDefaultIconCommandLine.Format(_T("\"%s\",0"), strPathName, 0);

		CString strFilterExt, strFileTypeId, strFileTypeName;
		if (pTemplate->GetDocString(strFileTypeId,
			CDocTemplate::regFileTypeId) && !strFileTypeId.IsEmpty())
		{
			// enough info to register it
			if (!pTemplate->GetDocString(strFileTypeName,
				CDocTemplate::regFileTypeName))
				strFileTypeName = strFileTypeId;    // use id name

			ASSERT(strFileTypeId.Find(' ') == -1);  // no spaces allowed

			// first register the type ID of our server
			if (!SetRegKey(strFileTypeId, strFileTypeName))
			{
				bSuccess = false;
				continue;
			}

			strTemp.Format(_T("%s\\DefaultIcon"), (LPCTSTR)strFileTypeId);
			if (!SetRegKey(strTemp, strDefaultIconCommandLine))
			{
				bSuccess = false;
				continue;
			}

			strTemp.Format(_T("%s\\shell\\open\\%s"), (LPCTSTR)strFileTypeId,
				(LPCTSTR)_T("ddeexec"));
			if (!SetRegKey(strTemp, _T("[open(\"%1\")]")))
			{
				bSuccess = false;
				continue;
			}

			strTemp.Format(_T("%s\\shell\\open\\%s\\Application"), (LPCTSTR)strFileTypeId,
				(LPCTSTR)_T("ddeexec"));
			if (!SetRegKey(strTemp, _T("WinDjView")))
			{
				bSuccess = false;
				continue;
			}

			strTemp.Format(_T("%s\\shell\\open\\%s\\Topic"), (LPCTSTR)strFileTypeId,
				(LPCTSTR)_T("ddeexec"));
			if (!SetRegKey(strTemp, _T("System")))
			{
				bSuccess = false;
				continue;
			}

			strOpenCommandLine += _T(" \"%1\"");

			// path\shell\open\command = path filename
			strTemp.Format(_T("%s\\shell\\open\\%s"), (LPCTSTR)strFileTypeId,
				_T("command"));
			if (!SetRegKey(strTemp, strOpenCommandLine))
			{
				bSuccess = false;
				continue;
			}

			CString strExtensions;
			pTemplate->GetDocString(strExtensions, CDocTemplate::filterExt);

			int nExt = 0;
			while (AfxExtractSubString(strFilterExt, strExtensions, nExt++, ';') &&
				!strFilterExt.IsEmpty())
			{
				ASSERT(strFilterExt[0] == '.');

				LONG lSize = _MAX_PATH * 2;
				LONG lResult = ::RegQueryValue(HKEY_CLASSES_ROOT, strFilterExt,
					strTemp.GetBuffer(lSize), &lSize);
				strTemp.ReleaseBuffer();

				if (lResult != ERROR_SUCCESS || strTemp.IsEmpty() ||
					strTemp != strFileTypeId)
				{
					// no association for that suffix
					if (!SetRegKey(strFilterExt, strFileTypeId))
					{
						bSuccess = false;
						continue;
					}
				}
			}
		}
	}

	return bSuccess;
}

void CDjViewApp::OnFileSettings()
{
	CSettingsDlg dlg;
	if (dlg.DoModal() == IDOK)
	{
		CAppSettings::bRestoreAssocs = !!dlg.m_pageAssocs.m_bRestoreAssocs;

		CAppSettings::bGenAllThumbnails = !!dlg.m_pageGeneral.m_bGenAllThumbnails;
		CAppSettings::bFullscreenClicks = !!dlg.m_pageGeneral.m_bFullscreenClicks;
		CAppSettings::bFullscreenHideScroll = !!dlg.m_pageGeneral.m_bFullscreenHideScroll;
		CAppSettings::bWarnCloseMultiple = !!dlg.m_pageGeneral.m_bWarnCloseMultiple;
		CAppSettings::bInvertWheelZoom = !!dlg.m_pageGeneral.m_bInvertWheelZoom;
		CAppSettings::bCloseOnEsc = !!dlg.m_pageGeneral.m_bCloseOnEsc;
		CAppSettings::bWrapLongBookmarks = !!dlg.m_pageGeneral.m_bWrapLongBookmarks;
		CAppSettings::bRestoreView = !!dlg.m_pageGeneral.m_bRestoreView;

		CAppSettings::displaySettings = dlg.m_pageDisplay.m_displaySettings;

		SaveSettings();

		// Thumbnails settings may have changed, let all thumbnail views know
		// Also let all views know about new gamma settings
		POSITION pos = GetFirstDocTemplatePosition();
		while (pos != NULL)
		{
			CDocTemplate* pTemplate = GetNextDocTemplate(pos);
			ASSERT_KINDOF(CDocTemplate, pTemplate);

			POSITION posDoc = pTemplate->GetFirstDocPosition();
			while (posDoc != NULL)
			{
				CDocument* pDoc = pTemplate->GetNextDoc(posDoc);
				CDjVuView* pView = ((CDjVuDoc*)pDoc)->GetDjVuView();
				CChildFrame* pFrame = (CChildFrame*)pView->GetParentFrame();

				pFrame->GetDjVuView()->OnSettingsChanged();
				pFrame->GetThumbnailsView()->OnSettingsChanged();

				if (pFrame->GetBookmarksWnd() != NULL)
					pFrame->GetBookmarksWnd()->OnSettingsChanged();
			}
		}
	}
}

CDjVuDoc* CDjViewApp::OpenDocument(const CString& strPathName, const GUTF8String& strPage)
{
/**/
	CDjVuDoc* pDoc = (CDjVuDoc*)FindOpenDocument(strPathName);
	if (pDoc == NULL)
		pDoc = (CDjVuDoc*)OpenDocumentFile(strPathName);

	if (pDoc == NULL)
	{
		AfxMessageBox(LoadString(IDS_FAILED_TO_OPEN) + strPathName);
		return NULL;
	}

	CDjVuView* pView = pDoc->GetDjVuView();
	CFrameWnd* pFrame = pView->GetParentFrame();
	pFrame->ActivateFrame();
/*/
	CDjVuDoc* pDoc = (CDjVuDoc*)OpenDocumentFile(strPathName);
	if (pDoc == NULL)
	{
		AfxMessageBox(LoadString(IDS_FAILED_TO_OPEN) + strPathName);
		return NULL;
	}

	CDjVuView* pView = pDoc->GetDjVuView();
/**/
	if (strPage.length() > 0)
	{
		pView->GoToURL(strPage, -1, CDjVuView::AddTarget);
	}

	return pDoc;
}

CDocument* CDjViewApp::FindOpenDocument(LPCTSTR lpszFileName)
{
	TCHAR szPath[_MAX_PATH], szTemp[_MAX_PATH];
	ASSERT(lstrlen(lpszFileName) < _countof(szPath));
	if (lpszFileName[0] == '\"')
		++lpszFileName;
	lstrcpyn(szTemp, lpszFileName, _MAX_PATH);
	LPTSTR lpszLast = _tcsrchr(szTemp, '\"');
	if (lpszLast != NULL)
		*lpszLast = 0;

	if (!AfxFullPath(szPath, szTemp))
	{
		ASSERT(FALSE);
		return NULL; // We won't open the file. MFC requires paths with length < _MAX_PATH
	}

	POSITION pos = GetFirstDocTemplatePosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = GetNextDocTemplate(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);

		CDocument* pDocument = NULL;
		if (pTemplate->MatchDocType(szPath, pDocument) == CDocTemplate::yesAlreadyOpen)
			return pDocument;
	}

	return NULL;
}

void CDjViewApp::OnCheckForUpdate()
{
	CUpdateDlg dlg;
	dlg.DoModal();
}

BOOL CDjViewApp::OnOpenRecentFile(UINT nID)
{
	// Fixed MFC's CWinApp::OnOpenRecentFile
	// Always moves the file to the top of the recents list

	ASSERT_VALID(this);
	ASSERT(m_pRecentFileList != NULL);

	ASSERT(nID >= ID_FILE_MRU_FILE1 && nID < ID_FILE_MRU_FILE1 + (UINT)m_pRecentFileList->GetSize());
	int nIndex = nID - ID_FILE_MRU_FILE1;

	CString& strPathName = (*m_pRecentFileList)[nIndex];
	ASSERT(strPathName.GetLength() != 0);
	TRACE(_T("MRU: open file (%d) '%s'.\n"), nIndex + 1, strPathName);

	CDocument* pDoc = OpenDocumentFile(strPathName);
	if (pDoc == NULL)
		m_pRecentFileList->Remove(nIndex);
	else
		AddToRecentFileList(pDoc->GetPathName());

	return true;
}

void ReportFatalError()
{
	if (!s_bFatalError)
	{
		s_bFatalError = true;
		if (AfxMessageBox(IDS_FATAL_ERROR, MB_ICONERROR | MB_OKCANCEL) == IDOK)
		{
			ExitProcess(1);
		}
	}
}

bool IsFromCurrentProcess(CWnd* pWnd)
{
	DWORD dwProcessId = 0;
	::GetWindowThreadProcessId(pWnd->GetSafeHwnd(), &dwProcessId);
	return (dwProcessId == ::GetCurrentProcessId());
}

CString FormatDouble(double fValue)
{
	char nDecimalPoint = localeconv()->decimal_point[0];

	CString strResult;
	strResult.Format(_T("%.6f"), fValue);
	while (!strResult.IsEmpty() && strResult[strResult.GetLength() - 1] == '0')
		strResult = strResult.Left(strResult.GetLength() - 1);

	if (!strResult.IsEmpty() && strResult[strResult.GetLength() - 1] == nDecimalPoint)
		strResult = strResult.Left(strResult.GetLength() - 1);

	if (strResult.IsEmpty())
		strResult = _T("0");

	return strResult;
}

void AFXAPI DDX_MyText(CDataExchange* pDX, int nIDC, double& value, double def, LPCTSTR pszSuffix)
{
	CString strText = FormatDouble(value) + pszSuffix;
	DDX_Text(pDX, nIDC, strText);

	if (pDX->m_bSaveAndValidate)
	{
		if (_stscanf(strText, _T("%lf"), &value) != 1)
			value = def;
	}
}

void AFXAPI DDX_MyText(CDataExchange* pDX, int nIDC, DWORD& value, DWORD def, LPCTSTR pszSuffix)
{
	CString strText;
	strText.Format(_T("%u%s"), value, (LPCTSTR)CString(pszSuffix));
	DDX_Text(pDX, nIDC, strText);

	if (pDX->m_bSaveAndValidate)
	{
		if (_stscanf(strText, _T("%u"), &value) != 1)
			value = def;
	}
}
