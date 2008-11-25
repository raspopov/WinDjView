//	WinDjView
//	Copyright (C) 2004-2008 Andrew Zhezherun
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

// $Id$

#include "stdafx.h"
#include "WinDjView.h"
#include "MyTabBar.h"
#include "Drawing.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static const int s_nSideMargin = 3;
static const int s_nBottomMargin = 2;
static const int s_nTopMargin = 3;
static const int s_nPadding = 9;
static const int s_nMaxTabWidth = 250;
static const int s_nMinTabWidth = 100;
static const int s_nArrowWidth = 10;

// CMyTabBar

IMPLEMENT_DYNCREATE(CMyTabBar, CControlBar)

CMyTabBar::CMyTabBar()
	: m_nActiveTab(-1), m_nScrollPos(0), m_bShowArrows(false)
{
	m_cxLeftBorder = m_cxRightBorder = m_cyBottomBorder = m_cyTopBorder = 0;

	UpdateMetrics();
}

CMyTabBar::~CMyTabBar()
{
}


BEGIN_MESSAGE_MAP(CMyTabBar, CControlBar)
	ON_WM_PAINT()
	ON_WM_NCPAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETTINGCHANGE()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()


// CMyTabBar message handlers

BOOL CMyTabBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	DWORD dwNewStyle = dwStyle | WS_CHILD | WS_CLIPSIBLINGS;

	// Save the style
	m_dwStyle = (dwNewStyle & CBRS_ALL);
	return CControlBar::Create(NULL, NULL, dwNewStyle, CRect(0, 0, 0, 0), pParentWnd, nID);
}

void CMyTabBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	COLORREF clrBtnface = ::GetSysColor(COLOR_BTNFACE);
	COLORREF clrBackground = ChangeBrightness(clrBtnface, 0.88);
	COLORREF clrBtnshadow = ::GetSysColor(COLOR_BTNSHADOW);

	if (m_tabs.empty())
	{
		dc.FillSolidRect(CRect(0, 0, m_size.cx, m_size.cy), clrBackground);
		return;
	}

	// Draw tabs offscreen
	m_offscreenDC.Create(&dc, m_size);
	CDC* pDC = &m_offscreenDC;

	CRect rcTopLine(0, 0, m_size.cx, 1);
	pDC->FillSolidRect(rcTopLine, clrBtnshadow);
	CRect rcTop(0, 1, m_size.cx, s_nTopMargin);
	pDC->FillSolidRect(rcTop, clrBackground);
	CRect rcBottom(0, rcTop.bottom + m_nTabHeight, m_size.cx, m_size.cy);
	pDC->FillSolidRect(rcBottom, clrBtnface);

	if (m_bShowArrows)
	{
		CRect rcLeftArrow(0, s_nTopMargin, s_nArrowWidth, rcBottom.top - 1);
		pDC->FillSolidRect(rcLeftArrow, clrBackground);
		CRect rcRightArrow(m_size.cx - s_nArrowWidth, s_nTopMargin, m_size.cx, rcBottom.top - 1);
		pDC->FillSolidRect(rcRightArrow, clrBackground);
		CRect rcRightLine(m_size.cx - s_nArrowWidth, rcBottom.top - 1, m_size.cx, rcBottom.top);
		pDC->FillSolidRect(rcRightLine, clrBtnshadow);

		pDC->IntersectClipRect(s_nArrowWidth, rcTop.bottom, m_size.cx - s_nArrowWidth, rcBottom.top);
		pDC->SetViewportOrg(s_nArrowWidth - m_nScrollPos, 0);
	}
	else
	{
		CRect rcLeft(0, s_nTopMargin, s_nSideMargin, rcBottom.top - 1);
		pDC->FillSolidRect(rcLeft, clrBackground);
		CRect rcLeftLine(rcLeft.left, rcBottom.top - 1, rcLeft.right, rcBottom.top);
		pDC->FillSolidRect(rcLeftLine, clrBtnshadow);
	}

	for (size_t nTab = 0; nTab < m_tabs.size(); ++nTab)
		DrawTab(pDC, nTab);

	if (!m_bShowArrows)
	{
		int nRight = m_tabs.back().rcTab.right;
		if (nRight < m_size.cx)
		{
			CRect rcRight(nRight, s_nTopMargin, m_size.cx, rcBottom.top - 1);
			pDC->FillSolidRect(rcRight, clrBackground);
			CRect rcRightLine(rcRight.left, rcBottom.top - 1, rcRight.right, rcBottom.top);
			pDC->FillSolidRect(rcRightLine, clrBtnshadow);
		}
	}

	// Flush bitmap to screen dc
	pDC->SetViewportOrg(0, 0);
	dc.BitBlt(0, 0, m_size.cx, m_size.cy, pDC, 0, 0, SRCCOPY);
	m_offscreenDC.Release();
}

void CMyTabBar::DrawTab(CDC* pDC, int nTab)
{
	Tab& tab = m_tabs[nTab];

	COLORREF clrBtnface = ::GetSysColor(COLOR_BTNFACE);
	COLORREF clrTabStripBg = ChangeBrightness(clrBtnface, 0.88);
	COLORREF clrBtnshadow = ::GetSysColor(COLOR_BTNSHADOW);
	COLORREF clrShadow1 = ChangeBrightness(clrBtnshadow, 1.12);
	COLORREF clrShadow2 = ChangeBrightness(clrBtnshadow, 1.1);
	COLORREF clrHilight1 = ChangeBrightness(clrBtnface, 0.95);
	COLORREF clrHilight2 = ChangeBrightness(clrBtnface, 0.9);
	COLORREF clrTabBg1 = ChangeBrightness(clrBtnface, 0.91);
	COLORREF clrTabBg2 = ChangeBrightness(clrBtnface, 0.93);
	COLORREF clrActiveTabBg1 = ChangeBrightness(clrBtnface, 1.05);

	if (nTab == m_nActiveTab)
	{
		CPen penBtnshadow(PS_SOLID, 1, clrBtnshadow);
		CPen* pOldPen = pDC->SelectObject(&penBtnshadow);
		pDC->MoveTo(tab.rcTab.left, tab.rcTab.top + 3);
		pDC->LineTo(tab.rcTab.left, tab.rcTab.bottom);
		pDC->MoveTo(tab.rcTab.left + 3, tab.rcTab.top);
		pDC->LineTo(tab.rcTab.right - 3, tab.rcTab.top);
		pDC->MoveTo(tab.rcTab.right - 1, tab.rcTab.top + 3);
		pDC->LineTo(tab.rcTab.right - 1, tab.rcTab.bottom);

		pDC->SelectObject(pOldPen);

		CRect rcTopBg(tab.rcTab.left + 1, tab.rcTab.top + 1,
				tab.rcTab.right - 1, tab.rcTab.CenterPoint().y);
		pDC->FillSolidRect(rcTopBg, clrActiveTabBg1);
		CRect rcBottomBg(rcTopBg.left, rcTopBg.bottom, rcTopBg.right, tab.rcTab.bottom);
		pDC->FillSolidRect(rcBottomBg, clrBtnface);

		pDC->SetPixel(tab.rcTab.left, tab.rcTab.top, clrTabStripBg);
		pDC->SetPixel(tab.rcTab.left + 1, tab.rcTab.top, clrTabStripBg);
		pDC->SetPixel(tab.rcTab.left, tab.rcTab.top + 1, clrTabStripBg);
		pDC->SetPixel(tab.rcTab.left + 1, tab.rcTab.top + 1, clrBtnshadow);
		pDC->SetPixel(tab.rcTab.left, tab.rcTab.top + 2, clrShadow1);
		pDC->SetPixel(tab.rcTab.left + 2, tab.rcTab.top, clrShadow1);
		pDC->SetPixel(tab.rcTab.left + 1, tab.rcTab.top + 2, clrHilight2);
		pDC->SetPixel(tab.rcTab.left + 2, tab.rcTab.top + 1, clrHilight2);

		pDC->SetPixel(tab.rcTab.right - 1, tab.rcTab.top, clrTabStripBg);
		pDC->SetPixel(tab.rcTab.right - 2, tab.rcTab.top, clrTabStripBg);
		pDC->SetPixel(tab.rcTab.right - 1, tab.rcTab.top + 1, clrTabStripBg);
		pDC->SetPixel(tab.rcTab.right - 2, tab.rcTab.top + 1, clrBtnshadow);
		pDC->SetPixel(tab.rcTab.right - 1, tab.rcTab.top + 2, clrShadow1);
		pDC->SetPixel(tab.rcTab.right - 3, tab.rcTab.top, clrShadow1);
		pDC->SetPixel(tab.rcTab.right - 2, tab.rcTab.top + 2, clrHilight2);
		pDC->SetPixel(tab.rcTab.right - 3, tab.rcTab.top + 1, clrHilight2);
	}
	else
	{
		CPen penTabStrip(PS_SOLID, 1, clrTabStripBg);
		CPen* pOldPen = pDC->SelectObject(&penTabStrip);
		pDC->MoveTo(tab.rcTab.left, tab.rcTab.top);
		pDC->LineTo(tab.rcTab.right, tab.rcTab.top);

		CPen penBtnshadow(PS_SOLID, 1, clrBtnshadow);
		pDC->SelectObject(&penBtnshadow);
		pDC->MoveTo(tab.rcTab.left, tab.rcTab.top + 2);
		pDC->LineTo(tab.rcTab.left, tab.rcTab.bottom - 1);
		pDC->MoveTo(tab.rcTab.left + 1, tab.rcTab.top + 1);
		pDC->LineTo(tab.rcTab.right - 1, tab.rcTab.top + 1);
		pDC->MoveTo(tab.rcTab.left, tab.rcTab.bottom - 1);
		pDC->LineTo(tab.rcTab.right, tab.rcTab.bottom - 1);

		CPen penShadow2(PS_SOLID, 1, clrShadow2);
		pDC->SelectObject(&penShadow2);
		pDC->MoveTo(tab.rcTab.right - 1, tab.rcTab.top + 2);
		pDC->LineTo(tab.rcTab.right - 1, tab.rcTab.bottom - 1);

		CPen penHilight1(PS_SOLID, 1, clrHilight1);
		pDC->SelectObject(&penHilight1);
		pDC->MoveTo(tab.rcTab.left + 1, tab.rcTab.top + 3);
		pDC->LineTo(tab.rcTab.left + 1, tab.rcTab.bottom - 1);
		pDC->MoveTo(tab.rcTab.left + 2, tab.rcTab.top + 2);
		pDC->LineTo(tab.rcTab.right - 1, tab.rcTab.top + 2);

		pDC->SelectObject(pOldPen);

		CRect rcTopBg(tab.rcTab.left + 2, tab.rcTab.top + 3,
				tab.rcTab.right - 1, tab.rcTab.CenterPoint().y + 4);
		pDC->FillSolidRect(rcTopBg, clrTabBg1);
		CRect rcBottomBg(rcTopBg.left, rcTopBg.bottom, rcTopBg.right, tab.rcTab.bottom - 1);
		pDC->FillSolidRect(rcBottomBg, clrTabBg2);

		pDC->SetPixel(tab.rcTab.left, tab.rcTab.top + 1, clrTabStripBg);
		pDC->SetPixel(tab.rcTab.right - 1, tab.rcTab.top + 1, clrTabStripBg);
		pDC->SetPixel(tab.rcTab.left + 1, tab.rcTab.top + 2, clrTabBg1);
	}

	COLORREF clrText = ::GetSysColor(COLOR_WINDOWTEXT);
	CRect rcText(tab.rcTab.left + s_nPadding, tab.rcTab.top + 2,
			tab.rcTab.right - s_nPadding, tab.rcTab.bottom);

	CFont* pOldFont = pDC->SelectObject(nTab == m_nActiveTab ? &m_fontActive : &m_font);
	pDC->SetTextColor(clrText);
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(tab.strName, rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
	pDC->SelectObject(pOldFont);
}

void CMyTabBar::UpdateMetrics()
{
	CreateSystemDialogFont(m_font);

	LOGFONT lf;
	m_font.GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	m_fontActive.CreateFontIndirect(&lf);

	CScreenDC dcScreen;
	CFont* pOldFont = dcScreen.SelectObject(&m_font);
	TEXTMETRIC tm;
	CSize szText = dcScreen.GetTextMetrics(&tm);
	dcScreen.SelectObject(pOldFont);

	m_nTabHeight = tm.tmHeight + 11;
}

void CMyTabBar::OnSize(UINT nType, int cx, int cy)
{
	CControlBar::OnSize(nType, cx, cy);

	m_size = CSize(cx, cy);
	UpdateTabRects();
	UpdateScrollState();
	UpdateToolTips();

	Invalidate();
}

void CMyTabBar::UpdateScrollState()
{
	if (!m_bShowArrows)
	{
		m_nScrollPos = 0;
		return;
	}

	ASSERT(!m_tabs.empty());
	int nScrollWidth = m_size.cx - 2*s_nArrowWidth;
	int nTabsWidth = m_tabs.back().rcTab.right;
	int nScrollPos = max(0, min(nTabsWidth - nScrollWidth, m_nScrollPos));
	if (nScrollPos != m_nScrollPos)
	{
		m_nScrollPos = nScrollPos;
		Invalidate();
	}
}

int CMyTabBar::AddTab(CFrameWnd* pFrame, const CString& strName)
{
	Tab tab;
	tab.pFrame = pFrame;
	tab.strName = strName;
	m_tabs.push_back(tab);

	UpdateTabRects();
	UpdateScrollState();
	UpdateToolTips();

	if (m_nActiveTab == -1)
	{
		ActivateTab(m_tabs.size() - 1);
		UpdateObservers(FrameMsg(TAB_SELECTED, m_tabs[m_nActiveTab].pFrame));
	}

	if (::IsWindow(m_hWnd))
		Invalidate();

	return m_tabs.size() - 1;
}

void CMyTabBar::RemoveTab(CFrameWnd* pFrame)
{
	int nTab = TabFromFrame(pFrame);
	if (nTab == -1)
		return;

	m_tabs.erase(m_tabs.begin() + nTab);

	UpdateTabRects();
	UpdateScrollState();
	UpdateToolTips();

	if (m_nActiveTab >= nTab)
	{
		if (m_nActiveTab > nTab || m_nActiveTab >= static_cast<int>(m_tabs.size()))
			--m_nActiveTab;

		ActivateTab(m_nActiveTab);
		if (m_nActiveTab != -1)
			UpdateObservers(FrameMsg(TAB_SELECTED, m_tabs[m_nActiveTab].pFrame));
	}

	if (::IsWindow(m_hWnd))
		Invalidate();
}

void CMyTabBar::UpdateTabRects()
{
	if (m_tabs.empty())
	{
		m_bShowArrows = false;
		return;
	}

	int nLeft = s_nSideMargin;
	int nTop = s_nTopMargin;
	int nBottom = nTop + m_nTabHeight;

	int nTabArea = m_size.cx - 2*s_nSideMargin;
	int nTabWidth = max(s_nMinTabWidth, min(s_nMaxTabWidth, nTabArea / m_tabs.size()));

	if (nTabWidth * static_cast<int>(m_tabs.size()) > nTabArea)
	{
		m_bShowArrows = true;
		nLeft = 0;
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

		nLeft += nTabWidth;
	}
}

void CMyTabBar::UpdateToolTips()
{
	m_toolTip.Activate(false);

	for (int i = 0; i < m_toolTip.GetToolCount(); ++i)
		m_toolTip.DelTool(this, i + 1);

	for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i)
	{
		CRect rcTab = m_tabs[i].rcTab;
		if (m_bShowArrows)
		{
			rcTab.OffsetRect(s_nArrowWidth - m_nScrollPos, 0);
			rcTab.left = max(rcTab.left, s_nArrowWidth);
			rcTab.right = min(rcTab.right, m_size.cx - s_nArrowWidth);
			if (rcTab.left >= rcTab.right)
				continue;
		}

		m_toolTip.AddTool(this, m_tabs[i].strName, rcTab, i + 1);
	}

	m_toolTip.Activate(true);
}

BOOL CMyTabBar::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void CMyTabBar::OnNcPaint()
{
}

void CMyTabBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	int nTabClicked = TabFromPoint(point);
	if (nTabClicked != -1 && nTabClicked != m_nActiveTab)
	{
		ActivateTab(nTabClicked);
		UpdateObservers(FrameMsg(TAB_SELECTED, m_tabs[nTabClicked].pFrame));
	}

	CControlBar::OnLButtonDown(nFlags, point);
}

int CMyTabBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_toolTip.Create(this);
	m_toolTip.Activate(true);

	theApp.AddObserver(this);

	return 0;
}

void CMyTabBar::OnDestroy()
{
	theApp.RemoveObserver(this);

	CControlBar::OnDestroy();
}

BOOL CMyTabBar::PreTranslateMessage(MSG* pMsg)
{
	if (::IsWindow(m_toolTip.m_hWnd))
		m_toolTip.RelayEvent(pMsg);

	return CControlBar::PreTranslateMessage(pMsg);
}

bool CMyTabBar::PtInTab(int nTab, CPoint point)
{
	CRect rcTab = m_tabs[nTab].rcTab;
	if (m_bShowArrows)
	{
		rcTab.OffsetRect(s_nArrowWidth - m_nScrollPos, 0);
		rcTab.left = max(rcTab.left, s_nArrowWidth);
		rcTab.right = min(rcTab.right, m_size.cx - s_nArrowWidth);
		if (rcTab.left >= rcTab.right)
			return false;
	}

	return !!rcTab.PtInRect(point);
}

int CMyTabBar::TabFromPoint(CPoint point)
{
	for (size_t nTab = 0; nTab < m_tabs.size(); ++nTab)
		if (PtInTab(nTab, point))
			return nTab;

	return -1;
}

void CMyTabBar::ActivateTab(CFrameWnd* pFrame)
{
	int nTab = TabFromFrame(pFrame);
	ActivateTab(nTab);
}

void CMyTabBar::ActivateTab(int nTab)
{
	m_nActiveTab = nTab;
	EnsureVisible(nTab);

	if (::IsWindow(m_hWnd))
		Invalidate();
}

int CMyTabBar::TabFromFrame(CFrameWnd* pFrame)
{
	for (size_t nTab = 0; nTab < m_tabs.size(); ++nTab)
		if (m_tabs[nTab].pFrame == pFrame)
			return nTab;

	return -1;
}

void CMyTabBar::OnUpdate(const Observable* source, const Message* message)
{
}

void CMyTabBar::EnsureVisible(int nTab)
{
	if (nTab == -1 || !m_bShowArrows)
		return;

	int nScrollWidth = m_size.cx - 2*s_nArrowWidth;
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
		Invalidate();
	}
}

CSize CMyTabBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	return CSize(32768, s_nTopMargin + s_nBottomMargin + m_nTabHeight);
}

void CMyTabBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
}

void CMyTabBar::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	UpdateMetrics();
	UpdateTabRects();
	UpdateScrollState();
	UpdateToolTips();

	Invalidate();

	if (m_pDockSite != NULL)
		m_pDockSite->RecalcLayout();
}

void CMyTabBar::OnSysColorChange()
{
	Invalidate();
}
