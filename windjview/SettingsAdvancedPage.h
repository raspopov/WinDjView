//	WinDjView
//	Copyright (C) 2004-2012 Andrew Zhezherun
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


// CSettingsAdvancedPage dialog

class CSettingsAdvancedPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSettingsAdvancedPage)

public:
	CSettingsAdvancedPage();
	virtual ~CSettingsAdvancedPage();

// Dialog Data
	enum { IDD = IDD_SETTINGS_ADVANCED };
	BOOL m_bWarnNotDefaultViewer;
	BOOL m_bRestoreView;
	BOOL m_bCheckUpdates;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void UpdateButtons();

	afx_msg void OnMakeDefault();
	afx_msg void OnBackup();
	DECLARE_MESSAGE_MAP()
};
