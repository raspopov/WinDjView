//	WinDjView
//	Copyright (C) 2004-2005 Andrew Zhezherun
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
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//	http://www.gnu.org/copyleft/gpl.html

// $Id$

#include "stdafx.h"
#include "WinDjView.h"
#include "UpdateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUpdateDlg dialog

IMPLEMENT_DYNAMIC(CUpdateDlg, CDialog)
CUpdateDlg::CUpdateDlg(CWnd* pParent)
	: CDialog(CUpdateDlg::IDD, pParent), m_hThread(NULL)
{
}

CUpdateDlg::~CUpdateDlg()
{
	if (m_hThread != NULL)
		::CloseHandle(m_hThread);
}

void CUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CUpdateDlg, CDialog)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CUpdateDlg message handlers

void CUpdateDlg::OnOK()
{
}

void CUpdateDlg::OnCancel()
{
}

BOOL CUpdateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;
}

void CUpdateDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if (bShow)
	{
		DWORD dwThreadId;
		m_hThread = ::CreateThread(NULL, 0, UpdateThreadProc, this, 0, &dwThreadId);
	}
}

DWORD WINAPI CUpdateDlg::UpdateThreadProc(LPVOID pvData)
{
	CUpdateDlg* pDlg = reinterpret_cast<CUpdateDlg*>(pvData);

	CString strVersion;
	bool bOk = true;

	try
	{
		CWaitCursor wait;

		CInternetSession session;
		session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 30000);

		CFile* pFile = session.OpenURL(_T("http://windjview.sourceforge.net/version.txt"), 1,
			INTERNET_FLAG_TRANSFER_ASCII | INTERNET_FLAG_RELOAD);
		if (pFile == NULL)
			AfxThrowInternetException(1);

		LPTSTR pszBuffer = strVersion.GetBuffer(1024);
		int nRead = pFile->Read(pszBuffer, 1023);
		pszBuffer[nRead] = '\0';

		strVersion.ReleaseBuffer();
	}
	catch (CException* e)
	{
		bOk = false;
		e->Delete();

		AfxMessageBox(_T("An error occurred while connecting to the server. Please check your internet settings."),
			MB_ICONEXCLAMATION | MB_OK);
	}

	if (bOk)
	{
		strVersion.TrimLeft();
		strVersion.TrimRight();

		if (strVersion == CURRENT_VERSION)
		{
			AfxMessageBox(_T("No updates are available at this time. Please check again later."),
				MB_ICONINFORMATION | MB_OK);
		}
		else
		{
			CString strMessage = _T("A new version ") + strVersion
				+ _T(" is available. Do you want to go to program website now?");
			if (AfxMessageBox(strMessage, MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				::ShellExecute(NULL, "open", "http://windjview.sourceforge.net/",
					NULL, NULL, SW_SHOWNORMAL);
			}
		}
	}

	pDlg->EndDialog(IDOK);
	return 0;
}
