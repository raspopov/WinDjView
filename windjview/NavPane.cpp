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
#include "NavPane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CnavPaneWnd

const int CNavPaneWnd::s_nHorzBorderWidth = 5;
const int CNavPaneWnd::s_nVertBorderHeight = 4;

IMPLEMENT_DYNCREATE(CNavPaneWnd, CWnd)
CNavPaneWnd::CNavPaneWnd() : m_pChildWnd(NULL)
{
}

CNavPaneWnd::~CNavPaneWnd()
{
}


BEGIN_MESSAGE_MAP(CNavPaneWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CNavPaneWnd message handlers

void CNavPaneWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rcClient;
	GetClientRect(&rcClient);

	CRect rc = rcClient;
	COLORREF clrBtnface = ::GetSysColor(COLOR_BTNFACE);
	CBrush brushBtnface(clrBtnface);
	
	// Borders
	if (rc.Height() >= 0)
	{
		int nOffsetX = max(0, min(s_nHorzBorderWidth, rc.Width()));
		dc.FillRect(&CRect(rc.left, rc.top,
			rc.left + nOffsetX, rc.bottom), &brushBtnface);
		dc.FillRect(&CRect(rc.right - nOffsetX, rc.top,
			rc.right, rc.bottom), &brushBtnface);
	}

	if (rc.Width() >= 0)
	{
		int nOffsetY = max(0, min(s_nVertBorderHeight, rc.Height()));
		dc.FillRect(&CRect(rc.left, rc.top,
			rc.right, rc.top + nOffsetY), &brushBtnface);
		dc.FillRect(&CRect(rc.left, rc.bottom - nOffsetY,
			rc.right, rc.bottom), &brushBtnface);
	}
	
	rc.DeflateRect(s_nHorzBorderWidth, s_nVertBorderHeight);

	if (m_pChildWnd == NULL || !m_pChildWnd->IsWindowVisible())
	{
		if (rc.Width() >= 0 && rc.Height() >= 0)
		{
			dc.FillRect(&rc, &brushBtnface);
		}
	}
}

int CNavPaneWnd::GetMinWidth() const
{
	return 2*s_nHorzBorderWidth + 5;
}

void CNavPaneWnd::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CWnd::OnWindowPosChanged(lpwndpos);
	
	if (m_pChildWnd != NULL)
	{
		CRect rc;
		GetClientRect(&rc);
		rc.right -= 1;
		rc.DeflateRect(s_nHorzBorderWidth, s_nVertBorderHeight + 2);

		if (rc.Height() < 4 || rc.Width() < 4)
		{
			m_pChildWnd->ShowWindow(SW_HIDE);
		}
		else
		{
			m_pChildWnd->ShowWindow(SW_SHOW);

			m_pChildWnd->SetWindowPos(NULL,
				rc.left, rc.top, rc.Width(), rc.Height(),
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
}

void CNavPaneWnd::SetChildWnd(CWnd *pWnd)
{
	ASSERT(pWnd->GetParent() == this);
	m_pChildWnd = pWnd;
}

CWnd* CNavPaneWnd::GetChildWnd() const
{
	return m_pChildWnd;
}

BOOL CNavPaneWnd::OnEraseBkgnd(CDC* pDC)
{
	return true;
}
