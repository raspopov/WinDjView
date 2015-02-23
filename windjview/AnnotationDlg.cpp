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
#include "AnnotationDlg.h"
#include "DjVuSource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


const TCHAR* s_pszCustomColors = _T("Colors");


// CAnnotationDlg dialog

IMPLEMENT_DYNAMIC(CAnnotationDlg, CMyDialog)

CAnnotationDlg::CAnnotationDlg(UINT nTitle, CWnd* pParent)
	: CMyDialog(CAnnotationDlg::IDD, pParent), m_nTitle(nTitle),
	  m_bDisableShowComment(false)
{
	m_nBorderType = Annotation::BorderNone;
	m_nFillType = Annotation::FillSolid;
	m_crBorder = RGB(0, 0, 0);
	m_crFill = RGB(255, 255, 0);
	m_nTransparency = 75;
	m_bHideInactiveBorder = false;
	m_bHideInactiveFill = false;
	m_bAlwaysShowComment = false;
	m_bAddBookmark = false;
	m_bEnableBookmark = true;
}

CAnnotationDlg::~CAnnotationDlg()
{
}

void CAnnotationDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BORDER_COLOR, m_colorBorder);
	DDX_Control(pDX, IDC_FILL_COLOR, m_colorFill);
	DDX_Color(pDX, IDC_BORDER_COLOR, m_crBorder);
	DDX_Color(pDX, IDC_FILL_COLOR, m_crFill);
	DDX_Control(pDX, IDC_FILL_TRANSPARENCY, m_sliderTransparency);
	DDX_Control(pDX, IDC_BORDER_TYPE, m_cboBorderType);
	DDX_Control(pDX, IDC_FILL_TYPE, m_cboFillType);
	DDX_Text(pDX, IDC_COMMENT, m_strComment);
	DDX_Check(pDX, IDC_ALWAYS_SHOW_COMMENT, m_bAlwaysShowComment);
	DDX_Check(pDX, IDC_HIDE_INACTIVE_BORDER, m_bHideInactiveBorder);
	DDX_Check(pDX, IDC_HIDE_INACTIVE_FILL, m_bHideInactiveFill);
	DDX_Text(pDX, IDC_BOOKMARK_TITLE, m_strBookmark);

	CString strTransparency;
	strTransparency.Format(_T("%d%%"), m_nTransparency);
	DDX_Text(pDX, IDC_FILL_TRANSPARENCY_TEXT, strTransparency);
}


BEGIN_MESSAGE_MAP(CAnnotationDlg, CMyDialog)
	ON_CBN_SELCHANGE(IDC_BORDER_TYPE, OnChangeCombo)
	ON_CBN_SELCHANGE(IDC_FILL_TYPE, OnChangeCombo)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_ADD_BOOKMARK, OnAddBookmark)
END_MESSAGE_MAP()


// CAnnotationDlg message handlers

BOOL CAnnotationDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();

	SetWindowText(LoadString(m_nTitle));

	if (m_bDisableShowComment)
		GetDlgItem(IDC_ALWAYS_SHOW_COMMENT)->EnableWindow(false);

	m_sliderTransparency.SetRange(0, 100);
	m_sliderTransparency.SetPos(m_nTransparency);
	m_sliderTransparency.SetLineSize(1);
	m_sliderTransparency.SetPageSize(5);

	m_sliderTransparency.ClearTics();
	for (int i = 0; i <= 20; ++i)
		m_sliderTransparency.SetTic(5*i);

	CString strBorderTypes;
	strBorderTypes.LoadString(IDS_BORDER_TYPES);

	CString strPart;
	for (int nBorder = Annotation::BorderNone; nBorder <= Annotation::BorderXOR; ++nBorder)
	{
		AfxExtractSubString(strPart, strBorderTypes, nBorder, ',');
		m_cboBorderType.AddString(strPart);
	}

	CString strFillTypes;
	strFillTypes.LoadString(IDS_FILL_TYPES);

	for (int nFill = Annotation::FillNone; nFill <= Annotation::FillXOR; ++nFill)
	{
		AfxExtractSubString(strPart, strFillTypes, nFill, ',');
		m_cboFillType.AddString(strPart);
	}

	m_cboBorderType.SetCurSel(m_nBorderType);
	m_cboFillType.SetCurSel(m_nFillType);

	m_colorBorder.SetCustomText(LoadString(IDS_MORE_COLORS));
	m_colorBorder.SetColorNames(IDS_COLOR_PALETTE);
	m_colorBorder.SetRegSection(s_pszCustomColors);

	m_colorFill.SetCustomText(LoadString(IDS_MORE_COLORS));
	m_colorFill.SetColorNames(IDS_COLOR_PALETTE);
	m_colorFill.SetRegSection(s_pszCustomColors);

	UpdateControls();

	if (!m_bEnableBookmark)
		GetDlgItem(IDC_ADD_BOOKMARK)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_BOOKMARK_TITLE)->ShowWindow(SW_HIDE);
	ToggleDialog(false, true);

	return true;
}

void CAnnotationDlg::ToggleDialog(bool bExpand, bool bCenterWindow)
{
	CRect rcWindow;
	GetWindowRect(rcWindow);

	CRect rcPos;
	GetDlgItem(bExpand ? IDC_STATIC_MORE : IDC_STATIC_LESS)->GetWindowRect(rcPos);

	rcWindow.bottom = rcPos.bottom + ::GetSystemMetrics(SM_CYBORDER);
	MoveWindow(rcWindow);

	if (bCenterWindow)
		CenterWindow();

	GetDlgItem(IDC_ADD_BOOKMARK)->SetWindowText(LoadString(
		bExpand ? IDS_BOOKMARK_LESS : IDS_BOOKMARK_MORE));
	GetDlgItem(IDC_BOOKMARK_TITLE)->ShowWindow(bExpand ? SW_SHOW : SW_HIDE);
}

void CAnnotationDlg::UpdateControls()
{
	m_colorBorder.EnableWindow(m_nBorderType == Annotation::BorderSolid);
	GetDlgItem(IDC_HIDE_INACTIVE_BORDER)->EnableWindow(m_nBorderType != Annotation::BorderNone);

	m_colorFill.EnableWindow(m_nFillType == Annotation::FillSolid);
	m_sliderTransparency.EnableWindow(m_nFillType == Annotation::FillSolid);
	GetDlgItem(IDC_FILL_TRANSPARENCY_TEXT)->EnableWindow(m_nFillType == Annotation::FillSolid);
	GetDlgItem(IDC_HIDE_INACTIVE_FILL)->EnableWindow(m_nFillType != Annotation::FillNone);
}

void CAnnotationDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;
	if (pSlider == &m_sliderTransparency)
	{
		m_nTransparency = m_sliderTransparency.GetPos();
		UpdateData(false);
	}
}

void CAnnotationDlg::OnChangeCombo()
{
	UpdateData();
	m_nBorderType = m_cboBorderType.GetCurSel();
	m_nFillType = m_cboFillType.GetCurSel();

	UpdateControls();
}

void CAnnotationDlg::OnAddBookmark()
{
	UpdateData();

	if (m_bEnableBookmark)
	{
		m_bAddBookmark = !m_bAddBookmark;
		ToggleDialog(m_bAddBookmark);
	}

	UpdateControls();
}
