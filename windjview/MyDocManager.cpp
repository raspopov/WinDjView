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

#include "stdafx.h"
#include "WinDjView.h"
#include "MyDocManager.h"
#include "MainFrm.h"
#include "DjVuDoc.h"
#include "DjVuView.h"
#include "MyFileDialog.h"
#include "FullscreenWnd.h"

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

	CMyFileDialog dlgFile(!!bOpenFileDialog);

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

	INT_PTR nResult = dlgFile.DoModal();
	fileName.ReleaseBuffer();
	return (nResult == IDOK);
}

CDocument* CMyDocManager::OpenDocumentFile(LPCTSTR lpszFileName)
{
	// From MFC: CDocManager::OpenDocumentFile

	CString strFileName = lpszFileName;

	strFileName.TrimLeft();
	strFileName.TrimRight();
	if (strFileName[0] == '\"')
		strFileName.Delete(0);
	int nPos = strFileName.ReverseFind('\"');
	if (nPos != -1)
		strFileName.Delete(nPos);

	CString strQuery, strPage;
	nPos = strFileName.Find('?');
	if (nPos != -1)
	{
		strQuery = strFileName.Mid(nPos + 1);
		strFileName = strFileName.Left(nPos);
	}

	bool bPathTooLong = false;
	TCHAR szPath[_MAX_PATH];
	if (!AfxFullPath(szPath, strFileName))
		bPathTooLong = true;

	if (bPathTooLong || !PathFileExists(szPath))
	{
		// Try extracting page number
		nPos = strFileName.ReverseFind('#');
		if (nPos != -1)
		{
			strPage = strFileName.Mid(nPos + 1);
			strFileName = strFileName.Left(nPos);

			if (!AfxFullPath(szPath, strFileName))
				bPathTooLong = true;
		}
	}

	if (bPathTooLong)
	{
		AfxMessageBox(FormatString(IDS_PATH_TOO_LONG, szPath), MB_ICONEXCLAMATION | MB_OK);
		return NULL;
	}

	TCHAR szLinkName[_MAX_PATH];
	if (AfxResolveShortcut(GetMainWnd(), szPath, szLinkName, _MAX_PATH))
		lstrcpy(szPath, szLinkName);

	// find the highest confidence
	CDocTemplate::Confidence bestMatch = CDocTemplate::noAttempt;
	CDocTemplate* pBestTemplate = NULL;
	CDocument* pOpenDocument = NULL;
	CMainFrame* pOldMainFrm = (CMainFrame*) theApp.m_pMainWnd;

	POSITION pos = m_templateList.GetHeadPosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);

		CDocTemplate::Confidence match;
		ASSERT(pOpenDocument == NULL);
		match = pTemplate->MatchDocType(szPath, pOpenDocument);
		if (match > bestMatch)
		{
			bestMatch = match;
			pBestTemplate = pTemplate;
		}
		if (match == CDocTemplate::yesAlreadyOpen)
			break;
	}

	if (pOpenDocument != NULL)
	{
		POSITION pos = pOpenDocument->GetFirstViewPosition();
		if (pos != NULL)
		{
			CView* pView = pOpenDocument->GetNextView(pos);
			ASSERT_VALID(pView);

			CMainFrame* pMainFrm = (CMainFrame*) pView->GetTopLevelFrame();
			pMainFrm->ActivateDocument(pOpenDocument);
		}
		else
			TRACE(_T("Error: Can not find a view for document to activate.\n"));
	}

	if (pOpenDocument == NULL)
	{
		if (pBestTemplate == NULL)
		{
			AfxMessageBox(AFX_IDP_FAILED_TO_OPEN_DOC);
			return NULL;
		}

		pOpenDocument = pBestTemplate->OpenDocumentFile(szPath);
	}

	if (pOpenDocument != NULL)
	{
		CDjVuDoc* pDoc = (CDjVuDoc*) pOpenDocument;
		CDjVuView* pView = pDoc->GetDjVuView();
		CMainFrame* pMainFrm = pView->GetMainFrame();

		// CDocManager::OnDDECommand shows the previous main window.
		// If it was in the fullscreen mode, hide it back.
		if (pOldMainFrm != NULL && pOldMainFrm != pMainFrm && pOldMainFrm->IsFullscreenMode())
			pOldMainFrm->ShowWindow(SW_HIDE);

		if (!strPage.IsEmpty())
			pView->GoToURL(MakeUTF8String(_T("#") + strPage));
	}

	return pOpenDocument;
}

void CMyDocManager::OnFileOpen()
{
	// prompt the user (with all document templates)
	CString strPathName;
	if (!DoPromptFileName(strPathName, AFX_IDS_OPENFILE,
			OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TRUE, NULL))
		return;

	theApp.OpenDocumentFile(strPathName);
}
