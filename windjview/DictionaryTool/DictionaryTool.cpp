//	DjVu Dictionary Tool
//	Copyright (C) 2006-2007 Andrew Zhezherun
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
#include "DictionaryTool.h"
#include "DictionaryDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDictionaryApp

BEGIN_MESSAGE_MAP(CDictionaryApp, CWinApp)
END_MESSAGE_MAP()


// CDictionaryApp construction

CDictionaryApp::CDictionaryApp()
{
}


// The one and only CDictionaryApp object
CDictionaryApp theApp;


// CDictionaryApp initialization

BOOL CDictionaryApp::InitInstance()
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

	CDictionaryDlg dlg;
	m_pMainWnd = &dlg;

	dlg.DoModal();

	AfxOleTerm();

	// Since the dialog has been closed, return FALSE so that we exit the
	// application, rather than start the application's message pump.
	return false;
}
