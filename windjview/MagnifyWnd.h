//	WinDjView
//	Copyright (C) 2004-2009 Andrew Zhezherun
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

// $Id$

#pragma once

class CDjVuView;


// CFullscreenWnd

class CMagnifyWnd : public CWnd
{
	DECLARE_DYNAMIC(CMagnifyWnd)

public:
	CMagnifyWnd();
	virtual ~CMagnifyWnd();

	BOOL Create();
	void Show(CDjVuView* pOwner, CDjVuView* pContents, const CPoint& ptCenter);
	void Hide();
	void CenterOnPoint(const CPoint& point);

	void Update();

	CDjVuView* GetView() const { return m_pView; }
	CDjVuView* GetOwner() const { return m_pOwner; }

	int GetViewWidth() const { return m_nWidth - 2; }
	int GetViewHeight() const { return m_nHeight - 2; }

protected:
	CDjVuView* m_pView;
	CDjVuView* m_pOwner;
	int m_nWidth, m_nHeight;
	bool m_bFirstUpdate;

	typedef BOOL (WINAPI* pfnSetLayeredWindowAttributes)(HWND hwnd,
			COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
	pfnSetLayeredWindowAttributes m_pSetLayeredWindowAttributes;
	HMODULE m_hUser32;

	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()
};
