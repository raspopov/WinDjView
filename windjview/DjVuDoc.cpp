//	WinDjView 0.1
//	Copyright (C) 2004 Andrew Zhezherun
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
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  http://www.gnu.org/copyleft/gpl.html

// $Id$

#include "stdafx.h"
#include "WinDjView.h"

#include "DjVuDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDjVuDoc

IMPLEMENT_DYNCREATE(CDjVuDoc, CDocument)

BEGIN_MESSAGE_MAP(CDjVuDoc, CDocument)
END_MESSAGE_MAP()


// CDjVuDoc construction/destruction

CDjVuDoc::CDjVuDoc()
{
	// TODO: add one-time construction code here

}

CDjVuDoc::~CDjVuDoc()
{
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

	TCHAR pszName[MAX_PATH] = { 0 };
	LPTSTR pszFileName;
	GetFullPathName(lpszPathName, MAX_PATH, pszName, &pszFileName);

	CFile file;
	if (!file.Open(pszName, CFile::readOnly | CFile::shareDenyWrite))
	{
		AfxMessageBox("Failed to open file " + CString(pszName));
		return false;
	}

	G_TRY
	{
		m_pDjVuDoc = DjVuDocument::create("file://" + GUTF8String(pszName));
		m_pDjVuDoc->wait_get_pages_num();
	}
	G_CATCH(ex) 
	{
		AfxMessageBox("Error opening file " + CString(pszName) + ":\n" + CString(ex.get_cause()));
		return false;
	}
	G_ENDCATCH;

	m_pages.clear();
	m_pages.resize(m_pDjVuDoc->get_pages_num(), NULL);

	return true;
}

GP<DjVuImage> CDjVuDoc::GetPage(int nPage)
{
	ASSERT(nPage >= 0 && nPage < m_pDjVuDoc->get_pages_num());
	if (m_pages[nPage] == NULL)
		m_pages[nPage] = m_pDjVuDoc->get_page(nPage);

	return m_pages[nPage];
}
