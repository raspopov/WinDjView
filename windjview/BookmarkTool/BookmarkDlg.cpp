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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct Bookmark
{
	vector<Bookmark> children;
	string name;
	string url;
};


// CBookmarkDlg dialog

bool CBookmarkDlg::bPrompt = true;

CBookmarkDlg::CBookmarkDlg(CWnd* pParent)
	: CDialog(CBookmarkDlg::IDD, pParent), m_bHasDjVuFile(false),
	  m_bHasBookmarkFile(false), m_bPrompt(bPrompt)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_strDjVuFile.LoadString(IDS_BROWSE_DJVU_PROMPT);
	m_strBookmarkFile.LoadString(IDS_BROWSE_BOOKMARKS_PROMPT);
}

void CBookmarkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DJVU_FILE, m_strDjVuFile);
	DDX_Text(pDX, IDC_BOOKMARK_FILE, m_strBookmarkFile);
	DDX_Check(pDX, IDC_PROMPT_FILENAME, m_bPrompt);
}

BEGIN_MESSAGE_MAP(CBookmarkDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_EMBED, OnEmbed)
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
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
	SetIcon(m_hIcon, true);			// Set big icon
	SetIcon(m_hIcon, false);		// Set small icon

	m_dropTarget.Register(this);

	return true;  // return true unless you set the focus to a control
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

void CBookmarkDlg::OnExport()
{
	TCHAR szDrive[_MAX_DRIVE], szPath[_MAX_PATH], szName[_MAX_FNAME], szExt[_MAX_EXT];
	_tsplitpath(m_strDjVuFile, szDrive, szPath, szName, szExt);
	CString strFileName = szDrive + CString(szPath) + CString(szName) + CString(_T(".html"));

	CFileDialog dlg(false, _T("html"), strFileName, OFN_OVERWRITEPROMPT |
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN,
		LoadString(IDS_BOOKMARKS_FILTER));

	CString strTitle;
	strTitle.LoadString(IDS_EXPORT_BOOKMARKS);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

	UINT nResult = dlg.DoModal();
	if (nResult != IDOK)
		return;

	CString strBookmarkFile = dlg.GetPathName();

	if (ExportBookmarks(strBookmarkFile))
	{
		m_strBookmarkFile = strBookmarkFile;
		m_bHasBookmarkFile = true;
		UpdateData(false);
	}
}

void CBookmarkDlg::OnBrowseDjvu()
{
	CString strFileName = (m_bHasDjVuFile ? m_strDjVuFile : _T(""));

	CFileDialog dlg(true, _T("djvu"), strFileName,
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

GUTF8String MakeUTF8String(const CString& strText)
{
	int nSize;

#ifdef _UNICODE
	LPCWSTR pszUnicodeText = strText;
#else
	nSize = ::MultiByteToWideChar(CP_ACP, 0, (LPCSTR)strText, -1, NULL, 0);
	LPWSTR pszUnicodeText = new WCHAR[nSize];
	::MultiByteToWideChar(CP_ACP, 0, (LPCSTR)strText, -1, pszUnicodeText, nSize);
#endif

	nSize = ::WideCharToMultiByte(CP_UTF8, 0, pszUnicodeText, -1, NULL, 0, NULL, NULL);
	LPSTR pszTextUTF8 = new CHAR[nSize];
	::WideCharToMultiByte(CP_UTF8, 0, pszUnicodeText, -1, pszTextUTF8, nSize, NULL, NULL);

	GUTF8String utf8String(pszTextUTF8);
	delete[] pszTextUTF8;

#ifndef _UNICODE
	delete[] pszUnicodeText;
#endif

	return utf8String;
}

bool CBookmarkDlg::OpenDocument(const CString& strFileName)
{
	try
	{
		CWaitCursor wait;

		GURL url = GURL::Filename::UTF8(MakeUTF8String(strFileName));
		GP<DjVuDocument> doc = DjVuDocument::create(url);
		doc->wait_for_complete_init();

		m_strDjVuFile = strFileName;
		m_bHasDjVuFile = true;
		m_pDjVuDoc = doc;
		UpdateData(false);

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
	m_pDjVuDoc = NULL;
	m_bHasDjVuFile = false;

	// Close all open handles to this file
	GURL url = GURL::Filename::UTF8(MakeUTF8String(m_strDjVuFile));
	DataPool::load_file(url);

	m_strDjVuFile.LoadString(IDS_BROWSE_DJVU_PROMPT);

	if (bUpdateData)
		UpdateData(false);
}

void CBookmarkDlg::OnBrowseBookmarks()
{
	CString strFileName = (m_bHasBookmarkFile ? m_strBookmarkFile : _T(""));

	CFileDialog dlg(true, _T("html"), strFileName,
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		LoadString(IDS_BOOKMARKS_FILTER));

	CString strTitle;
	strTitle.LoadString(IDS_BROWSE_BOOKMARKS);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

	UINT nResult = dlg.DoModal();
	if (nResult != IDOK)
		return;

	m_strBookmarkFile = dlg.GetPathName();
	m_bHasBookmarkFile = true;
	UpdateData(false);
}

HBRUSH CBookmarkDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH brush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_DJVU_FILE && !m_bHasDjVuFile)
	{
		pDC->SetTextColor(::GetSysColor(COLOR_3DSHADOW));
	}
	if (pWnd->GetDlgCtrlID() == IDC_BOOKMARK_FILE && !m_bHasBookmarkFile)
	{
		pDC->SetTextColor(::GetSysColor(COLOR_3DSHADOW));
	}

	return brush;
}

void CBookmarkDlg::OnKickIdle()
{
	GP<DjVmNav> nav;
	if (m_pDjVuDoc != NULL)
		nav = m_pDjVuDoc->get_djvm_nav();

	GetDlgItem(IDC_EXPORT)->EnableWindow(m_bHasDjVuFile && nav != NULL);
	GetDlgItem(IDC_EMBED)->EnableWindow(m_bHasBookmarkFile);
}

string html_unescape(string str)
{
	size_t pos = 0;
	while ((pos = str.find("&lt;", pos)) != string::npos)
		str.replace(pos, 4, "<");

	pos = 0;
	while ((pos = str.find("&gt;", pos)) != string::npos)
		str.replace(pos, 4, ">");

	pos = 0;
	while ((pos = str.find("&quot;", pos)) != string::npos)
		str.replace(pos, 6, "\"");

	pos = 0;
	while ((pos = str.find("&amp;", pos)) != string::npos)
		str.replace(pos, 5, "&");

	return str;
}

string html_escape(string str)
{
	size_t pos = 0;
	while ((pos = str.find("&", pos)) != string::npos)
		str.replace(pos++, 1, "&amp;");

	pos = 0;
	while ((pos = str.find("<", pos)) != string::npos)
		str.replace(pos++, 1, "&lt;");

	pos = 0;
	while ((pos = str.find(">", pos)) != string::npos)
		str.replace(pos++, 1, "&gt;");

	pos = 0;
	while ((pos = str.find("\"", pos)) != string::npos)
		str.replace(pos++, 1, "&quot;");

	return str;
}

ofstream& operator<<(ofstream& out, GP<DjVmNav> nav)
{
	out << "<html>" << endl << "<head>" << endl
		<< "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">" << endl
		<< "</head>" << endl << "<body>" << endl << "<ul>" << endl;

	stack<int> count, cur;
	count.push(nav->getBookMarkCount());
	cur.push(0);

	for (int pos = 0; pos < nav->getBookMarkCount(); ++pos)
	{
		GP<DjVmNav::DjVuBookMark> bm;
		nav->getBookMark(bm, pos);

		for (size_t i = 0; i < count.size() - 1; ++i)
			out << "  ";

		out << "<li><a href=\"" << html_escape((const char*)bm->url) << "\">"
			<< html_escape((const char*)bm->displayname) << "</a>" << endl;

		++cur.top();

		if (bm->count > 0)
		{
			count.push(bm->count);
			cur.push(0);

			for (size_t i = 0; i < count.size() - 1; ++i)
				out << "  ";

			out << "<ul>" << endl;
		}

		while (cur.top() >= count.top())
		{
			for (size_t i = 0; i < count.size() - 1; ++i)
				out << "  ";

			out << "</ul>" << endl;

			count.pop();
			cur.pop();
		}
	}

	out << "</ul>" << endl << "</body>" << endl << "</html>" << endl;
	return out;
}

bool CBookmarkDlg::ExportBookmarks(const CString& strBookmarkFile)
{
	try
	{
		CWaitCursor wait;

		GP<DjVmNav> nav = m_pDjVuDoc->get_djvm_nav();
		if (nav == NULL)
		{
			AfxMessageBox(IDS_NO_BOOKMARKS);
			return false;
		}

		ofstream out(strBookmarkFile, ios::out);
		if (!out)
		{
			AfxMessageBox(IDS_CANNOT_WRITE);
			return false;
		}

		out << nav;

		out.close();

		AfxMessageBox(IDS_EXPORT_DONE, MB_OK | MB_ICONINFORMATION);
		return true;
	}
	catch (GException&)
	{
		AfxMessageBox(IDS_EXPORT_ERROR);
		return false;
	}
	catch (...)
	{
		AfxMessageBox(IDS_FATAL_ERROR);
		return false;
	}
}

void CBookmarkDlg::OnDestroy()
{
	UpdateData();
	bPrompt = !!m_bPrompt;

	m_dropTarget.Revoke();

	CDialog::OnDestroy();
}

inline string tolower(string str)
{
	for (string::iterator it = str.begin(); it != str.end(); ++it)
		*it = tolower(*it, locale());
	return str;
}

bool read_bookmarks(const string& data, vector<Bookmark>& bookmarks)
{
	stack<Bookmark*> parents;

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
			Bookmark bookmark;

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

				bookmark.url = html_unescape(data.substr(pos + 1, pos_end - pos - 1));
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

			bookmark.name = html_unescape(data.substr(pos + 1, pos_end - pos - 1));

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

void create_djvu_bookmarks(const vector<Bookmark>& bookmarks, GP<DjVmNav> nav)
{
	for (size_t i = 0; i < bookmarks.size(); ++i)
	{
		const Bookmark& bookmark = bookmarks[i];
		GP<DjVmNav::DjVuBookMark> djvu_bm = DjVmNav::DjVuBookMark::create(
			bookmark.children.size(),
			bookmark.name.c_str(),
			bookmark.url.c_str());

		nav->append(djvu_bm);

		create_djvu_bookmarks(bookmark.children, nav);
	}
}

void CBookmarkDlg::OnEmbed()
{
	UpdateData();

	ifstream in(m_strBookmarkFile, ios::in);
	if (!in)
	{
		AfxMessageBox(IDS_CANNOT_OPEN_BOOKMARK_FILE, MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	vector<Bookmark> bookmarks;

	istreambuf_iterator<char> begin(in), end;
	vector<char> raw_data;
	copy(begin, end, back_inserter(raw_data));

	raw_data.push_back(0);
	string data = &raw_data[0];

	raw_data.clear();

	if (!read_bookmarks(data, bookmarks))
	{
		AfxMessageBox(IDS_ERROR_READING_BOOKMARKS, MB_OK | MB_ICONEXCLAMATION);
		in.close();
		return;
	}
	in.close();

	TCHAR szDrive[_MAX_DRIVE], szPath[_MAX_PATH], szName[_MAX_FNAME], szExt[_MAX_EXT];
	_tsplitpath(m_strDjVuFile, szDrive, szPath, szName, szExt);

	CString strNewFile = m_strDjVuFile;
	if (m_bPrompt)
	{
		CString strFileName = szDrive + CString(szPath) + CString(szName) + _T(".new") + CString(szExt);

		CFileDialog dlg(false, _T("djvu"), strFileName, OFN_OVERWRITEPROMPT |
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
		if (!GetTempFileName(szDrive + CString(szPath), "DJV", 0, szNewName))
		{
			AfxMessageBox(IDS_ERROR_WRITING_TEMP);
			return;
		}

		strNewFile = szNewName;
	}

	GP<DjVmNav> nav = NULL;
	if (!bookmarks.empty())
	{
		nav = DjVmNav::create();
		create_djvu_bookmarks(bookmarks, nav);
	}

	try
	{
		m_pDjVuDoc->set_djvm_nav(nav);

		GURL url = GURL::Filename::UTF8(MakeUTF8String(strNewFile));

		DataPool::load_file(url);
		m_pDjVuDoc->write(ByteStream::create(url, "wb"), true);
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

	if (bReplace)
	{
		CString strOrigFile = m_strDjVuFile;
		CloseDocument(false);

		TCHAR szTemp[_MAX_PATH];
		if (!GetTempFileName(szDrive + CString(szPath), "DJV", 0, szTemp))
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

		OpenDocument(strOrigFile);
	}

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
	GetDlgItem(IDC_BOOKMARK_GROUP)->GetWindowRect(rcBookmark);
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
	GetDlgItem(IDC_BOOKMARK_GROUP)->GetWindowRect(rcBookmark);
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
		m_strBookmarkFile = strPath;
		m_bHasBookmarkFile = true;
		UpdateData(false);
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
