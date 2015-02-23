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

#include "MyDialog.h"
#include "MyColorPicker.h"


// CAnnotationDlg dialog

class CAnnotationDlg : public CMyDialog
{
	DECLARE_DYNAMIC(CAnnotationDlg)

public:
	CAnnotationDlg(UINT nTitle, CWnd* pParent = NULL);
	virtual ~CAnnotationDlg();

// Dialog Data
	enum { IDD = IDD_ANNOTATION };
	CMyColorPicker m_colorBorder;
	CMyColorPicker m_colorFill;
	CSliderCtrl m_sliderTransparency;
	CComboBox m_cboBorderType;
	CComboBox m_cboFillType;
	BOOL m_bHideInactiveBorder;
	BOOL m_bHideInactiveFill;
	int m_nBorderType;
	int m_nFillType;
	COLORREF m_crBorder;
	COLORREF m_crFill;
	int m_nTransparency;
	CString m_strComment;
	BOOL m_bAlwaysShowComment;
	bool m_bAddBookmark;
	bool m_bEnableBookmark;
	CString m_strBookmark;

	bool m_bDisableShowComment;

protected:
	UINT m_nTitle;
	void UpdateControls();
	void ToggleDialog(bool bExpand, bool bCenterWindow = false);

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	afx_msg void OnChangeCombo();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnAddBookmark();
	DECLARE_MESSAGE_MAP()
};
