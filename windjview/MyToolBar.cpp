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
#include "MyToolBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMyToolBar

IMPLEMENT_DYNAMIC(CMyToolBar, CToolBar)
CMyToolBar::CMyToolBar()
{
}

CMyToolBar::~CMyToolBar()
{
}

void CMyToolBar::DrawBorders(CDC* pDC, CRect& rect)
{
	// MFC fix (original code from CControlBar::DrawBorders)

	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	DWORD dwStyle = m_dwStyle;
	if (!(dwStyle & CBRS_BORDER_ANY))
		return;

	// prepare for dark lines
	ASSERT(rect.top == 0 && rect.left == 0);
	CRect rect1, rect2;
	rect1 = rect;
	rect2 = rect;
	COLORREF clr = afxData.clrBtnShadow;

	// draw dark line one pixel back/up
	if (dwStyle & CBRS_BORDER_3D)
	{
		rect1.right -= CX_BORDER;
		rect1.bottom -= CY_BORDER;
	}
	if (dwStyle & CBRS_BORDER_TOP)
		rect2.top += afxData.cyBorder2;
	if (dwStyle & CBRS_BORDER_BOTTOM)
		rect2.bottom -= afxData.cyBorder2;

	if (IsKindOf(RUNTIME_CLASS(CToolBar)) && (dwStyle & TBSTYLE_FLAT) && (dwStyle & CBRS_BORDER_ANY))
	{
		CRect rctBk;
		rctBk.left = 0;
		rctBk.right = rect.right;
		rctBk.top = rect.top;
		rctBk.bottom = rect.bottom;

//-		pDC->FillSolidRect((LPRECT)rctBk, pDC->GetBkColor());
		pDC->FillSolidRect((LPRECT)rctBk, ::GetSysColor(COLOR_BTNFACE));
	}

	// draw left and top
	if (dwStyle & CBRS_BORDER_LEFT)
		pDC->FillSolidRect(0, rect2.top, CX_BORDER, rect2.Height(), clr);
	if (dwStyle & CBRS_BORDER_TOP)
		pDC->FillSolidRect(0, 0, rect.right, CY_BORDER, clr);

	// draw right and bottom
	if (dwStyle & CBRS_BORDER_RIGHT)
		pDC->FillSolidRect(rect1.right, rect2.top, -CX_BORDER, rect2.Height(), clr);
	if (dwStyle & CBRS_BORDER_BOTTOM)
		pDC->FillSolidRect(0, rect1.bottom, rect.right, -CY_BORDER, clr);

	if (dwStyle & CBRS_BORDER_3D)
	{
		// prepare for hilite lines
		clr = afxData.clrBtnHilite;

		// draw left and top
		if (dwStyle & CBRS_BORDER_LEFT)
			pDC->FillSolidRect(1, rect2.top, CX_BORDER, rect2.Height(), clr);
		if (dwStyle & CBRS_BORDER_TOP)
			pDC->FillSolidRect(0, 1, rect.right, CY_BORDER, clr);

		// draw right and bottom
		if (dwStyle & CBRS_BORDER_RIGHT)
			pDC->FillSolidRect(rect.right, rect2.top, -CX_BORDER, rect2.Height(), clr);
		if (dwStyle & CBRS_BORDER_BOTTOM)
			pDC->FillSolidRect(0, rect.bottom, rect.right, -CY_BORDER, clr);
	}

	if (dwStyle & CBRS_BORDER_LEFT)
		rect.left += afxData.cxBorder2;
	if (dwStyle & CBRS_BORDER_TOP)
		rect.top += afxData.cyBorder2;
	if (dwStyle & CBRS_BORDER_RIGHT)
		rect.right -= afxData.cxBorder2;
	if (dwStyle & CBRS_BORDER_BOTTOM)
		rect.bottom -= afxData.cyBorder2;
}


BEGIN_MESSAGE_MAP(CMyToolBar, CToolBar)
END_MESSAGE_MAP()


// CMyToolBar message handlers

void CMyToolBar::SetSizes(SIZE sizeButton, SIZE sizeImage)
{
	CToolBar::SetSizes(sizeButton, sizeImage);
	m_sizeButton = sizeButton;
	m_sizeImage = sizeImage;
}
