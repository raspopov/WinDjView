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
#include "UpdateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUpdateDlg dialog

IMPLEMENT_DYNAMIC(CUpdateDlg, CDialog)
CUpdateDlg::CUpdateDlg(CWnd* pParent)
	: CDialog(CUpdateDlg::IDD, pParent), m_hThread(NULL), m_bOk(false)
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
	CDialog::OnShowWindow(bShow, nStatus);

	if (bShow && m_hThread == NULL)
	{
		UINT nThreadId;
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, UpdateThreadProc, this, 0, &nThreadId);
	}
}

unsigned int __stdcall CUpdateDlg::UpdateThreadProc(void* pvData)
{
	CUpdateDlg* pDlg = reinterpret_cast<CUpdateDlg*>(pvData);

	CString strVersion;
	bool bOk = true;

	try
	{
		CWaitCursor wait;

		CInternetSession session;
		session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 30000);

		CFile* pFile = session.OpenURL(LoadString(IDS_VERSION_URL), 1,
			INTERNET_FLAG_TRANSFER_ASCII | INTERNET_FLAG_RELOAD);
		if (pFile == NULL)
			AfxThrowInternetException(1);

		CHAR szBuffer[1024];
		int nRead = pFile->Read(szBuffer, 1023);
		szBuffer[nRead] = '\0';

		strVersion = szBuffer;
		strVersion.TrimLeft();
		strVersion.TrimRight();
	}
	catch (CException* e)
	{
		bOk = false;
		e->Delete();
	}

	pDlg->m_bOk = bOk;
	pDlg->m_strVersion = strVersion;

	pDlg->SendMessage(WM_ENDDIALOG);
	return 0;
}

void CUpdateDlg::OnEndDialog()
{
	if (m_bOk)
	{
		if (m_strVersion == CURRENT_VERSION)
		{
			AfxMessageBox(IDS_NO_UPDATES_AVAILABLE, MB_ICONINFORMATION | MB_OK);
		}
		else if (!m_strVersion.IsEmpty() && m_strVersion.GetLength() < 16 && m_strVersion.Find('<') == -1)
		{
			if (AfxMessageBox(FormatString(IDS_NEW_VERSION_AVAILABLE, m_strVersion),
					MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				::ShellExecute(NULL, _T("open"), LoadString(IDS_WEBSITE_URL),
					NULL, NULL, SW_SHOWNORMAL);
			}
		}
		else
		{
			AfxMessageBox(IDS_CONNECT_ERROR, MB_ICONINFORMATION | MB_OK);
		}
	}
	else
	{
		AfxMessageBox(IDS_CONNECT_ERROR, MB_ICONEXCLAMATION | MB_OK);
	}

	EndDialog(IDOK);
}
