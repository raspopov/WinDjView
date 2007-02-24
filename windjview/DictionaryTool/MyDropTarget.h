//	DjVu Dictionary Tool
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

#pragma once


class IMyDropTarget
{
public:
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point) = 0;
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point) = 0;
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
		DROPEFFECT dropEffect, CPoint point) = 0;
	virtual void OnDragLeave(CWnd* pWnd) = 0;
	virtual DROPEFFECT OnDragScroll(CWnd* pWnd, DWORD dwKeyState,
		CPoint point) = 0;
};

// CMyDropTarget command target

class CMyDropTarget : public COleDropTarget
{
public:
	CMyDropTarget();
	virtual ~CMyDropTarget();

// Overrides
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
		DROPEFFECT dropEffect, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDragScroll(CWnd* pWnd, DWORD dwKeyState,
		CPoint point);
};
