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
#include "MyFileDialog.h"
#include "InstallDicDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDjVuDoc

IMPLEMENT_DYNCREATE(CDjVuDoc, CDocument)

BEGIN_MESSAGE_MAP(CDjVuDoc, CDocument)
	ON_COMMAND(ID_FILE_SAVE_COPY_AS, OnSaveCopyAs)
	ON_COMMAND(ID_FILE_EXPORT_TEXT, OnFileExportText)
	ON_UPDATE_COMMAND_UI(ID_FILE_EXPORT_TEXT, OnUpdateFileExportText)
	ON_COMMAND(ID_FILE_INSTALL, OnFileInstall)
	ON_UPDATE_COMMAND_UI(ID_FILE_INSTALL, OnUpdateFileInstall)
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
		theApp.ReportFatalError();
		return false;
	}

	if (m_pSource == NULL)
	{
		AfxMessageBox(LoadString(IDS_FAILED_TO_OPEN) + lpszPathName + LoadString(IDS_NOT_VALID_DOCUMENT));
		return false;
	}
/*
	if (m_pSource->IsDictionary() && !m_pSource->GetDictionaryInfo()->bInstalled)
	{
		if (AfxMessageBox(IDS_PROMPT_INSTALL_ON_OPEN, MB_ICONQUESTION | MB_YESNO) == IDYES)
		{
			if (DoInstall())
			{
				m_pSource->Release();
				AfxMessageBox(IDS_INSTALLATION_SUCCEEDED, MB_ICONEXCLAMATION | MB_OK);
				return false;
			}
			else
			{
				AfxMessageBox(IDS_INSTALLATION_FAILED, MB_ICONEXCLAMATION | MB_OK);
			}
		}
	}
*/
	return true;
}

CDjVuView* CDjVuDoc::GetDjVuView()
{
	POSITION pos = GetFirstViewPosition();
	ASSERT(pos != NULL);

	CDjVuView* pView = (CDjVuView*) GetNextView(pos);
	return pView;
}

void CDjVuDoc::OnSaveCopyAs()
{
	CString strFileName = GetTitle();

	CMyFileDialog dlg(false, _T("djvu"), strFileName, OFN_OVERWRITEPROMPT |
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
		theApp.ReportFatalError();
	}
}

void CDjVuDoc::OnFileExportText()
{
	CString strPathName = m_pSource->GetFileName();
	if (!PathRenameExtension(strPathName.GetBuffer(MAX_PATH), _T(".txt")))
		PathRemoveExtension(strPathName.GetBuffer(MAX_PATH));

	strPathName.ReleaseBuffer();
	CMyFileDialog dlg(false, _T("txt"), strPathName, OFN_OVERWRITEPROMPT |
		OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST,
		LoadString(IDS_TEXT_FILTER));

	CString strTitle;
	strTitle.LoadString(IDS_EXPORT_TEXT);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

	if (dlg.DoModal() != IDOK)
		return;

	CWaitCursor wait;

	strPathName = dlg.GetPathName();
	wstring wtext;
	GetDjVuView()->GetNormalizedText(wtext);
	CString strText = MakeCString(wtext);

	CFile file;
	if (file.Open(strPathName, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive))
	{
#ifdef _UNICODE
		// Get ANSI text
		int nSize = ::WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DISCARDNS,
			strText, -1, NULL, 0, NULL, NULL);
		LPSTR pszText = new CHAR[nSize];
		::WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DISCARDNS,
			strText, -1, pszText, nSize, NULL, NULL);

		file.Write(pszText, strlen(pszText));
		delete[] pszText;
#else
		file.Write(strText, strText.GetLength());
#endif
		file.Close();
	}
}

void CDjVuDoc::OnUpdateFileExportText(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pSource->HasText());

	if (m_pSource->IsDictionary()
			&& !m_pSource->GetDictionaryInfo()->bInstalled
			&& pCmdUI->m_pMenu != NULL
			&& pCmdUI->m_pMenu->GetMenuItemID(pCmdUI->m_nIndex + 1) != ID_FILE_INSTALL)
	{
		pCmdUI->m_pMenu->InsertMenu(pCmdUI->m_nIndex + 1, MF_BYPOSITION | MF_STRING,
				ID_FILE_INSTALL, LoadString(IDS_FILE_INSTALL));
		pCmdUI->m_nIndexMax = pCmdUI->m_pMenu->GetMenuItemCount();
	}
}

void CDjVuDoc::OnFileInstall()
{
	if (m_pSource->GetDictionaryInfo()->bInstalled)
		return;

	HRESULT hResult = DoInstall();

	if (SUCCEEDED(hResult))
	{
		// Dictionary could become installed without copying, if the dictionary's location
		// has just been selected as a new user location
		if (!m_pSource->GetDictionaryInfo()->bInstalled)
			OnCloseDocument();

		AfxMessageBox(IDS_INSTALLATION_SUCCEEDED, MB_ICONEXCLAMATION | MB_OK);
	}
	else if (hResult != E_ABORT)
	{
		AfxMessageBox(IDS_INSTALLATION_FAILED, MB_ICONEXCLAMATION | MB_OK);
	}
}

HRESULT CDjVuDoc::DoInstall()
{
	DictionaryInfo* pInfo = theApp.GetDictionaryInfo(m_strPathName, false);
	bool bExists = (pInfo != NULL);
	if (bExists && AfxComparePath(pInfo->strPathName, m_strPathName))
		return E_ABORT;

	CInstallDicDlg dlg(bExists ? IDD_INSTALL_DIC_REPLACE : IDD_INSTALL_DIC);

	if (dlg.DoModal() != IDOK)
		return E_ABORT;

	if (!bExists)
	{
		theApp.GetAppSettings()->nDictChoice = dlg.m_nChoice;

		if (theApp.GetAppSettings()->strDictLocation.IsEmpty() && !dlg.m_strDictLocation.IsEmpty())
		{
			theApp.GetAppSettings()->strDictLocation = dlg.m_strDictLocation;
			theApp.ReloadDictionaries();
		}
	}

	int nChoice = -1;
	if (!bExists)
		nChoice = dlg.m_nChoice;

	CWaitCursor wait;
	if (theApp.InstallDictionary(m_pSource, nChoice, !!dlg.m_bKeepOriginal))
		return S_OK;
	else
		return E_FAIL;
}

void CDjVuDoc::OnUpdateFileInstall(CCmdUI* pCmdUI)
{
	if (pCmdUI->m_pMenu != NULL
			&& (!m_pSource->IsDictionary() || m_pSource->GetDictionaryInfo()->bInstalled))
	{
		pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nIndex, MF_BYPOSITION);
		pCmdUI->m_nIndex--;
		pCmdUI->m_nIndexMax = pCmdUI->m_pMenu->GetMenuItemCount();
	}
}
