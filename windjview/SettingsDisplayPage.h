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

#include "AppSettings.h"


// CSettingsDisplayPage dialog

class CSettingsDisplayPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSettingsDisplayPage)

public:
	CSettingsDisplayPage();
	virtual ~CSettingsDisplayPage();

// Dialog Data
	enum { IDD = IDD_SETTINGS_DISPLAY };
	BOOL m_bHQColorScaling;
	BOOL m_bSubpixelScaling;
	BOOL m_bInvertColors;
	BOOL m_bAdjustDisplay;
	BOOL m_bAdjustPrinting;
	CSliderCtrl m_sliderBrightness;
	CSliderCtrl m_sliderContrast;
	CSliderCtrl m_sliderGamma;
	int m_nBrightness;
	int m_nContrast;
	int m_nGamma;
	int m_nUnits;
	CComboBox m_cboUnits;

	CDisplaySettings m_displaySettings;

protected:
	bool m_bSlidersInitialized;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnKillActive();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnAdjustDisplay();
	DECLARE_MESSAGE_MAP()
};
