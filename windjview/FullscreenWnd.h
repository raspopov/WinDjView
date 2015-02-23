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

class CDjVuView;


// CFullscreenWnd

class CFullscreenWnd : public CWnd
{
	DECLARE_DYNAMIC(CFullscreenWnd)

public:
	CFullscreenWnd();
	virtual ~CFullscreenWnd();

	BOOL Create();

	void Show(CDjVuView* pOwner, CDjVuView* pContents);
	void Hide();

	CDjVuView* GetView() const { return m_pView; }
	CDjVuView* GetOwner() const { return m_pOwner; }

// Overrides
public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

protected:
	CDjVuView* m_pView;
	CDjVuView* m_pOwner;
	HICON m_hIcon;

	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnAppCommand(WPARAM wParam, LPARAM lParam);
	void OnEnable(BOOL bEnable);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()
};
