//	WinDjView
//	Copyright (C) 2004-2005 Andrew Zhezherun
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
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//	http://www.gnu.org/copyleft/gpl.html

// $Id$

#pragma once

#include "Drawing.h"

struct CDisplaySettings
{
	CDisplaySettings() :
			bAdjustDisplay(false), nBrightness(0), nContrast(0), fGamma(1.0) {}

	int GetBrightness() const { return bAdjustDisplay ? nBrightness : 0; }
	int GetContrast() const { return bAdjustDisplay ? nContrast : 0; }
	double GetGamma() const { return bAdjustDisplay ? fGamma : 1.0; }

	bool operator!=(const CDisplaySettings& rhs) const { return !(*this == rhs); }
	bool operator==(const CDisplaySettings& rhs) const
	{
		return GetBrightness() == rhs.GetBrightness() && GetContrast() == rhs.GetContrast()
				&& GetGamma() == rhs.GetGamma();
	}

	bool bAdjustDisplay;
	double fGamma;
	int nBrightness;
	int nContrast;
};

struct CAppSettings
{
	static int nWindowPosX;
	static int nWindowPosY;
	static int nWindowWidth;
	static int nWindowHeight;

	static bool bWindowMaximized;
	static bool bChildMaximized;

	static bool bToolbar;
	static bool bStatusBar;

	static int nDefaultZoomType;
	static double fDefaultZoom;
	static int nDefaultLayout;
	static int nDefaultMode;

	static bool bNavPaneCollapsed;
	static int nNavPaneWidth;

	static bool bRestoreAssocs;
	static bool bGenAllThumbnails;
	static bool bFullscreenClicks;

	static CDisplaySettings displaySettings;

	static CPrintSettings printSettings;

	// The following settings are not stored in registry
	static int nCopies;
	static bool bCollate;

	static bool bLandscape;
	static bool bTwoPages;

	static CString strPrinter;
	static WORD nPaper;

	static CString strVersion;
};
