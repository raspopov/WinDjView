//	WinDjView
//	Copyright (C) 2004-2008 Andrew Zhezherun
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

// $Id$

#pragma once

#include "Drawing.h"


// CMyTabBar

class CMyTabBar : public CControlBar, public Observer, public Observable
{
	DECLARE_DYNCREATE(CMyTabBar)

public:
	CMyTabBar();
	virtual ~CMyTabBar();

	virtual BOOL Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID);

// Operations
public:
	int AddTab(CFrameWnd* pFrame, const CString& strName);
	void ActivateTab(CFrameWnd* pFrame);
	void ActivateNextTab();
	void ActivatePrevTab();
	void RemoveTab(CFrameWnd* pFrame);
	CFrameWnd* GetActiveTab() const;
	int GetTabCount() const { return static_cast<int>(m_tabs.size()); }

	virtual void OnUpdate(const Observable* source, const Message* message);

// Overrides
public:
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

// Implementation
protected:
	struct Tab
	{
		CFrameWnd* pFrame;
		CString strName;
		CRect rcTab;
	};
	vector<Tab> m_tabs;
	int m_nActiveTab;
	int m_nHoverTab;

	CFont m_font;
	CFont m_fontActive;
	int m_nTabHeight;
	CSize m_size;
	int m_nScrollPos;
	bool m_bShowArrows;

	CToolTipCtrl m_toolTip;
	COffscreenDC m_offscreenDC;

	void UpdateMetrics();
	void UpdateTabRects();
	void UpdateScrollState();
	void UpdateToolTips();
	void DrawTab(CDC* pDC, int nTab);
	void DrawActiveTabRect(CDC* pDC, const CRect& rect, bool bHover = false);
	void DrawInactiveTabRect(CDC* pDC, const CRect& rect,
			bool bArrow = false, bool bArrowEnabled = false, bool bArrowHover = false);
	int TabFromPoint(const CPoint& point);
	bool PtInTab(int nTab, const CPoint& point);
	void EnsureVisible(int nTab);
	int TabFromFrame(CFrameWnd* pFrame);
	void ActivateTab(int nTab);
	void RemoveTab(int nTab);

	// Generated message map functions
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnSysColorChange();
	DECLARE_MESSAGE_MAP()
};
