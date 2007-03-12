//	DjVu Dictionary Tool
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
#include "DjVuSource.h"


// CDictionaryDlg dialog

class CDictionaryDlg : public CDialog, public IMyDropTarget
{
public:
	CDictionaryDlg(CWnd* pParent = NULL);

// Dialog Data
	enum { IDD = IDD_DICTIONARYTOOL };
	CString m_strDjVuFile;
	CString m_strPageIndexFile;
	CString m_strCharMapFile;
	CString m_strTitle;
	int m_nPageIndexAction;
	int m_nCharMapAction;
	CComboBox m_cboLangFrom;
	CComboBox m_cboLangTo;

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
	bool m_bHasPageIndexFile;
	bool m_bHasCharMapFile;
	GUTF8String m_strPageIndexXML;
	GUTF8String m_strCharMapXML;
	CMyDropTarget m_dropTarget;
	const Language* m_pLangFrom;
	const Language* m_pLangTo;

	map<const Language*, vector<DictionaryInfo::LocalizedString> > m_lang;
	void Normalize(vector<DictionaryInfo::LocalizedString>& loc);

	bool ExportPageIndex(const CString& strPageIndexFile);
	bool ExportCharMap(const CString& strCharMapFile);
	bool OpenDocument(const CString& strFileName);
	void CloseDocument(bool bUpdateData = true);
	void SaveDocument(const CString& strFileName = _T(""));
	bool OpenPageIndex(const CString& strFileName);
	bool OpenCharMap(const CString& strFileName);
	void InitDocProperties();

	GUTF8String GetTitleXML();
	GUTF8String GetLangFromXML();
	GUTF8String GetLangToXML();
	static GUTF8String GetLocalizedStringsXML(vector<DictionaryInfo::LocalizedString>& loc);

	DjVuSource* m_pSource;
	DictionaryInfo* m_pDictInfo;

	// Message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSave();
	afx_msg void OnSaveAs();
	afx_msg void OnExportPageIndex();
	afx_msg void OnExportCharMap();
	afx_msg void OnBrowseDjvu();
	afx_msg void OnBrowsePageIndex();
	afx_msg void OnBrowseCharMap();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnKickIdle();
	afx_msg void OnDestroy();
	afx_msg void OnSelChangeFrom();
	afx_msg void OnSelChangeTo();
	afx_msg void OnLocalizeTitle();
	afx_msg void OnLocalizeLangFrom();
	afx_msg void OnLocalizeLangTo();
	DECLARE_MESSAGE_MAP()
};
