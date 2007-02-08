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

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define _WIN32_WINNT	0x0400

#if (_MSC_VER >= 1300)
#define WINVER			0x0500
#define _WIN32_IE		0x0400
#endif

#if (_MFC_VER > 0x0600)
#define _WIN32_WINDOWS	0x0410
#endif

#if (_MSC_VER < 1300)
#pragma warning(disable: 4200 4786)
#endif

#define WM_ENDDIALOG (WM_APP+6)

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit
#define _CRT_SECURE_NO_DEPRECATE // Turn off warnings in VC++ 2005

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#define _WIN32_DCOM

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
#include <afxinet.h>
#include <afxdlgs.h>

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#if (_MFC_VER > 0x0600)
#include <../src/mfc/afximpl.h>
#else
#include <../src/afximpl.h>
#endif

#include <afxpriv.h>
#include <winspool.h>
#include <afxmt.h>

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

#ifdef NDEBUG
#define NO_DEBUG
#endif

#define HAS_WCTYPE 1
#define THREADMODEL 0
#define DO_CHANGELOCALE 0
#define WIN32_MONITOR
#define NEED_JPEG_DECODER
#define WIN32_JPEG

#include "libdjvu/DjVuDocument.h"
#include "libdjvu/DjVuImage.h"
#include "libdjvu/GBitmap.h"
#include "libdjvu/GScaler.h"
#include "libdjvu/IFFByteStream.h"
#include "libdjvu/BSByteStream.h"
#include "libdjvu/DataPool.h"
#include "libdjvu/DjVuText.h"
#include "libdjvu/DjVmNav.h"

#if (_MSC_VER < 1300)
#define DWORD_PTR UINT_PTR
#define IDC_HAND MAKEINTRESOURCE(32649)
#define COLOR_HOTLIGHT 26
#endif

inline CString LoadString(UINT nID)
{
	CString strResult;
	strResult.LoadString(nID);
	return strResult;
}

inline CString FormatString(LPCTSTR pszFormat, ...)
{
	CString strResult;

	va_list argList;
	va_start(argList, pszFormat);
	strResult.FormatV(pszFormat, argList);
	va_end(argList);

	return strResult;
}

inline CString FormatString(UINT nFormatID, ...)
{
	CString strResult, strFormat;
	VERIFY(strFormat.LoadString(nFormatID));

	va_list argList;
	va_start(argList, nFormatID);
	strResult.FormatV(strFormat, argList);
	va_end(argList);

	return strResult;
}

inline int GetTotalRotate(GP<DjVuImage> pImage, int nRotate)
{
	GP<DjVuInfo> info = pImage->get_info();
	return (nRotate + (info != NULL ? info->orientation : 0)) % 4;
}
