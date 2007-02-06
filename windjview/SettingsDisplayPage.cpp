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
#include "SettingsDisplayPage.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSettingsDisplayPage dialog

IMPLEMENT_DYNAMIC(CSettingsDisplayPage, CPropertyPage)

CSettingsDisplayPage::CSettingsDisplayPage()
	: CPropertyPage(CSettingsDisplayPage::IDD)
{
	m_displaySettings = CAppSettings::displaySettings;
	m_bAdjustDisplay = m_displaySettings.bAdjustDisplay;
	m_bHQScaling = (m_displaySettings.nScaleMethod != CDisplaySettings::Default);
	m_bInvertColors = m_displaySettings.bInvertColors;
}

CSettingsDisplayPage::~CSettingsDisplayPage()
{
}

void CSettingsDisplayPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ADJUST_DISPLAY, m_bAdjustDisplay);
	DDX_Check(pDX, IDC_HQ_SCALING, m_bHQScaling);
	DDX_Check(pDX, IDC_INVERT_COLORS, m_bInvertColors);

	DDX_Control(pDX, IDC_BRIGHTNESS, m_sliderBrightness);
	DDX_Control(pDX, IDC_CONTRAST, m_sliderContrast);
	DDX_Control(pDX, IDC_GAMMA, m_sliderGamma);

	m_strBrightnessValue.Format(m_displaySettings.nBrightness == 0 ?
		IDS_BRIGHTNESS_TEXT_ZERO : IDS_BRIGHTNESS_TEXT, m_displaySettings.nBrightness);
	DDX_Text(pDX, IDC_BRIGHTNESS_TEXT, m_strBrightnessValue);
	m_strContrastValue.Format(m_displaySettings.nContrast == 0 ?
		IDS_CONTRAST_TEXT_ZERO : IDS_CONTRAST_TEXT, m_displaySettings.nContrast);
	DDX_Text(pDX, IDC_CONTRAST_TEXT, m_strContrastValue);
	m_strGammaValue.Format(IDS_GAMMA_TEXT, m_displaySettings.fGamma);
	DDX_Text(pDX, IDC_GAMMA_TEXT, m_strGammaValue);

	m_sliderBrightness.EnableWindow(m_bAdjustDisplay);
	m_sliderContrast.EnableWindow(m_bAdjustDisplay);
	m_sliderGamma.EnableWindow(m_bAdjustDisplay);
	GetDlgItem(IDC_BRIGHTNESS_TEXT)->EnableWindow(m_bAdjustDisplay);
	GetDlgItem(IDC_CONTRAST_TEXT)->EnableWindow(m_bAdjustDisplay);
	GetDlgItem(IDC_GAMMA_TEXT)->EnableWindow(m_bAdjustDisplay);

	m_displaySettings.bAdjustDisplay = !!m_bAdjustDisplay;
	m_displaySettings.nScaleMethod = (m_bHQScaling ?
			CDisplaySettings::PnmScaleFixed : CDisplaySettings::Default);
	m_displaySettings.bInvertColors = !!m_bInvertColors;
}


BEGIN_MESSAGE_MAP(CSettingsDisplayPage, CPropertyPage)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_ADJUST_DISPLAY, OnAdjustDisplay)
END_MESSAGE_MAP()


// CSettingsDlg message handlers

BOOL CSettingsDisplayPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_sliderGamma.SetRange(1, 50);
	m_sliderGamma.SetPos(static_cast<int>(m_displaySettings.fGamma * 10));
	m_sliderGamma.SetLineSize(1);
	m_sliderGamma.SetPageSize(5);

	m_sliderGamma.ClearTics();
	m_sliderGamma.SetTic(1);
	for (int i = 1; i <= 10; ++i)
		m_sliderGamma.SetTic(5*i);

	m_sliderBrightness.SetRange(0, 200);
	m_sliderBrightness.SetPos(m_displaySettings.nBrightness + 100);
	m_sliderBrightness.SetLineSize(1);
	m_sliderBrightness.SetPageSize(5);

	m_sliderBrightness.ClearTics();
	for (int j = 0; j <= 10; ++j)
		m_sliderBrightness.SetTic(20*j);

	m_sliderContrast.SetRange(0, 200);
	m_sliderContrast.SetPos(m_displaySettings.nContrast + 100);
	m_sliderContrast.SetLineSize(1);
	m_sliderContrast.SetPageSize(5);

	m_sliderContrast.ClearTics();
	for (int k = 0; k <= 10; ++k)
		m_sliderContrast.SetTic(20*k);

	return true;
}

void CSettingsDisplayPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;
	if (pSlider == &m_sliderGamma || pSlider == &m_sliderBrightness || pSlider == &m_sliderContrast)
	{
		m_displaySettings.nBrightness = m_sliderBrightness.GetPos() - 100;
		m_displaySettings.nContrast = m_sliderContrast.GetPos() - 100;
		m_displaySettings.fGamma = m_sliderGamma.GetPos() * 0.1;

		UpdateData(false);
	}
}

void CSettingsDisplayPage::OnAdjustDisplay()
{
	UpdateData();
}
