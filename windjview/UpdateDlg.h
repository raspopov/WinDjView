//	WinDjView
//	Copyright (C) 2004-2012 Andrew Zhezherun
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


// CUpdateDlg dialog

class CUpdateDlg : public CMyDialog
{
	DECLARE_DYNAMIC(CUpdateDlg)

public:
	CUpdateDlg(CWnd* pParent = NULL);
	virtual ~CUpdateDlg();

// Dialog Data
	enum { IDD = IDD_UPDATE };

protected:
	bool m_bOk;
	CString m_strNewVersion;

	HANDLE m_hThread;
	static unsigned int __stdcall UpdateThreadProc(void* pvData);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();

	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnEndDialog();
	DECLARE_MESSAGE_MAP()
};
