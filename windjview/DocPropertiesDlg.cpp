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
#include "DocPropertiesDlg.h"
#include "DjVuSource.h"

#include "libdjvu/DjVmDir.h"
#include "libdjvu/DjVmDir0.h"
#include "libdjvu/DjVuNavDir.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDocPropertiesDlg dialog

IMPLEMENT_DYNAMIC(CDocPropertiesDlg, CMyDialog)

CDocPropertiesDlg::CDocPropertiesDlg(DjVuSource* pSource, CWnd* pParent)
	: CMyDialog(CDocPropertiesDlg::IDD, pParent), m_nSortBy(-1), m_bSortAsc(true)
{
	m_bShowAllFiles = false;
	m_bHideShowAll = false;

	if (::AfxGetFileTitle(pSource->GetFileName(), m_strDocName.GetBuffer(_MAX_FNAME), _MAX_FNAME) == 0)
		m_strDocName.ReleaseBuffer();
	else
		m_strDocName = pSource->GetFileName();

	m_nDocSize = -1;
	CFile file;
	if (file.Open(pSource->GetFileName(), CFile::modeRead | CFile::shareDenyWrite))
	{
		m_nDocSize = static_cast<int>(file.GetLength());
		file.Close();
	}

	CString strDocTypes = LoadString(IDS_DOC_TYPES);
	CString strFileTypes = LoadString(IDS_FILE_TYPES);

	int nType = pSource->GetDjVuDoc()->get_doc_type();
	int nSubstring = 5;  // Unknown
	if (nType == DjVuDocument::OLD_BUNDLED)
		nSubstring = 0;
	else if (nType == DjVuDocument::OLD_INDEXED)
		nSubstring = 1;
	else if (nType == DjVuDocument::BUNDLED)
		nSubstring = 2;
	else if (nType == DjVuDocument::INDIRECT)
		nSubstring = 3;
	else if (nType == DjVuDocument::SINGLE_PAGE)
		nSubstring = 4;

	AfxExtractSubString(m_strDocType, strDocTypes, nSubstring);

	m_nPages = pSource->GetPageCount();
	m_nFiles = m_nPages;

	GP<DjVmDir> pDir = NULL;
	if (nType == DjVuDocument::BUNDLED || nType == DjVuDocument::INDIRECT)
		pDir = pSource->GetDjVuDoc()->get_djvm_dir();
	GP<DjVmDir0> pDir0 = NULL;
	if (nType == DjVuDocument::OLD_BUNDLED)
		pDir0 = pSource->GetDjVuDoc()->get_djvm_dir0();
	GP<DjVuNavDir> pNavDir = pSource->GetDjVuDoc()->get_nav_dir();

	if (pDir != NULL)
	{
		m_nFiles = pDir->get_files_num();
		m_files.resize(m_nFiles);
		for (int i = 0; i < m_nFiles; ++i)
		{
			m_files[i].nIndex = i;
			GP<DjVmDir::File> pFile = pDir->pos_to_file(i, &m_files[i].nPageIndex);
			int nSubstring = 4;  // Unknown
			if (pFile->is_include())
				nSubstring = 0;
			else if (pFile->is_page())
				nSubstring = 1;
			else if (pFile->is_thumbnails())
				nSubstring = 2;
			else if (pFile->is_shared_anno())
				nSubstring = 3;

			AfxExtractSubString(m_files[i].strType, strFileTypes, nSubstring);

			if (!pFile->is_page())
				m_files[i].nPageIndex = -1;

			m_files[i].nSize = pFile->size;
			m_files[i].strName = MakeCString(pFile->get_load_name());
		}
	}
	else if (pDir0 != NULL)
	{
		m_nFiles = pDir0->get_files_num();
		m_files.resize(m_nFiles);
		for (int i = 0; i < m_nFiles; ++i)
		{
			GP<DjVmDir0::FileRec> pRec = pDir0->get_file(i);
			m_files[i].nIndex = i;
			m_files[i].nSize = pRec->size;
			m_files[i].strName = MakeCString(pRec->name);

			if (pNavDir != NULL)
			{
				m_files[i].nPageIndex = pNavDir->name_to_page(pRec->name);
				int nSubstring = (m_files[i].nPageIndex >= 0 ? 1 : 0);
				AfxExtractSubString(m_files[i].strType, strFileTypes, nSubstring);
			}
			else
			{
				m_files[i].nPageIndex = i;
				AfxExtractSubString(m_files[i].strType, strFileTypes, 4);  // Unknown
			}
		}
	}
	else
	{
		m_bHideShowAll = true;

		m_files.resize(m_nPages);
		for (int i = 0; i < m_nPages; ++i)
		{
			m_files[i].nIndex = i;
			m_files[i].nPageIndex = i;
			m_files[i].nSize = -1;
			AfxExtractSubString(m_files[i].strType, strFileTypes, 1);  // Page

			if (pNavDir != NULL)
				m_files[i].strName = MakeCString(pNavDir->page_to_name(i));

			GP<DjVuFile> pFile = pSource->GetDjVuDoc()->get_djvu_file(i, true);
			GP<DataPool> pPool = (pFile != NULL ? pFile->get_init_data_pool() : NULL);
			if (pPool != NULL)
				m_files[i].nSize = pPool->get_length();
		}
	}
}

CDocPropertiesDlg::~CDocPropertiesDlg()
{
}

void CDocPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_NAME, m_strDocName);
	DDX_Text(pDX, IDC_STATIC_TYPE, m_strDocType);
	DDX_Text(pDX, IDC_STATIC_SIZE, m_nDocSize);
	DDX_Text(pDX, IDC_STATIC_PAGES, m_nPages);
	DDX_Text(pDX, IDC_STATIC_FILES, m_nFiles);
	DDX_Check(pDX, IDC_SHOW_ALL_FILES, m_bShowAllFiles);
	DDX_Control(pDX, IDC_LIST_FILES, m_listFiles);
}


BEGIN_MESSAGE_MAP(CDocPropertiesDlg, CMyDialog)
	ON_BN_CLICKED(IDC_SHOW_ALL_FILES, OnShowAllFiles)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_FILES, OnSortFiles)
END_MESSAGE_MAP()


// CDocPropertiesDlg message handlers

BOOL CDocPropertiesDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	
	if (m_bHideShowAll)
		GetDlgItem(IDC_SHOW_ALL_FILES)->ShowWindow(SW_HIDE);

	m_listFiles.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
	PopulateFiles();

	return true;
}

void CDocPropertiesDlg::OnShowAllFiles()
{
	if (!UpdateData())
		return;

	PopulateFiles();
}

void CDocPropertiesDlg::PopulateFiles()
{
	m_listFiles.SetRedraw(false);
	m_listFiles.GetHeaderCtrl()->SetRedraw(false);

	m_listFiles.DeleteAllItems();
	while (m_listFiles.GetHeaderCtrl()->GetItemCount() > 0)
		m_listFiles.DeleteColumn(m_listFiles.GetHeaderCtrl()->GetItemCount() - 1);

	CString strColumnNames = LoadString(IDS_DOC_FILES_COLUMNS);
	CString strColumns[5];
	for (int i = 0; i < 5; ++i)
		AfxExtractSubString(strColumns[i], strColumnNames, i);

	HDITEM hdi;
	hdi.mask = HDI_LPARAM;

	if (m_bShowAllFiles)
	{
		m_listFiles.InsertColumn(0, strColumns[1]);  // File
		m_listFiles.SetColumnWidth(0, 80);
		hdi.lParam = 1;
		m_listFiles.GetHeaderCtrl()->SetItem(0, &hdi);

		m_listFiles.InsertColumn(1, strColumns[2]);  // Name
		m_listFiles.SetColumnWidth(1, 185);
		hdi.lParam = 2;
		m_listFiles.GetHeaderCtrl()->SetItem(1, &hdi);

		m_listFiles.InsertColumn(2, strColumns[3]);  // Type
		m_listFiles.SetColumnWidth(2, 110);
		hdi.lParam = 3;
		m_listFiles.GetHeaderCtrl()->SetItem(2, &hdi);

		m_listFiles.InsertColumn(3, strColumns[4]);  // Size
		m_listFiles.SetColumnWidth(3, 90);
		hdi.lParam = 4;
		m_listFiles.GetHeaderCtrl()->SetItem(3, &hdi);

		LVCOLUMN lvc;
		lvc.mask = LVCF_FMT;
		m_listFiles.GetColumn(3, &lvc);
		lvc.fmt &= ~(LVCFMT_LEFT | LVCFMT_CENTER | LVCFMT_RIGHT);
		lvc.fmt |= LVCFMT_RIGHT;
		m_listFiles.SetColumn(3, &lvc);

		for (int i = 0; i < (int)m_files.size(); ++i)
		{
			int nIndex = m_listFiles.InsertItem(i, FormatString(_T("%d"), i + 1));
			m_listFiles.SetItemText(nIndex, 1, m_files[i].strName);
			m_listFiles.SetItemText(nIndex, 2, m_files[i].strType);
			m_listFiles.SetItemText(nIndex, 3, FormatString(_T("%d"), m_files[i].nSize));
			m_listFiles.SetItemData(nIndex, (DWORD) &m_files[i]);
		}

		if (m_nSortBy == -1 || m_nSortBy == 0)
			m_nSortBy = 1;
	}
	else
	{
		m_listFiles.InsertColumn(0, strColumns[0]);  // Page
		m_listFiles.SetColumnWidth(0, 80);
		hdi.lParam = 0;
		m_listFiles.GetHeaderCtrl()->SetItem(0, &hdi);

		m_listFiles.InsertColumn(1, strColumns[2]);  // Name
		m_listFiles.SetColumnWidth(1, 295);
		hdi.lParam = 2;
		m_listFiles.GetHeaderCtrl()->SetItem(1, &hdi);

		m_listFiles.InsertColumn(2, strColumns[4]);  // Size
		m_listFiles.SetColumnWidth(2, 90);
		hdi.lParam = 4;
		m_listFiles.GetHeaderCtrl()->SetItem(2, &hdi);

		LVCOLUMN lvc;
		lvc.mask = LVCF_FMT;
		m_listFiles.GetColumn(2, &lvc);
		lvc.fmt &= ~(LVCFMT_LEFT | LVCFMT_CENTER | LVCFMT_RIGHT);
		lvc.fmt |= LVCFMT_RIGHT;
		m_listFiles.SetColumn(2, &lvc);

		for (size_t i = 0; i < m_files.size(); ++i)
		{
			if (m_files[i].nPageIndex == -1)
				continue;

			int nIndex = m_listFiles.InsertItem(m_files[i].nPageIndex, FormatString(_T("%d"), m_files[i].nPageIndex + 1));
			m_listFiles.SetItemText(nIndex, 1, m_files[i].strName);
			m_listFiles.SetItemText(nIndex, 2, FormatString(_T("%d"), m_files[i].nSize));
			m_listFiles.SetItemData(nIndex, (DWORD) &m_files[i]);
		}

		if (m_nSortBy == -1 || m_nSortBy == 1 || m_nSortBy == 3)
			m_nSortBy = 0;
	}

	SortFiles();

	m_listFiles.GetHeaderCtrl()->SetRedraw(true);
	m_listFiles.SetRedraw(true);
	m_listFiles.GetHeaderCtrl()->Invalidate();
	m_listFiles.Invalidate();
	m_listFiles.UpdateWindow();
}

void CDocPropertiesDlg::OnSortFiles(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMListView = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	int nColumn = pNMListView->iSubItem;
	HDITEM hdi;
	hdi.mask = HDI_LPARAM;
	m_listFiles.GetHeaderCtrl()->GetItem(nColumn, &hdi);
	int nSortBy = (int)hdi.lParam;

	if (nSortBy == m_nSortBy)
	{
		m_bSortAsc = !m_bSortAsc;
	}
	else
	{
		m_nSortBy = nSortBy;
		m_bSortAsc = true;
	}

	SortFiles();

	*pResult = 0;
}

void CDocPropertiesDlg::SortFiles()
{
	for (int i = 0; i < m_listFiles.GetHeaderCtrl()->GetItemCount(); ++i)
	{
		HDITEM hdi;
		hdi.mask = HDI_LPARAM | HDI_FORMAT;
		m_listFiles.GetHeaderCtrl()->GetItem(i, &hdi);

		hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
		if (hdi.lParam == m_nSortBy)
			hdi.fmt |= m_bSortAsc ? HDF_SORTUP : HDF_SORTDOWN;
		m_listFiles.GetHeaderCtrl()->SetItem(i, &hdi);
	}

	int nSortBy = m_nSortBy;
	if (!m_bSortAsc)
		nSortBy = -nSortBy - 1;
	m_listFiles.SortItems(&CompareFiles, nSortBy);
}

int CALLBACK CDocPropertiesDlg::CompareFiles(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	FileInfo* lhs = reinterpret_cast<FileInfo*>(lParam1);
	FileInfo* rhs = reinterpret_cast<FileInfo*>(lParam2);

	int nMult = (lParamSort >= 0 ? 1 : -1);
	if (nMult == -1)
		lParamSort = -lParamSort - 1;

	switch (lParamSort)
	{
	case 1:
		return nMult*(lhs->nIndex - rhs->nIndex);

	case 2:
		return nMult*_tcscmp(lhs->strName, rhs->strName);

	case 3:
		return nMult*_tcscmp(lhs->strType, rhs->strType);

	case 4:
		return nMult*(lhs->nSize - rhs->nSize);

	case 0:
	default:
		return nMult*(lhs->nPageIndex - rhs->nPageIndex);
	}
}
