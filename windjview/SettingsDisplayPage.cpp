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
	m_bHQColorScaling = m_displaySettings.bScaleColorPnm;
	m_bSubpixelScaling = m_displaySettings.bScaleSubpix;
	m_bInvertColors = m_displaySettings.bInvertColors;

	m_nBrightness = m_displaySettings.nBrightness + 100;
	m_nContrast = m_displaySettings.nContrast + 100;
	m_nGamma = static_cast<int>(m_displaySettings.fGamma * 10 + 0.5);

	m_bAdjustPrinting = theApp.GetPrintSettings()->bAdjustPrinting;
	m_nUnits = theApp.GetAppSettings()->nUnits;
}

CSettingsDisplayPage::~CSettingsDisplayPage()
{
}

void CSettingsDisplayPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ADJUST_DISPLAY, m_bAdjustDisplay);
	DDX_Check(pDX, IDC_HQ_COLOR_SCALING, m_bHQColorScaling);
	DDX_Check(pDX, IDC_SUBPIXEL_SCALING, m_bSubpixelScaling);
	DDX_Check(pDX, IDC_INVERT_COLORS, m_bInvertColors);
	DDX_Check(pDX, IDC_ADJUST_PRINTING, m_bAdjustPrinting);

	DDX_Control(pDX, IDC_BRIGHTNESS, m_sliderBrightness);
	DDX_Control(pDX, IDC_CONTRAST, m_sliderContrast);
	DDX_Control(pDX, IDC_GAMMA, m_sliderGamma);

	if (m_bSlidersInitialized)
	{
		// Two strange effects here: First, negative min/max limits don't work properly
		// Second: If position is set before SetRange is called, slider is not updated
		// until position changes. That's why all this mess.

		DDX_Slider(pDX, IDC_BRIGHTNESS, m_nBrightness);
		DDX_Slider(pDX, IDC_CONTRAST, m_nContrast);
		DDX_Slider(pDX, IDC_GAMMA, m_nGamma);
	}

	DDX_Control(pDX, IDC_COMBO_UNITS, m_cboUnits);
	DDX_CBIndex(pDX, IDC_COMBO_UNITS, m_nUnits);

	CString strBrightness, strContrast, strGamma;
	strBrightness.Format(m_nBrightness == 100 ? IDS_BRIGHTNESS_TEXT_ZERO : IDS_BRIGHTNESS_TEXT, m_nBrightness - 100);
	strContrast.Format(m_nContrast == 100 ? IDS_CONTRAST_TEXT_ZERO : IDS_CONTRAST_TEXT, m_nContrast - 100);
	strGamma.Format(IDS_GAMMA_TEXT, m_nGamma / 10.0);
	DDX_Text(pDX, IDC_BRIGHTNESS_TEXT, strBrightness);
	DDX_Text(pDX, IDC_CONTRAST_TEXT, strContrast);
	DDX_Text(pDX, IDC_GAMMA_TEXT, strGamma);

	m_sliderBrightness.EnableWindow(m_bAdjustDisplay);
	m_sliderContrast.EnableWindow(m_bAdjustDisplay);
	m_sliderGamma.EnableWindow(m_bAdjustDisplay);
	GetDlgItem(IDC_BRIGHTNESS_TEXT)->EnableWindow(m_bAdjustDisplay);
	GetDlgItem(IDC_CONTRAST_TEXT)->EnableWindow(m_bAdjustDisplay);
	GetDlgItem(IDC_GAMMA_TEXT)->EnableWindow(m_bAdjustDisplay);
}


BEGIN_MESSAGE_MAP(CSettingsDisplayPage, CPropertyPage)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_ADJUST_DISPLAY, OnAdjustDisplay)
END_MESSAGE_MAP()


// CSettingsDisplayPage message handlers

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

BOOL CSettingsDisplayPage::OnKillActive()
{
	if (!UpdateData())
		return false;

	m_displaySettings.bAdjustDisplay = !!m_bAdjustDisplay;
	m_displaySettings.bScaleColorPnm = !!m_bHQColorScaling;
	m_displaySettings.bScaleSubpix = !!m_bSubpixelScaling;
	m_displaySettings.bInvertColors = !!m_bInvertColors;

	m_displaySettings.nBrightness = m_nBrightness - 100;
	m_displaySettings.nContrast = m_nContrast - 100;
	m_displaySettings.fGamma = m_nGamma / 10.0;

	return true;
}
