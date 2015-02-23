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


// CTabbedMDIWnd

class CTabbedMDIWnd : public CWnd, public Observer, public Observable
{
	DECLARE_DYNCREATE(CTabbedMDIWnd)

public:
	CTabbedMDIWnd();
	virtual ~CTabbedMDIWnd();

// Operations
public:
	int AddTab(CWnd* pWnd, const CString& strName, const CString& strPathName);
	void ActivateTab(CWnd* pWnd);
	void ActivateTab(int nTab);
	void ActivateNextTab();
	void ActivatePrevTab();
	void CloseTab(CWnd* pWnd);
	CWnd* GetActiveTab() const;
	int GetActiveTabIndex() const;
	int GetTabCount() const { return static_cast<int>(m_tabs.size()); }
	CString GetTabName(int nTab) const;
	CString GetTabPathName(int nTab) const;

	bool IsTabBarVisible() const { return !m_bTabBarHidden; }
	void ShowTabBar(bool bShow);

	virtual void OnUpdate(const Observable* source, const Message* message);

// Implementation
protected:
	struct Tab
	{
		CWnd* pWnd;
		CString strName;
		CString strPathName;
		bool bReactivate;
		CRect rcTab, rcClose;
	};
	vector<Tab> m_tabs;
	int m_nActiveTab;
	int m_nHoverTab;

	CFont m_font;
	CFont m_fontActive;
	int m_nTabHeight;
	CSize m_szTabBar;
	CRect m_rcContent;
	int m_nScrollPos;
	bool m_bShowArrows;
	CRect m_rcLeftArrow, m_rcRightArrow;
	bool m_bHoverLeft, m_bHoverRight, m_bHoverClose;
	int m_nClosePressedTab, m_nMButtonPressedTab;
	bool m_bTabBarHidden;
	CImageList m_imgClose;
	bool m_bIgnoreMouseLeave;

	CToolTipCtrl m_toolTip;
	CString m_strCloseTab;
	COffscreenDC m_offscreenDC;

	bool HasCloseButton(int nTab, CRect* prcClose = NULL);
	void UpdateMetrics();
	void UpdateTabRects();
	void UpdateScrollState();
	void UpdateToolTips();
	void UpdateHoverTab();
	void DrawTab(CDC* pDC, int nTab);
	void DrawActiveTabRect(CDC* pDC, const CRect& rect, bool bHover = false);
	void DrawInactiveTabRect(CDC* pDC, const CRect& rect,
			bool bArrow = false, bool bArrowEnabled = false, bool bArrowHover = false);
	int TabFromPoint(const CPoint& point);
	bool PtInTab(int nTab, const CPoint& point);
	void EnsureVisible(int nTab);
	int TabFromFrame(CWnd* pWnd);
	int FindTab(const CString& strPathName);
	void ActivateTabImpl(int nTab, bool bRedraw = true);
	void CloseTab(int nTab, bool bRedraw = true);
	void InvalidateTabs();
	void UpdateSize(int cx, int cy);

	// Generated message map functions
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnSysColorChange();
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
	DECLARE_MESSAGE_MAP()
};
