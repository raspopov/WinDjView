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


// CMyStatusBar

class CMyStatusBar : public CStatusBar
{
	DECLARE_DYNAMIC(CMyStatusBar)

public:
	CMyStatusBar();
	virtual ~CMyStatusBar();

// Operations
public:
	void SetHilightMessage(LPCTSTR pszMessage);

// Overrides
protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItem);

// Implementation
protected:
	CString m_strHilightMsg;

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
};