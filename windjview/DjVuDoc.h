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

#include "DjVuSource.h"

class CDjVuView;


class CDjVuDoc : public CDocument
{
protected: // create from serialization only
	CDjVuDoc();
	DECLARE_DYNCREATE(CDjVuDoc)

// Operations
public:
	DjVuSource* GetSource() { return m_pSource; }
	CDjVuView* GetDjVuView();

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void OnCloseDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CDjVuDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DjVuSource* m_pSource;
	HRESULT DoInstall();

	// Generated message map functions
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	afx_msg void OnSaveCopyAs();
	afx_msg void OnFileExportText();
	afx_msg void OnUpdateFileExportText(CCmdUI* pCmdUI);
	afx_msg void OnFileInstall();
	afx_msg void OnUpdateFileInstall(CCmdUI* pCmdUI);
	afx_msg void OnFileClose();
	afx_msg void OnFileDocProperties();
	afx_msg void OnFileExportBookmarks();
	afx_msg void OnUpdateFileExportBookmarks(CCmdUI* pCmdUI);
	afx_msg void OnFileImportBookmarks();
	DECLARE_MESSAGE_MAP()
};
