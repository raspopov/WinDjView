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
//  http://www.gnu.org/copyleft/gpl.html

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
	static const int s_nHorzBorderWidth;
	static const int s_nVertBorderHeight;
	
// Implementation
public:
	CWnd* GetChildWnd() const;
	void SetChildWnd(CWnd* pWnd);
	int GetMinWidth() const;

protected:
	CWnd* m_pChildWnd;
	int m_nDefPaneWidth;
	int m_nLockPos;

	// Generated message map functions
	afx_msg void OnPaint();
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
};
