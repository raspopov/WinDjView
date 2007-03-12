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
	virtual void OnOK();

// Implementation
protected:
	CMyEdit m_edtPage;
	int m_nPageCount;

	// Message map functions
	afx_msg void OnPageUpDown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateDialogData();
	DECLARE_MESSAGE_MAP()
};
