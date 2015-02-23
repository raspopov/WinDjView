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

#include "stdafx.h"
#include "WinDjView.h"
#include "TabbedMDIWnd.h"
#include "Drawing.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static const int s_nSideMargin = 3;
static const int s_nBottomMargin = 2;
static const int s_nTopMargin = 3;
static const int s_nPadding = 9;
static const int s_nBtnPadding = 7;
static const int s_nCloseBtnSize = 14;
static const int s_nTextBtnPadding = 2;
static const int s_nMaxTabWidth = 250;
static const int s_nMinTabWidth = 100;
static const int s_nMinTabCloseWidth = 140;
static const int s_nArrowWidth = 10;
static const int s_nArrowSizeX = 2;
static const int s_nArrowSizeY = 4;

// CTabbedMDIWnd

IMPLEMENT_DYNCREATE(CTabbedMDIWnd, CWnd)

CTabbedMDIWnd::CTabbedMDIWnd()
	: m_nActiveTab(-1), m_nHoverTab(-1), m_nScrollPos(0), m_bShowArrows(false),
	  m_bHoverLeft(false), m_bHoverRight(false), m_bHoverClose(false),
	  m_nClosePressedTab(-1), m_nMButtonPressedTab(-1), m_bIgnoreMouseLeave(false)
{
	UpdateMetrics();
	m_rcContent.SetRectEmpty();

	m_bTabBarHidden = theApp.m_bTopLevelDocs;
	m_strCloseTab = LoadString(IDS_CLOSE_TAB);
}

CTabbedMDIWnd::~CTabbedMDIWnd()
{
}


BEGIN_MESSAGE_MAP(CTabbedMDIWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE_VOID(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETTINGCHANGE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_MENUSELECT()
	ON_WM_ENTERIDLE()
END_MESSAGE_MAP()


// CTabbedMDIWnd message handlers

void CTabbedMDIWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	COLORREF clrBtnface = ::GetSysColor(COLOR_BTNFACE);
	COLORREF clrBackground = ChangeBrightness(clrBtnface, 0.85);
	COLORREF clrBtnshadow = ::GetSysColor(COLOR_BTNSHADOW);

	if (m_tabs.empty())
	{
		CRect rcTopLine(0, 0, m_szTabBar.cx, 1);
		dc.FillSolidRect(rcTopLine, clrBtnshadow);
		dc.FillSolidRect(0, 1, m_rcContent.right, m_rcContent.bottom, ::GetSysColor(COLOR_APPWORKSPACE));
		return;
	}
	else if (m_bTabBarHidden)
	{
		return;
	}

	// Draw tabs offscreen
	m_offscreenDC.Create(&dc, m_szTabBar);
	CDC* pDC = &m_offscreenDC;

	CRect rcTopLine(0, 0, m_szTabBar.cx, 1);
	pDC->FillSolidRect(rcTopLine, clrBtnshadow);
	CRect rcTop(0, 1, m_szTabBar.cx, s_nTopMargin);
	pDC->FillSolidRect(rcTop, clrBackground);
	CRect rcBottom(0, rcTop.bottom + m_nTabHeight, m_szTabBar.cx, m_szTabBar.cy);
	pDC->FillSolidRect(rcBottom, clrBtnface);

	if (m_bShowArrows)
	{
		int nScrollWidth = m_szTabBar.cx - 2*s_nArrowWidth;
		bool bLeftEnabled = (m_nScrollPos > 0);
		bool bRightEnabled = (m_nScrollPos < m_tabs.back().rcTab.right - nScrollWidth);

		CRect rcLeftArrow = m_rcLeftArrow;
		rcLeftArrow.left = -2;
		DrawInactiveTabRect(pDC, rcLeftArrow, true, bLeftEnabled, m_bHoverLeft);
		CRect rcRightArrow = m_rcRightArrow;
		rcRightArrow.right = m_szTabBar.cx + 2;
		DrawInactiveTabRect(pDC, rcRightArrow, true, bRightEnabled, m_bHoverRight);

		CPoint ptLeft = rcLeftArrow.CenterPoint() - CPoint(s_nArrowSizeX / 2, s_nArrowSizeY / 2);
		CRect rcLeft(ptLeft, CSize(s_nArrowSizeX, s_nArrowSizeY));
		DrawArrow(pDC, ARR_LEFT, rcLeft, ::GetSysColor(bLeftEnabled ? COLOR_WINDOWTEXT : COLOR_BTNSHADOW));

		CPoint ptRight = rcRightArrow.CenterPoint() - CPoint(s_nArrowSizeX / 2, s_nArrowSizeY / 2);
		CRect rcRight(ptRight, CSize(s_nArrowSizeX, s_nArrowSizeY));
		DrawArrow(pDC, ARR_RIGHT, rcRight, ::GetSysColor(bRightEnabled ? COLOR_WINDOWTEXT : COLOR_BTNSHADOW));

		pDC->IntersectClipRect(s_nArrowWidth, rcTop.bottom, m_szTabBar.cx - s_nArrowWidth, rcBottom.top);
		pDC->SetViewportOrg(s_nArrowWidth - m_nScrollPos, 0);
	}
	else
	{
		CRect rcLeft(0, s_nTopMargin, s_nSideMargin, rcBottom.top - 1);
		pDC->FillSolidRect(rcLeft, clrBackground);
		CRect rcLeftLine(rcLeft.left, rcBottom.top - 1, rcLeft.right, rcBottom.top);
		pDC->FillSolidRect(rcLeftLine, clrBtnshadow);
	}

	for (int nTab = 0; nTab < (int)m_tabs.size(); ++nTab)
		DrawTab(pDC, nTab);

	if (!m_bShowArrows)
	{
		int nRight = m_tabs.back().rcTab.right;
		if (nRight < m_szTabBar.cx)
		{
			CRect rcRight(nRight, s_nTopMargin, m_szTabBar.cx, rcBottom.top - 1);
			pDC->FillSolidRect(rcRight, clrBackground);
			CRect rcRightLine(rcRight.left, rcBottom.top - 1, rcRight.right, rcBottom.top);
			pDC->FillSolidRect(rcRightLine, clrBtnshadow);
		}
	}

	if (m_nActiveTab == -1)
		dc.FillSolidRect(m_rcContent, ::GetSysColor(COLOR_APPWORKSPACE));

	// Flush bitmap to screen dc
	pDC->SetViewportOrg(0, 0);
	dc.BitBlt(0, 0, m_szTabBar.cx, m_szTabBar.cy, pDC, 0, 0, SRCCOPY);
	m_offscreenDC.Release();
}

void CTabbedMDIWnd::DrawTab(CDC* pDC, int nTab)
{
	Tab& tab = m_tabs[nTab];

	if (nTab == m_nActiveTab)
		DrawActiveTabRect(pDC, tab.rcTab);
	else if (nTab == m_nHoverTab)
		DrawActiveTabRect(pDC, tab.rcTab, true);
	else
		DrawInactiveTabRect(pDC, tab.rcTab);

	COLORREF clrText = ::GetSysColor(COLOR_WINDOWTEXT);
	CRect rcText(tab.rcTab.left + s_nPadding, tab.rcTab.top + 2,
			tab.rcTab.right - s_nPadding, tab.rcTab.bottom);

	// Draw close button
	if (HasCloseButton(nTab))
	{
		if (nTab == m_nClosePressedTab)
		{
			if (m_bHoverClose && nTab == m_nHoverTab)
				m_imgClose.Draw(pDC, 3, tab.rcClose.TopLeft(), ILD_NORMAL);
			else
				m_imgClose.Draw(pDC, 0, tab.rcClose.TopLeft(), ILD_NORMAL);
		}
		else if (m_bHoverClose && nTab == m_nHoverTab)
			m_imgClose.Draw(pDC, 2, tab.rcClose.TopLeft(), ILD_NORMAL);
		else if (nTab == m_nActiveTab)
			m_imgClose.Draw(pDC, 0, tab.rcClose.TopLeft(), ILD_NORMAL);
		else
			m_imgClose.Draw(pDC, 1, tab.rcClose.TopLeft(), ILD_NORMAL);

		rcText.right = tab.rcTab.right - s_nBtnPadding - s_nCloseBtnSize - s_nTextBtnPadding;
	}

	CFont* pOldFont = pDC->SelectObject(nTab == m_nActiveTab ? &m_fontActive : &m_font);
	pDC->SetTextColor(clrText);
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(tab.strName, rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
	pDC->SelectObject(pOldFont);
}

void CTabbedMDIWnd::DrawActiveTabRect(CDC* pDC, const CRect& rect, bool bHover)
{
	COLORREF clrBtnface = ::GetSysColor(COLOR_BTNFACE);
	COLORREF clrTabStripBg = ChangeBrightness(clrBtnface, 0.88);
	COLORREF clrBtnshadow = ::GetSysColor(COLOR_BTNSHADOW);
	COLORREF clrActiveTabTopBg = ChangeBrightness(clrBtnface, 1.04);
	COLORREF clrHoverTabTopBg = ChangeBrightness(clrBtnface, 0.97);
	COLORREF clrHoverTabBottomBg = ChangeBrightness(clrBtnface, 0.94);
	COLORREF clrActiveShadow = ChangeBrightness(clrBtnshadow, 1.12);
	COLORREF clrActiveHilight = ChangeBrightness(clrBtnface, 0.9);

	CPen penBtnshadow(PS_SOLID, 1, clrBtnshadow);
	CPen* pOldPen = pDC->SelectObject(&penBtnshadow);
	pDC->MoveTo(rect.left, rect.top + 3);
	pDC->LineTo(rect.left, rect.bottom);
	pDC->MoveTo(rect.left + 3, rect.top);
	pDC->LineTo(rect.right - 3, rect.top);
	pDC->MoveTo(rect.right - 1, rect.top + 3);
	pDC->LineTo(rect.right - 1, rect.bottom);

	CRect rcTopBg(rect.left + 1, rect.top + 1, rect.right - 1, rect.CenterPoint().y);
	CRect rcBottomBg(rcTopBg.left, rcTopBg.bottom, rcTopBg.right, rect.bottom);

	if (!bHover)
	{
		pDC->FillSolidRect(rcTopBg, clrActiveTabTopBg);
		pDC->FillSolidRect(rcBottomBg, clrBtnface);
	}
	else
	{
		rcBottomBg.bottom = rect.bottom - 1;
		pDC->FillSolidRect(rcTopBg, clrHoverTabTopBg);
		pDC->FillSolidRect(rcBottomBg, clrHoverTabBottomBg);

		pDC->MoveTo(rect.left + 1, rect.bottom - 1);
		pDC->LineTo(rect.right - 1, rect.bottom - 1);
	}

	pDC->SelectObject(pOldPen);

	pDC->SetPixel(rect.left, rect.top, clrTabStripBg);
	pDC->SetPixel(rect.left + 1, rect.top, clrTabStripBg);
	pDC->SetPixel(rect.left, rect.top + 1, clrTabStripBg);
	pDC->SetPixel(rect.left + 1, rect.top + 1, clrBtnshadow);
	pDC->SetPixel(rect.left, rect.top + 2, clrActiveShadow);
	pDC->SetPixel(rect.left + 2, rect.top, clrActiveShadow);
	pDC->SetPixel(rect.left + 1, rect.top + 2, clrActiveHilight);
	pDC->SetPixel(rect.left + 2, rect.top + 1, clrActiveHilight);

	pDC->SetPixel(rect.right - 1, rect.top, clrTabStripBg);
	pDC->SetPixel(rect.right - 2, rect.top, clrTabStripBg);
	pDC->SetPixel(rect.right - 1, rect.top + 1, clrTabStripBg);
	pDC->SetPixel(rect.right - 2, rect.top + 1, clrBtnshadow);
	pDC->SetPixel(rect.right - 1, rect.top + 2, clrActiveShadow);
	pDC->SetPixel(rect.right - 3, rect.top, clrActiveShadow);
	pDC->SetPixel(rect.right - 2, rect.top + 2, clrActiveHilight);
	pDC->SetPixel(rect.right - 3, rect.top + 1, clrActiveHilight);
}

void CTabbedMDIWnd::DrawInactiveTabRect(CDC* pDC, const CRect& rect,
		bool bArrow, bool bArrowEnabled, bool bArrowHover)
{
	COLORREF clrBtnface = ::GetSysColor(COLOR_BTNFACE);
	COLORREF clrTabStripBg = ChangeBrightness(clrBtnface, 0.88);
	COLORREF clrBtnshadow = ::GetSysColor(COLOR_BTNSHADOW);
	COLORREF clrShadow = ChangeBrightness(clrBtnshadow, 1.1);
	COLORREF clrHilight = ChangeBrightness(clrBtnface, 0.95);
	COLORREF clrTabTopBg = ChangeBrightness(clrBtnface, 0.91);
	COLORREF clrTabBottomBg = ChangeBrightness(clrBtnface, 0.93);
	COLORREF clrHoverTabTopBg = ChangeBrightness(clrBtnface, 0.97);
	COLORREF clrHoverTabBottomBg = ChangeBrightness(clrBtnface, 0.94);

	CPen penTabStrip(PS_SOLID, 1, clrTabStripBg);
	CPen* pOldPen = pDC->SelectObject(&penTabStrip);
	pDC->MoveTo(rect.left, rect.top);
	pDC->LineTo(rect.right, rect.top);

	CPen penBtnshadow(PS_SOLID, 1, clrBtnshadow);
	CPen penShadow(PS_SOLID, 1, clrShadow);

	if (!bArrow || bArrowEnabled)
		pDC->SelectObject(&penBtnshadow);
	else
		pDC->SelectObject(&penShadow);

	pDC->MoveTo(rect.left, rect.top + 2);
	pDC->LineTo(rect.left, rect.bottom - 1);
	pDC->MoveTo(rect.left + 1, rect.top + 1);
	pDC->LineTo(rect.right - 1, rect.top + 1);
	pDC->MoveTo(rect.left, rect.bottom - 1);
	pDC->LineTo(rect.right, rect.bottom - 1);

	if (!bArrow)
		pDC->SelectObject(&penShadow);

	pDC->MoveTo(rect.right - 1, rect.top + 2);
	pDC->LineTo(rect.right - 1, rect.bottom - 1);

	CPen penHilight(PS_SOLID, 1, clrHilight);
	pDC->SelectObject(&penHilight);
	pDC->MoveTo(rect.left + 1, rect.top + 3);
	pDC->LineTo(rect.left + 1, rect.bottom - 1);
	pDC->MoveTo(rect.left + 2, rect.top + 2);
	pDC->LineTo(rect.right - 1, rect.top + 2);

	pDC->SelectObject(pOldPen);

	if (!bArrow || !bArrowEnabled || !bArrowHover)
	{
		CRect rcTopBg(rect.left + 2, rect.top + 3, rect.right - 1, rect.CenterPoint().y + 4);
		CRect rcBottomBg(rcTopBg.left, rcTopBg.bottom, rcTopBg.right, rect.bottom - 1);
		pDC->FillSolidRect(rcTopBg, clrTabTopBg);
		pDC->FillSolidRect(rcBottomBg, clrTabBottomBg);
	}
	else
	{
		CRect rcTopBg(rect.left + 2, rect.top + 3, rect.right - 1, rect.CenterPoint().y);
		CRect rcBottomBg(rcTopBg.left, rcTopBg.bottom, rcTopBg.right, rect.bottom - 1);
		pDC->FillSolidRect(rcTopBg, clrHoverTabTopBg);
		pDC->FillSolidRect(rcBottomBg, clrHoverTabBottomBg);
	}

	pDC->SetPixel(rect.left, rect.top + 1, clrTabStripBg);
	pDC->SetPixel(rect.right - 1, rect.top + 1, clrTabStripBg);
	pDC->SetPixel(rect.left + 1, rect.top + 2, clrTabTopBg);
}

void CTabbedMDIWnd::UpdateMetrics()
{
	CreateSystemDialogFont(m_font);

	LOGFONT lf;
	m_font.GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;

	if (m_fontActive.m_hObject != NULL)
		m_fontActive.DeleteObject();
	m_fontActive.CreateFontIndirect(&lf);

	CScreenDC dcScreen;
	CFont* pOldFont = dcScreen.SelectObject(&m_font);
	TEXTMETRIC tm;
	CSize szText = dcScreen.GetTextMetrics(&tm);
	dcScreen.SelectObject(pOldFont);

	m_nTabHeight = tm.tmHeight + 11;
}

void CTabbedMDIWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	UpdateSize(cx, cy);
}

void CTabbedMDIWnd::UpdateSize(int cx, int cy)
{
	if (cx <= 0 || cy <= 0)
		return;

	m_szTabBar = CSize(cx, s_nTopMargin + s_nBottomMargin + m_nTabHeight);

	if (!m_bTabBarHidden)
		m_rcContent = CRect(0, m_szTabBar.cy, cx, max(m_szTabBar.cy, cy));
	else
		m_rcContent = CRect(0, 0, cx, cy);

	UpdateTabRects();
	UpdateScrollState();
	UpdateToolTips();
	UpdateHoverTab();

	if (m_nActiveTab != -1)
	{
		m_tabs[m_nActiveTab].pWnd->SetWindowPos(NULL, m_rcContent.left, m_rcContent.top,
				m_rcContent.Width(), m_rcContent.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	}

	InvalidateTabs();
}

void CTabbedMDIWnd::UpdateScrollState()
{
	if (!m_bShowArrows)
	{
		m_nScrollPos = 0;
		return;
	}

	ASSERT(!m_tabs.empty());
	int nScrollWidth = m_szTabBar.cx - 2*s_nArrowWidth;
	int nTabsWidth = m_tabs.back().rcTab.right;
	int nScrollPos = max(0, min(nTabsWidth - nScrollWidth, m_nScrollPos));
	if (nScrollPos != m_nScrollPos)
	{
		m_nScrollPos = nScrollPos;
		InvalidateTabs();
	}
}

int CTabbedMDIWnd::AddTab(CWnd* pWnd, const CString& strName, const CString& strPathName)
{
	int nTab = FindTab(strPathName);
	if (nTab != -1)
	{
		CWnd* pWnd = m_tabs[nTab].pWnd;
		pWnd->DestroyWindow();
		m_tabs[nTab].bReactivate = true;
	}
	else
	{
		m_tabs.push_back(Tab());
		nTab = (int)m_tabs.size() - 1;
		m_tabs[nTab].bReactivate = false;
	}

	m_tabs[nTab].pWnd = pWnd;
	m_tabs[nTab].strName = strName;
	m_tabs[nTab].strPathName = strPathName;

	UpdateTabRects();
	UpdateScrollState();
	UpdateToolTips();
	UpdateHoverTab();

	pWnd->ShowWindow(SW_HIDE);
	pWnd->SetParent(this);
	pWnd->SetWindowPos(NULL, m_rcContent.left, m_rcContent.top,
				m_rcContent.Width(), m_rcContent.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

	InvalidateTabs();

	return nTab;
}

void CTabbedMDIWnd::CloseTab(CWnd* pWnd)
{
	int nTab = TabFromFrame(pWnd);
	if (nTab == -1)
		return;

	CloseTab(nTab);
}

int CTabbedMDIWnd::FindTab(const CString& strPathName)
{
	for (int nTab = 0; nTab < (int)m_tabs.size(); ++nTab)
	{
		if (AfxComparePath(strPathName, m_tabs[nTab].strPathName))
			return nTab;
	}
	return -1;
}

CString CTabbedMDIWnd::GetTabName(int nTab) const
{
	if (nTab < 0 || nTab >= GetTabCount())
		return _T("");

	return m_tabs[nTab].strName;
}

CString CTabbedMDIWnd::GetTabPathName(int nTab) const
{
	if (nTab < 0 || nTab >= GetTabCount())
		return _T("");

	return m_tabs[nTab].strPathName;
}

void CTabbedMDIWnd::CloseTab(int nTab, bool bRedraw)
{
	bool bWasActive = false;
	if (m_nActiveTab == nTab)
	{
		bWasActive = true;
		int nNewActive = (m_nActiveTab == m_tabs.size() - 1 ? m_nActiveTab - 1 : m_nActiveTab + 1);
		ActivateTabImpl(nNewActive, false);
	}

	CWnd* pWnd = m_tabs[nTab].pWnd;
	m_tabs.erase(m_tabs.begin() + nTab);

	UpdateTabRects();
	UpdateScrollState();
	UpdateToolTips();
	UpdateHoverTab();

	if (m_nActiveTab > nTab)
		--m_nActiveTab;

	if (bWasActive && m_nActiveTab != -1)
		EnsureVisible(m_nActiveTab);

	InvalidateTabs();
	if (m_nActiveTab == -1)
		Invalidate();

	if (m_nClosePressedTab == nTab)
		m_nClosePressedTab = -1;
	else if (m_nClosePressedTab > nTab)
		--m_nClosePressedTab;

	if (m_nMButtonPressedTab == nTab)
		m_nMButtonPressedTab = -1;
	else if (m_nMButtonPressedTab > nTab)
		--m_nMButtonPressedTab;

	UpdateObservers(TabMsg(TAB_CLOSED, pWnd, -1));
	pWnd->DestroyWindow();

	if (bRedraw)
		UpdateWindow();
}

void CTabbedMDIWnd::UpdateTabRects()
{
	if (m_tabs.empty() || m_bTabBarHidden)
	{
		m_bShowArrows = false;
		return;
	}

	int nLeft = s_nSideMargin;
	int nTop = s_nTopMargin;
	int nBottom = nTop + m_nTabHeight;

	int nTabArea = m_szTabBar.cx - 2*s_nSideMargin;
	int nTabWidth = max(s_nMinTabWidth, min(s_nMaxTabWidth, nTabArea / (int)m_tabs.size()));

	if (nTabWidth * static_cast<int>(m_tabs.size()) > nTabArea)
	{
		m_bShowArrows = true;
		nLeft = 0;

		m_rcLeftArrow = CRect(0, s_nTopMargin, s_nArrowWidth, s_nTopMargin + m_nTabHeight);
		m_rcRightArrow = CRect(m_szTabBar.cx - s_nArrowWidth, s_nTopMargin,
				m_szTabBar.cx, s_nTopMargin + m_nTabHeight);
	}
	else
	{
		m_bShowArrows = false;
	}

	for (size_t nTab = 0; nTab < m_tabs.size(); ++nTab)
	{
		Tab& tab = m_tabs[nTab];

		tab.rcTab.left = nLeft;
		tab.rcTab.right = nLeft + nTabWidth;
		tab.rcTab.top = nTop;
		tab.rcTab.bottom = nBottom;

		tab.rcClose.left = tab.rcTab.right - s_nCloseBtnSize - s_nBtnPadding;
		tab.rcClose.right = tab.rcClose.left + s_nCloseBtnSize;
		tab.rcClose.top = tab.rcTab.CenterPoint().y - s_nCloseBtnSize / 2 + 1;
		tab.rcClose.bottom = tab.rcClose.top + s_nCloseBtnSize;

		nLeft += nTabWidth;
	}
}

void CTabbedMDIWnd::UpdateToolTips()
{
	if (m_bTabBarHidden)
		return;

	m_toolTip.Activate(false);

	for (int nTool = m_toolTip.GetToolCount() - 1; nTool >= 0; --nTool)
		m_toolTip.DelTool(this, nTool + 1);

	CRect rcContents(CPoint(0, 0), m_szTabBar);
	if (m_bShowArrows)
		rcContents.DeflateRect(s_nArrowWidth, 0);

	for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i)
	{
		CRect rcTab = m_tabs[i].rcTab;
		CRect rcClose = m_tabs[i].rcClose;
		if (m_bShowArrows)
		{
			rcTab.OffsetRect(s_nArrowWidth - m_nScrollPos, 0);
			rcClose.OffsetRect(s_nArrowWidth - m_nScrollPos, 0);
		}

		if (rcTab.IntersectRect(CRect(rcTab), rcContents))
			m_toolTip.AddTool(this, m_tabs[i].strName, rcTab, 2*i + 1);
		if (HasCloseButton(i) && rcClose.IntersectRect(CRect(rcClose), rcContents))
			m_toolTip.AddTool(this, m_strCloseTab, rcClose, 2*i + 2);
	}

	m_toolTip.Activate(true);
}

BOOL CTabbedMDIWnd::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

bool CTabbedMDIWnd::HasCloseButton(int nTab, CRect* prcClose)
{
	if (nTab == -1 || nTab != m_nActiveTab && m_tabs[nTab].rcTab.Width() <= s_nMinTabCloseWidth)
		return false;

	if (prcClose != NULL)
	{
		*prcClose = m_tabs[nTab].rcClose;
		if (m_bShowArrows)
			prcClose->OffsetRect(s_nArrowWidth - m_nScrollPos, 0);
	}
	return true;
}

void CTabbedMDIWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	int nTabClicked = TabFromPoint(point);
	CRect rcClose;
	if (HasCloseButton(nTabClicked, &rcClose) && rcClose.PtInRect(point))
	{
		m_nClosePressedTab = nTabClicked;
		InvalidateRect(rcClose);
		SetCapture();
		return;
	}

	if (nTabClicked != -1)
		ActivateTabImpl(nTabClicked);

	if (m_bShowArrows)
	{
		int nScrollWidth = m_szTabBar.cx - 2*s_nArrowWidth;
		if (m_rcLeftArrow.PtInRect(point) && (m_nScrollPos > 0))
		{
			int nTab = 0;
			while (nTab < static_cast<int>(m_tabs.size()) - 1 && m_tabs[nTab].rcTab.right < m_nScrollPos)
				++nTab;
			EnsureVisible(nTab);
		}
		else if (m_rcRightArrow.PtInRect(point) && m_nScrollPos < m_tabs.back().rcTab.right - nScrollWidth)
		{
			int nTab = static_cast<int>(m_tabs.size()) - 1;
			while (nTab > 0 && m_tabs[nTab].rcTab.left > m_nScrollPos + nScrollWidth)
				--nTab;
			EnsureVisible(nTab);
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CTabbedMDIWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_nClosePressedTab != -1)
	{
		int nTab = TabFromPoint(point);
		CRect rcClose;
		if (nTab == m_nClosePressedTab && HasCloseButton(nTab, &rcClose) && rcClose.PtInRect(point))
			CloseTab(nTab);

		ReleaseCapture();
		m_nClosePressedTab = -1;

		UpdateHoverTab();
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CTabbedMDIWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnLButtonDown(nFlags, point);
}

void CTabbedMDIWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	int nHoverTab = TabFromPoint(point);
	if (nHoverTab != m_nHoverTab)
	{
		// Deactivate and then reactivate the tooltip, so that when
		// mouse pointer moves from one tab to the next, the tooltip
		// for the new tab will not pop up immediately
		m_toolTip.Activate(false);
		m_toolTip.Activate(true);

		m_nHoverTab = nHoverTab;
		m_bHoverClose = false;

		InvalidateTabs();
	}

	CRect rcClose;
	if (HasCloseButton(m_nHoverTab, &rcClose))
	{
		bool bHoverClose = !!rcClose.PtInRect(point);
		if (bHoverClose != m_bHoverClose)
		{
			m_toolTip.Activate(false);
			m_toolTip.Activate(true);

			m_bHoverClose = bHoverClose;
			InvalidateRect(rcClose);
		}
	}

	if (m_bShowArrows)
	{
		bool bHoverLeft = !!m_rcLeftArrow.PtInRect(point);
		bool bHoverRight = !!m_rcRightArrow.PtInRect(point);
		if (bHoverLeft != m_bHoverLeft || bHoverRight != m_bHoverRight)
		{
			m_bHoverLeft = bHoverLeft;
			m_bHoverRight = bHoverRight;
			InvalidateTabs();
		}
	}

	if (m_nHoverTab != -1 || m_bHoverLeft || m_bHoverRight)
	{
		TRACKMOUSEEVENT tme;

		ZeroMemory(&tme, sizeof(tme));
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		tme.dwHoverTime = HOVER_DEFAULT;

		TrackMouseEvent(&tme);
	}
}

void CTabbedMDIWnd::OnMouseLeave()
{
	if (m_bIgnoreMouseLeave)
		return;

	if (m_nHoverTab != -1)
	{
		m_nHoverTab = -1;
		m_bHoverClose = false;
		InvalidateTabs();
	}

	if (m_bHoverLeft || m_bHoverRight)
	{
		m_bHoverLeft = false;
		m_bHoverRight = false;
		InvalidateTabs();
	}
}

void CTabbedMDIWnd::OnMButtonDown(UINT nFlags, CPoint point)
{
	int nTabClicked = TabFromPoint(point);
	if (nTabClicked != -1)
	{
		m_nMButtonPressedTab = nTabClicked;
		SetCapture();
		return;
	}

	CWnd::OnMButtonDown(nFlags, point);
}

void CTabbedMDIWnd::OnMButtonUp(UINT nFlags, CPoint point)
{
	if (m_nMButtonPressedTab != -1)
	{
		int nTab = TabFromPoint(point);
		if (nTab == m_nMButtonPressedTab)
			CloseTab(nTab);

		ReleaseCapture();
		m_nMButtonPressedTab = -1;

		UpdateHoverTab();
	}

	CWnd::OnMButtonUp(nFlags, point);
}

void CTabbedMDIWnd::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	CPoint ptClient = pos;
	ScreenToClient(&ptClient);
	int nTab = TabFromPoint(ptClient);
	if (nTab == -1)
		return;

	CMenu menu;
	menu.LoadMenu(IDR_POPUP);

	CMenu* pPopup = menu.GetSubMenu(4);
	ASSERT(pPopup != NULL);

	if (m_tabs.size() == 1)
		pPopup->EnableMenuItem(ID_TAB_CLOSE_OTHER, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	m_bIgnoreMouseLeave = true;
	UINT nID = pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			pos.x, pos.y, this);
	if (nID == ID_TAB_CLOSE)
	{
		CWnd* pWnd = m_tabs[nTab].pWnd;
		CloseTab(nTab);
	}
	else if (nID == ID_TAB_CLOSE_OTHER && m_tabs.size() > 1)
	{
		if (nTab != m_nActiveTab)
			ActivateTabImpl(nTab, false);

		for (int i = (int)m_tabs.size() - 1; i >= 0; --i)
		{
			if (i != nTab)
			{
				CWnd* pWnd = m_tabs[i].pWnd;
				CloseTab(i, false);
			}
		}

		UpdateWindow();
	}

	if (m_nHoverTab != -1)
	{
		ReleaseCapture();
		m_nHoverTab = -1;
		m_bHoverClose = false;
	}

	UpdateHoverTab();
}

void CTabbedMDIWnd::UpdateHoverTab()
{
	CPoint ptCursor;
	GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);
	OnMouseMove(0, ptCursor);
	InvalidateTabs();
}

int CTabbedMDIWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_toolTip.Create(this);

	if (!m_bTabBarHidden)
		m_toolTip.Activate(true);

	m_imgClose.Create(14, 14, ILC_COLOR24 | ILC_MASK, 0, 1);
	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_CLOSE_TAB);
	m_imgClose.Add(&bitmap, RGB(0, 0, 255));

	theApp.AddObserver(this);

	return 0;
}

void CTabbedMDIWnd::OnDestroy()
{
	theApp.RemoveObserver(this);

	CWnd::OnDestroy();
}

BOOL CTabbedMDIWnd::PreTranslateMessage(MSG* pMsg)
{
	if (::IsWindow(m_toolTip.m_hWnd))
		m_toolTip.RelayEvent(pMsg);

	return CWnd::PreTranslateMessage(pMsg);
}

bool CTabbedMDIWnd::PtInTab(int nTab, const CPoint& point)
{
	if (m_bTabBarHidden)
		return false;

	CRect rcContents(CPoint(0, 0), m_szTabBar);
	CRect rcTab = m_tabs[nTab].rcTab;
	if (m_bShowArrows)
	{
		rcContents.DeflateRect(s_nArrowWidth, 0);
		rcTab.OffsetRect(s_nArrowWidth - m_nScrollPos, 0);
	}

	return rcContents.PtInRect(point) && rcTab.PtInRect(point);
}

int CTabbedMDIWnd::TabFromPoint(const CPoint& point)
{
	for (int nTab = 0; nTab < (int)m_tabs.size(); ++nTab)
		if (PtInTab(nTab, point))
			return nTab;

	return -1;
}

void CTabbedMDIWnd::ActivateTab(CWnd* pWnd)
{
	int nTab = TabFromFrame(pWnd);
	ActivateTabImpl(nTab);
}

void CTabbedMDIWnd::ActivateTab(int nTab)
{
	if (nTab >= 0 && nTab < GetTabCount())
		ActivateTabImpl(nTab);
}

void CTabbedMDIWnd::ActivateTabImpl(int nTab, bool bRedraw)
{
	if (m_nActiveTab == nTab && (nTab == -1 || !m_tabs[nTab].bReactivate))
		return;

	// Give the main frame a chance to replace tab contents before it's activated
	UpdateObservers(TabMsg(TAB_ACTIVATING, nTab != -1 ? m_tabs[nTab].pWnd : NULL, nTab));

	if (nTab != -1)
	{
		m_tabs[nTab].pWnd->SetWindowPos(&wndTop, m_rcContent.left, m_rcContent.top,
				m_rcContent.Width(), m_rcContent.Height(), SWP_NOACTIVATE);
		m_tabs[nTab].pWnd->ShowWindow(SW_SHOW);
		m_tabs[nTab].pWnd->SendMessage(WM_MDI_ACTIVATE, true);
		m_tabs[nTab].pWnd->SendMessageToDescendants(WM_MDI_ACTIVATE, true, 0, true, true);
	}
	if (m_nActiveTab != -1 && m_nActiveTab != nTab)
	{
		m_tabs[m_nActiveTab].pWnd->ShowWindow(SW_HIDE);
		m_tabs[m_nActiveTab].pWnd->SendMessage(WM_MDI_ACTIVATE, false);
		m_tabs[m_nActiveTab].pWnd->SendMessageToDescendants(WM_MDI_ACTIVATE, false, 0, true, true);
	}

	m_nActiveTab = nTab;
	if (nTab != -1)
	{
		m_tabs[nTab].bReactivate = false;
		EnsureVisible(nTab);
	}

	if (m_nActiveTab == -1)
		Invalidate();
	else
		InvalidateTabs();

	UpdateToolTips();

	if (bRedraw)
		UpdateWindow();

	UpdateObservers(TabMsg(TAB_ACTIVATED, nTab != -1 ? m_tabs[nTab].pWnd : NULL, nTab));
}

void CTabbedMDIWnd::ActivateNextTab()
{
	if (m_tabs.size() < 2)
		return;

	int nTab = (m_nActiveTab == -1 ? 0 : (m_nActiveTab + 1) % (int)m_tabs.size());
	ActivateTabImpl(nTab);
}

void CTabbedMDIWnd::ActivatePrevTab()
{
	if (m_tabs.size() < 2)
		return;

	int nTabs = (int)m_tabs.size();
	int nTab = (m_nActiveTab == -1 ? nTabs - 1 : (m_nActiveTab - 1 + nTabs) % nTabs);
	ActivateTabImpl(nTab);
}

int CTabbedMDIWnd::TabFromFrame(CWnd* pWnd)
{
	for (int nTab = 0; nTab < (int)m_tabs.size(); ++nTab)
		if (m_tabs[nTab].pWnd == pWnd)
			return nTab;

	return -1;
}

void CTabbedMDIWnd::OnUpdate(const Observable* source, const Message* message)
{
	if (message->code == APP_LANGUAGE_CHANGED)
	{
		m_strCloseTab = LoadString(IDS_CLOSE_TAB);
		UpdateToolTips();
	}
}

void CTabbedMDIWnd::EnsureVisible(int nTab)
{
	if (nTab == -1 || !m_bShowArrows)
		return;

	int nScrollWidth = m_szTabBar.cx - 2*s_nArrowWidth;
	int nTabsWidth = m_tabs.back().rcTab.right;

	CRect rcTab = m_tabs[nTab].rcTab;
	rcTab.OffsetRect(-m_nScrollPos, 0);
	int nScrollBy = 0;
	if (rcTab.right > nScrollWidth)
		nScrollBy = rcTab.right - nScrollWidth;
	if (rcTab.left - nScrollBy < 0)
		nScrollBy = rcTab.left - nScrollBy;

	int nScrollPos = max(0, min(nTabsWidth - nScrollWidth, m_nScrollPos + nScrollBy));
	if (nScrollPos != m_nScrollPos)
	{
		m_nScrollPos = nScrollPos;
		UpdateToolTips();
		UpdateHoverTab();
		InvalidateTabs();
	}
}

void CTabbedMDIWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	UpdateMetrics();

	CSize szClient = ::GetClientSize(this);
	OnSize(0, szClient.cx, szClient.cy);
}

void CTabbedMDIWnd::OnSysColorChange()
{
	InvalidateTabs();
}

void CTabbedMDIWnd::InvalidateTabs()
{
	if (m_bTabBarHidden)
		return;

	InvalidateRect(CRect(CPoint(0, 0), m_szTabBar), false);
}

CWnd* CTabbedMDIWnd::GetActiveTab() const
{
	if (m_nActiveTab != -1)
		return m_tabs[m_nActiveTab].pWnd;

	return NULL;
}

int CTabbedMDIWnd::GetActiveTabIndex() const
{
	return m_nActiveTab;
}

void CTabbedMDIWnd::ShowTabBar(bool bShow)
{
	if (bShow != m_bTabBarHidden)
		return;

	m_bTabBarHidden = !bShow;

	CSize szClient = ::GetClientSize(this);
	UpdateSize(szClient.cx, szClient.cy);
	UpdateWindow();
}

void CTabbedMDIWnd::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	CWnd::OnMenuSelect(nItemID, nFlags, hSysMenu);

	GetTopLevelFrame()->SendMessage(WM_MENUSELECT, MAKEWPARAM(nItemID, nFlags),
			(LPARAM) hSysMenu);
}

void CTabbedMDIWnd::OnEnterIdle(UINT nWhy, CWnd* pWho)
{
	CWnd::OnEnterIdle(nWhy, pWho);

	GetTopLevelFrame()->SendMessage(WM_ENTERIDLE, nWhy, (LPARAM) pWho->GetSafeHwnd());
}
