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

#include "stdafx.h"
#include "WinDjView.h"
#include "FindDlg.h"
#include "MainFrm.h"
#include "AppSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFindDlg dialog

IMPLEMENT_DYNAMIC(CFindDlg, CDialog)
CFindDlg::CFindDlg(CWnd* pParent)
	: CDialog(CFindDlg::IDD, pParent)
{
	m_strFind = theApp.GetAppSettings()->strFind;
	m_bMatchCase = theApp.GetAppSettings()->bMatchCase;
}

CFindDlg::~CFindDlg()
{
}

void CFindDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FIND, m_strFind);
	DDX_Check(pDX, IDC_MATCH_CASE, m_bMatchCase);
}


BEGIN_MESSAGE_MAP(CFindDlg, CDialog)
	ON_BN_CLICKED(IDC_FIND_ALL, OnFindAll)
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_FIND, OnEnChangeFind)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CFindDlg message handlers

void CFindDlg::OnOK()
{
	if (!UpdateData())
		return;

	GetMainFrame()->SetMessageText(AFX_IDS_IDLEMESSAGE);
	GetMainFrame()->SendMessage(WM_COMMAND, ID_FIND_STRING);
}

void CFindDlg::OnCancel()
{
	GetMainFrame()->SetMessageText(AFX_IDS_IDLEMESSAGE);
	ShowWindow(SW_HIDE);
}

void CFindDlg::OnClose()
{
	GetMainFrame()->SetMessageText(AFX_IDS_IDLEMESSAGE);
	ShowWindow(SW_HIDE);
}

void CFindDlg::OnFindAll()
{
	if (!UpdateData())
		return;

	SetStatusText(LoadString(IDS_SEARCHING));
	GetDlgItem(IDC_SEARCH_STATUS)->ShowWindow(SW_SHOW);

	GetMainFrame()->SetMessageText(AFX_IDS_IDLEMESSAGE);
	GetMainFrame()->SendMessage(WM_COMMAND, ID_FIND_ALL);

	GetDlgItem(IDC_SEARCH_STATUS)->ShowWindow(SW_HIDE);
}

void CFindDlg::SetStatusText(const CString& strStatus)
{
	GetDlgItem(IDC_SEARCH_STATUS)->SetWindowText(strStatus);
}

void CFindDlg::OnEnChangeFind()
{
	UpdateButtons();
}

void CFindDlg::UpdateButtons()
{
	CString strText;
	GetDlgItem(IDC_FIND)->GetWindowText(strText);

	bool bEnable = !strText.IsEmpty();
	GetDlgItem(IDOK)->EnableWindow(bEnable);
	GetDlgItem(IDC_FIND_ALL)->EnableWindow(bEnable);
}

void CFindDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if (bShow)
		UpdateButtons();
}
