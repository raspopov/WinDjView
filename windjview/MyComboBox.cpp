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
#include "MyComboBox.h"
#include "Global.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#if (_MFC_VER <= 0x0600)
struct COMBOBOXINFO
{
    DWORD cbSize;
    RECT rcItem;
    RECT rcButton;
    DWORD stateButton;
    HWND hwndCombo;
    HWND hwndItem;
    HWND hwndList;
};

WINUSERAPI BOOL WINAPI GetComboBoxInfo(HWND hwndCombo, COMBOBOXINFO* pcbi);
#endif

// CMyComboBox

IMPLEMENT_DYNAMIC(CMyComboBox, CComboBox)

CMyComboBox::CMyComboBox()
{
}

CMyComboBox::~CMyComboBox()
{
}

BEGIN_MESSAGE_MAP(CMyComboBox, CComboBox)
	ON_WM_CREATE()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()


// CMyComboBox message handlers

int CMyComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CComboBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	if ((GetStyle() & 0x000F) == CBS_DROPDOWN)
	{
		COMBOBOXINFO cbi;
		cbi.cbSize = sizeof(cbi);
		::GetComboBoxInfo(m_hWnd, &cbi);
		m_edit.SubclassWindow(cbi.hwndItem);
	}

	return 0;
}

BOOL CMyComboBox::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	CWnd* pWnd = WindowFromPoint(point);
	if (pWnd != this && !IsChild(pWnd) && IsFromCurrentProcess(pWnd)
			&& pWnd->SendMessage(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(point.x, point.y)) != 0)
		return true;

	if ((nFlags & MK_CONTROL) != 0 || (nFlags & MK_SHIFT) != 0)
		return false;

	NMCBWHEEL nm;
	ZeroMemory(&nm, sizeof(nm));
	nm.hdr.hwndFrom = GetSafeHwnd();
	nm.hdr.idFrom = GetDlgCtrlID();
	nm.hdr.code = CBN_MOUSEWHEEL;
	nm.bUp = (zDelta > 0);

	if (GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM) &nm))
		return true;

	return CComboBox::OnMouseWheel(nFlags, zDelta, point);
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
			return true;
		}
		else if (pMsg->wParam == VK_ESCAPE)
		{
			pParent->SendMessage(WM_COMMAND, MAKEWPARAM(pCombo->GetDlgCtrlID(), CBN_CANCELEDIT), (LPARAM)pCombo->m_hWnd);
			return true;
		}
	}

	return CMyEdit::PreTranslateMessage(pMsg);
}

BOOL CMyComboBox::CNotifyingEdit::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (message == WM_MOUSEWHEEL)
	{
		LRESULT lResult = GetParent()->SendMessage(WM_MOUSEWHEEL, wParam, lParam);
		if (pResult != NULL)
			*pResult = lResult;
		return true;
	}

	return CMyEdit::OnWndMsg(message, wParam, lParam, pResult);
}

// CMyComboBoxEx

IMPLEMENT_DYNAMIC(CMyComboBoxEx, CComboBoxEx)

CMyComboBoxEx::CMyComboBoxEx()
{
}

CMyComboBoxEx::~CMyComboBoxEx()
{
}

BEGIN_MESSAGE_MAP(CMyComboBoxEx, CComboBoxEx)
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CMyComboBox message handlers

int CMyComboBoxEx::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CComboBoxEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_edit.SubclassWindow(CComboBoxEx::GetEditCtrl()->GetSafeHwnd());

	return 0;
}

BOOL CMyComboBoxEx::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	CWnd* pWnd = WindowFromPoint(point);
	if (pWnd != this && !IsChild(pWnd) && IsFromCurrentProcess(pWnd)
			&& pWnd->SendMessage(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(point.x, point.y)) != 0)
		return true;

	if ((nFlags & MK_CONTROL) != 0 || (nFlags & MK_SHIFT) != 0)
		return false;

	NMCBWHEEL nm;
	ZeroMemory(&nm, sizeof(nm));
	nm.hdr.hwndFrom = GetSafeHwnd();
	nm.hdr.idFrom = GetDlgCtrlID();
	nm.hdr.code = CBN_MOUSEWHEEL;
	nm.bUp = (zDelta > 0);

	if (GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM) &nm))
		return true;

	return CComboBoxEx::OnMouseWheel(nFlags, zDelta, point);
}
