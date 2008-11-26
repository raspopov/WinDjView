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

#include "DjVuDoc.h"
#include "ThumbnailsView.h"
#include "Drawing.h"
#include "ThumbnailsThread.h"
#include "ChildFrm.h"
#include "DjVuView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


const int nVertMargin = 3;
const int nHorzMargin = 3;
const int nFrameWidth = 1;
const int nPadding = 3;
const int nNumberSkip = 2;
const int nNumberHeight = 15;
const int nNumberWidth = 34;


// CThumbnailsView

IMPLEMENT_DYNAMIC(CThumbnailsView, CMyScrollView)

BEGIN_MESSAGE_MAP(CThumbnailsView, CMyScrollView)
	ON_WM_ERASEBKGND()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEACTIVATE()
	ON_MESSAGE(WM_THUMBNAIL_RENDERED, OnThumbnailRendered)
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(WM_SHOW_SETTINGS, OnShowSettings)
END_MESSAGE_MAP()

// CThumbnailsView construction/destruction

CThumbnailsView::CThumbnailsView(DjVuSource* pSource)
	: m_bInsideUpdateView(false), m_nPageCount(0), m_bVisible(false),
	  m_pThread(NULL), m_pIdleThread(NULL), m_nSelectedPage(-1), m_pSource(pSource),
	  m_nCurrentPage(-1), m_nRotate(0), m_nPagesInRow(1), m_bInitialized(false)
{
	m_pSource->AddRef();

	CFont systemFont;
	CreateSystemDialogFont(systemFont);

	LOGFONT lf;
	systemFont.GetLogFont(&lf);

	_tcscpy(lf.lfFaceName, _T("Arial"));
	lf.lfHeight = -12;
	m_font.CreateFontIndirect(&lf);

	m_nThumbnailSize = theApp.GetAppSettings()->nThumbnailSize;
	m_szThumbnail.cx = CAppSettings::thumbnailWidth[m_nThumbnailSize];
	m_szThumbnail.cy = CAppSettings::thumbnailHeight[m_nThumbnailSize];
}

CThumbnailsView::~CThumbnailsView()
{
	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		m_pages[nPage].DeleteBitmap();

	m_dataLock.Lock();

	for (set<CDIB*>::iterator it = m_bitmaps.begin(); it != m_bitmaps.end(); ++it)
		delete *it;
	m_bitmaps.clear();

	m_dataLock.Unlock();

	if (m_pSource != NULL)
		m_pSource->Release();
}

// CThumbnailsView drawing

void CThumbnailsView::OnDraw(CDC* pDC)
{
	// CScrollView sets viewport to minus scroll position, so that
	// all drawing can be done in natural coordinates. Unfortunately,
	// this does not work in Win98, because in this OS coordinates
	// cannot be larger than 32767. So we will subtract scroll position
	// explicitely.
	pDC->SetViewportOrg(CPoint(0, 0));

	CRect rcClip;
	pDC->GetClipBox(rcClip);
	rcClip.OffsetRect(GetDeviceScrollPosition());

	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		Page& page = m_pages[nPage];

		if (page.rcDisplay.top < rcClip.bottom &&
			page.rcDisplay.bottom > rcClip.top)
		{
			DrawPage(pDC, nPage);
		}
	}
}

void CThumbnailsView::DrawPage(CDC* pDC, int nPage)
{
	Page& page = m_pages[nPage];

	COLORREF clrWindow = ::GetSysColor(COLOR_WINDOW);
	COLORREF clrFrame = ::GetSysColor(COLOR_WINDOWFRAME);
	COLORREF clrShadow = ::GetSysColor(COLOR_BTNSHADOW);
	COLORREF clrBtnface = ::GetSysColor(COLOR_BTNFACE);
	COLORREF clrHilight = ::GetSysColor(COLOR_HIGHLIGHT);
	static CPen penFrame(PS_SOLID, 1, clrFrame);

	CPoint ptScrollPos = GetDeviceScrollPosition();

	CRect rcClip;
	pDC->GetClipBox(rcClip);
	rcClip.OffsetRect(ptScrollPos);
	rcClip.top = max(rcClip.top, page.rcDisplay.top);
	rcClip.bottom = min(rcClip.bottom, page.rcDisplay.bottom);

	if (rcClip.IsRectEmpty())
		return;

	CRect rcCorner;

	if (page.pBitmap != NULL && page.pBitmap->IsValid())
	{
		page.pBitmap->Draw(pDC, page.rcBitmap.TopLeft() - ptScrollPos, page.szDisplay);
	}
	else
	{
		pDC->FillSolidRect(page.rcBitmap - ptScrollPos, clrWindow);

		// Draw corner
		rcCorner = CRect(page.rcBitmap.TopLeft(), CSize(15, 15));
		rcCorner.OffsetRect(-1, -1);
		rcCorner.OffsetRect(-ptScrollPos);

		CPoint points[] = {
				CPoint(rcCorner.left, rcCorner.bottom - 1),
				CPoint(rcCorner.right - 1, rcCorner.top),
				CPoint(rcCorner.right - 1, rcCorner.bottom - 1),
				CPoint(rcCorner.left, rcCorner.bottom - 1) };

		CPen* pOldPen = pDC->SelectObject(&penFrame);
		pDC->Polyline(points, 4);
		pDC->SelectObject(pOldPen);

		CRect rcWhiteBar(CPoint(rcCorner.left, rcCorner.top), CSize(1, rcCorner.Height() - 1));
		pDC->FillSolidRect(rcWhiteBar, clrWindow);
		rcWhiteBar = CRect(CPoint(rcCorner.left, rcCorner.top), CSize(rcCorner.Width() - 1, 1));
		pDC->FillSolidRect(rcWhiteBar, clrWindow);
	}

	// Page border and shadow
	CRect rcBorder(page.rcBitmap);

	if (nPage == m_nSelectedPage && GetFocus() == this)
	{
		for (int i = 0; i < 4; i++)
		{
			rcBorder.InflateRect(1, 1);
			FrameRect(pDC, rcBorder - ptScrollPos, clrHilight);
		}
	}
	else
	{
		rcBorder.InflateRect(1, 1);

		if (page.pBitmap != NULL && page.pBitmap->IsValid())
		{
			FrameRect(pDC, rcBorder - ptScrollPos, clrFrame);
		}
		else
		{
			int nSaveDC = pDC->SaveDC();
			pDC->ExcludeClipRect(rcCorner);
			FrameRect(pDC, rcBorder - ptScrollPos, clrFrame);
			pDC->RestoreDC(nSaveDC);
		}

		if (nPage == m_nSelectedPage)
		{
			CRect rcInactiveBorder = rcBorder;
			for (int i = 0; i < 3; i++)
			{
				rcInactiveBorder.InflateRect(1, 1);
				FrameRect(pDC, rcInactiveBorder - ptScrollPos, clrBtnface);
			}
		}
		else
		{
			CRect rcWhiteBar(CPoint(rcBorder.left, rcBorder.bottom), CSize(3, 1));
			pDC->FillSolidRect(rcWhiteBar - ptScrollPos, clrWindow);
			rcWhiteBar = CRect(CPoint(rcBorder.right, rcBorder.top), CSize(1, 3));
			pDC->FillSolidRect(rcWhiteBar - ptScrollPos, clrWindow);
		}

		CRect rcShadow(CPoint(rcBorder.left + 3, rcBorder.bottom), CSize(rcBorder.Width() - 2, 1));
		pDC->FillSolidRect(rcShadow - ptScrollPos, clrShadow);
		rcShadow = CRect(CPoint(rcBorder.right, rcBorder.top + 3), CSize(1, rcBorder.Height() - 2));
		pDC->FillSolidRect(rcShadow - ptScrollPos, clrShadow);

		rcBorder.InflateRect(0, 0, 1, 1);
		if (nPage == m_nSelectedPage)
			rcBorder.InflateRect(3, 3, 2, 2);
	}

	// Page number
	CRect rcNumberFrame(page.rcNumber);
	FrameRect(pDC, rcNumberFrame - ptScrollPos, clrFrame);

	CRect rcNumber(rcNumberFrame);
	rcNumber.DeflateRect(1, 1);
	COLORREF color = (nPage == m_nCurrentPage ? clrHilight : clrWindow);
	pDC->FillSolidRect(rcNumber - ptScrollPos, color);

	CFont* pOldFont = pDC->SelectObject(&m_font);
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(nPage == m_nCurrentPage ? clrWindow : clrFrame);
	pDC->DrawText(FormatString(_T("%d"), nPage + 1), rcNumber - ptScrollPos,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pDC->SelectObject(pOldFont);

	if (nPage == m_nSelectedPage)
	{
		COLORREF color = (GetFocus() == this ? clrHilight : clrBtnface);

		for (int i = 0; i < 3; i++)
		{
			if (GetFocus() == this || i == 0)
				rcNumberFrame.InflateRect(1, 1);
			else
				rcNumberFrame.InflateRect(1, 0, 1, 1);

			FrameRect(pDC, rcNumberFrame - ptScrollPos, color);
		}
	}

	// Fill everything else with window background color
	int nSaveDC = pDC->SaveDC();
	pDC->ExcludeClipRect(rcBorder - ptScrollPos);
	pDC->ExcludeClipRect(rcNumberFrame - ptScrollPos);
	pDC->FillSolidRect(page.rcDisplay - ptScrollPos, clrWindow);
	pDC->RestoreDC(nSaveDC);
}

// CThumbnailsView diagnostics

#ifdef _DEBUG
void CThumbnailsView::AssertValid() const
{
	CMyScrollView::AssertValid();
}

void CThumbnailsView::Dump(CDumpContext& dc) const
{
	CMyScrollView::Dump(dc);
}
#endif //_DEBUG


// CThumbnailsView message handlers

void CThumbnailsView::OnInitialUpdate()
{
	CMyScrollView::OnInitialUpdate();

	theApp.AddObserver(this);

	m_nPageCount = m_pSource->GetPageCount();
	m_pages.resize(m_nPageCount);

	m_displaySetting = *theApp.GetDisplaySettings();
	m_displaySetting.nScaleMethod = CDisplaySettings::Default;

	m_pThread = new CThumbnailsThread(m_pSource, this);
	m_pThread->SetThumbnailSize(m_szThumbnail);

	m_pIdleThread = new CThumbnailsThread(m_pSource, this, true);
	m_pIdleThread->SetThumbnailSize(m_szThumbnail);

	UpdateView(RECALC);
}

void CThumbnailsView::UpdateAllThumbnails()
{
	if (m_pThread != NULL && m_pIdleThread != NULL)
	{
		m_pThread->RejectCurrentJob();
		m_pIdleThread->RejectCurrentJob();
	}

	UpdateView(RECALC);
	UpdateVisiblePages();

	Invalidate();
	UpdateWindow();
}

BOOL CThumbnailsView::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void CThumbnailsView::OnSize(UINT nType, int cx, int cy)
{
	if (m_nPageCount != 0)
	{
		UpdateView(TOP);
	}

	CMyScrollView::OnSize(nType, cx, cy);
}

void CThumbnailsView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CSize szScroll(0, 0);
	switch (nChar)
	{
	case VK_RIGHT:
		if (m_nSelectedPage != -1)
		{
			if (m_nSelectedPage < m_nPageCount - 1)
				SetSelectedPage(m_nSelectedPage + 1);
		}
		else
			SetSelectedPage(0);
		return;

	case VK_LEFT:
		if (m_nSelectedPage != -1)
		{
			if (m_nSelectedPage >= 1)
				SetSelectedPage(m_nSelectedPage - 1);
		}
		else
			SetSelectedPage(m_nPageCount - 1);
		return;

	case VK_DOWN:
		if (m_nSelectedPage != -1)
		{
			if (m_nSelectedPage < m_nPageCount - m_nPagesInRow)
				SetSelectedPage(m_nSelectedPage + m_nPagesInRow);
		}
		else
			SetSelectedPage(0);
		return;

	case VK_UP:
		if (m_nSelectedPage != -1)
		{
			if (m_nSelectedPage >= m_nPagesInRow)
				SetSelectedPage(m_nSelectedPage - m_nPagesInRow);
		}
		else
			SetSelectedPage(m_nPageCount - 1);
		return;

	case VK_RETURN:
	case VK_SPACE:
		if (m_nSelectedPage != -1)
		{
			UpdateObservers(PageMsg(THUMBNAIL_CLICKED, m_nSelectedPage));
		}
		return;

	case VK_NEXT:
		szScroll.cy = m_pageDev.cy;
		break;

	case VK_PRIOR:
	case VK_BACK:
		szScroll.cy = -m_pageDev.cy;
		break;

	case VK_HOME:
		szScroll.cy = -GetScrollPosition().y;
		break;

	case VK_END:
		szScroll.cy = GetScrollLimit(SB_VERT) - GetScrollPosition().y;
		break;
	}

	OnScrollBy(szScroll);

	if (szScroll.cy != 0)
		UpdateVisiblePages();

	CMyScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CThumbnailsView::OnSetFocus(CWnd* pOldWnd)
{
	if (m_nSelectedPage != -1)
		InvalidatePage(m_nSelectedPage);

	CMyScrollView::OnSetFocus(pOldWnd);
}

void CThumbnailsView::OnKillFocus(CWnd* pNewWnd)
{
	if (m_nSelectedPage != -1)
		InvalidatePage(m_nSelectedPage);

	CMyScrollView::OnKillFocus(pNewWnd);
}

void CThumbnailsView::SetSelectedPage(int nPage)
{
	if (nPage == m_nSelectedPage)
		return;

	if (m_nSelectedPage != -1)
		InvalidatePage(m_nSelectedPage);

	m_nSelectedPage = nPage;

	if (m_nSelectedPage != -1)
	{
		InvalidatePage(m_nSelectedPage);
		EnsureVisible(m_nSelectedPage);
	}

	UpdateWindow();
}

void CThumbnailsView::SetCurrentPage(int nPage)
{
	if (nPage == m_nCurrentPage)
		return;

	if (m_nCurrentPage != -1)
		InvalidatePage(m_nCurrentPage);

	m_nCurrentPage = nPage;

	if (m_nCurrentPage != -1)
		InvalidatePage(m_nCurrentPage);

	UpdateWindow();
}

void CThumbnailsView::OnLButtonDown(UINT nFlags, CPoint point)
{
	int nPage = GetPageFromPoint(point);
	SetSelectedPage(nPage);

	if (nPage != -1)
	{
		UpdateObservers(PageMsg(THUMBNAIL_CLICKED, nPage));
	}

	CMyScrollView::OnLButtonDown(nFlags, point);
}

void CThumbnailsView::OnRButtonDown(UINT nFlags, CPoint point)
{
	int nPage = GetPageFromPoint(point);
	SetSelectedPage(nPage);

	CMyScrollView::OnRButtonDown(nFlags, point);
}

LRESULT CThumbnailsView::OnShowSettings(WPARAM wParam, LPARAM lParam)
{
	CMenu menu;
	menu.LoadMenu(IDR_POPUP);

	CMenu* pPopup = menu.GetSubMenu(2);
	ASSERT(pPopup != NULL);

	CRect rcButton = (LPRECT) lParam;
	TPMPARAMS tpm;
	tpm.cbSize = sizeof(tpm);
	tpm.rcExclude = rcButton;


	if (m_nThumbnailSize == 0)
		pPopup->EnableMenuItem(ID_THUMBNAILS_REDUCE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	if (m_nThumbnailSize == CAppSettings::ThumbnailSizes - 1)
		pPopup->EnableMenuItem(ID_THUMBNAILS_ENLARGE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	int nID = pPopup->TrackPopupMenuEx(TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			rcButton.left, rcButton.bottom, this, &tpm);

	int nThumbnailSize = m_nThumbnailSize;
	if (nID == ID_THUMBNAILS_REDUCE)
		--nThumbnailSize;
	else if (nID == ID_THUMBNAILS_ENLARGE)
		++nThumbnailSize;
	nThumbnailSize = max(0, min(CAppSettings::ThumbnailSizes - 1, nThumbnailSize));

	if (nThumbnailSize != m_nThumbnailSize)
	{
		m_nThumbnailSize = nThumbnailSize;
		m_szThumbnail.cx = CAppSettings::thumbnailWidth[m_nThumbnailSize];
		m_szThumbnail.cy = CAppSettings::thumbnailHeight[m_nThumbnailSize];

		if (m_pThread != NULL && m_pIdleThread != NULL)
		{
			m_pThread->SetThumbnailSize(m_szThumbnail);
			m_pIdleThread->SetThumbnailSize(m_szThumbnail);
		}

		UpdateAllThumbnails();
	}

	theApp.GetAppSettings()->nThumbnailSize = nThumbnailSize;

	return 0;
}

BOOL CThumbnailsView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
	CRect rcClient;
	GetClientRect(rcClient);

	int nCode = HIBYTE(nScrollCode);
	if (nCode == SB_THUMBTRACK)
	{
		SCROLLINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(si);

		if (!GetScrollInfo(SB_VERT, &si, SIF_TRACKPOS))
			return true;

		int yOrig = GetScrollPos(SB_VERT);

		BOOL bResult = OnScrollBy(CSize(0, si.nTrackPos - yOrig), bDoScroll);
		if (bResult && bDoScroll)
		{
			UpdateVisiblePages();
			UpdateWindow();
		}

		return bResult;
	}

	BOOL bResult = CMyScrollView::OnScroll(nScrollCode, nPos, bDoScroll);
	UpdateVisiblePages();

	return bResult;
}

BOOL CThumbnailsView::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	// we don't handle anything but scrolling
	if ((nFlags & MK_CONTROL) != 0)
		return true;

	CWnd* pWnd = WindowFromPoint(point);
	if (pWnd != this && !IsChild(pWnd) && IsFromCurrentProcess(pWnd) &&
			pWnd->SendMessage(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(point.x, point.y)) != 0)
		return true;

	BOOL bHasHorzBar, bHasVertBar;
	CheckScrollBars(bHasHorzBar, bHasVertBar);

	if (!bHasVertBar && !bHasHorzBar)
		return false;

	UINT uWheelScrollLines = GetMouseScrollLines();
	int nToScroll = ::MulDiv(-zDelta, uWheelScrollLines, WHEEL_DELTA);
	int nDisplacement;

	if (bHasVertBar && (!bHasHorzBar || (nFlags & MK_SHIFT) == 0))
	{
		if (uWheelScrollLines == WHEEL_PAGESCROLL)
		{
			nDisplacement = m_pageDev.cy;
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
		}
		else
		{
			nDisplacement = nToScroll * m_lineDev.cy;
			nDisplacement = min(nDisplacement, m_pageDev.cy);
		}

		OnScrollBy(CSize(0, nDisplacement));
		UpdateVisiblePages();
	}
	else if (bHasHorzBar)
	{
		if (uWheelScrollLines == WHEEL_PAGESCROLL)
		{
			nDisplacement = m_pageDev.cx;
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
		}
		else
		{
			nDisplacement = nToScroll * m_lineDev.cx;
			nDisplacement = min(nDisplacement, m_pageDev.cx);
		}

		OnScrollBy(CSize(nDisplacement, 0));
	}

	UpdateWindow();
	return true;
}

bool CThumbnailsView::InvalidatePage(int nPage)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);

	CRect rect;
	GetClientRect(rect);

	CPoint ptScrollPos = GetDeviceScrollPosition();
	if (rect.IntersectRect(rect, m_pages[nPage].rcDisplay - ptScrollPos))
	{
		InvalidateRect(rect);
		return true;
	}

	return false;
}

int CThumbnailsView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	// From MFC: CView::OnMouseActivate
	// Don't call CFrameWnd::SetActiveView

	int nResult = CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
	if (nResult == MA_NOACTIVATE || nResult == MA_NOACTIVATEANDEAT)
		return nResult;

	if (message != WM_LBUTTONDOWN)
		return MA_NOACTIVATE;

	// set focus to this view, but don't notify the parent frame
	OnActivateView(TRUE, this, this);
	return nResult;
}

void CThumbnailsView::UpdateView(UpdateType updateType)
{
	if (m_bInsideUpdateView)
		return;

	m_bInsideUpdateView = true;

	// Save page and offset to restore after changes
	int nAnchorPage;
	CPoint ptAnchorOffset;
	CPoint ptTop = GetScrollPosition();

	if (updateType == TOP)
	{
		int nTopPage = 0;
		while (nTopPage < m_nPageCount - 1 &&
				ptTop.y >= m_pages[nTopPage].rcDisplay.bottom)
			++nTopPage;

		nAnchorPage = nTopPage;
		ptAnchorOffset = ptTop - m_pages[nTopPage].rcPage.TopLeft();
	}

	int nThumbnailWidth = 2*nHorzMargin + 2*nFrameWidth + m_szThumbnail.cx;
	int nThumbnailHeight = 2*nVertMargin + 2*nFrameWidth + m_szThumbnail.cy
			+ nNumberSkip + nNumberHeight;

	for (int i = 0; i < 2; ++i)
	{
		CRect rcClient;
		GetClientRect(rcClient);

		m_nPagesInRow = max(1, (rcClient.Width() - 2*nPadding) / nThumbnailWidth);
		m_nPagesInRow = min(m_nPagesInRow, m_nPageCount);
		int nRowCount = (m_nPageCount - 1) / m_nPagesInRow + 1;

		m_szDisplay = CSize(max(rcClient.Width(), m_nPagesInRow*nThumbnailWidth + 2*nPadding),
			max(rcClient.Height(), nThumbnailHeight*nRowCount + 2*nPadding));

		double nOffsetX = (rcClient.Width() - 2*nPadding - nThumbnailWidth*m_nPagesInRow) / (2.0*m_nPagesInRow);
		if (nOffsetX < 0)
			nOffsetX = 0;

		for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		{
			int nRow = nPage / m_nPagesInRow;
			int nCol = nPage % m_nPagesInRow;

			Page& page = m_pages[nPage];

			CPoint ptOffset(nPadding + nCol*nThumbnailWidth + static_cast<int>(2*nCol*nOffsetX),
					nPadding + nRow*nThumbnailHeight);
			int nWidthExtra = static_cast<int>(2*(nCol + 1)*nOffsetX) - static_cast<int>(2*nCol*nOffsetX);
			page.rcDisplay = CRect(ptOffset, CSize(nThumbnailWidth + nWidthExtra, nThumbnailHeight));

			ptOffset.Offset(static_cast<int>(nOffsetX), 0);

			page.rcPage = CRect(CPoint(ptOffset.x + nHorzMargin + nFrameWidth,
				ptOffset.y + nVertMargin + nFrameWidth), m_szThumbnail);

			if (nCol == 0)
				page.rcDisplay.left = 0;
			if (nCol == m_nPagesInRow - 1)
				page.rcDisplay.right = m_szDisplay.cx;
			if (nRow == 0)
				page.rcDisplay.top = 0;
			if (nRow == nRowCount - 1)
			{
				page.rcDisplay.bottom = m_szDisplay.cy;
				if (nPage == m_nPageCount - 1 && nCol != m_nPagesInRow - 1)
					page.rcDisplay.right = m_szDisplay.cx;
			}

			RecalcPageRects(nPage);
		}

		CSize szDevPage(rcClient.Width()*9/10, rcClient.Height()*9/10);
		CSize szDevLine(15, 15);

		SetScrollSizesNoRepaint(m_szDisplay, szDevPage, szDevLine);
	}

	if (updateType == TOP)
	{
		ScrollToPositionNoRepaint(m_pages[nAnchorPage].rcPage.TopLeft() + ptAnchorOffset);
	}

	m_bInsideUpdateView = false;
	UpdateVisiblePages();
}

void CThumbnailsView::OnDestroy()
{
	if (m_pThread != NULL)
	{
		m_pThread->Stop();
		m_pThread = NULL;
	}

	if (m_pIdleThread != NULL)
	{
		m_pIdleThread->Stop();
		m_pIdleThread = NULL;
	}

	theApp.RemoveObserver(this);

	CMyScrollView::OnDestroy();
}

void CThumbnailsView::UpdateVisiblePages()
{
	if (!m_bInitialized || m_pThread->IsPaused())
		return;

	CRect rcClient;
	GetClientRect(rcClient);
	int nTop = GetScrollPos(SB_VERT);

	m_pThread->RemoveAllJobs();
	m_pIdleThread->RemoveAllJobs();

	if (m_bVisible)
	{
		m_pThread->PauseJobs();
		m_pIdleThread->PauseJobs();

		int nTopPage = 0;
		while (nTopPage < m_nPageCount - 1 &&
				nTop >= m_pages[nTopPage].rcDisplay.bottom)
			++nTopPage;

		int nBottomPage = nTopPage + 1;
		while (nBottomPage < m_nPageCount &&
				m_pages[nBottomPage].rcDisplay.top < nTop + rcClient.Height())
			++nBottomPage;

		if (theApp.GetAppSettings()->bGenAllThumbnails)
		{
			for (int nDiff = m_nPageCount; nDiff >= 1; --nDiff)
			{
				if (nTopPage - nDiff >= 0)
					UpdatePage(nTopPage - nDiff, m_pIdleThread);
				if (nBottomPage + nDiff - 1 < m_nPageCount)
					UpdatePage(nBottomPage + nDiff - 1, m_pIdleThread);
			}
		}

		for (int nPage = nBottomPage - 1; nPage >= nTopPage; --nPage)
			UpdatePage(nPage, m_pThread);

		m_pThread->ResumeJobs();
		m_pIdleThread->ResumeJobs();
	}
}

void CThumbnailsView::UpdatePage(int nPage, CThumbnailsThread* pThread)
{
	Page& page = m_pages[nPage];

	if (!page.bRendered ||
		!(page.szBitmap.cx <= m_szThumbnail.cx && page.szBitmap.cy == m_szThumbnail.cy) ||
		!(page.szBitmap.cx == m_szThumbnail.cx && page.szBitmap.cy <= m_szThumbnail.cy))
	{
		pThread->AddJob(nPage, m_nRotate, m_displaySetting);
		InvalidatePage(nPage);
	}
}

LRESULT CThumbnailsView::OnThumbnailRendered(WPARAM wParam, LPARAM lParam)
{
	int nPage = (int)wParam;
	CDIB* pBitmap = reinterpret_cast<CDIB*>(lParam);

	m_dataLock.Lock();
	m_bitmaps.erase(pBitmap);
	m_dataLock.Unlock();

	Page& page = m_pages[nPage];
	page.DeleteBitmap();
	page.bRendered = true;
	if (pBitmap != NULL)
	{
		page.pBitmap = CLightweightDIB::Create(pBitmap);
		page.szBitmap = pBitmap->GetSize();
		delete pBitmap;
	}
	else
	{
		page.szBitmap = m_szThumbnail;
	}

	RecalcPageRects(nPage);

	if (InvalidatePage(nPage))
		UpdateWindow();

	return 0;
}

void CThumbnailsView::OnUpdate(const Observable* source, const Message* message)
{
	if (message->code == THUMBNAIL_RENDERED)
	{
		const BitmapMsg* msg = static_cast<const BitmapMsg*>(message);

		LPARAM lParam = reinterpret_cast<LPARAM>(msg->pDIB);
		if (msg->pDIB != NULL)
		{
			m_dataLock.Lock();
			m_bitmaps.insert(msg->pDIB);
			m_dataLock.Unlock();
		}

		PostMessage(WM_THUMBNAIL_RENDERED, msg->nPage, lParam);
	}
	else if (message->code == CURRENT_PAGE_CHANGED)
	{
		const PageMsg* msg = static_cast<const PageMsg*>(message);

		if (GetCurrentPage() != msg->nPage)
		{
			SetCurrentPage(msg->nPage);
			EnsureVisible(msg->nPage);
		}
	}
	else if (message->code == ROTATE_CHANGED)
	{
		const RotateChanged* msg = static_cast<const RotateChanged*>(message);

		if (m_nRotate != msg->nRotate)
		{
			m_nRotate = msg->nRotate;

			for (int nPage = 0; nPage < m_nPageCount; ++nPage)
				m_pages[nPage].DeleteBitmap();

			UpdateAllThumbnails();
		}
	}
	else if (message->code == APP_SETTINGS_CHANGED)
	{
		SettingsChanged();
	}
	else if (message->code == VIEW_INITIALIZED)
	{
		const CDjVuView* pView = static_cast<const CDjVuView*>(source);
		m_nRotate = pView->GetRotate();

		m_bInitialized = true;
		UpdateView(RECALC);
	}
}

void CThumbnailsView::RecalcPageRects(int nPage)
{
	Page& page = m_pages[nPage];

	if (page.pBitmap != NULL)
	{
		page.szDisplay.cx = m_szThumbnail.cx;
		page.szDisplay.cy = page.szBitmap.cy * m_szThumbnail.cx / page.szBitmap.cx;
		if (page.szDisplay.cy > m_szThumbnail.cy)
		{
			page.szDisplay.cy = m_szThumbnail.cy;
			page.szDisplay.cx = page.szBitmap.cx * m_szThumbnail.cy / page.szBitmap.cy;
		}

		CPoint ptOffset = page.rcPage.Size() - page.szDisplay;
		page.rcBitmap = CRect(page.rcPage.TopLeft() + CPoint(ptOffset.x / 2, ptOffset.y / 2),
			page.szDisplay);
	}
	else
	{
		page.rcBitmap = page.rcPage;
	}

	page.rcNumber = CRect(CPoint(page.rcPage.CenterPoint().x - nNumberWidth / 2,
		page.rcBitmap.bottom + nFrameWidth + nNumberSkip), CSize(nNumberWidth, nNumberHeight));
}

void CThumbnailsView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CMyScrollView::OnShowWindow(bShow, nStatus);

	m_bVisible = !!bShow;
	UpdateVisiblePages();
}

int CThumbnailsView::GetPageFromPoint(CPoint point)
{
	point += GetScrollPosition();

	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		if (m_pages[nPage].rcBitmap.PtInRect(point)
			|| m_pages[nPage].rcNumber.PtInRect(point))
		{
			return nPage;
		}
	}

	return -1;
}

void CThumbnailsView::EnsureVisible(int nPage)
{
	CRect rcClient;
	GetClientRect(rcClient);

	CPoint ptScrollPos = GetDeviceScrollPosition();
	Page& page = m_pages[nPage];

	int nScrollY = 0;
	if (page.rcDisplay.top <= ptScrollPos.y)
		nScrollY = page.rcDisplay.top - ptScrollPos.y;
	else if (page.rcDisplay.bottom > ptScrollPos.y + rcClient.Height())
		nScrollY = page.rcDisplay.bottom - ptScrollPos.y - rcClient.Height();

	if (nScrollY != 0)
	{
		OnScrollBy(CSize(0, nScrollY));
		UpdateVisiblePages();
	}
}

void CThumbnailsView::PauseDecoding()
{
	m_pThread->PauseJobs();
	m_pThread->RemoveAllJobs();

	m_pIdleThread->PauseJobs();
	m_pIdleThread->RemoveAllJobs();
}

void CThumbnailsView::ResumeDecoding()
{
	m_pThread->ResumeJobs();
	m_pIdleThread->ResumeJobs();

	UpdateView(TOP);
}

void CThumbnailsView::SettingsChanged()
{
	CDisplaySettings appSettings = *theApp.GetDisplaySettings();
	appSettings.nScaleMethod = CDisplaySettings::Default;

	if (m_displaySetting != appSettings)
	{
		m_displaySetting = appSettings;

		for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		{
			m_pages[nPage].DeleteBitmap();
			InvalidatePage(nPage);
		}
	}

	UpdateVisiblePages();
}

void CThumbnailsView::OnPan(CSize szScroll)
{
	OnScrollBy(szScroll, true);

	if (szScroll.cy != 0)
		UpdateVisiblePages();
}
