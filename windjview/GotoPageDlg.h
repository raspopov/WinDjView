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


// CGotoPageDlg dialog

#include "MyEdit.h"

class CGotoPageDlg : public CDialog
{
	DECLARE_DYNAMIC(CGotoPageDlg)

public:
	CGotoPageDlg(int nPage, int nPageCount, CWnd* pParent = NULL);
	virtual ~CGotoPageDlg();

// Dialog Data
	enum { IDD = IDD_GOTO_PAGE };
	int m_nPage;

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	CMyEdit m_edtPage;
	int m_nPageCount;

// Generated message map functions
	DECLARE_MESSAGE_MAP()
};