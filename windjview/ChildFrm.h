//	WinDjView
//	Copyright (C) 2004-2005 Andrew Zhezherun
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

#include "MySplitterWnd.h"
#include "DjVuDoc.h"
class CDjVuView;
class CNavPaneWnd;
class CThumbnailsView;
class CBookmarksView;


// CChildFrame

class CChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();

// Overrides
public:
	virtual void ActivateFrame(int nCmdShow = -1);
	CDjVuView* GetDjVuView();
	CNavPaneWnd* GetNavPane();
	virtual CDjVuDoc* GetActiveDocument();
	void CreateNavPanes();

protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	virtual void OnUpdateFrameMenu(BOOL bActivate, CWnd* pActivateWnd, HMENU hMenuAlt);

// Implementation
public:
	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	bool m_bCreated;
	CMySplitterWnd m_wndSplitter;
	CThumbnailsView* m_pThumbnailsView;
	CBookmarksView* m_pBookmarksView;

// Generated message map functions
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnExpandPane();
	afx_msg void OnCollapsePane();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
};
