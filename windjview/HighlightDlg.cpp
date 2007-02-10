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

#include "stdafx.h"
#include "WinDjView.h"
#include "HighlightDlg.h"
#include "DjVuSource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


const TCHAR* s_pszCustomColors = _T("Colors");


// CHighlightDlg dialog

IMPLEMENT_DYNAMIC(CHighlightDlg, CDialog)

CHighlightDlg::CHighlightDlg(CWnd* pParent)
	: CDialog(CHighlightDlg::IDD, pParent)
{
	m_nBorderType = DjVuHighlight::BorderNone;
	m_nFillType = DjVuHighlight::FillSolid;
	m_crBorder = RGB(0, 0, 0);
	m_crFill = RGB(255, 255, 0);
	m_nTransparency = 75;
	m_bHideInactive = false;
}

CHighlightDlg::~CHighlightDlg()
{
}

void CHighlightDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BORDER_COLOR, m_colorBorder);
	DDX_Control(pDX, IDC_FILL_COLOR, m_colorFill);
	DDX_Color(pDX, IDC_BORDER_COLOR, m_crBorder);
	DDX_Color(pDX, IDC_FILL_COLOR, m_crFill);
	DDX_Control(pDX, IDC_FILL_TRANSPARENCY, m_sliderTransparency);
	DDX_Control(pDX, IDC_BORDER_TYPE, m_cboBorderType);
	DDX_Control(pDX, IDC_FILL_TYPE, m_cboFillType);
	DDX_Text(pDX, IDC_URL, m_strURL);
	DDX_Text(pDX, IDC_COMMENT, m_strComment);
	DDX_Check(pDX, IDC_HIDE_INACTIVE, m_bHideInactive);

	CString strTransparency;
	strTransparency.Format(_T("%d%%"), m_nTransparency);
	DDX_Text(pDX, IDC_FILL_TRANSPARENCY_TEXT, strTransparency);
}


BEGIN_MESSAGE_MAP(CHighlightDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_BORDER_TYPE, OnChangeCombo)
	ON_CBN_SELCHANGE(IDC_FILL_TYPE, OnChangeCombo)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CHighlightDlg message handlers

BOOL CHighlightDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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
	for (int nBorder = DjVuHighlight::BorderNone; nBorder <= DjVuHighlight::BorderXOR; ++nBorder)
	{
		AfxExtractSubString(strPart, strBorderTypes, nBorder, ',');
		m_cboBorderType.AddString(strPart);
	}

	CString strFillTypes;
	strFillTypes.LoadString(IDS_FILL_TYPES);

	for (int nFill = DjVuHighlight::FillNone; nFill <= DjVuHighlight::FillXOR; ++nFill)
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

	return true;
}

void CHighlightDlg::OnOK()
{
	if (!UpdateData())
		return;

	CDialog::OnOK();
}

void CHighlightDlg::OnCancel()
{
	CDialog::OnCancel();
}

void CHighlightDlg::UpdateControls()
{
	m_colorBorder.EnableWindow(m_nBorderType == DjVuHighlight::BorderSolid);
	GetDlgItem(IDC_HIDE_INACTIVE)->EnableWindow(m_nBorderType != DjVuHighlight::BorderNone);

	m_colorFill.EnableWindow(m_nFillType == DjVuHighlight::FillSolid);
	m_sliderTransparency.EnableWindow(m_nFillType == DjVuHighlight::FillSolid);
	GetDlgItem(IDC_FILL_TRANSPARENCY_TEXT)->EnableWindow(m_nFillType == DjVuHighlight::FillSolid);

	GetDlgItem(IDOK)->EnableWindow(m_nBorderType != DjVuHighlight::BorderNone
			|| m_nFillType != DjVuHighlight::FillNone);
}

void CHighlightDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;
	if (pSlider == &m_sliderTransparency)
	{
		m_nTransparency = m_sliderTransparency.GetPos();
		UpdateData(false);
	}
}

void CHighlightDlg::OnChangeCombo()
{
	UpdateData();
	m_nBorderType = m_cboBorderType.GetCurSel();
	m_nFillType = m_cboFillType.GetCurSel();

	UpdateControls();
}
