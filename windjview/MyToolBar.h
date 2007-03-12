//	WinDjView
//	Copyright (C) 2004-2007 Andrew Zhezherun
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

#include "Global.h"
#include "MyTheme.h"


// CMyToolBar

class CMyToolBar : public CToolBar, public Observer
{
	DECLARE_DYNAMIC(CMyToolBar)

public:
	CMyToolBar();
	virtual ~CMyToolBar();

	void SetSizes(SIZE sizeButton, SIZE sizeImage);
	virtual void DrawBorders(CDC* pDC, CRect& rect);
	void EraseNonClient();
	virtual void DoPaint(CDC* pDC);

	void InsertLabel(int nPos, UINT nID, CFont* pFont);

	virtual void OnUpdate(const Observable* source, const Message* message);

protected:
	struct Label
	{
		UINT nID;
		CString strText;
		CFont* pFont;
	};
	vector<Label> m_labels;

	HTHEME m_hTheme;
	afx_msg void OnNcPaint();
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnThemeChanged();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
