//	WinDjView
//	Copyright (C) 2004-2006 Andrew Zhezherun
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

#pragma once


// CProgressDlg dialog
struct IProgressInfo
{
	virtual void SetStatus(const CString& strText) = 0;
	virtual void SetRange(int nStart, int nEnd) = 0;
	virtual void SetPos(int nPos) = 0;
	virtual bool IsCancelled() = 0;
	virtual void StopProgress(int nCode) = 0;
	virtual DWORD_PTR GetUserData() = 0;
};


typedef unsigned int (__stdcall *ThreadProcEx)(void*);

class CProgressDlg : public CDialog, public IProgressInfo
{
	DECLARE_DYNAMIC(CProgressDlg)

public:
	CProgressDlg(ThreadProcEx pfnThreadProc, CWnd* pParent = NULL);   // standard constructor
	virtual ~CProgressDlg();

// Dialog Data
	enum { IDD = IDD_PROGRESS };
	CStatic m_status;
	CProgressCtrl m_progress;

// Operations
public:
	int GetResultCode() { return m_nCode; }
	void SetUserData(DWORD_PTR dwData) { m_dwUserData = dwData; }

	// IProgressInfo implementation
	virtual void SetStatus(const CString& strText);
	virtual void SetRange(int nStart, int nEnd);
	virtual void SetPos(int nPos);
	virtual bool IsCancelled();
	virtual void StopProgress(int nCode);
	virtual DWORD_PTR GetUserData() { return m_dwUserData; }

// Overrides
public:
	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnCancel();
	virtual void OnOK();

// Implementation
protected:
	ThreadProcEx m_pfnThreadProc;
	DWORD_PTR m_nCancelled;
	DWORD_PTR m_nCode;
	DWORD_PTR m_dwUserData;
	HANDLE m_hThread;

// Generated message map functions
	afx_msg LRESULT OnEndDialog(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
