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

#pragma once

#include "MyDialog.h"
#include "MyEdit.h"


// CCustomZoomDlg dialog

class CCustomZoomDlg : public CMyDialog
{
	DECLARE_DYNAMIC(CCustomZoomDlg)

public:
	CCustomZoomDlg(double fZoom, CWnd* pParent = NULL);
	virtual ~CCustomZoomDlg();

// Dialog Data
	enum { IDD = IDD_ZOOM };
	double m_fZoom;

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	CMyEdit m_edtZoom;

// Generated message map functions
	DECLARE_MESSAGE_MAP()
};
