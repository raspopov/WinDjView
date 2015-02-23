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
#include "NavPane.h"

class CThumbnailsView;
class CBookmarksView;
class CSearchResultsView;
class CPageIndexWnd;
class CDjVuDoc;


// CMDIChild

class CMDIChild : public CWnd, public Observer
{
	DECLARE_DYNCREATE(CMDIChild)

public:
	CMDIChild();
	virtual ~CMDIChild();

// Operations
public:
	CNavPaneWnd* GetNavPane();
	CWnd* GetContent();

	bool IsPlaceholder();
	void SetError(const CString& strPathName);

	void HideNavPane(bool bHide = true);
	bool IsNavPaneHidden() const { return m_bNavHidden; }
	void CollapseNavPane(bool bCollapse = true);
	bool IsNavPaneCollapsed() const { return m_bNavCollapsed; }

	CThumbnailsView* GetThumbnailsView() { return m_pThumbnailsView; }
	CBookmarksView* GetContentsTree() { return m_pContentsTree; }
	CPageIndexWnd* GetPageIndex() { return m_pPageIndexWnd; }
	bool HasBookmarksTree() const { return m_pBookmarksTree != NULL; }
	CBookmarksView* GetBookmarksTree(bool bActivate = true);
	CSearchResultsView* GetSearchResults(bool bActivate = true);

	virtual void OnUpdate(const Observable* source, const Message* message);

// Implementation
protected:
	CNavPaneWnd m_navPane;
	CWnd* m_pContentWnd;
	CDjVuDoc* m_pDocument;
	bool m_bError;
	CString m_strPathName;

	CThumbnailsView* m_pThumbnailsView;
	CBookmarksView* m_pContentsTree;
	CBookmarksView* m_pBookmarksTree;
	CSearchResultsView* m_pResultsView;
	CPageIndexWnd* m_pPageIndexWnd;

	CFont m_font;
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
	void UpdateMetrics();

	virtual void PostNcDestroy();

	// Generated message map functions
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnExpandNav();
	afx_msg void OnCollapseNav();
	afx_msg LRESULT OnClickedNavTab(WPARAM wparam, LPARAM lParam);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	DECLARE_MESSAGE_MAP()
};
