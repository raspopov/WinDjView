//	DjVu Dictionary Tool
//	Copyright (C) 2006-2007 Andrew Zhezherun
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

#include "DjVuSource.h"


// CAddStringDlg dialog

class CAddStringDlg : public CDialog
{
	DECLARE_DYNAMIC(CAddStringDlg)

public:
	CAddStringDlg(const vector<DictionaryInfo::LocalizedString>& loc, CWnd* pParent = NULL);
	virtual ~CAddStringDlg();

	int m_nLocalization;

// Dialog Data
	enum { IDD = IDD_ADD_STRING };
	CComboBox m_loc;

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	set<DWORD> m_excluded;

	// Message map functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
};
