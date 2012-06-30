//	WinDjView
//	Copyright (C) 2004-2012 Andrew Zhezherun
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

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define WINVER			0x0500
#define _WIN32_WINNT	0x0400
#define _WIN32_IE		0x0400

#if (_MFC_VER > 0x0600)
#define _WIN32_WINDOWS	0x0500
#endif

#ifdef NDEBUG
#define NO_DEBUG
#endif

#if (_MSC_VER < 1200)
#pragma warning(disable: 4200 4786)
#endif

// Only support UNICODE builds
#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// Turn off warnings in VC++ 2005
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
// Don's use secure standard library from VC++ 2005
#define _SECURE_SCL 0

#define _WIN32_DCOM

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS
#pragma warning(push)
#pragma warning(disable: 4244 4812)

#include <afxwin.h>
#include <afxext.h>
#include <afxcview.h>
#include <afxinet.h>
#include <afxdlgs.h>
#include <afxdtctl.h>
#include <afxcmn.h>
#include <afxmt.h>
#include <afxpriv.h>
#include <shlwapi.h>

#if (_MFC_VER > 0x0600)
#include <../src/mfc/afximpl.h>
#else
#include <../src/afximpl.h>
#endif

#pragma warning(pop)

#include <winspool.h>
#include <process.h>
#include <locale.h>
#include <string>
#include <vector>
#include <set>
#include <stack>
#include <algorithm>
#include <cmath>
#include <list>
#include <map>
#include <sstream>
using namespace std;

#define HAS_WCTYPE 1
#define THREADMODEL 0
#define DO_CHANGELOCALE 0
#define WIN32_MONITOR
#define NEED_JPEG_DECODER
#define WIN32_JPEG
#define LIBDJVU_STATIC

#include "libdjvu/DjVuDocument.h"
#include "libdjvu/DjVuImage.h"
#include "libdjvu/GBitmap.h"
#include "libdjvu/GScaler.h"
#include "libdjvu/IFFByteStream.h"
#include "libdjvu/BSByteStream.h"
#include "libdjvu/DataPool.h"
#include "libdjvu/DjVuText.h"
#include "libdjvu/DjVmNav.h"

// Application messages
#define WM_PAGE_DECODED (WM_APP + 1)
#define WM_PAGE_RENDERED (WM_APP + 2)
#define WM_THUMBNAIL_RENDERED (WM_APP + 3)
#define WM_ENDDIALOG (WM_APP + 4)
#define WM_EXPAND_NAV (WM_APP + 5)
#define WM_COLLAPSE_NAV (WM_APP + 6)
#define WM_CLICKED_NAV_TAB (WM_APP + 7)
#define WM_SHOW_SETTINGS (WM_APP + 8)
#define WM_MDI_ACTIVATE (WM_APP + 9)
#define WM_SHOWPARENT (WM_APP + 10)
#define WM_NOTIFY_NEW_VERSION (WM_APP + 11)

#ifndef DWORD_PTR
#define DWORD_PTR UINT_PTR
#endif

#ifndef IDC_HAND
#define IDC_HAND MAKEINTRESOURCE(32649)
#endif

#ifndef COLOR_HOTLIGHT
#define COLOR_HOTLIGHT 26
#endif

#ifndef BIF_USENEWUI
#define BIF_USENEWUI 0x00000050
#endif

#ifndef BS_TYPEMASK
#define BS_TYPEMASK 0x0000000F
#endif

#ifndef CSIDL_COMMON_APPDATA
#define CSIDL_COMMON_APPDATA 0x00000023
#define CSIDL_FLAG_CREATE 0x00008000
#endif

#ifndef WM_APPCOMMAND
#define WM_APPCOMMAND                0x0319
#define APPCOMMAND_BROWSER_BACKWARD       1
#define APPCOMMAND_BROWSER_FORWARD        2
#define FAPPCOMMAND_MASK             0xF000
#define GET_APPCOMMAND_LPARAM(lParam) ((short)(HIWORD(lParam) & ~FAPPCOMMAND_MASK))
#endif

#ifndef BCM_FIRST
#define BCM_FIRST 0x1600
#endif

#ifndef BCM_SETSHIELD
#define BCM_SETSHIELD (BCM_FIRST + 0x000C)
#endif

#ifndef LVS_EX_DOUBLEBUFFER
#define LVS_EX_DOUBLEBUFFER 0x00010000
#endif

#ifndef LVS_EX_LABELTIP
#define LVS_EX_LABELTIP 0x00004000
#endif

#ifndef HDF_SORTUP
#define HDF_SORTUP 0x0400
#define HDF_SORTDOWN 0x0200
#endif

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED 0x00080000
#define LWA_COLORKEY 0x00000001
#define LWA_ALPHA 0x00000002
#define ULW_COLORKEY 0x00000001
#define ULW_ALPHA 0x00000002
#define ULW_OPAQUE 0x00000004
#endif

#ifndef TBSTYLE_EX_DOUBLEBUFFER
#define TBSTYLE_EX_DOUBLEBUFFER 0x00000080
#endif
