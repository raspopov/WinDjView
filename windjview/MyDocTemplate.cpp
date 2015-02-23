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
#include "MyDocTemplate.h"
#include "DjVuView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMyDocTemplate

IMPLEMENT_DYNAMIC(CMyDocTemplate, CMultiDocTemplate)

CMyDocTemplate::CMyDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass, 
		CRuntimeClass* pMDIChildClass, CRuntimeClass* pViewClass)
	: CMultiDocTemplate(nIDResource, pDocClass, NULL, pViewClass),
	  m_pMDIChildClass(pMDIChildClass)
{
	ASSERT(pMDIChildClass == NULL ||
		pMDIChildClass->IsDerivedFrom(RUNTIME_CLASS(CWnd)));
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

void CMyDocTemplate::InitialUpdateMDIChild(CWnd* pMDIChild, CDocument* pDoc, BOOL bMakeVisible)
{
	CMainFrame* pMainFrame = (CMainFrame*) theApp.m_pMainWnd;

	// Create a new MDI Frame window
	if (theApp.m_bTopLevelDocs && pMainFrame->GetActiveView() != NULL)
		pMainFrame = theApp.CreateMainFrame();

	ASSERT(pMainFrame != NULL);

	pMainFrame->AddMDIChild(pMDIChild, pDoc);

	pMDIChild->SendMessage(WM_INITIALUPDATE, 0, 0);
	pMDIChild->SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, true, true);

	pMainFrame->ActivateDocument(pDoc);
	pMainFrame->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
}

CWnd* CMyDocTemplate::CreateEmptyMDIChild()
{
	return CreateNewMDIChild(NULL);
}

CWnd* CMyDocTemplate::CreateNewMDIChild(CDocument* pDoc)
{
	CMainFrame* pMainFrame = (CMainFrame*) theApp.m_pMainWnd;
	ASSERT(pMainFrame != NULL);

	CCreateContext context;
	context.m_pCurrentFrame = NULL;
	context.m_pCurrentDoc = pDoc;
	context.m_pNewViewClass = m_pViewClass;
	context.m_pNewDocTemplate = this;

	CWnd* pMDIChild = (CWnd*) m_pMDIChildClass->CreateObject();
	if (pMDIChild == NULL)
	{
		TRACE(_T("Warning: Dynamic create of MDI child %hs failed.\n"),
				m_pMDIChildClass->m_lpszClassName);
		return NULL;
	}

	if (!pMDIChild->Create(NULL, NULL, WS_CHILD, CRect(0, 0, 0, 0), pMainFrame, AFX_IDW_PANE_FIRST, &context))
	{
		TRACE(_T("Warning: could not create MDI child.\n"));
		return NULL;
	}

	return pMDIChild;
}

CDocument* CMyDocTemplate::OpenDocumentFile(LPCTSTR lpszPathName, BOOL bMakeVisible)
{
	if (lpszPathName == NULL)
	{
		TRACE(_T("Creating new documents is disabled.\n"));
		return NULL;
	}

	CWaitCursor wait;

	CDocument* pDocument = CreateNewDocument();
	if (pDocument == NULL)
	{
		TRACE(_T("CDocTemplate::CreateNewDocument returned NULL.\n"));
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
		return NULL;
	}
	ASSERT_VALID(pDocument);

	BOOL bAutoDelete = pDocument->m_bAutoDelete;
	pDocument->m_bAutoDelete = false;   // don't destroy if something goes wrong

	CWnd* pMDIChild = CreateNewMDIChild(pDocument);

	pDocument->m_bAutoDelete = bAutoDelete;
	if (pMDIChild == NULL)
	{
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
		delete pDocument;       // explicit delete on error
		return NULL;
	}
	ASSERT_VALID(pMDIChild);

	// open an existing document
	if (!pDocument->OnOpenDocument(lpszPathName))
	{
		// user has be alerted to what failed in OnOpenDocument
		TRACE(_T("CDocument::OnOpenDocument returned FALSE.\n"));
		pMDIChild->DestroyWindow();
		return NULL;
	}
	pDocument->SetPathName(lpszPathName);

	InitialUpdateMDIChild(pMDIChild, pDocument, bMakeVisible);
	return pDocument;
}
