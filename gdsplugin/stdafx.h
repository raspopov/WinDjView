//	Google Desktop Search DjVu Indexer Plugin
//	Copyright (C) 2005 Andrew Zhezherun
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

// stdafx.h : include file for standard system include files, or project specific
// include files that are used frequently, but are changed infrequently

#pragma once

#define STRICT

#define WINVER			0x0400
#define _WIN32_WINNT	0x0400
#define _WIN32_WINDOWS	0x0410
#define _WIN32_IE		0x0400

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off ATL's hiding of some common and often safely ignored warning messages
#define _ATL_ALL_WARNINGS

#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atltypes.h>
#include <atlctl.h>
#include <atlhost.h>
#include <atlsafe.h>

using namespace ATL;

#import "C:\Program Files\Google\Google Desktop Search\GoogleDesktopAPI2.dll" auto_rename
using namespace GoogleDesktopSearchAPILib;

#define HAS_WCTYPE
#define DJVUREFAPI

#include "../libdjvu/DjVuDocument.h"
#include "../libdjvu/DjVuFile.h"
#include "../libdjvu/DjVuImage.h"
#include "../libdjvu/IFFByteStream.h"
#include "../libdjvu/BSByteStream.h"
#include "../libdjvu/DataPool.h"
#include "../libdjvu/DjVuText.h"
#include "../libdjvu/GPixmap.h"
#include "../libdjvu/GBitmap.h"

#define FREEIMAGE_LIB
#include "FreeImage.h"
