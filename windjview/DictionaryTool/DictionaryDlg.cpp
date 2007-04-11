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
#include "DjVuSource.h"
#include "LocalizedStringDlg.h"
#include "XMLParser.h"
#include "WarningsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const char pszPageIndexKey[] = "page-index";
static const char pszCharMapKey[] = "char-map";
static const char pszTitleKey[] = "title-localized";
static const char pszLangFromKey[] = "language-from";
static const char pszLangToKey[] = "language-to";


// CDictionaryDlg dialog

CDictionaryDlg::CDictionaryDlg(CWnd* pParent)
	: CDialog(CDictionaryDlg::IDD, pParent), m_pSource(NULL), m_pDictInfo(NULL),
	  m_bHasPageIndexFile(false), m_bHasCharMapFile(false),
	  m_nPageIndexAction(0), m_nCharMapAction(0), m_pLangFrom(NULL), m_pLangTo(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_strDjVuFile.LoadString(IDS_BROWSE_DJVU_PROMPT);
	m_strPageIndexFile.LoadString(IDS_BROWSE_PAGE_INDEX_PROMPT);
	m_strCharMapFile.LoadString(IDS_BROWSE_CHAR_MAP_PROMPT);
}

void CDictionaryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DJVU_FILE, m_strDjVuFile);
	DDX_Text(pDX, IDC_PAGE_INDEX_FILE, m_strPageIndexFile);
	DDX_Text(pDX, IDC_CHAR_MAP_FILE, m_strCharMapFile);
	DDX_Text(pDX, IDC_TITLE, m_strTitle);
	DDX_Radio(pDX, IDC_PAGE_INDEX_DONTCHANGE, m_nPageIndexAction);
	DDX_Radio(pDX, IDC_CHAR_MAP_DONTCHANGE, m_nCharMapAction);
	DDX_Control(pDX, IDC_LANG_FROM, m_cboLangFrom);
	DDX_Control(pDX, IDC_LANG_TO, m_cboLangTo);
}

BEGIN_MESSAGE_MAP(CDictionaryDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_BN_CLICKED(IDC_SAVE_AS, OnSaveAs)
	ON_BN_CLICKED(IDC_EXPORT_PAGE_INDEX, OnExportPageIndex)
	ON_BN_CLICKED(IDC_EXPORT_CHAR_MAP, OnExportCharMap)
	ON_BN_CLICKED(IDC_BROWSE_DJVU, OnBrowseDjvu)
	ON_BN_CLICKED(IDC_BROWSE_PAGE_INDEX, OnBrowsePageIndex)
	ON_BN_CLICKED(IDC_BROWSE_CHAR_MAP, OnBrowseCharMap)
	ON_WM_CTLCOLOR()
	ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_LANG_FROM, OnSelChangeFrom)
	ON_CBN_SELCHANGE(IDC_LANG_TO, OnSelChangeTo)
	ON_BN_CLICKED(IDC_TITLE_LOCALIZED, OnLocalizeTitle)
	ON_BN_CLICKED(IDC_FROM_LOCALIZED, OnLocalizeLangFrom)
	ON_BN_CLICKED(IDC_TO_LOCALIZED, OnLocalizeLangTo)
	ON_BN_CLICKED(IDC_WARNINGS, OnWarnings)
END_MESSAGE_MAP()


// CDictionaryDlg message handlers

BOOL CDictionaryDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.
	SetIcon(m_hIcon, true);	// Set big icon
	SetIcon(m_hIcon, false); // Set small icon

	m_dropTarget.Register(this);
	OnKickIdle();

	m_cboLangFrom.AddString(_T("(Not selected)"));
	m_cboLangTo.AddString(_T("(Not selected)"));
	for (size_t i = 0; i < theApp.GetLanguageCount(); ++i)
	{
		const Language& lang = theApp.GetLanguage(i);
		m_cboLangFrom.AddString(lang.strName);
		m_cboLangTo.AddString(lang.strName);
	}

	m_cboLangFrom.SetCurSel(0);
	m_cboLangTo.SetCurSel(0);

	return true;
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

void CDictionaryDlg::OnExportPageIndex()
{
	if (m_pDictInfo->strPageIndex.length() == 0)
		return;

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

	if (ExportPageIndex(strPageIndexFile) && !m_bHasPageIndexFile)
	{
		m_strPageIndexFile = strPageIndexFile;
		m_strPageIndexXML = m_pDictInfo->strPageIndex;
		m_bHasPageIndexFile = true;
		UpdateData(false);
	}
}

void CDictionaryDlg::OnExportCharMap()
{
	if (m_pDictInfo->strCharMap.length() == 0)
		return;

	TCHAR szDrive[_MAX_DRIVE], szPath[_MAX_PATH], szName[_MAX_FNAME], szExt[_MAX_EXT];
	_tsplitpath(m_strDjVuFile, szDrive, szPath, szName, szExt);
	CString strFileName = szDrive + CString(szPath) + CString(szName) + CString(_T(".chars.xml"));

	CMyFileDialog dlg(false, _T("xml"), strFileName, OFN_OVERWRITEPROMPT |
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN,
		LoadString(IDS_CHAR_MAP_FILTER_EXPORT));

	CString strTitle;
	strTitle.LoadString(IDS_EXPORT_CHAR_MAP);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

	UINT nResult = dlg.DoModal();
	if (nResult != IDOK)
		return;

	CString strCharMapFile = dlg.GetPathName();

	if (ExportCharMap(strCharMapFile) && !m_bHasCharMapFile)
	{
		m_strCharMapFile = strCharMapFile;
		m_strCharMapXML = m_pDictInfo->strCharMap;
		m_bHasCharMapFile = true;
		UpdateData(false);
	}
}

void CDictionaryDlg::OnBrowseDjvu()
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
	if (OpenDocument(strDjVuFile))
		m_warnings.clear();
}

bool CDictionaryDlg::OpenDocument(const CString& strFileName)
{
	try
	{
		CWaitCursor wait;

		DjVuSource* pNewSource = DjVuSource::FromFile(strFileName);
		if (m_pSource == pNewSource)
			return false;

		if (m_pSource != NULL)
			m_pSource->Release();
		m_pSource = pNewSource;

		m_pDictInfo = m_pSource->GetDictionaryInfo();
		m_strDjVuFile = strFileName;

		if (m_pDictInfo->strPageIndex.length() == 0 && m_nPageIndexAction == 2)
			m_nPageIndexAction = 0;
		if (m_pDictInfo->strCharMap.length() == 0 && m_nCharMapAction == 2)
			m_nCharMapAction = 0;

		InitDocProperties();

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

void CDictionaryDlg::CloseDocument(bool bUpdateData)
{
	if (m_pSource != NULL)
	{
		m_pSource->Release();
		m_pSource = NULL;
		m_pDictInfo = NULL;
	}

	m_strDjVuFile.LoadString(IDS_BROWSE_DJVU_PROMPT);

	if (bUpdateData)
		UpdateData(false);
}

void CDictionaryDlg::InitDocProperties()
{
	m_strTitle.Empty();
	if (!m_pDictInfo->titleLoc.empty())
	{
		Normalize(m_pDictInfo->titleLoc);
		m_strTitle = MakeCString(m_pDictInfo->titleLoc[0].second);
	}

	m_pLangFrom = NULL;
	m_pLangTo = NULL;
	for (size_t nLang = 0; nLang < theApp.GetLanguageCount(); ++nLang)
	{
		const Language& lang = theApp.GetLanguage(nLang);
		if (lang.strCode == MakeCString(m_pDictInfo->strLangFromCode))
		{
			m_cboLangFrom.SetCurSel(nLang + 1);
			m_pLangFrom = &lang;

			m_lang[m_pLangFrom] = m_pDictInfo->langFromLoc;
			Normalize(m_lang[m_pLangFrom]);
			m_lang[m_pLangFrom][0].second = MakeUTF8String(lang.strName);
		}

		if (lang.strCode == MakeCString(m_pDictInfo->strLangToCode))
		{
			m_cboLangTo.SetCurSel(nLang + 1);
			m_pLangTo = &lang;

			m_lang[m_pLangTo] = m_pDictInfo->langToLoc;
			Normalize(m_lang[m_pLangTo]);
			m_lang[m_pLangTo][0].second = MakeUTF8String(lang.strName);
		}
	}

	if (m_pLangFrom == NULL)
		m_cboLangFrom.SetCurSel(0);
	if (m_pLangTo == NULL)
		m_cboLangTo.SetCurSel(0);
}

void CDictionaryDlg::OnLocalizeTitle()
{
	CLocalizedStringDlg dlg(m_pDictInfo->titleLoc);
	if (dlg.DoModal() == IDOK)
	{
		m_pDictInfo->titleLoc = dlg.m_loc;

		m_strTitle.Empty();
		if (!m_pDictInfo->titleLoc.empty())
		{
			Normalize(m_pDictInfo->titleLoc);
			m_strTitle = MakeCString(m_pDictInfo->titleLoc[0].second);
		}

		UpdateData(false);
	}
}

void CDictionaryDlg::Normalize(vector<DictionaryInfo::LocalizedString>& loc)
{
	set<DWORD> existing;
	bool bHasEnglish = false;
	size_t i = 0;
	while (i < loc.size())
	{
		if (existing.find(loc[i].first) != existing.end())
		{
			loc.erase(loc.begin() + i);
			continue;
		}

		existing.insert(loc[i].first);
		if (loc[i].first == theApp.GetEnglishLoc().nCode)
		{
			bHasEnglish = true;
			if (i != 0)
			{
				DictionaryInfo::LocalizedString str = loc[i];
				loc.erase(loc.begin() + i);
				loc.insert(loc.begin(), str);
			}
		}
		++i;
	}

	if (!bHasEnglish)
		loc.insert(loc.begin(), make_pair(theApp.GetEnglishLoc().nCode, GUTF8String("")));
}

void CDictionaryDlg::OnLocalizeLangFrom()
{
	if (m_pLangFrom == NULL)
		return;

	CLocalizedStringDlg dlg(m_lang[m_pLangFrom]);
	if (dlg.DoModal() == IDOK)
	{
		m_lang[m_pLangFrom] = dlg.m_loc;

		Normalize(m_lang[m_pLangFrom]);
		m_lang[m_pLangFrom][0].second = MakeUTF8String(m_pLangFrom->strName);
	}
}

void CDictionaryDlg::OnLocalizeLangTo()
{
	if (m_pLangTo == NULL)
		return;

	CLocalizedStringDlg dlg(m_lang[m_pLangTo]);
	if (dlg.DoModal() == IDOK)
	{
		m_lang[m_pLangTo] = dlg.m_loc;

		Normalize(m_lang[m_pLangTo]);
		m_lang[m_pLangTo][0].second = MakeUTF8String(m_pLangTo->strName);
	}
}

GUTF8String CDictionaryDlg::GetTitleXML()
{
	GUTF8String strResult;
	strResult += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	strResult += "<title>\n";
	strResult += GetLocalizedStringsXML(m_pDictInfo->titleLoc);
	strResult += "</title>\n";
	return strResult;
}

GUTF8String CDictionaryDlg::GetLangFromXML()
{
	if (m_pLangFrom == NULL)
		return "";

	GUTF8String strResult;
	strResult += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	strResult += "<language code=\"" + MakeUTF8String(m_pLangFrom->strCode).toEscaped() + "\">\n";
	strResult += GetLocalizedStringsXML(m_lang[m_pLangFrom]);
	strResult += "</language>\n";
	return strResult;
}

GUTF8String CDictionaryDlg::GetLangToXML()
{
	if (m_pLangTo == NULL)
		return "";

	GUTF8String strResult;
	strResult += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	strResult += "<language code=\"" + MakeUTF8String(m_pLangTo->strCode).toEscaped() + "\">\n";
	strResult += GetLocalizedStringsXML(m_lang[m_pLangTo]);
	strResult += "</language>\n";
	return strResult;
}

GUTF8String CDictionaryDlg::GetLocalizedStringsXML(vector<DictionaryInfo::LocalizedString>& loc)
{
	GUTF8String strResult;
	for (size_t i = 0; i < loc.size(); ++i)
	{
		strResult += "<string localization=\""
				+ MakeUTF8String(FormatString(_T("%04x"), loc[i].first)).toEscaped()
				+ "\" value=\"" + loc[i].second.toEscaped() + "\"/>\n";
	}
	return strResult;
}

GUTF8String ReadRawXML(LPCTSTR pszFileName)
{
	ifstream in(pszFileName, ios::in);
	if (!in)
		return "";

	istreambuf_iterator<char> begin(in), end;
	vector<char> raw_data;
	copy(begin, end, back_inserter(raw_data));
	in.close();

	raw_data.push_back('\0');
	GUTF8String result(&raw_data[0]);

	stringstream sin((const char*) result);
	XMLParser parser;
	if (!parser.Parse(sin))
		return "";

	return result;
}

void Append(string& dest, const char* src)
{
	size_t length = strlen(src);
	if (dest.size() + length < dest.capacity())
		dest.reserve(max(dest.size() * 2, dest.size() + length));

	dest.insert(dest.length(), src, length);
}

GUTF8String ReadExcelPageIndex(LPCTSTR pszFileName)
{
	Excel::_ApplicationPtr pApplication;

	if (FAILED(pApplication.CreateInstance(_T("Excel.Application"))))
		return "";

	string strResult;
	Append(strResult, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<index>\n");

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

		Excel::RangePtr pRange = pSheet->GetRange(_bstr_t("A1"), _bstr_t("D16384"));
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
				{
					strResult = "";
					break;
				}
				bFirst = false;
			}
			else
			{
				if (nNewLevel <= 0 || nNewLevel > nNewLevel + 1)
				{
					strResult = "";
					break;
				}

				if (nNewLevel == nLevel + 1)
					Append(strResult, ">\n");
				else
				{
					Append(strResult, "/>\n");
					for (int i = nLevel - 1; i >= nNewLevel; --i)
					{
						for (int k = 1; k < i; ++k)
							Append(strResult, "  ");
						Append(strResult, "</entry>\n");
					}
				}
			}
			nLevel = nNewLevel;

			for (int k = 1; k < nLevel; ++k)
				Append(strResult, "  ");
			Append(strResult, "<entry");

			if (bstrFirst.length() > 0)
				Append(strResult, " first=\"" + MakeUTF8String(wstring(bstrFirst)).toEscaped() + "\"");
			if (bstrLast.length() > 0)
				Append(strResult, " last=\"" + MakeUTF8String(wstring(bstrLast)).toEscaped() + "\"");
			if (bstrURL.length() > 0)
				Append(strResult, " url=\"" + MakeUTF8String(wstring(bstrURL)).toEscaped() + "\"");
		}

		if (bFirst)
			strResult = "";

		if (strResult.length() > 0)
		{
			Append(strResult, "/>\n");
			for (int i = 1; i < nLevel; ++i)
				Append(strResult, "</entry>\n");
			Append(strResult, "</index>\n");
		}

		pBook->Close(VARIANT_FALSE);
	}
	catch (_com_error&)
	{
		strResult = "";
	}

	try
	{
		pApplication->Quit();
	}
	catch (_com_error&)
	{
	}

	return strResult.c_str();
}

GUTF8String ReadExcelCharMap(LPCTSTR pszFileName)
{
	Excel::_ApplicationPtr pApplication;

	if (FAILED(pApplication.CreateInstance(_T("Excel.Application"))))
		return "";

	GUTF8String strResult;
	strResult += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	strResult += "<char-map>\n";

	try
	{
		int nCount = 0;
		bool bFirst = true;
		_variant_t varOption((long)DISP_E_PARAMNOTFOUND, VT_ERROR);

		Excel::_WorkbookPtr pBook = pApplication->Workbooks->Open(
				pszFileName, 0L, VARIANT_TRUE, varOption, varOption,
				varOption, varOption, varOption, varOption,
				varOption, varOption, varOption, varOption);
		Excel::_WorksheetPtr pSheet = pBook->Sheets->Item[1L];

		Excel::RangePtr pRange = pSheet->GetRange(_bstr_t("A1"), _bstr_t("B16384"));
		for (long nRow = 1; nRow < 16384; ++nRow, ++nCount)
		{
			_bstr_t bstrFrom(_variant_t(pRange->Item[nRow][1L]));
			_bstr_t bstrTo(_variant_t(pRange->Item[nRow][2L]));

			if (bstrFrom.length() == 0)
				break;

			strResult += "<entry from=\"" + MakeUTF8String(wstring(bstrFrom)).toEscaped() + "\"";
			strResult += " to=\"" + MakeUTF8String(wstring(bstrTo)).toEscaped() + "\"/>\n";
		}

		if (nCount == 0)
			strResult = "";

		if (strResult.length() > 0)
			strResult += "</char-map>\n";

		pBook->Close(VARIANT_FALSE);
	}
	catch (_com_error&)
	{
		strResult = "";
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

bool CDictionaryDlg::OpenPageIndex(const CString& strFileName)
{
	CWaitCursor wait;

	TCHAR szInputExt[_MAX_EXT];
	_tsplitpath(strFileName, NULL, NULL, NULL, szInputExt);

	GUTF8String strPageIndexXML;
	if (CString(szInputExt).CompareNoCase(_T(".xls")) == 0)
		strPageIndexXML = ReadExcelPageIndex(strFileName);
	else
		strPageIndexXML = ReadRawXML(strFileName);

	if (strPageIndexXML.length() == 0)
	{
		AfxMessageBox(IDS_CANNOT_OPEN_INDEX_FILE, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	m_bHasPageIndexFile = true;
	m_strPageIndexFile = strFileName;
	m_strPageIndexXML = strPageIndexXML;

	OnKickIdle();
	m_nPageIndexAction = 1;
	UpdateData(false);

	return true;
}

bool CDictionaryDlg::OpenCharMap(const CString& strFileName)
{
	CWaitCursor wait;

	GUTF8String strCharMapXML = ReadExcelCharMap(strFileName);
	if (strCharMapXML.length() == 0)
		strCharMapXML = ReadRawXML(strFileName);

	if (strCharMapXML.length() == 0)
	{
		AfxMessageBox(IDS_CANNOT_OPEN_MAP_FILE, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	m_bHasCharMapFile = true;
	m_strCharMapFile = strFileName;
	m_strCharMapXML = strCharMapXML;

	OnKickIdle();
	m_nCharMapAction = 1;
	UpdateData(false);

	return true;
}

void CDictionaryDlg::OnBrowsePageIndex()
{
	CString strFileName = (m_bHasPageIndexFile ? m_strPageIndexFile : _T(""));

	CMyFileDialog dlg(true, _T("xls"), strFileName,
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		LoadString(IDS_PAGE_INDEX_FILTER));

	CString strTitle;
	strTitle.LoadString(IDS_BROWSE_PAGE_INDEX);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

	UINT nResult = dlg.DoModal();
	if (nResult != IDOK)
		return;

	OpenPageIndex(dlg.GetPathName());
}

void CDictionaryDlg::OnBrowseCharMap()
{
	CString strFileName = (m_bHasCharMapFile ? m_strCharMapFile : _T(""));

	CMyFileDialog dlg(true, _T("xls"), strFileName,
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		LoadString(IDS_CHAR_MAP_FILTER));

	CString strTitle;
	strTitle.LoadString(IDS_BROWSE_CHAR_MAP);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);

	UINT nResult = dlg.DoModal();
	if (nResult != IDOK)
		return;

	OpenCharMap(dlg.GetPathName());
}

HBRUSH CDictionaryDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH brush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_DJVU_FILE && m_pSource == NULL)
	{
		pDC->SetTextColor(::GetSysColor(COLOR_3DSHADOW));
	}
	if (pWnd->GetDlgCtrlID() == IDC_PAGE_INDEX_FILE && !m_bHasPageIndexFile)
	{
		pDC->SetTextColor(::GetSysColor(COLOR_3DSHADOW));
	}
	if (pWnd->GetDlgCtrlID() == IDC_CHAR_MAP_FILE && !m_bHasCharMapFile)
	{
		pDC->SetTextColor(::GetSysColor(COLOR_3DSHADOW));
	}

	return brush;
}

void CDictionaryDlg::OnKickIdle()
{
	GetDlgItem(IDC_EXPORT_PAGE_INDEX)->EnableWindow(m_pSource != NULL
			&& m_pDictInfo->strPageIndex.length() > 0);
	GetDlgItem(IDC_PAGE_INDEX_REMOVE)->EnableWindow(m_pSource != NULL
			&& m_pDictInfo->strPageIndex.length() > 0);
	GetDlgItem(IDC_EXPORT_CHAR_MAP)->EnableWindow(m_pSource != NULL
			&& m_pDictInfo->strCharMap.length() > 0);
	GetDlgItem(IDC_CHAR_MAP_REMOVE)->EnableWindow(m_pSource != NULL
			&& m_pDictInfo->strCharMap.length() > 0);

	GetDlgItem(IDC_SAVE)->EnableWindow(m_pSource != NULL);
	GetDlgItem(IDC_SAVE_AS)->EnableWindow(m_pSource != NULL);
	GetDlgItem(IDC_PAGE_INDEX_DONTCHANGE)->EnableWindow(m_pSource != NULL);
	GetDlgItem(IDC_CHAR_MAP_DONTCHANGE)->EnableWindow(m_pSource != NULL);
	GetDlgItem(IDC_PAGE_INDEX_REPLACE)->EnableWindow(m_bHasPageIndexFile);
	GetDlgItem(IDC_CHAR_MAP_REPLACE)->EnableWindow(m_bHasCharMapFile);

	m_cboLangFrom.EnableWindow(m_pSource != NULL);
	m_cboLangTo.EnableWindow(m_pSource != NULL);

	GetDlgItem(IDC_TITLE_LOCALIZED)->EnableWindow(m_pSource != NULL);
	GetDlgItem(IDC_FROM_LOCALIZED)->EnableWindow(m_pSource != NULL && m_pLangFrom != NULL);
	GetDlgItem(IDC_TO_LOCALIZED)->EnableWindow(m_pSource != NULL && m_pLangTo != NULL);

	GetDlgItem(IDC_WARNINGS)->EnableWindow(!m_warnings.empty());
}

void CDictionaryDlg::OnSelChangeFrom()
{
	int nFrom = m_cboLangFrom.GetCurSel();
	if (nFrom > 0)
	{
		m_pLangFrom = &(theApp.GetLanguage(nFrom - 1));
		vector<DictionaryInfo::LocalizedString>& loc = m_lang[m_pLangFrom];
		if (loc.empty())
			loc.push_back(make_pair(theApp.GetEnglishLoc().nCode, MakeUTF8String(m_pLangFrom->strName)));
	}
	else
		m_pLangFrom = NULL;
}

void CDictionaryDlg::OnSelChangeTo()
{
	int nTo = m_cboLangTo.GetCurSel();
	if (nTo > 0)
	{
		m_pLangTo = &(theApp.GetLanguage(nTo - 1));
		vector<DictionaryInfo::LocalizedString>& loc = m_lang[m_pLangTo];
		if (loc.empty())
			loc.push_back(make_pair(theApp.GetEnglishLoc().nCode, MakeUTF8String(m_pLangTo->strName)));
	}
	else
		m_pLangTo = NULL;
}

bool CDictionaryDlg::ExportPageIndex(const CString& strPageIndexFile)
{
	ofstream out(strPageIndexFile, ios::out);
	if (!out)
	{
		AfxMessageBox(IDS_CANNOT_WRITE);
		return false;
	}

	out << (const char*) m_pDictInfo->strPageIndex;
	out.close();

	AfxMessageBox(IDS_EXPORT_DONE, MB_OK | MB_ICONINFORMATION);
	return true;
}

bool CDictionaryDlg::ExportCharMap(const CString& strCharMapFile)
{
	ofstream out(strCharMapFile, ios::out);
	if (!out)
	{
		AfxMessageBox(IDS_CANNOT_WRITE);
		return false;
	}

	out << (const char*)m_pDictInfo->strCharMap;
	out.close();

	AfxMessageBox(IDS_EXPORT_DONE, MB_OK | MB_ICONINFORMATION);
	return true;
}

void CDictionaryDlg::OnDestroy()
{
	CloseDocument(false);
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

void CDictionaryDlg::OnSave()
{
	UpdateData();

	SaveDocument(m_strDjVuFile);
}

void CDictionaryDlg::OnSaveAs()
{
	UpdateData();

	SaveDocument();
}

void CDictionaryDlg::SaveDocument(const CString& strFileName)
{
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
		TestIndex();

		GP<DjVuFile> pFile = m_pSource->GetDjVuDoc()->get_djvu_file(0);

		GMap<GUTF8String, GUTF8String> meta;

		GP<ByteStream> pAnnoStream = pFile->get_anno();
		if (pAnnoStream && pAnnoStream->size())
		{
			pAnnoStream->seek(0);
			GP<DjVuAnno> pDjVuAnno = DjVuAnno::create();
			pDjVuAnno->decode(pAnnoStream);
			meta = pDjVuAnno->ant->metadata;
		}

		if (m_nPageIndexAction == 1 && m_bHasPageIndexFile)
		{
			string strEncodedIndex(m_strPageIndexXML);
			Base64Encode(strEncodedIndex);
			meta[pszPageIndexKey] = strEncodedIndex.c_str();
		}
		else if (m_nPageIndexAction == 2)
		{
			meta.del(pszPageIndexKey);
		}

		if (m_nCharMapAction == 1 && m_bHasCharMapFile)
		{
			string strEncodedChars(m_strCharMapXML);
			Base64Encode(strEncodedChars);
			meta[pszCharMapKey] = strEncodedChars.c_str();
		}
		else if (m_nCharMapAction == 2)
		{
			meta.del(pszCharMapKey);
		}

		GUTF8String strTitle = GetTitleXML();
		if (strTitle.length() > 0)
		{
			string strEncoded(strTitle);
			Base64Encode(strEncoded);
			meta[pszTitleKey] = strEncoded.c_str();
		}
		else
			meta.del(pszTitleKey);

		GUTF8String strLangFrom = GetLangFromXML();
		if (strLangFrom.length() > 0)
		{
			string strEncoded(strLangFrom);
			Base64Encode(strEncoded);
			meta[pszLangFromKey] = strEncoded.c_str();
		}
		else
			meta.del(pszLangFromKey);

		GUTF8String strLangTo = GetLangToXML();
		if (strLangTo.length() > 0)
		{
			string strEncoded(strLangTo);
			Base64Encode(strEncoded);
			meta[pszLangToKey] = strEncoded.c_str();
		}
		else
			meta.del(pszLangToKey);

		ModifyMeta(pFile, &meta);

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
	if (m_warnings.empty())
		AfxMessageBox(IDS_EMBEDDING_DONE, MB_OK | MB_ICONINFORMATION);
	else
		AfxMessageBox(IDS_EMBEDDING_DONE_WARNINGS, MB_OK | MB_ICONINFORMATION);
}

inline wstring& tolower(wstring& str)
{
	for (wstring::iterator it = str.begin(); it != str.end(); ++it)
		*it = towlower(*it);
	return str;
}

wstring& CDictionaryDlg::MapCharacters(wstring& str)
{
	if (m_charMap.empty())
		return str;

	wstring result;
	const wchar_t* cur = str.c_str();
	const wchar_t* last = cur + str.length();

	while (*cur != '\0')
	{
		int l = 0, r = static_cast<int>(m_charMap.size());
		if (m_charMap[l].first > cur)
		{
			result += *cur++;
			continue;
		}

		while (r - l > 1)
		{
			int mid = (l + r) / 2;
			if (m_charMap[mid].first <= cur)
				l = mid;
			else
				r = mid;
		}

		bool bFound = false;
		while (l >= 0 && m_charMap[l].first[0] == *cur)
		{
			size_t len = m_charMap[l].first.length();
			if (last - cur >= static_cast<int>(len)
					&& wcsncmp(m_charMap[l].first.c_str(), cur, len) == 0)
			{
				cur += len;
				result += m_charMap[l].second;
				bFound = true;
				break;
			}
			--l;
		}

		if (!bFound)
			result += *cur++;
	}

	str.swap(result);
	return str;
}

void CDictionaryDlg::TestIndex()
{
	prevFirst.erase();
	prevLast.erase();
	m_warnings.clear();
	m_charMap.clear();

	GUTF8String strPageIndexXML;
	if (m_bHasPageIndexFile)
		strPageIndexXML = m_strPageIndexXML;
	else
		strPageIndexXML = m_pSource->GetDictionaryInfo()->strPageIndex;

	if (strPageIndexXML.length() == 0)
		return;

	GUTF8String strCharMapXML;
	if (m_bHasCharMapFile)
		strCharMapXML = m_strCharMapXML;
	else
		strCharMapXML = m_pSource->GetDictionaryInfo()->strCharMap;

	if (strCharMapXML.length() != 0)
	{
		stringstream sin((const char*) strCharMapXML);
		XMLParser parser;
		if (parser.Parse(sin))
		{
			const XMLNode& root = *parser.GetRoot();
			list<XMLNode>::const_iterator it;
			for (it = root.childElements.begin(); it != root.childElements.end(); ++it)
			{
				const XMLNode& node = *it;
				if (MakeCString(node.tagName) != _T("entry"))
					continue;

				wstring strFrom, strTo;
				node.GetAttribute(_T("from"), strFrom);
				node.GetAttribute(_T("to"), strTo);

				if (strFrom.length() == 0)
					continue;

				strFrom = tolower(strFrom);
				strTo = tolower(strTo);
				m_charMap.push_back(make_pair(strFrom, strTo));
			}

			sort(m_charMap.begin(), m_charMap.end());
		}
	}

	stringstream sin((const char*) strPageIndexXML);
	XMLParser parser;
	if (parser.Parse(sin))
		TestEntries(*parser.GetRoot(), 0);
}

int CDictionaryDlg::TestEntries(const XMLNode& parent, int nEntries)
{
	list<XMLNode>::const_iterator it;
	int nCount = nEntries;
	for (it = parent.childElements.begin(); it != parent.childElements.end(); ++it)
	{
		const XMLNode& node = *it;
		if (MakeCString(node.tagName) != _T("entry"))
			continue;

		wstring strFirst, strLast;
		node.GetAttribute(_T("first"), strFirst);
		node.GetAttribute(_T("last"), strLast);
		++nCount;

		strFirst = MapCharacters(tolower(strFirst));
		strLast = MapCharacters(tolower(strLast));

		if (!strFirst.empty())
		{
			if (!strLast.empty() && strFirst > strLast)
				m_warnings.push_back(FormatString(_T("Cell C%d is lexicographically smaller than B%d.\r\n"), nCount, nCount));

			if (!prevFirst.empty() && strFirst < prevFirst)
				m_warnings.push_back(FormatString(_T("Cell B%d is lexicographically smaller than B%d.\r\n"), nCount, nCount - 1));

			if (!prevLast.empty() && strFirst < prevLast)
				m_warnings.push_back(FormatString(_T("Cell B%d is lexicographically smaller than C%d.\r\n"), nCount, nCount - 1));
		}
		else if (!strLast.empty())
		{
			if (!prevFirst.empty() && strLast < prevFirst)
				m_warnings.push_back(FormatString(_T("Cell C%d is lexicographically smaller than B%d.\r\n"), nCount, nCount - 1));

			if (!prevLast.empty() && strLast < prevLast)
				m_warnings.push_back(FormatString(_T("Cell C%d is lexicographically smaller than C%d.\r\n"), nCount, nCount - 1));
		}
		else
		{
			m_warnings.push_back(FormatString(_T("Cell B%d and C%d are both empty.\r\n"), nCount, nCount));
		}

		prevFirst = strFirst;
		prevLast = strLast;

		nCount = TestEntries(node, nCount);
	}

	return nCount;
}

void CDictionaryDlg::OnWarnings()
{
	CWarningsDlg dlg;
	dlg.m_strWarnings.Empty();

	for (size_t i = 0; i < m_warnings.size(); ++i)
		dlg.m_strWarnings += m_warnings[i];

	dlg.DoModal();
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

	CRect rcPageIndex;
	GetDlgItem(IDC_PAGE_INDEX_GROUP)->GetWindowRect(rcPageIndex);
	ScreenToClient(rcPageIndex);

	CRect rcCharMap;
	GetDlgItem(IDC_CHAR_MAP_GROUP)->GetWindowRect(rcCharMap);
	ScreenToClient(rcCharMap);

	if (rcDjVu.PtInRect(point) || rcPageIndex.PtInRect(point) || rcCharMap.PtInRect(point))
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

	CRect rcPageIndex;
	GetDlgItem(IDC_PAGE_INDEX_GROUP)->GetWindowRect(rcPageIndex);
	ScreenToClient(rcPageIndex);

	CRect rcCharMap;
	GetDlgItem(IDC_CHAR_MAP_GROUP)->GetWindowRect(rcCharMap);
	ScreenToClient(rcCharMap);

	if (!rcDjVu.PtInRect(point) && !rcPageIndex.PtInRect(point) && !rcCharMap.PtInRect(point))
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
		if (OpenDocument(strPath))
			m_warnings.clear();
	}
	else if (rcPageIndex.PtInRect(point))
	{
		OpenPageIndex(strPath);
	}
	else if (rcCharMap.PtInRect(point))
	{
		OpenCharMap(strPath);
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
