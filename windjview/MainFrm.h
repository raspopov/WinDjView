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

#include "Global.h"
#include "MyToolBar.h"
#include "MyStatusBar.h"
#include "MyComboBox.h"
#include "FindDlg.h"
#include "DjVuSource.h"
class CDjVuView;
class CFullscreenWnd;
class CMagnifyWnd;

#define WM_UPDATE_KEYBOARD (WM_APP + 4)


class CMainFrame : public CMDIFrameWnd, public Observer
{
	DECLARE_DYNAMIC(CMainFrame)

public:
	CMainFrame();
	virtual ~CMainFrame();

// Attributes
public:
	CFindDlg* m_pFindDlg;

// Operations
public:
	void HilightStatusMessage(LPCTSTR pszMessage);

	void AddToHistory(CDjVuView* pView);
	void AddToHistory(CDjVuView* pView, int nPage);
	void AddToHistory(CDjVuView* pView, const Bookmark& bookmark);

	CFullscreenWnd* GetFullscreenWnd();
	bool IsFullscreenMode();
	CMagnifyWnd* GetMagnifyWnd();

	void SetStartupLanguage();
	int GetDocumentCount();

	virtual void OnUpdate(const Observable* source, const Message* message);

// Overrides
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	virtual void OnUpdateFrameMenu(HMENU hMenuAlt);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// control bar embedded members
	CMyStatusBar m_wndStatusBar;
	CMyToolBar m_wndToolBar;

	CFullscreenWnd* m_pFullscreenWnd;
	CMagnifyWnd* m_pMagnifyWnd;

	CMyComboBox m_cboPage, m_cboZoom;
	void UpdatePageCombo(const CDjVuView* pView);
	void UpdateZoomCombo(const CDjVuView* pView);

	static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
	static HHOOK hHook;

	void UpdateSettings();

	CFont m_font;

	struct HistoryPos
	{
		bool operator==(const HistoryPos& rhs) const
		{
			ASSERT(bmView.nLinkType == Bookmark::View && rhs.bmView.nLinkType == Bookmark::View);
			return strFileName == rhs.strFileName
					&& bmView.nPage == rhs.bmView.nPage
					&& bmView.ptOffset == rhs.bmView.ptOffset;
		}
		bool operator!=(const HistoryPos& rhs) const
			{ return !(*this == rhs); }

		CString strFileName;
		Bookmark bookmark;
		Bookmark bmView;
	};
	list<HistoryPos> m_history;
	list<HistoryPos>::iterator m_historyPos;
	void GoToHistoryPos(const HistoryPos& pos);
	void AddToHistory(const HistoryPos& pos);

	struct LanguageInfo
	{
		DWORD nLanguage;
		CString strLanguage;
		CString strLibraryPath;
		HINSTANCE hInstance;
		bool bEnabled;
	};
	vector<LanguageInfo> m_languages;
	int m_nLanguage;
	void LoadLanguages();
	void SetLanguage(UINT nLanguage);
	void LanguageChanged();

protected:
	// Generated message map functions
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewToolbar();
	afx_msg void OnViewStatusBar();
	afx_msg void OnViewSidebar();
	afx_msg void OnUpdateViewSidebar(CCmdUI* pCmdUI);
	afx_msg void OnChangePage();
	afx_msg void OnChangePageEdit();
	afx_msg void OnChangeZoom();
	afx_msg void OnChangeZoomEdit();
	afx_msg void OnCancelChangePageZoom();
	afx_msg LRESULT OnDDEExecute(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEditFind();
	afx_msg void OnUpdateEditFind(CCmdUI *pCmdUI);
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
	afx_msg void OnDestroy();
	afx_msg LRESULT OnUpdateKeyboard(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSetLanguage(UINT nID);
	afx_msg void OnUpdateLanguageList(CCmdUI *pCmdUI);
	afx_msg void OnUpdateLanguage(CCmdUI *pCmdUI);
	afx_msg void OnClose();
	afx_msg LRESULT OnAppCommand(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

inline CMainFrame* GetMainFrame()
{
	return static_cast<CMainFrame*>(AfxGetApp()->GetMainWnd());
}
