//	WinDjView 0.1
//	Copyright (C) 2004 Andrew Zhezherun
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
//  http://www.gnu.org/copyleft/gpl.html

// $Id$

#include "stdafx.h"
#include "WinDjView.h"
#include "MyComboBox.h"


// CMyComboBox

IMPLEMENT_DYNAMIC(CMyComboBox, CComboBox)

CMyComboBox::CMyComboBox()
{
}

CMyComboBox::~CMyComboBox()
{
}

BEGIN_MESSAGE_MAP(CMyComboBox, CComboBox)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CMyComboBox message handlers

HBRUSH CMyComboBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_EDIT)
	{
		if (m_edit.GetSafeHwnd() == NULL)
			m_edit.SubclassWindow(pWnd->GetSafeHwnd());
	}

	HBRUSH hbr = CComboBox::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}

BOOL CMyComboBox::CNotifyingEdit::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		CWnd* pCombo = GetParent();
		CWnd* pParent = pCombo->GetParent();

		if (pMsg->wParam == VK_RETURN)
		{
			pParent->SendMessage(WM_COMMAND, MAKEWPARAM(pCombo->GetDlgCtrlID(), CBN_FINISHEDIT), (LPARAM)pCombo->m_hWnd);
			return TRUE;
		}
		else if (pMsg->wParam == VK_ESCAPE)
		{
			pParent->SendMessage(WM_COMMAND, MAKEWPARAM(pCombo->GetDlgCtrlID(), CBN_CANCELEDIT), (LPARAM)pCombo->m_hWnd);
			return TRUE;
		}
	}

	return CMyEdit::PreTranslateMessage(pMsg);
}
