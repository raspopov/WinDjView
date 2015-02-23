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

#include "Drawing.h"
#include "MyTheme.h"

class CChildFrame;


// CNavPaneWnd

class CNavPaneWnd : public CWnd, public Observer, public Observable
{
	DECLARE_DYNCREATE(CNavPaneWnd)

public:
	CNavPaneWnd();
	virtual ~CNavPaneWnd();

// Attributes
public:
	enum
	{
		s_nTabsWidth = 22,
		s_nTabSize = 20,
		s_nLeftMargin = 5,
		s_nTopMargin = 22,
		s_nMinExpandedWidth = 100
	};

// Operations
public:
	int AddTab(const CString& strName, CWnd* pWnd);
	int GetTabIndex(CWnd* pTabContent);
	void ActivateTab(CWnd* pTabContent, bool bExpand = true);
	void ActivateTab(int nTab, bool bExpand = true);
	void SetTabName(CWnd* pTabContent, const CString& strName);
	void SetTabName(int nTab, const CString& strName);
	void SetTabBorder(CWnd* pTabContent, bool bHasBorder);
	void SetTabBorder(int nTab, bool bHasBorder);
	void SetTabSettings(int nTab, bool bHasSettings);
	void SetTabSettings(CWnd* pTabContent, bool bHasSettings);
	CWnd* GetActiveTab() const;

	virtual void OnUpdate(const Observable* source, const Message* message);
	CChildFrame* GetParentFrame() const;

// Implementation
protected:
	struct Tab
	{
		CString strName;
		CRect rcTab;
		CWnd* pWnd;
		CRect rcContent;
		bool bHasBorder;
		bool bHasSettings;
	};
	vector<Tab> m_tabs;
	CFont m_font;
	CFont m_fontActive;
	int m_nActiveTab;
	void DrawTab(CDC* pDC, int nTab, bool bActive);
	void UpdateButtons();
	void UpdateCloseButton(bool bPressed, bool bActive);
	int GetTabFromPoint(CPoint point);
	void UpdateTabContents();
	bool PtInTab(int nTab, CPoint point);
	CToolTipCtrl m_toolTip;
	CImageList m_imgClose, m_imgSettings;
	CRect m_rcClose, m_rcSettings;
	HTHEME m_hTheme;

	COffscreenDC m_offscreenDC;

	bool m_bCloseActive, m_bClosePressed;
	bool m_bSettingsActive, m_bSettingsPressed;
	bool m_bDragging;

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	// Generated message map functions
	afx_msg void OnPaint();
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnInitialUpdate();
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnThemeChanged();
	afx_msg void OnSysColorChange();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	DECLARE_MESSAGE_MAP()
};
