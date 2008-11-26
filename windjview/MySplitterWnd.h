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

#include "NavPane.h"


// CMySplitterWnd

class CMySplitterWnd : public CWnd
{
	DECLARE_DYNAMIC(CMySplitterWnd)

public:
	CMySplitterWnd();
	virtual ~CMySplitterWnd();

	BOOL Create(CWnd* pParent, UINT nID = AFX_IDW_PANE_FIRST);
	BOOL CreateContent(CRuntimeClass* pContentClass, CCreateContext* pContext = NULL);

// Operations
public:
	CNavPaneWnd* GetNavPane();
	CWnd* GetContent();

	void HideNavPane(bool bHide = true);
	bool IsNavPaneHidden() const { return m_bNavHidden; }
	void CollapseNavPane(bool bCollapse = true);
	bool IsNavPaneCollapsed() const { return m_bNavCollapsed; }

// Implementation
protected:
	CNavPaneWnd m_navPane;
	CWnd* m_pContentWnd;

	CRect m_rcNavPane;
	CRect m_rcContent;
	CRect m_rcSplitter;
	bool m_bNavCollapsed;
	bool m_bNavHidden;
	int m_nSplitterPos;
	int m_nExpandedNavWidth;
	CPoint m_ptDragStart;
	int m_nOrigSplitterPos;
	bool m_bDragging;

	void RecalcLayout();
	void StopDragging();

	// Generated message map functions
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()
};
