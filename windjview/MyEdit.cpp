//	WinDjView
//	Copyright (C) 2004-2006 Andrew Zhezherun
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
//	http://www.gnu.org/copyleft/gpl.html

// $Id$

#include "stdafx.h"
#include "MyEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMyEdit

IMPLEMENT_DYNAMIC(CMyEdit, CEdit)
CMyEdit::CMyEdit()
{
	m_nType = EditNormal;
}

CMyEdit::~CMyEdit()
{
}


BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()



// CMyEdit message handlers


void CMyEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_nType == EditReal)
	{
		if ((nChar < '0' || nChar > '9') && nChar != '.' && nChar != VK_BACK &&
				(!m_bPercent || nChar != '%'))
			return;
	}
	else if (m_nType == EditInteger)
	{
		if ((nChar < '0' || nChar > '9') && nChar != VK_BACK &&
				(!m_bPercent || nChar != '%'))
			return;
	}

	CEdit::OnChar(nChar, nRepCnt, nFlags);
}
