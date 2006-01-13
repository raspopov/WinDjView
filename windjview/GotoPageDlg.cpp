//	WinDjView
//	Copyright (C) 2004-2006 Andrew Zhezherun
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
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//	http://www.gnu.org/copyleft/gpl.html

// $Id $

#include "stdafx.h"
#include "WinDjView.h"
#include "GotoPageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGotoPageDlg dialog

IMPLEMENT_DYNAMIC(CGotoPageDlg, CDialog)
CGotoPageDlg::CGotoPageDlg(int nPage, int nPageCount, CWnd* pParent)
	: CDialog(CGotoPageDlg::IDD, pParent), m_nPage(nPage + 1), m_nPageCount(nPageCount)
{
	m_edtPage.SetInteger();
}

CGotoPageDlg::~CGotoPageDlg()
{
}

void CGotoPageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PAGE, m_edtPage);
	DDX_Text(pDX, IDC_PAGE, m_nPage);

	if (!pDX->m_bSaveAndValidate)
	{
		CString strPageCount;
		strPageCount.Format(IDS_OF_PAGE_COUNT, m_nPageCount);
		DDX_Text(pDX, IDC_PAGE_COUNT, strPageCount);
	}
}


BEGIN_MESSAGE_MAP(CGotoPageDlg, CDialog)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_PAGE, OnPageUpDown)
	ON_EN_KILLFOCUS(IDC_PAGE, OnUpdateDialogData)
END_MESSAGE_MAP()


// CGotoPageDlg message handlers

void CGotoPageDlg::OnPageUpDown(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData();

	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	if (pNMUpDown->iDelta < 0)
		++m_nPage;
	else
		--m_nPage;

	if (m_nPage < 1)
		m_nPage = 1;
	else if (m_nPage > m_nPageCount)
		m_nPage = m_nPageCount;

	UpdateData(false);

	*pResult = 0;
}

void CGotoPageDlg::OnUpdateDialogData()
{
	UpdateData();

	if (m_nPage < 1)
		m_nPage = 1;
	else if (m_nPage > m_nPageCount)
		m_nPage = m_nPageCount;

	UpdateData(false);
}

void CGotoPageDlg::OnOK()
{
	OnUpdateDialogData();

	CDialog::OnOK();
}
