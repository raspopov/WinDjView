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
#include "MyDocTemplate.h"
#include "ChildFrm.h"
#include "DjVuView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMyDocTemplate

IMPLEMENT_DYNAMIC(CMyDocTemplate, CMultiDocTemplate)

CMyDocTemplate::CMyDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass, 
		CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass)
	: CMultiDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass)
{
}

CMyDocTemplate::~CMyDocTemplate()
{
}


// CMyDocTemplate member functions

CDocTemplate::Confidence CMyDocTemplate::MatchDocType(
		LPCTSTR lpszPathName, CDocument*& rpDocMatch)
{
	// From MFC: CDocTemplate::MatchDocType

	ASSERT(lpszPathName != NULL);
	rpDocMatch = NULL;

	// go through all documents
	POSITION pos = GetFirstDocPosition();
	while (pos != NULL)
	{
		CDocument* pDoc = GetNextDoc(pos);
		if (AfxComparePath(pDoc->GetPathName(), lpszPathName))
		{
			// already open
			rpDocMatch = pDoc;
			return yesAlreadyOpen;
		}
	}

	// see if it matches our default suffix
	CString strExtensions;
	GetDocString(strExtensions, CDocTemplate::filterExt);

	CString strFilterExt;
	int nExt = 0;
	while (AfxExtractSubString(strFilterExt, strExtensions, nExt++, ';') &&
			!strFilterExt.IsEmpty())
	{
		// see if extension matches
		ASSERT(strFilterExt[0] == '.');
		LPCTSTR lpszDot = _tcsrchr(lpszPathName, '.');
		if (lpszDot != NULL && lstrcmpi(lpszDot, strFilterExt) == 0)
			return yesAttemptNative; // extension matches, looks like ours
	}

	// otherwise we will guess it may work
	return yesAttemptForeign;
}

void CMyDocTemplate::UpdateTemplate()
{
	if (!m_strDocStrings.LoadString(m_nIDResource) || m_strDocStrings.IsEmpty())
	{
		TRACE1("Warning: no document names in string for template #%d.\n", m_nIDResource);
	}
}

void CMyDocTemplate::InitialUpdateFrame(CFrameWnd* pFrame, CDocument* pDoc, BOOL bMakeVisible)
{
	CChildFrame* pChildFrm = (CChildFrame*)pFrame;
	pChildFrm->CreateNavPanes();

	pChildFrm->GetDjVuView()->AddObserver(GetMainFrame());

	CMultiDocTemplate::InitialUpdateFrame(pFrame, pDoc, bMakeVisible);
}
