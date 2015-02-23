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

#pragma once

#if (_WIN32_WINNT >= 0x0500) && defined(OPENFILENAME_SIZE_VERSION_400)
typedef OPENFILENAME OpenFileNameEx;
#else
struct OpenFileNameEx : public OPENFILENAME
{
	LPVOID pvReserved;
	DWORD dwReserved;
	DWORD FlagsEx;
};
#endif


// CMyFileDialog

class CMyFileDialog : public CCommonDialog
{
	DECLARE_DYNAMIC(CMyFileDialog)

public:
	explicit CMyFileDialog(bool bOpenFileDialog,  // true for open, false for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

// Operations
public:
	CString GetPathName() const;  // Return full path and filename
	CString GetFileName() const;  // Return only filename
	CString GetFileExt() const;   // Return only ext
	CString GetFileTitle() const; // Return file title
	bool GetReadOnlyPref() const; // Return true if readonly checked

	OpenFileNameEx m_ofn;  // Windows 2000/XP version of OPENFILENAME

// Overrides
public:
	virtual INT_PTR DoModal();

protected:
	bool m_bOpenFileDialog;
	CString m_strFilter;  // Separate fields with '|', terminate with '||\0'
	TCHAR m_szFileTitle[_MAX_PATH + 1];  // Contains file title after return
	TCHAR m_szFileName[_MAX_PATH + 1];  // Contains full path name after return

	static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
