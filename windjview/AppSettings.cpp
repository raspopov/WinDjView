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
#include "AppSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAppSettings

double CAppSettings::unitsPerInch[] = { 2.54, 25.4, 1.0 };

int CAppSettings::thumbnailWidth[] = { 50, 71, 100, 141, 200 };
int CAppSettings::thumbnailHeight[] = { 55, 78, 110, 155, 220 };

CAppSettings::CAppSettings()
{
	nWindowPosX = 50;
	nWindowPosY = 50;
	nWindowWidth = 800;
	nWindowHeight = 600;

	bWindowMaximized = false;

	bToolbar = true;
	bTabBar = true;
	bStatusBar = true;
	bDictBar = true;

	nDefaultZoomType = -3; // Fit page
	fDefaultZoom = 100.0;
	nDefaultLayout = 1; // Continuous
	bFirstPageAlone = false;
	nDefaultMode = 0; // Drag

	bNavPaneHidden = false;
	bNavPaneCollapsed = false;
	nNavPaneWidth = 205;

	bWarnNotDefaultViewer = true;
	bTopLevelDocs = false;
	bHideSingleTab = false;
	bGenAllThumbnails = true;
	bFullscreenClicks = true;
	bFullscreenHideScroll = true;
	bFullscreenContinuousScroll = false;
	bInvertWheelZoom = false;
	bCloseOnEsc = false;
	bWrapLongBookmarks = true;
	bRestoreView = true;
	nStartupTab = 0;
	bRestoreTabs = true;

	bCheckUpdates = true;
	nLastUpdateTime = 0;

	nLanguage = 0x409;

	bMatchCase = false;

	nUnits = Centimeters;

	DWORD dwMeasureSys;
	if (::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IMEASURE | LOCALE_RETURN_NUMBER,
			(LPTSTR) &dwMeasureSys, sizeof(DWORD)))
		nUnits = (dwMeasureSys == 1 ? Inches : Centimeters);

	nThumbnailSize = 2;

	nCurLang = -1;
	nCurDict = -1;
	nDictChoice = 0;

	nActiveSettingsTab = 0;
	nImageFormat = 2;  // png
}
