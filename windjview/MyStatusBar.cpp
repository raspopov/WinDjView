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
#include "MyStatusBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMyStatusBar

IMPLEMENT_DYNAMIC(CMyStatusBar, CStatusBar)
CMyStatusBar::CMyStatusBar()
{
}

CMyStatusBar::~CMyStatusBar()
{
}


BEGIN_MESSAGE_MAP(CMyStatusBar, CStatusBar)
	ON_WM_SETTEXT()
END_MESSAGE_MAP()


// CMyStatusBar message handlers

int CMyStatusBar::OnSetText(LPCTSTR lpszText)
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_hWnd));

	m_strHilightMsg = _T("");
	int result = CStatusBar::OnSetText(lpszText);
	GetStatusBarCtrl().SetText(lpszText, 0, SBT_NOBORDERS);

	return result;
}

void CMyStatusBar::SetHilightMessage(LPCTSTR pszMessage)
{
	m_strHilightMsg = pszMessage;

	GetStatusBarCtrl().SetText(LPCTSTR(0x1234), 0, SBT_NOBORDERS | SBT_OWNERDRAW);
}

void CMyStatusBar::DrawItem(LPDRAWITEMSTRUCT lpDrawItem)
{
	if (lpDrawItem->itemData != (ULONG)0x1234)
		return;

	CRect rcItem;
	GetItemRect(0, &rcItem);

	rcItem.InflateRect(0, 1, 0, 0);

	CDC dc;
	dc.Attach(lpDrawItem->hDC);

	CBrush brush(::GetSysColor(COLOR_HIGHLIGHT));
	CBrush* pOldBrush = dc.SelectObject(&brush);
	dc.Rectangle(&rcItem);

	rcItem.DeflateRect(8, 0, 5, 0);

	COLORREF crTextColor = dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(m_strHilightMsg, &rcItem, DT_SINGLELINE | DT_VCENTER | DT_LEFT);

	dc.SetTextColor(crTextColor);
	dc.SelectObject(pOldBrush);

	dc.Detach();
}
