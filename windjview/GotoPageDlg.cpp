//	WinDjView
//	Copyright (C) 2004-2005 Andrew Zhezherun
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
	DDV_MinMaxInt(pDX, m_nPage, 1, m_nPageCount);

	if (!pDX->m_bSaveAndValidate)
	{
		CString strPageCount;
		strPageCount.Format(_T("of %d"), m_nPageCount);
		DDX_Text(pDX, IDC_PAGE_COUNT, strPageCount);
	}
}


BEGIN_MESSAGE_MAP(CGotoPageDlg, CDialog)
END_MESSAGE_MAP()


// CGotoPageDlg message handlers
