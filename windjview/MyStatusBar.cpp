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
//  http://www.gnu.org/copyleft/gpl.html

// $Id$

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
	ON_MESSAGE(WM_SETTEXT, OnSetText)
END_MESSAGE_MAP()


// CMyStatusBar message handlers

LRESULT CMyStatusBar::OnSetText(WPARAM wParam, LPARAM lParam)
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_hWnd));

	m_strHilightMsg = "";
	LRESULT result = CStatusBar::OnSetText(wParam, lParam);
	GetStatusBarCtrl().SetText((LPCTSTR)lParam, 0, SBT_NOBORDERS);

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

	CBrush brush(RGB(0, 0, 128));
	CBrush* pOldBrush = dc.SelectObject(&brush);
	dc.Rectangle(&rcItem);

	rcItem.DeflateRect(8, 0, 5, 0);

	COLORREF clrOldText = dc.GetTextColor();
	dc.SetTextColor(RGB(255, 255, 255));

	COLORREF clrOldBk = dc.GetBkColor();
	dc.SetBkColor(RGB(0, 0, 128));

	dc.DrawText(m_strHilightMsg, &rcItem, DT_SINGLELINE | DT_VCENTER | DT_LEFT);

	dc.SetBkColor(clrOldBk);
	dc.SetTextColor(clrOldText);
	dc.SelectObject(pOldBrush);

	dc.Detach();
}