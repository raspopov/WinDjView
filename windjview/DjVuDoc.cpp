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

#include "stdafx.h"
#include "WinDjView.h"

#include "ChildFrm.h"
#include "DjVuDoc.h"
#include "DjVuView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDjVuDoc

IMPLEMENT_DYNCREATE(CDjVuDoc, CDocument)

BEGIN_MESSAGE_MAP(CDjVuDoc, CDocument)
	ON_COMMAND(ID_FILE_SAVE_COPY_AS, OnSaveCopyAs)
END_MESSAGE_MAP()


// CDjVuDoc construction/destruction

CDjVuDoc::CDjVuDoc()
	: m_pSource(NULL)
{
}

CDjVuDoc::~CDjVuDoc()
{
	if (m_pSource != NULL)
		m_pSource->Release();
}

BOOL CDjVuDoc::OnNewDocument()
{
	return false;
}

BOOL CDjVuDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	return false;
}

void CDjVuDoc::Serialize(CArchive& ar)
{
	ASSERT(false);
}


// CDjVuDoc diagnostics

#ifdef _DEBUG
void CDjVuDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDjVuDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

BOOL CDjVuDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	DeleteContents();

	CFile file;
	if (!file.Open(lpszPathName, CFile::modeRead | CFile::shareDenyWrite))
	{
		AfxMessageBox(LoadString(IDS_FAILED_TO_OPEN) + lpszPathName);
		return false;
	}
	file.Close();

	try
	{
		m_pSource = DjVuSource::FromFile(lpszPathName);
	}
	catch (...)
	{
		ReportFatalError();
		return false;
	}

	if (m_pSource == NULL)
	{
		AfxMessageBox(LoadString(IDS_FAILED_TO_OPEN) + lpszPathName + LoadString(IDS_NOT_VALID_DOCUMENT));
		return false;
	}

	return true;
}

CDjVuView* CDjVuDoc::GetDjVuView()
{
	POSITION pos = GetFirstViewPosition();
	ASSERT(pos != NULL);

	CDjVuView* pView = (CDjVuView*)GetNextView(pos);
	return pView;
}

void CDjVuDoc::OnSaveCopyAs()
{
	CString strFileName = GetTitle();

	CFileDialog dlg(false, _T("djvu"), strFileName, OFN_OVERWRITEPROMPT |
		OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST,
		LoadString(IDS_DJVU_FILTER));

	CString strTitle;
	strTitle.LoadString(IDS_SAVE_COPY_AS);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

	UINT nResult = dlg.DoModal();
	GetDjVuView()->SetFocus();
	if (nResult != IDOK)
		return;

	CWaitCursor wait;
	strFileName = dlg.GetPathName();

	if (AfxComparePath(strFileName, m_pSource->GetFileName()))
	{
		AfxMessageBox(IDS_CANNOT_SAVE_TO_ORIG, MB_ICONERROR | MB_OK);
		return;
	}

	try
	{
		if (!m_pSource->SaveAs(strFileName))
			AfxMessageBox(IDS_SAVE_ERROR, MB_ICONERROR | MB_OK);
	}
	catch (...)
	{
		ReportFatalError();
	}
}
