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
//	51 Franklin Street, Fifth Floor, Boston, MA 02111-1307 USA.
//	http://www.gnu.org/copyleft/gpl.html

#include "stdafx.h"
#include "WinDjView.h"
#include "SettingsDlg.h"
#include "Drawing.h"

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
	AddPage(&m_pageAdvanced);

	if (theApp.GetDictLangsCount() > 0)
		AddPage(&m_pageDict);

	m_psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
}

CSettingsDlg::~CSettingsDlg()
{
}


BEGIN_MESSAGE_MAP(CSettingsDlg, CPropertySheet)
	ON_WM_CTLCOLOR()
	ON_WM_CREATE()
	ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
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

	CScreenDC dcScreen;

	lf.lfHeight = -MulDiv(7, dcScreen.GetDeviceCaps(LOGPIXELSY), 72);
	m_font.CreateFontIndirect(&lf);

	CWnd* pOK = GetDlgItem(IDOK);
	CWnd* pPage = GetActivePage();

	CRect rcOk, rcPage;
	pOK->GetWindowRect(rcOk);
	ScreenToClient(rcOk);
	pPage->GetWindowRect(rcPage);
	ScreenToClient(rcPage);
	CRect rcClient = ::GetClientRect(this);

	m_ctlAbout.Create(FormatString(IDS_VERSION_INFO, CURRENT_VERSION), WS_CHILD | WS_VISIBLE,
		CRect(rcPage.left, rcOk.top, rcOk.left - 5, rcClient.bottom - 5), this, IDC_STATIC_ABOUT);
	m_ctlAbout.SetFont(&m_font);

	int nPage = theApp.GetAppSettings()->nActiveSettingsTab;
	nPage = max(0, min(GetPageCount() - 1, nPage));
	SetActivePage(nPage);

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

void CSettingsDlg::OnKickIdle()
{
	SendMessageToDescendants(WM_KICKIDLE);

	theApp.GetAppSettings()->nActiveSettingsTab = GetActiveIndex();
}

INT_PTR CSettingsDlg::DoModal()
{
	set<CWnd*> disabled;
	theApp.DisableTopLevelWindows(disabled);

	INT_PTR nResult = CPropertySheet::DoModal();

	theApp.EnableWindows(disabled);
	return nResult;
}
