//	WinDjView
//	Copyright (C) 2004 Andrew Zhezherun
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
//  http://www.gnu.org/copyleft/gpl.html

// $Id$

#include "stdafx.h"
#include "Parser.h"
#include "AppSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAppSettings

int CAppSettings::nWindowPosX = 50;
int CAppSettings::nWindowPosY = 50;
int CAppSettings::nWindowWidth = 700;
int CAppSettings::nWindowHeight = 500;

bool CAppSettings::bWindowMaximized = false;
bool CAppSettings::bChildMaximized = false;

bool CAppSettings::bToolbar = true;
bool CAppSettings::bStatusBar = true;

int CAppSettings::nDefaultZoomType = 0;
double CAppSettings::fDefaultZoom = 100.0;
int CAppSettings::nDefaultLayout = 0;

bool CAppSettings::bRestoreAssocs = false;

int CAppSettings::nCopies = 1;
bool CAppSettings::bCollate = false;

CPrintSettings CAppSettings::printSettings;

bool CAppSettings::bLandscape = false;
bool CAppSettings::bTwoPages = false;

CString CAppSettings::strPrinter;
WORD CAppSettings::nPaper = 0;
