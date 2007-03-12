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


// CMyComboBox

#include "MyEdit.h"

#define CBN_FINISHEDIT 100
#define CBN_CANCELEDIT 101

class CMyComboBox : public CComboBox
{
	DECLARE_DYNAMIC(CMyComboBox)

public:
	CMyComboBox();
	virtual ~CMyComboBox();

	CMyEdit* GetEditCtrl() { return &m_edit; }

protected:
	class CNotifyingEdit : public CMyEdit
	{
		virtual BOOL PreTranslateMessage(MSG* pMsg);
	};

	CNotifyingEdit m_edit;

	// Message map functions
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()

	friend class CMyComboBoxEx;
};

class CMyComboBoxEx : public CComboBoxEx
{
	DECLARE_DYNAMIC(CMyComboBoxEx)

public:
	CMyComboBoxEx();
	virtual ~CMyComboBoxEx();

	CMyEdit* GetEditCtrl() { return &m_edit; }

protected:
	CMyComboBox::CNotifyingEdit m_edit;

	// Message map functions
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
};
