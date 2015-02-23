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

//////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation. All rights reserved.
// Windows XP theme API

#pragma once
#if 0
#pragma pack(push,4)
#ifndef THEMEAPI

typedef HANDLE HTHEME;

#define DTT_GRAYED 0x1 // draw a grayed-out string

enum THEMESIZE
{
    TS_MIN,             // minimum size
    TS_TRUE,            // size without stretching
    TS_DRAW,            // size that theme mgr will use to draw part
};

#define HTTB_BACKGROUNDSEG          0x0000
#define HTTB_FIXEDBORDER            0x0002  // Return code may be either HTCLIENT or HTBORDER.
#define HTTB_CAPTION                0x0004
#define HTTB_RESIZINGBORDER_LEFT    0x0010  // Hit test left resizing border
#define HTTB_RESIZINGBORDER_TOP     0x0020  // Hit test top resizing border
#define HTTB_RESIZINGBORDER_RIGHT   0x0040  // Hit test right resizing border
#define HTTB_RESIZINGBORDER_BOTTOM  0x0080  // Hit test bottom resizing border
#define HTTB_RESIZINGBORDER         (HTTB_RESIZINGBORDER_LEFT|HTTB_RESIZINGBORDER_TOP|\
                                     HTTB_RESIZINGBORDER_RIGHT|HTTB_RESIZINGBORDER_BOTTOM)
#define HTTB_SIZINGTEMPLATE         0x0100
#define HTTB_SYSTEMSIZINGMARGINS    0x0200

struct MARGINS
{
    int cxLeftWidth;      // width of left border that retains its size
    int cxRightWidth;     // width of right border that retains its size
    int cyTopHeight;      // height of top border that retains its size
    int cyBottomHeight;   // height of bottom border that retains its size
};
typedef MARGINS* PMARGINS;

#define MAX_INTLIST_COUNT 10

struct INTLIST
{
    int iValueCount;      // number of values in iValues
    int iValues[MAX_INTLIST_COUNT];
};
typedef INTLIST* PINTLIST;

enum PROPERTYORIGIN
{
    PO_STATE,           // property was found in the state section
    PO_PART,            // property was found in the part section
    PO_CLASS,           // property was found in the class section
    PO_GLOBAL,          // property was found in [globals] section
    PO_NOTFOUND         // property was not found
};

#define ETDT_DISABLE        0x00000001
#define ETDT_ENABLE         0x00000002
#define ETDT_USETABTEXTURE  0x00000004
#define ETDT_ENABLETAB      (ETDT_ENABLE | ETDT_USETABTEXTURE)

#define STAP_ALLOW_NONCLIENT    (1 << 0)
#define STAP_ALLOW_CONTROLS     (1 << 1)
#define STAP_ALLOW_WEBCONTENT   (1 << 2)

#define SZ_THDOCPROP_DISPLAYNAME       L"DisplayName"
#define SZ_THDOCPROP_CANONICALNAME     L"ThemeName"
#define SZ_THDOCPROP_TOOLTIP           L"ToolTip"
#define SZ_THDOCPROP_AUTHOR            L"author"

#define DTBG_CLIPRECT        0x00000001   // rcClip has been specified
#define DTBG_DRAWSOLID       0x00000002   // draw transparent/alpha images as solid
#define DTBG_OMITBORDER      0x00000004   // don't draw border of part
#define DTBG_OMITCONTENT     0x00000008   // don't draw content area of part
#define DTBG_COMPUTINGREGION 0x00000010   // TRUE if calling to compute region
#define DTBG_MIRRORDC        0x00000020   // assume the hdc is mirrorred and

struct DTBGOPTS
{
    DWORD dwSize;           // size of the struct
    DWORD dwFlags;          // which options have been specified
    RECT rcClip;            // clipping rectangle
};
typedef DTBGOPTS* PDTBGOPTS;

struct TMPROPINFO
{
    LPCWSTR pszName;
    SHORT sEnumVal;
    BYTE bPrimVal;
};

struct TMSCHEMAINFO
{
    DWORD dwSize;
    int iSchemaDefVersion;
    int iThemeMgrVersion;
    int iPropCount;
    const TMPROPINFO* pPropTable;
};

#endif
#pragma pack(pop)

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED     0x031A
#endif
#endif

bool IsThemed();

HTHEME XPOpenThemeData(HWND hwnd, LPCWSTR pszClassList);
HRESULT XPCloseThemeData(HTHEME hTheme);
HRESULT XPDrawThemeBackground(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, const RECT* pRect, const RECT* pClipRect);
HRESULT XPDrawThemeText(HTHEME hTheme, HDC hdc, int iPartId,
    int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags,
    DWORD dwTextFlags2, const RECT* pRect);
HRESULT XPGetThemeBackgroundContentRect(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId,  const RECT* pBoundingRect,
    OUT RECT* pContentRect);
HRESULT XPGetThemeBackgroundExtent(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, const RECT* pContentRect,
    OUT RECT* pExtentRect);
HRESULT XPGetThemePartSize(HTHEME hTheme, HDC hdc, int iPartId,
    int iStateId, RECT* prc, THEMESIZE eSize, OUT SIZE* psz);
HRESULT XPGetThemeTextExtent(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, LPCWSTR pszText, int iCharCount,
    DWORD dwTextFlags, const RECT* pBoundingRect,
    OUT RECT* pExtentRect);
HRESULT XPGetThemeTextMetrics(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, OUT TEXTMETRIC* ptm);
HRESULT XPGetThemeBackgroundRegion(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, const RECT* pRect, OUT HRGN* pRegion);
HRESULT XPHitTestThemeBackground(HTHEME hTheme, HDC hdc, int iPartId,
    int iStateId, DWORD dwOptions, const RECT* pRect, HRGN hrgn,
    POINT ptTest, OUT WORD* pwHitTestCode);
HRESULT XPDrawThemeEdge(HTHEME hTheme, HDC hdc, int iPartId,
	int iStateId, const RECT* pDestRect, UINT uEdge, UINT uFlags, OUT RECT* pContentRect);
HRESULT XPDrawThemeIcon(HTHEME hTheme, HDC hdc, int iPartId,
    int iStateId, const RECT* pRect, HIMAGELIST himl, int iImageIndex);
BOOL XPIsThemePartDefined(HTHEME hTheme, int iPartId,
    int iStateId);
BOOL XPIsThemeBackgroundPartiallyTransparent(HTHEME hTheme,
    int iPartId, int iStateId);
HRESULT XPGetThemeColor(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT COLORREF* pColor);
HRESULT XPGetThemeMetric(HTHEME hTheme, HDC hdc, int iPartId,
    int iStateId, int iPropId, OUT int* piVal);
HRESULT XPGetThemeString(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT LPWSTR pszBuff, int cchMaxBuffChars);
HRESULT XPGetThemeBool(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT BOOL* pfVal);
HRESULT XPGetThemeInt(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT int* piVal);
HRESULT XPGetThemeEnumValue(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT int* piVal);
HRESULT XPGetThemePosition(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT POINT* pPoint);
HRESULT XPGetThemeFont(HTHEME hTheme, HDC hdc, int iPartId,
    int iStateId, int iPropId, OUT LOGFONTW* pFont);
HRESULT XPGetThemeRect(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT RECT* pRect);
HRESULT XPGetThemeMargins(HTHEME hTheme, HDC hdc, int iPartId,
    int iStateId, int iPropId, RECT* prc, OUT MARGINS* pMargins);
HRESULT XPGetThemeIntList(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT INTLIST* pIntList);
HRESULT XPGetThemePropertyOrigin(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT PROPERTYORIGIN* pOrigin);
HRESULT XPSetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName,
    LPCWSTR pszSubIdList);
HRESULT XPGetThemeFilename(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT LPWSTR pszThemeFileName, int cchMaxBuffChars);
COLORREF XPGetThemeSysColor(HTHEME hTheme, int iColorId);
HBRUSH XPGetThemeSysColorBrush(HTHEME hTheme, int iColorId);
BOOL XPGetThemeSysBool(HTHEME hTheme, int iBoolId);
int XPGetThemeSysSize(HTHEME hTheme, int iSizeId);
HRESULT XPGetThemeSysFont(HTHEME hTheme, int iFontId, OUT LOGFONTW* plf);
HRESULT XPGetThemeSysString(HTHEME hTheme, int iStringId,
    OUT LPWSTR pszStringBuff, int cchMaxStringChars);
HRESULT XPGetThemeSysInt(HTHEME hTheme, int iIntId, int* piValue);
BOOL XPIsThemeActive();
BOOL XPIsAppThemed();
HTHEME XPGetWindowTheme(HWND hwnd);
HRESULT XPEnableThemeDialogTexture(HWND hwnd, DWORD dwFlags);
BOOL XPIsThemeDialogTextureEnabled(HWND hwnd);
DWORD XPGetThemeAppProperties();
void XPSetThemeAppProperties(DWORD dwFlags);
HRESULT XPGetCurrentThemeName(
    OUT LPWSTR pszThemeFileName, int cchMaxNameChars,
    OUT LPWSTR pszColorBuff, int cchMaxColorChars,
    OUT LPWSTR pszSizeBuff, int cchMaxSizeChars);
HRESULT XPGetThemeDocumentationProperty(LPCWSTR pszThemeName,
    LPCWSTR pszPropertyName, OUT LPWSTR pszValueBuff, int cchMaxValChars);
HRESULT XPDrawThemeParentBackground(HWND hwnd, HDC hdc, RECT* prc);
HRESULT XPEnableTheming(BOOL fEnable);
HRESULT XPDrawThemeBackgroundEx(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, const RECT* pRect, const DTBGOPTS* pOptions);

#if 0
/////////////////////////////////////////////////////////////////////
// Theme Manager properties, parts, states, etc

enum BGTYPE
{
    BT_IMAGEFILE = 0,
    BT_BORDERFILL = 1,
    BT_NONE = 2,
};

enum IMAGELAYOUT
{
    IL_VERTICAL = 0,
    IL_HORIZONTAL = 1,
};

enum BORDERTYPE
{
    BT_RECT = 0,
    BT_ROUNDRECT = 1,
    BT_ELLIPSE = 2,
};

enum FILLTYPE
{
    FT_SOLID = 0,
    FT_VERTGRADIENT = 1,
    FT_HORZGRADIENT = 2,
    FT_RADIALGRADIENT = 3,
    FT_TILEIMAGE = 4,
};

enum SIZINGTYPE
{
    ST_TRUESIZE = 0,
    ST_STRETCH = 1,
    ST_TILE = 2,
};

enum HALIGN
{
    HA_LEFT = 0,
    HA_CENTER = 1,
    HA_RIGHT = 2,
};

enum CONTENTALIGNMENT
{
    CA_LEFT = 0,
    CA_CENTER = 1,
    CA_RIGHT = 2,
};

enum VALIGN
{
    VA_TOP = 0,
    VA_CENTER = 1,
    VA_BOTTOM = 2,
};

enum OFFSETTYPE
{
    OT_TOPLEFT = 0,
    OT_TOPRIGHT = 1,
    OT_TOPMIDDLE = 2,
    OT_BOTTOMLEFT = 3,
    OT_BOTTOMRIGHT = 4,
    OT_BOTTOMMIDDLE = 5,
    OT_MIDDLELEFT = 6,
    OT_MIDDLERIGHT = 7,
    OT_LEFTOFCAPTION = 8,
    OT_RIGHTOFCAPTION = 9,
    OT_LEFTOFLASTBUTTON = 10,
    OT_RIGHTOFLASTBUTTON = 11,
    OT_ABOVELASTBUTTON = 12,
    OT_BELOWLASTBUTTON = 13,
};

enum ICONEFFECT
{
    ICE_NONE = 0,
    ICE_GLOW = 1,
    ICE_SHADOW = 2,
    ICE_PULSE = 3,
    ICE_ALPHA = 4,
};

enum TEXTSHADOWTYPE
{
    TST_NONE = 0,
    TST_SINGLE = 1,
    TST_CONTINUOUS = 2,
};

enum GLYPHTYPE
{
    GT_NONE = 0,
    GT_IMAGEGLYPH = 1,
    GT_FONTGLYPH = 2,
};

enum IMAGESELECTTYPE
{
    IST_NONE = 0,
    IST_SIZE = 1,
    IST_DPI = 2,
};

enum TRUESIZESCALINGTYPE
{
    TSST_NONE = 0,
    TSST_SIZE = 1,
    TSST_DPI = 2,
};

enum GLYPHFONTSIZINGTYPE
{
    GFST_NONE = 0,
    GFST_SIZE = 1,
    GFST_DPI = 2,
};

enum PropValues
{
    TMT_STRING = 201,
    TMT_INT = 202,
    TMT_BOOL = 203,
    TMT_COLOR = 204,
    TMT_MARGINS = 205,
    TMT_FILENAME = 206,
    TMT_SIZE = 207,
    TMT_POSITION = 208,
    TMT_RECT = 209,
    TMT_FONT = 210,
    TMT_INTLIST = 211,

    TMT_COLORSCHEMES = 401,
    TMT_SIZES = 402,
    TMT_CHARSET = 403,

    TMT_DISPLAYNAME = 601,
    TMT_TOOLTIP = 602,
    TMT_COMPANY = 603,
    TMT_AUTHOR = 604,
    TMT_COPYRIGHT = 605,
    TMT_URL = 606,
    TMT_VERSION = 607,
    TMT_DESCRIPTION = 608,

    TMT_CAPTIONFONT = 801,
    TMT_SMALLCAPTIONFONT = 802,
    TMT_MENUFONT = 803,
    TMT_STATUSFONT = 804,
    TMT_MSGBOXFONT = 805,
    TMT_ICONTITLEFONT = 806,

    TMT_FLATMENUS = 1001,

    TMT_SIZINGBORDERWIDTH = 1201,
    TMT_SCROLLBARWIDTH = 1202,
    TMT_SCROLLBARHEIGHT = 1203,
    TMT_CAPTIONBARWIDTH = 1204,
    TMT_CAPTIONBARHEIGHT = 1205,
    TMT_SMCAPTIONBARWIDTH = 1206,
    TMT_SMCAPTIONBARHEIGHT = 1207,
    TMT_MENUBARWIDTH = 1208,
    TMT_MENUBARHEIGHT = 1209,

    TMT_MINCOLORDEPTH = 1301,

    TMT_CSSNAME = 1401,
    TMT_XMLNAME = 1402,

    TMT_SCROLLBAR = 1601,
    TMT_BACKGROUND = 1602,
    TMT_ACTIVECAPTION = 1603,
    TMT_INACTIVECAPTION = 1604,
    TMT_MENU = 1605,
    TMT_WINDOW = 1606,
    TMT_WINDOWFRAME = 1607,
    TMT_MENUTEXT = 1608,
    TMT_WINDOWTEXT = 1609,
    TMT_CAPTIONTEXT = 1610,
    TMT_ACTIVEBORDER = 1611,
    TMT_INACTIVEBORDER = 1612,
    TMT_APPWORKSPACE = 1613,
    TMT_HIGHLIGHT = 1614,
    TMT_HIGHLIGHTTEXT = 1615,
    TMT_BTNFACE = 1616,
    TMT_BTNSHADOW = 1617,
    TMT_GRAYTEXT = 1618,
    TMT_BTNTEXT = 1619,
    TMT_INACTIVECAPTIONTEXT = 1620,
    TMT_BTNHIGHLIGHT = 1621,
    TMT_DKSHADOW3D = 1622,
    TMT_LIGHT3D = 1623,
    TMT_INFOTEXT = 1624,
    TMT_INFOBK = 1625,
    TMT_BUTTONALTERNATEFACE = 1626,
    TMT_HOTTRACKING = 1627,
    TMT_GRADIENTACTIVECAPTION = 1628,
    TMT_GRADIENTINACTIVECAPTION = 1629,
    TMT_MENUHILIGHT = 1630,
    TMT_MENUBAR = 1631,

    TMT_FROMHUE1 = 1801,
    TMT_FROMHUE2 = 1802,
    TMT_FROMHUE3 = 1803,
    TMT_FROMHUE4 = 1804,
    TMT_FROMHUE5 = 1805,
    TMT_TOHUE1 = 1806,
    TMT_TOHUE2 = 1807,
    TMT_TOHUE3 = 1808,
    TMT_TOHUE4 = 1809,
    TMT_TOHUE5 = 1810,

    TMT_FROMCOLOR1 = 2001,
    TMT_FROMCOLOR2 = 2002,
    TMT_FROMCOLOR3 = 2003,
    TMT_FROMCOLOR4 = 2004,
    TMT_FROMCOLOR5 = 2005,
    TMT_TOCOLOR1 = 2006,
    TMT_TOCOLOR2 = 2007,
    TMT_TOCOLOR3 = 2008,
    TMT_TOCOLOR4 = 2009,
    TMT_TOCOLOR5 = 2010,

    TMT_TRANSPARENT = 2201,
    TMT_AUTOSIZE = 2202,
    TMT_BORDERONLY = 2203,
    TMT_COMPOSITED = 2204,
    TMT_BGFILL = 2205,
    TMT_GLYPHTRANSPARENT = 2206,
    TMT_GLYPHONLY = 2207,
    TMT_ALWAYSSHOWSIZINGBAR = 2208,
    TMT_MIRRORIMAGE = 2209,
    TMT_UNIFORMSIZING = 2210,
    TMT_INTEGRALSIZING = 2211,
    TMT_SOURCEGROW = 2212,
    TMT_SOURCESHRINK = 2213,

    TMT_IMAGECOUNT = 2401,
    TMT_ALPHALEVEL = 2402,
    TMT_BORDERSIZE = 2403,
    TMT_ROUNDCORNERWIDTH = 2404,
    TMT_ROUNDCORNERHEIGHT = 2405,
    TMT_GRADIENTRATIO1 = 2406,
    TMT_GRADIENTRATIO2 = 2407,
    TMT_GRADIENTRATIO3 = 2408,
    TMT_GRADIENTRATIO4 = 2409,
    TMT_GRADIENTRATIO5 = 2410,
    TMT_PROGRESSCHUNKSIZE = 2411,
    TMT_PROGRESSSPACESIZE = 2412,
    TMT_SATURATION = 2413,
    TMT_TEXTBORDERSIZE = 2414,
    TMT_ALPHATHRESHOLD = 2415,
    TMT_WIDTH = 2416,
    TMT_HEIGHT = 2417,
    TMT_GLYPHINDEX = 2418,
    TMT_TRUESIZESTRETCHMARK = 2419,
    TMT_MINDPI1 = 2420,
    TMT_MINDPI2 = 2421,
    TMT_MINDPI3 = 2422,
    TMT_MINDPI4 = 2423,
    TMT_MINDPI5 = 2424,

    TMT_GLYPHFONT = 2601,

    TMT_IMAGEFILE = 3001,
    TMT_IMAGEFILE1 = 3002,
    TMT_IMAGEFILE2 = 3003,
    TMT_IMAGEFILE3 = 3004,
    TMT_IMAGEFILE4 = 3005,
    TMT_IMAGEFILE5 = 3006,
    TMT_STOCKIMAGEFILE = 3007,
    TMT_GLYPHIMAGEFILE = 3008,

    TMT_TEXT = 3201,

    TMT_OFFSET = 3401,
    TMT_TEXTSHADOWOFFSET = 3402,
    TMT_MINSIZE = 3403,
    TMT_MINSIZE1 = 3404,
    TMT_MINSIZE2 = 3405,
    TMT_MINSIZE3 = 3406,
    TMT_MINSIZE4 = 3407,
    TMT_MINSIZE5 = 3408,
    TMT_NORMALSIZE = 3409,

    TMT_SIZINGMARGINS = 3601,
    TMT_CONTENTMARGINS = 3602,
    TMT_CAPTIONMARGINS = 3603,

    TMT_BORDERCOLOR = 3801,
    TMT_FILLCOLOR = 3802,
    TMT_TEXTCOLOR = 3803,
    TMT_EDGELIGHTCOLOR = 3804,
    TMT_EDGEHIGHLIGHTCOLOR = 3805,
    TMT_EDGESHADOWCOLOR = 3806,
    TMT_EDGEDKSHADOWCOLOR = 3807,
    TMT_EDGEFILLCOLOR = 3808,
    TMT_TRANSPARENTCOLOR = 3809,
    TMT_GRADIENTCOLOR1 = 3810,
    TMT_GRADIENTCOLOR2 = 3811,
    TMT_GRADIENTCOLOR3 = 3812,
    TMT_GRADIENTCOLOR4 = 3813,
    TMT_GRADIENTCOLOR5 = 3814,
    TMT_SHADOWCOLOR = 3815,
    TMT_GLOWCOLOR = 3816,
    TMT_TEXTBORDERCOLOR = 3817,
    TMT_TEXTSHADOWCOLOR = 3818,
    TMT_GLYPHTEXTCOLOR = 3819,
    TMT_GLYPHTRANSPARENTCOLOR = 3820,
    TMT_FILLCOLORHINT = 3821,
    TMT_BORDERCOLORHINT = 3822,
    TMT_ACCENTCOLORHINT = 3823,

    TMT_BGTYPE = 4001,
    TMT_BORDERTYPE = 4002,
    TMT_FILLTYPE = 4003,
    TMT_SIZINGTYPE = 4004,
    TMT_HALIGN = 4005,
    TMT_CONTENTALIGNMENT = 4006,
    TMT_VALIGN = 4007,
    TMT_OFFSETTYPE = 4008,
    TMT_ICONEFFECT = 4009,
    TMT_TEXTSHADOWTYPE = 4010,
    TMT_IMAGELAYOUT = 4011,
    TMT_GLYPHTYPE = 4012,
    TMT_IMAGESELECTTYPE = 4013,
    TMT_GLYPHFONTSIZINGTYPE = 4014,
    TMT_TRUESIZESCALINGTYPE = 4015,
    
    TMT_USERPICTURE = 5001,
    TMT_DEFAULTPANESIZE = 5002,
    TMT_BLENDCOLOR = 5003,
};

enum WINDOWPARTS
{
    WP_CAPTION = 1,
    WP_SMALLCAPTION = 2,
    WP_MINCAPTION = 3,
    WP_SMALLMINCAPTION = 4,
    WP_MAXCAPTION = 5,
    WP_SMALLMAXCAPTION = 6,
    WP_FRAMELEFT = 7,
    WP_FRAMERIGHT = 8,
    WP_FRAMEBOTTOM = 9,
    WP_SMALLFRAMELEFT = 10,
    WP_SMALLFRAMERIGHT = 11,
    WP_SMALLFRAMEBOTTOM = 12,
    
    WP_SYSBUTTON = 13,
    WP_MDISYSBUTTON = 14,
    WP_MINBUTTON = 15,
    WP_MDIMINBUTTON = 16,
    WP_MAXBUTTON = 17,
    WP_CLOSEBUTTON = 18,
    WP_SMALLCLOSEBUTTON = 19,
    WP_MDICLOSEBUTTON = 20,
    WP_RESTOREBUTTON = 21,
    WP_MDIRESTOREBUTTON = 22,
    WP_HELPBUTTON = 23,
    WP_MDIHELPBUTTON = 24,
    
    WP_HORZSCROLL = 25,
    WP_HORZTHUMB = 26,
    WP_VERTSCROLL = 27,
    WP_VERTTHUMB = 28,
    
    WP_DIALOG = 29,
    
    WP_CAPTIONSIZINGTEMPLATE = 30,
    WP_SMALLCAPTIONSIZINGTEMPLATE = 31,
    WP_FRAMELEFTSIZINGTEMPLATE = 32,
    WP_SMALLFRAMELEFTSIZINGTEMPLATE = 33,
    WP_FRAMERIGHTSIZINGTEMPLATE = 34,
    WP_SMALLFRAMERIGHTSIZINGTEMPLATE = 35,
    WP_FRAMEBOTTOMSIZINGTEMPLATE = 36,
    WP_SMALLFRAMEBOTTOMSIZINGTEMPLATE = 37,
};

enum FRAMESTATES
{
    FS_ACTIVE = 1,
    FS_INACTIVE = 2,
};

enum CAPTIONSTATES
{
    CS_ACTIVE = 1,
    CS_INACTIVE = 2,
    CS_DISABLED = 3,
};
    
enum MAXCAPTIONSTATES
{
    MXCS_ACTIVE = 1,
    MXCS_INACTIVE = 2,
    MXCS_DISABLED = 3,
};

enum MINCAPTIONSTATES
{
    MNCS_ACTIVE = 1,
    MNCS_INACTIVE = 2,
    MNCS_DISABLED = 3,
};

enum HORZSCROLLSTATES
{
    HSS_NORMAL = 1,
    HSS_HOT = 2,
    HSS_PUSHED = 3,
    HSS_DISABLED = 4,
};

enum HORZTHUMBSTATES
{
    HTS_NORMAL = 1,
    HTS_HOT = 2,
    HTS_PUSHED = 3,
    HTS_DISABLED = 4,
};

enum VERTSCROLLSTATES
{
    VSS_NORMAL = 1,
    VSS_HOT = 2,
    VSS_PUSHED = 3,
    VSS_DISABLED = 4,
};

enum VERTTHUMBSTATES
{
    VTS_NORMAL = 1,
    VTS_HOT = 2,
    VTS_PUSHED = 3,
    VTS_DISABLED = 4,
};

enum SYSBUTTONSTATES
{
    SBS_NORMAL = 1,
    SBS_HOT = 2,
    SBS_PUSHED = 3,
    SBS_DISABLED = 4,
};

enum MINBUTTONSTATES
{
    MINBS_NORMAL = 1,
    MINBS_HOT = 2,
    MINBS_PUSHED = 3,
    MINBS_DISABLED = 4,
};

enum MAXBUTTONSTATES
{
    MAXBS_NORMAL = 1,
    MAXBS_HOT = 2,
    MAXBS_PUSHED = 3,
    MAXBS_DISABLED = 4,
};

enum RESTOREBUTTONSTATES
{
    RBS_NORMAL = 1,
    RBS_HOT = 2,
    RBS_PUSHED = 3,
    RBS_DISABLED = 4,
};

enum HELPBUTTONSTATES
{
    HBS_NORMAL = 1,
    HBS_HOT = 2,
    HBS_PUSHED = 3,
    HBS_DISABLED = 4,
};

enum CLOSEBUTTONSTATES
{
    CBS_NORMAL = 1,
    CBS_HOT = 2,
    CBS_PUSHED = 3,
    CBS_DISABLED = 4,
};

enum BUTTONPARTS
{
    BP_PUSHBUTTON = 1,
    BP_RADIOBUTTON = 2,
    BP_CHECKBOX = 3,
    BP_GROUPBOX = 4,
    BP_USERBUTTON = 5,
};

enum PUSHBUTTONSTATES
{
    PBS_NORMAL = 1,
    PBS_HOT = 2,
    PBS_PRESSED = 3,
    PBS_DISABLED = 4,
    PBS_DEFAULTED = 5,
};

enum RADIOBUTTONSTATES
{
    RBS_UNCHECKEDNORMAL = 1,
    RBS_UNCHECKEDHOT = 2,
    RBS_UNCHECKEDPRESSED = 3,
    RBS_UNCHECKEDDISABLED = 4,
    RBS_CHECKEDNORMAL = 5,
    RBS_CHECKEDHOT = 6,
    RBS_CHECKEDPRESSED = 7,
    RBS_CHECKEDDISABLED = 8,
};

enum CHECKBOXSTATES
{
    CBS_UNCHECKEDNORMAL = 1,
    CBS_UNCHECKEDHOT = 2,
    CBS_UNCHECKEDPRESSED = 3,
    CBS_UNCHECKEDDISABLED = 4,
    CBS_CHECKEDNORMAL = 5,
    CBS_CHECKEDHOT = 6,
    CBS_CHECKEDPRESSED = 7,
    CBS_CHECKEDDISABLED = 8,
    CBS_MIXEDNORMAL = 9,
    CBS_MIXEDHOT = 10,
    CBS_MIXEDPRESSED = 11,
    CBS_MIXEDDISABLED = 12,
};

enum GROUPBOXSTATES
{
    GBS_NORMAL = 1,
    GBS_DISABLED = 2,
};

enum REBARPARTS
{
    RP_GRIPPER = 1,
    RP_GRIPPERVERT = 2,
    RP_BAND = 3,
    RP_CHEVRON = 4,
    RP_CHEVRONVERT = 5,
};

enum CHEVRONSTATES
{
    CHEVS_NORMAL = 1,
    CHEVS_HOT = 2,
    CHEVS_PRESSED = 3,
};

enum TOOLBARPARTS
{
    TP_BUTTON = 1,
    TP_DROPDOWNBUTTON = 2,
    TP_SPLITBUTTON = 3,
    TP_SPLITBUTTONDROPDOWN = 4,
    TP_SEPARATOR = 5,
    TP_SEPARATORVERT = 6,
};

enum TOOLBARSTATES
{
    TS_NORMAL = 1,
    TS_HOT = 2,
    TS_PRESSED = 3,
    TS_DISABLED = 4,
    TS_CHECKED = 5,
    TS_HOTCHECKED = 6,
};

enum STATUSPARTS
{
    SP_PANE = 1,
    SP_GRIPPERPANE = 2,
    SP_GRIPPER = 3,
};

enum MENUPARTS
{
    MP_MENUITEM = 1,
    MP_MENUDROPDOWN = 2,
    MP_MENUBARITEM = 3,
    MP_MENUBARDROPDOWN = 4,
    MP_CHEVRON = 5,
    MP_SEPARATOR = 6,
};

enum MENUSTATES
{
    MS_NORMAL = 1,
    MS_SELECTED = 2,
    MS_DEMOTED = 3,
};

enum LISTVIEWPARTS
{
    LVP_LISTITEM = 1,
    LVP_LISTGROUP = 2,
    LVP_LISTDETAIL = 3,
    LVP_LISTSORTEDDETAIL = 4,
    LVP_EMPTYTEXT = 5,
};

enum LISTITEMSTATES
{
    LIS_NORMAL = 1,
    LIS_HOT = 2,
    LIS_SELECTED = 3,
    LIS_DISABLED = 4,
    LIS_SELECTEDNOTFOCUS = 5,
};

enum HEADERPARTS
{
    HP_HEADERITEM = 1,
    HP_HEADERITEMLEFT = 2,
    HP_HEADERITEMRIGHT = 3,
    HP_HEADERSORTARROW = 4,
};

enum HEADERITEMSTATES
{
    HIS_NORMAL = 1,
    HIS_HOT = 2,
    HIS_PRESSED = 3,
};

enum HEADERITEMLEFTSTATES
{
    HILS_NORMAL = 1,
    HILS_HOT = 2,
    HILS_PRESSED = 3,
};

enum HEADERITEMRIGHTSTATES
{
    HIRS_NORMAL = 1,
    HIRS_HOT = 2,
    HIRS_PRESSED = 3,
};

enum HEADERSORTARROWSTATES
{
    HSAS_SORTEDUP = 1,
    HSAS_SORTEDDOWN = 2,
};

enum PROGRESSPARTS
{
    PP_BAR = 1,
    PP_BARVERT = 2,
    PP_CHUNK = 3,
    PP_CHUNKVERT = 4,
};

enum TABPARTS
{
    TABP_TABITEM = 1,
    TABP_TABITEMLEFTEDGE = 2,
    TABP_TABITEMRIGHTEDGE = 3,
    TABP_TABITEMBOTHEDGE = 4,
    TABP_TOPTABITEM = 5,
    TABP_TOPTABITEMLEFTEDGE = 6,
    TABP_TOPTABITEMRIGHTEDGE = 7,
    TABP_TOPTABITEMBOTHEDGE = 8,
    TABP_PANE = 9,
    TABP_BODY = 10,
};

enum TABITEMSTATES
{
    TIS_NORMAL = 1,
    TIS_HOT = 2,
    TIS_SELECTED = 3,
    TIS_DISABLED = 4,
    TIS_FOCUSED = 5,
};

enum TABITEMLEFTEDGESTATES
{
    TILES_NORMAL = 1,
    TILES_HOT = 2,
    TILES_SELECTED = 3,
    TILES_DISABLED = 4,
    TILES_FOCUSED = 5,
};

enum TABITEMRIGHTEDGESTATES
{
    TIRES_NORMAL = 1,
    TIRES_HOT = 2,
    TIRES_SELECTED = 3,
    TIRES_DISABLED = 4,
    TIRES_FOCUSED = 5,
};

enum TABITEMBOTHEDGESSTATES
{
    TIBES_NORMAL = 1,
    TIBES_HOT = 2,
    TIBES_SELECTED = 3,
    TIBES_DISABLED = 4,
    TIBES_FOCUSED = 5,
};

enum TOPTABITEMSTATES
{
    TTIS_NORMAL = 1,
    TTIS_HOT = 2,
    TTIS_SELECTED = 3,
    TTIS_DISABLED = 4,
    TTIS_FOCUSED = 5,
};

enum TOPTABITEMLEFTEDGESTATES
{
    TTILES_NORMAL = 1,
    TTILES_HOT = 2,
    TTILES_SELECTED = 3,
    TTILES_DISABLED = 4,
    TTILES_FOCUSED = 5,
};

enum TOPTABITEMRIGHTEDGESTATES
{
    TTIRES_NORMAL = 1,
    TTIRES_HOT = 2,
    TTIRES_SELECTED = 3,
    TTIRES_DISABLED = 4,
    TTIRES_FOCUSED = 5,
};

enum TOPTABITEMBOTHEDGESSTATES
{
    TTIBES_NORMAL = 1,
    TTIBES_HOT = 2,
    TTIBES_SELECTED = 3,
    TTIBES_DISABLED = 4,
    TTIBES_FOCUSED = 5,
};

enum TRACKBARPARTS
{
    TKP_TRACK = 1,
    TKP_TRACKVERT = 2,
    TKP_THUMB = 3,
    TKP_THUMBBOTTOM = 4,
    TKP_THUMBTOP = 5,
    TKP_THUMBVERT = 6,
    TKP_THUMBLEFT = 7,
    TKP_THUMBRIGHT = 8,
    TKP_TICS = 9,
    TKP_TICSVERT = 10,
};

enum TRACKBARSTATES
{
    TKS_NORMAL = 1,
};

enum TRACKSTATES
{
    TRS_NORMAL = 1,
};

enum TRACKVERTSTATES
{
    TRVS_NORMAL = 1,
};

enum THUMBSTATES
{
    TUS_NORMAL = 1,
    TUS_HOT = 2,
    TUS_PRESSED = 3,
    TUS_FOCUSED = 4,
    TUS_DISABLED = 5,
};

enum THUMBBOTTOMSTATES
{
    TUBS_NORMAL = 1,
    TUBS_HOT = 2,
    TUBS_PRESSED = 3,
    TUBS_FOCUSED = 4,
    TUBS_DISABLED = 5,
};

enum THUMBTOPSTATES
{
    TUTS_NORMAL = 1,
    TUTS_HOT = 2,
    TUTS_PRESSED = 3,
    TUTS_FOCUSED = 4,
    TUTS_DISABLED = 5,
};

enum THUMBVERTSTATES
{
    TUVS_NORMAL = 1,
    TUVS_HOT = 2,
    TUVS_PRESSED = 3,
    TUVS_FOCUSED = 4,
    TUVS_DISABLED = 5,
};

enum THUMBLEFTSTATES
{
    TUVLS_NORMAL = 1,
    TUVLS_HOT = 2,
    TUVLS_PRESSED = 3,
    TUVLS_FOCUSED = 4,
    TUVLS_DISABLED = 5,
};

enum THUMBRIGHTSTATES
{
    TUVRS_NORMAL = 1,
    TUVRS_HOT = 2,
    TUVRS_PRESSED = 3,
    TUVRS_FOCUSED = 4,
    TUVRS_DISABLED = 5,
};

enum TICSSTATES
{
    TSS_NORMAL = 1,
};

enum TICSVERTSTATES
{
    TSVS_NORMAL = 1,
};

enum TOOLTIPPARTS
{
    TTP_STANDARD = 1,
    TTP_STANDARDTITLE = 2,
    TTP_BALLOON = 3,
    TTP_BALLOONTITLE = 4,
    TTP_CLOSE = 5,
};

enum CLOSESTATES
{
	TTCS_NORMAL = 1,
	TTCS_HOT = 2,
	TTCS_PRESSED = 3,
};

enum STANDARDSTATES
{
	TTSS_NORMAL = 1,
	TTSS_LINK = 2,
};

enum BALLOONSTATES
{
	TTBS_NORMAL = 1,
	TTBS_LINK = 2,
};

enum TREEVIEWPARTS
{
    TVP_TREEITEM = 1,
    TVP_GLYPH = 2,
    TVP_BRANCH = 3,
};

enum TREEITEMSTATES
{
    TREIS_NORMAL = 1,
    TREIS_HOT = 2,
    TREIS_SELECTED = 3,
    TREIS_DISABLED = 4,
    TREIS_SELECTEDNOTFOCUS = 5,
};

enum GLYPHSTATES
{
    GLPS_CLOSED = 1,
    GLPS_OPENED = 2,
};

enum SPINPARTS
{
    SPNP_UP = 1,
    SPNP_DOWN = 2,
    SPNP_UPHORZ = 3,
    SPNP_DOWNHORZ = 4,
};

enum UPSTATES
{
    UPS_NORMAL = 1,
    UPS_HOT = 2,
    UPS_PRESSED = 3,
    UPS_DISABLED = 4,
};

enum DOWNSTATES
{
    DNS_NORMAL = 1,
    DNS_HOT = 2,
    DNS_PRESSED = 3,
    DNS_DISABLED = 4,
};

enum UPHORZSTATES
{
    UPHZS_NORMAL = 1,
    UPHZS_HOT = 2,
    UPHZS_PRESSED = 3,
    UPHZS_DISABLED = 4,
};

enum DOWNHORZSTATES
{
    DNHZS_NORMAL = 1,
    DNHZS_HOT = 2,
    DNHZS_PRESSED = 3,
    DNHZS_DISABLED = 4,
};

enum PAGEPARTS
{
    PGRP_UP = 1,
    PGRP_DOWN = 2,
    PGRP_UPHORZ = 3,
    PGRP_DOWNHORZ = 4,
};

enum SCROLLBARPARTS
{
    SBP_ARROWBTN = 1,
    SBP_THUMBBTNHORZ = 2,
    SBP_THUMBBTNVERT = 3,
    SBP_LOWERTRACKHORZ = 4,
    SBP_UPPERTRACKHORZ = 5,
    SBP_LOWERTRACKVERT = 6,
    SBP_UPPERTRACKVERT = 7,
    SBP_GRIPPERHORZ = 8,
    SBP_GRIPPERVERT = 9,
    SBP_SIZEBOX = 10,
};

enum ARROWBTNSTATES
{
    ABS_UPNORMAL = 1,
    ABS_UPHOT = 2,
    ABS_UPPRESSED = 3,
    ABS_UPDISABLED = 4,
    ABS_DOWNNORMAL = 5,
    ABS_DOWNHOT = 6,
    ABS_DOWNPRESSED = 7,
    ABS_DOWNDISABLED = 8,
    ABS_LEFTNORMAL = 9,
    ABS_LEFTHOT = 10,
    ABS_LEFTPRESSED = 11,
    ABS_LEFTDISABLED = 12,
    ABS_RIGHTNORMAL = 13,
    ABS_RIGHTHOT = 14,
    ABS_RIGHTPRESSED = 15,
    ABS_RIGHTDISABLED = 16,
};

enum SCROLLBARSTATES
{
    SCRBS_NORMAL = 1,
    SCRBS_HOT = 2,
    SCRBS_PRESSED = 3,
    SCRBS_DISABLED = 4,
};

enum SIZEBOXSTATES
{
    SZB_RIGHTALIGN = 1,
    SZB_LEFTALIGN = 2,
};

enum EDITPARTS
{
    EP_EDITTEXT = 1,
    EP_CARET = 2,
};

enum EDITTEXTSTATES
{
    ETS_NORMAL = 1,
    ETS_HOT = 2,
    ETS_SELECTED = 3,
    ETS_DISABLED = 4,
    ETS_FOCUSED = 5,
    ETS_READONLY = 6,
    ETS_ASSIST = 7,
};

enum COMBOBOXPARTS
{
    CP_DROPDOWNBUTTON = 1,
};

enum COMBOBOXSTATES
{
    CBXS_NORMAL = 1,
    CBXS_HOT = 2,
    CBXS_PRESSED = 3,
    CBXS_DISABLED = 4,
};

enum CLOCKPARTS
{
    CLP_TIME = 1,
};

enum CLOCKSTATES
{
    CLS_NORMAL = 1,
};

enum TRAYNOTIFYPARTS
{
    TNP_BACKGROUND = 1,
    TNP_ANIMBACKGROUND = 2,
};

enum TASKBARPARTS
{
    TBP_BACKGROUNDBOTTOM = 1,
    TBP_BACKGROUNDRIGHT = 2,
    TBP_BACKGROUNDTOP = 3,
    TBP_BACKGROUNDLEFT = 4,
    TBP_SIZINGBARBOTTOM = 5,
    TBP_SIZINGBARRIGHT = 6,
    TBP_SIZINGBARTOP = 7,
    TBP_SIZINGBARLEFT = 8,
};

enum TASKBANDPARTS
{
    TDP_GROUPCOUNT = 1,
    TDP_FLASHBUTTON = 2,
    TDP_FLASHBUTTONGROUPMENU = 3,
};

enum STARTPANELPARTS
{
    SPP_USERPANE = 1,
    SPP_MOREPROGRAMS = 2,
    SPP_MOREPROGRAMSARROW = 3,
    SPP_PROGLIST = 4,
    SPP_PROGLISTSEPARATOR = 5,
    SPP_PLACESLIST = 6,
    SPP_PLACESLISTSEPARATOR = 7,
    SPP_LOGOFF = 8,
    SPP_LOGOFFBUTTONS = 9,
    SPP_USERPICTURE = 10,
    SPP_PREVIEW = 11,
};

enum MOREPROGRAMSARROWSTATES
{
    SPS_NORMAL = 1,
    SPS_HOT = 2,
    SPS_PRESSED = 3,
};

enum LOGOFFBUTTONSSTATES
{
    SPLS_NORMAL = 1,
    SPLS_HOT = 2,
    SPLS_PRESSED = 3,
};

enum EXPLORERBARPARTS
{
    EBP_HEADERBACKGROUND = 1,
    EBP_HEADERCLOSE = 2,
    EBP_HEADERPIN = 3,
    EBP_IEBARMENU = 4,
    EBP_NORMALGROUPBACKGROUND = 5,
    EBP_NORMALGROUPCOLLAPSE = 6,
    EBP_NORMALGROUPEXPAND = 7,
    EBP_NORMALGROUPHEAD = 8,
    EBP_SPECIALGROUPBACKGROUND = 9,
    EBP_SPECIALGROUPCOLLAPSE = 10,
    EBP_SPECIALGROUPEXPAND = 11,
    EBP_SPECIALGROUPHEAD = 12,
};

enum HEADERCLOSESTATES
{
    EBHC_NORMAL = 1,
    EBHC_HOT = 2,
    EBHC_PRESSED = 3,
};

enum HEADERPINSTATES
{
    EBHP_NORMAL = 1,
    EBHP_HOT = 2,
    EBHP_PRESSED = 3,
    EBHP_SELECTEDNORMAL = 4,
    EBHP_SELECTEDHOT = 5,
    EBHP_SELECTEDPRESSED = 6,
};

enum IEBARMENUSTATES
{
    EBM_NORMAL = 1,
    EBM_HOT = 2,
    EBM_PRESSED = 3,
};

enum NORMALGROUPCOLLAPSESTATES
{
    EBNGC_NORMAL = 1,
    EBNGC_HOT = 2,
    EBNGC_PRESSED = 3,
};

enum NORMALGROUPEXPANDSTATES
{
    EBNGE_NORMAL = 1,
    EBNGE_HOT = 2,
    EBNGE_PRESSED = 3,
};

enum SPECIALGROUPCOLLAPSESTATES
{
    EBSGC_NORMAL = 1,
    EBSGC_HOT = 2,
    EBSGC_PRESSED = 3,
};

enum SPECIALGROUPEXPANDSTATES
{
    EBSGE_NORMAL = 1,
    EBSGE_HOT = 2,
    EBSGE_PRESSED = 3,
};

enum MENUBANDPARTS
{
    MDP_NEWAPPBUTTON = 1,
    MDP_SEPERATOR = 2,
};

enum MENUBANDSTATES
{
    MDS_NORMAL = 1,
    MDS_HOT = 2,
    MDS_PRESSED = 3,
    MDS_DISABLED = 4,
    MDS_CHECKED = 5,
    MDS_HOTCHECKED = 6,
};
#endif
