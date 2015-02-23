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
#include "SettingsDictPage.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSettingsDictPage dialog

IMPLEMENT_DYNAMIC(CSettingsDictPage, CPropertyPage)

CSettingsDictPage::CSettingsDictPage()
	: CPropertyPage(CSettingsDictPage::IDD)
{
	m_strDictLocation = theApp.GetAppSettings()->strDictLocation;

	PathRemoveBackslash(m_strDictLocation.GetBuffer(_MAX_PATH));
	m_strDictLocation.ReleaseBuffer();
}

CSettingsDictPage::~CSettingsDictPage()
{
}

void CSettingsDictPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);

	if (!m_strDictLocation.IsEmpty())
		DDX_Text(pDX, IDC_CUSTOM_LOCATION, m_strDictLocation);
}


BEGIN_MESSAGE_MAP(CSettingsDictPage, CPropertyPage)
	ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
	ON_BN_CLICKED(IDC_UNINSTALL, OnUninstall)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BROWSE_LOCATION, OnBrowseLocation)
END_MESSAGE_MAP()


// CSettingsDictPage message handlers

BOOL CSettingsDictPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// Image list is used to change row height
	m_imgList.Create(1, 16, ILC_COLOR24 | ILC_MASK, 0, 1);
	m_list.SetImageList(&m_imgList, LVSIL_SMALL);

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);

	CSize szList = GetClientSize(m_list);
	m_list.InsertColumn(0, _T(""), LVCFMT_LEFT, szList.cx - 4);

	for (int nLang = 0; nLang < theApp.GetDictLangsCount(); ++nLang)
	{
		for (int nDict = 0; nDict < theApp.GetDictionaryCount(nLang); ++nDict)
		{
			DictionaryInfo* pInfo = theApp.GetDictionaryInfo(nLang, nDict);
			int nItem = m_list.InsertItem(m_list.GetItemCount(), pInfo->strTitle);
			m_list.SetItemData(nItem, reinterpret_cast<DWORD>(pInfo));
		}
	}

	return true;
}

void CSettingsDictPage::OnKickIdle()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	GetDlgItem(IDC_UNINSTALL)->EnableWindow(pos != NULL);
}

void CSettingsDictPage::OnUninstall()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos == NULL)
		return;

	int nItem = m_list.GetNextSelectedItem(pos);
	DictionaryInfo* pInfo = reinterpret_cast<DictionaryInfo*>(m_list.GetItemData(nItem));

	if (AfxMessageBox(FormatString(IDS_UNINSTALL_DICT, pInfo->strTitle),
			MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		if (theApp.UninstallDictionary(pInfo))
		{
			m_list.DeleteItem(nItem);
		}
		else
		{
			AfxMessageBox(FormatString(IDS_UNINSTALL_FAILED, pInfo->strTitle, pInfo->strPathName),
					MB_ICONEXCLAMATION | MB_OK);
		}
	}
}

HBRUSH CSettingsDictPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH brush = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_CUSTOM_LOCATION && m_strDictLocation.IsEmpty())
	{
		pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
	}

	return brush;
}

int CALLBACK CSettingsDictPage::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
		CSettingsDictPage* pDlg = (CSettingsDictPage*) lpData;
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

void CSettingsDictPage::OnBrowseLocation()
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
		PathRemoveBackslash(szPath);
		m_strDictLocation = szPath;
		UpdateData(false);
	}

	LPMALLOC pMalloc;
	if (SUCCEEDED(SHGetMalloc(&pMalloc)))
		pMalloc->Free(pItem);
}
