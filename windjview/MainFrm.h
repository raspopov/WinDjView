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

#include "Global.h"
#include "MyToolBar.h"
#include "TabbedMDIWnd.h"
#include "MyStatusBar.h"
#include "MyComboBox.h"
#include "DjVuSource.h"
class CDjVuView;
class CFullscreenWnd;
class CMagnifyWnd;


class CMainFrame : public CFrameWnd, public Observer
{
	DECLARE_DYNAMIC(CMainFrame)

public:
	CMainFrame();
	virtual ~CMainFrame();

// Operations
public:
	void HilightStatusMessage(LPCTSTR pszMessage);

	CFullscreenWnd* GetFullscreenWnd();
	bool IsFullscreenMode();
	CMagnifyWnd* GetMagnifyWnd();

	void AddMDIChild(CWnd* pMDIChild, CDocument* pDocument);
	void CloseMDIChild(CWnd* pMDIChild);

	void ActivateDocument(CDocument* pDocument);
	void UpdateToolbars();

	int GetTabCount();
	void RestoreOpenTabs();
	void SaveOpenTabs();
	void LoadActiveTab();

	virtual void OnUpdate(const Observable* source, const Message* message);

	bool m_bDontActivate;

// Overrides
protected:
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	virtual void OnUpdateFrameMenu(HMENU hMenuAlt);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

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
	CMyToolBar m_wndDictBar;
	CImageList m_imageListBar;
	CImageList m_imageListGrayed;
	CImageList m_imageListDict;
	CImageList m_imageListDictGrayed;

	CTabbedMDIWnd m_tabbedMDI;
	CMenu m_appMenu, m_docMenu;
	bool m_bMaximized;

	void InitToolBar();
	void InitDictBar();

	CFullscreenWnd* m_pFullscreenWnd;
	CMagnifyWnd* m_pMagnifyWnd;

	CMyComboBox m_cboPage, m_cboZoom, m_cboLangs, m_cboDict;
	CMyComboBoxEx m_cboLookup;
	void UpdatePageCombo(const CDjVuView* pView);
	void UpdateZoomCombo(const CDjVuView* pView);
	void UpdateLangAndDict(const CDjVuView* pView, bool bReset = false);

	int m_nCurDict, m_nCurLang;
	bool FindAppDictionary(const CString& strPathName, int& nLang, int& nDict);
	void UpdateDictCombo();

	void UpdateSettings();
	void LanguageChanged();
	void OnViewActivated(const CDjVuView* pView);
	bool m_bTabActivating;
	bool m_bHadActiveView;
	HMENU m_hPrevMenu;

	CFont m_font, m_boldFont;

protected:
	// Generated message map functions
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnViewToolbar();
	afx_msg void OnViewTabBar();
	afx_msg void OnViewStatusBar();
	afx_msg void OnViewSidebar();
	afx_msg void OnViewDictBar();
	afx_msg void OnUpdateViewToolbar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewTabBar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewSidebar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewDictBar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewStatusBar(CCmdUI* pCmdUI);
	afx_msg void OnToggleNavPane();
	afx_msg void OnChangePage();
	afx_msg void OnChangePageEdit();
	afx_msg void OnDropDownPage();
	afx_msg void OnChangeZoom();
	afx_msg void OnChangeZoomEdit();
	afx_msg void OnCancelChange();
	afx_msg LRESULT OnDDEExecute(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEditFind();
	afx_msg void OnUpdateEditFind(CCmdUI *pCmdUI);
	afx_msg void OnUpdateWindowList(CCmdUI *pCmdUI);
	afx_msg void OnActivateWindow(UINT nID);
	afx_msg void OnUpdateStatusAdjust(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusMode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusPage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusSize(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg LRESULT OnAppCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLookup();
	afx_msg void OnLookupSetFocus();
	afx_msg void OnFocusToLookup();
	afx_msg void OnChangeDictionary();
	afx_msg void OnChangeLanguage();
	afx_msg void OnDictionaryInfo();
	afx_msg void OnDictionaryNext();
	afx_msg void OnDictionaryPrev();
	afx_msg void OnUpdateDictionaryInfo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDictionaryNext(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDictionaryPrev(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDictionaryLookup(CCmdUI* pCmdUI);
	afx_msg void OnWindowCascade();
	afx_msg void OnWindowTileHorz();
	afx_msg void OnWindowTileVert();
	afx_msg void OnUpdateWindowCascade(CCmdUI* pCmdUI);
	afx_msg void OnWindowNext();
	afx_msg void OnWindowPrev();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNextPane(UINT nID);
	afx_msg void OnUpdateNextPane(CCmdUI* pCmdUI);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnMouseWheelPage(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNewVersion();
	afx_msg void OnNcDestroy();
	afx_msg void OnUpdateDisable(CCmdUI* pCmdUI);
	afx_msg void OnFileClose();
	afx_msg void OnUpdateFileClose(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

inline CMainFrame* GetMainWnd()
{
	return (CMainFrame*) AfxGetApp()->GetMainWnd();
}
