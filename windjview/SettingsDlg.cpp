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
#include "SettingsDlg.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSettingsDlg dialog

IMPLEMENT_DYNAMIC(CSettingsDlg, CPropertySheet)

CSettingsDlg::CSettingsDlg(CWnd* pParent)
	: CPropertySheet(IDS_SETTINGS, pParent)
{
	AddPage(&m_pageGeneral);
	AddPage(&m_pageDisplay);
	AddPage(&m_pageAssocs);

	m_psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
}

CSettingsDlg::~CSettingsDlg()
{
}


BEGIN_MESSAGE_MAP(CSettingsDlg, CPropertySheet)
	ON_WM_CTLCOLOR()
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CSettingsDlg message handlers

int CSettingsDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	EnableStackedTabs(false);

	if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

BOOL CSettingsDlg::OnInitDialog()
{
	CPropertySheet::OnInitDialog();

	CFont fnt;
	CreateSystemDialogFont(fnt);
	LOGFONT lf;
	fnt.GetLogFont(&lf);

	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	lf.lfHeight = -MulDiv(7, dcScreen.GetDeviceCaps(LOGPIXELSY), 72);
	m_font.CreateFontIndirect(&lf);

	CWnd* pOK = GetDlgItem(IDOK);
	CWnd* pPage = GetActivePage();

	CRect rcOk, rcPage, rcClient;
	pOK->GetWindowRect(rcOk);
	ScreenToClient(rcOk);
	pPage->GetWindowRect(rcPage);
	ScreenToClient(rcPage);
	GetClientRect(rcClient);

	m_ctlAbout.Create(FormatString(IDS_VERSION_INFO, CURRENT_VERSION), WS_CHILD | WS_VISIBLE,
		CRect(rcPage.left, rcOk.top, rcOk.left - 5, rcClient.bottom - 5), this, IDC_STATIC_ABOUT);
	m_ctlAbout.SetFont(&m_font);

	return true;
}

HBRUSH CSettingsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH brush = CPropertySheet::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_ABOUT)
	{
		pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
	}

	return brush;
}
