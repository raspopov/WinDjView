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
//	51 Franklin Street, Fifth Floor, Boston, MA 02111-1307 USA.
//	http://www.gnu.org/copyleft/gpl.html

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

IMPLEMENT_DYNAMIC(CMyFileDialog, CDialog)

CMyFileDialog::CMyFileDialog(bool bOpenFileDialog, LPCTSTR lpszDefExt,
		LPCTSTR lpszFileName, DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd)
	: CCommonDialog(pParentWnd)
{
	ASSERT((dwFlags & (OFN_ENABLETEMPLATE | OFN_ENABLEHOOK)) == 0);

	ZeroMemory(&m_ofn, sizeof(m_ofn));
	ZeroMemory(m_szFileName, sizeof(m_szFileName));
	ZeroMemory(m_szFileTitle, sizeof(m_szFileTitle));

	m_ofn.lStructSize = sizeof(m_ofn);

	m_bOpenFileDialog = bOpenFileDialog;
	m_nIDHelp = bOpenFileDialog ? AFX_IDD_FILEOPEN : AFX_IDD_FILESAVE;

	m_ofn.lpstrFile = m_szFileName;
	m_ofn.nMaxFile = _MAX_PATH + 1;
	m_ofn.lpstrDefExt = lpszDefExt;
	m_ofn.lpstrFileTitle = m_szFileTitle;
	m_ofn.nMaxFileTitle = _MAX_PATH + 1;
	m_ofn.Flags |= dwFlags | OFN_EXPLORER | OFN_ENABLESIZING;
	m_ofn.hInstance = AfxGetResourceHandle();

	// setup initial file name
	if (lpszFileName != NULL)
		_tcsncpy(m_szFileName, lpszFileName, _MAX_PATH);

	// Translate filter into commdlg format (lots of \0)
	if (lpszFilter != NULL)
	{
		m_strFilter = lpszFilter;
		LPTSTR pch = m_strFilter.GetBuffer(0); // modify the buffer in place
		// MFC delimits with '|' not '\0'
		while ((pch = _tcschr(pch, '|')) != NULL)
			*pch++ = '\0';
		m_ofn.lpstrFilter = m_strFilter;
		// do not call ReleaseBuffer() since the string contains '\0' characters
	}
}


BEGIN_MESSAGE_MAP(CMyFileDialog, CCommonDialog)
END_MESSAGE_MAP()


INT_PTR CMyFileDialog::DoModal()
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

	int nResult;
	if (m_bOpenFileDialog)
		nResult = ::GetOpenFileName(&m_ofn);
	else
		nResult = ::GetSaveFileName(&m_ofn);

	if (nResult)
		ASSERT(pThreadState->m_pAlternateWndInit == NULL);
	pThreadState->m_pAlternateWndInit = NULL;

	::UnhookWindowsHookEx(pMyData->hHook);
	pMyData->hHook = NULL;

	// WINBUG: Second part of special case for file open/save dialog.
	if (bEnableParent)
		::EnableWindow(m_ofn.hwndOwner, TRUE);
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
			ASSERT_KINDOF(CMyFileDialog, pThreadState->m_pAlternateWndInit);
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
	return CCommonDialog::OnNotify(wParam, lParam, pResult);
}

CString CMyFileDialog::GetPathName() const
{
	ASSERT(m_hWnd == NULL);
	return m_ofn.lpstrFile;
}

CString CMyFileDialog::GetFileName() const
{
	ASSERT(m_hWnd == NULL);
	return m_ofn.lpstrFileTitle;
}

CString CMyFileDialog::GetFileExt() const
{
	ASSERT(m_hWnd == NULL);
	if (m_ofn.nFileExtension == 0)
		return _T("");
	else
		return m_ofn.lpstrFile + m_ofn.nFileExtension;
}

CString CMyFileDialog::GetFileTitle() const
{
	CString strResult = GetFileName();
	LPTSTR pszBuffer = strResult.GetBuffer(0);
	::PathRemoveExtension(pszBuffer);
	strResult.ReleaseBuffer();
	return strResult;
}

bool CMyFileDialog::GetReadOnlyPref() const
{
	ASSERT(m_hWnd == NULL);
	return (m_ofn.Flags & OFN_READONLY) != 0;
}
