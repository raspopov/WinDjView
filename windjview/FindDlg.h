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
class CMainFrame;


// CFindDlg dialog

class CFindDlg : public CMyDialog
{
	DECLARE_DYNAMIC(CFindDlg)

public:
	CFindDlg(CWnd* pParent = NULL);
	virtual ~CFindDlg();

	void SetStatusText(const CString& strStatus);
	CMainFrame* GetMainFrame();

// Dialog Data
	enum { IDD = IDD_FIND };
	CString m_strFind;
	CComboBoxEx m_cboFind;
	BOOL m_bMatchCase;

// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	void UpdateButtons();
	void UpdateSearchHistory();
	void InitSearchHistory();

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnClose();
	afx_msg void OnFindAll();
	afx_msg void OnEnChangeFind();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	DECLARE_MESSAGE_MAP()
};
