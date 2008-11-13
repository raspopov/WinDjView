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

#include "stdafx.h"
#include "WinDjView.h"
#include "MyToolBar.h"
#include "Drawing.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMyToolBar

IMPLEMENT_DYNAMIC(CMyToolBar, CToolBar)
CMyToolBar::CMyToolBar()
	: m_hTheme(NULL)
{
}

CMyToolBar::~CMyToolBar()
{
}


BEGIN_MESSAGE_MAP(CMyToolBar, CToolBar)
	ON_WM_CREATE()
	ON_WM_NCPAINT()
	ON_MESSAGE_VOID(WM_THEMECHANGED, OnThemeChanged)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CMyToolBar message handlers

int CMyToolBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (IsThemed())
		m_hTheme = XPOpenThemeData(m_hWnd, L"TOOLBAR");

	theApp.AddObserver(this);

	return 0;
}

void CMyToolBar::OnDestroy()
{
	theApp.RemoveObserver(this);

	if (m_hTheme != NULL)
	{
		XPCloseThemeData(m_hTheme);
		m_hTheme = NULL;
	}

	CToolBar::OnDestroy();
}

void CMyToolBar::SetSizes(SIZE sizeButton, SIZE sizeImage)
{
	CToolBar::SetSizes(sizeButton, sizeImage);
	m_sizeButton = sizeButton;
	m_sizeImage = sizeImage;
}

void CMyToolBar::EraseNonClient()
{
	// Copied from MFC's CControlBar::EraseNonClient

	// get window DC that is clipped to the non-client area
	CWindowDC dc(this);
	CRect rectClient;
	GetClientRect(rectClient);
	CRect rectWindow;
	GetWindowRect(rectWindow);
	ScreenToClient(rectWindow);
	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dc.ExcludeClipRect(rectClient);

	// draw borders in non-client area
	rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);
	DrawBorders(&dc, rectWindow);

	// erase parts not drawn
	dc.IntersectClipRect(rectWindow);
	SendMessage(WM_ERASEBKGND, (WPARAM)dc.m_hDC);

	// draw gripper in non-client area
	DrawGripper(&dc, rectWindow);
}

void CMyToolBar::DoPaint(CDC* pDC)
{
	// Copied from MFC's CControlBar::DoPaint

	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	// paint inside client area
	CRect rect;
	GetClientRect(rect);
	DrawBorders(pDC, rect);
	DrawGripper(pDC, rect);
}

void CMyToolBar::DrawBorders(CDC* pDC, CRect& rect)
{
	// MFC fix (original code from CControlBar::DrawBorders)

	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	DWORD dwStyle = m_dwStyle;
	if (!(dwStyle & CBRS_BORDER_ANY))
		return;

	// prepare for dark lines
	ASSERT(rect.top == 0 && rect.left == 0);
	CRect rect1, rect2;
	rect1 = rect;
	rect2 = rect;
	COLORREF clr = afxData.clrBtnShadow;

	// draw dark line one pixel back/up
	if (dwStyle & CBRS_BORDER_3D)
	{
		rect1.right -= CX_BORDER;
		rect1.bottom -= CY_BORDER;
	}
	if (dwStyle & CBRS_BORDER_TOP)
		rect2.top += afxData.cyBorder2;
	if (dwStyle & CBRS_BORDER_BOTTOM)
		rect2.bottom -= afxData.cyBorder2;

	if (dwStyle & CBRS_BORDER_ANY)
	{
		CRect rctBk;
		rctBk.left = 0;
		rctBk.right = rect.right;
		rctBk.top = rect.top;
		rctBk.bottom = rect.bottom;

		if (m_hTheme != NULL)
		{
			if (GetStyle() & TBSTYLE_TRANSPARENT)
			{
				XPDrawThemeParentBackground(m_hWnd, pDC->m_hDC, rctBk);
			}
			else
			{
				if (XPIsThemeBackgroundPartiallyTransparent(m_hTheme, 0, 0))
					XPDrawThemeParentBackground(m_hWnd, pDC->m_hDC, rctBk);
				XPDrawThemeBackground(m_hTheme, pDC->m_hDC, 0, 0, rctBk, rctBk);
			}
		}
		else
		{
			pDC->FillSolidRect((LPRECT)rctBk, ::GetSysColor(COLOR_BTNFACE));
		}
	}

	// draw left and top
	if (dwStyle & CBRS_BORDER_LEFT)
		pDC->FillSolidRect(0, rect2.top, CX_BORDER, rect2.Height(), clr);
	if (dwStyle & CBRS_BORDER_TOP)
		pDC->FillSolidRect(0, 0, rect.right, CY_BORDER, clr);

	// draw right and bottom
	if (dwStyle & CBRS_BORDER_RIGHT)
		pDC->FillSolidRect(rect1.right, rect2.top, -CX_BORDER, rect2.Height(), clr);
	if (dwStyle & CBRS_BORDER_BOTTOM)
		pDC->FillSolidRect(0, rect1.bottom, rect.right, -CY_BORDER, clr);

	if (dwStyle & CBRS_BORDER_3D)
	{
		// prepare for hilite lines
		clr = afxData.clrBtnHilite;

		// draw left and top
		if (dwStyle & CBRS_BORDER_LEFT)
			pDC->FillSolidRect(1, rect2.top, CX_BORDER, rect2.Height(), clr);
		if (dwStyle & CBRS_BORDER_TOP)
			pDC->FillSolidRect(0, 1, rect.right, CY_BORDER, clr);

		// draw right and bottom
		if (dwStyle & CBRS_BORDER_RIGHT)
			pDC->FillSolidRect(rect.right, rect2.top, -CX_BORDER, rect2.Height(), clr);
		if (dwStyle & CBRS_BORDER_BOTTOM)
			pDC->FillSolidRect(0, rect.bottom, rect.right, -CY_BORDER, clr);
	}

	if (dwStyle & CBRS_BORDER_LEFT)
		rect.left += afxData.cxBorder2;
	if (dwStyle & CBRS_BORDER_TOP)
		rect.top += afxData.cyBorder2;
	if (dwStyle & CBRS_BORDER_RIGHT)
		rect.right -= afxData.cxBorder2;
	if (dwStyle & CBRS_BORDER_BOTTOM)
		rect.bottom -= afxData.cyBorder2;
}

void CMyToolBar::OnNcPaint()
{
	EraseNonClient();
}

void CMyToolBar::OnThemeChanged()
{
	if (m_hTheme != NULL)
		XPCloseThemeData(m_hTheme);

	m_hTheme = NULL;

	if (IsThemed())
		m_hTheme = XPOpenThemeData(m_hWnd, L"TOOLBAR");
}

void CMyToolBar::OnRButtonDown(UINT nFlags, CPoint point)
{
	// Just eat the message. Standard windows toolbar does some strange things after right-clicking.
}

void CMyToolBar::OnRButtonUp(UINT nFlags, CPoint point)
{
	// Just eat the message. Standard windows toolbar does some strange things after right-clicking.
}

void CMyToolBar::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTBCUSTOMDRAW pTBCD = (LPNMTBCUSTOMDRAW) pNMHDR;

	if (pTBCD->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
		return;
	}

	if (pTBCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		for (size_t nLabel = 0; nLabel < m_labels.size(); ++nLabel)
		{
			if (pTBCD->nmcd.dwItemSpec == m_labels[nLabel].nID)
			{
				Label& label = m_labels[nLabel];

				CDC dc;
				dc.Attach(pTBCD->nmcd.hdc);

				CFont* pOldFont = dc.SelectObject(GetFont());
				COLORREF crTextColor = dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
				int nBkMode = dc.SetBkMode(TRANSPARENT);

				UINT nFlags = DT_LEFT | DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE;
				CRect rect(pTBCD->nmcd.rc);
				rect.left += 3;
				dc.DrawText(label.strText, rect, nFlags);

				dc.SetTextColor(crTextColor);
				dc.SetBkMode(nBkMode);
				dc.SelectObject(pOldFont);

				dc.Detach();

				*pResult = CDRF_SKIPDEFAULT;
				return;
			}
		}
	}

	*pResult = 0;
}

void CMyToolBar::InsertLabel(int nPos, UINT nID, CFont* pFont)
{
	Label label;
	label.nID = nID;
	label.strText = LoadString(nID);
	label.pFont = pFont;
	m_labels.push_back(label);

	CScreenDC dcScreen;
	CFont* pOldFont = dcScreen.SelectObject(pFont);
	int nWidth = dcScreen.GetTextExtent(label.strText).cx + 3;
	dcScreen.SelectObject(pOldFont);

	TBBUTTON btn;
	ZeroMemory(&btn, sizeof(btn));
	btn.idCommand = nID;
	btn.fsStyle = TBSTYLE_BUTTON;

	TBBUTTONINFO info;
	ZeroMemory(&info, sizeof(info));
	info.cbSize = sizeof(info);
	info.dwMask = TBIF_SIZE;
	info.cx = nWidth;

	GetToolBarCtrl().InsertButton(nPos, &btn);
	GetToolBarCtrl().SetButtonInfo(nID, &info);
}

void CMyToolBar::OnUpdate(const Observable* source, const Message* message)
{
	if (message->code == APP_LANGUAGE_CHANGED)
	{
		CScreenDC dcScreen;
		for (size_t nLabel = 0; nLabel < m_labels.size(); ++nLabel)
		{
			Label& label = m_labels[nLabel];
			label.strText = LoadString(label.nID);

			CFont* pOldFont = dcScreen.SelectObject(label.pFont);
			int nWidth = dcScreen.GetTextExtent(label.strText).cx + 3;
			dcScreen.SelectObject(pOldFont);

			TBBUTTONINFO info;
			ZeroMemory(&info, sizeof(info));
			info.cbSize = sizeof(info);
			info.dwMask = TBIF_SIZE;
			info.cx = nWidth;

			GetToolBarCtrl().SetButtonInfo(label.nID, &info);
		}

		if (!m_labels.empty())
		{
			for (int nButton = 0; nButton < GetToolBarCtrl().GetButtonCount(); ++nButton)
			{
				TBBUTTON btn;
				ZeroMemory(&btn, sizeof(btn));
				GetToolBarCtrl().GetButton(nButton, &btn);
				CWnd* pWnd = GetDlgItem(btn.idCommand);

				if (btn.fsStyle == TBSTYLE_SEP && pWnd != NULL)
				{
					CRect rcItem;
					GetItemRect(nButton, rcItem);
					rcItem.DeflateRect(3, 0);

					pWnd->MoveWindow(rcItem);
				}
			}

			GetToolBarCtrl().AutoSize();
			Invalidate(false);
		}
	}
}
