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
#include "MainFrm.h"

#include "ChildFrm.h"
#include "DjVuDoc.h"
#include "DjVuView.h"
#include "AppSettings.h"
#include "SettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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
const TCHAR* s_pszNavCollapsed = _T("nav-collapsed");
const TCHAR* s_pszNavWidth = _T("nav-width");

const TCHAR* s_pszGlobalSettings = _T("Settings");
const TCHAR* s_pszRestoreAssocs = _T("assocs");

const TCHAR* s_pszPrintSettings = _T("Print");
const TCHAR* s_pszMarginLeft = _T("m-left");
const TCHAR* s_pszMarginTop = _T("m-top");
const TCHAR* s_pszMarginRight = _T("m-right");
const TCHAR* s_pszMarginBottom = _T("m-bottom");
const TCHAR* s_pszPosLeft = _T("left");
const TCHAR* s_pszPosTop = _T("top");
const TCHAR* s_pszCenterImage = _T("center");


// CDjViewApp

BEGIN_MESSAGE_MAP(CDjViewApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	ON_COMMAND(ID_FILE_SETTINGS, OnFileSettings)
END_MESSAGE_MAP()


// CDjViewApp construction

CDjViewApp::CDjViewApp()
{
}


// The one and only CDjViewApp object

CDjViewApp theApp;

// CDjViewApp initialization

BOOL CDjViewApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	SetRegistryKey(_T("Andrew Zhezherun"));

	LoadStdProfileSettings(10);  // Load standard INI file options (including MRU)
	LoadSettings();

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;

#ifdef ELIBRA_READER
	pDocTemplate = new CMultiDocTemplate(IDR_ElibTYPE,
		RUNTIME_CLASS(CDjVuDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CDjVuView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
#endif

	pDocTemplate = new CMultiDocTemplate(IDR_DjVuTYPE,
		RUNTIME_CLASS(CDjVuDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CDjVuView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd
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
	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();
	return TRUE;
}

void CDjViewApp::EnableShellOpen()
{
	ASSERT(m_atomApp == NULL && m_atomSystemTopic == NULL); // do once

	m_atomApp = ::GlobalAddAtom(_T("WinDjView"));
	m_atomSystemTopic = ::GlobalAddAtom(_T("system"));
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
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_LINK, m_weblink);
#ifndef ELIBRA_READER
	DDX_Control(pDX, IDC_STATIC_LIB_LINK, m_weblinkLibrary);
#endif
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
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
#ifndef ELIBRA_READER
			|| pWnd->GetSafeHwnd() == m_weblinkLibrary.m_hWnd
#endif
		)
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

#ifndef ELIBRA_READER
	CRect rcWeblinkLibrary;
	m_weblinkLibrary.GetWindowRect(rcWeblinkLibrary);

	if (rcWeblinkLibrary.PtInRect(ptCursor))
	{
		::SetCursor(hCursor);
		return true;
	}
#endif

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CAboutDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rcWeblink;
	m_weblink.GetWindowRect(rcWeblink);
	ScreenToClient(rcWeblink);

	if (rcWeblink.PtInRect(point))
	{
#ifndef ELIBRA_READER
		::ShellExecute(NULL, "open", "http://sourceforge.net/projects/windjview",
			NULL, NULL, SW_SHOWNORMAL);
#else
		::ShellExecute(NULL, "open", "http://www.starpath.com/elibra",
			NULL, NULL, SW_SHOWNORMAL);
#endif
		return;
	}

#ifndef ELIBRA_READER
	CRect rcWeblinkLibrary;
	m_weblinkLibrary.GetWindowRect(rcWeblinkLibrary);
	ScreenToClient(rcWeblinkLibrary);

	if (rcWeblinkLibrary.PtInRect(point))
	{
		::ShellExecute(NULL, "open", "http://djvu.sourceforge.net",
			NULL, NULL, SW_SHOWNORMAL);
		return;
	}
#endif

	CDialog::OnLButtonDown(nFlags, point);
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

#ifndef ELIBRA_READER
	m_weblinkLibrary.SetFont(&m_font);
#endif

	return true;
}


// CDjViewApp message handlers

BOOL CDjViewApp::WriteProfileDouble(LPCTSTR pszSection, LPCTSTR pszEntry, double fValue)
{
	return WriteProfileString(pszSection, pszEntry, FormatDouble(fValue));
}

double CDjViewApp::GetProfileDouble(LPCTSTR pszSection, LPCTSTR pszEntry, double fDefault)
{
	CString strValue = GetProfileString(pszSection, pszEntry);

	double fValue;
	if (_stscanf(strValue, _T("%lf"), &fValue) != 1)
		return fDefault;

	return fValue;
}

void CDjViewApp::LoadSettings()
{
	CAppSettings::nWindowPosX = GetProfileInt(s_pszDisplaySettings, s_pszPosX, 50);
	CAppSettings::nWindowPosY = GetProfileInt(s_pszDisplaySettings, s_pszPosY, 50);
	CAppSettings::nWindowWidth = GetProfileInt(s_pszDisplaySettings, s_pszWidth, 700);
	CAppSettings::nWindowHeight = GetProfileInt(s_pszDisplaySettings, s_pszHeight, 500);
	CAppSettings::bWindowMaximized = !!GetProfileInt(s_pszDisplaySettings, s_pszMaximized, 0);
	CAppSettings::bChildMaximized = !!GetProfileInt(s_pszDisplaySettings, s_pszChildMaximized, 0);
	CAppSettings::bToolbar = !!GetProfileInt(s_pszDisplaySettings, s_pszToolbar, 1);
	CAppSettings::bStatusBar = !!GetProfileInt(s_pszDisplaySettings, s_pszStatusBar, 1);
	CAppSettings::nDefaultZoomType = GetProfileInt(s_pszDisplaySettings, s_pszZoom, 0);
	CAppSettings::fDefaultZoom = GetProfileDouble(s_pszDisplaySettings, s_pszZoomPercent, 100.0);
	CAppSettings::nDefaultLayout = GetProfileInt(s_pszDisplaySettings, s_pszLayout, 0);
	CAppSettings::bNavPaneCollapsed = !!GetProfileInt(s_pszDisplaySettings, s_pszNavCollapsed, 0);
	CAppSettings::nNavPaneWidth = GetProfileInt(s_pszDisplaySettings, s_pszNavWidth, 200);

	CAppSettings::bRestoreAssocs = !!GetProfileInt(s_pszGlobalSettings, s_pszRestoreAssocs, 0);

	CPrintSettings& ps = CAppSettings::printSettings;
	ps.fMarginLeft = GetProfileDouble(s_pszPrintSettings, s_pszMarginLeft, 0.0);
	ps.fMarginTop = GetProfileDouble(s_pszPrintSettings, s_pszMarginTop, 0.0);
	ps.fMarginRight = GetProfileDouble(s_pszPrintSettings, s_pszMarginRight, 0.0);
	ps.fMarginBottom = GetProfileDouble(s_pszPrintSettings, s_pszMarginBottom, 0.0);
	ps.fPosLeft = GetProfileDouble(s_pszPrintSettings, s_pszPosLeft, 0.0);
	ps.fPosTop = GetProfileDouble(s_pszPrintSettings, s_pszPosTop, 0.0);
	ps.bCenterImage = !!GetProfileInt(s_pszPrintSettings, s_pszCenterImage, 0);
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
	WriteProfileInt(s_pszDisplaySettings, s_pszNavCollapsed, CAppSettings::bNavPaneCollapsed);
	WriteProfileInt(s_pszDisplaySettings, s_pszNavWidth, CAppSettings::nNavPaneWidth);

	WriteProfileInt(s_pszGlobalSettings, s_pszRestoreAssocs, CAppSettings::bRestoreAssocs);

	CPrintSettings& ps = CAppSettings::printSettings;
	WriteProfileDouble(s_pszPrintSettings, s_pszMarginLeft, ps.fMarginLeft);
	WriteProfileDouble(s_pszPrintSettings, s_pszMarginTop, ps.fMarginTop);
	WriteProfileDouble(s_pszPrintSettings, s_pszMarginRight, ps.fMarginRight);
	WriteProfileDouble(s_pszPrintSettings, s_pszMarginBottom, ps.fMarginBottom);
	WriteProfileDouble(s_pszPrintSettings, s_pszPosLeft, ps.fPosLeft);
	WriteProfileDouble(s_pszPrintSettings, s_pszPosTop, ps.fPosTop);
	WriteProfileInt(s_pszPrintSettings, s_pszCenterImage, ps.bCenterImage);
}

int CDjViewApp::ExitInstance()
{
	SaveSettings();

	return CWinApp::ExitInstance();
}

bool SetRegKey(LPCTSTR lpszKey, LPCTSTR lpszValue)
{
	if (::RegSetValue(HKEY_CLASSES_ROOT, lpszKey, REG_SZ,
		lpszValue, lstrlen(lpszValue) * sizeof(TCHAR)) != ERROR_SUCCESS)
	{
		TRACE(traceAppMsg, 0, _T("Warning: registration database update failed for key '%s'.\n"),
			lpszKey);
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
		SaveSettings();
	}
}

void CDjViewApp::OpenDocument(const CString& strPathName, const GUTF8String& strPage)
{
	bool bAddToHistory = true;

	CDjVuDoc* pDoc = (CDjVuDoc*)FindOpenDocument(strPathName);
	if (pDoc == NULL)
	{
		pDoc = (CDjVuDoc*)OpenDocumentFile(strPathName);
		bAddToHistory = false;
	}

	if (pDoc == NULL)
	{
		AfxMessageBox("Error opening link " + strPathName);
		return;
	}

	POSITION pos = pDoc->GetFirstViewPosition();
	ASSERT(pos != NULL);
	CDjVuView* pView = (CDjVuView*)pDoc->GetNextView(pos);

	CFrameWnd* pFrame = pView->GetParentFrame();
	pFrame->ActivateFrame();

	if (strPage.length() > 0)
	{
		pView->GoToURL(strPage, pView->GetCurrentPage(), bAddToHistory);
	}
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
		return NULL; // We won't open the file. MFC requires paths with
		// length < _MAX_PATH
	}

	POSITION pos = GetFirstDocTemplatePosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)GetNextDocTemplate(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);

		CDocument* pDocument = NULL;
		if (pTemplate->MatchDocType(szPath, pDocument) == CDocTemplate::yesAlreadyOpen)
			return pDocument;
	}

	return NULL;
}
