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

#include "stdafx.h"
#include "WinDjView.h"
#include "FindDlg.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFindDlg dialog

IMPLEMENT_DYNAMIC(CFindDlg, CMyDialog)

CFindDlg::CFindDlg(CWnd* pParent)
	: CMyDialog(CFindDlg::IDD, pParent)
{
	m_bMatchCase = theApp.GetAppSettings()->bMatchCase;
	m_strFind = theApp.GetAppSettings()->strFind;
}

CFindDlg::~CFindDlg()
{
}

void CFindDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FIND, m_cboFind);
	DDX_Text(pDX, IDC_FIND, m_strFind);
	DDX_Check(pDX, IDC_MATCH_CASE, m_bMatchCase);
}


BEGIN_MESSAGE_MAP(CFindDlg, CMyDialog)
	ON_BN_CLICKED(IDC_FIND_ALL, OnFindAll)
	ON_WM_CLOSE()
	ON_CBN_EDITCHANGE(IDC_FIND, OnEnChangeFind)
	ON_WM_SHOWWINDOW()
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()


// CFindDlg message handlers

BOOL CFindDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();

	m_cboFind.SetExtendedStyle(CBES_EX_CASESENSITIVE | CBES_EX_NOEDITIMAGE,
			CBES_EX_CASESENSITIVE | CBES_EX_NOEDITIMAGE);
	m_cboFind.GetComboBoxCtrl()->ModifyStyle(CBS_SORT | CBS_NOINTEGRALHEIGHT,
			CBS_AUTOHSCROLL);

	InitSearchHistory();

	return true;
}

CMainFrame* CFindDlg::GetMainFrame()
{
	return (CMainFrame*) GetParent();
}

void CFindDlg::OnOK()
{
	if (!UpdateData())
		return;

	UpdateSearchHistory();

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

	UpdateSearchHistory();

	SetStatusText(LoadString(IDS_SEARCHING));
	GetDlgItem(IDC_SEARCH_STATUS)->ShowWindow(SW_SHOW);

	GetMainFrame()->SetMessageText(AFX_IDS_IDLEMESSAGE);
	GetParent()->SendMessage(WM_COMMAND, ID_FIND_ALL);

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
	m_cboFind.GetWindowText(strText);

	bool bEnable = !strText.IsEmpty();
	GetDlgItem(IDOK)->EnableWindow(bEnable);
	GetDlgItem(IDC_FIND_ALL)->EnableWindow(bEnable);
}

void CFindDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CMyDialog::OnShowWindow(bShow, nStatus);

	if (bShow)
	{
		UpdateButtons();
		m_cboFind.SetFocus();

		// Remove the "default" visual style that buttons could
		// erroneously retain after they were clicked.
		GetDlgItem(IDC_FIND_ALL)->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		GetDlgItem(IDCANCEL)->ModifyStyle(BS_DEFPUSHBUTTON, 0);
	}
}

void CFindDlg::InitSearchHistory()
{
	m_cboFind.GetWindowText(m_strFind);
	theApp.InitSearchHistory(m_cboFind);
}

void CFindDlg::UpdateSearchHistory()
{
	if (!m_strFind.IsEmpty())
	{
		theApp.UpdateSearchHistory(m_cboFind);
		theApp.GetAppSettings()->strFind = m_strFind;
	}
}

void CFindDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	if (nState == WA_ACTIVE || nState == WA_CLICKACTIVE)
		InitSearchHistory();

	CMyDialog::OnActivate(nState, pWndOther, bMinimized);
}

BOOL CFindDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F3)
	{
		bool bShiftPressed = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
		bool bControlPressed = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
		bool bAltPressed = (::GetKeyState(VK_MENU) & 0x8000) != 0;
		if (!bControlPressed && !bShiftPressed && !bAltPressed)
		{
			OnOK();
			return true;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}
