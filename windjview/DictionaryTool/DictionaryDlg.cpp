//	DjVu Dictionary Tool
//	Copyright (C) 2006-2007 Andrew Zhezherun
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
#include "DictionaryTool.h"
#include "DictionaryDlg.h"
#include "Global.h"
#include "MyFileDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDictionaryDlg dialog

bool CDictionaryDlg::bPrompt = true;

CDictionaryDlg::CDictionaryDlg(CWnd* pParent)
	: CDialog(CDictionaryDlg::IDD, pParent), m_bHasDjVuFile(false),
	  m_bHasPageIndexFile(false), m_bPrompt(bPrompt)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_strDjVuFile.LoadString(IDS_BROWSE_DJVU_PROMPT);
	m_strPageIndexFile.LoadString(IDS_BROWSE_PAGE_INDEX_PROMPT);
}

void CDictionaryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DJVU_FILE, m_strDjVuFile);
	DDX_Text(pDX, IDC_PAGE_INDEX_FILE, m_strPageIndexFile);
	DDX_Check(pDX, IDC_PROMPT_FILENAME, m_bPrompt);
}

BEGIN_MESSAGE_MAP(CDictionaryDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_EMBED, OnEmbed)
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
	ON_BN_CLICKED(IDC_BROWSE_DJVU, OnBrowseDjvu)
	ON_BN_CLICKED(IDC_BROWSE_PAGE_INDEX, OnBrowsePageIndex)
	ON_WM_CTLCOLOR()
	ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDictionaryDlg message handlers

BOOL CDictionaryDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.
	SetIcon(m_hIcon, true);			// Set big icon
	SetIcon(m_hIcon, false);		// Set small icon

	m_dropTarget.Register(this);

	return true;  // return true unless you set the focus to a control
}

void CDictionaryDlg::OnPaint() 
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
HCURSOR CDictionaryDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDictionaryDlg::OnExport()
{
	TCHAR szDrive[_MAX_DRIVE], szPath[_MAX_PATH], szName[_MAX_FNAME], szExt[_MAX_EXT];
	_tsplitpath(m_strDjVuFile, szDrive, szPath, szName, szExt);
	CString strFileName = szDrive + CString(szPath) + CString(szName) + CString(_T(".xml"));

	CMyFileDialog dlg(false, _T("xml"), strFileName, OFN_OVERWRITEPROMPT |
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN,
		LoadString(IDS_PAGE_INDEX_FILTER_EXPORT));

	CString strTitle;
	strTitle.LoadString(IDS_EXPORT_PAGE_INDEX);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

	UINT nResult = dlg.DoModal();
	if (nResult != IDOK)
		return;

	CString strPageIndexFile = dlg.GetPathName();

	if (ExportPageIndex(strPageIndexFile))
	{
		m_strPageIndexFile = strPageIndexFile;
		m_bHasPageIndexFile = true;
		UpdateData(false);
	}
}

void CDictionaryDlg::OnBrowseDjvu()
{
	CString strFileName = (m_bHasDjVuFile ? m_strDjVuFile : _T(""));

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

bool CDictionaryDlg::OpenDocument(const CString& strFileName)
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

void CDictionaryDlg::CloseDocument(bool bUpdateData)
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

void CDictionaryDlg::OnBrowsePageIndex()
{
	CString strFileName = (m_bHasPageIndexFile ? m_strPageIndexFile : _T(""));

	CMyFileDialog dlg(true, _T("html"), strFileName,
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		LoadString(IDS_PAGE_INDEX_FILTER));

	CString strTitle;
	strTitle.LoadString(IDS_BROWSE_PAGE_INDEX);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

	UINT nResult = dlg.DoModal();
	if (nResult != IDOK)
		return;

	m_strPageIndexFile = dlg.GetPathName();
	m_bHasPageIndexFile = true;
	UpdateData(false);
}

HBRUSH CDictionaryDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH brush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_DJVU_FILE && !m_bHasDjVuFile)
	{
		pDC->SetTextColor(::GetSysColor(COLOR_3DSHADOW));
	}
	if (pWnd->GetDlgCtrlID() == IDC_PAGE_INDEX_FILE && !m_bHasPageIndexFile)
	{
		pDC->SetTextColor(::GetSysColor(COLOR_3DSHADOW));
	}

	return brush;
}

void CDictionaryDlg::OnKickIdle()
{
	GetDlgItem(IDC_EXPORT)->EnableWindow(m_bHasDjVuFile);
	GetDlgItem(IDC_EMBED)->EnableWindow(m_bHasPageIndexFile);
}

bool CDictionaryDlg::ExportPageIndex(const CString& strPageIndexFile)
{
	try
	{
		CWaitCursor wait;

		GP<DjVuImage> page = m_pDjVuDoc->get_page(0);
		if (page == NULL)
		{
			AfxMessageBox(IDS_NO_PAGE_INDEX);
			return false;
		}

		GP<ByteStream> pAnnoStream = page->get_anno();
		pAnnoStream->seek(0);
		GP<DjVuAnno> pDjVuAnno = DjVuAnno::create();
		pDjVuAnno->decode(pAnnoStream);
		GUTF8String strPageIndex = pDjVuAnno->ant->metadata["page-index"];

		if (strPageIndex.length() == 0)
		{
			AfxMessageBox(IDS_NO_PAGE_INDEX);
			return false;
		}

		ofstream out(strPageIndexFile, ios::out);
		if (!out)
		{
			AfxMessageBox(IDS_CANNOT_WRITE);
			return false;
		}

		out << (const char*)strPageIndex;

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

void CDictionaryDlg::OnDestroy()
{
	UpdateData();
	bPrompt = !!m_bPrompt;

	m_dropTarget.Revoke();

	CDialog::OnDestroy();
}

void WriteString(const char* data, int length, ByteStream& out)
{
	out.write("\"", 1);
	while (*data != 0 && length > 0)
	{
		int span = 0;
		while (span < length && data[span] >= 0x20
				&& data[span] != '\"' && data[span] != '\\')
			span++;

		if (span > 0)
		{
			out.write(data, span);
			data += span;
			length -= span;
		}
		else
		{
			char buf[5];
			static char* tr1 = "\"\\tnrbf";
			static char* tr2 = "\"\\\t\n\r\b\f";
			sprintf(buf, "\\%03o", (int)(((unsigned char*)data)[0]));

			for (int i = 0; tr2[i] != 0; i++)
				if (*data == tr2[i])
					buf[1] = tr1[i];

			if (buf[1] < '0' || buf[1] > '3')
				buf[2] = 0;
			out.write(buf, ((buf[2]) ? 4 : 2));
			data += 1;
			length -= 1;
		}
	}

	out.write("\"", 1);
}

// --------------------------------------------------
// PARSING BYTESTREAM
// --------------------------------------------------

// A bytestream that performs buffering and 
// offers a stdio-like interface for reading files.

class ParsingByteStream : public ByteStream 
{
private:
	enum { bufsize = 512 };
	const GP<ByteStream>& gbs;
	ByteStream& bs;
	unsigned char buffer[bufsize];
	int bufpos;
	int bufend;
	bool goteof;
	ParsingByteStream(const GP<ByteStream>& gbs);

public:
	static GP<ParsingByteStream> create(const GP<ByteStream>& gbs)
		{ return new ParsingByteStream(gbs); }
	size_t read(void *buffer, size_t size);
	size_t write(const void *buffer, size_t size);
	long tell() const;
	int eof();
	int unget(int c);
	inline int get();
	int get_spaces(bool skipseparator = false);
	GUTF8String get_utf8_token(bool skipseparator = false, bool compat = false);
	GUTF8String get_native_token(bool skipseparator = false, bool compat = false);
};

ParsingByteStream::ParsingByteStream(const GP<ByteStream>& xgbs)
	: gbs(xgbs), bs(*gbs), bufpos(1), bufend(1), goteof(false)
{ 
}

int ParsingByteStream::eof()
{
	if (bufpos < bufend) 
		return false;
	if (goteof)
		return true;
	bufend = bufpos = 1;
	while (bs.read(buffer + bufend, 1) && ++bufend < (int)bufsize)
	{
		if (buffer[bufend - 1] == '\r' || buffer[bufend - 1] == '\n')
			break;
	}
	if (bufend == bufpos)
		goteof = true;
	return goteof;
}

size_t ParsingByteStream::read(void* buf, size_t size)
{
	if (size < 1)
		return 0;
	if (bufend == bufpos) 
    {
		if (size >= bufsize)
			return bs.read(buf, size);
		if (eof())
			return 0;
    }
	if (bufpos + (int)size > bufend)
		size = bufend - bufpos;
	memcpy(buf, buffer + bufpos, size);
	bufpos += size;
	return size;
}

size_t ParsingByteStream::write(const void*, size_t )
{
	G_THROW("Cannot write() into a ParsingByteStream");
	return 0;
}

long ParsingByteStream::tell() const
{ 
	G_THROW("Cannot tell() a ParsingByteStream");
	return 0;
}

inline int ParsingByteStream::get()
{
	if (bufpos < bufend || !eof())
		return buffer[bufpos++];
	return EOF;
}

int ParsingByteStream::unget(int c)
{
	if (bufpos > 0 && c != EOF) 
		return buffer[--bufpos] = (unsigned char)c;
	return EOF;
}

int ParsingByteStream::get_spaces(bool skipseparator)
{
	int c = get();
	while (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '#' || c == ';')
	{
		if (c == '#')
		{
			do
			{
				c = get();
			} while (c != EOF && c != '\n' && c != '\r');
		}
		if (!skipseparator && (c == '\n' || c == '\r' || c == ';'))
			break;
		c = get();
	}
	return c;
}

GUTF8String ParsingByteStream::get_utf8_token(bool skipseparator, bool compat)
{
	GUTF8String str;
	int c = get_spaces(skipseparator);
	if (c == EOF)
	{
		return str;
	}
	if (!skipseparator && (c == '\n' || c == '\r' || c == ';'))
	{
		unget(c);
		return str;
	}
	if (c != '\"' && c != '\'')
	{
		while (c != ' ' && c != '\t' && c != '\r' && c != ';' && c != '\n' && c != '#' && c != EOF)
		{
			str += c;
			c = get();
		}
		unget(c);
	}
	else 
	{
		int delim = c;
		c = get();
		while (c != delim && c != EOF) 
		{
			if (c == '\\') 
			{
				c = get();
				if (compat && c != '\"')
				{
					str += '\\';
				}
				else if (c >= '0' && c <= '7')
				{
					int x = 0;
					for (int i = 0; i < 3 && c >= '0' && c <= '7'; i++)
					{
						x = x * 8 + c - '0';
						c = get();
					}
					unget(c);
					c = x;
				}
				else 
				{
					char *tr1 = "tnrbfva";
					char *tr2 = "\t\n\r\b\f\013\007";
					for (int i = 0; tr1[i]; i++)
					{
						if (c == tr1[i])
							c = tr2[i];
					}
                }
            }
			if (c != EOF)
				str += c;
			c = get();
        }
    }
	return str;
}

GUTF8String ParsingByteStream::get_native_token(bool skipseparator, bool compat)
{
	GUTF8String fake = get_utf8_token(skipseparator, compat);
	GNativeString nat((const char*)fake);
	return GUTF8String(nat);
}

bool DeleteMetadata(GP<ByteStream> in, GP<ByteStream> out)
{
	int c;
	int plevel = 0;
	bool copy = true;
	bool unchanged = true;
	bool compat = false;
	GP<ByteStream> mem;
	GP<ParsingByteStream> inp;

	// Check correctness
	{
		mem = ByteStream::create();
		mem->copy(*in);
		mem->seek(0);
		char c;
		int state = 0;
		while (!compat && mem->read(&c, 1) > 0)
		{
			switch(state)
			{
			case 0:
				if (c == '\"')
					state = '\"';
				break;

			case '\"':
				if (c == '\"')
					state = 0;
				else if (c == '\\')
					state = '\\';
				else if ((unsigned char)c < 0x20 || c == 0x7f)
					compat = true;
				break;

			case '\\':
				if (!strchr("01234567tnrbfva\"\\", c))
					compat = true;
				state = '\"';
				break;
			}
		}
		mem->seek(0);
		inp = ParsingByteStream::create(mem);
	}
  
	while ((c = inp->get()) != EOF)
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
			break;

	inp->unget(c);
	while ((c = inp->get()) != EOF)
	{
		if (plevel == 0)
			if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
				copy = true;

		if (c == '\"')
		{
			inp->unget(c);
			GUTF8String token = inp->get_utf8_token(false, compat);
			if (copy)
				WriteString(token, token.length(), *out);
			if (compat)
				unchanged = false;
		}
		else if (c == '(')
		{
			while ((c = inp->get()) != EOF)
				if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
					break;

			inp->unget(c);
			if (plevel == 0 && c == 'm')
			{
				GUTF8String token = inp->get_utf8_token();
				if (token == "metadata")
					copy = unchanged = false;
				if (copy)
				{
					out->write8('(');
					out->write((const char*)token, token.length());
				}
			}
			else if (copy) 
			{
				out->write8('(');
			}

			plevel += 1;
		}
		else if (c == ')')
		{
			if (copy) 
				out->write8(c);
			if (--plevel < 0)
				plevel = 0;
		}
		else if (copy)
			out->write8(c);
	}

	return !unchanged;
}

bool DeleteMetadata(GP<IFFByteStream> iff, GP<ByteStream> out)
{
	GUTF8String chkid;
	bool bChanged = false;
	while (iff->get_chunk(chkid))
	{
		if (chkid == "ANTa") 
		{
			bChanged = DeleteMetadata(iff->get_bytestream(), out);
		}
		else if (chkid == "ANTz") 
		{
			GP<ByteStream> bsiff = BSByteStream::create(iff->get_bytestream());
			bChanged = DeleteMetadata(bsiff, out);
		}
		iff->close_chunk();
	}
	return bChanged;
}

bool ModifyMeta(GP<DjVuFile> pFile, GMap<GUTF8String, GUTF8String>* pMeta)
{
	bool bChanged = false;
	GP<ByteStream> pAntStream = ByteStream::create();
	if (pMeta && !pMeta->isempty())
	{
		pAntStream->writestring(GUTF8String("(metadata"));
		for (GPosition pos = pMeta->firstpos(); pos; ++pos)
		{
			GUTF8String key = pMeta->key(pos); 
			GUTF8String val = (*pMeta)[pos];
			pAntStream->write("\n\t(", 3);
			pAntStream->writestring(key);
			pAntStream->write(" ", 1);
			WriteString((const char*)val, val.length(), *pAntStream);
			pAntStream->write(")", 1);
		}
		pAntStream->write(" )\n", 3);
		bChanged = true;
	}

	GP<ByteStream> pOldAnno = pFile->get_anno();
	if (pOldAnno && pOldAnno->size()) 
	{
		GP<IFFByteStream> iff = IFFByteStream::create(pOldAnno);
		if (DeleteMetadata(iff, pAntStream))
			bChanged = true;
	}

	const GP<ByteStream> pAntz = ByteStream::create();
	if (bChanged)
	{
		GP<ByteStream> bzz = BSByteStream::create(pAntz, 100);

		pAntStream->seek(0);
		bzz->copy(*pAntStream);
		bzz = 0;

		pAntz->seek(0);

		const GP<ByteStream> pStream(ByteStream::create());
		if (pAntz->size())
		{
			const GP<IFFByteStream> out(IFFByteStream::create(pStream));
			out->put_chunk("ANTz");
			out->copy(*pAntz);
			out->close_chunk();
		}

		pFile->anno = pStream;
		if (!pStream->size())
			pFile->remove_anno();
		pFile->set_modified(true);
	}
	
	return bChanged;
}

GUTF8String ReadRawPageIndex(LPCTSTR pszFileName)
{
	ifstream in(pszFileName, ios::in);
	if (!in)
		return "";

	istreambuf_iterator<char> begin(in), end;
	vector<char> raw_data;
	copy(begin, end, back_inserter(raw_data));
	in.close();

	raw_data.push_back(0);
	return GUTF8String(&raw_data[0]);
}

GUTF8String ReadExcelPageIndex(LPCTSTR pszFileName)
{
	Excel::_ApplicationPtr pApplication;

	if (FAILED(pApplication.CreateInstance(_T("Excel.Application"))))
		return "";

	GUTF8String strResult;
	strResult += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	strResult += "<index>\n";

	try
	{
		int nLevel = 1;
		bool bFirst = true;
		_variant_t varOption((long)DISP_E_PARAMNOTFOUND, VT_ERROR);

		Excel::_WorkbookPtr pBook = pApplication->Workbooks->Open(
				pszFileName, 0L, VARIANT_TRUE, varOption, varOption,
				varOption, varOption, varOption, varOption,
				varOption, varOption, varOption, varOption);
		Excel::_WorksheetPtr pSheet = pBook->Sheets->Item[1L];

		Excel::RangePtr pRange = pSheet->GetRange(_bstr_t("A1"), _bstr_t("Z16384"));
		for (long nRow = 1; nRow < 16384; ++nRow)
		{
			_bstr_t bstrLevel(_variant_t(pRange->Item[nRow][1L]));
			_bstr_t bstrFirst(_variant_t(pRange->Item[nRow][2L]));
			_bstr_t bstrLast(_variant_t(pRange->Item[nRow][3L]));
			_bstr_t bstrURL(_variant_t(pRange->Item[nRow][4L]));

			if (bstrLevel.length() == 0)
				break;

			int nNewLevel = _wtoi(bstrLevel);
			if (bFirst)
			{
				if (nNewLevel <= 0 || nNewLevel > 1)
					return "";
				bFirst = false;
			}
			else
			{
				if (nNewLevel <= 0 || nNewLevel > nNewLevel + 1)
					return "";
				if (nNewLevel == nLevel + 1)
					strResult += ">\n";
				else
				{
					strResult += "/>\n";
					for (int i = nNewLevel; i < nLevel; ++i)
						strResult += "</entry>\n";
				}
			}
			nLevel = nNewLevel;

			strResult += "<entry";

			if (bstrFirst.length() > 0)
				strResult += " first=\"" + MakeUTF8String(wstring(bstrFirst)).toEscaped() + "\"";
			if (bstrLast.length() > 0)
				strResult += " last=\"" + MakeUTF8String(wstring(bstrLast)).toEscaped() + "\"";
			if (bstrURL.length() > 0)
				strResult += " url=\"" + MakeUTF8String(wstring(bstrURL)).toEscaped() + "\"";
		}

		if (!bFirst)
		{
			strResult += "/>\n";
			for (int i = 1; i < nLevel; ++i)
				strResult += "</entry>\n";
		}
		strResult += "</index>\n";

		pBook->Close(VARIANT_FALSE);
	}
	catch (_com_error&)
	{
		return "";
	}

	try
	{
		pApplication->Quit();
	}
	catch (_com_error&)
	{
	}

	return strResult;
}

void CDictionaryDlg::OnEmbed()
{
	UpdateData();

	TCHAR szInputExt[_MAX_EXT];
	_tsplitpath(m_strPageIndexFile, NULL, NULL, NULL, szInputExt);

	GUTF8String strPageIndex;
	if (CString(szInputExt).CompareNoCase(_T(".xml")) == 0)
		strPageIndex = ReadRawPageIndex(m_strPageIndexFile);
	else
		strPageIndex = ReadExcelPageIndex(m_strPageIndexFile);

	if (strPageIndex.length() == 0)
	{
		AfxMessageBox(IDS_CANNOT_OPEN_INDEX_FILE, MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	TCHAR szDrive[_MAX_DRIVE], szPath[_MAX_PATH], szName[_MAX_FNAME], szExt[_MAX_EXT];
	_tsplitpath(m_strDjVuFile, szDrive, szPath, szName, szExt);

	CString strNewFile = m_strDjVuFile;
	if (m_bPrompt)
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
		if (!GetTempFileName(szDrive + CString(szPath), "DJV", 0, szNewName))
		{
			AfxMessageBox(IDS_ERROR_WRITING_TEMP);
			return;
		}

		strNewFile = szNewName;
	}

	try
	{
		GP<DjVuFile> pFile = m_pDjVuDoc->get_djvu_file(0);
		if (pFile == NULL)
		{
			AfxMessageBox(IDS_NO_PAGE_INDEX);
			return;
		}

		GMap<GUTF8String, GUTF8String> meta;

		GP<ByteStream> pAnnoStream = pFile->get_anno();
		if (pAnnoStream && pAnnoStream->size())
		{
			pAnnoStream->seek(0);
			GP<DjVuAnno> pDjVuAnno = DjVuAnno::create();
			pDjVuAnno->decode(pAnnoStream);
			meta = pDjVuAnno->ant->metadata;
		}

		meta["page-index"] = strPageIndex;
		ModifyMeta(pFile, &meta);

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

DROPEFFECT CDictionaryDlg::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
	DWORD dwKeyState, CPoint point)
{
	return OnDragOver(pWnd, pDataObject, dwKeyState, point);
}

DROPEFFECT CDictionaryDlg::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
	DWORD dwKeyState, CPoint point)
{
	if (!pDataObject->IsDataAvailable(CF_HDROP))
		return DROPEFFECT_NONE;

	CRect rcDjVu;
	GetDlgItem(IDC_DJVU_GROUP)->GetWindowRect(rcDjVu);
	ScreenToClient(rcDjVu);

	CRect rcBookmark;
	GetDlgItem(IDC_PAGE_INDEX_GROUP)->GetWindowRect(rcBookmark);
	ScreenToClient(rcBookmark);

	if (rcDjVu.PtInRect(point) || rcBookmark.PtInRect(point))
		return DROPEFFECT_LINK;

	return DROPEFFECT_NONE;
}

BOOL CDictionaryDlg::OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
	DROPEFFECT dropEffect, CPoint point)
{
	if (!pDataObject->IsDataAvailable(CF_HDROP))
		return false;

	CRect rcDjVu;
	GetDlgItem(IDC_DJVU_GROUP)->GetWindowRect(rcDjVu);
	ScreenToClient(rcDjVu);

	CRect rcBookmark;
	GetDlgItem(IDC_PAGE_INDEX_GROUP)->GetWindowRect(rcBookmark);
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
		m_strPageIndexFile = strPath;
		m_bHasPageIndexFile = true;
		UpdateData(false);
	}

	return true;
}

void CDictionaryDlg::OnDragLeave(CWnd* pWnd)
{
}

DROPEFFECT CDictionaryDlg::OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point)
{
	return DROPEFFECT_NONE;
}
