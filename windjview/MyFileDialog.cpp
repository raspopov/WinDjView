//	WinDjView
//	Copyright (C) 2004-2009 Andrew Zhezherun
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
//	51 Franklin Street, Fifth Floor, Boston, MA 02111-1307 USA.
//	http://www.gnu.org/copyleft/gpl.html

// $Id$

#include "stdafx.h"
#include "WinDjView.h"
#include "MyFileDialog.h"
#include "Global.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct MyFileDialogData : public CNoTrackObject
{
	MyFileDialogData() : hHook(NULL) {}
	virtual ~MyFileDialogData() {}
	HHOOK hHook;
};
THREAD_LOCAL(MyFileDialogData, _myFileDlgData)


// CMyFileDialog

IMPLEMENT_DYNAMIC(CMyFileDialog, CFileDialog)

CMyFileDialog::CMyFileDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt,
		LPCTSTR lpszFileName, DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd)
	: CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	m_ofn.Flags |= OFN_EXPLORER | OFN_ENABLESIZING;
	m_ofn.Flags &= ~OFN_ENABLEHOOK;
	m_ofn.lpfnHook = NULL;
}


BEGIN_MESSAGE_MAP(CMyFileDialog, CFileDialog)
END_MESSAGE_MAP()


int CMyFileDialog::DoModal()
{
	// From MFC:  CFileDialog::DoModal
	// Uses the OpenFileNameEx structure on Win2k+

	set<CWnd*> disabled;
	theApp.DisableTopLevelWindows(disabled);

	ASSERT_VALID(this);
	ASSERT((m_ofn.Flags & OFN_EXPLORER) != 0);
	ASSERT((m_ofn.Flags & OFN_ENABLEHOOK) == 0);
	ASSERT(m_ofn.lpfnHook == NULL);  // Not using hooks

	// zero out the file buffer for consistent parsing later
	ASSERT(AfxIsValidAddress(m_ofn.lpstrFile, m_ofn.nMaxFile));
	DWORD nOffset = lstrlen(m_ofn.lpstrFile) + 1;
	ASSERT(nOffset <= m_ofn.nMaxFile);
	ZeroMemory(m_ofn.lpstrFile + nOffset, (m_ofn.nMaxFile - nOffset)*sizeof(TCHAR));

	// WINBUG: This is a special case for the file open/save dialog,
	//  which sometimes pumps while it is coming up but before it has
	//  disabled the main window.
	HWND hWndFocus = ::GetFocus();
	BOOL bEnableParent = FALSE;
	m_ofn.hwndOwner = PreModal();
	if (m_ofn.hwndOwner != NULL && ::IsWindowEnabled(m_ofn.hwndOwner))
	{
		bEnableParent = TRUE;
		::EnableWindow(m_ofn.hwndOwner, FALSE);
	}

	AfxUnhookWindowCreate();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	ASSERT(pThreadState->m_pAlternateWndInit == NULL);
	pThreadState->m_pAlternateWndInit = this;

	// Install our own hook to subclass the file dialog
	MyFileDialogData* pMyData = _myFileDlgData.GetData();
	ASSERT(pMyData->hHook == NULL);
	pMyData->hHook = ::SetWindowsHookEx(WH_CBT, &HookProc, NULL, ::GetCurrentThreadId());

	ZeroMemory(&m_ofnEx, sizeof(m_ofnEx));
	if (IsWin2kOrLater())
	{
		m_ofnEx.lStructSize = sizeof(m_ofnEx);
	}
	else
	{
#if defined(OPENFILENAME_SIZE_VERSION_400)
		m_ofnEx.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#else
		m_ofnEx.lStructSize = sizeof(OPENFILENAME);
#endif
	}

	ASSERT(m_ofnEx.lStructSize >= m_ofn.lStructSize);
	memcpy(&m_ofnEx.hwndOwner, &m_ofn.hwndOwner, m_ofn.lStructSize - sizeof(DWORD));

	int nResult;
	if (m_bOpenFileDialog)
		nResult = ::GetOpenFileName(&m_ofnEx);
	else
		nResult = ::GetSaveFileName(&m_ofnEx);

	memcpy(&m_ofn.hwndOwner, &m_ofnEx.hwndOwner, m_ofn.lStructSize - sizeof(DWORD));

	if (nResult)
		ASSERT(pThreadState->m_pAlternateWndInit == NULL);
	pThreadState->m_pAlternateWndInit = NULL;

	::UnhookWindowsHookEx(pMyData->hHook);
	pMyData->hHook = NULL;

	// WINBUG: Second part of special case for file open/save dialog.
	if (bEnableParent)
		::EnableWindow(m_ofnEx.hwndOwner, TRUE);
	if (::IsWindow(hWndFocus))
		::SetFocus(hWndFocus);

	PostModal();

	theApp.EnableWindows(disabled);
	return nResult ? nResult : IDCANCEL;
}

LRESULT CALLBACK CMyFileDialog::HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (nCode == HCBT_ACTIVATE)
	{
		HWND hWnd = (HWND) wParam;
		ASSERT(hWnd != NULL);

		if (pThreadState->m_pAlternateWndInit != NULL && CWnd::FromHandlePermanent(hWnd) == NULL)
		{
			ASSERT_KINDOF(CFileDialog, pThreadState->m_pAlternateWndInit);
			pThreadState->m_pAlternateWndInit->SubclassWindow(hWnd);

			pThreadState->m_pAlternateWndInit->CenterWindow();
			pThreadState->m_pAlternateWndInit = NULL;
		}
	}

	MyFileDialogData* pMyData = _myFileDlgData.GetData();
	return ::CallNextHookEx(pMyData->hHook, nCode, wParam, lParam);
}

BOOL CMyFileDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	memcpy(&m_ofn.hwndOwner, &m_ofnEx.hwndOwner, m_ofn.lStructSize - sizeof(DWORD));

	return CFileDialog::OnNotify(wParam, lParam, pResult);
}
