//	WinDjView
//	Copyright (C) 2004-2006 Andrew Zhezherun
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

#include "stdafx.h"
#include "MyTheme.h"

typedef HTHEME (__stdcall *pfOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT (__stdcall *pfCloseThemeData)(HTHEME hTheme);
typedef HRESULT (__stdcall *pfDrawThemeBackground)(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, const RECT* pRect, const RECT* pClipRect);
typedef HRESULT (__stdcall *pfDrawThemeText)(HTHEME hTheme, HDC hdc, int iPartId,
    int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags,
    DWORD dwTextFlags2, const RECT* pRect);
typedef HRESULT (__stdcall *pfGetThemeBackgroundContentRect)(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId,  const RECT* pBoundingRect,
    OUT RECT* pContentRect);
typedef HRESULT (__stdcall *pfGetThemeBackgroundExtent)(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, const RECT* pContentRect,
    OUT RECT* pExtentRect);
typedef HRESULT (__stdcall *pfGetThemePartSize)(HTHEME hTheme, HDC hdc, int iPartId,
    int iStateId, RECT* prc, THEMESIZE eSize, OUT SIZE* psz);
typedef HRESULT (__stdcall *pfGetThemeTextExtent)(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, LPCWSTR pszText, int iCharCount,
    DWORD dwTextFlags, const RECT* pBoundingRect,
    OUT RECT* pExtentRect);
typedef HRESULT (__stdcall *pfGetThemeTextMetrics)(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, OUT TEXTMETRIC* ptm);
typedef HRESULT (__stdcall *pfGetThemeBackgroundRegion)(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, const RECT* pRect, OUT HRGN* pRegion);
typedef HRESULT (__stdcall *pfHitTestThemeBackground)(HTHEME hTheme, HDC hdc, int iPartId,
    int iStateId, DWORD dwOptions, const RECT* pRect, HRGN hrgn,
    POINT ptTest, OUT WORD* pwHitTestCode);
typedef HRESULT (__stdcall *pfDrawThemeEdge)(HTHEME hTheme, HDC hdc, int iPartId,
	int iStateId, const RECT* pDestRect, UINT uEdge, UINT uFlags, OUT RECT* pContentRect);
typedef HRESULT (__stdcall *pfDrawThemeIcon)(HTHEME hTheme, HDC hdc, int iPartId,
    int iStateId, const RECT* pRect, HIMAGELIST himl, int iImageIndex);
typedef BOOL (__stdcall *pfIsThemePartDefined)(HTHEME hTheme, int iPartId,
    int iStateId);
typedef BOOL (__stdcall *pfIsThemeBackgroundPartiallyTransparent)(HTHEME hTheme,
    int iPartId, int iStateId);
typedef HRESULT (__stdcall *pfGetThemeColor)(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT COLORREF* pColor);
typedef HRESULT (__stdcall *pfGetThemeMetric)(HTHEME hTheme, HDC hdc, int iPartId,
    int iStateId, int iPropId, OUT int* piVal);
typedef HRESULT (__stdcall *pfGetThemeString)(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT LPWSTR pszBuff, int cchMaxBuffChars);
typedef HRESULT (__stdcall *pfGetThemeBool)(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT BOOL* pfVal);
typedef HRESULT (__stdcall *pfGetThemeInt)(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT int* piVal);
typedef HRESULT (__stdcall *pfGetThemeEnumValue)(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT int* piVal);
typedef HRESULT (__stdcall *pfGetThemePosition)(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT POINT* pPoint);
typedef HRESULT (__stdcall *pfGetThemeFont)(HTHEME hTheme, HDC hdc, int iPartId,
    int iStateId, int iPropId, OUT LOGFONT* pFont);
typedef HRESULT (__stdcall *pfGetThemeRect)(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT RECT* pRect);
typedef HRESULT (__stdcall *pfGetThemeMargins)(HTHEME hTheme, HDC hdc, int iPartId,
    int iStateId, int iPropId, RECT* prc, OUT MARGINS* pMargins);
typedef HRESULT (__stdcall *pfGetThemeIntList)(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT INTLIST* pIntList);
typedef HRESULT (__stdcall *pfGetThemePropertyOrigin)(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT PROPERTYORIGIN* pOrigin);
typedef HRESULT (__stdcall *pfSetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName,
    LPCWSTR pszSubIdList);
typedef HRESULT (__stdcall *pfGetThemeFilename)(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT LPWSTR pszThemeFileName, int cchMaxBuffChars);
typedef COLORREF (__stdcall *pfGetThemeSysColor)(HTHEME hTheme, int iColorId);
typedef HBRUSH (__stdcall *pfGetThemeSysColorBrush)(HTHEME hTheme, int iColorId);
typedef BOOL (__stdcall *pfGetThemeSysBool)(HTHEME hTheme, int iBoolId);
typedef int (__stdcall *pfGetThemeSysSize)(HTHEME hTheme, int iSizeId);
typedef HRESULT (__stdcall *pfGetThemeSysFont)(HTHEME hTheme, int iFontId, OUT LOGFONT* plf);
typedef HRESULT (__stdcall *pfGetThemeSysString)(HTHEME hTheme, int iStringId,
    OUT LPWSTR pszStringBuff, int cchMaxStringChars);
typedef HRESULT (__stdcall *pfGetThemeSysInt)(HTHEME hTheme, int iIntId, int* piValue);
typedef BOOL (__stdcall *pfIsThemeActive)();
typedef BOOL (__stdcall *pfIsAppThemed)();
typedef HTHEME (__stdcall *pfGetWindowTheme)(HWND hwnd);
typedef HRESULT (__stdcall *pfEnableThemeDialogTexture)(HWND hwnd, DWORD dwFlags);
typedef BOOL (__stdcall *pfIsThemeDialogTextureEnabled)(HWND hwnd);
typedef DWORD (__stdcall *pfGetThemeAppProperties)();
typedef void (__stdcall *pfSetThemeAppProperties)(DWORD dwFlags);
typedef HRESULT (__stdcall *pfGetCurrentThemeName)(
    OUT LPWSTR pszThemeFileName, int cchMaxNameChars,
    OUT LPWSTR pszColorBuff, int cchMaxColorChars,
    OUT LPWSTR pszSizeBuff, int cchMaxSizeChars);
typedef HRESULT (__stdcall *pfGetThemeDocumentationProperty)(LPCWSTR pszThemeName,
    LPCWSTR pszPropertyName, OUT LPWSTR pszValueBuff, int cchMaxValChars);
typedef HRESULT (__stdcall *pfDrawThemeParentBackground)(HWND hwnd, HDC hdc, RECT* prc);
typedef HRESULT (__stdcall *pfEnableTheming)(BOOL fEnable);
typedef HRESULT (__stdcall *pfDrawThemeBackgroundEx)(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, const RECT* pRect, const DTBGOPTS* pOptions);

pfOpenThemeData pOpenThemeData = NULL;
pfCloseThemeData pCloseThemeData = NULL;
pfDrawThemeBackground pDrawThemeBackground = NULL;
pfDrawThemeText pDrawThemeText = NULL;
pfGetThemeBackgroundContentRect pGetThemeBackgroundContentRect = NULL;
pfGetThemeBackgroundExtent pGetThemeBackgroundExtent = NULL;
pfGetThemePartSize pGetThemePartSize = NULL;
pfGetThemeTextExtent pGetThemeTextExtent = NULL;
pfGetThemeTextMetrics pGetThemeTextMetrics = NULL;
pfGetThemeBackgroundRegion pGetThemeBackgroundRegion = NULL;
pfHitTestThemeBackground pHitTestThemeBackground = NULL;
pfDrawThemeEdge pDrawThemeEdge = NULL;
pfDrawThemeIcon pDrawThemeIcon = NULL;
pfIsThemePartDefined pIsThemePartDefined = NULL;
pfIsThemeBackgroundPartiallyTransparent pIsThemeBackgroundPartiallyTransparent = NULL;
pfGetThemeColor pGetThemeColor = NULL;
pfGetThemeMetric pGetThemeMetric = NULL;
pfGetThemeString pGetThemeString = NULL;
pfGetThemeBool pGetThemeBool = NULL;
pfGetThemeInt pGetThemeInt = NULL;
pfGetThemeEnumValue pGetThemeEnumValue = NULL;
pfGetThemePosition pGetThemePosition = NULL;
pfGetThemeFont pGetThemeFont = NULL;
pfGetThemeRect pGetThemeRect = NULL;
pfGetThemeMargins pGetThemeMargins = NULL;
pfGetThemeIntList pGetThemeIntList = NULL;
pfGetThemePropertyOrigin pGetThemePropertyOrigin = NULL;
pfSetWindowTheme pSetWindowTheme = NULL;
pfGetThemeFilename pGetThemeFilename = NULL;
pfGetThemeSysColor pGetThemeSysColor = NULL;
pfGetThemeSysColorBrush pGetThemeSysColorBrush = NULL;
pfGetThemeSysBool pGetThemeSysBool = NULL;
pfGetThemeSysSize pGetThemeSysSize = NULL;
pfGetThemeSysFont pGetThemeSysFont = NULL;
pfGetThemeSysString pGetThemeSysString = NULL;
pfGetThemeSysInt pGetThemeSysInt = NULL;
pfIsThemeActive pIsThemeActive = NULL;
pfIsAppThemed pIsAppThemed = NULL;
pfGetWindowTheme pGetWindowTheme = NULL;
pfEnableThemeDialogTexture pEnableThemeDialogTexture = NULL;
pfIsThemeDialogTextureEnabled pIsThemeDialogTextureEnabled = NULL;
pfGetThemeAppProperties pGetThemeAppProperties = NULL;
pfSetThemeAppProperties pSetThemeAppProperties = NULL;
pfGetCurrentThemeName pGetCurrentThemeName = NULL;
pfGetThemeDocumentationProperty pGetThemeDocumentationProperty = NULL;
pfDrawThemeParentBackground pDrawThemeParentBackground = NULL;
pfEnableTheming pEnableTheming = NULL;
pfDrawThemeBackgroundEx pDrawThemeBackgroundEx = NULL;

HMODULE hThemesDLL = NULL;
bool bThemeAPILoaded = false;

void LoadThemeAPI()
{
	hThemesDLL = ::LoadLibrary(_T("uxtheme.dll"));
	if (hThemesDLL)
	{
		pOpenThemeData = (pfOpenThemeData)::GetProcAddress(hThemesDLL, "OpenThemeData");
		pCloseThemeData = (pfCloseThemeData)::GetProcAddress(hThemesDLL, "CloseThemeData");
		pDrawThemeBackground = (pfDrawThemeBackground)::GetProcAddress(hThemesDLL, "DrawThemeBackground");
		pDrawThemeText = (pfDrawThemeText)::GetProcAddress(hThemesDLL, "DrawThemeText");
		pIsThemeBackgroundPartiallyTransparent = (pfIsThemeBackgroundPartiallyTransparent)::GetProcAddress(hThemesDLL, "IsThemeBackgroundPartiallyTransparent");
		pSetWindowTheme = (pfSetWindowTheme)::GetProcAddress(hThemesDLL, "SetWindowTheme");
		pIsThemeActive = (pfIsThemeActive)::GetProcAddress(hThemesDLL, "IsThemeActive");
		pIsAppThemed = (pfIsAppThemed)::GetProcAddress(hThemesDLL, "IsAppThemed");
		pDrawThemeParentBackground = (pfDrawThemeParentBackground)::GetProcAddress(hThemesDLL, "DrawThemeParentBackground");
/*
		pfGetThemeBackgroundContentRect pGetThemeBackgroundContentRect = NULL;
		pfGetThemeBackgroundExtent pGetThemeBackgroundExtent = NULL;
		pfGetThemePartSize pGetThemePartSize = NULL;
		pfGetThemeTextExtent pGetThemeTextExtent = NULL;
		pfGetThemeTextMetrics pGetThemeTextMetrics = NULL;
		pfGetThemeBackgroundRegion pGetThemeBackgroundRegion = NULL;
		pfHitTestThemeBackground pHitTestThemeBackground = NULL;
		pfDrawThemeEdge pDrawThemeEdge = NULL;
		pfDrawThemeIcon pDrawThemeIcon = NULL;
		pfIsThemePartDefined pIsThemePartDefined = NULL;
		pfGetThemeColor pGetThemeColor = NULL;
		pfGetThemeMetric pGetThemeMetric = NULL;
		pfGetThemeString pGetThemeString = NULL;
		pfGetThemeBool pGetThemeBool = NULL;
		pfGetThemeInt pGetThemeInt = NULL;
		pfGetThemeEnumValue pGetThemeEnumValue = NULL;
		pfGetThemePosition pGetThemePosition = NULL;
		pfGetThemeFont pGetThemeFont = NULL;
		pfGetThemeRect pGetThemeRect = NULL;
		pfGetThemeMargins pGetThemeMargins = NULL;
		pfGetThemeIntList pGetThemeIntList = NULL;
		pfGetThemePropertyOrigin pGetThemePropertyOrigin = NULL;
		pfGetThemeFilename pGetThemeFilename = NULL;
		pfGetThemeSysColor pGetThemeSysColor = NULL;
		pfGetThemeSysColorBrush pGetThemeSysColorBrush = NULL;
		pfGetThemeSysBool pGetThemeSysBool = NULL;
		pfGetThemeSysSize pGetThemeSysSize = NULL;
		pfGetThemeSysFont pGetThemeSysFont = NULL;
		pfGetThemeSysString pGetThemeSysString = NULL;
		pfGetThemeSysInt pGetThemeSysInt = NULL;
		pfGetWindowTheme pGetWindowTheme = NULL;
		pfEnableThemeDialogTexture pEnableThemeDialogTexture = NULL;
		pfIsThemeDialogTextureEnabled pIsThemeDialogTextureEnabled = NULL;
		pfGetThemeAppProperties pGetThemeAppProperties = NULL;
		pfSetThemeAppProperties pSetThemeAppProperties = NULL;
		pfGetCurrentThemeName pGetCurrentThemeName = NULL;
		pfGetThemeDocumentationProperty pGetThemeDocumentationProperty = NULL;
		pfEnableTheming pEnableTheming = NULL;
		pfDrawThemeBackgroundEx pDrawThemeBackgroundEx = NULL;
*/	
	}

	if (pOpenThemeData != NULL
		&& pCloseThemeData != NULL
		&& pDrawThemeBackground != NULL
		&& pDrawThemeText != NULL
		&& pIsThemeBackgroundPartiallyTransparent != NULL
		&& pSetWindowTheme != NULL
		&& pIsThemeActive != NULL
		&& pIsAppThemed != NULL
		&& pDrawThemeParentBackground != NULL)
	{
		bThemeAPILoaded = true;
	}
	else
	{
		::FreeLibrary(hThemesDLL);
		hThemesDLL = NULL;
	}
}

void UnloadThemeAPI()
{
	if (hThemesDLL != NULL)
	{
		::FreeLibrary(hThemesDLL);
		hThemesDLL = NULL;
	}

	bThemeAPILoaded = false;
}

HTHEME XPOpenThemeData(HWND hwnd, LPCWSTR pszClassList)
{
	if (!bThemeAPILoaded)
		return NULL;

	return pOpenThemeData(hwnd, pszClassList);
}

HRESULT XPCloseThemeData(HTHEME hTheme)
{
	if (!bThemeAPILoaded)
		return E_FAIL;

	return pCloseThemeData(hTheme);
}

HRESULT XPDrawThemeBackground(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, const RECT* pRect, const RECT* pClipRect)
{
	if (!bThemeAPILoaded)
		return E_FAIL;

	return pDrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}

HRESULT XPDrawThemeText(HTHEME hTheme, HDC hdc, int iPartId,
    int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags,
    DWORD dwTextFlags2, const RECT* pRect)
{
	if (!bThemeAPILoaded)
		return E_FAIL;

	return pDrawThemeText(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, dwTextFlags2, pRect);
}

BOOL XPIsThemeBackgroundPartiallyTransparent(HTHEME hTheme, int iPartId, int iStateId)
{
	if (!bThemeAPILoaded)
		return false;

	return pIsThemeBackgroundPartiallyTransparent(hTheme, iPartId, iStateId);
}

HRESULT XPSetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList)
{
	if (!bThemeAPILoaded)
		return E_FAIL;

	return pSetWindowTheme(hwnd, pszSubAppName, pszSubIdList);
}

BOOL XPIsThemeActive()
{
	if (!bThemeAPILoaded)
		return false;

	return pIsThemeActive();
}

BOOL XPIsAppThemed()
{
	if (!bThemeAPILoaded)
		return false;

	return pIsAppThemed();
}

HRESULT XPDrawThemeParentBackground(HWND hwnd, HDC hdc, RECT* prc)
{
	if (!bThemeAPILoaded)
		return E_FAIL;

	return pDrawThemeParentBackground(hwnd, hdc, prc);
}

struct XPThemeAPI
{
	XPThemeAPI() { LoadThemeAPI(); }
	~XPThemeAPI() { UnloadThemeAPI(); }
	static XPThemeAPI theAPI;
};

XPThemeAPI XPThemeAPI::theAPI;