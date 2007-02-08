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

struct CDisplaySettings
{
	enum ScaleMethod
	{
		Default,
		PnmScaleFixed
	};

	CDisplaySettings() :
			bAdjustDisplay(false), nBrightness(0), nContrast(0),
			fGamma(1.0), nScaleMethod(Default), bInvertColors(false) {}

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
				&& GetGamma() == rhs.GetGamma() && nScaleMethod == rhs.nScaleMethod
				&& bInvertColors == rhs.bInvertColors;
	}

	bool IsAdjusted() const
	{
		return bInvertColors || bAdjustDisplay && (nBrightness != 0 || nContrast != 0 || fGamma != 1.0);
	}

	bool bAdjustDisplay;
	double fGamma;
	int nBrightness;
	int nContrast;

	int nScaleMethod;
	bool bInvertColors;
};

struct CPrintSettings
{
	CPrintSettings() :
		fMarginLeft(0.0), fMarginTop(0.0), fMarginRight(0.0), fMarginBottom(0.0),
		fPosLeft(0.0), fPosTop(0.0), bCenterImage(true), bClipContent(false),
		fScale(100.0), bShrinkOversized(true), bScaleToFit(false),
		bIgnorePrinterMargins(false), nCopies(1), bCollate(false),
		bLandscape(false), bTwoPages(false), nPaperCode(0) {}

	double fMarginLeft;
	double fMarginTop;
	double fMarginRight;
	double fMarginBottom;

	double fPosLeft;
	double fPosTop;
	BOOL bCenterImage;
	BOOL bClipContent;

	double fScale;
	BOOL bShrinkOversized;
	BOOL bScaleToFit;
	BOOL bIgnorePrinterMargins;

	// The following settings are not stored in registry
	DWORD nCopies;
	BOOL bCollate;

	BOOL bLandscape;
	BOOL bTwoPages;

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
	bool bChildMaximized;

	bool bToolbar;
	bool bStatusBar;

	int nDefaultZoomType;
	double fDefaultZoom;
	int nDefaultLayout;
	bool bFirstPageAlone;
	int nDefaultMode;

	bool bNavPaneCollapsed;
	int nNavPaneWidth;

	bool bRestoreAssocs;
	bool bGenAllThumbnails;
	bool bFullscreenClicks;
	bool bFullscreenHideScroll;
	bool bWarnCloseMultiple;
	bool bInvertWheelZoom;
	bool bCloseOnEsc;
	bool bWrapLongBookmarks;
	bool bRestoreView;

	DWORD nLanguage;

	CString strVersion;

	bool bLocalized;
	HMENU hDjVuMenu;
	HMENU hDefaultMenu;

	CString strFind;
	bool bMatchCase;
};
