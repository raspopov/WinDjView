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
//  http://www.gnu.org/copyleft/gpl.html

// $Id$

#include "stdafx.h"
#include "WinDjView.h"
#include "ProgressDlg.h"


// CProgressDlg dialog

IMPLEMENT_DYNAMIC(CProgressDlg, CDialog)

CProgressDlg::CProgressDlg(LPTHREAD_START_ROUTINE pfnThreadProc, CWnd* pParent)
	: CDialog(CProgressDlg::IDD, pParent),
	  m_pfnThreadProc(pfnThreadProc), m_dwUserData(0)
{
}

CProgressDlg::~CProgressDlg()
{
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_STATIC_TEXT, m_status);
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
END_MESSAGE_MAP()


// CProgressDlg message handlers

void CProgressDlg::SetStatus(const CString& strText)
{
	m_status.SetWindowText(strText);
}

void CProgressDlg::SetRange(int nStart, int nEnd)
{
	m_progress.SetRange32(nStart, nEnd);

	Invalidate();
	UpdateWindow();
}

void CProgressDlg::SetPos(int nPos)
{
	m_progress.SetPos(nPos);

	Invalidate();
	UpdateWindow();
}

bool CProgressDlg::IsCancelled()
{
	return (InterlockedCompareExchange(&m_nCancelled, 1, 1) == 1);
}

void CProgressDlg::StopProgress(int nCode)
{
	InterlockedExchange(&m_nCode, nCode);
	EndDialog(m_nCancelled ? IDCANCEL : IDOK);
}

BOOL CProgressDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_nCancelled = 0;
	m_nCode = 0;
	m_progress.SetRange32(0, 1);
	m_progress.SetPos(0);
	m_status.SetWindowText(_T("Please wait..."));

	DWORD dwThreadID;
	HANDLE hThread = ::CreateThread(NULL, 0, m_pfnThreadProc,
		static_cast<IProgressInfo*>(this), 0, &dwThreadID);
	::SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);

	return true;
}

void CProgressDlg::OnCancel()
{
}

void CProgressDlg::OnOK()
{
	InterlockedExchange(&m_nCancelled, 1);
}
