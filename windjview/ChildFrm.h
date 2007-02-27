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

#include "MySplitterWnd.h"
class CDjVuView;
class CNavPaneWnd;
class CThumbnailsView;
class CBookmarksWnd;
class CSearchResultsView;
class CPageIndexWnd;


// CChildFrame

class CChildFrame : public CMDIChildWnd, public Observer
{
	DECLARE_DYNCREATE(CChildFrame)

public:
	CChildFrame();

// Overrides
public:
	virtual void ActivateFrame(int nCmdShow = -1);
	CDjVuView* GetDjVuView();
	CNavPaneWnd* GetNavPane();
	virtual CDocument* GetActiveDocument();
	void CreateNavPanes();
	void SaveStartupPage();

	void HideNavPane(bool bHide = true);
	bool IsNavPaneHidden() const;
	bool IsNavPaneCollapsed() const;

	CThumbnailsView* GetThumbnailsView() { return m_pThumbnailsView; }
	CBookmarksWnd* GetBookmarks() { return m_pBookmarksWnd; }
	CPageIndexWnd* GetPageIndex() { return m_pPageIndexWnd; }
	CBookmarksWnd* GetCustomBookmarks(bool bActivate = true);
	CSearchResultsView* GetSearchResults(bool bActivate = true);

	virtual void OnUpdate(const Observable* source, const Message* message);

protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	virtual void OnUpdateFrameMenu(BOOL bActivate, CWnd* pActivateWnd, HMENU hMenuAlt);

// Implementation
public:
	virtual ~CChildFrame();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	bool m_bCreated;
	bool m_bActivating;
	bool m_bFirstShow;
	bool m_bLockingUpdate;
	int m_nStartupPage;
	CPoint m_ptStartupOffset;
	CMySplitterWnd m_wndSplitter;
//	CSplitterWnd m_wndDynSplitter;
	CThumbnailsView* m_pThumbnailsView;
	CBookmarksWnd* m_pBookmarksWnd;
	CBookmarksWnd* m_pCustomBookmarksWnd;
	CSearchResultsView* m_pResultsView;
	CPageIndexWnd* m_pPageIndexWnd;

	// Generated message map functions
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnExpandPane();
	afx_msg void OnCollapsePane();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnNcPaint();
	DECLARE_MESSAGE_MAP()
};
