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


// CNavPaneWnd

class CNavPaneWnd : public CWnd
{
	DECLARE_DYNCREATE(CNavPaneWnd)

public:
	CNavPaneWnd();
	virtual ~CNavPaneWnd();

// Attributes
public:
	enum
	{
		s_nTabsWidth = 20,
		s_nTabSize = 20,
		s_nLeftMargin = 5,
		s_nTopMargin = 22,
		s_nMinExpandedWidth = 100
	};

// Operations
public:
	int AddTab(const CString& strName, CWnd* pWnd);
	int GetTabIndex(CWnd* pTabContent);
	void ActivateTab(CWnd* pTabContent);
	void ActivateTab(int nTab);
	void SetTabName(CWnd* pTabContent, const CString& strName);
	void SetTabName(int nTab, const CString& strName);

// Implementation
protected:
	struct Tab
	{
		CString strName;
		CRect rcTab;
		CWnd* pWnd;
	};
	vector<Tab> m_tabs;
	CFont m_font;
	CFont m_fontActive;
	int m_nActiveTab;
	void DrawTab(CDC* pDC, int nTab, bool bActive);
	void UpdateCloseButton(bool bLButtonDown);
	void UpdateCloseButtonImpl(bool bPressed, bool bActive, const CRect& rcButton);
	int GetTabFromPoint(CPoint point);
	void UpdateTabContents();
	bool PtInTab(int nTab, CPoint point);
	CToolTipCtrl m_toolTip;
	CImageList m_imgClose;

	CBitmap* m_pBitmapTabs;
	CSize m_szBitmap;

	bool m_bCloseActive;
	bool m_bClosePressed;
	bool m_bDragging;

	// Generated message map functions
	afx_msg void OnPaint();
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLanguageChanged();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()
};

COLORREF ChangeBrightness(COLORREF color, double fFactor);
