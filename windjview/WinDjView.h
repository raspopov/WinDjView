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
#include "DjVuSource.h"

extern CString CURRENT_VERSION;


CString FormatDouble(double fValue);
void AFXAPI DDX_MyText(CDataExchange* pDX, int nIDC, double& value, double def = 0.0, LPCTSTR pszSuffix = NULL);
void AFXAPI DDX_MyText(CDataExchange* pDX, int nIDC, DWORD& value, DWORD def = 0, LPCTSTR pszSuffix = NULL);

class CDjVuDoc;
class CMyDocTemplate;
struct DocSettings;

void CreateSystemDialogFont(CFont& font);
void CreateSystemIconFont(CFont& font);
void CreateSystemMenuFont(CFont& font);
UINT GetMouseScrollLines();
CRect GetMonitorWorkArea(const CPoint& point);
CRect GetMonitorWorkArea(CWnd* pWnd);
CRect GetMonitorRect(CWnd* pWnd);

bool IsFromCurrentProcess(CWnd* pWnd);


// CDjViewApp

class CDjViewApp : public CWinApp, public Observable, public IApplication, public Observer
{
public:
	CDjViewApp();

	BOOL WriteProfileDouble(LPCTSTR pszSection, LPCTSTR pszEntry, double fValue);
	double GetProfileDouble(LPCTSTR pszSection, LPCTSTR pszEntry, double fDefault);
	BOOL WriteProfileCompressed(LPCTSTR pszSection, LPCTSTR pszEntry, const GUTF8String& value);
	BOOL GetProfileCompressed(LPCTSTR pszSection, LPCTSTR pszEntry, GUTF8String& value);

	virtual bool LoadDocSettings(const CString& strKey, DocSettings* pSettings);
	virtual DictionaryInfo* GetDictionaryInfo(const CString& strFileName, bool bCheckPath = true);
	virtual void ReportFatalError();

	CAppSettings* GetAppSettings() { return &m_appSettings; }
	CDisplaySettings* GetDisplaySettings() { return &m_displaySettings; }
	CPrintSettings* GetPrintSettings() { return &m_printSettings; }
	Annotation* GetAnnoTemplate() { return &m_annoTemplate; }

	void InitSearchHistory(CComboBoxEx& cboFind);
	void UpdateSearchHistory(CComboBoxEx& cboFind);

	DictionaryInfo* GetDictionaryInfo(int nIndex);
	int GetDictionaryCount() const { return static_cast<int>(m_dictVector.size()); }
	void Lookup(const CString& strLookup, DictionaryInfo* pInfo);

	CDjVuDoc* OpenDocument(LPCTSTR lpszFileName, const GUTF8String& strPage, bool bAddToHistory = true);
	bool InstallDictionary(CDjVuDoc* pDoc, bool bAllUsers, bool bKeepOriginal);
	void SaveSettings();
	bool RegisterShellFileTypes();

	// Register running threads
	void ThreadStarted();
	void ThreadTerminated();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	virtual void OnUpdate(const Observable* source, const Message* message);

	bool m_bInitialized;

// Implementation
protected:
	CAppSettings m_appSettings;
	CDisplaySettings m_displaySettings;
	CPrintSettings m_printSettings;
	Annotation m_annoTemplate;

	CMyDocTemplate* m_pDjVuTemplate;
	CDocument* FindOpenDocument(LPCTSTR lpszFileName);

	void LoadSettings();
	void EnableShellOpen();

	CEvent m_terminated;
	long m_nThreadCount;
	CEvent m_docClosed;
	DjVuSource* m_pPendingSource;

	struct LanguageInfo
	{
		DWORD nLanguage;
		CString strLanguage;
		CString strLibraryPath;
		HINSTANCE hInstance;
	};
	vector<LanguageInfo> m_languages;
	int m_nLangIndex;
	void LoadLanguages();
	void SetLanguage(UINT nLanguage);
	void SetStartupLanguage();

	set<DjVuSource*> m_deleteOnRelease;
	map<CString, DictionaryInfo> m_dictionaries;
	vector<DictionaryInfo*> m_dictVector;
	void LoadDictionaries();
	void LoadDictionaries(const CString& strDirectory);
	void LoadDictionaryInfo(DictionaryInfo& info);
	bool LoadDictionaryInfoFromDisk(DictionaryInfo& info);
	void UpdateDictVector();
	void UpdateDictProperties();
	static CString FindLocalizedString(const vector<DictionaryInfo::LocalizedString>& loc, DWORD nCurrentLang);

	HHOOK m_hHook;
	UINT m_nTimerID;
	bool m_bShiftPressed, m_bControlPressed;
	static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
	static void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);

	// Generated message map functions
	afx_msg void OnAppAbout();
	afx_msg void OnFileSettings();
	afx_msg void OnCheckForUpdate();
	afx_msg BOOL OnOpenRecentFile(UINT nID);
	afx_msg void OnSetLanguage(UINT nID);
	afx_msg void OnUpdateLanguageList(CCmdUI *pCmdUI);
	afx_msg void OnUpdateLanguage(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP()
};

extern CDjViewApp theApp;

