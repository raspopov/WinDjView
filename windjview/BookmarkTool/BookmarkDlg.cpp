//	DjVu Bookmark Tool
//	Copyright (C) 2005-2007 Andrew Zhezherun
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
#include "BookmarkTool.h"
#include "BookmarkDlg.h"
#include "Global.h"
#include "MyFileDialog.h"
#include "DjVuSource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBookmarkDlg dialog

CBookmarkDlg::CBookmarkDlg(CWnd* pParent)
	: CDialog(CBookmarkDlg::IDD, pParent), m_pSource(NULL),
	  m_bHasBookmarksFile(false), m_pBookmarks(NULL), m_nBookmarksAction(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_strDjVuFile.LoadString(IDS_BROWSE_DJVU_PROMPT);
	m_strBookmarksFile.LoadString(IDS_BROWSE_BOOKMARKS_PROMPT);
}

void CBookmarkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DJVU_FILE, m_strDjVuFile);
	DDX_Text(pDX, IDC_BOOKMARKS_FILE, m_strBookmarksFile);
	DDX_Radio(pDX, IDC_BOOKMARKS_DONTCHANGE, m_nBookmarksAction);
}

BEGIN_MESSAGE_MAP(CBookmarkDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_BN_CLICKED(IDC_SAVE_AS, OnSaveAs)
	ON_BN_CLICKED(IDC_EXPORT_BOOKMARKS, OnExportBookmarks)
	ON_BN_CLICKED(IDC_BROWSE_DJVU, OnBrowseDjvu)
	ON_BN_CLICKED(IDC_BROWSE_BOOKMARKS, OnBrowseBookmarks)
	ON_WM_CTLCOLOR()
	ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CBookmarkDlg message handlers

BOOL CBookmarkDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.
	SetIcon(m_hIcon, true);	// Set big icon
	SetIcon(m_hIcon, false); // Set small icon

	m_dropTarget.Register(this);
	OnKickIdle();

	return true;
}

void CBookmarkDlg::OnPaint() 
{
	if (IsIconic())
	{
		// Draw minimized icon
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);

		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display
// while the user drags the minimized window.
HCURSOR CBookmarkDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CBookmarkDlg::OnExportBookmarks()
{
	if (m_pSource->GetContents() == NULL)
		return;

	TCHAR szDrive[_MAX_DRIVE], szPath[_MAX_PATH], szName[_MAX_FNAME], szExt[_MAX_EXT];
	_tsplitpath(m_strDjVuFile, szDrive, szPath, szName, szExt);
	CString strFileName = szDrive + CString(szPath) + CString(szName) + CString(_T(".html"));

	CMyFileDialog dlg(false, _T("html"), strFileName, OFN_OVERWRITEPROMPT |
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN,
		LoadString(IDS_BOOKMARKS_FILTER));

	CString strTitle;
	strTitle.LoadString(IDS_EXPORT_BOOKMARKS);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

	UINT nResult = dlg.DoModal();
	if (nResult != IDOK)
		return;

	CString strBookmarksFile = dlg.GetPathName();

	if (ExportBookmarks(strBookmarksFile) && !m_bHasBookmarksFile)
	{
		m_strBookmarksFile = strBookmarksFile;
		m_pBookmarks = m_pSource->GetContents();
		m_bHasBookmarksFile = true;
		UpdateData(false);
	}
}

void CBookmarkDlg::OnBrowseDjvu()
{
	CString strFileName = (m_pSource != NULL ? m_strDjVuFile : _T(""));

	CMyFileDialog dlg(true, _T("djvu"), strFileName,
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		LoadString(IDS_DJVU_FILTER));

	CString strTitle;
	strTitle.LoadString(IDS_BROWSE_DJVU);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

	UINT nResult = dlg.DoModal();
	if (nResult != IDOK)
		return;

	CString strDjVuFile = dlg.GetPathName();
	OpenDocument(strDjVuFile);
}

bool CBookmarkDlg::OpenDocument(const CString& strFileName)
{
	try
	{
		CWaitCursor wait;

		m_pSource = DjVuSource::FromFile(strFileName);
		m_strDjVuFile = strFileName;

		if (m_pSource->GetContents() != NULL && m_nBookmarksAction == 2)
			m_nBookmarksAction = 0;

		UpdateData(false);
		OnKickIdle();
		return true;
	}
	catch (GException&)
	{
		AfxMessageBox(IDS_LOAD_ERROR);
		return false;
	}
	catch (...)
	{
		AfxMessageBox(IDS_FATAL_ERROR);
		return false;
	}
}

void CBookmarkDlg::CloseDocument(bool bUpdateData)
{
	if (m_pSource != NULL)
	{
		m_pSource->Release();
		m_pSource = NULL;
	}

	m_strDjVuFile.LoadString(IDS_BROWSE_DJVU_PROMPT);

	if (bUpdateData)
		UpdateData(false);
}

struct DjVuBookmark
{
	vector<DjVuBookmark> children;
	GUTF8String name;
	GUTF8String url;
};

inline string tolower(string str)
{
	for (string::iterator it = str.begin(); it != str.end(); ++it)
		*it = tolower(*it, locale());
	return str;
}

GUTF8String HTMLEscape(const GUTF8String& str)
{
	if (str.length() == 0)
		return "";

	bool modified = false;
	char* ret;
	GPBuffer<char> gret(ret, str.length() * 7);
	ret[0] = 0;
	char* retptr=ret;
	const char* start = str;
	const char* s = start;
	const char* last = s;
	const char* end = s + str.length();
	GP<GStringRep> special;
	for (unsigned long w; (w = GStringRep::UTF8toUCS4((const unsigned char*&)s, end)); last = s)
	{
		char const* ss = 0;
		switch (w)
		{
		case '<':
			ss = "&lt;";
			break;
		case '>':
			ss = "&gt;";
			break;
		case '&':
			ss = "&amp;";
			break;
		case '\"':
			ss="&quot;";
			break;
		}

		if (ss)
		{
			modified = true;
			if (s != start)
			{
				size_t len = (size_t) last - (size_t) start;
				strncpy(retptr, start, len);
				retptr += len;
				start = s;
			}
			if (ss[0])
			{
				size_t len = strlen(ss);
				strcpy(retptr, ss);
				retptr += len;
			}
		}
	}

	GUTF8String retval;
	if (modified)
	{
		strcpy(retptr, start);
		retval = ret;
	}
	else
	{
		retval = str;
	}

	return retval;
}

void FixWhitespace(wstring& text)
{
	wstring result;
	result.reserve(text.length());

	int nStart = 0;
	while (nStart < static_cast<int>(text.length()) && text[nStart] <= 0x20)
		++nStart;
	int nEnd = text.length() - 1;
	while (nEnd >= nStart && text[nEnd] <= 0x20)
		--nEnd;

	bool bHadSpace = false;
	for (int nCur = nStart; nCur <= nEnd; ++nCur)
	{
		if (text[nCur] <= 0x20)
		{
			if (!bHadSpace)
			{
				bHadSpace = true;
				result += text.substr(nStart, nCur - nStart) + L" ";
			}
		}
		else if (bHadSpace)
		{
			bHadSpace = false;
			nStart = nCur;
		}
	}

	if (!bHadSpace)
		result += text.substr(nStart, nEnd + 1 - nStart);

	text.swap(result);
}

bool ReadBookmarks(const string& data, vector<DjVuBookmark>& bookmarks)
{
	stack<DjVuBookmark*> parents;

	string data_lc = tolower(data);

	// Bookmarks are supplied in html format as nested <ul>'s with each
	// bookmark specified by the <a> tag. Reading <li>'s is not needed
	size_t pos_list = data_lc.find("<ul", 0);
	size_t pos_list_end = data_lc.find("</ul", 0);
	size_t pos_link = data_lc.find("<a", 0);

	while (pos_link != string::npos || pos_list != string::npos || pos_list_end != string::npos)
	{
		if (pos_link < pos_list && pos_link < pos_list_end)
		{
			DjVuBookmark bookmark;

			string url;
			size_t pos = data_lc.find("href", pos_link);
			size_t pos_rbracket = data_lc.find(">", pos_link);

			if (pos < pos_rbracket)
			{
				pos = data_lc.find("=", pos);
				pos = min(data_lc.find("\"", pos), data_lc.find("'", pos));
				if (pos == string::npos)
				{
					return false;
				}

				size_t pos_end = data_lc.find(data_lc[pos], pos + 1);
				if (pos_end == string::npos)
				{
					return false;
				}

				bookmark.url = GUTF8String(data.substr(pos + 1, pos_end - pos - 1).c_str()).fromEscaped();

				wstring str;
				MakeWString(bookmark.url, str);
				bookmark.url = MakeUTF8String(str);
			}

			pos = pos_rbracket;
			if (pos == string::npos)
			{
				return false;
			}

			size_t pos_end = data_lc.find("</a", pos);
			if (pos_end == string::npos)
			{
				return false;
			}

			bookmark.name = GUTF8String(data.substr(pos + 1, pos_end - pos - 1).c_str()).fromEscaped();

			wstring str;
			MakeWString(bookmark.name, str);

			FixWhitespace(str);
			bookmark.name = MakeUTF8String(str);

			if (parents.empty())
				bookmarks.push_back(bookmark);
			else
				parents.top()->children.push_back(bookmark);

			pos_link = data_lc.find("<a", pos_end);
		}
		else if (pos_list < pos_link && pos_list < pos_list_end)
		{
			// Open new level
			if (parents.empty())
			{
				if (!bookmarks.empty())
					parents.push(&bookmarks.back());
			}
			else if (!parents.top()->children.empty())
			{
				parents.push(&parents.top()->children.back());
			}

			pos_list = data_lc.find("<ul", pos_list + 1);
		}
		else
		{
			// Close a level
			if (!parents.empty())
				parents.pop();
			pos_list_end = data_lc.find("</ul", pos_list_end + 1);
		}
	}

	return true;
}

void InitDjVmNav(const vector<DjVuBookmark>& bookmarks, GP<DjVmNav> nav)
{
	for (size_t i = 0; i < bookmarks.size(); ++i)
	{
		const DjVuBookmark& bookmark = bookmarks[i];
		GP<DjVmNav::DjVuBookMark> djvu_bm = DjVmNav::DjVuBookMark::create(
			bookmark.children.size(), bookmark.name, bookmark.url);

		nav->append(djvu_bm);

		InitDjVmNav(bookmark.children, nav);
	}
}

bool CBookmarkDlg::OpenBookmarks(const CString& strFileName)
{
	CWaitCursor wait;

	ifstream in(strFileName, ios::in);
	if (!in)
	{
		AfxMessageBox(IDS_CANNOT_OPEN_BOOKMARKS_FILE, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	vector<DjVuBookmark> bookmarks;

	istreambuf_iterator<char> begin(in), end;
	vector<char> raw_data;
	copy(begin, end, back_inserter(raw_data));
	in.close();

	raw_data.push_back(0);
	string data = &raw_data[0];
	raw_data.clear();

	if (!ReadBookmarks(data, bookmarks))
	{
		AfxMessageBox(IDS_ERROR_READING_BOOKMARKS, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	m_pBookmarks = DjVmNav::create();
	InitDjVmNav(bookmarks, m_pBookmarks);

	m_bHasBookmarksFile = true;
	m_strBookmarksFile = strFileName;

	HANDLE hFile = ::CreateFile(strFileName, GENERIC_READ,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != NULL)
	{
		::ZeroMemory(&m_ftModified, sizeof(FILETIME));
		::GetFileTime(hFile, NULL, NULL, &m_ftModified);
		::CloseHandle(hFile);
	}

	OnKickIdle();
	m_nBookmarksAction = 1;
	UpdateData(false);

	return true;
}

void CBookmarkDlg::OnBrowseBookmarks()
{
	CString strFileName = (m_bHasBookmarksFile ? m_strBookmarksFile : _T(""));

	CMyFileDialog dlg(true, _T("html"), strFileName,
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		LoadString(IDS_BOOKMARKS_FILTER));

	CString strTitle;
	strTitle.LoadString(IDS_BROWSE_BOOKMARKS);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

	UINT nResult = dlg.DoModal();
	if (nResult != IDOK)
		return;

	OpenBookmarks(dlg.GetPathName());
}

HBRUSH CBookmarkDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH brush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_DJVU_FILE && m_pSource == NULL)
	{
		pDC->SetTextColor(::GetSysColor(COLOR_3DSHADOW));
	}
	if (pWnd->GetDlgCtrlID() == IDC_BOOKMARKS_FILE && !m_bHasBookmarksFile)
	{
		pDC->SetTextColor(::GetSysColor(COLOR_3DSHADOW));
	}

	return brush;
}

void CBookmarkDlg::OnKickIdle()
{
	GetDlgItem(IDC_EXPORT_BOOKMARKS)->EnableWindow(m_pSource != NULL
			&& m_pSource->GetContents() != NULL);
	GetDlgItem(IDC_BOOKMARKS_REMOVE)->EnableWindow(m_pSource != NULL
			&& m_pSource->GetContents() != NULL);
	GetDlgItem(IDC_SAVE)->EnableWindow(m_pSource != NULL);
	GetDlgItem(IDC_SAVE_AS)->EnableWindow(m_pSource != NULL);

	GetDlgItem(IDC_BOOKMARKS_DONTCHANGE)->EnableWindow(m_pSource != NULL);
	GetDlgItem(IDC_BOOKMARKS_REPLACE)->EnableWindow(m_bHasBookmarksFile);
}

ofstream& operator<<(ofstream& out, GP<DjVmNav> nav)
{
	out << "<html>\n<head>\n"
		<< "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\n"
		<< "</head>\n<body>\n<ul>\n";

	int nCount = nav->getBookMarkCount();

	stack<int> count, cur;
	count.push(nCount);
	cur.push(0);

	const GPList<DjVmNav::DjVuBookMark>& bookmarks = nav->getBookMarkList();
	GPosition pos = bookmarks;

	for (int k = 0; k < nCount && !!pos; ++k, ++pos)
	{
		const GP<DjVmNav::DjVuBookMark> bm = bookmarks[pos];

		for (size_t i = 0; i < count.size() - 1; ++i)
			out << "  ";

		wstring strURL;
		MakeWString(bm->url, strURL);

		wstring strName;
		MakeWString(bm->displayname, strName);
		FixWhitespace(strName);

		out << "<li><a";
		if (!strURL.empty())
			out << " href=\"" << (const char*) HTMLEscape(MakeUTF8String(strURL)) << "\"";
		out << ">" << (const char*) HTMLEscape(MakeUTF8String(strName)) << "</a>\n";

		++cur.top();

		if (bm->count > 0)
		{
			count.push(bm->count);
			cur.push(0);

			for (size_t i = 0; i < count.size() - 1; ++i)
				out << "  ";

			out << "<ul>\n";
		}

		while (cur.top() >= count.top())
		{
			for (size_t i = 0; i < count.size() - 1; ++i)
				out << "  ";

			out << "</ul>\n";

			count.pop();
			cur.pop();
		}
	}

	out << "</ul>\n</body>\n</html>\n";
	return out;
}

bool CBookmarkDlg::ExportBookmarks(const CString& strBookmarksFile)
{
	ofstream out(strBookmarksFile, ios::out);
	if (!out)
	{
		AfxMessageBox(IDS_CANNOT_WRITE);
		return false;
	}

	out << m_pSource->GetContents();

	out.close();

	AfxMessageBox(IDS_EXPORT_DONE, MB_OK | MB_ICONINFORMATION);
	return true;
}

void CBookmarkDlg::OnDestroy()
{
	CloseDocument(false);
	m_dropTarget.Revoke();

	CDialog::OnDestroy();
}

void CBookmarkDlg::OnSave()
{
	UpdateData();

	SaveDocument(m_strDjVuFile);
}

void CBookmarkDlg::OnSaveAs()
{
	UpdateData();

	SaveDocument();
}

void CBookmarkDlg::SaveDocument(const CString& strFileName)
{
	if (m_nBookmarksAction == 1 && m_bHasBookmarksFile)
	{
		FILETIME ftModified;
		HANDLE hFile = ::CreateFile(m_strBookmarksFile, GENERIC_READ,
				FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile != NULL)
		{
			::ZeroMemory(&ftModified, sizeof(FILETIME));
			::GetFileTime(hFile, NULL, NULL, &ftModified);
			::CloseHandle(hFile);
		}

		if (memcmp(&ftModified, &m_ftModified, sizeof(FILETIME)) != 0)
		{
			UINT nResult = AfxMessageBox(IDS_BOOKMARKS_CHANGED, MB_ICONEXCLAMATION | MB_YESNOCANCEL);
			if (nResult == IDCANCEL)
				return;
			if (nResult == IDYES && !OpenBookmarks(m_strBookmarksFile))
				return;
		}
	}

	TCHAR szDrive[_MAX_DRIVE], szPath[_MAX_PATH], szName[_MAX_FNAME], szExt[_MAX_EXT];
	_tsplitpath(m_strDjVuFile, szDrive, szPath, szName, szExt);

	CString strNewFile = strFileName;
	if (strNewFile.IsEmpty())
	{
		CString strFileName = szDrive + CString(szPath) + CString(szName) + _T(".new") + CString(szExt);

		CMyFileDialog dlg(false, _T("djvu"), strFileName, OFN_OVERWRITEPROMPT |
			OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN,
			LoadString(IDS_DJVU_FILTER));

		CString strTitle;
		strTitle.LoadString(IDS_EXPORT_DJVU);
		dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

		UINT nResult = dlg.DoModal();
		if (nResult != IDOK)
			return;

		strNewFile = dlg.GetPathName();
	}

	bool bReplace = !!AfxComparePath(strNewFile, m_strDjVuFile);
	if (bReplace)
	{
		TCHAR szNewName[_MAX_PATH];
		if (!GetTempFileName(szDrive + CString(szPath), _T("DJV"), 0, szNewName))
		{
			AfxMessageBox(IDS_ERROR_WRITING_TEMP);
			return;
		}

		strNewFile = szNewName;
	}


	try
	{
		CWaitCursor wait;

		if (m_nBookmarksAction == 1 && m_bHasBookmarksFile)
		{
			m_pSource->GetDjVuDoc()->set_djvm_nav(m_pBookmarks);
		}
		else if (m_nBookmarksAction == 2)
		{
			m_pSource->GetDjVuDoc()->set_djvm_nav(NULL);
		}

		if (!m_pSource->SaveAs(strNewFile))
		    G_THROW(ERR_MSG("save.failed"));
	}
	catch (GException&)
	{
		if (bReplace)
			AfxMessageBox(IDS_ERROR_WRITING_TEMP);
		else
			AfxMessageBox(IDS_ERROR_WRITING_RESULT);

		return;
	}
	catch (...)
	{
		AfxMessageBox(IDS_FATAL_ERROR);
		return;
	}

	CString strOrigFile = (bReplace ? m_strDjVuFile : strNewFile);
	CloseDocument(false);

	if (bReplace)
	{
		TCHAR szTemp[_MAX_PATH];
		if (!GetTempFileName(szDrive + CString(szPath), _T("DJV"), 0, szTemp))
		{
			DeleteFile(strNewFile);
			AfxMessageBox(IDS_ERROR_WRITING_TEMP);
			return;
		}
		DeleteFile(szTemp);

		if (!MoveFile(strOrigFile, szTemp))
		{
			DeleteFile(strNewFile);
			DeleteFile(szTemp);
			AfxMessageBox(IDS_REPLACE_ERROR, MB_OK | MB_ICONERROR);

			OpenDocument(strOrigFile);
			return;
		}

		if (!MoveFile(strNewFile, strOrigFile))
		{
			DeleteFile(strNewFile);
			bool bRestored = !!MoveFile(szTemp, strOrigFile);
			AfxMessageBox(IDS_REPLACE_ERROR, MB_OK | MB_ICONERROR);

			if (bRestored)
				OpenDocument(strOrigFile);
			return;
		}

		DeleteFile(szTemp);
	}

	OpenDocument(strOrigFile);
	AfxMessageBox(IDS_EMBEDDING_DONE, MB_OK | MB_ICONINFORMATION);
}

DROPEFFECT CBookmarkDlg::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
	DWORD dwKeyState, CPoint point)
{
	return OnDragOver(pWnd, pDataObject, dwKeyState, point);
}

DROPEFFECT CBookmarkDlg::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
	DWORD dwKeyState, CPoint point)
{
	if (!pDataObject->IsDataAvailable(CF_HDROP))
		return DROPEFFECT_NONE;

	CRect rcDjVu;
	GetDlgItem(IDC_DJVU_GROUP)->GetWindowRect(rcDjVu);
	ScreenToClient(rcDjVu);

	CRect rcBookmark;
	GetDlgItem(IDC_BOOKMARKS_GROUP)->GetWindowRect(rcBookmark);
	ScreenToClient(rcBookmark);

	if (rcDjVu.PtInRect(point) || rcBookmark.PtInRect(point))
		return DROPEFFECT_LINK;

	return DROPEFFECT_NONE;
}

BOOL CBookmarkDlg::OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
	DROPEFFECT dropEffect, CPoint point)
{
	if (!pDataObject->IsDataAvailable(CF_HDROP))
		return false;

	CRect rcDjVu;
	GetDlgItem(IDC_DJVU_GROUP)->GetWindowRect(rcDjVu);
	ScreenToClient(rcDjVu);

	CRect rcBookmark;
	GetDlgItem(IDC_BOOKMARKS_GROUP)->GetWindowRect(rcBookmark);
	ScreenToClient(rcBookmark);

	if (!rcDjVu.PtInRect(point) && !rcBookmark.PtInRect(point))
		return false;

	HDROP hDrop = (HDROP) pDataObject->GetGlobalData(CF_HDROP);

	UINT nLength = DragQueryFile(hDrop, 0, NULL, 0);
	if (nLength == 0)
		return false;

	CString strPath;
	DragQueryFile(hDrop, 0, strPath.GetBuffer(nLength + 1), nLength + 1);
	strPath.ReleaseBuffer();

	if (rcDjVu.PtInRect(point))
	{
		OpenDocument(strPath);
	}
	else if (rcBookmark.PtInRect(point))
	{
		OpenBookmarks(strPath);
	}

	return true;
}

void CBookmarkDlg::OnDragLeave(CWnd* pWnd)
{
}

DROPEFFECT CBookmarkDlg::OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point)
{
	return DROPEFFECT_NONE;
}
