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
//  http://www.gnu.org/copyleft/gpl.html

// $Id$

#include "stdafx.h"
#include "WinDjView.h"
#include "SettingsDlg.h"

#include "AppSettings.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSettingsDlg dialog

IMPLEMENT_DYNAMIC(CSettingsDlg, CDialog)
CSettingsDlg::CSettingsDlg(CWnd* pParent)
	: CDialog(CSettingsDlg::IDD, pParent)
{
	m_bRestoreAssocs = CAppSettings::bRestoreAssocs;
}

CSettingsDlg::~CSettingsDlg()
{
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_RESTORE_ASSOCS, m_bRestoreAssocs);
	DDX_Control(pDX, IDC_STATIC_ABOUT, m_ctlAbout);
}


BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
	ON_BN_CLICKED(IDC_ASSOCIATE, OnAssociate)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CSettingsDlg message handlers

void CSettingsDlg::OnAssociate()
{
	if (theApp.RegisterShellFileTypes())
	{
		AfxMessageBox(
			_T("Now you will be able to open .djvu files ")
			_T("with WinDjView\nby double-clicking them in ")
			_T("the explorer."), MB_ICONINFORMATION | MB_OK);
	}
	else
	{
		AfxMessageBox(
			_T("An error occurred while trying to register file associations.\n")
			_T("You may not have enough permissions to write system registry."),
			MB_ICONERROR | MB_OK);
	}
}

void CSettingsDlg::OnOK()
{
	if (!UpdateData())
		return;

	CAppSettings::bRestoreAssocs = !!m_bRestoreAssocs;

	CDialog::OnOK();
}

BOOL CSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFont fnt;
	CreateSystemDialogFont(fnt);
	LOGFONT lf;
	fnt.GetLogFont(&lf);

	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	lf.lfHeight = -MulDiv(7, dcScreen.GetDeviceCaps(LOGPIXELSY), 72);
	m_font.CreateFontIndirect(&lf);

	m_ctlAbout.SetFont(&m_font);

	return TRUE;
}

HBRUSH CSettingsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH brush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetSafeHwnd() == m_ctlAbout.m_hWnd)
	{
		pDC->SetTextColor(::GetSysColor(COLOR_3DSHADOW));
	}

	return brush;
}
