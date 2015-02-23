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


// CMyToolBar

// CMyToolbar wraps a CToolBar with a CControlBar. This causes the
// toolbar to get a standard COLOR_BTNFACE background in XP, which
// is consistent with the other elements of the UI. Without wrapping,
// the toolbar buttons always have themed background.

class CMyToolBar : public CControlBar, public Observer
{
	DECLARE_DYNAMIC(CMyToolBar)

public:
	class CAuxToolBar : public CToolBar
	{
	public:
		virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
		bool LoadToolBar(UINT nIDResource);
		bool LoadToolBar(LPCTSTR lpszResourceName);
		bool LoadBitmap(UINT nIDResource);
		bool LoadBitmap(LPCTSTR lpszResourceName);
	};

	CMyToolBar();
	virtual ~CMyToolBar();

	virtual BOOL Create(CWnd* pParentWnd,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP,
		UINT nID = AFX_IDW_TOOLBAR);
	virtual BOOL CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle = TBSTYLE_FLAT,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP,
		CRect rcBorders = CRect(0, 0, 0, 0),
		UINT nID = AFX_IDW_TOOLBAR);

	CAuxToolBar& GetToolBar() { return m_toolBar; }
	CToolBarCtrl& GetToolBarCtrl() const { return m_toolBar.GetToolBarCtrl(); }

	void InsertLabel(int nPos, UINT nID, CFont* pFont);
	void SetControl(int nPos, UINT nID, int nWidth);

// Implementation
public:
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual CSize CalcDynamicLayout(int nLength, DWORD nMode);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

	virtual void OnUpdate(const Observable* source, const Message* message);

protected:
	struct Label
	{
		UINT nID;
		CString strText;
		CFont* pFont;
	};
	vector<Label> m_labels;
	set<DWORD_PTR> m_controls;

	CAuxToolBar m_toolBar;

	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnWindowPosChanging(LPWINDOWPOS lpWndPos);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnSysColorChange();
	DECLARE_MESSAGE_MAP()
};
