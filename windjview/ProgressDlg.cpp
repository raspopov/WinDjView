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
#include "ProgressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CProgressDlg dialog

IMPLEMENT_DYNAMIC(CProgressDlg, CMyDialog)

CProgressDlg::CProgressDlg(ThreadProcEx pfnThreadProc, CWnd* pParent)
	: CMyDialog(CProgressDlg::IDD, pParent), m_nCode(0), m_nCancelled(0),
	  m_pfnThreadProc(pfnThreadProc), m_dwUserData(0), m_hThread(NULL)
{
}

CProgressDlg::~CProgressDlg()
{
	if (m_hThread != NULL)
		::CloseHandle(m_hThread);
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_STATIC_TEXT, m_status);
}


BEGIN_MESSAGE_MAP(CProgressDlg, CMyDialog)
	ON_MESSAGE(WM_ENDDIALOG, OnEndDialog)
END_MESSAGE_MAP()


// CProgressDlg message handlers

void CProgressDlg::SetStatus(const CString& strText)
{
	m_status.SetWindowText(strText);
}

void CProgressDlg::SetRange(int nStart, int nEnd)
{
	m_progress.SetRange32(nStart, nEnd);

	Invalidate(false);
	UpdateWindow();
}

void CProgressDlg::SetPos(int nPos)
{
	m_progress.SetPos(nPos);

	Invalidate(false);
	UpdateWindow();
}

bool CProgressDlg::IsCancelled()
{
	return (InterlockedExchangeAdd(&m_nCancelled, 0) == 1);
}

long CProgressDlg::GetResultCode()
{
	return InterlockedExchangeAdd(&m_nCode, 0);
}

void CProgressDlg::StopProgress(long nCode)
{
	InterlockedExchange(&m_nCode, nCode);
	SendMessage(WM_ENDDIALOG, m_nCancelled ? IDCANCEL : IDOK);
}

BOOL CProgressDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();

	m_progress.SetRange32(0, 1);
	m_progress.SetPos(0);
	m_status.SetWindowText(LoadString(IDS_PLEASE_WAIT));

	UINT nThreadID;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, m_pfnThreadProc,
		static_cast<IProgressInfo*>(this), 0, &nThreadID);
	::SetThreadPriority(m_hThread, THREAD_PRIORITY_BELOW_NORMAL);

	return true;
}

void CProgressDlg::OnCancel()
{
}

void CProgressDlg::OnOK()
{
	InterlockedExchange(&m_nCancelled, 1);
}

LRESULT CProgressDlg::OnEndDialog(WPARAM wParam, LPARAM lParam)
{
	EndDialog((int)wParam);
	return 0;
}
