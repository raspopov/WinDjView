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
#include "WinDjView.h"
#include "MyDocManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMyDocManager

IMPLEMENT_DYNAMIC(CMyDocManager, CDocManager)

CMyDocManager::CMyDocManager()
{
}

CMyDocManager::~CMyDocManager()
{
}


void AppendFilterSuffix(CString& filter, OPENFILENAME& ofn,
		CDocTemplate* pTemplate, CString* pstrDefaultExt)
{
	// From MFC: _AfxAppendFilterSuffix

	ASSERT_VALID(pTemplate);
	ASSERT_KINDOF(CDocTemplate, pTemplate);

	CString strExtensions, strFilterName;
	if (pTemplate->GetDocString(strExtensions, CDocTemplate::filterExt)
			&& !strExtensions.IsEmpty()
			&& pTemplate->GetDocString(strFilterName, CDocTemplate::filterName)
			&& !strFilterName.IsEmpty())
	{
		filter += strFilterName;
		ASSERT(!filter.IsEmpty());  // must have a file type name
		filter += (TCHAR)'\0';  // next string please

		CString strFilterExt;
		int nExt = 0;
		while (AfxExtractSubString(strFilterExt, strExtensions, nExt++, ';') &&
				!strFilterExt.IsEmpty())
		{
			// see if extension matches
			ASSERT(strFilterExt[0] == '.');

			// a file based document template - add to filter list
			if (pstrDefaultExt != NULL && nExt == 1)
			{
				// set the default extension
				*pstrDefaultExt = ((LPCTSTR)strFilterExt) + 1;  // skip the '.'
				ofn.lpstrDefExt = (LPTSTR)(LPCTSTR)(*pstrDefaultExt);
				ofn.nFilterIndex = ofn.nMaxCustFilter + 1;  // 1 based number
			}

			// add to filter
			if (nExt > 1)
				filter += (TCHAR)';';

			filter += (TCHAR)'*';
			filter += strFilterExt;
		}

		filter += (TCHAR)'\0';  // next string please
		ofn.nMaxCustFilter++;
	}
}

// CMyDocManager member functions

BOOL CMyDocManager::DoPromptFileName(CString& fileName, UINT nIDSTitle,
		DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate)
{
	// From MFC: CDocManager::DoPromptFileName

	CFileDialog dlgFile(bOpenFileDialog);

	CString title;
	VERIFY(title.LoadString(nIDSTitle == AFX_IDS_OPENFILE ? IDS_OPENFILE : nIDSTitle));

	dlgFile.m_ofn.Flags |= lFlags;

	CString strFilter;
	CString strDefault;
	if (pTemplate != NULL)
	{
		ASSERT_VALID(pTemplate);
		AppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate, &strDefault);
	}
	else
	{
		// do for all doc template
		POSITION pos = m_templateList.GetHeadPosition();
		BOOL bFirst = TRUE;
		while (pos != NULL)
		{
			CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
			AppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate,
				bFirst ? &strDefault : NULL);
			bFirst = FALSE;
		}
	}

	// append the "*.*" all files filter
	CString allFilter;
	VERIFY(allFilter.LoadString(IDS_ALLFILTER));
	strFilter += allFilter;
	strFilter += (TCHAR)'\0';   // next string please
	strFilter += _T("*.*");
	strFilter += (TCHAR)'\0';   // last string
	dlgFile.m_ofn.nMaxCustFilter++;

	dlgFile.m_ofn.lpstrFilter = strFilter;
	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);

	int nResult = dlgFile.DoModal();
	fileName.ReleaseBuffer();
	return nResult == IDOK;
}
