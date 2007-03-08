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
#include "InstallDicDlg.h"
#include "DjVuSource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CInstallDicDlg dialog

IMPLEMENT_DYNAMIC(CInstallDicDlg, CDialog)

CInstallDicDlg::CInstallDicDlg(UINT nID, CWnd* pParent)
	: CDialog(nID, pParent), m_nTemplateID(nID), m_bKeepOriginal(true), m_nChoice(0)
{
}

CInstallDicDlg::~CInstallDicDlg()
{
}

void CInstallDicDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_KEEP_ORIGINAL, m_bKeepOriginal);
	if (m_nTemplateID == IDD_INSTALL_DIC)
		DDX_Radio(pDX, IDC_CURRENT_USER, m_nChoice);
}


BEGIN_MESSAGE_MAP(CInstallDicDlg, CDialog)
END_MESSAGE_MAP()


// CInstallDicDlg message handlers

BOOL CInstallDicDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	return true;
}
