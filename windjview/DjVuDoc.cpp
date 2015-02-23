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
#include "DjVuDoc.h"

#include "DjVuView.h"
#include "MainFrm.h"
#include "MDIChild.h"
#include "MyFileDialog.h"
#include "InstallDicDlg.h"
#include "DocPropertiesDlg.h"
#include "XMLParser.h"

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
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_COMMAND(ID_FILE_DOC_PROPERTIES, OnFileDocProperties)
	ON_COMMAND(ID_FILE_EXPORT_BOOKMARKS, OnFileExportBookmarks)
	ON_UPDATE_COMMAND_UI(ID_FILE_EXPORT_BOOKMARKS, OnUpdateFileExportBookmarks)
	ON_COMMAND(ID_FILE_IMPORT_BOOKMARKS, OnFileImportBookmarks)
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

	::SHAddToRecentDocs(SHARD_PATH, lpszPathName);
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
	if (pos == NULL)
		return NULL;

	return (CDjVuView*) GetNextView(pos);
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

	UINT_PTR nResult = dlg.DoModal();
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
	CFile file;
	if (!file.Open(strPathName, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive))
	{
		AfxMessageBox(FormatString(IDS_CANNOT_WRITE_TO_FILE, strPathName), MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	wstring wtext;
	GetDjVuView()->GetNormalizedText(wtext);
	string text;
	MakeANSIString(wtext, text);

	file.Write(text.c_str(), (UINT)text.length());
	file.Close();
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

		AfxMessageBox(IDS_INSTALLATION_SUCCEEDED, MB_ICONINFORMATION | MB_OK);
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

void CDjVuDoc::OnFileClose()
{
	CMainFrame* pMainFrame = GetDjVuView()->GetMainFrame();
	if (pMainFrame->IsFullscreenMode())
		return;

	if (theApp.m_bTopLevelDocs && theApp.GetDocumentCount() > 1)
	{
		pMainFrame->SendMessage(WM_CLOSE);
		return;
	}

	theApp.SaveSettings();

	CDocument::OnFileClose();
}

void CDjVuDoc::OnCloseDocument()
{
	BOOL bAutoDelete = m_bAutoDelete;
	m_bAutoDelete = false;  // don't destroy document while closing views

	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CDjVuView* pView = (CDjVuView*) GetNextView(pos);
		CMDIChild* pMDIChild = pView->GetMDIChild();
		pView->GetMainFrame()->CloseMDIChild(pMDIChild);

		pos = GetFirstViewPosition();
	}

	m_bAutoDelete = bAutoDelete;

	// clean up contents of document before destroying the document itself
	DeleteContents();

	// delete the document if necessary
	if (m_bAutoDelete)
		delete this;
}

void CDjVuDoc::OnFileDocProperties()
{
	CDocPropertiesDlg dlg(m_pSource);
	dlg.DoModal();
}

void CDjVuDoc::OnFileExportBookmarks()
{
	CString strPathName = m_pSource->GetFileName();
	if (!PathRenameExtension(strPathName.GetBuffer(MAX_PATH), _T(".bookmarks")))
		PathRemoveExtension(strPathName.GetBuffer(MAX_PATH));

	strPathName.ReleaseBuffer();
	CMyFileDialog dlg(false, _T("bookmarks"), strPathName, OFN_OVERWRITEPROMPT |
		OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST,
		LoadString(IDS_BOOKMARKS_FILTER));

	CString strTitle;
	strTitle.LoadString(IDS_EXPORT_BOOKMARKS);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

	if (dlg.DoModal() != IDOK)
		return;

	CWaitCursor wait;

	strPathName = dlg.GetPathName();
	GUTF8String strXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			+ m_pSource->GetSettings()->GetXML(true);

	// Convert line endings.
	stringstream out;
	out << (const char*)strXML;

	CFile file;
	if (!file.Open(strPathName, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive))
	{
		AfxMessageBox(FormatString(IDS_CANNOT_WRITE_TO_FILE, strPathName), MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	file.Write(out.str().c_str(), strXML.length());
	file.Close();
}

void CDjVuDoc::OnUpdateFileExportBookmarks(CCmdUI* pCmdUI)
{
	bool bHasBookmarks = !m_pSource->GetSettings()->bookmarks.empty();
	bool bHasAnnotations = !m_pSource->GetSettings()->pageSettings.empty();
	pCmdUI->Enable(bHasBookmarks || bHasAnnotations);
}

void CDjVuDoc::OnFileImportBookmarks()
{
	CMyFileDialog dlg(true, _T("bookmarks"), NULL, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		LoadString(IDS_BOOKMARKS_FILTER));

	CString strTitle;
	strTitle.LoadString(IDS_IMPORT_BOOKMARKS);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

	if (dlg.DoModal() != IDOK)
		return;

	CWaitCursor wait;

	CFile file;
	CString strContent;
	if (!file.Open(dlg.GetPathName(), CFile::modeRead | CFile::shareDenyWrite))
	{
		AfxMessageBox(IDS_CANNOT_OPEN_BOOKMARKS, MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	int nLength = static_cast<int>(file.GetLength());
	vector<char> data(nLength);
	file.Read(&data[0], nLength);
	file.Close();

	string str(data.begin(), data.end());
	stringstream in(str);
	XMLParser parser;
	if (!parser.Parse(in))
	{
		AfxMessageBox(IDS_CANNOT_READ_BOOKMARKS, MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	DocSettings settings;
	settings.Load(*parser.GetRoot());
	// Remove annotations for non-existing pages.
	map<int, PageSettings>::iterator it;
	for (it = settings.pageSettings.begin(); it != settings.pageSettings.end();)
	{
		int nPage = it->first;
		if (nPage < 0 || nPage >= m_pSource->GetPageCount())
			settings.pageSettings.erase(it++);
		else
			++it;
	}

	if (settings.bookmarks.empty() && settings.pageSettings.empty())
	{
		AfxMessageBox(IDS_BOOKMARKS_EMPTY, MB_OK | MB_ICONINFORMATION);
		return;
	}

	list<Bookmark> tmpBookmarks;
	map<int, PageSettings> tmpPageSettings;

	if (!settings.bookmarks.empty() && !m_pSource->GetSettings()->bookmarks.empty()
			|| !settings.pageSettings.empty() && !m_pSource->GetSettings()->pageSettings.empty())
	{
		CDjViewApp::MessageBoxOptions mbo;
		mbo.strCaptions = LoadString(IDS_BOOKMARKS_PROMPT_CAPTIONS);
		int nResult = theApp.DoMessageBox(LoadString(IDS_BOOKMARKS_PROMPT),
				MB_YESNOCANCEL | MB_ICONQUESTION, 0, mbo);
		if (nResult == IDCANCEL)
			return;

		if (nResult == IDNO)
		{
			if (!settings.bookmarks.empty())
				tmpBookmarks.swap(m_pSource->GetSettings()->bookmarks);
			if (!settings.pageSettings.empty())
				tmpPageSettings.swap(m_pSource->GetSettings()->pageSettings);
		}
	}

	m_pSource->GetSettings()->bookmarks.insert(m_pSource->GetSettings()->bookmarks.end(),
			settings.bookmarks.begin(), settings.bookmarks.end());
	for (it = settings.pageSettings.begin(); it != settings.pageSettings.end(); ++it)
	{
		int nPage = it->first;
		if (nPage < 0 || nPage >= m_pSource->GetPageCount())
			continue;

		PageSettings& page = m_pSource->GetSettings()->pageSettings[nPage];
		PageSettings& rhsPage = it->second;
		page.anno.insert(page.anno.end(), rhsPage.anno.begin(), rhsPage.anno.end());
	}

	m_pSource->GetSettings()->UpdateObservers(BOOKMARKS_CHANGED);
	m_pSource->GetSettings()->UpdateObservers(ANNOTATIONS_CHANGED);
}
