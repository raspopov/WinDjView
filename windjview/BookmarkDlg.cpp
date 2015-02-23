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
#include "BookmarkDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBookmarkDlg dialog

IMPLEMENT_DYNAMIC(CBookmarkDlg, CMyDialog)

CBookmarkDlg::CBookmarkDlg(UINT nTitle, CWnd* pParent)
	: CMyDialog(CBookmarkDlg::IDD, pParent), m_nTitle(nTitle)
{
}

CBookmarkDlg::~CBookmarkDlg()
{
}

void CBookmarkDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_BOOKMARK_TITLE, m_strTitle);
}


BEGIN_MESSAGE_MAP(CBookmarkDlg, CMyDialog)
END_MESSAGE_MAP()


// CBookmarkDlg message handlers

BOOL CBookmarkDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	
	SetWindowText(LoadString(m_nTitle));

	return true;
}
