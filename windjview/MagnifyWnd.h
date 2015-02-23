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

#pragma once

#include "Drawing.h"

class CDjVuView;


// CFullscreenWnd

class CMagnifyWnd : public CWnd
{
	DECLARE_DYNAMIC(CMagnifyWnd)

public:
	CMagnifyWnd();
	virtual ~CMagnifyWnd();

	BOOL Create();
	void Init(CDjVuView* pOwner, CDjVuView* pContents);
	void Hide();
	void CenterOnPoint(const CPoint& point);

	CDjVuView* GetOwner() const { return m_pOwner; }
	CDjVuView* GetView() const { return m_pView; }
	CSize GetViewSize() const { return m_rcPos.Size() - CSize(2, 2); }
	CPoint GetCenterPoint() const { return m_rcPos.CenterPoint(); }

protected:
	CDjVuView* m_pView;
	CDjVuView* m_pOwner;
	CRect m_rcPos;
	COffscreenDC m_offscreenDC;

	typedef BOOL (WINAPI* pfnUpdateLayeredWindow)(HWND hwnd, HDC hdcDst,
			POINT* pptDst, SIZE* psize, HDC hdcSrc, POINT* pptSrc,
            COLORREF crKey, BLENDFUNCTION* pblend, DWORD dwFlags);
	pfnUpdateLayeredWindow m_pUpdateLayeredWindow;
	HMODULE m_hUser32;

	void RepaintContents();

	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()
};
