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
#include "AppSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAppSettings

double CAppSettings::unitsPerInch[] = { 2.54, 25.4, 1.0 };

CAppSettings::CAppSettings()
{
	nWindowPosX = 50;
	nWindowPosY = 50;
	nWindowWidth = 700;
	nWindowHeight = 500;

	bWindowMaximized = false;
	bChildMaximized = true;

	bToolbar = true;
	bStatusBar = true;

	nDefaultZoomType = -3; // Fit page
	fDefaultZoom = 100.0;
	nDefaultLayout = 1; // Continuous
	bFirstPageAlone = true;
	nDefaultMode = 0; // Drag

	bNavPaneCollapsed = false;
	nNavPaneWidth = 200;

	bRestoreAssocs = false;
	bGenAllThumbnails = true;
	bFullscreenClicks = true;
	bFullscreenHideScroll = true;
	bWarnCloseMultiple = false;
	bInvertWheelZoom = false;
	bCloseOnEsc = false;
	bWrapLongBookmarks = true;
	bRestoreView = true;

	nLanguage = 0x409;

	bLocalized = false;
	hDjVuMenu = NULL;
	hDefaultMenu = NULL;

	bMatchCase = false;

	nUnits = Centimeters;

	DWORD dwMeasureSys;
	if (::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IMEASURE | LOCALE_RETURN_NUMBER,
			(LPTSTR) &dwMeasureSys, sizeof(DWORD)))
		nUnits = (dwMeasureSys == 1 ? Inches : Centimeters);
}
