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


// CMyComboBox

#include "MyEdit.h"

#define CBN_FINISHEDIT 100
#define CBN_CANCELEDIT 101
#define CBN_MOUSEWHEEL 102

struct NMCBWHEEL
{
	NMHDR hdr;
	BOOL bUp;
};

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
	public:
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	};

	CNotifyingEdit m_edit;

	// Message map functions
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
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
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	DECLARE_MESSAGE_MAP()
};
