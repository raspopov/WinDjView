//	WinDjView
//	Copyright (C) 2004-2006 Andrew Zhezherun
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
//	http://www.gnu.org/copyleft/gpl.html

// $Id$

#pragma once

#include "MyToolBar.h"
#include "MyStatusBar.h"
#include "MyComboBox.h"
#include "MyBitmapButton.h"
#include "FindDlg.h"
class CDjVuView;
class CFullscreenWnd;

#define WM_SHOWALLLINKS (WM_APP + 4)
#define WM_LANGUAGE_CHANGED (WM_APP + 5)


class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:
	CMyComboBox m_cboPage, m_cboZoom;
	CFindDlg* m_pFindDlg;
	CFullscreenWnd* m_pFullscreenWnd;

// Operations
public:
	void HilightStatusMessage(LPCTSTR pszMessage);
	void UpdatePageCombo(CDjVuView* pView);
	void UpdateZoomCombo(int nZoomType, double fZoom);

	void AddToHistory(CDjVuView* pView, int nPage);
	void SetFullscreenWnd(CFullscreenWnd* pWnd) { m_pFullscreenWnd = pWnd; }
	void SetStartupLanguage();

// Overrides
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	virtual void OnUpdateFrameMenu(HMENU hMenuAlt);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// control bar embedded members
	CMyStatusBar m_wndStatusBar;
	CMyToolBar m_wndToolBar;

	static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
	static HHOOK hHook;

	bool m_bFirstShow;
	void UpdateSettings();

	CFont m_font;

	struct HistoryPos
	{
		CString strFileName;
		int nPage;

		bool operator==(const HistoryPos& rhs) const { return strFileName == rhs.strFileName && nPage == rhs.nPage; }
		bool operator!=(const HistoryPos& rhs) const { return !(*this == rhs); }
	};
	list<HistoryPos> m_history;
	list<HistoryPos>::iterator m_historyPos;
	void GoToHistoryPos(const HistoryPos& pos);

	struct LanguageInfo
	{
		CString strLanguage;
		CString strLibraryPath;
		HINSTANCE hInstance;
		bool bEnabled;
	};
	vector<LanguageInfo> m_languages;
	int m_nLanguage;
	void LoadLanguages();
	void SetLanguage(int nLanguage);

protected:
	// Generated message map functions
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewToolbar();
	afx_msg void OnViewStatusBar();
	afx_msg void OnChangePage();
	afx_msg void OnChangePageEdit();
	afx_msg void OnChangeZoom();
	afx_msg void OnChangeZoomEdit();
	afx_msg void OnCancelChangePageZoom();
	afx_msg LRESULT OnDDEExecute(WPARAM wParam, LPARAM lParam);
	afx_msg void OnViewFind();
	afx_msg void OnUpdateViewFind(CCmdUI *pCmdUI);
	afx_msg void OnUpdateWindowList(CCmdUI *pCmdUI);
	afx_msg void OnActivateWindow(UINT nID);
	afx_msg void OnViewBack();
	afx_msg void OnUpdateViewBack(CCmdUI *pCmdUI);
	afx_msg void OnViewForward();
	afx_msg void OnUpdateViewForward(CCmdUI *pCmdUI);
	afx_msg void OnUpdateStatusAdjust(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusMode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusPage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusSize(CCmdUI* pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnShowAllLinks(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSetLanguage(UINT nID);
	afx_msg void OnUpdateLanguageList(CCmdUI *pCmdUI);
	afx_msg void OnUpdateLanguage(CCmdUI *pCmdUI);
	afx_msg void OnLanguageChanged();
	DECLARE_MESSAGE_MAP()
};

inline CMainFrame* GetMainFrame()
{
	return static_cast<CMainFrame*>(AfxGetApp()->GetMainWnd());
}

void CreateSystemDialogFont(CFont& font);
UINT GetMouseScrollLines();
