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
#include "SettingsGeneralPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSettingsGeneralPage dialog

IMPLEMENT_DYNAMIC(CSettingsGeneralPage, CPropertyPage)

CSettingsGeneralPage::CSettingsGeneralPage()
	: CPropertyPage(CSettingsGeneralPage::IDD)
{
	m_bGenAllThumbnails = CAppSettings::bGenAllThumbnails;
	m_bFullscreenClicks = CAppSettings::bFullscreenClicks;
	m_bFullscreenHideScroll = CAppSettings::bFullscreenHideScroll;
	m_bWarnCloseMultiple = CAppSettings::bWarnCloseMultiple;
	m_bInvertWheelZoom = CAppSettings::bInvertWheelZoom;
	m_bCloseOnEsc = CAppSettings::bCloseOnEsc;
	m_bWrapLongBookmarks = CAppSettings::bWrapLongBookmarks;
	m_bRestoreView = CAppSettings::bRestoreView;
}

CSettingsGeneralPage::~CSettingsGeneralPage()
{
}

void CSettingsGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_GEN_ALL_THUMBNAILS, m_bGenAllThumbnails);
	DDX_Check(pDX, IDC_FULLSCREEN_CLICKS, m_bFullscreenClicks);
	DDX_Check(pDX, IDC_FULLSCREEN_HIDESCROLL, m_bFullscreenHideScroll);
	DDX_Check(pDX, IDC_WARN_CLOSE_MULTIPLE, m_bWarnCloseMultiple);
	DDX_Check(pDX, IDC_INVERT_WHEEL_ZOOM, m_bInvertWheelZoom);
	DDX_Check(pDX, IDC_CLOSE_ON_ESC, m_bCloseOnEsc);
	DDX_Check(pDX, IDC_WRAP_BOOKMARKS, m_bWrapLongBookmarks);
	DDX_Check(pDX, IDC_RESTORE_VIEW, m_bRestoreView);
}


BEGIN_MESSAGE_MAP(CSettingsGeneralPage, CPropertyPage)
END_MESSAGE_MAP()

// CSettingsGeneralPage message handlers

