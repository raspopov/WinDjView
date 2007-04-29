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
#include "PageIndexWnd.h"
#include "NavPane.h"
#include "XMLParser.h"

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
const TCHAR* s_pszScaleMethod = _T("scale-method");
const TCHAR* s_pszInvertColors = _T("invert");
const TCHAR* s_pszUnits = _T("units");

const TCHAR* s_pszGlobalSection = _T("Settings");
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
const TCHAR* s_pszMatchCase = _T("match-case");
const TCHAR* s_pszCurrentLang = _T("cur-lang");
const TCHAR* s_pszCurrentDict = _T("cur-dict");
const TCHAR* s_pszDictLocation = _T("dict-location");
const TCHAR* s_pszDictChoice = _T("dict-choice");

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
	: m_bInitialized(false), m_pDjVuTemplate(NULL), m_nThreadCount(0), m_hHook(NULL),
	  m_pPendingSource(NULL), m_bShiftPressed(false), m_bControlPressed(false),
	  m_nLangIndex(0), m_nTimerID(0)
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

	CURRENT_VERSION.LoadString(IDS_CURRENT_VERSION);

	LoadStdProfileSettings(10);  // Load recently open documents
	LoadSettings();
	LoadLanguages();
	LoadDictionaries();

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
		return false;
	m_pMainWnd = pMainFrame;
	pMainFrame->SetRedraw(false);

	pMainFrame->UpdateToolbars();

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

	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	SetStartupLanguage();

	pMainFrame->SetRedraw(true);
	pMainFrame->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

	// Dispatch commands specified on the command line.  Will return false if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return false;

	m_hHook = ::SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, NULL, ::GetCurrentThreadId());
	m_nTimerID = ::SetTimer(NULL, 1, 100, TimerProc);

	m_bInitialized = true;

	if (m_appSettings.strVersion != CURRENT_VERSION)
		OnAppAbout();

	ThreadStarted();
	return true;
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
	m_appSettings.nWindowPosX = GetProfileInt(s_pszDisplaySection, s_pszPosX, m_appSettings.nWindowPosX);
	m_appSettings.nWindowPosY = GetProfileInt(s_pszDisplaySection, s_pszPosY, m_appSettings.nWindowPosY);
	m_appSettings.nWindowWidth = GetProfileInt(s_pszDisplaySection, s_pszWidth, m_appSettings.nWindowWidth);
	m_appSettings.nWindowHeight = GetProfileInt(s_pszDisplaySection, s_pszHeight, m_appSettings.nWindowHeight);
	m_appSettings.bWindowMaximized = !!GetProfileInt(s_pszDisplaySection, s_pszMaximized, m_appSettings.bWindowMaximized);
	m_appSettings.bChildMaximized = !!GetProfileInt(s_pszDisplaySection, s_pszChildMaximized, m_appSettings.bChildMaximized);
	m_appSettings.bToolbar = !!GetProfileInt(s_pszDisplaySection, s_pszToolbar, m_appSettings.bToolbar);
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

	m_appSettings.bRestoreAssocs = !!GetProfileInt(s_pszGlobalSection, s_pszRestoreAssocs, m_appSettings.bRestoreAssocs);
	m_appSettings.bGenAllThumbnails = !!GetProfileInt(s_pszGlobalSection, s_pszGenAllThumbnails, m_appSettings.bGenAllThumbnails);
	m_appSettings.bFullscreenClicks = !!GetProfileInt(s_pszGlobalSection, s_pszFullscreenClicks, m_appSettings.bFullscreenClicks);
	m_appSettings.bFullscreenHideScroll = !!GetProfileInt(s_pszGlobalSection, s_pszFullscreenHideScroll, m_appSettings.bFullscreenHideScroll);
	m_appSettings.bWarnCloseMultiple = !!GetProfileInt(s_pszGlobalSection, s_pszWarnCloseMultiple, m_appSettings.bWarnCloseMultiple);
	m_appSettings.bInvertWheelZoom = !!GetProfileInt(s_pszGlobalSection, s_pszInvertWheelZoom, m_appSettings.bInvertWheelZoom);
	m_appSettings.bCloseOnEsc = !!GetProfileInt(s_pszGlobalSection, s_pszCloseOnEsc, m_appSettings.bCloseOnEsc);
	m_appSettings.bWrapLongBookmarks = !!GetProfileInt(s_pszGlobalSection, s_pszWrapLongBookmarks, m_appSettings.bWrapLongBookmarks);
	m_appSettings.bRestoreView = !!GetProfileInt(s_pszGlobalSection, s_pszRestoreView, m_appSettings.bRestoreView);
	m_appSettings.nLanguage = GetProfileInt(s_pszGlobalSection, s_pszLanguage, m_appSettings.nLanguage);
	m_appSettings.bMatchCase = !!GetProfileInt(s_pszGlobalSection, s_pszMatchCase, m_appSettings.bMatchCase);
	m_appSettings.strVersion = GetProfileString(s_pszGlobalSection, s_pszVersion, CURRENT_VERSION);
	m_appSettings.nCurLang = GetProfileInt(s_pszGlobalSection, s_pszCurrentLang, m_appSettings.nCurLang);
	m_appSettings.nCurDict = GetProfileInt(s_pszGlobalSection, s_pszCurrentDict, m_appSettings.nCurDict);
	m_appSettings.strDictLocation = GetProfileString(s_pszGlobalSection, s_pszDictLocation, m_appSettings.strDictLocation);
	m_appSettings.nDictChoice = GetProfileInt(s_pszGlobalSection, s_pszDictChoice, m_appSettings.nDictChoice);

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

	m_displaySettings.nScaleMethod = GetProfileInt(s_pszDisplaySection, s_pszScaleMethod, m_displaySettings.nScaleMethod);
	m_displaySettings.bInvertColors = !!GetProfileInt(s_pszDisplaySection, s_pszInvertColors, m_displaySettings.bInvertColors);
	m_displaySettings.bAdjustDisplay = !!GetProfileInt(s_pszDisplaySection, s_pszAdjustDisplay, m_displaySettings.bAdjustDisplay);
	m_displaySettings.fGamma = GetProfileDouble(s_pszDisplaySection, s_pszGamma, m_displaySettings.fGamma);
	m_displaySettings.nBrightness = GetProfileInt(s_pszDisplaySection, s_pszBrightness, m_displaySettings.nBrightness);
	m_displaySettings.nContrast = GetProfileInt(s_pszDisplaySection, s_pszContrast, m_displaySettings.nContrast);
	m_displaySettings.Fix();

	m_appSettings.nUnits = GetProfileInt(s_pszDisplaySection, s_pszUnits, m_appSettings.nUnits);
	if (m_appSettings.nUnits < CAppSettings::Centimeters || m_appSettings.nUnits > CAppSettings::Inches)
		m_appSettings.nUnits = CAppSettings::Centimeters;

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

bool CDjViewApp::LoadDocSettings(const CString& strKey, DocSettings* pSettings)
{
	GUTF8String strSettings;
	if (!GetProfileCompressed(s_pszDocumentsSection + CString(_T("\\")) + strKey, s_pszSettings, strSettings))
		return false;

	stringstream sin((const char*) strSettings);
	XMLParser parser;
	if (parser.Parse(sin))
		pSettings->Load(*parser.GetRoot());

	return true;
}

void CDjViewApp::SaveSettings()
{
	WriteProfileInt(s_pszDisplaySection, s_pszPosX, m_appSettings.nWindowPosX);
	WriteProfileInt(s_pszDisplaySection, s_pszPosY, m_appSettings.nWindowPosY);
	WriteProfileInt(s_pszDisplaySection, s_pszWidth, m_appSettings.nWindowWidth);
	WriteProfileInt(s_pszDisplaySection, s_pszHeight, m_appSettings.nWindowHeight);
	WriteProfileInt(s_pszDisplaySection, s_pszMaximized, m_appSettings.bWindowMaximized);
	WriteProfileInt(s_pszDisplaySection, s_pszChildMaximized, m_appSettings.bChildMaximized);
	WriteProfileInt(s_pszDisplaySection, s_pszToolbar, m_appSettings.bToolbar);
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

	WriteProfileInt(s_pszGlobalSection, s_pszRestoreAssocs, m_appSettings.bRestoreAssocs);
	WriteProfileInt(s_pszGlobalSection, s_pszGenAllThumbnails, m_appSettings.bGenAllThumbnails);
	WriteProfileInt(s_pszGlobalSection, s_pszFullscreenClicks, m_appSettings.bFullscreenClicks);
	WriteProfileInt(s_pszGlobalSection, s_pszFullscreenHideScroll, m_appSettings.bFullscreenHideScroll);
	WriteProfileInt(s_pszGlobalSection, s_pszWarnCloseMultiple, m_appSettings.bWarnCloseMultiple);
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

	WriteProfileInt(s_pszDisplaySection, s_pszScaleMethod, m_displaySettings.nScaleMethod);
	WriteProfileInt(s_pszDisplaySection, s_pszInvertColors, m_displaySettings.bInvertColors);
	WriteProfileInt(s_pszDisplaySection, s_pszAdjustDisplay, m_displaySettings.bAdjustDisplay);
	WriteProfileDouble(s_pszDisplaySection, s_pszGamma, m_displaySettings.fGamma);
	WriteProfileInt(s_pszDisplaySection, s_pszBrightness, m_displaySettings.nBrightness);
	WriteProfileInt(s_pszDisplaySection, s_pszContrast, m_displaySettings.nContrast);
	WriteProfileInt(s_pszDisplaySection, s_pszUnits, m_appSettings.nUnits);

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

		const DocSettings& settings = (*it).second;
		GUTF8String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + settings.GetXML();

		WriteProfileCompressed(s_pszDocumentsSection + CString(_T("\\")) + strKey, s_pszSettings, xml);
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
		int nRead;
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
	SaveSettings();

	::KillTimer(NULL, m_nTimerID);
	::UnhookWindowsHookEx(m_hHook);

	ThreadTerminated();
	::WaitForSingleObject(m_terminated, INFINITE);

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

	cboFind.SetCurSel(-1);
	int nItem = cboFind.FindStringExact(-1, strText);
	if (nItem != CB_ERR)
		cboFind.DeleteItem(nItem);
	else if (cboFind.GetCount() >= CAppSettings::HistorySize)
		cboFind.DeleteItem(cboFind.GetCount() - 1);

	list<CString>::iterator it = find(m_appSettings.searchHistory.begin(),
			m_appSettings.searchHistory.end(), strText);
	if (it != m_appSettings.searchHistory.end())
		m_appSettings.searchHistory.erase(it);

	if (m_appSettings.searchHistory.size() >= CAppSettings::HistorySize)
		m_appSettings.searchHistory.pop_back();

	COMBOBOXEXITEM item;
	item.mask = CBEIF_TEXT;
	item.iItem = 0;
	item.pszText = strText.GetBuffer(0);
	cboFind.InsertItem(&item);
	cboFind.SetCurSel(0);

	m_appSettings.searchHistory.push_front(strText);
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

void CALLBACK CDjViewApp::TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
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

	GetModuleFileName(m_hInstance, strPathName.GetBuffer(MAX_PATH), MAX_PATH);
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

				LONG lSize = MAX_PATH * 2;
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
	}
}

CDjVuDoc* CDjViewApp::OpenDocument(LPCTSTR lpszPathName, const GUTF8String& strPage, bool bAddToHistory)
{
	bool bAlreadyOpen = false;
	CDjVuDoc* pDoc = (CDjVuDoc*) ((CMyDocManager*) m_pDocManager)->OpenDocumentFile(
			lpszPathName, false, &bAlreadyOpen);
	if (pDoc == NULL)
		return NULL;

	CDjVuView* pView = pDoc->GetDjVuView();
	CFrameWnd* pFrame = pView->GetParentFrame();
	pFrame->ActivateFrame();

	int nAddToHistory = (bAddToHistory ? CDjVuView::AddTarget : 0);
	if (bAddToHistory && bAlreadyOpen)
		nAddToHistory |= CDjVuView::AddSource;

	if (strPage.length() > 0)
		pView->GoToURL(strPage, nAddToHistory);
	if (bAddToHistory)
		pView->GetMainFrame()->AddToHistory(pView);

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
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			CString strPathName = strPath + fd.cFileName;

			DWORD dwHandle;
			DWORD dwSize = ::GetFileVersionInfoSize(strPathName, &dwHandle);
			if (dwSize <= 0)
				continue;

			vector<BYTE> versionInfo(dwSize);
			LPBYTE pVersionInfo = &versionInfo[0];
			if (::GetFileVersionInfo(strPathName, dwHandle, dwSize, pVersionInfo) == 0)
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

	m_appSettings.bLocalized = (info.hInstance != AfxGetInstanceHandle());
	m_appSettings.nLanguage = info.nLanguage;

	if (nLangIndex == m_nLangIndex)
		return;

	m_nLangIndex = nLangIndex;
	AfxSetResourceHandle(info.hInstance);

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
	UpdateDictProperties();

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
	for (size_t i = 0; i < m_languages.size(); ++i)
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

		CDjVuView* pView = pDoc->GetDjVuView();
		pView->GetMainFrame()->AddToHistory(pView);
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
#ifdef UNICODE
		pSHGetFolderPath = (pfnSHGetFolderPath) ::GetProcAddress(hShell32, "SHGetFolderPathW");
		pSHGetSpecialFolderPath = (pfnSHGetSpecialFolderPath) ::GetProcAddress(hShell32, "SHGetSpecialFolderPathW");
#else
		pSHGetFolderPath = (pfnSHGetFolderPath) ::GetProcAddress(hShell32, "SHGetFolderPathA");
		pSHGetSpecialFolderPath = (pfnSHGetSpecialFolderPath) ::GetProcAddress(hShell32, "SHGetSpecialFolderPathA");
#endif

		if (pSHGetFolderPath == NULL)
		{
			hSHFolder = ::LoadLibrary(_T("shfolder.dll"));
			if (hSHFolder != NULL)
			{
#ifdef UNICODE
				pSHGetFolderPath = (pfnSHGetFolderPath) ::GetProcAddress(hSHFolder, "SHGetFolderPathW");
#else
				pSHGetFolderPath = (pfnSHGetFolderPath) ::GetProcAddress(hSHFolder, "SHGetFolderPathA");
#endif
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
			DWORD dwSize = MAX_PATH;
			FILETIME ftWrite;
			for (int nKey = static_cast<int>(nSubKeys) - 1; nKey >= 0; --nKey)
			{
				if (::RegEnumKeyEx(hSecKey, nKey, szKey, &dwSize, 0, NULL, NULL, &ftWrite) != ERROR_SUCCESS)
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
	if (hFile == NULL)
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

	langNames[""] = MakeUTF8String(LoadString(IDS_NOT_SPECIFIED));
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
	CChildFrame* pFrame = (CChildFrame*) pView->GetParentFrame();
	CPageIndexWnd* pIndex = pFrame->GetPageIndex();
	if (pIndex == NULL)
		return;

	pFrame->GetNavPane()->ActivateTab(pIndex, false);
	pIndex->Lookup(strLookup);
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
	CString strText = FormatString(_T("%u%s"), value, CString(pszSuffix));
	DDX_Text(pDX, nIDC, strText);

	if (pDX->m_bSaveAndValidate)
	{
		if (_stscanf(strText, _T("%u"), &value) != 1)
			value = def;
	}
}

void CreateSystemDialogFont(CFont& font)
{
	LOGFONT lf;

	HGDIOBJ hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	::GetObject(hFont, sizeof(LOGFONT), &lf);

	if (IsWin2kOrLater())
		_tcscpy(lf.lfFaceName, _T("MS Shell Dlg 2"));

	font.CreateFontIndirect(&lf);
}

void CreateSystemIconFont(CFont& font)
{
	LOGFONT lf;

	if (IsThemed())
	{
		LOGFONTW lfw;
		HRESULT hr = XPGetThemeSysFont(NULL, TMT_ICONTITLEFONT, &lfw);
		if (SUCCEEDED(hr))
		{
#ifdef UNICODE
			memcpy(&lf, &lfw, sizeof(LOGFONT));
#else
			memcpy(&lf, &lfw, (char*)&lfw.lfFaceName - (char*)&lfw);
			_tcscpy(lf.lfFaceName, CString(lfw.lfFaceName));
#endif
			font.CreateFontIndirect(&lf);
			return;
		}
	}

	if (!SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0))
	{
		CreateSystemDialogFont(font);
		return;
	}

	font.CreateFontIndirect(&lf);
}

void CreateSystemMenuFont(CFont& font)
{
	LOGFONT lf;

	if (IsThemed())
	{
		LOGFONTW lfw;
		HRESULT hr = XPGetThemeSysFont(NULL, TMT_MENUFONT, &lfw);
		if (SUCCEEDED(hr))
		{
#ifdef UNICODE
			memcpy(&lf, &lfw, sizeof(LOGFONT));
#else
			memcpy(&lf, &lfw, (char*)&lfw.lfFaceName - (char*)&lfw);
			_tcscpy(lf.lfFaceName, CString(lfw.lfFaceName));
#endif
			font.CreateFontIndirect(&lf);
			return;
		}
	}

	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	if (!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0))
	{
		CreateSystemDialogFont(font);
		return;
	}

	font.CreateFontIndirect(&ncm.lfMenuFont);
}


// MonitorAPI

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
