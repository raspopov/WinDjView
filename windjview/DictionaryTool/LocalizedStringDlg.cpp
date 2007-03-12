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
#include "LocalizedStringDlg.h"
#include "AddStringDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int c_nLocColWidth = 100;


// CLocalizedStringDlg dialog

IMPLEMENT_DYNAMIC(CLocalizedStringDlg, CDialog)
CLocalizedStringDlg::CLocalizedStringDlg(const vector<DictionaryInfo::LocalizedString>& loc, CWnd* pParent)
	: CDialog(CLocalizedStringDlg::IDD, pParent), m_loc(loc)
{
}

CLocalizedStringDlg::~CLocalizedStringDlg()
{
}

void CLocalizedStringDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CLocalizedStringDlg, CDialog)
	ON_BN_CLICKED(ID_ADD, OnAdd)
	ON_BN_CLICKED(ID_REMOVE, OnRemove)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LIST, OnBeginEditLabel)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST, OnEndEditLabel)
	ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()


// CLocalizedStringDlg message handlers

BOOL CLocalizedStringDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Image list is used to change row height
	m_imgList.Create(1, 16, ILC_COLOR24 | ILC_MASK, 0, 1);
	m_list.SetImageList(&m_imgList, LVSIL_SMALL);

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	CRect rcClient;
	m_list.GetClientRect(rcClient);

	m_list.InsertColumn(0, _T("Value"), LVCFMT_LEFT, rcClient.Width() - c_nLocColWidth);
	m_list.InsertColumn(1, _T("Localization"), LVCFMT_LEFT, c_nLocColWidth, 1);

	for (size_t i = 0; i < m_loc.size(); ++i)
	{
		const Localization* pLocalization = NULL;
		for (size_t nLoc = 0; nLoc < theApp.GetLocalizationCount(); ++nLoc)
		{
			const Localization& cur = theApp.GetLocalization(nLoc);
			if (cur.nCode == m_loc[i].first)
			{
				pLocalization = &cur;
				break;
			}
		}

		m_list.InsertItem(i, MakeCString(m_loc[i].second));
		if (pLocalization != NULL)
			m_list.SetItemText(i, 1, pLocalization->strName);
		else
			m_list.SetItemText(i, 1, _T("Unknown"));
	}

	return true;
}

void CLocalizedStringDlg::OnAdd()
{
	const Localization* pLocalization = NULL;
	if (m_loc.empty())
	{
		pLocalization = &theApp.GetEnglishLoc();
	}
	else
	{
		CAddStringDlg dlg(m_loc);
		if (dlg.DoModal() != IDOK)
			return;

		pLocalization = &(theApp.GetLocalization(dlg.m_nLocalization));
	}

	m_loc.push_back(make_pair(pLocalization->nCode, GUTF8String("")));

	int nIndex = static_cast<int>(m_loc.size()) - 1;
	m_list.InsertItem(nIndex, _T(""));
	m_list.SetItemText(nIndex, 1, pLocalization->strName);

	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		m_list.SetItemState(nIndex, 0, LVIS_SELECTED);
	}

	m_list.SetItemState(nIndex, LVIS_SELECTED, LVIS_SELECTED);
	m_list.SetSelectionMark(nIndex);

	m_list.SetFocus();
	m_list.EditLabel(nIndex);
}

void CLocalizedStringDlg::OnRemove()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		m_list.DeleteItem(nItem);
		m_loc.erase(m_loc.begin() + nItem);
	}
}

void CLocalizedStringDlg::OnOK()
{
	if (!UpdateData())
		return;

	for (size_t i = 0; i < m_loc.size(); ++i)
		m_loc[i].second = MakeUTF8String(m_list.GetItemText(i, 0));

	CDialog::OnOK();
}

void CLocalizedStringDlg::OnBeginEditLabel(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Always allow
	*pResult = 0;
}

void CLocalizedStringDlg::OnEndEditLabel(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Always accept
	*pResult = 1;
}

void CLocalizedStringDlg::OnKickIdle()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	GetDlgItem(ID_REMOVE)->EnableWindow(pos != NULL);
}
