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
//
//	Based on ColourPickerXP from CodeProject
//	Copyright (C) 2002-2003 Zorglab

#include "stdafx.h"
#include "WinDjView.h"
#include "MyColorPicker.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#ifndef SPI_GETFLATMENU
#define SPI_GETFLATMENU 0x1022
#endif

#ifndef SPI_GETDROPSHADOW
#define SPI_GETDROPSHADOW 0x1024
#endif

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW 0x00020000
#endif

#ifndef ODS_HOTLIGHT
#define ODS_HOTLIGHT 0x0040
#endif

#ifndef COLOR_MENUHILIGHT
#define COLOR_MENUHILIGHT 29
#endif

struct ColorPreset
{
	COLORREF color;
	TCHAR pszName[32];
};

ColorPreset colorTable[] =
{
	{ RGB(0x00, 0x00, 0x00), _T("Black")           },
	{ RGB(0x99, 0x33, 0x00), _T("Brown")           },
	{ RGB(0x33, 0x33, 0x00), _T("Olive Green")     },
	{ RGB(0x00, 0x33, 0x00), _T("Dark Green")      },
	{ RGB(0x00, 0x33, 0x66), _T("Dark Teal")       },
	{ RGB(0x00, 0x00, 0x80), _T("Dark Blue")       },
	{ RGB(0x33, 0x33, 0x99), _T("Indigo")          },
	{ RGB(0x33, 0x33, 0x33), _T("Gray-80%")        },

	{ RGB(0x80, 0x00, 0x00), _T("Dark Red")        },
	{ RGB(0xFF, 0x66, 0x00), _T("Orange")          },
	{ RGB(0x80, 0x80, 0x00), _T("Dark Yellow")     },
	{ RGB(0x00, 0x80, 0x00), _T("Green")           },
	{ RGB(0x00, 0x80, 0x80), _T("Teal")            },
	{ RGB(0x00, 0x00, 0xFF), _T("Blue")            },
	{ RGB(0x66, 0x66, 0x99), _T("Blue-Gray")       },
	{ RGB(0x80, 0x80, 0x80), _T("Gray-50%")        },

	{ RGB(0xFF, 0x00, 0x00), _T("Red")             },
	{ RGB(0xFF, 0x99, 0x00), _T("Light Orange")    },
	{ RGB(0x99, 0xCC, 0x00), _T("Lime")            },
	{ RGB(0x33, 0x99, 0x66), _T("Sea Green")       },
	{ RGB(0x33, 0xCC, 0xCC), _T("Aqua")            },
	{ RGB(0x33, 0x66, 0xFF), _T("Light Blue")      },
	{ RGB(0x80, 0x00, 0x80), _T("Violet")          },
	{ RGB(0x99, 0x99, 0x99), _T("Gray-40%")        },

	{ RGB(0xFF, 0x00, 0xFF), _T("Pink")            },
	{ RGB(0xFF, 0xCC, 0x00), _T("Gold")            },
	{ RGB(0xFF, 0xFF, 0x00), _T("Yellow")          },
	{ RGB(0x00, 0xFF, 0x00), _T("Bright Green")    },
	{ RGB(0x00, 0xFF, 0xFF), _T("Turquoise")       },
	{ RGB(0x00, 0xCC, 0xFF), _T("Sky Blue")        },
	{ RGB(0x99, 0x33, 0x66), _T("Plum")            },
	{ RGB(0xC0, 0xC0, 0xC0), _T("Gray-25%")        },

	{ RGB(0xFF, 0x99, 0xCC), _T("Rose")            },
	{ RGB(0xFF, 0xCC, 0x99), _T("Tan")             },
	{ RGB(0xFF, 0xFF, 0x99), _T("Light Yellow")    },
	{ RGB(0xCC, 0xFF, 0xCC), _T("Light Green")     },
	{ RGB(0xCC, 0xFF, 0xFF), _T("Light Turquoise") },
	{ RGB(0x99, 0xCC, 0xFF), _T("Pale Blue")       },
	{ RGB(0xCC, 0x99, 0xFF), _T("Lavender")        },
	{ RGB(0xFF, 0xFF, 0xFF), _T("White")           }
};

const int c_nColorTableSize = sizeof(colorTable)/sizeof(ColorPreset);


const COLORREF COMBOBOX_DISABLED_COLOR_OUT = RGB(201, 199, 186);
const COLORREF COMBOBOX_DISABLED_COLOR_IN = RGB(245, 244, 234);

const int c_nArrowSizeX = 4;
const int c_nArrowSizeY = 2;


void AFXAPI DDX_Color(CDataExchange *pDX, int nIDC, COLORREF& color)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	ASSERT(hWndCtrl != NULL);                
    
	CMyColorPicker* pColorButton = (CMyColorPicker*) CWnd::FromHandle(hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		color = pColorButton->GetColor();
    }
	else
	{
		pColorButton->SetColor(color);
	}
}


IMPLEMENT_DYNCREATE(CMyColorPicker, CButton)

CMyColorPicker::CMyColorPicker()
	: CButton(), m_color(CLR_DEFAULT),
	  m_colorDefault(::GetSysColor(COLOR_APPWORKSPACE)),
	  m_bPopupActive(false), m_bTrackSelection(false), m_bMouseOver(false),
	  m_bFlatMenus(false), m_hThemeButton(NULL), m_bIsDefault(false)
{
	BOOL bFlatMenus = FALSE;
	if (::SystemParametersInfo(SPI_GETFLATMENU, 0, &bFlatMenus, false))
		m_bFlatMenus = !!bFlatMenus;
}

CMyColorPicker::~CMyColorPicker()
{
}


// CMyColorPicker getters/setters

COLORREF CMyColorPicker::GetColor(bool bTranslateDefault) const
{
	return (bTranslateDefault && m_color == CLR_DEFAULT ? m_colorDefault : m_color);
}

void CMyColorPicker::SetColor(COLORREF color)
{
	m_color = color;
	if (::IsWindow(m_hWnd))
        Invalidate(false);
}

COLORREF CMyColorPicker::GetDefaultColor() const
{
	return m_colorDefault;
}

void CMyColorPicker::SetDefaultColor(COLORREF color)
{
	m_colorDefault = color;
}

void CMyColorPicker::SetCustomText(LPCTSTR pszText)
{
	m_popup.SetCustomText(pszText);
}

void CMyColorPicker::SetDefaultText(LPCTSTR pszText)
{
	m_popup.SetDefaultText(pszText);
	m_strDefaultText = pszText;
}

void CMyColorPicker::SetColorNames(UINT nID)
{
	m_popup.SetColorNames(nID);
}

void CMyColorPicker::SetRegSection(LPCTSTR pszRegSection)
{
	m_popup.SetRegSection(pszRegSection);
}

void CMyColorPicker::SetTrackSelection(bool bTrack)
{
	m_bTrackSelection = bTrack;
}

bool CMyColorPicker::IsTrackSelection() const
{
	return m_bTrackSelection;
}


BEGIN_MESSAGE_MAP(CMyColorPicker, CButton)
	ON_WM_DESTROY()
	ON_MESSAGE(CPN_SELENDOK, OnSelEndOK)
	ON_MESSAGE(CPN_SELENDCANCEL, OnSelEndCancel)
	ON_MESSAGE(CPN_SELCHANGE, OnSelChange)
	ON_WM_MOUSEMOVE()
	ON_MESSAGE_VOID(WM_MOUSELEAVE, OnMouseLeave)
	ON_CONTROL_REFLECT_EX(BN_CLICKED, OnClicked)
	ON_MESSAGE_VOID(WM_THEMECHANGED, OnThemeChanged)
	ON_WM_GETDLGCODE()
	ON_MESSAGE(BM_SETSTYLE, OnSetStyle)
	ON_WM_KILLFOCUS()
	ON_WM_CANCELMODE()
END_MESSAGE_MAP()


// CMyColorPicker message handlers

void CMyColorPicker::PreSubclassWindow()
{
	ModifyStyle(0, BS_OWNERDRAW);

	CButton::PreSubclassWindow();
}

LRESULT CMyColorPicker::OnSelEndOK(WPARAM wParam, LPARAM lParam)
{
	m_bPopupActive = false;

	COLORREF colorOld = m_color;
	SetColor((COLORREF) wParam);

	CWnd* pParent = GetParent();
	if (pParent)
	{
		pParent->SendMessage(CPN_CLOSEUP, wParam, (LPARAM) GetDlgCtrlID());
		pParent->SendMessage(CPN_SELENDOK, wParam, (LPARAM) GetDlgCtrlID());
	}

	if (colorOld != m_color && pParent != NULL)
		pParent->SendMessage(CPN_SELCHANGE, wParam, (LPARAM) GetDlgCtrlID());

	return 1;
}

LRESULT CMyColorPicker::OnSelEndCancel(WPARAM wParam, LPARAM lParam)
{
	m_bPopupActive = false;

	Invalidate(false);

	CWnd* pParent = GetParent();
	if (pParent)
	{
		pParent->SendMessage(CPN_CLOSEUP, wParam, (LPARAM) GetDlgCtrlID());
		pParent->SendMessage(CPN_SELENDCANCEL, wParam, (LPARAM) GetDlgCtrlID());
	}

	return 1;
}

LRESULT CMyColorPicker::OnSelChange(WPARAM wParam, LPARAM lParam)
{
	if (m_bTrackSelection) 
		SetColor((COLORREF)wParam);

	CWnd* pParent = GetParent();

	if (pParent)
		pParent->SendMessage(CPN_SELCHANGE, wParam, (LPARAM) GetDlgCtrlID());

	return true;
}

void CMyColorPicker::OnDestroy()
{
	if (m_hThemeButton != NULL)
		XPCloseThemeData(m_hThemeButton);

	m_hThemeButton = NULL;

	CButton::OnDestroy();
}

BOOL CMyColorPicker::OnClicked()
{
	if (m_bPopupActive)
	{
		m_popup.EndSelection(CPN_SELENDCANCEL);
		return true;
	}

	m_bPopupActive = true;

	CRect rcWindow;
	GetWindowRect(rcWindow);

	if (!::IsWindow(m_popup.m_hWnd))
		m_popup.Create(this);

	m_popup.Show(rcWindow, m_color);

	CWnd* pParent = GetParent();
	if (pParent)
		pParent->SendMessage(CPN_DROPDOWN, (WPARAM) m_color, (LPARAM) GetDlgCtrlID());

	return true;
}

void CMyColorPicker::OnThemeChanged()
{
	if (m_hThemeButton != NULL)
		XPCloseThemeData(m_hThemeButton);

	m_hThemeButton = NULL;

	if (IsThemed())
		m_hThemeButton = XPOpenThemeData(m_hWnd, L"BUTTON");

	Invalidate(false);
}

void CMyColorPicker::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_bMouseOver)
	{
		m_bMouseOver = true;

		TRACKMOUSEEVENT tme;

		ZeroMemory(&tme, sizeof(tme));
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		tme.dwHoverTime = HOVER_DEFAULT;

		TrackMouseEvent(&tme);

		Invalidate(false);
	}

	CButton::OnMouseMove(nFlags, point);
}

void CMyColorPicker::OnMouseLeave(void)
{
	if (m_bMouseOver)
	{
		m_bMouseOver = false;

		Invalidate(false);
	}
}

void CMyColorPicker::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	ASSERT(lpDrawItemStruct != NULL);

	if (IsThemed())
	{
		if (m_hThemeButton == NULL)
			m_hThemeButton = XPOpenThemeData(m_hWnd, L"BUTTON");
	}

	UINT nState = lpDrawItemStruct->itemState;

	if (m_bPopupActive)
		nState |= ODS_SELECTED;

	if ((nState & ODS_FOCUS) != NULL)
		nState |= ODS_DEFAULT;

	bool bDisabled = (nState & ODS_DISABLED) != 0;
	bool bSelected = (nState & ODS_SELECTED) != 0;
	bool bFocus = (nState & ODS_FOCUS) != 0;
	bool bDefault = (nState & ODS_DEFAULT) != 0;

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	CRect rcDraw = lpDrawItemStruct->rcItem;
	m_offscreenDC.Create(pDC, rcDraw.Size());
	m_offscreenDC.SetViewportOrg(-rcDraw.TopLeft());

	SendMessage(WM_ERASEBKGND, (WPARAM) m_offscreenDC.m_hDC);

	if (m_hThemeButton != NULL)
	{
		int nFrameState = PBS_NORMAL;
		if (bDisabled)
			nFrameState = PBS_DISABLED;
		else if (bSelected || m_bPopupActive)
			nFrameState = PBS_PRESSED;
		else if ((nState & ODS_HOTLIGHT) != 0 || m_bMouseOver)
			nFrameState = PBS_HOT;
		else if (bDefault)
			nFrameState = PBS_DEFAULTED;

		// Draw outer edge
		XPDrawThemeBackground(m_hThemeButton, m_offscreenDC.m_hDC, BP_PUSHBUTTON, nFrameState, rcDraw, NULL);
		XPGetThemeBackgroundContentRect(m_hThemeButton, m_offscreenDC.m_hDC, BP_PUSHBUTTON, nFrameState, rcDraw, rcDraw);

		if (bFocus)
			m_offscreenDC.DrawFocusRect(rcDraw);
	}
	else
	{
		if (bDefault)
		{
			FrameRect(&m_offscreenDC, rcDraw, ::GetSysColor(COLOR_WINDOWFRAME));
			rcDraw.DeflateRect(1, 1);
		}

		// Draw outer edge
		if (bSelected)
		{
			FrameRect(&m_offscreenDC, rcDraw, ::GetSysColor(COLOR_BTNSHADOW));
		}
		else
		{
			UINT nFrameState = DFCS_BUTTONPUSH | (bDisabled ? DFCS_INACTIVE : 0);
			m_offscreenDC.DrawFrameControl(rcDraw, DFC_BUTTON, nFrameState);
		}

		rcDraw.DeflateRect(2, 2);
		if (!bDefault)
			rcDraw.DeflateRect(1, 1);

		if (bFocus)
			DrawDottedRect(&m_offscreenDC, rcDraw, RGB(0, 0, 0));

		rcDraw.DeflateRect(0, 0, 1, 1);
		if (bSelected)
			rcDraw.OffsetRect(1, 1);
	}

	rcDraw.DeflateRect(2, 2);

	// Draw arrow
	CRect rcArrow;
	rcArrow.left = rcDraw.right - c_nArrowSizeX - ::GetSystemMetrics(SM_CXEDGE) / 2;
	rcArrow.right = rcArrow.left + c_nArrowSizeX;
	rcArrow.top = rcDraw.CenterPoint().y - c_nArrowSizeY / 2;
	rcArrow.bottom = rcDraw.CenterPoint().y + c_nArrowSizeY / 2;

	DrawArrow(&m_offscreenDC, ARR_DOWN, rcArrow, ::GetSysColor(bDisabled ? COLOR_GRAYTEXT : COLOR_WINDOWTEXT));

	rcDraw.right = rcArrow.left - ::GetSystemMetrics(SM_CXEDGE) / 2 - 2;

	// Draw separator
	m_offscreenDC.DrawEdge(rcDraw, EDGE_ETCHED, BF_RIGHT | (m_bFlatMenus ? BF_FLAT : 0));
	rcDraw.right -= ::GetSystemMetrics(SM_CXEDGE) * 2 + 1;

	// Draw color
	if (!bDisabled)
	{
		m_offscreenDC.FillSolidRect(rcDraw, m_color == CLR_DEFAULT ? m_colorDefault : m_color);
		FrameRect(&m_offscreenDC, rcDraw, ::GetSysColor(COLOR_WINDOWTEXT));
	}

	rcDraw = lpDrawItemStruct->rcItem;
	pDC->BitBlt(rcDraw.left, rcDraw.top, rcDraw.Width(), rcDraw.Height(),
			&m_offscreenDC, rcDraw.left, rcDraw.top, SRCCOPY);
	m_offscreenDC.Release();
}

BOOL CMyColorPicker::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (m_bPopupActive)
		{
			switch (pMsg->wParam)
			{
			case VK_UP:
			case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
			case VK_ESCAPE:
			case VK_RETURN:
			case VK_SPACE:
			case VK_F4:
				m_popup.SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
				return true;
			}
		}
		else
		{
			switch (pMsg->wParam)
			{
			case VK_UP:
			case VK_DOWN:
			case VK_F4:
				OnClicked();
				return true;
			}
		}
	}

	return CButton::PreTranslateMessage(pMsg);
}

UINT CMyColorPicker::OnGetDlgCode() 
{
	UINT nCode = CButton::OnGetDlgCode();

	// Tell the dialog that we are a button
	nCode |= (m_bIsDefault ? DLGC_DEFPUSHBUTTON : DLGC_UNDEFPUSHBUTTON);

	return nCode;
}

LRESULT CMyColorPicker::OnSetStyle(WPARAM wParam, LPARAM lParam)
{
	WPARAM nNewType = (wParam & BS_TYPEMASK);

	// Update our state and restore owner draw flag
	if (nNewType == BS_DEFPUSHBUTTON)
	{
		m_bIsDefault = true;
	}
	else if (nNewType == BS_PUSHBUTTON)
	{
		m_bIsDefault = false;
	}

	return DefWindowProc(BM_SETSTYLE, (wParam & ~BS_TYPEMASK) | BS_OWNERDRAW, lParam);
}

void CMyColorPicker::OnKillFocus(CWnd* pNewWnd)
{
	CButton::OnKillFocus(pNewWnd);

	if (m_bPopupActive)
		m_popup.EndSelection(CPN_SELENDCANCEL);
}

void CMyColorPicker::OnCancelMode()
{
	CButton::OnCancelMode();

	if (m_bPopupActive)
		m_popup.EndSelection(CPN_SELENDCANCEL);
}


// CMyColorPopup

const TCHAR* s_pszColorEntry = _T("color");

const int DEFAULT_BOX_VALUE = -3;
const int CUSTOM_BOX_VALUE = -2;
const int INVALID_COLOR = -1;

const CSize s_szTextHiBorder(3, 3);
const CSize s_szTextMargin(2, 2);
const CSize s_szBoxHiBorder(2, 2);
const CSize s_szBoxMargin(0, 0);


CMyColorPopup::CMyColorPopup()
	: m_bFlatMenus(false), m_nColumns(0), m_nRows(0), m_nCurSel(INVALID_COLOR),
	  m_nOrigSel(INVALID_COLOR)
{
	m_nBoxSize = 18;
	m_nMargin = ::GetSystemMetrics(SM_CXEDGE);
	m_color = m_colorOrig = RGB(0, 0, 0);

	// Make sure the color square is at least 5 x 5
	if (m_nBoxSize - 2*m_nMargin - 2 < 5)
		m_nBoxSize = 5 + 2*m_nMargin + 2;

	BOOL bFlatMenus = FALSE;
	if (::SystemParametersInfo(SPI_GETFLATMENU, 0, &bFlatMenus, false))
		m_bFlatMenus = !!bFlatMenus;

	InitColorTable();

	CreateSystemMenuFont(m_font);
}

CMyColorPopup::~CMyColorPopup()
{
}

BEGIN_MESSAGE_MAP(CMyColorPopup, CWnd)
	ON_WM_MOUSEACTIVATE()
    ON_WM_LBUTTONUP()
    ON_WM_PAINT()
    ON_WM_MOUSEMOVE()
    ON_WM_KEYDOWN()
END_MESSAGE_MAP()


// CMyColorPopup message handlers

bool CMyColorPopup::Create(CWnd* pParent)
{
	ASSERT(pParent != NULL && ::IsWindow(pParent->m_hWnd));

	UINT nClassStyle = CS_DBLCLKS;
	BOOL bDropShadow = FALSE;
	::SystemParametersInfo(SPI_GETDROPSHADOW, 0, &bDropShadow, false);
	if (bDropShadow)
		nClassStyle |= CS_DROPSHADOW;

	static CString strWndClass = AfxRegisterWndClass(nClassStyle,
			::LoadCursor(NULL, IDC_ARROW), (HBRUSH) (COLOR_MENU + 1));

	if (!CreateEx(WS_EX_TOOLWINDOW, strWndClass, _T(""),
			WS_POPUP, CRect(0, 0, 100, 100), pParent, 0))
		return false;

	SetOwner(pParent);

	if (m_toolTip.Create(this))
		m_toolTip.Activate(true);

	return true;
}

void CMyColorPopup::InitColorTable()
{
	m_colors.resize(c_nColorTableSize);

	for (size_t i = 0; i < m_colors.size(); ++i)
	{
		m_colors[i].color = colorTable[i].color;
		m_colors[i].strName = colorTable[i].pszName;
	}
}

void CMyColorPopup::InitLayout()
{
	CSize szText(0, 0);

	// If we are showing a custom or default text area, get the font and text size.
	if (!m_strCustomText.IsEmpty() || !m_strDefaultText.IsEmpty())
	{
		CClientDC dc(this);
		CFont* pOldFont = (CFont*) dc.SelectObject(&m_font);

		if (!m_strCustomText.IsEmpty())
			szText = dc.GetTextExtent(m_strCustomText);

		if (!m_strDefaultText.IsEmpty())
		{
			CSize szDefault = dc.GetTextExtent(m_strDefaultText);
			szText.cx = max(szDefault.cx, szText.cx);
			szText.cy = max(szDefault.cy, szText.cy);
		}

		dc.SelectObject(pOldFont);
		szText += CSize(2*m_nMargin, 2*m_nMargin);

		// Add space to draw the horizontal line
		szText.cy += 2*m_nMargin + 2;
	}

	m_nColumns = 8;
	m_nRows = ((int)m_colors.size() + m_nColumns - 1) / m_nColumns;

	// Get the current window position, and set the new size
	m_size.cx = m_nColumns*m_nBoxSize + 2*m_nMargin;
	m_size.cy = m_nRows*m_nBoxSize + 2*m_nMargin;

	// Expand the window to allow for text
	m_rcDefaultText.SetRectEmpty();
	if (!m_strDefaultText.IsEmpty())
	{
		m_size.cx = max(m_size.cx, szText.cx);
		m_size.cy += szText.cy + 2*m_nMargin;

		m_rcDefaultText.SetRect(m_nMargin, m_nMargin, m_size.cx - m_nMargin, 2*m_nMargin + szText.cy);
    }

	m_rcCustomText.SetRectEmpty();
    if (!m_strCustomText.IsEmpty())
    {
		m_size.cx = max(m_size.cx, szText.cx);
		m_size.cy += szText.cy + 2*m_nMargin;

		m_rcCustomText.SetRect(m_nMargin, m_size.cy - 2*m_nMargin - szText.cy,
			m_size.cx - m_nMargin, m_size.cy - m_nMargin);
	}

	// Set boxes rectangle
	int nBoxesWidth = m_nColumns * m_nBoxSize;
	m_rcBoxes.SetRect((m_size.cx - nBoxesWidth) / 2, m_nMargin,
		(m_size.cx - nBoxesWidth) / 2 + nBoxesWidth, m_nMargin + m_nRows*m_nBoxSize);
	if (!m_strDefaultText.IsEmpty())
		m_rcBoxes.OffsetRect(0, m_rcDefaultText.top);
}

void CMyColorPopup::Show(const CRect& rcParent, COLORREF color)
{
	m_color = m_colorOrig = color;

	SetOrigColor(color);

	InitLayout();
	SetPosition(rcParent);

	InitToolTips();

	ShowWindow(SW_SHOWNA);

	SetCapture();
}

BOOL CMyColorPopup::PreTranslateMessage(MSG* pMsg)
{
	m_toolTip.RelayEvent(pMsg);

	if (GetCapture() != this)
		EndSelection(CPN_SELENDCANCEL);

	return CWnd::PreTranslateMessage(pMsg);
}

void CMyColorPopup::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);

	int nNewSel = m_nCurSel;

	switch (nChar)
	{
	case VK_DOWN:
	case VK_RIGHT:
		if (m_nCurSel == DEFAULT_BOX_VALUE)
			nNewSel = 0;
		else if (m_nCurSel == CUSTOM_BOX_VALUE || m_nCurSel == INVALID_COLOR)
		{
			if (!m_strDefaultText.IsEmpty())
				nNewSel = DEFAULT_BOX_VALUE;
			else
				nNewSel = 0;
		}
		else
		{
			nNewSel = m_nCurSel + (nChar == VK_RIGHT ? 1 : m_nColumns);
			if (nNewSel >= static_cast<int>(m_colors.size()))
			{
				if (!m_strCustomText.IsEmpty())
					nNewSel = CUSTOM_BOX_VALUE;
				else if (!m_strDefaultText.IsEmpty())
					nNewSel = DEFAULT_BOX_VALUE;
				else
					nNewSel = 0;
			}
		}
		break;

	case VK_UP:
	case VK_LEFT:
		if (m_nCurSel == DEFAULT_BOX_VALUE || m_nCurSel == INVALID_COLOR)
		{
			if (!m_strCustomText.IsEmpty())
				nNewSel = CUSTOM_BOX_VALUE;
			else
				nNewSel = static_cast<int>(m_colors.size()) - 1;
		}
		else if (m_nCurSel == CUSTOM_BOX_VALUE)
			nNewSel = static_cast<int>(m_colors.size()) - 1;
		else
		{
			nNewSel = m_nCurSel - (nChar == VK_LEFT ? 1 : m_nColumns);
			if (nNewSel < 0)
			{
				if (!m_strDefaultText.IsEmpty())
					nNewSel = DEFAULT_BOX_VALUE;
				else if (!m_strCustomText.IsEmpty())
					nNewSel = CUSTOM_BOX_VALUE;
				else
					nNewSel = static_cast<int>(m_colors.size()) - 1;
            }
        }
		break;

	case VK_ESCAPE:
	case VK_F4:
		EndSelection(CPN_SELENDCANCEL);
        return;

	case VK_RETURN:
	case VK_SPACE:
		EndSelection(CPN_SELENDOK);
		return;
    }

	Select(nNewSel);
}

void CMyColorPopup::OnPaint()
{
	CPaintDC dc(this);

	if (!m_strDefaultText.IsEmpty())
		DrawCell(&dc, DEFAULT_BOX_VALUE);
 
	for (size_t i = 0; i < m_colors.size(); ++i)
		DrawCell(&dc, static_cast<int>(i));
    
	if (!m_strCustomText.IsEmpty())
		DrawCell(&dc, CUSTOM_BOX_VALUE);

    CRect rect = ::GetClientRect(this);
	DrawBorder(&dc, rect, EDGE_RAISED, BF_RECT);
}

void CMyColorPopup::OnMouseMove(UINT nFlags, CPoint point) 
{
	int nNewSel = INVALID_COLOR;

	if (m_rcCustomText.PtInRect(point))
		nNewSel = CUSTOM_BOX_VALUE;
	else if (m_rcDefaultText.PtInRect(point))
		nNewSel = DEFAULT_BOX_VALUE;
	else if (!m_rcBoxes.PtInRect(point))
		nNewSel = INVALID_COLOR;
	else
	{
		CPoint ptAdjusted(point);
		ptAdjusted.Offset(-m_rcBoxes.TopLeft());

		int nRow = ptAdjusted.y / m_nBoxSize;
		int nCol = ptAdjusted.x / m_nBoxSize;
		int nIndex = nRow*m_nColumns + nCol;

		if (nRow >= 0 && nRow < m_nRows && nCol >= 0 && nCol < m_nColumns &&
				nIndex >= 0 && nIndex < static_cast<int>(m_colors.size()))
		{
			nNewSel = nIndex;
		}
	}

	Select(nNewSel);

	CWnd::OnMouseMove(nFlags, point);
}

void CMyColorPopup::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonUp(nFlags, point);

	CRect rcClient = ::GetClientRect(this);
	if (rcClient.PtInRect(point) && m_nCurSel != INVALID_COLOR)
		EndSelection(CPN_SELENDOK);
    else
		EndSelection(CPN_SELENDCANCEL);
}

void CMyColorPopup::SetOrigColor(COLORREF color)
{
	if (color == CLR_DEFAULT && !m_strDefaultText.IsEmpty())
	{
		m_nOrigSel = DEFAULT_BOX_VALUE;
		return;
	}

	for (size_t i = 0; i < m_colors.size(); ++i)
	{
		if (m_colors[i].color == color)
		{
			m_nOrigSel = (int)i;
			return;
		}
	}

	if (!m_strCustomText.IsEmpty())
		m_nOrigSel = CUSTOM_BOX_VALUE;
	else
		m_nOrigSel = INVALID_COLOR;
}

CRect CMyColorPopup::GetCellRect(int nIndex)
{
	if (nIndex == CUSTOM_BOX_VALUE)
		return m_rcCustomText;

	if (nIndex == DEFAULT_BOX_VALUE)
		return m_rcDefaultText;

	ASSERT(nIndex >= 0 && nIndex < static_cast<int>(m_colors.size()));

	CPoint ptTopLeft(m_rcBoxes.left + (nIndex % m_nColumns) * m_nBoxSize,
			m_rcBoxes.top + (nIndex / m_nColumns) * m_nBoxSize);

	return CRect(ptTopLeft, CSize(m_nBoxSize, m_nBoxSize));
}

void CMyColorPopup::SetPosition(const CRect& rcParent)
{
	CPoint ptBottomLeft(CPoint(rcParent.left, rcParent.bottom));
	CRect rcWindow(ptBottomLeft, m_size);

	CRect rcWorkArea = GetMonitorWorkArea(ptBottomLeft);

	if (rcWindow.right > rcWorkArea.right)
		rcWindow.OffsetRect(rcWorkArea.right - rcWindow.right, 0);

    if (rcWindow.left < rcWorkArea.left)
        rcWindow.OffsetRect(rcWorkArea.left - rcWindow.left, 0);

    // If the window does not fit vertically, try to fit it at the top-left
	// point of rcParent. Check if this point is on a different monitor.
	if (rcWindow.bottom > rcWorkArea.bottom)
	{
		CRect rcWindowUp(CPoint(rcParent.left, rcParent.top - m_size.cy), m_size);

		rcWorkArea = GetMonitorWorkArea(rcParent.TopLeft());

		if (rcWindowUp.right > rcWorkArea.right)
			rcWindowUp.OffsetRect(rcWorkArea.right - rcWindowUp.right, 0);

		if (rcWindowUp.left < rcWorkArea.left)
			rcWindowUp.OffsetRect(rcWorkArea.left - rcWindowUp.left, 0);

		if (rcWindowUp.top >= rcWorkArea.top)
			rcWindow = rcWindowUp;
	}

	SetWindowPos(&wndTop, rcWindow.left, rcWindow.top,
			rcWindow.Width(), rcWindow.Height(), SWP_NOACTIVATE);
}

void CMyColorPopup::InitToolTips()
{
	if (!::IsWindow(m_toolTip.m_hWnd))
		return;

	for (size_t i = 0; i < m_colors.size(); ++i)
	{
		CRect rect = GetCellRect(static_cast<int>(i));

		m_toolTip.DelTool(this, i + 1);
		m_toolTip.AddTool(this, m_colors[i].strName, rect, i + 1);
    }
}

void CMyColorPopup::Select(int nIndex)
{
	if (nIndex != INVALID_COLOR && nIndex != CUSTOM_BOX_VALUE && nIndex != DEFAULT_BOX_VALUE
			&& (nIndex < 0 || nIndex > static_cast<int>(m_colors.size())))
		nIndex = CUSTOM_BOX_VALUE;

	if (nIndex == m_nCurSel)
		return;

	CClientDC dc(this);

	if (m_nCurSel != INVALID_COLOR)
	{
		// Redraw old selection
		int nItem = m_nCurSel;
		m_nCurSel = INVALID_COLOR;
		DrawCell(&dc, nItem);
	}

	m_nCurSel = nIndex;
	if (m_nCurSel != INVALID_COLOR)
		DrawCell(&dc, m_nCurSel);

	if (m_nCurSel == CUSTOM_BOX_VALUE || m_nCurSel == INVALID_COLOR)
		m_color = m_colorOrig;
	else if (m_nCurSel == DEFAULT_BOX_VALUE)
		m_color = CLR_DEFAULT;
	else
		m_color = m_colors[m_nCurSel].color;

	GetOwner()->SendMessage(CPN_SELCHANGE, (WPARAM) m_color);
}

void CMyColorPopup::EndSelection(int nMessage)
{
	if (!IsWindowVisible())
		return;

	ReleaseCapture();
	ShowWindow(SW_HIDE);

	if (nMessage != CPN_SELENDCANCEL && m_nCurSel == CUSTOM_BOX_VALUE)
    {
		CColorDialog dlg(m_colorOrig, CC_FULLOPEN | CC_ANYCOLOR, GetOwner());

		COLORREF customColors[16];

		for (int i = 0; i < 16; ++i)
		{
			customColors[i] = 0xffffff;

			if (!m_strRegSection.IsEmpty())
			{
				CString strEntry;
				strEntry.Format(_T("%s%d"), s_pszColorEntry, i);
				customColors[i] = AfxGetApp()->GetProfileInt(m_strRegSection, strEntry, 0xffffff);
			}
		}

		dlg.m_cc.lpCustColors = customColors;

		if (dlg.DoModal() == IDOK)
			m_color = dlg.GetColor();
		else
			nMessage = CPN_SELENDCANCEL;


		if (!m_strRegSection.IsEmpty())
		{
			for (int i = 0; i < 16; ++i)
			{
				CString strEntry;
				strEntry.Format(_T("%s%d"), s_pszColorEntry, i);
				AfxGetApp()->WriteProfileInt(m_strRegSection, strEntry, customColors[i]);
			}
		}
	}

	if (nMessage == CPN_SELENDCANCEL)
		m_color = m_colorOrig;

	GetOwner()->SendMessage(nMessage, (WPARAM) m_color);
}

void CMyColorPopup::DrawCell(CDC* pDC, int nIndex)
{
	if (nIndex == CUSTOM_BOX_VALUE && m_strCustomText.IsEmpty()
			|| nIndex == DEFAULT_BOX_VALUE && m_strDefaultText.IsEmpty())
		return;

	CRect rect = GetCellRect(nIndex);

	CString strText;
	COLORREF clrBox;
	CSize szMargin;
	CSize szHiBorder;
	UINT nBorder = 0;

	if (nIndex == CUSTOM_BOX_VALUE)
	{
		strText = m_strCustomText;
		szMargin = s_szTextMargin;
		szHiBorder = s_szTextHiBorder;
		nBorder = BF_TOP;
	}
	else if (nIndex == DEFAULT_BOX_VALUE)
	{
		strText = m_strDefaultText;
		szMargin = s_szTextMargin;
		szHiBorder = s_szTextHiBorder;
		nBorder = BF_BOTTOM;
	}
	else
	{
		clrBox = m_colors[nIndex].color;
		szMargin = s_szBoxMargin;
		szHiBorder = s_szBoxHiBorder;
	}

	COLORREF clrBackground = ::GetSysColor(COLOR_MENU);
	COLORREF clrMenuText = ::GetSysColor(COLOR_MENUTEXT);
	COLORREF clrHighlightText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);

	COLORREF clrSelection = ::GetSysColor(COLOR_MENUHILIGHT);

	COLORREF clrOrigSelection = AlphaCombine(clrBackground, clrSelection, 50);
	clrSelection = AlphaCombine(clrBackground, clrSelection, 200);

	COLORREF clrFrame;
	COLORREF clrText;
	bool bSelected;
	COLORREF clr3dTopLeft, clr3dBottomRight;

	if (m_nCurSel == nIndex)
	{
		bSelected = true;
		clrFrame = clrSelection;
		clrText = (m_bFlatMenus ? clrHighlightText : clrMenuText);
		clr3dTopLeft = ::GetSysColor(COLOR_BTNHIGHLIGHT);
		clr3dBottomRight = ::GetSysColor(COLOR_3DSHADOW);
	}
	else if (m_nOrigSel == nIndex)
	{
		bSelected = true;
		clrFrame = clrOrigSelection;
		clrText = clrMenuText;
		clr3dTopLeft = ::GetSysColor(COLOR_3DSHADOW);
		clr3dBottomRight = ::GetSysColor(COLOR_3DHIGHLIGHT);
	}
	else
	{
		bSelected = false;
		clrText = clrMenuText;
	}

	pDC->FillSolidRect(rect, clrBackground);
	rect.DeflateRect(szMargin.cx, szMargin.cy);

	if (bSelected)
	{
		if (m_bFlatMenus)
		{
			CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));
			CPen* pOldPen = pDC->SelectObject(&pen);

			CBrush brush(clrFrame);
			CBrush* pOldBrush = pDC->SelectObject(&brush);

			pDC->Rectangle(rect);

			pDC->SelectObject(pOldPen);
			pDC->SelectObject(pOldBrush);
		}
		else
		{
			pDC->Draw3dRect(rect, clr3dTopLeft, clr3dBottomRight);
		}
	}

	rect.DeflateRect(szHiBorder.cx, szHiBorder.cy);

	if (!strText.IsEmpty())
	{
		CFont* pOldFont = (CFont*) pDC->SelectObject(&m_font);
		pDC->SetTextColor(clrText);
		pDC->SetBkMode(TRANSPARENT);
		pDC->DrawText(strText, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		pDC->SelectObject(pOldFont);
	}
	else
	{
		CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));
		CPen* pOldPen = pDC->SelectObject(&pen);

		CBrush brush(m_colors[nIndex].color);
		CBrush* pOldBrush = pDC->SelectObject(&brush);

		rect.DeflateRect(1, 1);
		pDC->Rectangle(rect);

		pDC->SelectObject(pOldPen);
		pDC->SelectObject(pOldBrush);
	}

	if (nBorder != 0)
	{
		CRect r = GetCellRect(nIndex);
		r.InflateRect(-2, 1);
		DrawBorder(pDC, r, EDGE_ETCHED, nBorder);
	}
}

void CMyColorPopup::DrawBorder(CDC* pDC, CRect rect, UINT nEdge, UINT nBorder)
{
	if (m_bFlatMenus)
	{
		CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_GRAYTEXT));
		CPen* pOldPen = pDC->SelectObject(&pen);

		rect.DeflateRect(0, 0, 1, 1);

		if (nBorder & BF_TOP)
		{
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.right + 1, rect.top);
		}

		if (nBorder & BF_BOTTOM)
		{
			pDC->MoveTo(rect.left, rect.bottom);
			pDC->LineTo(rect.right + 1, rect.bottom);
		}

		if (nBorder & BF_LEFT)
		{
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.left, rect.bottom + 1);
		}

		if (nBorder & BF_RIGHT)
		{
			pDC->MoveTo(rect.right, rect.top);
			pDC->LineTo(rect.right, rect.bottom + 1);
		}

		pDC->SelectObject(pOldPen);
	}
	else
		pDC->DrawEdge(rect, nEdge, nBorder);
}

void CMyColorPopup::SetColorNames(UINT nID)
{
	size_t nFromResource = 0;

	if (nID != 0)
	{
		CString strColors;
		if (strColors.LoadString(nID))
		{
			strColors += _T(",");

			int nPos = -1, nNextPos = 0;
			size_t nColor = 0;

			while (nColor < m_colors.size() && (nNextPos = strColors.Find(',', nPos)) != -1)
			{
				m_colors[nColor].strName = strColors.Mid(nPos + 1, nNextPos - nPos);
				nPos = nNextPos;
			}

			nFromResource = nColor;
		}
	}

	// Set default names for colors missing in the resource string
	for (size_t i = nFromResource; i < m_colors.size(); ++i)
		m_colors[i].strName = colorTable[i].pszName;
}

void CMyColorPopup::SetRegSection(LPCTSTR pszRegSection)
{
	m_strRegSection = pszRegSection;
}

int CMyColorPopup::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	return MA_NOACTIVATE;
}

void CMyColorPopup::SetCustomText(LPCTSTR pszText)
{
	m_strCustomText = pszText;
}

void CMyColorPopup::SetDefaultText(LPCTSTR pszText)
{
	m_strDefaultText = pszText;
}
