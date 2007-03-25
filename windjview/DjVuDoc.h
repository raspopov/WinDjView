//	WinDjView
//	Copyright (C) 2004-2007 Andrew Zhezherun
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
	DECLARE_MESSAGE_MAP()
};
