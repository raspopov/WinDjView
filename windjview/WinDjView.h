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

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "AppSettings.h"
#include "Global.h"

extern CString CURRENT_VERSION;


CString FormatDouble(double fValue);
void AFXAPI DDX_MyText(CDataExchange* pDX, int nIDC, double& value, double def = 0.0, LPCTSTR pszSuffix = NULL);
void AFXAPI DDX_MyText(CDataExchange* pDX, int nIDC, DWORD& value, DWORD def = 0, LPCTSTR pszSuffix = NULL);

class CDjVuDoc;
class CMyDocTemplate;
struct DjVuUserData;

void ReportFatalError();

void CreateSystemDialogFont(CFont& font);
void CreateSystemIconFont(CFont& font);
void CreateSystemMenuFont(CFont& font);
UINT GetMouseScrollLines();
CRect GetMonitorWorkArea(const CPoint& point);

bool IsFromCurrentProcess(CWnd* pWnd);


// CDjViewApp

class CDjViewApp : public CWinApp, public Observable
{
public:
	CDjViewApp();

	BOOL WriteProfileDouble(LPCTSTR pszSection, LPCTSTR pszEntry, double fValue);
	double GetProfileDouble(LPCTSTR pszSection, LPCTSTR pszEntry, double fDefault);
	void LoadDjVuUserData(const CString& strKey, DjVuUserData* pData);

	CAppSettings* GetAppSettings() { return &m_appSettings; }
	CDisplaySettings* GetDisplaySettings() { return &m_displaySettings; }
	CPrintSettings* GetPrintSettings() { return &m_printSettings; }

	void SetLanguage(HINSTANCE hResources, DWORD nLanguage);
	void SaveSettings();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	bool RegisterShellFileTypes();

	CDjVuDoc* OpenDocument(const CString& strPathName, const GUTF8String& strPage);

	bool m_bInitialized;
	CMyDocTemplate* m_pDjVuTemplate;

// Implementation
protected:
	CAppSettings m_appSettings;
	CDisplaySettings m_displaySettings;
	CPrintSettings m_printSettings;

	void LoadSettings();
	void EnableShellOpen();
	CDocument* FindOpenDocument(LPCTSTR lpszFileName);

	// Generated message map functions
	afx_msg void OnAppAbout();
	afx_msg void OnFileSettings();
	afx_msg void OnCheckForUpdate();
	afx_msg BOOL OnOpenRecentFile(UINT nID);
	DECLARE_MESSAGE_MAP()
};

extern CDjViewApp theApp;

