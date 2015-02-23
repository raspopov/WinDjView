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

#pragma once

#include "MyDialog.h"
class DjVuSource;

// CDocPropertiesDlg dialog

class CDocPropertiesDlg : public CMyDialog
{
	DECLARE_DYNAMIC(CDocPropertiesDlg)

public:
	CDocPropertiesDlg(DjVuSource* pSource, CWnd* pParent = NULL);
	virtual ~CDocPropertiesDlg();

// Dialog Data
	enum { IDD = IDD_DOC_PROPERTIES };
	CString m_strDocName;
	CString m_strDocType;
	int m_nDocSize;
	int m_nPages;
	int m_nFiles;
	CListCtrl m_listFiles;
	BOOL m_bShowAllFiles;

protected:
	struct FileInfo
	{
		int nIndex;
		int nPageIndex;
		CString strName;
		CString strType;
		int nSize;
	};
	vector<FileInfo> m_files;
	bool m_bHideShowAll;
	int m_nSortBy;
	bool m_bSortAsc;

	void PopulateFiles();
	void SortFiles();
	static int CALLBACK CompareFiles(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	afx_msg void OnShowAllFiles();
	afx_msg void OnSortFiles(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
