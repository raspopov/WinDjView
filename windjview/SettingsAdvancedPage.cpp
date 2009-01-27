//	WinDjView
//	Copyright (C) 2004-2008 Andrew Zhezherun
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

// $Id$

#include "stdafx.h"
#include "WinDjView.h"
#include "AppSettings.h"
#include "SettingsAdvancedPage.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSettingsAdvancedPage dialog

IMPLEMENT_DYNAMIC(CSettingsAdvancedPage, CPropertyPage)

CSettingsAdvancedPage::CSettingsAdvancedPage()
	: CPropertyPage(CSettingsAdvancedPage::IDD)
{
	m_bWarnNotDefaultViewer = theApp.GetAppSettings()->bWarnNotDefaultViewer;
	m_bRestoreView = theApp.GetAppSettings()->bRestoreView;
}

CSettingsAdvancedPage::~CSettingsAdvancedPage()
{
}

void CSettingsAdvancedPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_WARN_NOT_DEFAULT_VIEWER, m_bWarnNotDefaultViewer);
	DDX_Check(pDX, IDC_RESTORE_VIEW, m_bRestoreView);
}


BEGIN_MESSAGE_MAP(CSettingsAdvancedPage, CPropertyPage)
	ON_BN_CLICKED(IDC_MAKE_DEFAULT, OnMakeDefault)
	ON_BN_CLICKED(IDC_CREATE_BACKUP, OnBackup)
END_MESSAGE_MAP()


// CSettingsAssocsPage message handlers

BOOL CSettingsAdvancedPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	GetDlgItem(IDC_MAKE_DEFAULT)->SendMessage(BCM_SETSHIELD, 0, true);
	UpdateButtons();

	return true;
}

void CSettingsAdvancedPage::OnMakeDefault()
{
	if (theApp.RegisterShellFileTypesElevate(this))
		UpdateButtons();
	else
		AfxMessageBox(IDS_MAKE_DEFAULT_FAILED, MB_ICONERROR | MB_OK);
}

void CSettingsAdvancedPage::UpdateButtons()
{
	bool bNotDefault = theApp.RegisterShellFileTypes(true);
	GetDlgItem(IDC_DEFAULT_VIEWER_STATUS)->SetWindowText(LoadString(
		bNotDefault ? IDS_NOT_DEFAULT_VIEWER : IDS_DEFAULT_VIEWER));
	GetDlgItem(IDC_MAKE_DEFAULT)->EnableWindow(bNotDefault);
}

void CSettingsAdvancedPage::OnBackup()
{
}
