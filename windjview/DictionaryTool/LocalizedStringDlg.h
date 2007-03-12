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


// CLocalizedStringDlg dialog

class CLocalizedStringDlg : public CDialog
{
	DECLARE_DYNAMIC(CLocalizedStringDlg)

public:
	CLocalizedStringDlg(const vector<DictionaryInfo::LocalizedString>& loc, CWnd* pParent = NULL);
	virtual ~CLocalizedStringDlg();

	vector<DictionaryInfo::LocalizedString> m_loc;

// Dialog Data
	enum { IDD = IDD_LOCALIZED };
	CListCtrl m_list;

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	CImageList m_imgList;

	// Message map functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnBeginEditLabel(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndEditLabel(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKickIdle();
	DECLARE_MESSAGE_MAP()
};
