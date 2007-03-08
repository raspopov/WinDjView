//	DjVu Bookmark Tool
//	Copyright (C) 2005-2007 Andrew Zhezherun
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
#include "BookmarkTool.h"
#include "BookmarkDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBookmarkApp

BEGIN_MESSAGE_MAP(CBookmarkApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


const TCHAR* s_pszGlobalSettings = _T("Settings");
const TCHAR* s_pszPrompt = _T("prompt");

// CBookmarkApp construction

CBookmarkApp::CBookmarkApp()
{
}


// The one and only CBookmarkApp object
CBookmarkApp theApp;


// CBookmarkApp initialization

BOOL CBookmarkApp::InitInstance()
{
	_tsetlocale(LC_ALL, _T(""));

	AfxOleInit();

	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization

	// Change the registry key under which our settings are stored
	SetRegistryKey(_T("Andrew Zhezherun"));

	CBookmarkDlg dlg;
	m_pMainWnd = &dlg;

	dlg.DoModal();

	AfxOleTerm();

	// Since the dialog has been closed, return FALSE so that we exit the
	// application, rather than start the application's message pump.
	return FALSE;
}
