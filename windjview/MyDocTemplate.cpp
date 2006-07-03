//	WinDjView
//	Copyright (C) 2004-2006 Andrew Zhezherun
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
//	http://www.gnu.org/copyleft/gpl.html

// $Id$

#include "stdafx.h"
#include "MyDocTemplate.h"

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
