//	DjVu Bookmark Tool
//	Copyright (C) 2005-2007 Andrew Zhezherun
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

#define _SECURE_ATL 1
#define STRICT

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// Turn off warnings in VC++ 2005
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <sdkddkver.h>

#include <afxwin.h>
#include <afxext.h>
#include <afxdisp.h>
#include <afxole.h>
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

#include <locale.h>
#include <locale>
#include <vector>
#include <stack>
#include <fstream>
#include <string>
#include <set>
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
#define LIBDJVU_STATIC

#include "../libdjvu/DjVuDocument.h"
#include "../libdjvu/DjVuImage.h"
#include "../libdjvu/DjVmNav.h"
#include "../libdjvu/DjVuText.h"
#include "../libdjvu/DataPool.h"
#include "../libdjvu/ByteStream.h"
#include "../libdjvu/IFFByteStream.h"

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
