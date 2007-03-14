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
#include "SettingsDictPage.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSettingsDictPage dialog

IMPLEMENT_DYNAMIC(CSettingsDictPage, CPropertyPage)

CSettingsDictPage::CSettingsDictPage()
	: CPropertyPage(CSettingsDictPage::IDD)
{
}

CSettingsDictPage::~CSettingsDictPage()
{
}

void CSettingsDictPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CSettingsDictPage, CPropertyPage)
	ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
	ON_BN_CLICKED(IDC_UNINSTALL, OnUninstall)
END_MESSAGE_MAP()


// CSettingsDictPage message handlers

BOOL CSettingsDictPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// Image list is used to change row height
	m_imgList.Create(1, 16, ILC_COLOR24 | ILC_MASK, 0, 1);
	m_list.SetImageList(&m_imgList, LVSIL_SMALL);

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	CRect rcClient;
	m_list.GetClientRect(rcClient);

	m_list.InsertColumn(0, _T(""), LVCFMT_LEFT, rcClient.Width() - 4);

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
