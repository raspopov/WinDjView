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

#pragma once


// CBookmarkDlg dialog

class CBookmarkDlg : public CDialog
{
	DECLARE_DYNAMIC(CBookmarkDlg)

public:
	CBookmarkDlg(UINT nTitle, CWnd* pParent = NULL);
	virtual ~CBookmarkDlg();

// Dialog Data
	enum { IDD = IDD_RENAME_BOOKMARK };
	CString m_strTitle;

protected:
	UINT m_nTitle;

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
