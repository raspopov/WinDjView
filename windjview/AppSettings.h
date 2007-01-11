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

#include "Drawing.h"

struct CDisplaySettings
{
	enum ScaleMethod
	{
		Default,
		PnmScaleFixed
	};

	CDisplaySettings() :
			bAdjustDisplay(false), nBrightness(0), nContrast(0),
			fGamma(1.0), nScaleMethod(Default) {}

	int GetBrightness() const { return bAdjustDisplay ? nBrightness : 0; }
	int GetContrast() const { return bAdjustDisplay ? nContrast : 0; }
	double GetGamma() const { return bAdjustDisplay ? fGamma : 1.0; }

	void Fix()
	{
		nBrightness = (nBrightness < -100 || nBrightness > 100 ? 0 : nBrightness);
		nContrast = (nContrast < -100 || nContrast > 100 ? 0 : nContrast);
		fGamma = (fGamma < 0.1 || fGamma > 5.0 ? 1.0 : fGamma);
	}

	bool operator!=(const CDisplaySettings& rhs) const { return !(*this == rhs); }
	bool operator==(const CDisplaySettings& rhs) const
	{
		return GetBrightness() == rhs.GetBrightness() && GetContrast() == rhs.GetContrast()
				&& GetGamma() == rhs.GetGamma() && nScaleMethod == rhs.nScaleMethod;
	}

	bool IsAdjusted() const
	{
		return bAdjustDisplay && (nBrightness != 0 || nContrast != 0 || fGamma != 1.0);
	}

	bool bAdjustDisplay;
	double fGamma;
	int nBrightness;
	int nContrast;

	int nScaleMethod;
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
	static bool bFirstPageAlone;
	static int nDefaultMode;

	static bool bNavPaneCollapsed;
	static int nNavPaneWidth;

	static bool bRestoreAssocs;
	static bool bGenAllThumbnails;
	static bool bFullscreenClicks;
	static bool bFullscreenHideScroll;
	static bool bWarnCloseMultiple;
	static bool bInvertWheelZoom;
	static bool bCloseOnEsc;
	static bool bWrapLongBookmarks;

	static CString strLanguage;

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

	static bool bLocalized;
	static HMENU hDjVuMenu;
	static HMENU hDefaultMenu;

	static CString strFind;
	static bool bMatchCase;
};
