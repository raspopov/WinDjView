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
	: m_nActiveTab(-1), m_nHoverTab(-1), m_nScrollPos(0), m_bShowArrows(false)
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
	ON_WM_MOUSEMOVE()
	ON_WM_CONTEXTMENU()
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
		int nScrollWidth = m_size.cx - 2*s_nArrowWidth;
		bool bLeftEnabled = (m_nScrollPos > 0);
		bool bRightEnabled = (m_nScrollPos < m_tabs.back().rcTab.right - nScrollWidth);

		CRect rcLeftArrow(-2, s_nTopMargin, s_nArrowWidth, rcBottom.top);
		DrawInactiveTabRect(pDC, rcLeftArrow, true, bLeftEnabled);
		CRect rcRightArrow(m_size.cx - s_nArrowWidth, s_nTopMargin, m_size.cx + 2, rcBottom.top);
		DrawInactiveTabRect(pDC, rcRightArrow, true, bRightEnabled);

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

	if (nTab == m_nActiveTab)
		DrawActiveTabRect(pDC, tab.rcTab);
	else if (nTab == m_nHoverTab)
		DrawActiveTabRect(pDC, tab.rcTab, true);
	else
		DrawInactiveTabRect(pDC, tab.rcTab);

	COLORREF clrText = ::GetSysColor(COLOR_WINDOWTEXT);
	CRect rcText(tab.rcTab.left + s_nPadding, tab.rcTab.top + 2,
			tab.rcTab.right - s_nPadding, tab.rcTab.bottom);

	CFont* pOldFont = pDC->SelectObject(nTab == m_nActiveTab ? &m_fontActive : &m_font);
	pDC->SetTextColor(clrText);
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(tab.strName, rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
	pDC->SelectObject(pOldFont);
}

void CMyTabBar::DrawActiveTabRect(CDC* pDC, const CRect& rect, bool bHover)
{
	COLORREF clrBtnface = ::GetSysColor(COLOR_BTNFACE);
	COLORREF clrTabStripBg = ChangeBrightness(clrBtnface, 0.88);
	COLORREF clrBtnshadow = ::GetSysColor(COLOR_BTNSHADOW);
	COLORREF clrActiveTabTopBg = ChangeBrightness(clrBtnface, 1.05);
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

void CMyTabBar::DrawInactiveTabRect(CDC* pDC, const CRect& rect,
		bool bArrow, bool bArrowEnabled, bool bArrowHover)
{
	COLORREF clrBtnface = ::GetSysColor(COLOR_BTNFACE);
	COLORREF clrTabStripBg = ChangeBrightness(clrBtnface, 0.88);
	COLORREF clrBtnshadow = ::GetSysColor(COLOR_BTNSHADOW);
	COLORREF clrShadow = ChangeBrightness(clrBtnshadow, 1.1);
	COLORREF clrHilight = ChangeBrightness(clrBtnface, 0.95);
	COLORREF clrTabTopBg = ChangeBrightness(clrBtnface, 0.91);
	COLORREF clrTabBottomBg = ChangeBrightness(clrBtnface, 0.93);

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

	CRect rcTopBg(rect.left + 2, rect.top + 3, rect.right - 1, rect.CenterPoint().y + 4);
	CRect rcBottomBg(rcTopBg.left, rcTopBg.bottom, rcTopBg.right, rect.bottom - 1);
	pDC->FillSolidRect(rcTopBg, clrTabTopBg);
	pDC->FillSolidRect(rcBottomBg, clrTabBottomBg);

	pDC->SetPixel(rect.left, rect.top + 1, clrTabStripBg);
	pDC->SetPixel(rect.right - 1, rect.top + 1, clrTabStripBg);
	pDC->SetPixel(rect.left + 1, rect.top + 2, clrTabTopBg);
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

	RemoveTab(nTab);
}

void CMyTabBar::RemoveTab(int nTab)
{
	bool bActiveTab = (m_nActiveTab == nTab);
	m_tabs.erase(m_tabs.begin() + nTab);

	UpdateTabRects();
	UpdateScrollState();
	UpdateToolTips();

	if (m_nActiveTab >= nTab)
	{
		if (m_nActiveTab > nTab || m_nActiveTab >= static_cast<int>(m_tabs.size()))
			--m_nActiveTab;

		ActivateTab(m_nActiveTab);
		if (bActiveTab && m_nActiveTab != -1)
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

	for (int i = m_toolTip.GetToolCount() - 1; i >= 0; --i)
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

void CMyTabBar::OnMouseMove(UINT nFlags, CPoint point)
{
	int nHoverTab = TabFromPoint(point);
	if (nHoverTab != m_nHoverTab)
	{
		if (m_nHoverTab == -1)
			SetCapture();
		else if (nHoverTab == -1)
			ReleaseCapture();

		m_nHoverTab = nHoverTab;
		Invalidate();
	}
}

void CMyTabBar::OnContextMenu(CWnd* pWnd, CPoint pos)
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

	UINT nID = pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			pos.x, pos.y, this);
	if (nID == ID_TAB_CLOSE)
	{
		CFrameWnd* pFrame = m_tabs[nTab].pFrame;
		RemoveTab(nTab);
		UpdateObservers(FrameMsg(TAB_CLOSED, pFrame));
	}
	else if (nID == ID_TAB_CLOSE_OTHER && m_tabs.size() > 1)
	{
		if (nTab != m_nActiveTab)
		{
			ActivateTab(nTab);
			UpdateObservers(FrameMsg(TAB_SELECTED, m_tabs[m_nActiveTab].pFrame));
		}

		for (int i = m_tabs.size() - 1; i >= 0; --i)
		{
			if (i != nTab)
			{
				CFrameWnd* pFrame = m_tabs[i].pFrame;
				RemoveTab(i);
				UpdateObservers(FrameMsg(TAB_CLOSED, pFrame));
			}
		}
	}

	if (m_nHoverTab != -1)
	{
		ReleaseCapture();
		m_nHoverTab = -1;
	}

	// Update hover tab
	CPoint ptCursor;
	GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);
	OnMouseMove(0, ptCursor);
	Invalidate();
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

bool CMyTabBar::PtInTab(int nTab, const CPoint& point)
{
	CRect rcContents(CPoint(0, 0), m_size);
	CRect rcTab = m_tabs[nTab].rcTab;
	if (m_bShowArrows)
	{
		rcContents.DeflateRect(s_nArrowWidth, 0);
		rcTab.OffsetRect(s_nArrowWidth - m_nScrollPos, 0);
	}

	return rcContents.PtInRect(point) && rcTab.PtInRect(point);
}

int CMyTabBar::TabFromPoint(const CPoint& point)
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

void CMyTabBar::ActivateNextTab()
{
	if (m_tabs.size() < 2)
		return;

	int nTab = (m_nActiveTab == -1 ? 0 : (m_nActiveTab + 1) % m_tabs.size());
	ActivateTab(nTab);
	UpdateObservers(FrameMsg(TAB_SELECTED, m_tabs[nTab].pFrame));
}

void CMyTabBar::ActivatePrevTab()
{
	if (m_tabs.size() < 2)
		return;

	int nTab = (m_nActiveTab == -1 ? m_tabs.size() - 1 : (m_nActiveTab - 1 + m_tabs.size()) % m_tabs.size());
	ActivateTab(nTab);
	UpdateObservers(FrameMsg(TAB_SELECTED, m_tabs[nTab].pFrame));
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
