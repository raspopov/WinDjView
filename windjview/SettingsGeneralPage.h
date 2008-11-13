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


// CSettingsGeneralPage dialog

class CSettingsGeneralPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSettingsGeneralPage)

public:
	CSettingsGeneralPage();
	virtual ~CSettingsGeneralPage();

// Dialog Data
	enum { IDD = IDD_SETTINGS_GENERAL };
	BOOL m_bTopLevelDocs;
	BOOL m_bGenAllThumbnails;
	BOOL m_bFullscreenClicks;
	BOOL m_bFullscreenHideScroll;
	BOOL m_bWarnCloseMultiple;
	BOOL m_bInvertWheelZoom;
	BOOL m_bCloseOnEsc;
	BOOL m_bWrapLongBookmarks;
	BOOL m_bRestoreView;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
};
