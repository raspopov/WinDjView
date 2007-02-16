//	DjVu Page Index Tool
//	Copyright (C) 2006-2007 Andrew Zhezherun
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

#include "stdafx.h"
#include "IndexTool.h"
#include "MyDropTarget.h"


// CMyDropTarget

CMyDropTarget::CMyDropTarget()
{
}

CMyDropTarget::~CMyDropTarget()
{
}

DROPEFFECT CMyDropTarget::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
	DWORD dwKeyState, CPoint point)
{
	IMyDropTarget* pTarget = dynamic_cast<IMyDropTarget*>(pWnd);
	if (pTarget != NULL)
		return pTarget->OnDragEnter(pWnd, pDataObject, dwKeyState, point);
	else
		return DROPEFFECT_NONE;
}

DROPEFFECT CMyDropTarget::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
	DWORD dwKeyState, CPoint point)
{
	IMyDropTarget* pTarget = dynamic_cast<IMyDropTarget*>(pWnd);
	if (pTarget != NULL)
		return pTarget->OnDragOver(pWnd, pDataObject, dwKeyState, point);
	else
		return DROPEFFECT_NONE;
}

BOOL CMyDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
	DROPEFFECT dropEffect, CPoint point)
{
	IMyDropTarget* pTarget = dynamic_cast<IMyDropTarget*>(pWnd);
	if (pTarget != NULL)
		return pTarget->OnDrop(pWnd, pDataObject, dropEffect, point);
	else
		return false;
}

void CMyDropTarget::OnDragLeave(CWnd* pWnd)
{
	IMyDropTarget* pTarget = dynamic_cast<IMyDropTarget*>(pWnd);
	if (pTarget != NULL)
		pTarget->OnDragLeave(pWnd);
}

DROPEFFECT CMyDropTarget::OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point)
{
	IMyDropTarget* pTarget = dynamic_cast<IMyDropTarget*>(pWnd);
	if (pTarget != NULL)
		return pTarget->OnDragScroll(pWnd, dwKeyState, point);
	else
		return DROPEFFECT_NONE;
}
