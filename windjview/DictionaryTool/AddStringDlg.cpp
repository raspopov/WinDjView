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
#include "AddStringDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAddStringDlg dialog

IMPLEMENT_DYNAMIC(CAddStringDlg, CDialog)
CAddStringDlg::CAddStringDlg(const vector<DictionaryInfo::LocalizedString>& loc, CWnd* pParent)
	: CDialog(CAddStringDlg::IDD, pParent)
{
	for (size_t i = 0; i < loc.size(); ++i)
		m_excluded.insert(loc[i].first);
}

CAddStringDlg::~CAddStringDlg()
{
}

void CAddStringDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOCALIZATION, m_loc);
}


BEGIN_MESSAGE_MAP(CAddStringDlg, CDialog)
END_MESSAGE_MAP()


// CAddStringDlg message handlers

BOOL CAddStringDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	for (size_t i = 0; i < theApp.GetLocalizationCount(); ++i)
	{
		const Localization& loc = theApp.GetLocalization(i);
		if (m_excluded.find(loc.nCode) == m_excluded.end())
		{
			int nString = m_loc.AddString(loc.strName);
			m_loc.SetItemData(nString, (DWORD) i);
		}
	}

	if (m_loc.GetCount() == 0)
	{
		GetDlgItem(IDOK)->EnableWindow(false);
	}
	else
	{
		m_loc.SetCurSel(0);
	}

	return true;
}

void CAddStringDlg::OnOK()
{
	if (m_loc.GetCurSel() == CB_ERR)
		return;

	m_nLocalization = (int) m_loc.GetItemData(m_loc.GetCurSel());

	CDialog::OnOK();
}