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

#include "stdafx.h"
#include "WinDjView.h"
#include "SettingsDlg.h"

#include "AppSettings.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSettingsDlg dialog

IMPLEMENT_DYNAMIC(CSettingsDlg, CDialog)
CSettingsDlg::CSettingsDlg(CWnd* pParent)
	: CDialog(CSettingsDlg::IDD, pParent),
	  m_fGammaValue(1.0), m_strGammaValue(_T("1.0"))
{
	m_bRestoreAssocs = CAppSettings::bRestoreAssocs;
	m_bGenAllThumbnails = CAppSettings::bGenAllThumbnails;
	m_bAdjustDisplay = CAppSettings::bAdjustDisplay;
	m_fGammaValue = CAppSettings::fGamma;
	m_nBrightnessValue = CAppSettings::nBrightness;
	m_nContrastValue = CAppSettings::nContrast;
}

CSettingsDlg::~CSettingsDlg()
{
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_RESTORE_ASSOCS, m_bRestoreAssocs);
	DDX_Check(pDX, IDC_GEN_ALL_THUMBNAILS, m_bGenAllThumbnails);
	DDX_Control(pDX, IDC_STATIC_ABOUT, m_ctlAbout);

#ifndef ELIBRA_READER
	DDX_Check(pDX, IDC_ADJUST_DISPLAY, m_bAdjustDisplay);
	DDX_Control(pDX, IDC_BRIGHTNESS, m_sliderBrightness);
	DDX_Control(pDX, IDC_CONTRAST, m_sliderContrast);
	DDX_Control(pDX, IDC_GAMMA, m_sliderGamma);

	m_strBrightnessValue.Format(_T("&Brightness: %d"), m_nBrightnessValue);
	DDX_Text(pDX, IDC_BRIGHTNESS_TEXT, m_strBrightnessValue);
	m_strContrastValue.Format(_T("&Contrast: %d"), m_nContrastValue);
	DDX_Text(pDX, IDC_CONTRAST_TEXT, m_strContrastValue);
	m_strGammaValue.Format(_T("&Gamma: %1.1f"), m_fGammaValue);
	DDX_Text(pDX, IDC_GAMMA_TEXT, m_strGammaValue);

	m_sliderBrightness.EnableWindow(m_bAdjustDisplay);
	m_sliderContrast.EnableWindow(m_bAdjustDisplay);
	m_sliderGamma.EnableWindow(m_bAdjustDisplay);
	GetDlgItem(IDC_BRIGHTNESS_TEXT)->EnableWindow(m_bAdjustDisplay);
	GetDlgItem(IDC_CONTRAST_TEXT)->EnableWindow(m_bAdjustDisplay);
	GetDlgItem(IDC_GAMMA_TEXT)->EnableWindow(m_bAdjustDisplay);
#endif
}


BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
	ON_BN_CLICKED(IDC_ASSOCIATE, OnAssociate)
	ON_WM_CTLCOLOR()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_ADJUST_DISPLAY, OnAdjustDisplay)
END_MESSAGE_MAP()


// CSettingsDlg message handlers

void CSettingsDlg::OnAssociate()
{
	if (theApp.RegisterShellFileTypes())
		AfxMessageBox(IDS_ASSOCIATE_SUCCESSFUL, MB_ICONINFORMATION | MB_OK);
	else
		AfxMessageBox(IDS_ASSOCIATE_FAILED, MB_ICONERROR | MB_OK);
}

void CSettingsDlg::OnOK()
{
	if (!UpdateData())
		return;

	CAppSettings::bRestoreAssocs = !!m_bRestoreAssocs;
	CAppSettings::bGenAllThumbnails = !!m_bGenAllThumbnails;

	CAppSettings::bAdjustDisplay = !!m_bAdjustDisplay;
	CAppSettings::nBrightness = m_nBrightnessValue;
	CAppSettings::nContrast = m_nContrastValue;
	CAppSettings::fGamma = m_fGammaValue;

	CDialog::OnOK();
}

BOOL CSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFont fnt;
	CreateSystemDialogFont(fnt);
	LOGFONT lf;
	fnt.GetLogFont(&lf);

	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	lf.lfHeight = -MulDiv(7, dcScreen.GetDeviceCaps(LOGPIXELSY), 72);
	m_font.CreateFontIndirect(&lf);

	m_ctlAbout.SetFont(&m_font);

	m_sliderGamma.SetRange(1, 50);
	m_sliderGamma.SetPos(static_cast<int>(m_fGammaValue * 10));
	m_sliderGamma.SetLineSize(1);
	m_sliderGamma.SetPageSize(5);

	m_sliderGamma.ClearTics();
	m_sliderGamma.SetTic(1);
	for (int i = 1; i <= 10; ++i)
		m_sliderGamma.SetTic(5*i);

	m_sliderBrightness.SetRange(0, 200);
	m_sliderBrightness.SetPos(m_nBrightnessValue + 100);
	m_sliderBrightness.SetLineSize(1);
	m_sliderBrightness.SetPageSize(5);

	m_sliderBrightness.ClearTics();
	for (int i = 0; i <= 20; ++i)
		m_sliderBrightness.SetTic(10*i);

	m_sliderContrast.SetRange(0, 200);
	m_sliderContrast.SetPos(m_nContrastValue + 100);
	m_sliderContrast.SetLineSize(1);
	m_sliderContrast.SetPageSize(5);

	m_sliderContrast.ClearTics();
	for (int i = 0; i <= 20; ++i)
		m_sliderContrast.SetTic(10*i);

	return TRUE;
}

HBRUSH CSettingsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH brush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetSafeHwnd() == m_ctlAbout.m_hWnd)
	{
		pDC->SetTextColor(::GetSysColor(COLOR_3DSHADOW));
	}

	return brush;
}

void CSettingsDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;
	if (pSlider == &m_sliderGamma || pSlider == &m_sliderBrightness || pSlider == &m_sliderContrast)
	{
		m_nBrightnessValue = m_sliderBrightness.GetPos() - 100;
		m_nContrastValue = m_sliderContrast.GetPos() - 100;
		m_fGammaValue = m_sliderGamma.GetPos() * 0.1;

		UpdateData(false);
	}
}

void CSettingsDlg::OnAdjustDisplay()
{
	UpdateData();
}
