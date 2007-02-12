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
#include "XMLParser.h"

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
//const TCHAR* s_pszStartupPage = _T("page");
//const TCHAR* s_pszDisplayMode = _T("display");


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
		pMainFrame->ShowControlBar(pBar, m_appSettings.bToolbar, FALSE);

	pBar = pMainFrame->GetControlBar(ID_VIEW_STATUS_BAR);
	if (pBar)
		pMainFrame->ShowControlBar(pBar, m_appSettings.bStatusBar, FALSE);

	pMainFrame->SetWindowPos(NULL, m_appSettings.nWindowPosX, m_appSettings.nWindowPosY,
				m_appSettings.nWindowWidth, m_appSettings.nWindowHeight,
				SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();

	if (m_appSettings.bRestoreAssocs)
		RegisterShellFileTypes();

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	if (m_appSettings.bWindowMaximized && (m_nCmdShow == -1 || m_nCmdShow == SW_SHOWNORMAL))
		m_nCmdShow = SW_SHOWMAXIMIZED;

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return false;

	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->SetStartupLanguage();

	pMainFrame->UnlockWindowUpdate();
	pMainFrame->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

	m_bInitialized = true;
	CChildFrame* pChildFrame = (CChildFrame*)pMainFrame->MDIGetActive();
	if (pChildFrame != NULL)
		pChildFrame->GetDjVuView()->UpdateVisiblePages();

	if (m_appSettings.strVersion != CURRENT_VERSION)
		OnAppAbout();

	return true;
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

CAboutDlg::CAboutDlg()
	: CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_LINK, m_weblink);
	DDX_Control(pDX, IDC_STATIC_LIB_LINK, m_weblinkLibrary);
	DDX_Control(pDX, IDC_DONATE, m_btnDonate);

	CString strVersion = FormatString(IDS_VERSION_ABOUT, CURRENT_VERSION);
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
	m_appSettings.nWindowPosX = GetProfileInt(s_pszDisplaySettings, s_pszPosX, m_appSettings.nWindowPosX);
	m_appSettings.nWindowPosY = GetProfileInt(s_pszDisplaySettings, s_pszPosY, m_appSettings.nWindowPosY);
	m_appSettings.nWindowWidth = GetProfileInt(s_pszDisplaySettings, s_pszWidth, m_appSettings.nWindowWidth);
	m_appSettings.nWindowHeight = GetProfileInt(s_pszDisplaySettings, s_pszHeight, m_appSettings.nWindowHeight);
	m_appSettings.bWindowMaximized = !!GetProfileInt(s_pszDisplaySettings, s_pszMaximized, m_appSettings.bWindowMaximized);
	m_appSettings.bChildMaximized = !!GetProfileInt(s_pszDisplaySettings, s_pszChildMaximized, m_appSettings.bChildMaximized);
	m_appSettings.bToolbar = !!GetProfileInt(s_pszDisplaySettings, s_pszToolbar, m_appSettings.bToolbar);
	m_appSettings.bStatusBar = !!GetProfileInt(s_pszDisplaySettings, s_pszStatusBar, m_appSettings.bStatusBar);
	m_appSettings.nDefaultZoomType = GetProfileInt(s_pszDisplaySettings, s_pszZoom, m_appSettings.nDefaultZoomType);
	m_appSettings.fDefaultZoom = GetProfileDouble(s_pszDisplaySettings, s_pszZoomPercent, m_appSettings.fDefaultZoom);
	m_appSettings.nDefaultLayout = GetProfileInt(s_pszDisplaySettings, s_pszLayout, m_appSettings.nDefaultLayout);
	m_appSettings.bFirstPageAlone = !!GetProfileInt(s_pszDisplaySettings, s_pszFirstPage, m_appSettings.bFirstPageAlone);
	m_appSettings.nDefaultMode = GetProfileInt(s_pszDisplaySettings, s_pszMode, m_appSettings.nDefaultMode);
	m_appSettings.bNavPaneCollapsed = !!GetProfileInt(s_pszDisplaySettings, s_pszNavCollapsed, m_appSettings.bNavPaneCollapsed);
	m_appSettings.nNavPaneWidth = GetProfileInt(s_pszDisplaySettings, s_pszNavWidth, m_appSettings.nNavPaneWidth);

	m_appSettings.bRestoreAssocs = !!GetProfileInt(s_pszGlobalSettings, s_pszRestoreAssocs, m_appSettings.bRestoreAssocs);
	m_appSettings.bGenAllThumbnails = !!GetProfileInt(s_pszGlobalSettings, s_pszGenAllThumbnails, m_appSettings.bGenAllThumbnails);
	m_appSettings.bFullscreenClicks = !!GetProfileInt(s_pszGlobalSettings, s_pszFullscreenClicks, m_appSettings.bFullscreenClicks);
	m_appSettings.bFullscreenHideScroll = !!GetProfileInt(s_pszGlobalSettings, s_pszFullscreenHideScroll, m_appSettings.bFullscreenHideScroll);
	m_appSettings.bWarnCloseMultiple = !!GetProfileInt(s_pszGlobalSettings, s_pszWarnCloseMultiple, m_appSettings.bWarnCloseMultiple);
	m_appSettings.bInvertWheelZoom = !!GetProfileInt(s_pszGlobalSettings, s_pszInvertWheelZoom, m_appSettings.bInvertWheelZoom);
	m_appSettings.bCloseOnEsc = !!GetProfileInt(s_pszGlobalSettings, s_pszCloseOnEsc, m_appSettings.bCloseOnEsc);
	m_appSettings.bWrapLongBookmarks = !!GetProfileInt(s_pszGlobalSettings, s_pszWrapLongBookmarks, m_appSettings.bWrapLongBookmarks);
	m_appSettings.bRestoreView = !!GetProfileInt(s_pszGlobalSettings, s_pszRestoreView, m_appSettings.bRestoreView);
	m_appSettings.nLanguage = GetProfileInt(s_pszGlobalSettings, s_pszLanguage, m_appSettings.nLanguage);
	m_appSettings.strFind = GetProfileString(s_pszGlobalSettings, s_pszFind, m_appSettings.strFind);
	m_appSettings.bMatchCase = !!GetProfileInt(s_pszGlobalSettings, s_pszMatchCase, m_appSettings.bMatchCase);
	m_appSettings.strVersion = GetProfileString(s_pszGlobalSettings, s_pszVersion, CURRENT_VERSION);

	m_displaySettings.nScaleMethod = GetProfileInt(s_pszDisplaySettings, s_pszScaleMethod, m_displaySettings.nScaleMethod);
	m_displaySettings.bInvertColors = !!GetProfileInt(s_pszDisplaySettings, s_pszInvertColors, m_displaySettings.bInvertColors);
	m_displaySettings.bAdjustDisplay = !!GetProfileInt(s_pszDisplaySettings, s_pszAdjustDisplay, m_displaySettings.bAdjustDisplay);
	m_displaySettings.fGamma = GetProfileDouble(s_pszDisplaySettings, s_pszGamma, m_displaySettings.fGamma);
	m_displaySettings.nBrightness = GetProfileInt(s_pszDisplaySettings, s_pszBrightness, m_displaySettings.nBrightness);
	m_displaySettings.nContrast = GetProfileInt(s_pszDisplaySettings, s_pszContrast, m_displaySettings.nContrast);
	m_displaySettings.Fix();

	m_printSettings.fMarginLeft = GetProfileDouble(s_pszPrintSettings, s_pszMarginLeft, m_printSettings.fMarginLeft);
	m_printSettings.fMarginTop = GetProfileDouble(s_pszPrintSettings, s_pszMarginTop, m_printSettings.fMarginTop);
	m_printSettings.fMarginRight = GetProfileDouble(s_pszPrintSettings, s_pszMarginRight, m_printSettings.fMarginRight);
	m_printSettings.fMarginBottom = GetProfileDouble(s_pszPrintSettings, s_pszMarginBottom, m_printSettings.fMarginBottom);
	m_printSettings.fPosLeft = GetProfileDouble(s_pszPrintSettings, s_pszPosLeft, m_printSettings.fPosLeft);
	m_printSettings.fPosTop = GetProfileDouble(s_pszPrintSettings, s_pszPosTop, m_printSettings.fPosTop);
	m_printSettings.bCenterImage = !!GetProfileInt(s_pszPrintSettings, s_pszCenterImage, m_printSettings.bCenterImage);
	m_printSettings.bIgnorePrinterMargins = !!GetProfileInt(s_pszPrintSettings, s_pszIgnoreMargins, m_printSettings.bIgnorePrinterMargins);
	m_printSettings.bShrinkOversized = !!GetProfileInt(s_pszPrintSettings, s_pszShrinkOversized, m_printSettings.bShrinkOversized);
}

void CDjViewApp::LoadDocSettings(const CString& strKey, DocSettings* pSettings)
{
	LPBYTE pBuf;
	UINT nSize;

	if (GetProfileBinary(s_pszDocumentsSettings, strKey, &pBuf, &nSize))
	{
		GP<ByteStream> raw = ByteStream::create(pBuf, nSize);
		GP<ByteStream> compressed = BSByteStream::create(raw);

		GUTF8String text;
		char szTemp[1024];
		int nRead;
		while ((nRead = compressed->read(szTemp, 1024)) != 0)
			text += GUTF8String(szTemp, nRead);

		stringstream sin((const char*) text);
		XMLParser parser;
		if (parser.Parse(sin))
			pSettings->Load(*parser.GetRoot());

		delete[] pBuf;
	}
}

void CDjViewApp::SaveSettings()
{
	WriteProfileInt(s_pszDisplaySettings, s_pszPosX, m_appSettings.nWindowPosX);
	WriteProfileInt(s_pszDisplaySettings, s_pszPosY, m_appSettings.nWindowPosY);
	WriteProfileInt(s_pszDisplaySettings, s_pszWidth, m_appSettings.nWindowWidth);
	WriteProfileInt(s_pszDisplaySettings, s_pszHeight, m_appSettings.nWindowHeight);
	WriteProfileInt(s_pszDisplaySettings, s_pszMaximized, m_appSettings.bWindowMaximized);
	WriteProfileInt(s_pszDisplaySettings, s_pszChildMaximized, m_appSettings.bChildMaximized);
	WriteProfileInt(s_pszDisplaySettings, s_pszToolbar, m_appSettings.bToolbar);
	WriteProfileInt(s_pszDisplaySettings, s_pszStatusBar, m_appSettings.bStatusBar);
	WriteProfileInt(s_pszDisplaySettings, s_pszZoom, m_appSettings.nDefaultZoomType);
	WriteProfileDouble(s_pszDisplaySettings, s_pszZoomPercent, m_appSettings.fDefaultZoom);
	WriteProfileInt(s_pszDisplaySettings, s_pszLayout, m_appSettings.nDefaultLayout);
	WriteProfileInt(s_pszDisplaySettings, s_pszFirstPage, m_appSettings.bFirstPageAlone);
	WriteProfileInt(s_pszDisplaySettings, s_pszMode, m_appSettings.nDefaultMode);
	WriteProfileInt(s_pszDisplaySettings, s_pszNavCollapsed, m_appSettings.bNavPaneCollapsed);
	WriteProfileInt(s_pszDisplaySettings, s_pszNavWidth, m_appSettings.nNavPaneWidth);

	WriteProfileInt(s_pszGlobalSettings, s_pszRestoreAssocs, m_appSettings.bRestoreAssocs);
	WriteProfileInt(s_pszGlobalSettings, s_pszGenAllThumbnails, m_appSettings.bGenAllThumbnails);
	WriteProfileInt(s_pszGlobalSettings, s_pszFullscreenClicks, m_appSettings.bFullscreenClicks);
	WriteProfileInt(s_pszGlobalSettings, s_pszFullscreenHideScroll, m_appSettings.bFullscreenHideScroll);
	WriteProfileInt(s_pszGlobalSettings, s_pszWarnCloseMultiple, m_appSettings.bWarnCloseMultiple);
	WriteProfileInt(s_pszGlobalSettings, s_pszInvertWheelZoom, m_appSettings.bInvertWheelZoom);
	WriteProfileInt(s_pszGlobalSettings, s_pszCloseOnEsc, m_appSettings.bCloseOnEsc);
	WriteProfileInt(s_pszGlobalSettings, s_pszWrapLongBookmarks, m_appSettings.bWrapLongBookmarks);
	WriteProfileInt(s_pszGlobalSettings, s_pszRestoreView, m_appSettings.bRestoreView);
	WriteProfileString(s_pszGlobalSettings, s_pszVersion, CURRENT_VERSION);
	WriteProfileInt(s_pszGlobalSettings, s_pszLanguage, m_appSettings.nLanguage);
	WriteProfileString(s_pszGlobalSettings, s_pszFind, m_appSettings.strFind);
	WriteProfileInt(s_pszGlobalSettings, s_pszMatchCase, m_appSettings.bMatchCase);

	WriteProfileInt(s_pszDisplaySettings, s_pszScaleMethod, m_displaySettings.nScaleMethod);
	WriteProfileInt(s_pszDisplaySettings, s_pszInvertColors, m_displaySettings.bInvertColors);
	WriteProfileInt(s_pszDisplaySettings, s_pszAdjustDisplay, m_displaySettings.bAdjustDisplay);
	WriteProfileDouble(s_pszDisplaySettings, s_pszGamma, m_displaySettings.fGamma);
	WriteProfileInt(s_pszDisplaySettings, s_pszBrightness, m_displaySettings.nBrightness);
	WriteProfileInt(s_pszDisplaySettings, s_pszContrast, m_displaySettings.nContrast);

	WriteProfileDouble(s_pszPrintSettings, s_pszMarginLeft, m_printSettings.fMarginLeft);
	WriteProfileDouble(s_pszPrintSettings, s_pszMarginTop, m_printSettings.fMarginTop);
	WriteProfileDouble(s_pszPrintSettings, s_pszMarginRight, m_printSettings.fMarginRight);
	WriteProfileDouble(s_pszPrintSettings, s_pszMarginBottom, m_printSettings.fMarginBottom);
	WriteProfileDouble(s_pszPrintSettings, s_pszPosLeft, m_printSettings.fPosLeft);
	WriteProfileDouble(s_pszPrintSettings, s_pszPosTop, m_printSettings.fPosTop);
	WriteProfileInt(s_pszPrintSettings, s_pszCenterImage, m_printSettings.bCenterImage);
	WriteProfileInt(s_pszPrintSettings, s_pszIgnoreMargins, m_printSettings.bIgnorePrinterMargins);
	WriteProfileInt(s_pszPrintSettings, s_pszShrinkOversized, m_printSettings.bShrinkOversized);

	const map<MD5, DocSettings>& settings = DjVuSource::GetAllSettings();
	for (map<MD5, DocSettings>::const_iterator it = settings.begin(); it != settings.end(); ++it)
	{
		CString strDocumentKey = (*it).first.ToString();

		const DocSettings& settings = (*it).second;
		GUTF8String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + settings.GetXML();

		GP<ByteStream> raw = ByteStream::create();

		GP<ByteStream> compressed = BSByteStream::create(raw, 128);
		compressed->writall((const char*) xml, xml.length());
		compressed = NULL;

		UINT nSize = raw->size();
		if (nSize > 0)
		{
			LPBYTE pBuf = new BYTE[nSize];
			raw->readat(pBuf, nSize, 0);

			WriteProfileBinary(s_pszDocumentsSettings, strDocumentKey, pBuf, nSize);
			delete[] pBuf;
		}
/*
		OutputDebugString(data.GetXML());
		stringstream sin((const char*) data.GetXML());
		XMLParser parser;
		if (!parser.Parse(sin))
		{
			OutputDebugString("Error!");
		}
		else
		{
			DocSettings newData;
			newData.Load(*parser.GetRoot());
			if (data.GetXML() != newData.GetXML())
			{
				OutputDebugString("Strings differ!\n");
				OutputDebugString(newData.GetXML());
			}
			else
				OutputDebugString("OK!\n");
		}
*/
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
		m_appSettings.bRestoreAssocs = !!dlg.m_pageAssocs.m_bRestoreAssocs;

		m_appSettings.bGenAllThumbnails = !!dlg.m_pageGeneral.m_bGenAllThumbnails;
		m_appSettings.bFullscreenClicks = !!dlg.m_pageGeneral.m_bFullscreenClicks;
		m_appSettings.bFullscreenHideScroll = !!dlg.m_pageGeneral.m_bFullscreenHideScroll;
		m_appSettings.bWarnCloseMultiple = !!dlg.m_pageGeneral.m_bWarnCloseMultiple;
		m_appSettings.bInvertWheelZoom = !!dlg.m_pageGeneral.m_bInvertWheelZoom;
		m_appSettings.bCloseOnEsc = !!dlg.m_pageGeneral.m_bCloseOnEsc;
		m_appSettings.bWrapLongBookmarks = !!dlg.m_pageGeneral.m_bWrapLongBookmarks;
		m_appSettings.bRestoreView = !!dlg.m_pageGeneral.m_bRestoreView;

		m_displaySettings = dlg.m_pageDisplay.m_displaySettings;

		SaveSettings();

		UpdateObservers(AppSettingsChanged());
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

void CDjViewApp::SetLanguage(HINSTANCE hResources, DWORD nLanguage)
{
	m_appSettings.bLocalized = (hResources != AfxGetInstanceHandle());
	m_appSettings.nLanguage = nLanguage;

	AfxSetResourceHandle(hResources);

	if (m_appSettings.hDjVuMenu != NULL)
	{
		::DestroyMenu(m_appSettings.hDjVuMenu);
		::DestroyMenu(m_appSettings.hDefaultMenu);
		m_appSettings.hDjVuMenu = NULL;
		m_appSettings.hDefaultMenu = NULL;
	}

	if (m_appSettings.bLocalized)
	{
		CMenu menuDjVu, menuDefault;
		menuDjVu.LoadMenu(IDR_DjVuTYPE);
		menuDefault.LoadMenu(IDR_MAINFRAME);

		m_appSettings.hDjVuMenu = menuDjVu.Detach();
		m_appSettings.hDefaultMenu = menuDefault.Detach();
	}

	m_pDjVuTemplate->UpdateTemplate();

	UpdateObservers(AppLanguageChanged());
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

UINT GetMouseScrollLines()
{
	static UINT uCachedScrollLines;
	static bool bGotScrollLines = false;

	// If we've already got it and we're not refreshing,
	// return what we've already got

	if (bGotScrollLines)
		return uCachedScrollLines;

	// see if we can find the mouse window

	bGotScrollLines = true;

	static UINT msgGetScrollLines;
	static WORD nRegisteredMessage;

	if (afxData.bWin95)
	{
		if (nRegisteredMessage == 0)
		{
			msgGetScrollLines = ::RegisterWindowMessage(MSH_SCROLL_LINES);
			if (msgGetScrollLines == 0)
				nRegisteredMessage = 1;     // couldn't register!  never try again
			else
				nRegisteredMessage = 2;     // it worked: use it
		}

		if (nRegisteredMessage == 2)
		{
			HWND hwMouseWheel = NULL;
			hwMouseWheel = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);
			if (hwMouseWheel && msgGetScrollLines)
			{
				uCachedScrollLines = (UINT)::SendMessage(hwMouseWheel, msgGetScrollLines, 0, 0);
			}
		}
	}

	if (uCachedScrollLines == 0)
	{
		::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uCachedScrollLines, false);
		if (uCachedScrollLines == 0)
			uCachedScrollLines = 3; // reasonable default
	}

	return uCachedScrollLines;
}

CString FormatDouble(double fValue)
{
	char nDecimalPoint = localeconv()->decimal_point[0];

	CString strResult = FormatString(_T("%.6f"), fValue);
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
	CString strText = FormatString(_T("%u%s"), value, (LPCTSTR)CString(pszSuffix));
	DDX_Text(pDX, nIDC, strText);

	if (pDX->m_bSaveAndValidate)
	{
		if (_stscanf(strText, _T("%u"), &value) != 1)
			value = def;
	}
}

struct MonitorAPI
{
	MonitorAPI();
	~MonitorAPI();

	struct MonitorInfo
	{
		MonitorInfo() : cbSize(sizeof(MonitorInfo)) {}

		DWORD cbSize;
		RECT rcMonitor;
		RECT rcWork;
		DWORD dwFlags;
	};

	enum
	{
		DefaultToNull = 0,
		DefaultToPrimary = 1,
		DefaultToNearest = 2
	};

	typedef HANDLE (WINAPI* pfnMonitorFromPoint)(POINT pt, DWORD dwFlags);
	typedef BOOL (WINAPI* pfnGetMonitorInfo)(HANDLE hMonitor, MonitorInfo* pmi);
	typedef HANDLE (WINAPI* pfnMonitorFromWindow)(HWND hWnd, DWORD dwFlags);

	pfnMonitorFromPoint pMonitorFromPoint;
	pfnMonitorFromWindow pMonitorFromWindow;
	pfnGetMonitorInfo pGetMonitorInfo;

	bool IsLoaded() const { return hUser32 != NULL; }

	HINSTANCE hUser32;
};

static MonitorAPI theMonitorAPI;

MonitorAPI::MonitorAPI()
	: pMonitorFromPoint(NULL),
	  pMonitorFromWindow(NULL),
	  pGetMonitorInfo(NULL)
{
	hUser32 = ::LoadLibrary(_T("user32.dll"));
	if (hUser32 != NULL)
	{
		pMonitorFromPoint = (pfnMonitorFromPoint) ::GetProcAddress(hUser32, "MonitorFromPoint");
		pMonitorFromWindow = (pfnMonitorFromWindow) ::GetProcAddress(hUser32, "MonitorFromWindow");
		pGetMonitorInfo = (pfnGetMonitorInfo) ::GetProcAddress(hUser32, "GetMonitorInfoA");

		if (pMonitorFromPoint == NULL
				|| pMonitorFromWindow == NULL
				|| pGetMonitorInfo == NULL)
		{
			::FreeLibrary(hUser32);
			hUser32 = NULL;
		}
	}
}

MonitorAPI::~MonitorAPI()
{
	if (hUser32 != NULL)
		::FreeLibrary(hUser32);
}

CRect GetMonitorWorkArea(const CPoint& point)
{
	CRect rcWorkArea;

	if (!::SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rcWorkArea, false))
	{
		CSize szScreen(::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
		rcWorkArea = CRect(CPoint(0, 0), szScreen);
	}

	if (theMonitorAPI.IsLoaded())
	{
		MonitorAPI::MonitorInfo mi;
		HANDLE hMonitor = theMonitorAPI.pMonitorFromPoint(point, MonitorAPI::DefaultToNearest);
		if (hMonitor != NULL && theMonitorAPI.pGetMonitorInfo(hMonitor, &mi))
			rcWorkArea = mi.rcWork;
	}

	return rcWorkArea;
}

CRect GetMonitorWorkArea(CWnd* pWnd)
{
	CRect rcWorkArea;

	if (!::SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rcWorkArea, false))
	{
		CSize szScreen(::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
		rcWorkArea = CRect(CPoint(0, 0), szScreen);
	}

	if (theMonitorAPI.IsLoaded())
	{
		MonitorAPI::MonitorInfo mi;
		HANDLE hMonitor = theMonitorAPI.pMonitorFromWindow(pWnd->GetSafeHwnd(), MonitorAPI::DefaultToNearest);
		if (hMonitor != NULL && theMonitorAPI.pGetMonitorInfo(hMonitor, &mi))
			rcWorkArea = mi.rcWork;
	}

	return rcWorkArea;
}

CRect GetMonitorRect(CWnd* pWnd)
{
	CSize szScreen(::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
	CRect rcMonitor(CPoint(0, 0), szScreen);

	if (theMonitorAPI.IsLoaded())
	{
		MonitorAPI::MonitorInfo mi;
		HANDLE hMonitor = theMonitorAPI.pMonitorFromWindow(pWnd->GetSafeHwnd(), MonitorAPI::DefaultToNearest);
		if (hMonitor != NULL && theMonitorAPI.pGetMonitorInfo(hMonitor, &mi))
			rcMonitor = mi.rcMonitor;
	}

	return rcMonitor;
}
