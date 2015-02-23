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
#include "WinDjView.h"
#include "MyBitmapButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMyBitmapButton

IMPLEMENT_DYNAMIC(CMyBitmapButton, CBitmapButton)
CMyBitmapButton::CMyBitmapButton()
{
}

CMyBitmapButton::~CMyBitmapButton()
{
}

BEGIN_MESSAGE_MAP(CMyBitmapButton, CBitmapButton)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


// CMyBitmapButton message handlers

BOOL CMyBitmapButton::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	static HCURSOR hCursor = NULL;
	if (hCursor == NULL)
	{
		hCursor = ::LoadCursor(NULL, IDC_HAND);
		if (hCursor == NULL)
			hCursor = theApp.LoadCursor(IDC_CURSOR_LINK);
	}

	::SetCursor(hCursor);
	return TRUE;
}
