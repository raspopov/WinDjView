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
	: CPropertyPage(CSettingsDisplayPage::IDD), m_bSlidersInitialized(false)
{
	m_displaySettings = *theApp.GetDisplaySettings();
	m_bAdjustDisplay = m_displaySettings.bAdjustDisplay;
	m_bHQScaling = (m_displaySettings.nScaleMethod != CDisplaySettings::Default);
	m_bInvertColors = m_displaySettings.bInvertColors;

	m_nUnits = theApp.GetAppSettings()->nUnits;
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

	if (m_bSlidersInitialized)
	{
		// Two strange effects here: First, negative min/max limits don't work properly
		// Second: If position is set before SetRange is called, slider is not updated
		// until position changes. That's why all this mess.

		int nBrightness = m_displaySettings.nBrightness + 100;
		int nContrast = m_displaySettings.nContrast + 100;
		int nGamma = static_cast<int>(m_displaySettings.fGamma * 10 + 0.5);
		DDX_Slider(pDX, IDC_BRIGHTNESS, nBrightness);
		DDX_Slider(pDX, IDC_CONTRAST, nContrast);
		DDX_Slider(pDX, IDC_GAMMA, nGamma);
		if (pDX->m_bSaveAndValidate)
		{
			m_displaySettings.nBrightness = nBrightness - 100;
			m_displaySettings.nContrast = nContrast - 100;
			m_displaySettings.fGamma = nGamma / 10.0;
		}
	}

	DDX_Control(pDX, IDC_COMBO_UNITS, m_cboUnits);
	DDX_CBIndex(pDX, IDC_COMBO_UNITS, m_nUnits);

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

	m_sliderBrightness.SetRange(0, 200);
	m_sliderBrightness.SetLineSize(1);
	m_sliderBrightness.SetPageSize(5);

	m_sliderContrast.SetRange(0, 200);
	m_sliderContrast.SetLineSize(1);
	m_sliderContrast.SetPageSize(5);

	m_sliderBrightness.ClearTics();
	m_sliderContrast.ClearTics();
	for (int j = 0; j <= 10; ++j)
	{
		m_sliderBrightness.SetTic(20*j);
		m_sliderContrast.SetTic(20*j);
	}

	m_sliderGamma.SetRange(1, 50);
	m_sliderGamma.SetLineSize(1);
	m_sliderGamma.SetPageSize(5);

	m_sliderGamma.ClearTics();
	m_sliderGamma.SetTic(1);
	for (int i = 1; i <= 10; ++i)
		m_sliderGamma.SetTic(5*i);

	CString strAllUnits;
	strAllUnits.LoadString(IDS_UNITS_FULL);

	for (int nUnits = CAppSettings::Centimeters; nUnits <= CAppSettings::Inches; ++nUnits)
	{
		CString strUnits;
		AfxExtractSubString(strUnits, strAllUnits, nUnits, ',');
		m_cboUnits.AddString(strUnits);
	}

	m_bSlidersInitialized = true;
	UpdateData(false);

	return true;
}

void CSettingsDisplayPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UpdateData();

	// Update static text
	UpdateData(false);
}

void CSettingsDisplayPage::OnAdjustDisplay()
{
	UpdateData();
}
