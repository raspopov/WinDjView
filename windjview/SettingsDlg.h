//	WinDjView
//	Copyright (C) 2004-2008 Andrew Zhezherun
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
//	51 Franklin Street, Fifth Floor, Boston, MA 02111-1307 USA.
//	http://www.gnu.org/copyleft/gpl.html

// $Id$

#pragma once

#include "AppSettings.h"
#include "SettingsGeneralPage.h"
#include "SettingsDisplayPage.h"
#include "SettingsAssocsPage.h"
#include "SettingsDictPage.h"


// CSettingsDlg dialog

class CSettingsDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(CSettingsDlg)

public:
	CSettingsDlg(CWnd* pParent = NULL);
	virtual ~CSettingsDlg();

	virtual int DoModal();

	CSettingsGeneralPage m_pageGeneral;
	CSettingsDisplayPage m_pageDisplay;
	CSettingsAssocsPage m_pageAssocs;
	CSettingsDictPage m_pageDict;

protected:
	CFont m_font;
	CStatic m_ctlAbout;

	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKickIdle();
	DECLARE_MESSAGE_MAP()
};
