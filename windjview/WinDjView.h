//	WinDjView
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
//	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//	http://www.gnu.org/copyleft/gpl.html

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "AppSettings.h"
#include "Global.h"
#include "DjVuSource.h"

class CDjVuDoc;
class CMyDocTemplate;
class CMainFrame;
class CFindDlg;
struct DocSettings;

extern const TCHAR* s_pszDocumentsSection;
extern const TCHAR* s_pszSettings;
extern const TCHAR* s_pszLastKnownLocation;


// CDjViewApp

class CDjViewApp : public CWinApp, public Observable, public IApplication, public Observer
{
public:
	CDjViewApp();

	BOOL WriteProfileDouble(LPCTSTR pszSection, LPCTSTR pszEntry, double fValue);
	double GetProfileDouble(LPCTSTR pszSection, LPCTSTR pszEntry, double fDefault);
	BOOL WriteProfileCompressed(LPCTSTR pszSection, LPCTSTR pszEntry, const GUTF8String& value);
	BOOL GetProfileCompressed(LPCTSTR pszSection, LPCTSTR pszEntry, GUTF8String& value);
	bool EnumProfileKeys(LPCTSTR pszSection, vector<CString>& keys);
	bool DeleteProfileKey(LPCTSTR pszSection, LPCTSTR pszKey);

	virtual bool LoadDocSettings(const CString& strKey, DocSettings* pSettings);
	virtual DictionaryInfo* GetDictionaryInfo(const CString& strPathName, bool bCheckPath = true);
	virtual void ReportFatalError();

	CAppSettings* GetAppSettings() { return &m_appSettings; }
	CDisplaySettings* GetDisplaySettings() { return &m_displaySettings; }
	CPrintSettings* GetPrintSettings() { return &m_printSettings; }
	Annotation* GetAnnoTemplate() { return &m_annoTemplate; }

	CDjVuDoc* OpenDocument(LPCTSTR lpszFileName, const GUTF8String& strPage, bool bAddHistoryPoint = true);
	CDjVuDoc* FindOpenDocument(LPCTSTR lpszFileName);
	int GetDocumentCount();
	CMyDocTemplate* GetDocumentTemplate();

	CMainFrame* CreateMainFrame(bool bAppStartup = false, int nCmdShow = -1);
	void RemoveMainFrame(CMainFrame* pMainFrame);
	void ChangeMainWnd(CMainFrame* pMainFrame, bool bActivate = false);
	void DisableTopLevelWindows(set<CWnd*>& disabled);
	void EnableWindows(set<CWnd*>& disabled);

	void SaveSettings();
	bool RegisterShellFileTypes(bool bCheckOnly = false);
	bool RegisterShellFileTypesElevate(CWnd* pWnd = NULL);

	void InitSearchHistory(CComboBoxEx& cboFind);
	void UpdateSearchHistory(CComboBoxEx& cboFind);

	CFindDlg* GetFindDlg(bool bCreate = true);
	void UpdateFindDlg(CWnd* pNewParent = NULL);

	static CString DownloadLastVersionString();

	// Dictionary API
	int GetDictLangsCount() const
		{ return static_cast<int>(m_dictsByLang.size()); }
	int GetDictionaryCount(int nLangIndex) const
		{ return static_cast<int>(m_dictsByLang[nLangIndex].dicts.size()); }
	CString GetLangFrom(int nLangIndex) const
		{ return m_dictsByLang[nLangIndex].strFrom; }
	CString GetLangTo(int nLangIndex) const
		{ return m_dictsByLang[nLangIndex].strTo; }
	DictionaryInfo* GetDictionaryInfo(int nLangIndex, int nIndex)
		{ return m_dictsByLang[nLangIndex].dicts[nIndex]; }
	void Lookup(const CString& strLookup, DictionaryInfo* pInfo);
	bool InstallDictionary(DjVuSource* pSource, int nLocationChoice, bool bKeepOriginal);
	bool UninstallDictionary(DictionaryInfo* pInfo);
	void ReloadDictionaries();

	// Register running threads
	void ThreadStarted();
	void ThreadTerminated();

	struct DocIterator
	{
	public:
		DocIterator() : posDoc(NULL)
		{
			posTemplate = AfxGetApp()->GetFirstDocTemplatePosition();
			++*this;
		}

		operator bool() const { return pTemplate != NULL && pDoc != NULL; }
		CDocument* operator->() { return pDoc; }
		CDocument* operator*() { return pDoc; }

		DocIterator& operator++()  // preincrement
		{
			while (posTemplate && !posDoc)
			{
				pTemplate = AfxGetApp()->GetNextDocTemplate(posTemplate);
				posDoc = pTemplate->GetFirstDocPosition();
			}

			if (posDoc)
			{
				pDoc = pTemplate->GetNextDoc(posDoc);
			}
			else
			{
				pTemplate = NULL;
				pDoc = NULL;
			}

			return *this;
		}

		DocIterator operator++(int)  // postincrement
		{
			DocIterator tmp = *this;
			++*this;
			return tmp;
		}

	protected:
		POSITION posTemplate, posDoc;
		CDocTemplate* pTemplate;
		CDocument* pDoc;
	};

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL SaveAllModified();
	virtual int DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDHelp);

	struct MessageBoxOptions
	{
		MessageBoxOptions() : pCheckValue(NULL), bVerbatim(false) {}

		CString strCaptions;
		CString strCheckBox;
		bool* pCheckValue;
		bool bVerbatim;
	};

	virtual int DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDHelp, const MessageBoxOptions& mbo);

	virtual void OnUpdate(const Observable* source, const Message* message);

	bool m_bInitialized;
	bool m_bClosing;
	bool m_bTopLevelDocs;
	CString m_strNewVersion;
	bool m_bNoAboutOnStartup;

// Implementation
protected:
	CAppSettings m_appSettings;
	CDisplaySettings m_displaySettings;
	CPrintSettings m_printSettings;
	Annotation m_annoTemplate;

	CMyDocTemplate* m_pDjVuTemplate;
	CFindDlg* m_pFindDlg;
	list<CMainFrame*> m_frames;
	CCriticalSection m_mainWndLock;

	void LoadSettings();
	void EnableShellOpen();

	CEvent m_terminated;
	long m_nThreadCount;
	CEvent m_docClosed;
	DjVuSource* m_pPendingSource;

	HHOOK m_hMBHook;
	UINT m_nMBType;
	CString m_strMBPrompt;
	MessageBoxOptions m_mbo;
	static LRESULT CALLBACK MBHookProc(int nCode, WPARAM wParam, LPARAM lParam);

	class CMyMessageBox : public CWnd
	{
	public:
		CMyMessageBox() : m_hwndEdit(NULL), m_hwndCheck(NULL) {}
		virtual ~CMyMessageBox() {}
		virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
		HWND m_hwndEdit, m_hwndCheck;
	};
	CMyMessageBox* m_pMBWnd;

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
	void LoadDictionaries();
	void LoadDictionaries(CString strDirectory);
	void LoadDictionaryInfo(DictionaryInfo& info);
	bool LoadDictionaryInfoFromDisk(DictionaryInfo& info);
	void UpdateDictVector();
	void UpdateDictProperties();
	static CString FindLocalizedString(const vector<DictionaryInfo::LocalizedString>& loc,
			DWORD nCurrentLang, int* pnMatch = NULL);
	struct DictsByLang
	{
		CString strFrom, strTo;
		vector<DictionaryInfo*> dicts;
	};
	vector<DictsByLang> m_dictsByLang;

	HHOOK m_hHook;
	UINT_PTR m_nTimerID;
	bool m_bShiftPressed, m_bControlPressed;
	static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
	static void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

	bool m_bOnlyRegisterTypes;
	int m_nExitCode;

	static unsigned int __stdcall CheckUpdateThreadProc(void* pvData);
	HANDLE m_hUpdateThread;

	// Generated message map functions
	afx_msg void OnAppAbout();
	afx_msg void OnAppExit();
	afx_msg void OnFileSettings();
	afx_msg void OnCheckForUpdate();
	afx_msg BOOL OnOpenRecentFile(UINT nID);
	afx_msg void OnSetLanguage(UINT nID);
	afx_msg void OnUpdateLanguageList(CCmdUI *pCmdUI);
	afx_msg void OnUpdateLanguage(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP()
};

extern CDjViewApp theApp;
extern CString CURRENT_VERSION;
