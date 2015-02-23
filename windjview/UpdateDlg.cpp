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
#include "UpdateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUpdateDlg dialog

IMPLEMENT_DYNAMIC(CUpdateDlg, CMyDialog)

CUpdateDlg::CUpdateDlg(CWnd* pParent)
	: CMyDialog(CUpdateDlg::IDD, pParent), m_hThread(NULL), m_bOk(false)
{
}

CUpdateDlg::~CUpdateDlg()
{
	if (m_hThread != NULL)
		::CloseHandle(m_hThread);
}

void CUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CUpdateDlg, CMyDialog)
	ON_WM_SHOWWINDOW()
	ON_MESSAGE_VOID(WM_ENDDIALOG, OnEndDialog)
END_MESSAGE_MAP()


// CUpdateDlg message handlers

void CUpdateDlg::OnOK()
{
}

void CUpdateDlg::OnCancel()
{
}

void CUpdateDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CMyDialog::OnShowWindow(bShow, nStatus);

	if (bShow && m_hThread == NULL)
	{
		UINT nThreadId;
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, UpdateThreadProc, this, 0, &nThreadId);
		theApp.ThreadStarted();
	}
}

unsigned int __stdcall CUpdateDlg::UpdateThreadProc(void* pvData)
{
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	CUpdateDlg* pDlg = reinterpret_cast<CUpdateDlg*>(pvData);

	CString strNewVersion = CDjViewApp::DownloadLastVersionString();
	pDlg->m_bOk = !strNewVersion.IsEmpty();
	pDlg->m_strNewVersion = strNewVersion;

	pDlg->SendMessage(WM_ENDDIALOG);

	theApp.ThreadTerminated();
	return 0;
}

void CUpdateDlg::OnEndDialog()
{
	if (m_bOk)
	{
		if (CompareVersions(m_strNewVersion, CURRENT_VERSION) > 0)
		{
			if (AfxMessageBox(FormatString(IDS_NEW_VERSION_AVAILABLE, m_strNewVersion),
					MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				::ShellExecute(NULL, _T("open"), LoadString(IDS_WEBSITE_URL),
						NULL, NULL, SW_SHOW);
			}
		}
		else
			AfxMessageBox(IDS_NO_UPDATES_AVAILABLE, MB_ICONINFORMATION | MB_OK);
	}
	else
		AfxMessageBox(IDS_CONNECT_ERROR, MB_ICONEXCLAMATION | MB_OK);

	EndDialog(IDOK);
}
