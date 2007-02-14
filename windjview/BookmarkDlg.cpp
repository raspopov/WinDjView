//	WinDjView
//	Copyright (C) 2004-2007 Andrew Zhezherun
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
#include "WinDjView.h"
#include "BookmarkDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBookmarkDlg dialog

IMPLEMENT_DYNAMIC(CBookmarkDlg, CDialog)

CBookmarkDlg::CBookmarkDlg(UINT nTitle, CWnd* pParent)
	: CDialog(CBookmarkDlg::IDD, pParent), m_nTitle(nTitle)
{
}

CBookmarkDlg::~CBookmarkDlg()
{
}

void CBookmarkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_BOOKMARK_TITLE, m_strTitle);
}


BEGIN_MESSAGE_MAP(CBookmarkDlg, CDialog)
END_MESSAGE_MAP()


// CBookmarkDlg message handlers

BOOL CBookmarkDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	SetWindowText(LoadString(m_nTitle));

	return true;
}
