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

#pragma once

struct CDisplaySettings
{
	CDisplaySettings() :
			bAdjustDisplay(false), nBrightness(0), nContrast(0),
			fGamma(1.0), bScaleColorPnm(false), bScaleSubpix(false),
			bInvertColors(false) {}

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
				&& GetGamma() == rhs.GetGamma() && bScaleColorPnm == rhs.bScaleColorPnm
				&& bScaleSubpix == rhs.bScaleSubpix && bInvertColors == rhs.bInvertColors;
	}

	bool IsAdjusted() const
	{
		return bInvertColors || bAdjustDisplay && (nBrightness != 0 || nContrast != 0 || fGamma != 1.0);
	}

	bool bAdjustDisplay;
	double fGamma;
	int nBrightness;
	int nContrast;

	bool bScaleColorPnm;
	bool bScaleSubpix;
	bool bInvertColors;
};

struct CPrintSettings
{
	CPrintSettings() :
		fMarginLeft(0.0), fMarginTop(0.0), fMarginRight(0.0), fMarginBottom(0.0),
		fPosLeft(0.0), fPosTop(0.0), bCenterImage(true), bClipContent(false),
		fScale(100.0), bShrinkOversized(true), bScaleToFit(false),
		bIgnorePrinterMargins(false), nCopies(1), bCollate(false),
		bAutoRotate(false), bLandscape(false), bTwoPages(false),
		bBooklet(false), nPaperCode(0), bAdjustPrinting(false) {}

	double fMarginLeft;
	double fMarginTop;
	double fMarginRight;
	double fMarginBottom;

	double fPosLeft;
	double fPosTop;
	BOOL bCenterImage;
	BOOL bAutoRotate;
	BOOL bClipContent;

	BOOL bScaleToFit;
	BOOL bShrinkOversized;
	BOOL bIgnorePrinterMargins;

	BOOL bAdjustPrinting;

	// The following settings are not stored in registry
	double fScale;

	DWORD nCopies;
	BOOL bCollate;

	BOOL bLandscape;
	bool bTwoPages;
	bool bBooklet;

	CString strPrinter;
	WORD nPaperCode;
};

struct CAppSettings
{
	CAppSettings();

	int nWindowPosX;
	int nWindowPosY;
	int nWindowWidth;
	int nWindowHeight;

	bool bWindowMaximized;

	bool bToolbar;
	bool bTabBar;
	bool bStatusBar;
	bool bDictBar;

	int nDefaultZoomType;
	double fDefaultZoom;
	int nDefaultLayout;
	bool bFirstPageAlone;
	int nDefaultMode;

	bool bNavPaneHidden;
	bool bNavPaneCollapsed;
	int nNavPaneWidth;

	bool bWarnNotDefaultViewer;
	bool bTopLevelDocs;
	bool bHideSingleTab;
	bool bGenAllThumbnails;
	bool bInvertWheelZoom;
	bool bCloseOnEsc;
	bool bWrapLongBookmarks;
	bool bRestoreView;
	bool bFullscreenClicks;
	bool bFullscreenHideScroll;
	bool bFullscreenContinuousScroll;

	bool bCheckUpdates;
	__int64 nLastUpdateTime;

	DWORD nLanguage;

	CString strVersion;

	enum { HistorySize = 15 };
	list<CString> searchHistory;
	CString strFind;
	bool bMatchCase;

	bool bRestoreTabs;
	int nStartupTab;
	vector<CString> openTabs;

	enum UnitType
	{
		Centimeters = 0,
		Millimeters = 1,
		Inches = 2
	};

	int nUnits;
	static double unitsPerInch[];

	int nThumbnailSize;
	enum { ThumbnailSizes = 5 };
	static int thumbnailWidth[];
	static int thumbnailHeight[];

	int nCurLang;
	int nCurDict;
	CString strDictLocation;
	int nDictChoice;

	int nActiveSettingsTab;
	int nImageFormat;
};
