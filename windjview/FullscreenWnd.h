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
//	http://www.gnu.org/copyleft/gpl.html

// $Id$

#pragma once

class CDjVuView;


// CFullscreenWnd

class CFullscreenWnd : public CWnd
{
	DECLARE_DYNAMIC(CFullscreenWnd)

public:
	CFullscreenWnd(CDjVuView* pOwner);
	virtual ~CFullscreenWnd();

	BOOL Create();
	void SetView(CDjVuView* pView);
	CDjVuView* GetView() const { return m_pView; }
	CDjVuView* GetOwner() const { return m_pOwner; }

protected:
	CDjVuView* m_pView;
	CDjVuView* m_pOwner;
	int m_nWidth, m_nHeight;

	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
};
