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

// $Id$

#include "stdafx.h"
#include "WinDjView.h"
#include "ZoomDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCustomZoomDlg dialog

IMPLEMENT_DYNAMIC(CCustomZoomDlg, CDialog)
CCustomZoomDlg::CCustomZoomDlg(double fZoom, CWnd* pParent)
	: CDialog(CCustomZoomDlg::IDD, pParent), m_fZoom(fZoom)
{
	m_edtZoom.SetReal();
	m_edtZoom.SetPercent();
}

CCustomZoomDlg::~CCustomZoomDlg()
{
}

void CCustomZoomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ZOOM, m_edtZoom);
	DDX_MyText(pDX, IDC_ZOOM, m_fZoom, 100.0, _T("%"));
	DDV_MinMaxDouble(pDX, m_fZoom, 10.0, 1600.0);
}


BEGIN_MESSAGE_MAP(CCustomZoomDlg, CDialog)
END_MESSAGE_MAP()


// CCustomZoomDlg message handlers
