//	DjVu Page Index Tool
//	Copyright (C) 2006-2007 Andrew Zhezherun
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


// CIndexDlg dialog

class CIndexDlg : public CDialog, public IMyDropTarget
{
public:
	CIndexDlg(CWnd* pParent = NULL);

// Dialog Data
	enum { IDD = IDD_INDEXTOOL };
	CString m_strDjVuFile;
	CString m_strPageIndexFile;

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	static bool bPrompt;

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
	bool m_bHasPageIndexFile;
	BOOL m_bPrompt;
	CMyDropTarget m_dropTarget;

	bool ExportPageIndex(const CString& strPageIndexFile);
	bool OpenDocument(const CString& strFileName);
	void CloseDocument(bool bUpdateData = true);

	GP<DjVuDocument> m_pDjVuDoc;

// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnEmbed();
	afx_msg void OnExport();
	afx_msg void OnBrowseDjvu();
	afx_msg void OnBrowsePageIndex();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnKickIdle();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
};
