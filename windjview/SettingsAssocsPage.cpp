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
#include "SettingsAssocsPage.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSettingsAssocsPage dialog

IMPLEMENT_DYNAMIC(CSettingsAssocsPage, CPropertyPage)

CSettingsAssocsPage::CSettingsAssocsPage()
	: CPropertyPage(CSettingsAssocsPage::IDD)
{
	m_bRestoreAssocs = theApp.GetAppSettings()->bRestoreAssocs;
}

CSettingsAssocsPage::~CSettingsAssocsPage()
{
}

void CSettingsAssocsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_RESTORE_ASSOCS, m_bRestoreAssocs);
}


BEGIN_MESSAGE_MAP(CSettingsAssocsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_ASSOCIATE, OnAssociate)
END_MESSAGE_MAP()


// CSettingsAssocsPage message handlers

void CSettingsAssocsPage::OnAssociate()
{
	if (theApp.RegisterShellFileTypes())
		AfxMessageBox(IDS_ASSOCIATE_SUCCESSFUL, MB_ICONINFORMATION | MB_OK);
	else
		AfxMessageBox(IDS_ASSOCIATE_FAILED, MB_ICONERROR | MB_OK);
}
