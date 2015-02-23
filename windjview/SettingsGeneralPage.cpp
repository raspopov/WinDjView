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
#include "SettingsGeneralPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSettingsGeneralPage dialog

IMPLEMENT_DYNAMIC(CSettingsGeneralPage, CPropertyPage)

CSettingsGeneralPage::CSettingsGeneralPage()
	: CPropertyPage(CSettingsGeneralPage::IDD)
{
	const CAppSettings& appSettings = *theApp.GetAppSettings();

	m_bTopLevelDocs = appSettings.bTopLevelDocs;
	m_bRestoreTabs = appSettings.bRestoreTabs;
	m_bHideSingleTab = appSettings.bHideSingleTab;
	m_bGenAllThumbnails = appSettings.bGenAllThumbnails;
	m_bInvertWheelZoom = appSettings.bInvertWheelZoom;
	m_bCloseOnEsc = appSettings.bCloseOnEsc;
	m_bWrapLongBookmarks = appSettings.bWrapLongBookmarks;
	m_bFullscreenClicks = appSettings.bFullscreenClicks;
	m_bFullscreenHideScroll = appSettings.bFullscreenHideScroll;
	m_bFullscreenContinuousScroll = appSettings.bFullscreenContinuousScroll;
}

CSettingsGeneralPage::~CSettingsGeneralPage()
{
}

void CSettingsGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_TOP_LEVEL_DOCUMENTS, m_bTopLevelDocs);
	DDX_Check(pDX, IDC_RESTORE_TABS, m_bRestoreTabs);
	DDX_Check(pDX, IDC_HIDE_SINGLE_TAB, m_bHideSingleTab);
	DDX_Check(pDX, IDC_GEN_ALL_THUMBNAILS, m_bGenAllThumbnails);
	DDX_Check(pDX, IDC_INVERT_WHEEL_ZOOM, m_bInvertWheelZoom);
	DDX_Check(pDX, IDC_CLOSE_ON_ESC, m_bCloseOnEsc);
	DDX_Check(pDX, IDC_WRAP_BOOKMARKS, m_bWrapLongBookmarks);
	DDX_Check(pDX, IDC_FULLSCREEN_CLICKS, m_bFullscreenClicks);
	DDX_Check(pDX, IDC_FULLSCREEN_HIDESCROLL, m_bFullscreenHideScroll);
	DDX_Check(pDX, IDC_FULLSCREEN_CONTINUOUS, m_bFullscreenContinuousScroll);

	GetDlgItem(IDC_RESTORE_TABS)->EnableWindow(!m_bTopLevelDocs);
}


BEGIN_MESSAGE_MAP(CSettingsGeneralPage, CPropertyPage)
	ON_BN_CLICKED(IDC_TOP_LEVEL_DOCUMENTS, OnTopLevelDocs)
END_MESSAGE_MAP()

// CSettingsGeneralPage message handlers

void CSettingsGeneralPage::OnTopLevelDocs()
{
	UpdateData();
}
