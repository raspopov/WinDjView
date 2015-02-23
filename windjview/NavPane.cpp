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
#include "NavPane.h"
#include "Drawing.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNavPaneWnd

IMPLEMENT_DYNCREATE(CNavPaneWnd, CWnd)
CNavPaneWnd::CNavPaneWnd()
	: m_nActiveTab(-1), m_hTheme(NULL), m_bCloseActive(false), m_bClosePressed(false),
	  m_bSettingsActive(false), m_bSettingsPressed(false), m_bDragging(false)
{
	CFont systemFont;
	CreateSystemDialogFont(systemFont);

	LOGFONT lf;
	systemFont.GetLogFont(&lf);

	_tcscpy(lf.lfFaceName, _T("Arial"));
	lf.lfHeight = -13;
	lf.lfEscapement = lf.lfOrientation = 900;
	m_font.CreateFontIndirect(&lf);

	lf.lfWeight = FW_BOLD;
	m_fontActive.CreateFontIndirect(&lf);
}

CNavPaneWnd::~CNavPaneWnd()
{
}


BEGIN_MESSAGE_MAP(CNavPaneWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE_VOID(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_CREATE()
	ON_MESSAGE_VOID(WM_INITIALUPDATE, OnInitialUpdate)
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_MESSAGE_VOID(WM_THEMECHANGED, OnThemeChanged)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CNavPaneWnd message handlers

void CNavPaneWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rcClient = ::GetClientRect(this);

	COLORREF clrBtnface = ::GetSysColor(COLOR_BTNFACE);
	COLORREF clrTabBg = ChangeBrightness(clrBtnface, 0.85);
	COLORREF clrHilight = ::GetSysColor(COLOR_BTNHILIGHT);
	COLORREF clrShadow = ::GetSysColor(COLOR_BTNSHADOW);
	COLORREF clrFrame = ::GetSysColor(COLOR_WINDOWFRAME);

	if (m_tabs.empty())
	{
		dc.FillSolidRect(rcClient, clrBtnface);
		return;
	}

	// Draw tabs offscreen
	CSize szTabs(s_nTabsWidth + 1, rcClient.Height());
	m_offscreenDC.Create(&dc, szTabs);
	CDC* pDC = &m_offscreenDC;

	CRect rcTabs(rcClient);
	rcTabs.right = rcTabs.left + s_nTabsWidth;
	pDC->FillSolidRect(rcTabs, clrTabBg);

	// Vertical line
	CRect rcLine(rcTabs.right, rcClient.top, rcTabs.right + 1, rcClient.bottom);
	pDC->FillSolidRect(rcLine, clrShadow);

	for (int nDiff = (int)m_tabs.size(); nDiff > 0; --nDiff)
	{
		if (m_nActiveTab - nDiff >= 0)
			DrawTab(pDC, m_nActiveTab - nDiff, false);
		if (m_nActiveTab + nDiff < static_cast<int>(m_tabs.size()))
			DrawTab(pDC, m_nActiveTab + nDiff, false);
	}
	DrawTab(pDC, m_nActiveTab, true);

	// Flush bitmap to screen dc
	dc.BitBlt(0, 0, szTabs.cx, szTabs.cy, pDC, 0, 0, SRCCOPY);
	m_offscreenDC.Release();

	// Left space
	rcTabs.left = rcTabs.right + 1;
	rcTabs.right = rcTabs.left + s_nLeftMargin;
	dc.FillSolidRect(rcTabs, clrBtnface);

	if (m_tabs[m_nActiveTab].bHasBorder)
	{
		rcLine.OffsetRect(s_nLeftMargin + 1, 0);
		rcLine.top += s_nTopMargin;
		dc.FillSolidRect(rcLine, clrShadow);
	}

	// Top space
	rcTabs.right = rcClient.right;
	rcTabs.bottom = rcTabs.top + s_nTopMargin;
	dc.FillSolidRect(rcTabs, clrBtnface);

	if (m_tabs[m_nActiveTab].bHasBorder)
	{
		rcLine.right = rcClient.right;
		rcLine.bottom = rcLine.top + 1;
		dc.FillSolidRect(rcLine, clrShadow);
	}

	if (rcClient.Width() > 70)
	{
		// Close button
		if (m_hTheme != NULL)
		{
			if (m_bClosePressed)
				XPDrawThemeBackground(m_hTheme, dc.m_hDC, TP_BUTTON, TS_PRESSED, m_rcClose, NULL);
			else if (m_bCloseActive)
				XPDrawThemeBackground(m_hTheme, dc.m_hDC, TP_BUTTON, TS_HOT, m_rcClose, NULL);
		}
		else
		{
			if (m_bClosePressed)
				dc.Draw3dRect(m_rcClose, clrShadow, clrHilight);
			else if (m_bCloseActive)
				dc.Draw3dRect(m_rcClose, clrHilight, clrShadow);
		}

		CPoint ptOffset(m_rcClose.left + (m_rcClose.Width() - 10) / 2,
				m_rcClose.top + (m_rcClose.Height() - 10) / 2);
		if (m_bClosePressed)
			ptOffset.Offset(1, m_hTheme != NULL ? 0 : 1);

		m_imgClose.Draw(&dc, 0, ptOffset, ILD_NORMAL);

		// Settings button
		if (m_nActiveTab != -1 && m_tabs[m_nActiveTab].bHasSettings)
		{
			if (m_hTheme != NULL)
			{
				if (m_bSettingsPressed)
					XPDrawThemeBackground(m_hTheme, dc.m_hDC, TP_BUTTON, TS_PRESSED, m_rcSettings, NULL);
				else if (m_bSettingsActive)
					XPDrawThemeBackground(m_hTheme, dc.m_hDC, TP_BUTTON, TS_HOT, m_rcSettings, NULL);
			}
			else
			{
				if (m_bSettingsPressed)
					dc.Draw3dRect(m_rcSettings, clrShadow, clrHilight);
				else if (m_bSettingsActive)
					dc.Draw3dRect(m_rcSettings, clrHilight, clrShadow);
			}

			CPoint ptOffset2(m_rcSettings.left + (m_rcSettings.Width() - 10) / 2,
					m_rcSettings.top + (m_rcSettings.Height() - 10) / 2);
			if (m_bSettingsPressed)
				ptOffset2.Offset(1, m_hTheme != NULL ? 0 : 1);

			m_imgSettings.Draw(&dc, 0, ptOffset2, ILD_NORMAL);
		}
	}
}

void CNavPaneWnd::DrawTab(CDC* pDC, int nTab, bool bActive)
{
	Tab& tab = m_tabs[nTab];

	COLORREF clrBtnface = ::GetSysColor(COLOR_BTNFACE);
	COLORREF clrTabBg = ChangeBrightness(clrBtnface, 0.85);
	COLORREF clrHilight = ::GetSysColor(COLOR_BTNHILIGHT);
	COLORREF clrShadow = ::GetSysColor(COLOR_BTNSHADOW);
	COLORREF clrFrame = ::GetSysColor(COLOR_WINDOWFRAME);
	COLORREF clrText = ::GetSysColor(COLOR_WINDOWTEXT);

	CBrush brushBtnface(clrBtnface);
	CPen penBtnface(PS_SOLID, 1, clrBtnface);
	CPen penShadow(PS_SOLID, 1, clrShadow);
	CPen penHilight(PS_SOLID, 1, clrHilight);

	CRect rcTab = tab.rcTab;
	rcTab.bottom += s_nTabSize / 2 - 2;
	pDC->FillSolidRect(rcTab, clrBtnface);

	// Vertical line
	CRect rcLine(rcTab.left, rcTab.top, rcTab.left + 1, rcTab.bottom);
	pDC->FillSolidRect(rcLine, clrShadow);

	// Horizontal line
	CRect rcHorzLine(rcTab.left, rcTab.bottom, rcTab.right, rcTab.bottom + 1);
	pDC->FillSolidRect(rcHorzLine, clrShadow);

	CPoint points[] = {
			CPoint(rcTab.left, rcTab.top - 1),
			CPoint(rcTab.right - 1, rcTab.top - s_nTabSize + 1),
			CPoint(rcTab.right - 1, rcTab.top - 1) };

	CBrush* pOldBrush = pDC->SelectObject(&brushBtnface);
	CPen* pOldPen = pDC->SelectObject(&penBtnface);
	pDC->SetPolyFillMode(WINDING);
	pDC->Polygon(points, 3);
	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);

	CPoint points2[] = {
			CPoint(rcTab.left, rcTab.top - 1),
			CPoint(rcTab.right + 1, rcTab.top - s_nTabSize - 1) };

	pOldPen = pDC->SelectObject(&penShadow);
	pDC->Polyline(points2, 2);
	pDC->SelectObject(pOldPen);

	if (bActive)
	{
		rcLine.OffsetRect(s_nTabSize - 1, 0);
		rcLine.top -= s_nTabSize - 1;
		pDC->FillSolidRect(rcLine, clrBtnface);
	}

	CFont* pOldFont = pDC->SelectObject(bActive ? &m_fontActive : &m_font);
	pDC->SetTextColor(clrText);
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextAlign(TA_CENTER | TA_BASELINE);
	pDC->TextOut(tab.rcTab.right - 4, tab.rcTab.CenterPoint().y, tab.strName);
	pDC->SelectObject(pOldFont);
}

void CNavPaneWnd::OnWindowPosChanged(WINDOWPOS* lpwndpos) 
{
	CWnd::OnWindowPosChanged(lpwndpos);

	UpdateButtons();
	UpdateTabContents();
}

void CNavPaneWnd::UpdateTabContents()
{
	CRect rcContents = ::GetClientRect(this);
	rcContents.DeflateRect(s_nTabsWidth + s_nLeftMargin + 1, s_nTopMargin, 0, 0);

	CRect rcBordered(rcContents);
	rcBordered.DeflateRect(1, 1, 0, 0);

	for (size_t i = 0; i < m_tabs.size(); ++i)
	{
		CWnd* pWnd = m_tabs[i].pWnd;
		const CRect& rc = (m_tabs[i].bHasBorder ? rcBordered : rcContents);
		m_tabs[i].rcContent = rc;

		if (i == m_nActiveTab)
		{
			if (rc.Height() <= 0 || rc.Width() <= 0)
			{
				pWnd->ShowWindow(SW_HIDE);
			}
			else
			{
				pWnd->SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE);
				pWnd->ShowWindow(SW_SHOW);
			}
		}
	}
}

int CNavPaneWnd::AddTab(const CString& strName, CWnd* pWnd)
{
	Tab tab;
	tab.pWnd = pWnd;
	tab.strName = strName;
	tab.bHasBorder = true;
	tab.bHasSettings = false;
	tab.rcContent = CRect(0, 0, 0, 0);

	int nTop = s_nTabSize;
	if (!m_tabs.empty())
		nTop += m_tabs.back().rcTab.bottom;
	else
		nTop += 5;

	CScreenDC dcScreen;
	CFont* pOldFont = dcScreen.SelectObject(&m_fontActive);
	CSize szText = dcScreen.GetTextExtent(strName);
	dcScreen.SelectObject(pOldFont);

	tab.rcTab.top = nTop;
	tab.rcTab.bottom = nTop + szText.cx;
	tab.rcTab.left = s_nTabsWidth - s_nTabSize + 1;
	tab.rcTab.right = s_nTabsWidth;

	m_tabs.push_back(tab);

	pWnd->ShowWindow(SW_HIDE);
	pWnd->SetParent(this);
	pWnd->ModifyStyle(WS_BORDER, 0);
	pWnd->ModifyStyleEx(WS_EX_CLIENTEDGE, 0);

	if (m_nActiveTab == -1)
		m_nActiveTab = (int)m_tabs.size() - 1;

	if (::IsWindow(m_hWnd))
	{
		UpdateTabContents();
		Invalidate();
	}

	return (int)m_tabs.size() - 1;
}

BOOL CNavPaneWnd::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void CNavPaneWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	CSize szClient = ::GetClientSize(this);
	if (szClient.cx > 70)
	{
		if (m_rcClose.PtInRect(point))
		{
			m_bDragging = true;
			UpdateButtons();
			SetCapture();
			return;
		}
		else if (m_rcSettings.PtInRect(point))
		{
			m_bSettingsPressed = true;
			InvalidateRect(m_rcSettings);
			UpdateWindow();

			CRect rcButton = m_rcSettings;
			ClientToScreen(rcButton);
			if (m_nActiveTab != -1)
				m_tabs[m_nActiveTab].pWnd->SendMessage(WM_SHOW_SETTINGS, 0, (LPARAM)(LPRECT) rcButton);

			// Eat mouse messages during popupmenu
			MSG msg;
			if (::PeekMessage(&msg, m_hWnd, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_NOREMOVE))
			{
				if (rcButton.PtInRect(msg.pt))
					::PeekMessage(&msg, m_hWnd, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_REMOVE);
			}

			m_bSettingsPressed = false;
			m_bSettingsActive = false;
			InvalidateRect(m_rcSettings);
			UpdateWindow();
			return;
		}
	}

	int nTabClicked = GetTabFromPoint(point);
	if (nTabClicked != -1)
	{
		if (GetParent()->SendMessage(WM_CLICKED_NAV_TAB, 0, (LPARAM) m_tabs[nTabClicked].pWnd))
			ActivateTab(nTabClicked, true);
	}

	UpdateButtons();
	CWnd::OnLButtonDown(nFlags, point);
}

void CNavPaneWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDragging)
	{
		CRect rcClient = ::GetClientRect(this);
		if (rcClient.Width() > 70)
		{
			CRect rcButton(CPoint(rcClient.right - 21, rcClient.top + 2), CSize(19, 18));
			if (rcButton.PtInRect(point))
			{
				GetParent()->SendMessage(WM_COLLAPSE_NAV);
				m_bCloseActive = false;
				m_bClosePressed = false;
			}
		}

		m_bDragging = false;
		ReleaseCapture();
	}

	UpdateButtons();
	CWnd::OnLButtonUp(nFlags, point);
}

void CNavPaneWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	UpdateButtons();
	if (m_bCloseActive || m_bSettingsActive)
	{
		TRACKMOUSEEVENT tme;

		ZeroMemory(&tme, sizeof(tme));
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		tme.dwHoverTime = HOVER_DEFAULT;

		TrackMouseEvent(&tme);
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CNavPaneWnd::OnMouseLeave()
{
	UpdateButtons();
}

void CNavPaneWnd::UpdateButtons()
{
	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	CRect rcClient = ::GetClientRect(this);
	if (rcClient.Width() <= 70)
	{
		if (::IsWindow(m_toolTip.m_hWnd))
			m_toolTip.Activate(false);
		return;
	}

	m_rcClose = CRect(CPoint(rcClient.right - 21, rcClient.top + 2), CSize(19, 18));
	m_rcSettings = CRect(CPoint(rcClient.right - 40, rcClient.top + 2), CSize(19, 18));

	bool bInCloseRect = !!m_rcClose.PtInRect(ptCursor);

	if (m_bDragging)
		UpdateCloseButton(bInCloseRect, false);
	else
		UpdateCloseButton(false, bInCloseRect);

	if (m_nActiveTab != -1 && m_tabs[m_nActiveTab].bHasSettings)
	{
		bool bSettingsActive = !m_bDragging && !!m_rcSettings.PtInRect(ptCursor);
		if (bSettingsActive != m_bSettingsActive)
		{
			m_bSettingsActive = bSettingsActive;
			InvalidateRect(m_rcSettings);
		}
	}

	if (::IsWindow(m_toolTip.m_hWnd))
	{
		m_toolTip.Activate(true);
		m_toolTip.SetToolRect(this, 1, m_rcClose);

		if (m_nActiveTab != -1 && m_tabs[m_nActiveTab].bHasSettings)
			m_toolTip.SetToolRect(this, 2, m_rcSettings);
		else
			m_toolTip.SetToolRect(this, 2, CRect(0, 0, 0, 0));
	}
}

void CNavPaneWnd::UpdateCloseButton(bool bPressed, bool bActive)
{
	if (bPressed != m_bClosePressed || bActive != m_bCloseActive)
	{
		m_bClosePressed = bPressed;
		m_bCloseActive = bActive;
		InvalidateRect(m_rcClose);
	}
}

int CNavPaneWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_imgClose.Create(IDB_CLOSE, 10, 0, RGB(192, 192, 192));

	m_imgSettings.Create(10, 10, ILC_COLOR24 | ILC_MASK, 0, 1);
	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_SETTINGS);
	m_imgSettings.Add(&bitmap, RGB(255, 255, 255));

	if (IsThemed())
		m_hTheme = XPOpenThemeData(m_hWnd, L"TOOLBAR");

	theApp.AddObserver(this);

	return 0;
}

void CNavPaneWnd::OnInitialUpdate()
{
	// CMDIChild, which hosts CNavPaneWnd, may be initially created
	// with one parent and then switched to another parent. This creates
	// a problem for tooltips, so instead of creating the tooltip in
	// OnCreate(), it must be done later here.

	m_toolTip.Create(this);
	m_toolTip.AddTool(this, LoadString(IDS_TOOLTIP_HIDE), CRect(0, 0, 0, 0), 1);
	m_toolTip.AddTool(this, LoadString(IDS_TOOLTIP_SETTINGS), CRect(0, 0, 0, 0), 2);

	UpdateButtons();
}

void CNavPaneWnd::OnDestroy()
{
	theApp.RemoveObserver(this);

	if (m_hTheme != NULL)
	{
		XPCloseThemeData(m_hTheme);
		m_hTheme = NULL;
	}

	CWnd::OnDestroy();
}

BOOL CNavPaneWnd::PreTranslateMessage(MSG* pMsg)
{
	if (::IsWindow(m_toolTip.m_hWnd))
		m_toolTip.RelayEvent(pMsg);

	return CWnd::PreTranslateMessage(pMsg);
}

bool CNavPaneWnd::PtInTab(int nTab, CPoint point)
{
	CRect rcTab = m_tabs[nTab].rcTab;
	rcTab.bottom += s_nTabSize / 2 - 1;
	if (rcTab.PtInRect(point))
		return true;

	if (point.y >= rcTab.top - s_nTabSize + 1 && point.y < rcTab.top &&
			point.x >= rcTab.left + (rcTab.top - point.y) && point.x < rcTab.right)
		return true;

	if (point.x < rcTab.left && point.y >= rcTab.top && point.y < rcTab.bottom)
		return true;

	return false;
}

int CNavPaneWnd::GetTabFromPoint(CPoint point)
{
	int nTabClicked = -1;
	for (int nDiff = (int)m_tabs.size(); nDiff > 0; --nDiff)
	{
		if (m_nActiveTab - nDiff >= 0)
		{
			if (PtInTab(m_nActiveTab - nDiff, point))
				nTabClicked = m_nActiveTab - nDiff;
		}

		if (m_nActiveTab + nDiff < static_cast<int>(m_tabs.size()))
		{
			if (PtInTab(m_nActiveTab + nDiff, point))
				nTabClicked = m_nActiveTab + nDiff;
		}
	}

	if (m_nActiveTab != -1)
	{
		if (PtInTab(m_nActiveTab, point))
			nTabClicked = m_nActiveTab;
	}

	return nTabClicked;
}

int CNavPaneWnd::GetTabIndex(CWnd* pTabContent)
{
	for (size_t nTab = 0; nTab < m_tabs.size(); ++nTab)
		if (m_tabs[nTab].pWnd->GetSafeHwnd() == pTabContent->GetSafeHwnd())
			return static_cast<int>(nTab);

	return -1;
}

void CNavPaneWnd::ActivateTab(CWnd* pTabContent, bool bExpand)
{
	int nTab = GetTabIndex(pTabContent);
	if (nTab == -1)
		return;

	ActivateTab(nTab, bExpand);
}

void CNavPaneWnd::ActivateTab(int nTab, bool bExpand)
{
	m_nActiveTab = nTab;

	for (size_t i = 0; i < m_tabs.size(); ++i)
	{
		const CRect& rc = m_tabs[i].rcContent;
		if (i != nTab || rc.Width() <= 0 || rc.Height() <= 0)
		{
			m_tabs[i].pWnd->ShowWindow(SW_HIDE);
		}
		else
		{
			m_tabs[i].pWnd->SetWindowPos(NULL,
					rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE);
			m_tabs[i].pWnd->ShowWindow(SW_SHOW);
		}
	}

	UpdateButtons();
	Invalidate();

	if (bExpand)
		GetParent()->SendMessage(WM_EXPAND_NAV);

	m_tabs[nTab].pWnd->SetFocus();

	UpdateObservers(SIDEBAR_TAB_CHANGED);
}

void CNavPaneWnd::SetTabName(CWnd* pTabContent, const CString& strName)
{
	int nTab = GetTabIndex(pTabContent);
	if (nTab == -1)
		return;

	SetTabName(nTab, strName);
}

void CNavPaneWnd::SetTabName(int nTab, const CString& strName)
{
	Tab& tab = m_tabs[nTab];
	tab.strName = strName;

	CScreenDC dcScreen;
	CFont* pOldFont = dcScreen.SelectObject(&m_fontActive);
	CSize szText = dcScreen.GetTextExtent(strName);
	dcScreen.SelectObject(pOldFont);

	int offset = szText.cx - tab.rcTab.Height();
	tab.rcTab.bottom += offset;

	for (size_t i = nTab + 1; i < m_tabs.size(); ++i)
		m_tabs[i].rcTab.OffsetRect(0, offset);

	if (::IsWindow(m_hWnd))
	{
		UpdateTabContents();
		Invalidate();
	}
}

void CNavPaneWnd::OnUpdate(const Observable* source, const Message* message)
{
	if (message->code == APP_LANGUAGE_CHANGED)
	{
		m_toolTip.DelTool(this, 1);
		m_toolTip.DelTool(this, 2);
		m_toolTip.AddTool(this, LoadString(IDS_TOOLTIP_HIDE), CRect(0, 0, 0, 0), 1);
		m_toolTip.AddTool(this, LoadString(IDS_TOOLTIP_SETTINGS), CRect(0, 0, 0, 0), 2);
	}
}

void CNavPaneWnd::SetTabBorder(CWnd* pTabContent, bool bHasBorder)
{
	int nTab = GetTabIndex(pTabContent);
	if (nTab == -1)
		return;

	SetTabBorder(nTab, bHasBorder);
}

void CNavPaneWnd::SetTabBorder(int nTab, bool bHasBorder)
{
	Tab& tab = m_tabs[nTab];
	tab.bHasBorder = bHasBorder;

	if (::IsWindow(m_hWnd))
	{
		UpdateTabContents();
		Invalidate();
	}
}

void CNavPaneWnd::SetTabSettings(CWnd* pTabContent, bool bHasSettings)
{
	int nTab = GetTabIndex(pTabContent);
	if (nTab == -1)
		return;

	SetTabSettings(nTab, bHasSettings);
}

void CNavPaneWnd::SetTabSettings(int nTab, bool bHasSettings)
{
	Tab& tab = m_tabs[nTab];
	tab.bHasSettings = bHasSettings;

	if (::IsWindow(m_hWnd))
	{
		UpdateButtons();
		Invalidate();
	}
}

void CNavPaneWnd::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	if (m_nActiveTab != -1)
		m_tabs[m_nActiveTab].pWnd->SetFocus();
}

CChildFrame* CNavPaneWnd::GetParentFrame() const
{
	return (CChildFrame*) CWnd::GetParentFrame();
}

CWnd* CNavPaneWnd::GetActiveTab() const
{
	return (m_nActiveTab != -1 ? m_tabs[m_nActiveTab].pWnd : NULL);
}

void CNavPaneWnd::OnThemeChanged()
{
	if (m_hTheme != NULL)
		XPCloseThemeData(m_hTheme);

	m_hTheme = NULL;

	if (IsThemed())
		m_hTheme = XPOpenThemeData(m_hWnd, L"TOOLBAR");

	Invalidate();
}

void CNavPaneWnd::OnSysColorChange()
{
	Invalidate();
}

void CNavPaneWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CWnd::OnShowWindow(bShow, nStatus);

	CWnd* pParent = GetParent();
	if (pParent == NULL || pParent != NULL && pParent->IsWindowVisible())
		SendMessageToVisibleDescendants(m_hWnd, WM_SHOWPARENT, bShow, nStatus);
}
