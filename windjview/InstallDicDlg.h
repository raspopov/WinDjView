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


// CInstallDicDlg dialog

class CInstallDicDlg : public CMyDialog
{
	DECLARE_DYNAMIC(CInstallDicDlg)

public:
	CInstallDicDlg(UINT nID = CInstallDicDlg::IDD, CWnd* pParent = NULL);
	virtual ~CInstallDicDlg();

// Dialog Data
	enum { IDD = IDD_INSTALL_DIC };
	int m_nChoice;
	BOOL m_bKeepOriginal;
	CString m_strDictLocation;

protected:
	UINT m_nTemplateID;
	static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBrowseLocation();
	DECLARE_MESSAGE_MAP()
};
