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
#include "SettingsAdvancedPage.h"
#include "MyFileDialog.h"


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
	m_bCheckUpdates = theApp.GetAppSettings()->bCheckUpdates;
}

CSettingsAdvancedPage::~CSettingsAdvancedPage()
{
}

void CSettingsAdvancedPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_WARN_NOT_DEFAULT_VIEWER, m_bWarnNotDefaultViewer);
	DDX_Check(pDX, IDC_RESTORE_VIEW, m_bRestoreView);
	DDX_Check(pDX, IDC_CHECK_UPDATES, m_bCheckUpdates);
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
	CMyFileDialog dlg(false, _T("reg"), _T("WinDjView.reg"), OFN_OVERWRITEPROMPT |
		OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST,
		LoadString(IDS_REGISTRY_FILTER));

	CString strTitle;
	strTitle.LoadString(IDS_BACKUP_BOOKMARKS);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

	if (dlg.DoModal() != IDOK)
		return;

	CWaitCursor wait;

	CString strPathName = dlg.GetPathName();
	CFile file;
	if (!file.Open(strPathName, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive))
	{
		AfxMessageBox(FormatString(IDS_CANNOT_WRITE_TO_FILE, strPathName), MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	string text;

	text = "REGEDIT4\r\n\r\n";
	file.Write(text.c_str(), (UINT)text.length());

	vector<CString> keys;
	theApp.EnumProfileKeys(s_pszDocumentsSection, keys);
	for (size_t i = 0; i < keys.size(); ++i)
	{
		CString strKey = s_pszDocumentsSection + CString(_T("\\")) + keys[i];
		CString strFileName = theApp.GetProfileString(strKey, s_pszLastKnownLocation, _T(""));

		CString strFullKey = CString(_T("HKEY_CURRENT_USER\\Software\\"))
				+ theApp.m_pszRegistryKey + _T("\\") + theApp.m_pszProfileName + _T("\\")
				+ s_pszDocumentsSection + _T("\\") + keys[i];
		MakeANSIString(_T("[") + strFullKey + _T("]\r\n"), text);
		file.Write(text.c_str(), (UINT)text.length());

		if (!strFileName.IsEmpty())
		{
			strFileName.Replace(_T("\\"), _T("\\\\"));
			MakeANSIString(CString(_T("\"")) + s_pszLastKnownLocation + _T("\"=\"")
					+ strFileName + _T("\"\r\n"), text);
			file.Write(text.c_str(), (UINT)text.length());
		}

		LPBYTE pBuf;
		UINT nSize;
		if (theApp.GetProfileBinary(strKey, s_pszSettings, &pBuf, &nSize))
		{
			if (nSize > 0)
			{
				MakeANSIString(CString(_T("\"")) + s_pszSettings + _T("\"=hex:"), text);
				size_t nLineLength = text.length();
				for (size_t i = 0; i < nSize; ++i)
				{
					string block;
					if (nLineLength >= 76)
					{
						block = "\\\r\n  ";
						if (text.length() + block.length() > text.capacity())
							text.reserve(max(2*text.length(), text.length() + block.length()));
						text += block;
						nLineLength = 2;
					}

					MakeANSIString(FormatString(_T("%02x%s"), pBuf[i],
							i + 1 < nSize ? _T(",") : _T("")), block);

					if (text.length() + block.length() > text.capacity())
						text.reserve(max(2*text.length(), text.length() + block.length()));
					text += block;
					nLineLength += block.length();
				}
				text += "\r\n\r\n";
				file.Write(text.c_str(), (UINT)text.length());
			}

			delete[] pBuf;
		}
	}

	file.Close();
}
