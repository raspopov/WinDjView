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
	BOOL m_bHQScaling;
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
