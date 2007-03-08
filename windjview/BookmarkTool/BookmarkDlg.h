//	DjVu Bookmark Tool
//	Copyright (C) 2005-2007 Andrew Zhezherun
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

#pragma once

#include "MyDropTarget.h"

class DjVuSource;


// CBookmarkDlg dialog

class CBookmarkDlg : public CDialog, public IMyDropTarget
{
public:
	CBookmarkDlg(CWnd* pParent = NULL);

// Dialog Data
	enum { IDD = IDD_BOOKMARKTOOL };
	CString m_strDjVuFile;
	CString m_strBookmarksFile;
	int m_nBookmarksAction;

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
		DROPEFFECT dropEffect, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDragScroll(CWnd* pWnd, DWORD dwKeyState,
		CPoint point);

// Implementation
protected:
	HICON m_hIcon;
	bool m_bHasDjVuFile;
	bool m_bHasBookmarksFile;
	GP<DjVmNav> m_pBookmarks;
	CMyDropTarget m_dropTarget;

	bool ExportBookmarks(const CString& strBookmarkFile);
	bool OpenDocument(const CString& strFileName);
	void CloseDocument(bool bUpdateData = true);
	void SaveDocument(const CString& strFileName = _T(""));
	bool OpenBookmarks(const CString& strFileName);

	DjVuSource* m_pSource;

// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSave();
	afx_msg void OnSaveAs();
	afx_msg void OnExportBookmarks();
	afx_msg void OnBrowseDjvu();
	afx_msg void OnBrowseBookmarks();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnKickIdle();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
};
