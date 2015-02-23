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
#include "InstallDicDlg.h"
#include "DjVuSource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CInstallDicDlg dialog

IMPLEMENT_DYNAMIC(CInstallDicDlg, CMyDialog)

CInstallDicDlg::CInstallDicDlg(UINT nID, CWnd* pParent)
	: CMyDialog(nID, pParent), m_nTemplateID(nID), m_bKeepOriginal(true), m_nChoice(0)
{
	m_strDictLocation = theApp.GetAppSettings()->strDictLocation;

	PathRemoveBackslash(m_strDictLocation.GetBuffer(_MAX_PATH));
	m_strDictLocation.ReleaseBuffer();

	m_nChoice = max(0, min(2, theApp.GetAppSettings()->nDictChoice));
	if (m_strDictLocation.IsEmpty() && m_nChoice == 2)
		m_nChoice = 0;
}

CInstallDicDlg::~CInstallDicDlg()
{
}

void CInstallDicDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_KEEP_ORIGINAL, m_bKeepOriginal);

	if (m_nTemplateID == IDD_INSTALL_DIC)
	{
		DDX_Radio(pDX, IDC_CURRENT_USER, m_nChoice);

		if (!m_strDictLocation.IsEmpty())
			DDX_Text(pDX, IDC_CUSTOM_LOCATION, m_strDictLocation);
	}
}


BEGIN_MESSAGE_MAP(CInstallDicDlg, CMyDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BROWSE_LOCATION, OnBrowseLocation)
END_MESSAGE_MAP()


// CInstallDicDlg message handlers

BOOL CInstallDicDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();

	if (m_nTemplateID == IDD_INSTALL_DIC)
	{
		if (!m_strDictLocation.IsEmpty())
			GetDlgItem(IDC_BROWSE_LOCATION)->ShowWindow(SW_HIDE);
		else
			GetDlgItem(IDC_CUSTOM)->EnableWindow(false);
	}
	
	return true;
}

HBRUSH CInstallDicDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH brush = CMyDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_CUSTOM_LOCATION && m_strDictLocation.IsEmpty())
	{
		pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
	}

	return brush;
}

int CALLBACK CInstallDicDlg::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
		CInstallDicDlg* pDlg = (CInstallDicDlg*) lpData;
		if (!pDlg->m_strDictLocation.IsEmpty())
			::SendMessage(hwnd, BFFM_SETSELECTION, 1, (LPARAM)(LPCTSTR) pDlg->m_strDictLocation);
	}
	if (uMsg == BFFM_SELCHANGED)
	{
		LPITEMIDLIST pItem = (LPITEMIDLIST) lParam;

		TCHAR szPath[_MAX_PATH];
		bool bOk = !!SHGetPathFromIDList(pItem, szPath);

		::SendMessage(hwnd, BFFM_ENABLEOK, 0, bOk);
	}
	else if (uMsg == BFFM_VALIDATEFAILED)
	{
		AfxMessageBox(IDS_INVALID_DIR);
		return 1;
	}

	return 0;
}

void CInstallDicDlg::OnBrowseLocation()
{
	TCHAR szPath[_MAX_PATH];
	CString strTitle = LoadString(IDS_DICT_LOCATION_PROMPT);

	BROWSEINFO info;
	ZeroMemory(&info, sizeof(info));
	info.hwndOwner = m_hWnd;
	info.pszDisplayName = szPath;
	info.lpszTitle = strTitle;
	info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_USENEWUI;
	info.lpfn = BrowseCallbackProc;
	info.lParam = (LPARAM) this;

	LPITEMIDLIST pItem = SHBrowseForFolder(&info);
	if (pItem == NULL)
		return;

	if (SHGetPathFromIDList(pItem, szPath))
	{
		GetDlgItem(IDC_CUSTOM)->EnableWindow(true);
		m_nChoice = 2;
		PathRemoveBackslash(szPath);
		m_strDictLocation = szPath;
		UpdateData(false);
	}

	LPMALLOC pMalloc;
	if (SUCCEEDED(SHGetMalloc(&pMalloc)))
		pMalloc->Free(pItem);
}
