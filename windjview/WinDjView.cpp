p//	WinDjView
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
//	51 Franklin Street, Fifth Floor, Boston, MA 02111-1307 USA.
//	http://www.gnu.org/copyleft/gpl.html

#include "stdafx.h"
#include "WinDjView.h"

#include "MainFrm.h"
#include "MyBitmapButton.h"
#include "DjVuDoc.h"
#include "DjVuView.h"
#include "MDIChild.h"
#include "MyDocManager.h"
#include "MyDocTemplate.h"
#include "SettingsDlg.h"
#include "UpdateDlg.h"
#include "ThumbnailsView.h"
#include "BookmarksView.h"
#include "PageIndexWnd.h"
#include "NavPane.h"
#include "FullscreenWnd.h"
#include "XMLParser.h"
#include "MyDialog.h"
#include "FindDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString CURRENT_VERSION;
BOOL s_bFatalError = false;


const TCHAR* s_pszDisplaySection = _T("Display");
const TCHAR* s_pszPosX = _T("x");
const TCHAR* s_pszPosY = _T("y");
const TCHAR* s_pszWidth = _T("width");
const TCHAR* s_pszHeight = _T("height");
const TCHAR* s_pszMaximized = _T("maximized");
const TCHAR* s_pszChildMaximized = _T("child-maximized");
const TCHAR* s_pszToolbar = _T("toobar");
const TCHAR* s_pszTabBar = _T("tabbar");
const TCHAR* s_pszStatusBar = _T("statusbar");
const TCHAR* s_pszDictBar = _T("dictbar");
const TCHAR* s_pszZoom = _T("zoom");
const TCHAR* s_pszZoomPercent = _T("%");
const TCHAR* s_pszLayout = _T("layout");
const TCHAR* s_pszFirstPage = _T("first-page");
const TCHAR* s_pszMode = _T("mode");
const TCHAR* s_pszNavHidden = _T("nav-hidden");
const TCHAR* s_pszNavCollapsed = _T("nav-collapsed");
const TCHAR* s_pszNavWidth = _T("nav-width");
const TCHAR* s_pszAdjustDisplay = _T("adjust");
const TCHAR* s_pszGamma = _T("gamma");
const TCHAR* s_pszBrightness = _T("brightness");
const TCHAR* s_pszContrast = _T("contrast");
const TCHAR* s_pszScaleMethod = _T("hq-render-color");
const TCHAR* s_pszScaleSubpix = _T("subpix-scale");
const TCHAR* s_pszInvertColors = _T("invert");
const TCHAR* s_pszUnits = _T("units");
const TCHAR* s_pszThumbnailSize = _T("thumbnail-size");

const TCHAR* s_pszGlobalSection = _T("Settings");
const TCHAR* s_pszWarnNotDefaultViewer = _T("warn-not-default-viewer");
const TCHAR* s_pszTopLevelDocs = _T("top-level-docs");
const TCHAR* s_pszHideSingleTab = _T("hide-single-tab");
const TCHAR* s_pszGenAllThumbnails = _T("gen-all-thumbs");
const TCHAR* s_pszFullscreenClicks = _T("fullscreen-clicks");
const TCHAR* s_pszFullscreenHideScroll = _T("fullscreen-hide-scroll");
const TCHAR* s_pszFullscreenContinuousScroll = _T("fullscreen-continuous");
const TCHAR* s_pszInvertWheelZoom = _T("invert-wheel-zoom");
const TCHAR* s_pszCloseOnEsc = _T("close-on-esc");
const TCHAR* s_pszWrapLongBookmarks = _T("wrap-long-bookmarks");
const TCHAR* s_pszRestoreView = _T("restore-view");
const TCHAR* s_pszVersion = _T("version");
const TCHAR* s_pszLanguage = _T("language");
const TCHAR* s_pszMatchCase = _T("match-case");
const TCHAR* s_pszCurrentLang = _T("cur-lang");
const TCHAR* s_pszCurrentDict = _T("cur-dict");
const TCHAR* s_pszDictLocation = _T("dict-location");
const TCHAR* s_pszDictChoice = _T("dict-choice");
const TCHAR* s_pszCheckUpdates = _T("check-updates");
const TCHAR* s_pszLastUpdateLow = _T("last-update-l");
const TCHAR* s_pszLastUpdateHigh = _T("last-update-h");
const TCHAR* s_pszNoAboutOnStartup = _T("no-about-on-startup");

const TCHAR* s_pszTabsSection = _T("Tabs");
const TCHAR* s_pszRestoreTabs = _T("restore-tabs");
const TCHAR* s_pszStartupTab = _T("startup-tab");
const TCHAR* s_pszTabCount = _T("tab-count");
const TCHAR* s_pszTabPrefix = _T("tab");

const TCHAR* s_pszAnnotationsSection = _T("Annotations");
const TCHAR* s_pszHideInactiveBorder = _T("hide-inactive");
const TCHAR* s_pszBorderType = _T("border-type");
const TCHAR* s_pszBorderColor = _T("border-color");
const TCHAR* s_pszFillType = _T("fill-type");
const TCHAR* s_pszFillColor = _T("fill-color");
const TCHAR* s_pszTransparency = _T("transparency");

const TCHAR* s_pszSearchSection = _T("Search History");
const TCHAR* s_pszFindStringPrefix = _T("string");

const TCHAR* s_pszPrintSection = _T("Print");
const TCHAR* s_pszMarginLeft = _T("m-left");
const TCHAR* s_pszMarginTop = _T("m-top");
const TCHAR* s_pszMarginRight = _T("m-right");
const TCHAR* s_pszMarginBottom = _T("m-bottom");
const TCHAR* s_pszPosLeft = _T("left");
const TCHAR* s_pszPosTop = _T("top");
const TCHAR* s_pszCenterImage = _T("center");
const TCHAR* s_pszAutoRotate = _T("auto-rotate");
const TCHAR* s_pszClipContent = _T("clip-content");
const TCHAR* s_pszScaleToFit = _T("fit");
const TCHAR* s_pszShrinkOversized = _T("shrink");
const TCHAR* s_pszIgnoreMargins = _T("no-margins");
const TCHAR* s_pszAdjustPrinting = _T("adjust-printing");

const TCHAR* s_pszDocumentsSection = _T("Documents");
const TCHAR* s_pszSettings = _T("settings");
const TCHAR* s_pszLastKnownLocation = _T("last-known-location");

const TCHAR* s_pszDictionariesSection = _T("Dictionaries");
const TCHAR* s_pszFileTimeLow = _T("modified-l");
const TCHAR* s_pszFileTimeHigh = _T("modified-h");
const TCHAR* s_pszPageIndex = _T("page-index");
const TCHAR* s_pszTitle = _T("title");
const TCHAR* s_pszLangFrom = _T("lang-from");
const TCHAR* s_pszLangTo = _T("lang-to");


// CDjViewApp

BEGIN_MESSAGE_MAP(CDjViewApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	ON_COMMAND(ID_FILE_SETTINGS, OnFileSettings)
	ON_COMMAND(ID_CHECK_FOR_UPDATE, OnCheckForUpdate)
	ON_COMMAND_EX_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnOpenRecentFile)
	ON_COMMAND_RANGE(ID_LANGUAGE_FIRST + 1, ID_LANGUAGE_LAST, OnSetLanguage)
	ON_UPDATE_COMMAND_UI(ID_LANGUAGE_FIRST, OnUpdateLanguageList)
	ON_UPDATE_COMMAND_UI_RANGE(ID_LANGUAGE_FIRST + 1, ID_LANGUAGE_LAST, OnUpdateLanguage)
END_MESSAGE_MAP()


// CDjViewApp construction

CDjViewApp::CDjViewApp()
	: m_bInitialized(false), m_bClosing(false), m_bTopLevelDocs(false),
	  m_bNoAboutOnStartup(false),
	  m_pDjVuTemplate(NULL), m_pFindDlg(NULL),
	  m_nThreadCount(0), m_hHook(NULL), m_pPendingSource(NULL), m_bShiftPressed(false),
	  m_bControlPressed(false), m_nLangIndex(0), m_nTimerID(0),
	  m_bOnlyRegisterTypes(false), m_nExitCode(0), m_hUpdateThread(NULL)
{
	DjVuSource::SetApplication(this);
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

	m_pDocManager = new CMyDocManager();

	// Register the application's document template
	m_pDjVuTemplate = new CMyDocTemplate(IDR_DjVuTYPE,
		RUNTIME_CLASS(CDjVuDoc),
		RUNTIME_CLASS(CMDIChild),
		RUNTIME_CLASS(CDjVuView));
	if (!m_pDjVuTemplate)
		return FALSE;
	AddDocTemplate(m_pDjVuTemplate);

	// Check if we need to register file type associations and exit
	for (int i = 1; i < __argc; i++)
	{
		LPCTSTR pszParam = __targv[i];
		if (pszParam[0] == '-' || pszParam[0] == '/')
		{
			++pszParam;
			if (_tcsicmp(pszParam, _T("RegisterFileTypes")) == 0)
			{
				m_bOnlyRegisterTypes = true;
				if (!RegisterShellFileTypes())
					m_nExitCode = 1;
				return false;
			}
		}
	}

	CURRENT_VERSION.LoadString(IDS_CURRENT_VERSION);

	LoadStdProfileSettings(10);  // Load recently open documents
	LoadSettings();
	LoadDictionaries();
	LoadLanguages();
	SetStartupLanguage();

	m_bTopLevelDocs = m_appSettings.bTopLevelDocs;

	// Enable DDE Execute open
	EnableShellOpen();

	// Create main MDI Frame window
	CMainFrame* pMainFrame = CreateMainFrame(true, m_nCmdShow);
	if (pMainFrame == NULL)
		return false;

	m_hHook = ::SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, NULL, ::GetCurrentThreadId());
	m_nTimerID = ::SetTimer(NULL, 1, 100, TimerProc);

	if (m_appSettings.bWarnNotDefaultViewer && RegisterShellFileTypes(true))
	{
		MessageBoxOptions mbo;
		mbo.strCheckBox = LoadString(IDS_WARN_NOT_DEFAULT_VIEWER);
		mbo.pCheckValue = &m_appSettings.bWarnNotDefaultViewer;
		if (DoMessageBox(LoadString(IDS_PROMPT_MAKE_DEFAULT_VIEWER),
						 MB_ICONEXCLAMATION | MB_YESNO, 0, mbo) == IDYES
				&& !RegisterShellFileTypesElevate(pMainFrame))
		{
			AfxMessageBox(IDS_MAKE_DEFAULT_FAILED, MB_ICONERROR | MB_OK);
		}
	}

	__int64 cur_time = time(NULL);
	if (m_appSettings.bCheckUpdates
			&& cur_time - m_appSettings.nLastUpdateTime > 60*60*24*7)  // one week
	{
		UINT nThreadId;
		m_hUpdateThread = (HANDLE)_beginthreadex(NULL, 0, CheckUpdateThreadProc, NULL, 0, &nThreadId);
		ThreadStarted();
	}

	if (m_appSettings.strVersion != CURRENT_VERSION && !m_bNoAboutOnStartup)
		OnAppAbout();

	ThreadStarted();
	return true;
}

CMainFrame* CDjViewApp::CreateMainFrame(bool bAppStartup, int nCmdShow)
{
	bool bInitialized = m_bInitialized;
	m_bInitialized = false;

	// Create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW))
	{
		m_bInitialized = bInitialized;
		delete pMainFrame;
		return NULL;
	}

	m_frames.push_front(pMainFrame);

	m_mainWndLock.Lock();
	CWnd* pPrevMainWnd = m_pMainWnd;
	m_mainWndLock.Unlock();

	ChangeMainWnd(pMainFrame);

	pMainFrame->UpdateToolbars();

	pMainFrame->SetWindowPos(NULL, m_appSettings.nWindowPosX, m_appSettings.nWindowPosY,
			m_appSettings.nWindowWidth, m_appSettings.nWindowHeight,
			SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);

	if (pPrevMainWnd != NULL)
	{
		// Cascade the new window
		CRect rcMain;
		pMainFrame->GetWindowRect(rcMain);
		CRect rcMonitor = GetMonitorWorkArea(pMainFrame);
		if (rcMain.right + 20 <= rcMonitor.right && rcMain.bottom + 20 <= rcMonitor.bottom)
			rcMain += CPoint(20, 20);
		else
			rcMain += rcMonitor.TopLeft() - rcMain.TopLeft();
		pMainFrame->SetWindowPos(NULL, rcMain.left, rcMain.top, rcMain.Width(), rcMain.Height(),
				SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
	}

	// After resolution is changed, or a monitor is unplugged,
	// the window might appear completely offscreen. If this
	// happens, center window at the current monitor.
	CRect rcWorkArea = GetMonitorWorkArea(pMainFrame);
	CRect rcWindow, rcIntersect;
	pMainFrame->GetWindowRect(rcWindow);
	if (!rcIntersect.IntersectRect(rcWindow, rcWorkArea))
	{
		CPoint ptOffset(
				max(0, (rcWorkArea.Width() - rcWindow.Width()) / 2),
				max(0, (rcWorkArea.Height() - rcWindow.Height()) / 2));
		ptOffset += rcWorkArea.TopLeft();
		pMainFrame->SetWindowPos(NULL, ptOffset.x, ptOffset.y, 0, 0,
				SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
	}

	// Enable drag/drop open
	pMainFrame->DragAcceptFiles();

	// Parse command line for standard shell commands, DDE, file open
	if (bAppStartup)
	{
		if (!m_appSettings.bTopLevelDocs && m_appSettings.bRestoreTabs)
			pMainFrame->RestoreOpenTabs();

		CCommandLineInfo cmdInfo;
		ParseCommandLine(cmdInfo);
		if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
			cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

		// Dispatch commands specified on the command line.  Will return false if
		// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
		if (!ProcessShellCommand(cmdInfo))
		{
			m_bInitialized = bInitialized;
			delete pMainFrame;
			return NULL;
		}

		// The main window has been initialized, so show and update it
		if (!m_appSettings.bTopLevelDocs && m_appSettings.bRestoreTabs)
			pMainFrame->LoadActiveTab();

		if (m_appSettings.bWindowMaximized && (nCmdShow == -1 || nCmdShow == SW_SHOWNORMAL || nCmdShow == SW_SHOW))
			nCmdShow = SW_SHOWMAXIMIZED;

		pMainFrame->ShowWindow(nCmdShow);
		pMainFrame->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}

	m_bInitialized = true;
	return pMainFrame;
}

void CDjViewApp::RemoveMainFrame(CMainFrame* pMainFrame)
{
	ASSERT(m_frames.size() > 1);
	list<CMainFrame*>::iterator it = find(m_frames.begin(), m_frames.end(), pMainFrame);
	if (it != m_frames.end())
		m_frames.erase(it);

	m_mainWndLock.Lock();
	CWnd* pMainWnd = m_pMainWnd;
	m_mainWndLock.Unlock();
	if (pMainWnd == pMainFrame)
		ChangeMainWnd(m_frames.front(), true);
}

void CDjViewApp::ChangeMainWnd(CMainFrame* pMainFrame, bool bActivate)
{
	m_mainWndLock.Lock();
	CWnd* pMainWnd = m_pMainWnd;
	m_mainWndLock.Unlock();
	if (pMainWnd == pMainFrame)
		return;

	list<CMainFrame*>::iterator it = find(m_frames.begin(), m_frames.end(), pMainFrame);
	if (it != m_frames.end())
	{
		if (it != m_frames.begin())
		{
			list<CMainFrame*>::iterator it2 = it;
			m_frames.splice(m_frames.begin(), m_frames, it, ++it2);
		}

		m_mainWndLock.Lock();
		m_pMainWnd = pMainFrame;
		m_mainWndLock.Unlock();

		if (bActivate)
			pMainFrame->ActivateDocument(pMainFrame->GetActiveDocument());

		UpdateFindDlg();
	}
	else if (pMainFrame == NULL)
	{
		m_mainWndLock.Lock();
		m_pMainWnd = NULL;
		m_mainWndLock.Unlock();
	}
}

void CDjViewApp::EnableShellOpen()
{
	ASSERT(m_atomApp == NULL && m_atomSystemTopic == NULL); // do once

	m_atomApp = ::GlobalAddAtom(_T("WinDjView"));
	m_atomSystemTopic = ::GlobalAddAtom(_T("System"));
}

void CDjViewApp::ThreadStarted()
{
	InterlockedIncrement(&m_nThreadCount);
}

void CDjViewApp::ThreadTerminated()
{
	if (InterlockedDecrement(&m_nThreadCount) <= 0)
		m_terminated.SetEvent();
}

CMyDocTemplate* CDjViewApp::GetDocumentTemplate()
{
	return m_pDjVuTemplate;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CMyDialog
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
	: CMyDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_LINK, m_weblink);
	DDX_Control(pDX, IDC_STATIC_LIB_LINK, m_weblinkLibrary);
	DDX_Control(pDX, IDC_DONATE, m_btnDonate);

	CString strVersion = FormatString(IDS_VERSION_ABOUT, CURRENT_VERSION);
	DDX_Text(pDX, IDC_STATIC_VERSION, strVersion);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CMyDialog)
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
	HBRUSH brush = CMyDialog::OnCtlColor(pDC, pWnd, nCtlColor);

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

	return CMyDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CAboutDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rcWeblink;
	m_weblink.GetWindowRect(rcWeblink);
	ScreenToClient(rcWeblink);

	if (rcWeblink.PtInRect(point))
	{
		::ShellExecute(NULL, _T("open"), LoadString(IDS_WEBSITE_URL),
				NULL, NULL, SW_SHOW);
		return;
	}

	CRect rcWeblinkLibrary;
	m_weblinkLibrary.GetWindowRect(rcWeblinkLibrary);
	ScreenToClient(rcWeblinkLibrary);

	if (rcWeblinkLibrary.PtInRect(point))
	{
		::ShellExecute(NULL, _T("open"), LoadString(IDS_DJVULIBRE_URL),
				NULL, NULL, SW_SHOW);
		return;
	}

	CMyDialog::OnLButtonDown(nFlags, point);
}

void CAboutDlg::OnDonate()
{
	::ShellExecute(NULL, _T("open"), LoadString(IDS_DONATE_URL),
			NULL, NULL, SW_SHOW);
}

BOOL CAboutDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();

	CFont fnt;
	CreateSystemDialogFont(fnt);
	LOGFONT lf;
	fnt.GetLogFont(&lf);

	lf.lfUnderline = true;
	m_font.CreateFontIndirect(&lf);

	m_weblink.SetFont(&m_font);
	m_weblinkLibrary.SetFont(&m_font);

	m_btnDonate.LoadBitmaps(IDB_DONATE);

	GetDlgItem(IDC_STATIC_LICENSE)->SetWindowText(LoadString(IDS_ABOUT_LICENSE));

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
	m_appSettings.nWindowPosX = GetProfileInt(s_pszDisplaySection, s_pszPosX, m_appSettings.nWindowPosX);
	m_appSettings.nWindowPosY = GetProfileInt(s_pszDisplaySection, s_pszPosY, m_appSettings.nWindowPosY);
	m_appSettings.nWindowWidth = GetProfileInt(s_pszDisplaySection, s_pszWidth, m_appSettings.nWindowWidth);
	m_appSettings.nWindowHeight = GetProfileInt(s_pszDisplaySection, s_pszHeight, m_appSettings.nWindowHeight);
	m_appSettings.bWindowMaximized = !!GetProfileInt(s_pszDisplaySection, s_pszMaximized, m_appSettings.bWindowMaximized);
	m_appSettings.bToolbar = !!GetProfileInt(s_pszDisplaySection, s_pszToolbar, m_appSettings.bToolbar);
	m_appSettings.bTabBar = !!GetProfileInt(s_pszDisplaySection, s_pszTabBar, m_appSettings.bTabBar);
	m_appSettings.bStatusBar = !!GetProfileInt(s_pszDisplaySection, s_pszStatusBar, m_appSettings.bStatusBar);
	m_appSettings.bDictBar = !!GetProfileInt(s_pszDisplaySection, s_pszDictBar, m_appSettings.bDictBar);
	m_appSettings.nDefaultZoomType = GetProfileInt(s_pszDisplaySection, s_pszZoom, m_appSettings.nDefaultZoomType);
	m_appSettings.fDefaultZoom = GetProfileDouble(s_pszDisplaySection, s_pszZoomPercent, m_appSettings.fDefaultZoom);
	m_appSettings.nDefaultLayout = GetProfileInt(s_pszDisplaySection, s_pszLayout, m_appSettings.nDefaultLayout);
	m_appSettings.bFirstPageAlone = !!GetProfileInt(s_pszDisplaySection, s_pszFirstPage, m_appSettings.bFirstPageAlone);
	m_appSettings.nDefaultMode = GetProfileInt(s_pszDisplaySection, s_pszMode, m_appSettings.nDefaultMode);
	m_appSettings.bNavPaneHidden = !!GetProfileInt(s_pszDisplaySection, s_pszNavHidden, m_appSettings.bNavPaneHidden);
	m_appSettings.bNavPaneCollapsed = !!GetProfileInt(s_pszDisplaySection, s_pszNavCollapsed, m_appSettings.bNavPaneCollapsed);
	m_appSettings.nNavPaneWidth = GetProfileInt(s_pszDisplaySection, s_pszNavWidth, m_appSettings.nNavPaneWidth);

	m_appSettings.bWarnNotDefaultViewer = !!GetProfileInt(s_pszGlobalSection, s_pszWarnNotDefaultViewer, m_appSettings.bWarnNotDefaultViewer);
	m_appSettings.bTopLevelDocs = !!GetProfileInt(s_pszGlobalSection, s_pszTopLevelDocs, m_appSettings.bTopLevelDocs);
	m_appSettings.bHideSingleTab = !!GetProfileInt(s_pszGlobalSection, s_pszHideSingleTab, m_appSettings.bHideSingleTab);
	m_appSettings.bGenAllThumbnails = !!GetProfileInt(s_pszGlobalSection, s_pszGenAllThumbnails, m_appSettings.bGenAllThumbnails);
	m_appSettings.bFullscreenClicks = !!GetProfileInt(s_pszGlobalSection, s_pszFullscreenClicks, m_appSettings.bFullscreenClicks);
	m_appSettings.bFullscreenHideScroll = !!GetProfileInt(s_pszGlobalSection, s_pszFullscreenHideScroll, m_appSettings.bFullscreenHideScroll);
	m_appSettings.bFullscreenContinuousScroll = !!GetProfileInt(s_pszGlobalSection, s_pszFullscreenContinuousScroll, m_appSettings.bFullscreenContinuousScroll);
	m_appSettings.bInvertWheelZoom = !!GetProfileInt(s_pszGlobalSection, s_pszInvertWheelZoom, m_appSettings.bInvertWheelZoom);
	m_appSettings.bCloseOnEsc = !!GetProfileInt(s_pszGlobalSection, s_pszCloseOnEsc, m_appSettings.bCloseOnEsc);
	m_appSettings.bWrapLongBookmarks = !!GetProfileInt(s_pszGlobalSection, s_pszWrapLongBookmarks, m_appSettings.bWrapLongBookmarks);
	m_appSettings.bRestoreView = !!GetProfileInt(s_pszGlobalSection, s_pszRestoreView, m_appSettings.bRestoreView);
	m_appSettings.nLanguage = GetProfileInt(s_pszGlobalSection, s_pszLanguage, m_appSettings.nLanguage);
	m_appSettings.bMatchCase = !!GetProfileInt(s_pszGlobalSection, s_pszMatchCase, m_appSettings.bMatchCase);
	m_appSettings.strVersion = GetProfileString(s_pszGlobalSection, s_pszVersion, _T(""));
	m_appSettings.nCurLang = GetProfileInt(s_pszGlobalSection, s_pszCurrentLang, m_appSettings.nCurLang);
	m_appSettings.nCurDict = GetProfileInt(s_pszGlobalSection, s_pszCurrentDict, m_appSettings.nCurDict);
	m_appSettings.strDictLocation = GetProfileString(s_pszGlobalSection, s_pszDictLocation, m_appSettings.strDictLocation);
	m_appSettings.nDictChoice = GetProfileInt(s_pszGlobalSection, s_pszDictChoice, m_appSettings.nDictChoice);
	m_bNoAboutOnStartup = !!GetProfileInt(s_pszGlobalSection, s_pszNoAboutOnStartup, 0);


	m_appSettings.bCheckUpdates = !!GetProfileInt(s_pszGlobalSection, s_pszCheckUpdates, m_appSettings.bCheckUpdates);
	UINT nLow = GetProfileInt(s_pszGlobalSection, s_pszLastUpdateLow, static_cast<int>(m_appSettings.nLastUpdateTime & 0xFFFFFFFF));
	UINT nHigh = GetProfileInt(s_pszGlobalSection, s_pszLastUpdateHigh, static_cast<int>(m_appSettings.nLastUpdateTime >> 32));
	m_appSettings.nLastUpdateTime = (static_cast<__int64>(nHigh) << 32) | static_cast<__int64>(nLow);

	m_appSettings.bRestoreTabs = !!GetProfileInt(s_pszTabsSection, s_pszRestoreTabs, m_appSettings.bRestoreTabs);
	m_appSettings.nStartupTab = GetProfileInt(s_pszTabsSection, s_pszStartupTab, 0);
	int nTabCount = GetProfileInt(s_pszTabsSection, s_pszTabCount, 0);
	m_appSettings.openTabs.clear();
	for (int nTab = 0; nTab < nTabCount; ++nTab)
	{
		CString strPathName = GetProfileString(s_pszTabsSection,
			s_pszTabPrefix + FormatString(_T("%d"), nTab), _T(""));
		if (!strPathName.IsEmpty())
			m_appSettings.openTabs.push_back(strPathName);
	}

	m_annoTemplate.bHideInactiveBorder = !!GetProfileInt(s_pszAnnotationsSection, s_pszHideInactiveBorder, m_annoTemplate.bHideInactiveBorder);
	m_annoTemplate.nBorderType = GetProfileInt(s_pszAnnotationsSection, s_pszBorderType, m_annoTemplate.nBorderType);
	m_annoTemplate.crBorder = GetProfileInt(s_pszAnnotationsSection, s_pszBorderColor, m_annoTemplate.crBorder);
	m_annoTemplate.nFillType = GetProfileInt(s_pszAnnotationsSection, s_pszFillType, m_annoTemplate.nFillType);
	m_annoTemplate.crFill = GetProfileInt(s_pszAnnotationsSection, s_pszFillColor, m_annoTemplate.crFill);
	m_annoTemplate.fTransparency = GetProfileDouble(s_pszAnnotationsSection, s_pszTransparency, m_annoTemplate.fTransparency);
	m_annoTemplate.Fix();

	for (int nItem = 0; nItem < CAppSettings::HistorySize; ++nItem)
	{
		CString strItem = GetProfileString(s_pszSearchSection,
				s_pszFindStringPrefix + FormatString(_T("%d"), nItem), _T(""));
		if (!strItem.IsEmpty())
			m_appSettings.searchHistory.push_back(strItem);
	}

	if (!m_appSettings.searchHistory.empty())
		m_appSettings.strFind = m_appSettings.searchHistory.front();

	m_displaySettings.bScaleColorPnm = !!GetProfileInt(s_pszDisplaySection, s_pszScaleMethod, m_displaySettings.bScaleColorPnm);
	m_displaySettings.bScaleSubpix = !!GetProfileInt(s_pszDisplaySection, s_pszScaleSubpix, m_displaySettings.bScaleSubpix);
	m_displaySettings.bInvertColors = !!GetProfileInt(s_pszDisplaySection, s_pszInvertColors, m_displaySettings.bInvertColors);
	m_displaySettings.bAdjustDisplay = !!GetProfileInt(s_pszDisplaySection, s_pszAdjustDisplay, m_displaySettings.bAdjustDisplay);
	m_displaySettings.fGamma = GetProfileDouble(s_pszDisplaySection, s_pszGamma, m_displaySettings.fGamma);
	m_displaySettings.nBrightness = GetProfileInt(s_pszDisplaySection, s_pszBrightness, m_displaySettings.nBrightness);
	m_displaySettings.nContrast = GetProfileInt(s_pszDisplaySection, s_pszContrast, m_displaySettings.nContrast);
	m_displaySettings.Fix();

	m_appSettings.nUnits = GetProfileInt(s_pszDisplaySection, s_pszUnits, m_appSettings.nUnits);
	if (m_appSettings.nUnits < CAppSettings::Centimeters || m_appSettings.nUnits > CAppSettings::Inches)
		m_appSettings.nUnits = CAppSettings::Centimeters;

	m_appSettings.nThumbnailSize = GetProfileInt(s_pszDisplaySection, s_pszThumbnailSize, m_appSettings.nThumbnailSize);
	if (m_appSettings.nThumbnailSize < 0 || m_appSettings.nThumbnailSize >= CAppSettings::ThumbnailSizes)
		m_appSettings.nThumbnailSize = 2;

	m_printSettings.fMarginLeft = GetProfileDouble(s_pszPrintSection, s_pszMarginLeft, m_printSettings.fMarginLeft);
	m_printSettings.fMarginTop = GetProfileDouble(s_pszPrintSection, s_pszMarginTop, m_printSettings.fMarginTop);
	m_printSettings.fMarginRight = GetProfileDouble(s_pszPrintSection, s_pszMarginRight, m_printSettings.fMarginRight);
	m_printSettings.fMarginBottom = GetProfileDouble(s_pszPrintSection, s_pszMarginBottom, m_printSettings.fMarginBottom);
	m_printSettings.fPosLeft = GetProfileDouble(s_pszPrintSection, s_pszPosLeft, m_printSettings.fPosLeft);
	m_printSettings.fPosTop = GetProfileDouble(s_pszPrintSection, s_pszPosTop, m_printSettings.fPosTop);
	m_printSettings.bCenterImage = !!GetProfileInt(s_pszPrintSection, s_pszCenterImage, m_printSettings.bCenterImage);
	m_printSettings.bAutoRotate = !!GetProfileInt(s_pszPrintSection, s_pszAutoRotate, m_printSettings.bAutoRotate);
	m_printSettings.bClipContent = !!GetProfileInt(s_pszPrintSection, s_pszClipContent, m_printSettings.bClipContent);
	m_printSettings.bScaleToFit = !!GetProfileInt(s_pszPrintSection, s_pszScaleToFit, m_printSettings.bScaleToFit);
	m_printSettings.bShrinkOversized = !!GetProfileInt(s_pszPrintSection, s_pszShrinkOversized, m_printSettings.bShrinkOversized);
	m_printSettings.bIgnorePrinterMargins = !!GetProfileInt(s_pszPrintSection, s_pszIgnoreMargins, m_printSettings.bIgnorePrinterMargins);
	m_printSettings.bAdjustPrinting = !!GetProfileInt(s_pszPrintSection, s_pszAdjustPrinting, m_printSettings.bAdjustPrinting);
}

bool CDjViewApp::EnumProfileKeys(LPCTSTR pszSection, vector<CString>& keys)
{
	ASSERT(m_pszRegistryKey != NULL);
	HKEY hSecKey = GetSectionKey(pszSection);
	if (hSecKey == NULL)
		return false;

	TCHAR szName[MAX_PATH];
	for (DWORD dwSubKey = 0; ; ++dwSubKey)
	{
		DWORD dwNameChars = MAX_PATH;
		LONG lResult = ::RegEnumKeyEx(hSecKey, dwSubKey, szName, &dwNameChars, NULL, NULL, NULL, NULL);
		if (lResult == ERROR_SUCCESS)
		{
			dwNameChars = max(0, min(255, dwNameChars));
			szName[dwNameChars] = '\0';
			keys.push_back(szName);
		}
		else if (lResult == ERROR_MORE_DATA)
			continue;
		else
			break;
	}

	::RegCloseKey(hSecKey);
	return true;
}

bool CDjViewApp::LoadDocSettings(const CString& strKey, DocSettings* pSettings)
{
	CString strProfileKey = s_pszDocumentsSection + CString(_T("\\")) + strKey;
	GUTF8String strSettings;
	if (GetProfileCompressed(strProfileKey, s_pszSettings, strSettings))
	{
		// Found settings by key
		stringstream sin((const char*) strSettings);
		XMLParser parser;
		if (parser.Parse(sin))
			pSettings->Load(*parser.GetRoot());

		return true;
	}

	// Try to locate by last known filename
	vector<CString> keys;
	if (!EnumProfileKeys(s_pszDocumentsSection, keys))
		return false;

	for (size_t i = 0; i < keys.size(); ++i)
	{
		CString strPrevKey = s_pszDocumentsSection + CString(_T("\\")) + keys[i];
		CString strFileName = GetProfileString(strPrevKey, s_pszLastKnownLocation, _T(""));
		if (AfxComparePath(strFileName, pSettings->strLastKnownLocation))
		{
			GUTF8String strSettings;
			if (GetProfileCompressed(strPrevKey, s_pszSettings, strSettings))
			{
				// Found settings by file name
				stringstream sin((const char*) strSettings);
				XMLParser parser;
				if (parser.Parse(sin))
					pSettings->Load(*parser.GetRoot());
			}

			// Will be stored by the new name, copy the old values and remove
			// the old key.
			WriteProfileCompressed(strProfileKey, s_pszSettings, strSettings);
			WriteProfileString(strProfileKey, s_pszLastKnownLocation, strFileName);
			DeleteProfileKey(s_pszDocumentsSection, keys[i]);

			return true;
		}
	}

	return false;
}

void CDjViewApp::SaveSettings()
{
	WriteProfileInt(s_pszDisplaySection, s_pszPosX, m_appSettings.nWindowPosX);
	WriteProfileInt(s_pszDisplaySection, s_pszPosY, m_appSettings.nWindowPosY);
	WriteProfileInt(s_pszDisplaySection, s_pszWidth, m_appSettings.nWindowWidth);
	WriteProfileInt(s_pszDisplaySection, s_pszHeight, m_appSettings.nWindowHeight);
	WriteProfileInt(s_pszDisplaySection, s_pszMaximized, m_appSettings.bWindowMaximized);
	WriteProfileInt(s_pszDisplaySection, s_pszToolbar, m_appSettings.bToolbar);
	WriteProfileInt(s_pszDisplaySection, s_pszTabBar, m_appSettings.bTabBar);
	WriteProfileInt(s_pszDisplaySection, s_pszStatusBar, m_appSettings.bStatusBar);
	WriteProfileInt(s_pszDisplaySection, s_pszDictBar, m_appSettings.bDictBar);
	WriteProfileInt(s_pszDisplaySection, s_pszZoom, m_appSettings.nDefaultZoomType);
	WriteProfileDouble(s_pszDisplaySection, s_pszZoomPercent, m_appSettings.fDefaultZoom);
	WriteProfileInt(s_pszDisplaySection, s_pszLayout, m_appSettings.nDefaultLayout);
	WriteProfileInt(s_pszDisplaySection, s_pszFirstPage, m_appSettings.bFirstPageAlone);
	WriteProfileInt(s_pszDisplaySection, s_pszMode, m_appSettings.nDefaultMode);
	WriteProfileInt(s_pszDisplaySection, s_pszNavHidden, m_appSettings.bNavPaneHidden);
	WriteProfileInt(s_pszDisplaySection, s_pszNavCollapsed, m_appSettings.bNavPaneCollapsed);
	WriteProfileInt(s_pszDisplaySection, s_pszNavWidth, m_appSettings.nNavPaneWidth);

	WriteProfileInt(s_pszGlobalSection, s_pszWarnNotDefaultViewer, m_appSettings.bWarnNotDefaultViewer);
	WriteProfileInt(s_pszGlobalSection, s_pszTopLevelDocs, m_appSettings.bTopLevelDocs);
	WriteProfileInt(s_pszGlobalSection, s_pszHideSingleTab, m_appSettings.bHideSingleTab);
	WriteProfileInt(s_pszGlobalSection, s_pszGenAllThumbnails, m_appSettings.bGenAllThumbnails);
	WriteProfileInt(s_pszGlobalSection, s_pszFullscreenClicks, m_appSettings.bFullscreenClicks);
	WriteProfileInt(s_pszGlobalSection, s_pszFullscreenHideScroll, m_appSettings.bFullscreenHideScroll);
	WriteProfileInt(s_pszGlobalSection, s_pszFullscreenContinuousScroll, m_appSettings.bFullscreenContinuousScroll);
	WriteProfileInt(s_pszGlobalSection, s_pszInvertWheelZoom, m_appSettings.bInvertWheelZoom);
	WriteProfileInt(s_pszGlobalSection, s_pszCloseOnEsc, m_appSettings.bCloseOnEsc);
	WriteProfileInt(s_pszGlobalSection, s_pszWrapLongBookmarks, m_appSettings.bWrapLongBookmarks);
	WriteProfileInt(s_pszGlobalSection, s_pszRestoreView, m_appSettings.bRestoreView);
	WriteProfileString(s_pszGlobalSection, s_pszVersion, CURRENT_VERSION);
	WriteProfileInt(s_pszGlobalSection, s_pszLanguage, m_appSettings.nLanguage);
	WriteProfileInt(s_pszGlobalSection, s_pszMatchCase, m_appSettings.bMatchCase);
	WriteProfileInt(s_pszGlobalSection, s_pszCurrentLang, m_appSettings.nCurLang);
	WriteProfileInt(s_pszGlobalSection, s_pszCurrentDict, m_appSettings.nCurDict);
	WriteProfileString(s_pszGlobalSection, s_pszDictLocation, m_appSettings.strDictLocation);
	WriteProfileInt(s_pszGlobalSection, s_pszDictChoice, m_appSettings.nDictChoice);

	WriteProfileInt(s_pszGlobalSection, s_pszCheckUpdates, m_appSettings.bCheckUpdates);
	WriteProfileInt(s_pszGlobalSection, s_pszLastUpdateLow, static_cast<int>(m_appSettings.nLastUpdateTime & 0xFFFFFFFF));
	WriteProfileInt(s_pszGlobalSection, s_pszLastUpdateHigh, static_cast<int>(m_appSettings.nLastUpdateTime >> 32));

	WriteProfileInt(s_pszTabsSection, s_pszRestoreTabs, m_appSettings.bRestoreTabs);
	// Don't save open tabs if showing multiple top-level windows
	if (!m_bTopLevelDocs)
	{
		WriteProfileInt(s_pszTabsSection, s_pszStartupTab, m_appSettings.nStartupTab);
		WriteProfileInt(s_pszTabsSection, s_pszTabCount, (int)m_appSettings.openTabs.size());
		for (int nTab = 0; nTab < (int)m_appSettings.openTabs.size(); ++nTab)
		{
			WriteProfileString(s_pszTabsSection,
				s_pszTabPrefix + FormatString(_T("%d"), nTab), m_appSettings.openTabs[nTab]);
		}
	}

	WriteProfileInt(s_pszAnnotationsSection, s_pszHideInactiveBorder, m_annoTemplate.bHideInactiveBorder);
	WriteProfileInt(s_pszAnnotationsSection, s_pszBorderType, m_annoTemplate.nBorderType);
	WriteProfileInt(s_pszAnnotationsSection, s_pszBorderColor, m_annoTemplate.crBorder);
	WriteProfileInt(s_pszAnnotationsSection, s_pszFillType, m_annoTemplate.nFillType);
	WriteProfileInt(s_pszAnnotationsSection, s_pszFillColor, m_annoTemplate.crFill);
	WriteProfileDouble(s_pszAnnotationsSection, s_pszTransparency, m_annoTemplate.fTransparency);

	list<CString>::iterator itHist;
	int nItem = 0;
	for (itHist = m_appSettings.searchHistory.begin(); itHist != m_appSettings.searchHistory.end(); ++itHist)
	{
		if (!(*itHist).IsEmpty())
		{
			WriteProfileString(s_pszSearchSection,
					s_pszFindStringPrefix + FormatString(_T("%d"), nItem), *itHist);
			++nItem;
		}
	}

	for (; nItem < CAppSettings::HistorySize; ++nItem)
	{
		WriteProfileString(s_pszSearchSection,
				s_pszFindStringPrefix + FormatString(_T("%d"), nItem), _T(""));
	}

	WriteProfileInt(s_pszDisplaySection, s_pszScaleMethod, m_displaySettings.bScaleColorPnm);
	WriteProfileInt(s_pszDisplaySection, s_pszScaleSubpix, m_displaySettings.bScaleSubpix);
	WriteProfileInt(s_pszDisplaySection, s_pszInvertColors, m_displaySettings.bInvertColors);
	WriteProfileInt(s_pszDisplaySection, s_pszAdjustDisplay, m_displaySettings.bAdjustDisplay);
	WriteProfileDouble(s_pszDisplaySection, s_pszGamma, m_displaySettings.fGamma);
	WriteProfileInt(s_pszDisplaySection, s_pszBrightness, m_displaySettings.nBrightness);
	WriteProfileInt(s_pszDisplaySection, s_pszContrast, m_displaySettings.nContrast);
	WriteProfileInt(s_pszDisplaySection, s_pszUnits, m_appSettings.nUnits);
	WriteProfileInt(s_pszDisplaySection, s_pszThumbnailSize, m_appSettings.nThumbnailSize);

	WriteProfileDouble(s_pszPrintSection, s_pszMarginLeft, m_printSettings.fMarginLeft);
	WriteProfileDouble(s_pszPrintSection, s_pszMarginTop, m_printSettings.fMarginTop);
	WriteProfileDouble(s_pszPrintSection, s_pszMarginRight, m_printSettings.fMarginRight);
	WriteProfileDouble(s_pszPrintSection, s_pszMarginBottom, m_printSettings.fMarginBottom);
	WriteProfileDouble(s_pszPrintSection, s_pszPosLeft, m_printSettings.fPosLeft);
	WriteProfileDouble(s_pszPrintSection, s_pszPosTop, m_printSettings.fPosTop);
	WriteProfileInt(s_pszPrintSection, s_pszCenterImage, m_printSettings.bCenterImage);
	WriteProfileInt(s_pszPrintSection, s_pszAutoRotate, m_printSettings.bAutoRotate);
	WriteProfileInt(s_pszPrintSection, s_pszClipContent, m_printSettings.bClipContent);
	WriteProfileInt(s_pszPrintSection, s_pszScaleToFit, m_printSettings.bScaleToFit);
	WriteProfileInt(s_pszPrintSection, s_pszShrinkOversized, m_printSettings.bShrinkOversized);
	WriteProfileInt(s_pszPrintSection, s_pszIgnoreMargins, m_printSettings.bIgnorePrinterMargins);
	WriteProfileInt(s_pszPrintSection, s_pszAdjustPrinting, m_printSettings.bAdjustPrinting);

	const map<MD5, DocSettings>& settings = DjVuSource::GetAllSettings();
	for (map<MD5, DocSettings>::const_iterator it = settings.begin(); it != settings.end(); ++it)
	{
		CString strKey = (*it).first.ToString();
		CString strProfileKey = s_pszDocumentsSection + CString(_T("\\")) + strKey;

		const DocSettings& settings = (*it).second;
		GUTF8String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + settings.GetXML();

		WriteProfileCompressed(strProfileKey, s_pszSettings, xml);
		WriteProfileString(strProfileKey, s_pszLastKnownLocation, settings.strLastKnownLocation);
	}

	map<CString, DictionaryInfo>::iterator itDic;
	for (itDic = m_dictionaries.begin(); itDic != m_dictionaries.end(); ++itDic)
	{
		CString strSection = s_pszDictionariesSection + CString(_T("\\")) + (*itDic).second.strFileName;
		DictionaryInfo& info = (*itDic).second;

		WriteProfileInt(strSection, s_pszFileTimeLow, info.ftModified.dwLowDateTime);
		WriteProfileInt(strSection, s_pszFileTimeHigh, info.ftModified.dwHighDateTime);
		WriteProfileCompressed(strSection, s_pszPageIndex, info.strPageIndex);
		WriteProfileCompressed(strSection, s_pszTitle, info.strTitleRaw);
		WriteProfileCompressed(strSection, s_pszLangFrom, info.strLangFromRaw);
		WriteProfileCompressed(strSection, s_pszLangTo, info.strLangToRaw);
	}
}

BOOL CDjViewApp::WriteProfileCompressed(LPCTSTR pszSection, LPCTSTR pszEntry, const GUTF8String& value)
{
	BOOL bResult = false;
	if (value.length() == 0)
		return bResult;

	GP<ByteStream> raw = ByteStream::create();

	GP<ByteStream> compressed = BSByteStream::create(raw, 128);
	compressed->writall((const char*) value, value.length());
	compressed = NULL;

	UINT nSize = raw->size();
	if (nSize > 0)
	{
		LPBYTE pBuf = new BYTE[nSize];
		raw->readat(pBuf, nSize, 0);

		bResult = WriteProfileBinary(pszSection, pszEntry, pBuf, nSize);
		delete[] pBuf;
	}

	return bResult;
}

BOOL CDjViewApp::GetProfileCompressed(LPCTSTR pszSection, LPCTSTR pszEntry, GUTF8String& value)
{
	LPBYTE pBuf;
	UINT nSize;

	if (GetProfileBinary(pszSection, pszEntry, &pBuf, &nSize))
	{
		GP<ByteStream> raw = ByteStream::create(pBuf, nSize);
		GP<ByteStream> compressed = BSByteStream::create(raw);

		char szTemp[1024];
		string text;
		size_t nRead;
		while ((nRead = compressed->read(szTemp, 1024)) != 0)
		{
			if (text.length() + nRead > text.capacity())
				text.reserve(max(2*text.length(), text.length() + nRead));
			text.append(szTemp, nRead);
		}

		value = text.c_str();

		delete[] pBuf;
		return true;
	}

	return false;
}

int CDjViewApp::ExitInstance()
{
	if (m_bOnlyRegisterTypes)
	{
		::CoUninitialize();
		CWinApp::ExitInstance();
		return m_nExitCode;
	}

	if (m_pFindDlg != NULL)
	{
		m_pFindDlg->DestroyWindow();
		delete m_pFindDlg;
	}

	::KillTimer(NULL, m_nTimerID);
	::UnhookWindowsHookEx(m_hHook);

	ThreadTerminated();
	::WaitForSingleObject(m_terminated, INFINITE);

	SaveSettings();

	DataPool::close_all();
	::CoUninitialize();

	return CWinApp::ExitInstance();
}

void CDjViewApp::InitSearchHistory(CComboBoxEx& cboFind)
{
	CString strText;
	cboFind.GetWindowText(strText);
	cboFind.ResetContent();

	list<CString>::iterator it;
	for (it = m_appSettings.searchHistory.begin(); it != m_appSettings.searchHistory.end(); ++it)
	{
		COMBOBOXEXITEM item;
		item.mask = CBEIF_TEXT;
		item.iItem = cboFind.GetCount();
		item.pszText = (*it).GetBuffer(0);
		cboFind.InsertItem(&item);
	}

	cboFind.SetWindowText(strText);
}

void CDjViewApp::UpdateSearchHistory(CComboBoxEx& cboFind)
{
	CString strText;
	cboFind.GetWindowText(strText);

	if (strText.IsEmpty())
		return;

	int nItem = cboFind.FindStringExact(-1, strText);
	if (nItem != 0)
	{
		// Keep focus on Find dialog
		HWND hwndFocus = NULL;
		if (m_pFindDlg != NULL && m_pFindDlg->IsWindowVisible() && ::IsChild(m_pFindDlg->m_hWnd, ::GetFocus()))
			hwndFocus = ::GetFocus();

		if (nItem != CB_ERR)
			cboFind.DeleteItem(nItem);
		else if (cboFind.GetCount() >= CAppSettings::HistorySize)
			cboFind.DeleteItem(cboFind.GetCount() - 1);

		COMBOBOXEXITEM item;
		item.mask = CBEIF_TEXT;
		item.iItem = 0;
		item.pszText = strText.GetBuffer(0);
		cboFind.InsertItem(&item);

		if (hwndFocus != NULL)
			::SetFocus(hwndFocus);
	}
	cboFind.SetCurSel(0);

	list<CString>::iterator it = find(m_appSettings.searchHistory.begin(),
			m_appSettings.searchHistory.end(), strText);
	if (it == m_appSettings.searchHistory.end()
			|| it != m_appSettings.searchHistory.begin())
	{
		if (it != m_appSettings.searchHistory.end())
			m_appSettings.searchHistory.erase(it);

		if (m_appSettings.searchHistory.size() >= CAppSettings::HistorySize)
			m_appSettings.searchHistory.pop_back();

		m_appSettings.searchHistory.push_front(strText);
	}
}

LRESULT CALLBACK CDjViewApp::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		bool bPressed = (lParam & 0x80000000) == 0;
		if (wParam == VK_SHIFT && bPressed != theApp.m_bShiftPressed)
		{
			theApp.m_bShiftPressed = bPressed;
			theApp.UpdateObservers(KeyStateChanged(VK_SHIFT, bPressed));
		}
		else if (wParam == VK_CONTROL && bPressed != theApp.m_bControlPressed)
		{
			theApp.m_bControlPressed = bPressed;
			theApp.UpdateObservers(KeyStateChanged(VK_CONTROL, bPressed));
		}
	}

	return ::CallNextHookEx(theApp.m_hHook, nCode, wParam, lParam);
}

void CALLBACK CDjViewApp::TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	bool bShiftPressed = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
	bool bControlPressed = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
	if (bShiftPressed != theApp.m_bShiftPressed)
	{
		theApp.m_bShiftPressed = bShiftPressed;
		theApp.UpdateObservers(KeyStateChanged(VK_SHIFT, bShiftPressed));
	}
	if (bControlPressed != theApp.m_bControlPressed)
	{
		theApp.m_bControlPressed = bControlPressed;
		theApp.UpdateObservers(KeyStateChanged(VK_CONTROL, bControlPressed));
	}
}

bool RegDeleteKeyRec(HKEY hKeyRoot, LPCTSTR lpszSubKey)
{
	if (::RegDeleteKey(hKeyRoot, lpszSubKey) == ERROR_SUCCESS)
		return true;

	HKEY hKey;
	LRESULT lResult = ::RegOpenKeyEx(hKeyRoot, lpszSubKey, 0, KEY_READ, &hKey);
	if (lResult == ERROR_FILE_NOT_FOUND)
		return true;
	else if (lResult != ERROR_SUCCESS) 
		return false;

	CString strSubKey = lpszSubKey;
	if (!strSubKey.IsEmpty() && strSubKey[strSubKey.GetLength() - 1] != '\\')
		strSubKey += '\\';

    // Enumerate the keys
	TCHAR szKey[MAX_PATH];
	DWORD dwNameChars = MAX_PATH;
	while (::RegEnumKeyEx(hKey, 0, szKey, &dwNameChars, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
	{
		if (!RegDeleteKeyRec(hKeyRoot, strSubKey + szKey))
			break;
		dwNameChars = MAX_PATH;
	}

	::RegCloseKey(hKey);

    // Try again to delete the key.
	return (::RegDeleteKey(hKeyRoot, lpszSubKey) == ERROR_SUCCESS);
}

bool CDjViewApp::DeleteProfileKey(LPCTSTR pszSection, LPCTSTR pszKey)
{
	ASSERT(m_pszRegistryKey != NULL);
	HKEY hSecKey = GetSectionKey(pszSection);
	if (hSecKey == NULL)
		return false;

	bool bResult = RegDeleteKeyRec(hSecKey, pszKey);
	::RegCloseKey(hSecKey);
	return bResult;
}

bool SetRegHKCRValue(LPCTSTR lpszKey, LPCTSTR lpszValue, bool& bChanged, bool bCheckOnly)
{
	CString strOldValue;
	LONG cbData;
	if (::RegQueryValue(HKEY_CLASSES_ROOT, lpszKey, NULL, &cbData) == ERROR_SUCCESS &&
		::RegQueryValue(HKEY_CLASSES_ROOT, lpszKey, strOldValue.GetBufferSetLength(cbData / sizeof(TCHAR) + 1), &cbData) == ERROR_SUCCESS)
	{
		strOldValue.ReleaseBuffer();
		if (strOldValue == lpszValue)
			return true;
	}

	if (!bCheckOnly && ::RegSetValue(HKEY_CLASSES_ROOT, lpszKey, REG_SZ,
			lpszValue, lstrlen(lpszValue) * sizeof(TCHAR)) != ERROR_SUCCESS)
		return false;

	bChanged = true;
	return true;
}

bool CDjViewApp::RegisterShellFileTypes(bool bCheckOnly)
{
	bool bSuccess = true;
	bool bChanged = false;
	CString strPathName, strRawPathName, strTemp;

	GetModuleFileName(m_hInstance, strRawPathName.GetBuffer(MAX_PATH), MAX_PATH);
	strRawPathName.ReleaseBuffer();
	PathCanonicalize(strPathName.GetBuffer(MAX_PATH), strRawPathName);
	strPathName.ReleaseBuffer();

	POSITION pos = GetFirstDocTemplatePosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = GetNextDocTemplate(pos);

		CString strOpenCommandLine;
		strOpenCommandLine.Format(_T("\"%s\""), strPathName);

		CString strDefaultIconCommandLine;
		strDefaultIconCommandLine.Format(_T("%s,0"), strPathName, 0);

		CString strFilterExt, strFileTypeId, strFileTypeName;
		if (pTemplate->GetDocString(strFileTypeId,
			CDocTemplate::regFileTypeId) && !strFileTypeId.IsEmpty())
		{
			// enough info to register it
			if (!pTemplate->GetDocString(strFileTypeName, CDocTemplate::regFileTypeName))
				strFileTypeName = strFileTypeId;    // use id name

			ASSERT(strFileTypeId.Find(' ') == -1);  // no spaces allowed

			// first register the type ID of our server
			if (!SetRegHKCRValue(strFileTypeId, strFileTypeName, bChanged, bCheckOnly))
			{
				bSuccess = false;
				continue;
			}

			strTemp.Format(_T("%s\\DefaultIcon"), (LPCTSTR)strFileTypeId);
			if (!SetRegHKCRValue(strTemp, strDefaultIconCommandLine, bChanged, bCheckOnly))
			{
				bSuccess = false;
				continue;
			}

			strTemp.Format(_T("%s\\shell\\open\\%s"), (LPCTSTR)strFileTypeId,
				(LPCTSTR)_T("ddeexec"));
			if (!SetRegHKCRValue(strTemp, _T("[open(\"%1\")]"), bChanged, bCheckOnly))
			{
				bSuccess = false;
				continue;
			}

			strTemp.Format(_T("%s\\shell\\open\\%s\\Application"), (LPCTSTR)strFileTypeId,
				(LPCTSTR)_T("ddeexec"));
			if (!SetRegHKCRValue(strTemp, _T("WinDjView"), bChanged, bCheckOnly))
			{
				bSuccess = false;
				continue;
			}

			strTemp.Format(_T("%s\\shell\\open\\%s\\Topic"), (LPCTSTR)strFileTypeId,
				(LPCTSTR)_T("ddeexec"));
			if (!SetRegHKCRValue(strTemp, _T("System"), bChanged, bCheckOnly))
			{
				bSuccess = false;
				continue;
			}

			strTemp.Format(_T("%s\\shell\\open\\%s\\IfExec"), (LPCTSTR)strFileTypeId,
				(LPCTSTR)_T("ddeexec"));
			if (!SetRegHKCRValue(strTemp, _T("[rem open]"), bChanged, bCheckOnly))
			{
				bSuccess = false;
				continue;
			}

			strOpenCommandLine += _T(" \"%1\"");

			// path\shell\open\command = path filename
			strTemp.Format(_T("%s\\shell\\open\\%s"), (LPCTSTR)strFileTypeId,
				_T("command"));
			if (!SetRegHKCRValue(strTemp, strOpenCommandLine, bChanged, bCheckOnly))
			{
				bSuccess = false;
				continue;
			}

			CString strExtensions;
			pTemplate->GetDocString(strExtensions, CDocTemplate::filterExt);

			int nExt = 0;
			while (AfxExtractSubString(strFilterExt, strExtensions, nExt++, ';')
					&& !strFilterExt.IsEmpty())
			{
				ASSERT(strFilterExt[0] == '.');

				CString strExplorerKey = FormatString(_T("Software\\Microsoft\\Windows\\")
						_T("CurrentVersion\\Explorer\\FileExts\\%s"), strFilterExt);
				CString strUserChoiceKey = strExplorerKey + _T("\\UserChoice");
				CString strProgId = _T("ProgID");
				HKEY hKey = NULL;
				::RegOpenKeyEx(HKEY_CURRENT_USER, strExplorerKey, 0, KEY_READ | KEY_WRITE, &hKey);

				bool bPrevChanged = bChanged;
				bChanged = false;
				if (!SetRegHKCRValue(strFilterExt, strFileTypeId, bChanged, bCheckOnly)
						|| bCheckOnly && bChanged)
				{
					bChanged = bPrevChanged;

					// Setting global association failed, set the user-level one instead.
					if (hKey != NULL)
					{
						CString strData;
						DWORD dwType;
						DWORD cbData = 0;
						if (::RegQueryValueEx(hKey, strProgId, 0, &dwType, NULL, &cbData) != ERROR_SUCCESS
								|| dwType != REG_SZ
								|| ::RegQueryValueEx(hKey, strProgId, 0, &dwType, (LPBYTE)(LPTSTR) strData.GetBufferSetLength(cbData / sizeof(TCHAR) + 1), &cbData) != ERROR_SUCCESS
								|| (strData.ReleaseBuffer(), strData != strFileTypeId))
						{
							if (!bCheckOnly && ::RegSetValueEx(hKey, strProgId, 0, REG_SZ, (LPBYTE)(LPCTSTR) strFileTypeId, strFileTypeId.GetLength() * sizeof(TCHAR)) != ERROR_SUCCESS)
								bSuccess = false;
							else
								bChanged = true;
						}

						HKEY hChoiceKey = NULL;
						if (::RegOpenKeyEx(HKEY_CURRENT_USER, strUserChoiceKey, 0, KEY_READ, &hChoiceKey) != ERROR_SUCCESS
								|| ::RegQueryValueEx(hChoiceKey, _T("Progid"), 0, &dwType, NULL, &cbData) != ERROR_SUCCESS
								|| dwType != REG_SZ
								|| ::RegQueryValueEx(hChoiceKey, _T("Progid"), 0, &dwType, (LPBYTE)(LPTSTR) strData.GetBufferSetLength(cbData / sizeof(TCHAR) + 1), &cbData) != ERROR_SUCCESS
								|| (strData.ReleaseBuffer(), strData != strFileTypeId))
						{
							if (hChoiceKey != NULL)
								::RegCloseKey(hChoiceKey);

							if (bCheckOnly)
							{
								bChanged = true;
							}
							else if (!RegDeleteKeyRec(HKEY_CURRENT_USER, strUserChoiceKey))
							{
								bSuccess = false;
							}
							else
							{
								if (::RegCreateKey(HKEY_CURRENT_USER, strUserChoiceKey, &hChoiceKey) != ERROR_SUCCESS
										|| ::RegSetValueEx(hChoiceKey, _T("Progid"), 0, REG_SZ, (LPBYTE)(LPCTSTR) strFileTypeId, strFileTypeId.GetLength() * sizeof(TCHAR)) != ERROR_SUCCESS)
									bSuccess = false;
								else
									bChanged = true;
							}

							if (hChoiceKey != NULL)
								::RegCloseKey(hChoiceKey);
						}
					}
					else
					{
						HKEY hChoiceKey = NULL;
						if (bCheckOnly)
						{
							bChanged = true;
						}
						else if (::RegCreateKey(HKEY_CURRENT_USER, strExplorerKey, &hKey) != ERROR_SUCCESS)
						{
							bSuccess = false;
						}
						else
						{
							bChanged = true;
							if (::RegSetValueEx(hKey, strProgId, 0, REG_SZ, (LPBYTE)(LPCTSTR) strFileTypeId, strFileTypeId.GetLength() * sizeof(TCHAR)) != ERROR_SUCCESS
									|| ::RegCreateKey(HKEY_CURRENT_USER, strUserChoiceKey, &hChoiceKey) != ERROR_SUCCESS
									|| ::RegSetValueEx(hChoiceKey, _T("Progid"), 0, REG_SZ, (LPBYTE)(LPCTSTR) strFileTypeId, strFileTypeId.GetLength() * sizeof(TCHAR)) != ERROR_SUCCESS)
								bSuccess = false;
						}

						if (hChoiceKey != NULL)
							::RegCloseKey(hChoiceKey);
					}
				}
				else
				{
					bChanged = bPrevChanged || bChanged;

					if (hKey != NULL)
					{
						// Setting global association succeeded, remove the user-level one.
						DWORD dwType;
						DWORD cbData = 0;
						CString strData;
						if (::RegQueryValueEx(hKey, strProgId, 0, &dwType, NULL, &cbData) == ERROR_SUCCESS)
						{
							if (dwType != REG_SZ
									|| ::RegQueryValueEx(hKey, strProgId, 0, &dwType, (LPBYTE)(LPTSTR) strData.GetBufferSetLength(cbData / sizeof(TCHAR) + 1), &cbData) != ERROR_SUCCESS
									|| (strData.ReleaseBuffer(), strData != strFileTypeId))
							{
								if (!bCheckOnly && ::RegDeleteValue(hKey, strProgId) != ERROR_SUCCESS)
									bSuccess = false;
								else
									bChanged = true;
							}
						}

						HKEY hChoiceKey = NULL;
						if (::RegOpenKeyEx(HKEY_CURRENT_USER, strUserChoiceKey, 0, KEY_READ, &hChoiceKey) == ERROR_SUCCESS
								&& ::RegQueryValueEx(hChoiceKey, _T("Progid"), 0, &dwType, NULL, &cbData) == ERROR_SUCCESS)
						{
							if (dwType != REG_SZ
									|| ::RegQueryValueEx(hChoiceKey, _T("Progid"), 0, &dwType, (LPBYTE)(LPTSTR) strData.GetBufferSetLength(cbData / sizeof(TCHAR) + 1), &cbData) != ERROR_SUCCESS
									|| (strData.ReleaseBuffer(), strData != strFileTypeId))
							{
								::RegCloseKey(hChoiceKey);
								if (!bCheckOnly && !RegDeleteKeyRec(HKEY_CURRENT_USER, strUserChoiceKey))
									bSuccess = false;
								else
									bChanged = true;
							}
						}
					}
				}

				if (hKey != NULL)
					::RegCloseKey(hKey);
			}
		}
	}

	if (bCheckOnly)
		return bChanged;

	if (bChanged && bSuccess)
		::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

	return bSuccess;
}

bool CDjViewApp::RegisterShellFileTypesElevate(CWnd* pWnd)
{
	if (RegisterShellFileTypes())
		return true;

	// Try to run with elevated priveleges

	HINSTANCE hKernel32 = ::LoadLibrary(_T("kernel32.dll"));
	if (hKernel32 == NULL)
		return false;

	typedef BOOL (WINAPI* pfnGetExitCodeProcess)(HANDLE hProcess, LPDWORD lpExitCode);
	pfnGetExitCodeProcess pGetExitCodeProcess =
			(pfnGetExitCodeProcess) ::GetProcAddress(hKernel32, "GetExitCodeProcess");
	if (pGetExitCodeProcess == NULL)
	{
		::FreeLibrary(hKernel32);
		return false;
	}

	CString strPathName;
	GetModuleFileName(m_hInstance, strPathName.GetBuffer(MAX_PATH + 1), MAX_PATH + 1);
	strPathName.ReleaseBuffer();

	SHELLEXECUTEINFO execinfo;
	ZeroMemory(&execinfo, sizeof(execinfo));
	execinfo.cbSize = sizeof(execinfo);
	execinfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	execinfo.hwnd = pWnd->GetSafeHwnd();
	execinfo.lpVerb = _T("runas");
	execinfo.lpFile = strPathName;
	execinfo.lpParameters = _T("/RegisterFileTypes");
	execinfo.nShow = SW_HIDE;
	if (!ShellExecuteEx(&execinfo) || execinfo.hProcess == NULL)
	{
		::FreeLibrary(hKernel32);
		return false;
	}

	bool bSuccess = false;
	if (::WaitForSingleObject(execinfo.hProcess, INFINITE) == WAIT_OBJECT_0)
	{
		DWORD dwExitCode = 1;
		if (pGetExitCodeProcess(execinfo.hProcess, &dwExitCode))
			bSuccess = (dwExitCode == 0);
	}
	::CloseHandle(execinfo.hProcess);

	::FreeLibrary(hKernel32);
	return bSuccess;
}

void CDjViewApp::OnFileSettings()
{
	CSettingsDlg dlg;
	if (dlg.DoModal() == IDOK)
	{
		m_appSettings.bWarnNotDefaultViewer = !!dlg.m_pageAdvanced.m_bWarnNotDefaultViewer;
		m_appSettings.bRestoreView = !!dlg.m_pageAdvanced.m_bRestoreView;
		m_appSettings.bCheckUpdates = !!dlg.m_pageAdvanced.m_bCheckUpdates;

		m_appSettings.bTopLevelDocs = !!dlg.m_pageGeneral.m_bTopLevelDocs;
		m_appSettings.bRestoreTabs = !!dlg.m_pageGeneral.m_bRestoreTabs;
		m_appSettings.bHideSingleTab = !!dlg.m_pageGeneral.m_bHideSingleTab;
		m_appSettings.bGenAllThumbnails = !!dlg.m_pageGeneral.m_bGenAllThumbnails;
		m_appSettings.bInvertWheelZoom = !!dlg.m_pageGeneral.m_bInvertWheelZoom;
		m_appSettings.bCloseOnEsc = !!dlg.m_pageGeneral.m_bCloseOnEsc;
		m_appSettings.bWrapLongBookmarks = !!dlg.m_pageGeneral.m_bWrapLongBookmarks;
		m_appSettings.bFullscreenClicks = !!dlg.m_pageGeneral.m_bFullscreenClicks;
		m_appSettings.bFullscreenHideScroll = !!dlg.m_pageGeneral.m_bFullscreenHideScroll;
		m_appSettings.bFullscreenContinuousScroll = !!dlg.m_pageGeneral.m_bFullscreenContinuousScroll;

		m_appSettings.nUnits = dlg.m_pageDisplay.m_nUnits;
		m_displaySettings = dlg.m_pageDisplay.m_displaySettings;

		m_printSettings.bAdjustPrinting = !!dlg.m_pageDisplay.m_bAdjustPrinting;

		if (GetDictLangsCount() > 0 && m_appSettings.strDictLocation != dlg.m_pageDict.m_strDictLocation)
		{
			m_appSettings.strDictLocation = dlg.m_pageDict.m_strDictLocation;
			ReloadDictionaries();
		}

		SaveSettings();

		UpdateObservers(APP_SETTINGS_CHANGED);

		if (m_bTopLevelDocs != m_appSettings.bTopLevelDocs)
			AfxMessageBox(IDS_RESTART_NEEDED);
	}
}

CDjVuDoc* CDjViewApp::OpenDocument(LPCTSTR lpszPathName, const GUTF8String& strPage, bool bAddHistoryPoint)
{
	CDjVuDoc* pDoc = (CDjVuDoc*) m_pDocManager->OpenDocumentFile(lpszPathName);
	if (pDoc == NULL)
		return NULL;

	CDjVuView* pView = pDoc->GetDjVuView();
	if (strPage.length() > 0)
		pView->GoToURL(strPage, bAddHistoryPoint);

	return pDoc;
}

CDjVuDoc* CDjViewApp::FindOpenDocument(LPCTSTR lpszFileName)
{
	TCHAR szPath[MAX_PATH], szTemp[MAX_PATH];
	ASSERT(lstrlen(lpszFileName) < _countof(szPath));
	if (lpszFileName[0] == '\"')
		++lpszFileName;
	lstrcpyn(szTemp, lpszFileName, MAX_PATH);
	LPTSTR lpszLast = _tcsrchr(szTemp, '\"');
	if (lpszLast != NULL)
		*lpszLast = 0;

	if (!AfxFullPath(szPath, szTemp))
		return NULL; // We won't open the file. MFC requires paths with length < MAX_PATH

	TCHAR szLinkName[MAX_PATH];
	if (AfxResolveShortcut(GetMainWnd(), szPath, szLinkName, MAX_PATH))
		lstrcpy(szPath, szLinkName);

	POSITION pos = GetFirstDocTemplatePosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = GetNextDocTemplate(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);

		CDocument* pDocument = NULL;
		if (pTemplate->MatchDocType(szPath, pDocument) == CDocTemplate::yesAlreadyOpen)
			return (CDjVuDoc*) pDocument;
	}

	return NULL;
}

void CDjViewApp::OnCheckForUpdate()
{
	CUpdateDlg dlg;
	dlg.DoModal();
}

void CDjViewApp::LoadLanguages()
{
	LanguageInfo english;
	english.nLanguage = 0x409;
	english.strLanguage = _T("&English");
	english.hInstance = AfxGetInstanceHandle();
	m_languages.push_back(english);

	CString strPath;
	GetModuleFileName(m_hInstance, strPath.GetBuffer(MAX_PATH), MAX_PATH);
	PathRemoveFileSpec(strPath.GetBuffer(MAX_PATH));
	PathRemoveBackslash(strPath.GetBuffer(MAX_PATH));
	strPath.ReleaseBuffer();
	strPath += _T("\\");

	CString strFileMask = strPath;
	if (!PathAppend(strFileMask.GetBuffer(MAX_PATH), _T("WinDjView*.dll")))
		return;
	strFileMask.ReleaseBuffer();

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(strFileMask, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do
	{
		CString strPathName = strPath + fd.cFileName;

		DWORD dwHandle;
		DWORD dwSize = ::GetFileVersionInfoSize(strPathName.GetBuffer(0), &dwHandle);
		if (dwSize <= 0)
			continue;

		vector<BYTE> versionInfo(dwSize);
		LPBYTE pVersionInfo = &versionInfo[0];
		if (::GetFileVersionInfo(strPathName.GetBuffer(0), dwHandle, dwSize, pVersionInfo) == 0)
			continue;

		DWORD* pTranslations;
		UINT cbTranslations;
		if (::VerQueryValue(pVersionInfo, _T("\\VarFileInfo\\Translation"),
				(void**)&pTranslations, &cbTranslations) == 0 || cbTranslations == 0)
			continue;

		DWORD nLanguage = LOWORD(*pTranslations);
		CString strTranslation = FormatString(_T("%04x%04x"), nLanguage, HIWORD(*pTranslations));

		LPCTSTR pszBuffer;
		UINT dwLength;
		if (::VerQueryValue(pVersionInfo, FormatString(_T("\\StringFileInfo\\%s\\FileVersion"), strTranslation).GetBuffer(0),
				(void**)&pszBuffer, &dwLength) == 0 || dwLength == 0)
			continue;

		CString strVersion(pszBuffer);
		if (strVersion != CURRENT_VERSION)
			continue;

		if (::VerQueryValue(pVersionInfo, FormatString(_T("\\StringFileInfo\\%s\\Comments"), strTranslation).GetBuffer(0),
				(void**)&pszBuffer, &dwLength) == 0 || dwLength == 0)
			continue;

		CString strLanguage(pszBuffer);

		LanguageInfo info;
		info.nLanguage = nLanguage;
		info.strLanguage = strLanguage;
		info.strLibraryPath = strPathName;
		info.hInstance = NULL;
		m_languages.push_back(info);
	} while (FindNextFile(hFind, &fd) != 0);

	FindClose(hFind);
}

void CDjViewApp::OnSetLanguage(UINT nID)
{
	int nLangIndex = nID - ID_LANGUAGE_FIRST - 1;
	SetLanguage(nLangIndex);
}

void CDjViewApp::SetLanguage(UINT nLangIndex)
{
	if (nLangIndex < 0 || nLangIndex >= m_languages.size())
		return;

	LanguageInfo& info = m_languages[nLangIndex];
	if (info.hInstance == NULL)
	{
		info.hInstance = ::LoadLibrary(info.strLibraryPath);
		if (info.hInstance == NULL)
		{
			AfxMessageBox(_T("Could not change the language to ") + info.strLanguage);
			return;
		}
	}

	if (nLangIndex == m_nLangIndex)
		return;

	m_nLangIndex = nLangIndex;
	AfxSetResourceHandle(info.hInstance);
	m_appSettings.nLanguage = info.nLanguage;

	m_pDjVuTemplate->UpdateTemplate();
	UpdateDictProperties();
	UpdateFindDlg();

	UpdateObservers(APP_LANGUAGE_CHANGED);
}

void CDjViewApp::OnUpdateLanguageList(CCmdUI* pCmdUI)
{
	if (pCmdUI->m_pMenu == NULL)
		return;

	int nIndex = pCmdUI->m_nIndex;
	int nAdded = 0;

	for (size_t i = 0; i < m_languages.size() && i < ID_LANGUAGE_LAST - ID_LANGUAGE_FIRST - 1; ++i)
	{
		CString strText = m_languages[i].strLanguage;
		pCmdUI->m_pMenu->InsertMenu(ID_LANGUAGE_FIRST, MF_BYCOMMAND, ID_LANGUAGE_FIRST + i + 1, strText);
		++nAdded;
	}
	pCmdUI->m_pMenu->DeleteMenu(ID_LANGUAGE_FIRST, MF_BYCOMMAND);

	// update end menu count
	pCmdUI->m_nIndex -= 1;
	pCmdUI->m_nIndexMax += nAdded - 1;
}

void CDjViewApp::OnUpdateLanguage(CCmdUI* pCmdUI)
{
	int nLangIndex = pCmdUI->m_nID - ID_LANGUAGE_FIRST - 1;
	pCmdUI->SetCheck(nLangIndex == m_nLangIndex);
}

void CDjViewApp::SetStartupLanguage()
{
	for (int i = 0; i < (int)m_languages.size(); ++i)
	{
		if (m_appSettings.nLanguage == m_languages[i].nLanguage)
		{
			SetLanguage(i);
			return;
		}
	}

	// English by default
	SetLanguage(0);
}

BOOL CDjViewApp::OnOpenRecentFile(UINT nID)
{
	// Fixed MFC: CWinApp::OnOpenRecentFile
	// Always moves the file to the top of the recents list

	ASSERT_VALID(this);
	ASSERT(m_pRecentFileList != NULL);

	ASSERT(nID >= ID_FILE_MRU_FILE1 && nID < ID_FILE_MRU_FILE1 + (UINT)m_pRecentFileList->GetSize());
	int nIndex = nID - ID_FILE_MRU_FILE1;

	CString& strPathName = (*m_pRecentFileList)[nIndex];
	ASSERT(strPathName.GetLength() != 0);
	TRACE(_T("MRU: open file (%d) '%s'.\n"), nIndex + 1, strPathName);

	CDjVuDoc* pDoc = (CDjVuDoc*) OpenDocumentFile(strPathName);
	if (pDoc == NULL)
	{
		m_pRecentFileList->Remove(nIndex);
	}
	else
	{
		AddToRecentFileList(pDoc->GetPathName());
	}

	return true;
}

void CDjViewApp::ReportFatalError()
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

struct ShellAPI
{
	ShellAPI();
	~ShellAPI();

	typedef HRESULT (WINAPI* pfnSHGetFolderPath)(HWND hwndOwner, int nFolder,
		HANDLE hToken, DWORD dwFlags, LPTSTR pszPath);
	typedef BOOL (WINAPI* pfnSHGetSpecialFolderPath)(HWND hwndOwner, LPTSTR lpszPath,
		int nFolder, BOOL fCreate);

	pfnSHGetFolderPath pSHGetFolderPath;
	pfnSHGetSpecialFolderPath pSHGetSpecialFolderPath;

	bool IsLoaded() const { return hShell32 != NULL; }
	HINSTANCE hShell32, hSHFolder;
};

static ShellAPI theShellAPI;

ShellAPI::ShellAPI()
	: pSHGetFolderPath(NULL), pSHGetSpecialFolderPath(NULL),
	  hShell32(NULL), hSHFolder(NULL)
{
	hShell32 = ::LoadLibrary(_T("shell32.dll"));
	if (hShell32 != NULL)
	{
		pSHGetFolderPath = (pfnSHGetFolderPath) ::GetProcAddress(hShell32, "SHGetFolderPathW");
		pSHGetSpecialFolderPath = (pfnSHGetSpecialFolderPath) ::GetProcAddress(hShell32, "SHGetSpecialFolderPathW");

		if (pSHGetFolderPath == NULL)
		{
			hSHFolder = ::LoadLibrary(_T("shfolder.dll"));
			if (hSHFolder != NULL)
			{
				pSHGetFolderPath = (pfnSHGetFolderPath) ::GetProcAddress(hSHFolder, "SHGetFolderPathW");
			}
		}
	}
}

ShellAPI::~ShellAPI()
{
	if (hShell32 != NULL)
		::FreeLibrary(hShell32);
	if (hSHFolder != NULL)
		::FreeLibrary(hSHFolder);
}

void CDjViewApp::LoadDictionaries()
{
	TCHAR szFolder[MAX_PATH];
	if (theShellAPI.pSHGetFolderPath != NULL)
	{
		if (SUCCEEDED(theShellAPI.pSHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szFolder)))
		{
			if (PathAppend(szFolder, _T("WinDjView\\Dictionaries")))
				LoadDictionaries(szFolder);
		}

		if (SUCCEEDED(theShellAPI.pSHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szFolder)))
		{
			if (PathAppend(szFolder, _T("WinDjView\\Dictionaries")))
				LoadDictionaries(szFolder);
		}
	}
	else if (theShellAPI.pSHGetSpecialFolderPath != NULL)
	{
		if (theShellAPI.pSHGetSpecialFolderPath(NULL, szFolder, CSIDL_COMMON_APPDATA, false))
		{
			if (PathAppend(szFolder, _T("WinDjView\\Dictionaries")))
				LoadDictionaries(szFolder);
		}

		if (theShellAPI.pSHGetSpecialFolderPath(NULL, szFolder, CSIDL_APPDATA, false))
		{
			if (PathAppend(szFolder, _T("WinDjView\\Dictionaries")))
				LoadDictionaries(szFolder);
		}
	}

	GetModuleFileName(m_hInstance, szFolder, MAX_PATH);
	PathRemoveFileSpec(szFolder);
	if (PathAppend(szFolder, _T("Dictionaries")))
		LoadDictionaries(szFolder);

	if (!m_appSettings.strDictLocation.IsEmpty() && !PathIsRelative(m_appSettings.strDictLocation))
		LoadDictionaries(m_appSettings.strDictLocation);

	// Purge deleted dictionaries from registry
	HKEY hSecKey = GetSectionKey(s_pszDictionariesSection);
	if (hSecKey != NULL)
	{
		DWORD nSubKeys;
		if (RegQueryInfoKey(hSecKey, NULL, NULL, NULL, &nSubKeys, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
		{
			TCHAR szKey[MAX_PATH];
			for (int nKey = static_cast<int>(nSubKeys) - 1; nKey >= 0; --nKey)
			{
				DWORD dwSize = MAX_PATH;
				if (::RegEnumKeyEx(hSecKey, nKey, szKey, &dwSize, 0, NULL, NULL, NULL) != ERROR_SUCCESS)
					break;

				CString strKey = szKey;
				strKey.MakeLower();
				if (m_dictionaries.find(strKey) == m_dictionaries.end())
				{
					::RegDeleteKey(hSecKey, szKey);
				}
			}
		}

		::RegCloseKey(hSecKey);
	}

	UpdateDictVector();
	UpdateDictProperties();
}

void CDjViewApp::LoadDictionaries(CString strDirectory)
{
	PathRemoveBackslash(strDirectory.GetBuffer(MAX_PATH));
	strDirectory.ReleaseBuffer();
	strDirectory += _T("\\");

	CString strFileMask = strDirectory;
	if (!PathAppend(strFileMask.GetBuffer(MAX_PATH), _T("*")))
		return;
	strFileMask.ReleaseBuffer();

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(strFileMask, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			DictionaryInfo info;
			info.strPathName = strDirectory + fd.cFileName;
			info.strFileName = fd.cFileName;
			info.bInstalled = true;

			if ((GetFileAttributes(info.strPathName) & FILE_ATTRIBUTE_DIRECTORY) != 0)
				continue;

			CString strKey = info.strFileName;
			strKey.MakeLower();
			if (m_dictionaries.find(strKey) != m_dictionaries.end())
				continue;

			LoadDictionaryInfo(info);

			if (!LoadDictionaryInfoFromDisk(info))
				continue;

			m_dictionaries.insert(make_pair(strKey, info));
		} while (FindNextFile(hFind, &fd) != 0);

		FindClose(hFind);
	}
}

void CDjViewApp::LoadDictionaryInfo(DictionaryInfo& info)
{
	CString strSection = s_pszDictionariesSection + CString(_T("\\")) + info.strFileName;

	info.ftModified.dwLowDateTime = GetProfileInt(strSection, s_pszFileTimeLow, 0);
	info.ftModified.dwHighDateTime = GetProfileInt(strSection, s_pszFileTimeHigh, 0);

	GUTF8String str;
	if (GetProfileCompressed(strSection, s_pszPageIndex, str))
		info.ReadPageIndex(str, false);
	if (GetProfileCompressed(strSection, s_pszTitle, str))
		info.ReadTitle(str, false);
	if (GetProfileCompressed(strSection, s_pszLangFrom, str))
		info.ReadLangFrom(str, false);
	if (GetProfileCompressed(strSection, s_pszLangTo, str))
		info.ReadLangTo(str, false);
}

bool CDjViewApp::LoadDictionaryInfoFromDisk(DictionaryInfo& info)
{
	HANDLE hFile = ::CreateFile(info.strPathName, GENERIC_READ,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
		return false;

	FILETIME ftModified;
	if (!::GetFileTime(hFile, NULL, NULL, &ftModified))
	{
		::CloseHandle(hFile);
		return false;
	}

	::CloseHandle(hFile);

	if (memcmp(&ftModified, &info.ftModified, sizeof(FILETIME)) == 0)
		return true;

	info.ftModified = ftModified;

	DjVuSource* pSource = DjVuSource::FromFile(info.strPathName);
	if (pSource == NULL)
		return false;

	if (!pSource->IsDictionary())
	{
		pSource->Release();
		return false;
	}

	info.titleLoc = pSource->GetDictionaryInfo()->titleLoc;
	info.langFromLoc = pSource->GetDictionaryInfo()->langFromLoc;
	info.langToLoc = pSource->GetDictionaryInfo()->langToLoc;
	info.strLangFromCode = pSource->GetDictionaryInfo()->strLangFromCode;
	info.strLangToCode = pSource->GetDictionaryInfo()->strLangToCode;
	info.strTitleRaw = pSource->GetDictionaryInfo()->strTitleRaw;
	info.strLangFromRaw = pSource->GetDictionaryInfo()->strLangFromRaw;
	info.strLangToRaw = pSource->GetDictionaryInfo()->strLangToRaw;
	info.strPageIndex = pSource->GetDictionaryInfo()->strPageIndex;

	pSource->Release();
	return true;
}

bool CDjViewApp::InstallDictionary(DjVuSource* pSource, int nLocationChoice, bool bKeepOriginal)
{
	CString strOldPathName = pSource->GetFileName();
	CString strPath;

	DictionaryInfo* pPrevInfo = GetDictionaryInfo(strOldPathName, false);
	if (pPrevInfo != NULL)
	{
		// Replacing existing dictionary. Check that this is not the same file.
		if (AfxComparePath(pPrevInfo->strPathName, strOldPathName))
			return true;

		strPath = pPrevInfo->strPathName;
		PathRemoveFileSpec(strPath.GetBuffer(MAX_PATH));
		strPath.ReleaseBuffer();
	}
	else if (nLocationChoice == 0 || nLocationChoice == 1)
	{
		TCHAR szFolder[MAX_PATH];
		if (nLocationChoice == 0)
		{
			if (theShellAPI.pSHGetFolderPath != NULL)
			{
				if (!SUCCEEDED(theShellAPI.pSHGetFolderPath(NULL,
						CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, szFolder)))
					return false;
			}
			else if (theShellAPI.pSHGetSpecialFolderPath != NULL)
			{
				if (!theShellAPI.pSHGetSpecialFolderPath(NULL, szFolder,
						CSIDL_APPDATA | CSIDL_FLAG_CREATE, false))
					return false;
			}
			else
				return false;
		}
		else if (nLocationChoice == 1)
		{
			if (theShellAPI.pSHGetFolderPath != NULL)
			{
				if (!SUCCEEDED(theShellAPI.pSHGetFolderPath(NULL,
						CSIDL_COMMON_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, szFolder)))
					return false;
			}
			else if (theShellAPI.pSHGetSpecialFolderPath != NULL)
			{
				if (!theShellAPI.pSHGetSpecialFolderPath(NULL, szFolder,
						CSIDL_COMMON_APPDATA | CSIDL_FLAG_CREATE, false))
					return false;
			}
			else
				return false;
		}

		if (!PathAppend(szFolder, _T("WinDjView")))
			return false;
		if (!CreateDirectory(szFolder, NULL))
		{
			if (GetLastError() != ERROR_ALREADY_EXISTS)
				return false;
		}

		if (!PathAppend(szFolder, _T("Dictionaries")))
			return false;
		if (!CreateDirectory(szFolder, NULL))
		{
			if (GetLastError() != ERROR_ALREADY_EXISTS)
				return false;
		}

		strPath = szFolder;
	}
	else
	{
		if (m_appSettings.strDictLocation.IsEmpty() || PathIsRelative(m_appSettings.strDictLocation))
			return false;

		strPath = m_appSettings.strDictLocation;
	}

	PathRemoveBackslash(strPath.GetBuffer(MAX_PATH));
	strPath.ReleaseBuffer();
	strPath += _T("\\");

	CString strFileName = strOldPathName;
	PathStripPath(strFileName.GetBuffer(MAX_PATH));
	strFileName.ReleaseBuffer();

	CString strNewPathName = strPath + strFileName;

	CDjVuDoc* pPrevDoc = FindOpenDocument(strNewPathName);
	if (pPrevDoc != NULL)
	{
		m_pPendingSource = pPrevDoc->GetSource();
		m_docClosed.ResetEvent();

		pPrevDoc->GetSource()->AddObserver(this);
		pPrevDoc->OnCloseDocument();

		::WaitForSingleObject(m_docClosed, INFINITE);
	}

	if (!pSource->SaveAs(strNewPathName))
		return false;

	SaveSettings();

	// Create and initialize DictionaryInfo for the newly installed dictionary
	DictionaryInfo info;
	info.strFileName = strFileName;
	info.strPathName = strNewPathName;
	info.bInstalled = true;

	LoadDictionaryInfoFromDisk(info);

	CString strKey = info.strFileName;
	strKey.MakeLower();

	map<CString, DictionaryInfo>::iterator it = m_dictionaries.find(strKey);
	bool bHadDict = it != m_dictionaries.end();
	DictionaryInfo prevInfo;
	if (bHadDict)
		prevInfo = (*it).second;

	m_dictionaries[strKey] = info;

	// Open the new document and close the original
	CDjVuDoc* pNewDoc = (CDjVuDoc*) OpenDocumentFile(strNewPathName);
	if (pNewDoc == NULL)
	{
		if (bHadDict)
			m_dictionaries[strKey] = prevInfo;
		else
			m_dictionaries.erase(strKey);
		return false;
	}

	if (!bKeepOriginal)
	{
		// Document is closed immediately, but the rendering threads may still be running.
		// So we have to postpone the deleting until the document source is released.
		m_deleteOnRelease.insert(pSource);
		pSource->AddObserver(this);
	}

	// Remove from recents
	for (int i = 0; i < m_pRecentFileList->GetSize(); ++i)
	{
		if (AfxComparePath((*m_pRecentFileList)[i], strOldPathName))
		{
			m_pRecentFileList->Remove(i);
			break;
		}
	}

	UpdateDictVector();
	UpdateDictProperties();
	UpdateObservers(DICT_LIST_CHANGED);

	return true;
}

bool CDjViewApp::UninstallDictionary(DictionaryInfo* pInfo)
{
	CDjVuDoc* pPrevDoc = FindOpenDocument(pInfo->strPathName);
	if (pPrevDoc != NULL)
	{
		m_pPendingSource = pPrevDoc->GetSource();
		m_docClosed.ResetEvent();

		pPrevDoc->GetSource()->AddObserver(this);
		pPrevDoc->OnCloseDocument();

		::WaitForSingleObject(m_docClosed, INFINITE);
	}

	if (!MoveToTrash(pInfo->strPathName))
		return false;

	map<CString, DictionaryInfo>::iterator it;
	for (it = m_dictionaries.begin(); it != m_dictionaries.end(); ++it)
	{
		if (pInfo == &(*it).second)
		{
			m_dictionaries.erase(it);
			break;
		}
	}

	UpdateDictVector();
	UpdateDictProperties();
	UpdateObservers(DICT_LIST_CHANGED);

	return true;
}

void CDjViewApp::ReloadDictionaries()
{
	CWaitCursor wait;

	m_dictionaries.clear();
	LoadDictionaries();

	DjVuSource::UpdateDictionaries();

	UpdateDictVector();
	UpdateDictProperties();
	UpdateObservers(DICT_LIST_CHANGED);
}

void CDjViewApp::OnUpdate(const Observable* source, const Message* message)
{
	if (message->code == SOURCE_RELEASED)
	{
		DjVuSource* pSource = (DjVuSource*) source;
		set<DjVuSource*>::iterator it = m_deleteOnRelease.find(pSource);
		if (it != m_deleteOnRelease.end())
		{
			m_deleteOnRelease.erase(it);
			MoveToTrash(pSource->GetFileName());
		}

		if (pSource == m_pPendingSource)
		{
			m_pPendingSource = NULL;
			m_docClosed.SetEvent();
		}
	}
}

DictionaryInfo* CDjViewApp::GetDictionaryInfo(const CString& strPathName, bool bCheckPath)
{
	CString strFileName = strPathName;
	PathStripPath(strFileName.GetBuffer(MAX_PATH));
	strFileName.ReleaseBuffer();

	CString strKey = strFileName;
	strKey.MakeLower();

	map<CString, DictionaryInfo>::iterator it = m_dictionaries.find(strKey);
	if (it != m_dictionaries.end())
	{
		if (!bCheckPath || AfxComparePath(strPathName, (*it).second.strPathName))
			return &(*it).second;
	}

	return NULL;
}

void CDjViewApp::UpdateDictVector()
{
	m_dictsByLang.clear();

	map<pair<GUTF8String, GUTF8String>, DictsByLang> m;

	map<CString, DictionaryInfo>::iterator it;
	for (it = m_dictionaries.begin(); it != m_dictionaries.end(); ++it)
	{
		DictionaryInfo* pInfo = &(*it).second;
		m[make_pair(pInfo->strLangFromCode, pInfo->strLangToCode)].dicts.push_back(pInfo);
	}

	map<pair<GUTF8String, GUTF8String>, DictsByLang>::iterator itLangs;
	for (itLangs = m.begin(); itLangs != m.end(); ++itLangs)
		m_dictsByLang.push_back((*itLangs).second);
}

void CDjViewApp::UpdateDictProperties()
{
	map<GUTF8String, int> langMatch;
	map<GUTF8String, CString> langNames;

	langNames[""] = MakeUTF8String(LoadString(IDS_LANG_NOT_SPECIFIED));
	langMatch[""] = 0;

	map<CString, DictionaryInfo>::iterator it;
	for (it = m_dictionaries.begin(); it != m_dictionaries.end(); ++it)
	{
		DictionaryInfo& info = (*it).second;

		// Find a best match for title and language strings
		info.strTitle = FindLocalizedString(info.titleLoc, m_appSettings.nLanguage);
		if (info.strTitle.IsEmpty())
		{
			info.strTitle = info.strFileName;
			int nPos = info.strTitle.ReverseFind('.');
			if (nPos != -1)
			{
				CString strExt = info.strTitle.Mid(nPos);
				if (_tcsicmp(strExt, _T(".djvu")) == 0 || _tcsicmp(strExt, _T(".djv")) == 0)
					info.strTitle = info.strTitle.Left(nPos);
			}
		}

		// Get a best match for language names from all available dictionaries
		int nMatch;
		CString strLang = FindLocalizedString(info.langFromLoc, m_appSettings.nLanguage, &nMatch);
		map<GUTF8String, int>::iterator itLang = langMatch.find(info.strLangFromCode);
		if (itLang == langMatch.end() || (*itLang).second > nMatch)
		{
			langMatch[info.strLangFromCode] = nMatch;
			langNames[info.strLangFromCode] = strLang;
		}

		strLang = FindLocalizedString(info.langToLoc, m_appSettings.nLanguage, &nMatch);
		itLang = langMatch.find(info.strLangToCode);
		if (itLang == langMatch.end() || (*itLang).second > nMatch)
		{
			langMatch[info.strLangToCode] = nMatch;
			langNames[info.strLangToCode] = strLang;
		}
	}

	// Use consistent language names among all dictionaries
	for (it = m_dictionaries.begin(); it != m_dictionaries.end(); ++it)
	{
		DictionaryInfo& info = (*it).second;
		info.strLangFrom = langNames[info.strLangFromCode];
		info.strLangTo = langNames[info.strLangToCode];
	}

	for (size_t i = 0 ; i < m_dictsByLang.size(); ++i)
	{
		DictionaryInfo* pInfo = m_dictsByLang[i].dicts[0];
		m_dictsByLang[i].strFrom = pInfo->strLangFrom;
		m_dictsByLang[i].strTo = pInfo->strLangTo;
	}
}

CString CDjViewApp::FindLocalizedString(const vector<DictionaryInfo::LocalizedString>& loc,
		DWORD nCurrentLang, int* pnMatch)
{
	bool bFoundEng = false;
	size_t nIndexEng;

	for (size_t i = 0; i < loc.size(); ++i)
	{
		if (loc[i].first == nCurrentLang)
		{
			if (pnMatch != NULL)
				*pnMatch = 0;
			return MakeCString(loc[i].second);
		}

		if (loc[i].first == MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT))
		{
			bFoundEng = true;
			nIndexEng = i;
		}
	}

	if (bFoundEng)
	{
		if (pnMatch != NULL)
			*pnMatch = 1;
		return MakeCString(loc[nIndexEng].second);
	}
	else if (!loc.empty())
	{
		if (pnMatch != NULL)
			*pnMatch = 2;
		return MakeCString(loc[0].second);
	}
	else
	{
		if (pnMatch != NULL)
			*pnMatch = 3;
		return _T("");
	}
}

void CDjViewApp::Lookup(const CString& strLookup, DictionaryInfo* pInfo)
{
	CDjVuDoc* pDoc = OpenDocument(pInfo->strPathName, "");
	if (pDoc == NULL)
		return;

	CDjVuView* pView = pDoc->GetDjVuView();
	CMDIChild* pMDIChild = pView->GetMDIChild();
	CPageIndexWnd* pIndex = pMDIChild->GetPageIndex();
	if (pIndex == NULL)
		return;

	pMDIChild->GetNavPane()->ActivateTab(pIndex, false);
	pIndex->Lookup(strLookup);
}

void CDjViewApp::OnAppExit()
{
	// Close current main frame. This will work in both MDI and top-level mode.
	m_mainWndLock.Lock();
	CWnd* pMainWnd = m_pMainWnd;
	m_mainWndLock.Unlock();
	pMainWnd->SendMessage(WM_CLOSE);
}

int CDjViewApp::GetDocumentCount()
{
	int nCount = 0;
	for (CDjViewApp::DocIterator it; it; ++it)
		++nCount;

	return nCount;
}

BOOL CDjViewApp::SaveAllModified()
{
	if (!m_appSettings.bTopLevelDocs)
	{
		m_mainWndLock.Lock();
		CWnd* pMainWnd = m_pMainWnd;
		m_mainWndLock.Unlock();

		CMainFrame* pMainFrame = (CMainFrame*)pMainWnd;
		pMainFrame->SaveOpenTabs();
	}

	return true;
}

void CDjViewApp::DisableTopLevelWindows(set<CWnd*>& disabled)
{
	for (list<CMainFrame*>::iterator it = m_frames.begin(); it != m_frames.end(); ++it)
	{
		CMainFrame* pFrame = *it;
		if (pFrame->IsFullscreenMode())
		{
			CWnd* pFullscreenWnd = pFrame->GetFullscreenWnd();
			if (pFullscreenWnd->IsWindowEnabled())
			{
				disabled.insert(pFullscreenWnd);
				pFullscreenWnd->EnableWindow(false);
			}
		}
		if (pFrame->IsWindowEnabled())
		{
			disabled.insert(pFrame);
			pFrame->EnableWindow(false);
		}
	}
}

void CDjViewApp::EnableWindows(set<CWnd*>& disabled)
{
	for (set<CWnd*>::iterator it = disabled.begin(); it != disabled.end(); ++it)
	{
		CWnd* pWnd = *it;
		pWnd->EnableWindow();
	}
}

int CDjViewApp::DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDHelp)
{
	return DoMessageBox(lpszPrompt, nType, nIDHelp, MessageBoxOptions());
}

int CDjViewApp::DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDHelp, const MessageBoxOptions& mbo)
{
	set<CWnd*> disabled;
	DisableTopLevelWindows(disabled);

	ASSERT(m_pMBWnd == NULL);
	m_pMBWnd = new CMyMessageBox();

	ASSERT(m_hMBHook == NULL);
	m_hMBHook = SetWindowsHookEx(WH_CBT, &MBHookProc, NULL, GetCurrentThreadId());
	m_nMBType = nType;
	m_mbo = mbo;

	m_strMBPrompt = lpszPrompt;
	if (IsWinVistaOrLater() && !m_mbo.bVerbatim)
	{
		// Replace single \n characters with a space on Vista (it has a more
		// sane maximum width of a message box than earlier versions)
		for (int i = 0; i < m_strMBPrompt.GetLength(); ++i)
		{
			if (m_strMBPrompt[i] == '\n' && m_strMBPrompt[i + 1] != '\n' &&
					(i == 0 || m_strMBPrompt[i - 1] != '\n'))
			{
				m_strMBPrompt.SetAt(i, ' ');
				++i;
			}
		}
	}

	// Add the check box text to make Windows calculate dialog size for us.
	// The prompt will be replaced by the original prompt, and the check
	// box will be positioned at the bottom of the prompt area.
	CString strNewPrompt = m_strMBPrompt;
	if (mbo.pCheckValue != NULL)
		strNewPrompt += _T("\n\n") + m_mbo.strCheckBox;

	int nResult = CWinApp::DoMessageBox(strNewPrompt, nType, nIDHelp);

	::UnhookWindowsHookEx(m_hMBHook);
	m_hMBHook = NULL;

	delete m_pMBWnd;
	m_pMBWnd = NULL;

	m_mbo.pCheckValue = NULL;

	EnableWindows(disabled);
	return nResult;
}

LRESULT CALLBACK CDjViewApp::MBHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	ASSERT(theApp.m_pMBWnd != NULL);
	if (nCode == HCBT_ACTIVATE && theApp.m_pMBWnd->m_hWnd == NULL)
	{
		HWND hwndMessageBox = (HWND) wParam;
		ASSERT(hwndMessageBox != NULL);
		theApp.m_pMBWnd->SubclassWindow(hwndMessageBox);

		HWND hwndMessage = GetDlgItem(hwndMessageBox, 0xFFFF);
		if (hwndMessage != NULL)
		{
			CString strMessage = theApp.m_strMBPrompt;
			strMessage.Replace(_T("\n"), _T("\r\n"));

			CRect rc, rcClient;
			::GetWindowRect(hwndMessage, rc);
			::GetClientRect(hwndMessageBox, rcClient);
			POINT pt;
			pt.x = rc.left;
			pt.y = rc.top;
			::ScreenToClient(hwndMessageBox, &pt);

			// Create the alternate EDIT window
			HWND hwndEdit = ::CreateWindowEx(0, _T("edit"), strMessage,
					ES_READONLY | ES_MULTILINE | WS_CHILD,
					pt.x, pt.y, rc.Width() + 6, rc.Height(),
					hwndMessageBox, (HMENU) 0xFFFE, NULL, NULL);
			theApp.m_pMBWnd->m_hwndEdit = hwndEdit;

			HFONT hFont = (HFONT) ::SendMessage(hwndMessage, WM_GETFONT, 0, 0);
			::SendMessage(hwndEdit, WM_SETFONT, (WPARAM) hFont, 1);
			::SendMessage(hwndEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, 0);

			::ShowWindow(hwndEdit, SW_SHOW);
			::ShowWindow(hwndMessage, SW_HIDE);

			if (theApp.m_mbo.pCheckValue != NULL)
			{
				CRect rcCheckBox(0, 0, 1, 9);  // dialog units
				::MapDialogRect(hwndMessageBox, &rcCheckBox);

				HWND hwndCheckBox = ::CreateWindowEx(0, _T("button"),
					theApp.m_mbo.strCheckBox, WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX,
					pt.x, pt.y + rc.Height() - rcCheckBox.Height(),
					rc.Width() + 6, rcCheckBox.Height(),
					hwndMessageBox, (HMENU) 0xFFF0, NULL, NULL);
				theApp.m_pMBWnd->m_hwndCheck = hwndCheckBox;

				::SendMessage(hwndCheckBox, WM_SETFONT, (WPARAM) hFont, 1);
				::MoveWindow(hwndEdit, pt.x, pt.y, rc.Width() + 6,
						rc.Height() - rcCheckBox.Height(), true);

				::SendMessage(hwndCheckBox, BM_SETCHECK, *theApp.m_mbo.pCheckValue, 0);
				::ShowWindow(hwndCheckBox, SW_SHOW);
			}
		}

		int nType = (theApp.m_nMBType & MB_TYPEMASK);
		CString strCaptions = theApp.m_mbo.strCaptions;

		CString strOk, strCancel, strAbort, strRetry, strIgnore, strYes, strNo;
		switch (nType)
		{
		case MB_OK:
			AfxExtractSubString(strOk, strCaptions, 0);
			break;

		case MB_OKCANCEL:
			AfxExtractSubString(strOk, strCaptions, 0);
			AfxExtractSubString(strCancel, strCaptions, 1);
			break;

		case MB_ABORTRETRYIGNORE:
			AfxExtractSubString(strAbort, strCaptions, 0);
			AfxExtractSubString(strRetry, strCaptions, 1);
			AfxExtractSubString(strIgnore, strCaptions, 2);
			break;

		case MB_YESNO:
			AfxExtractSubString(strYes, strCaptions, 0);
			AfxExtractSubString(strNo, strCaptions, 1);
			break;

		case MB_YESNOCANCEL:
			AfxExtractSubString(strYes, strCaptions, 0);
			AfxExtractSubString(strNo, strCaptions, 1);
			AfxExtractSubString(strCancel, strCaptions, 2);
			break;

		case MB_RETRYCANCEL:
			AfxExtractSubString(strRetry, strCaptions, 0);
			AfxExtractSubString(strCancel, strCaptions, 1);
			break;
		}

		if (!strOk.IsEmpty() && GetDlgItem(hwndMessageBox, IDOK) != NULL)
			SetDlgItemText(hwndMessageBox, IDOK, strOk);
		if (!strCancel.IsEmpty() && GetDlgItem(hwndMessageBox, IDCANCEL) != NULL)
			SetDlgItemText(hwndMessageBox, IDCANCEL, strCancel);
		if (!strAbort.IsEmpty() && GetDlgItem(hwndMessageBox, IDABORT) != NULL)
			SetDlgItemText(hwndMessageBox, IDABORT, strAbort);
		if (!strRetry.IsEmpty() && GetDlgItem(hwndMessageBox, IDRETRY) != NULL)
			SetDlgItemText(hwndMessageBox, IDRETRY, strRetry);
		if (!strIgnore.IsEmpty() && GetDlgItem(hwndMessageBox, IDIGNORE) != NULL)
			SetDlgItemText(hwndMessageBox, IDIGNORE, strIgnore);
		if (!strYes.IsEmpty() && GetDlgItem(hwndMessageBox, IDYES) != NULL)
			SetDlgItemText(hwndMessageBox, IDYES, strYes);
		if (!strNo.IsEmpty() && GetDlgItem(hwndMessageBox, IDNO) != NULL)
			SetDlgItemText(hwndMessageBox, IDNO, strNo);
	}

	return ::CallNextHookEx(theApp.m_hMBHook, nCode, wParam, lParam);
}

BOOL CDjViewApp::CMyMessageBox::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (message == WM_COMMAND)
	{
		int nCode = HIWORD(wParam);
		UINT nID = LOWORD(wParam);
		HWND hWndCtrl = (HWND) lParam;
		if (nCode == BN_CLICKED && nID == 0xFFF0 && hWndCtrl == m_hwndCheck)
		{
			ASSERT(theApp.m_mbo.pCheckValue != NULL);
			*theApp.m_mbo.pCheckValue = (::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0) != 0);
		}
	}
	else if (message == WM_SETCURSOR)
	{
		UINT nHitTest = LOWORD(lParam);
		HWND hWndCtrl = (HWND) wParam;
		if (nHitTest == HTCLIENT && hWndCtrl == m_hwndEdit)
		{
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			if (pResult != NULL)
				*pResult = 1;
			return true;
		}
	}

	return CWnd::OnWndMsg(message, wParam, lParam, pResult);
}

CFindDlg* CDjViewApp::GetFindDlg(bool bCreate)
{
	if (m_pFindDlg == NULL && bCreate)
	{
		m_mainWndLock.Lock();
		CWnd* pMainWnd = m_pMainWnd;
		m_mainWndLock.Unlock();

		m_pFindDlg = new CFindDlg();
		m_pFindDlg->Create(IDD_FIND, pMainWnd);
		m_pFindDlg->CenterWindow();
	}

	return m_pFindDlg;
}

void CDjViewApp::UpdateFindDlg(CWnd* pNewParent)
{
	if (m_pFindDlg == NULL)
		return;

	m_pFindDlg->UpdateData();
	m_appSettings.strFind = m_pFindDlg->m_strFind;
	m_appSettings.bMatchCase = !!m_pFindDlg->m_bMatchCase;

	bool bVisible = !!m_pFindDlg->IsWindowVisible();

	CRect rcFindDlg;
	m_pFindDlg->GetWindowRect(rcFindDlg);

	m_mainWndLock.Lock();
	CWnd* pMainWnd = m_pMainWnd;
	m_mainWndLock.Unlock();

	CMainFrame* pOldFrame = (CMainFrame*)m_pFindDlg->GetParent();
	ASSERT(pOldFrame != NULL);
	CMainFrame* pMainFrame = (CMainFrame*)pMainWnd;

	pOldFrame->m_bDontActivate = true;
	pMainFrame->m_bDontActivate = true;

	CFindDlg* pOldFindDlg = m_pFindDlg;

	m_pFindDlg = new CFindDlg;
	m_pFindDlg->Create(IDD_FIND, pNewParent ? pNewParent : pMainWnd);

	CRect rcNewFindDlg;
	m_pFindDlg->GetWindowRect(rcNewFindDlg);

	m_pFindDlg->MoveWindow(rcFindDlg.left, rcFindDlg.top,
		rcNewFindDlg.Width(), rcNewFindDlg.Height());
	if (bVisible)
		m_pFindDlg->ShowWindow(SW_SHOWNOACTIVATE);

	pOldFindDlg->DestroyWindow();

	pOldFrame->m_bDontActivate = false;
	pMainFrame->m_bDontActivate = false;
	pMainFrame->SetFocus();
	pMainFrame->SetForegroundWindow();
}

CString CDjViewApp::DownloadLastVersionString()
{
	CString strVersion;
	CFile* pFile = NULL;

	try
	{
		CInternetSession session;
		session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 10000);

		pFile = session.OpenURL(LoadString(IDS_VERSION_URL), 1,
			INTERNET_FLAG_TRANSFER_ASCII | INTERNET_FLAG_RELOAD);
		if (pFile == NULL)
			AfxThrowInternetException(1);

		CHAR szBuffer[1024];
		int nRead = pFile->Read(szBuffer, 1023);
		szBuffer[nRead] = '\0';

		strVersion = szBuffer;
		strVersion.TrimLeft();
		strVersion.TrimRight();

		pFile->Close();
		delete pFile;
		pFile = NULL;
	}
	catch (CException* e)
	{
		e->Delete();
		strVersion.Empty();
	}

	if (pFile != NULL)
		delete pFile;

	if (strVersion.Find('<') != -1 || strVersion.GetLength() > 16)
		strVersion.Empty();

	return strVersion;
}

unsigned int __stdcall CDjViewApp::CheckUpdateThreadProc(void* pvData)
{
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	theApp.m_strNewVersion = DownloadLastVersionString();

	theApp.m_mainWndLock.Lock();
	if (theApp.m_pMainWnd != NULL)
	{
		theApp.GetAppSettings()->nLastUpdateTime = time(NULL);
		if (!theApp.m_strNewVersion.IsEmpty()
				&& CompareVersions(theApp.m_strNewVersion, CURRENT_VERSION) > 0)
		{
			theApp.m_pMainWnd->PostMessage(WM_NOTIFY_NEW_VERSION);
		}
	}
	theApp.m_mainWndLock.Unlock();

	theApp.ThreadTerminated();
	return 0;
}
