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

class CNavPaneWnd;


// CMySplitterWnd

class CMySplitterWnd : public CSplitterWnd
{
	DECLARE_DYNAMIC(CMySplitterWnd)

public:
	CMySplitterWnd();
	virtual ~CMySplitterWnd();

// Operations
public:
	CNavPaneWnd* GetNavPane();
	void HideNavPane(bool bHide = true);
	void CollapseNavPane(bool bCollapse = true);
	void UpdateNavPane();

	int GetSplitterWidth() { return m_cxSplitter; }
	int GetSplitterHeight() { return m_cySplitter; }

	// Overrides
protected:
	virtual void StopTracking(BOOL bAccept);
	virtual int HitTest(CPoint pt) const;

	virtual void TrackRowSize(int y, int row);
	virtual void TrackColumnSize(int x, int col);
	virtual void DrawAllSplitBars(CDC* pDC, int cxInside, int cyInside);

// Implementation
protected:
	int m_nNavPaneWidth;
	bool m_bCollapsed;

	bool m_bAllowTracking;
	bool m_bHideSplitter;

	int m_cxOrigSplitter;
	int m_cxOrigSplitterGap;
	int m_cyOrigSplitter;
	int m_cyOrigSplitterGap;

	void DrawLeftPaneBorder(CDC* pDC, const CRect& rcArg);
	void UpdateNavPaneWidth(int nWidth);

	// Generated message map functions
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};
