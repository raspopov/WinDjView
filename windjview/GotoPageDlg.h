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

#include "MyDialog.h"
#include "MyEdit.h"


// CGotoPageDlg dialog

class CGotoPageDlg : public CMyDialog
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
