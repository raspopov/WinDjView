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

#include "ThumbnailsView.h"
#include "Drawing.h"
#include "ThumbnailsThread.h"
#include "DjVuView.h"
#include "NavPane.h"
#include "TabbedMDIWnd.h"

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
	ON_MESSAGE(WM_SHOWPARENT, OnShowParent)
	ON_WM_MENUSELECT()
	ON_WM_ENTERIDLE()
END_MESSAGE_MAP()

// CThumbnailsView construction/destruction

CThumbnailsView::CThumbnailsView(DjVuSource* pSource)
	: m_bInsideUpdateLayout(false), m_nPageCount(0), m_bVisible(false),
	  m_pThread(NULL), m_pIdleThread(NULL), m_nActivePage(-1), m_pSource(pSource),
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
	CRect rcClip;
	pDC->GetClipBox(rcClip);
	rcClip.OffsetRect(GetScrollPosition());

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
	COLORREF clrWindowText = ::GetSysColor(COLOR_WINDOWTEXT);
	COLORREF clrShadow = ::GetSysColor(COLOR_BTNSHADOW);
	COLORREF clrFrame = ChangeBrightness(clrShadow, 0.6);
	COLORREF clrBtnface = ::GetSysColor(COLOR_BTNFACE);
	COLORREF clrHilight = ::GetSysColor(COLOR_HIGHLIGHT);
	static CPen penFrame(PS_SOLID, 1, clrFrame);

	CPoint ptScroll = GetScrollPosition();

	CRect rcClip;
	pDC->GetClipBox(rcClip);
	rcClip.OffsetRect(ptScroll);
	rcClip.top = max(rcClip.top, page.rcDisplay.top);
	rcClip.bottom = min(rcClip.bottom, page.rcDisplay.bottom);

	if (rcClip.IsRectEmpty())
		return;

	CRect rcCorner;

	if (page.pBitmap != NULL && page.pBitmap->IsValid())
	{
		page.pBitmap->Draw(pDC, page.rcBitmap.TopLeft() - ptScroll, page.szDisplay);
	}
	else
	{
		pDC->FillSolidRect(page.rcBitmap - ptScroll, clrWindow);

		// Draw corner
		rcCorner = CRect(page.rcBitmap.TopLeft(), CSize(15, 15)) - CPoint(1, 1) - ptScroll;

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

	if (page.bSelected && GetFocus() == this)
	{
		for (int i = 0; i < 4; i++)
		{
			rcBorder.InflateRect(1, 1);
			FrameRect(pDC, rcBorder - ptScroll, clrHilight);
		}
	}
	else
	{
		rcBorder.InflateRect(1, 1);

		if (page.pBitmap != NULL && page.pBitmap->IsValid())
		{
			FrameRect(pDC, rcBorder - ptScroll, clrFrame);
		}
		else
		{
			int nSaveDC = pDC->SaveDC();
			pDC->ExcludeClipRect(rcCorner);
			FrameRect(pDC, rcBorder - ptScroll, clrFrame);
			pDC->RestoreDC(nSaveDC);
		}

		if (page.bSelected)
		{
			CRect rcInactiveBorder = rcBorder;
			for (int i = 0; i < 3; i++)
			{
				rcInactiveBorder.InflateRect(1, 1);
				FrameRect(pDC, rcInactiveBorder - ptScroll, clrBtnface);
			}
		}
		else
		{
			CRect rcWhiteBar(CPoint(rcBorder.left, rcBorder.bottom), CSize(3, 1));
			pDC->FillSolidRect(rcWhiteBar - ptScroll, clrWindow);
			rcWhiteBar = CRect(CPoint(rcBorder.right, rcBorder.top), CSize(1, 3));
			pDC->FillSolidRect(rcWhiteBar - ptScroll, clrWindow);
		}

		CRect rcShadow(CPoint(rcBorder.left + 3, rcBorder.bottom), CSize(rcBorder.Width() - 2, 1));
		pDC->FillSolidRect(rcShadow - ptScroll, clrShadow);
		rcShadow = CRect(CPoint(rcBorder.right, rcBorder.top + 3), CSize(1, rcBorder.Height() - 2));
		pDC->FillSolidRect(rcShadow - ptScroll, clrShadow);

		rcBorder.InflateRect(0, 0, 1, 1);
		if (page.bSelected)
			rcBorder.InflateRect(3, 3, 2, 2);
	}

	bool bFocus = (GetFocus() == this);

	// Page number
	CRect rcNumberFrame(page.rcNumber);
	FrameRect(pDC, rcNumberFrame - ptScroll, clrFrame);

	CRect rcNumber(rcNumberFrame);
	rcNumber.DeflateRect(1, 1);
	COLORREF color = (nPage == m_nCurrentPage ? clrBtnface : clrWindow);
	pDC->FillSolidRect(rcNumber - ptScroll, color);

	CFont* pOldFont = pDC->SelectObject(&m_font);
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(clrWindowText);
	pDC->DrawText(FormatString(_T("%d"), nPage + 1), rcNumber - ptScroll,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pDC->SelectObject(pOldFont);

	if (page.bSelected)
	{
		COLORREF color = (bFocus ? clrHilight : clrBtnface);

		for (int i = 0; i < 3; i++)
		{
			if (bFocus || i == 0)
				rcNumberFrame.InflateRect(1, 1);
			else
				rcNumberFrame.InflateRect(1, 0, 1, 1);

			FrameRect(pDC, rcNumberFrame - ptScroll, color);
		}
	}

	// Fill everything else with window background color
	int nSaveDC = pDC->SaveDC();
	pDC->ExcludeClipRect(rcBorder - ptScroll);
	pDC->ExcludeClipRect(rcNumberFrame - ptScroll);
	pDC->FillSolidRect(page.rcDisplay - ptScroll, clrWindow);
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

	m_displaySettings = *theApp.GetDisplaySettings();
	m_displaySettings.bScaleColorPnm = false;
	m_displaySettings.bScaleSubpix = false;

	m_pThread = new CThumbnailsThread(m_pSource, this);
	m_pIdleThread = new CThumbnailsThread(m_pSource, this, true);

	UpdateLayout(RECALC);
}

void CThumbnailsView::UpdateAllThumbnails()
{
	if (m_pThread != NULL && m_pIdleThread != NULL)
	{
		m_pThread->RejectCurrentJob();
		m_pIdleThread->RejectCurrentJob();
	}

	UpdateLayout();

	InvalidateViewport();
	UpdateWindow();
}

BOOL CThumbnailsView::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void CThumbnailsView::OnSize(UINT nType, int cx, int cy)
{
	if (m_nPageCount != 0)
		UpdateLayout(TOP);

	CMyScrollView::OnSize(nType, cx, cy);
}

void CThumbnailsView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CSize szScrollBy(0, 0);
	switch (nChar)
	{
	case VK_RIGHT:
		if (m_nActivePage == -1)
			SetActivePage(0);
		else if (m_nActivePage < m_nPageCount - 1)
			SetActivePage(m_nActivePage + 1);
		return;

	case VK_LEFT:
		if (m_nActivePage == -1)
			SetActivePage(m_nPageCount - 1);
		else if (m_nActivePage >= 1)
			SetActivePage(m_nActivePage - 1);
		return;

	case VK_DOWN:
		if (m_nActivePage == -1)
			SetActivePage(0);
		else if (m_nActivePage < m_nPageCount - m_nPagesInRow)
			SetActivePage(m_nActivePage + m_nPagesInRow);
		return;

	case VK_UP:
		if (m_nActivePage == -1)
			SetActivePage(m_nPageCount - 1);
		else if (m_nActivePage >= m_nPagesInRow)
			SetActivePage(m_nActivePage - m_nPagesInRow);
		return;

	case VK_RETURN:
	case VK_SPACE:
		if (m_nActivePage != -1)
		{
			UpdateObservers(PageMsg(THUMBNAIL_CLICKED, m_nActivePage));
		}
		return;

	case VK_NEXT:
		szScrollBy.cy = m_szPage.cy;
		break;

	case VK_PRIOR:
	case VK_BACK:
		szScrollBy.cy = -m_szPage.cy;
		break;

	case VK_HOME:
		szScrollBy.cy = -GetScrollPosition().y;
		break;

	case VK_END:
		szScrollBy.cy = GetScrollLimit().cy;
		break;
	}

	OnScrollBy(szScrollBy);

	CMyScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CThumbnailsView::OnSetFocus(CWnd* pOldWnd)
{
	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		if (m_pages[nPage].bSelected)
			InvalidatePage(nPage);

	CMyScrollView::OnSetFocus(pOldWnd);
}

void CThumbnailsView::OnKillFocus(CWnd* pNewWnd)
{
	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		if (m_pages[nPage].bSelected)
			InvalidatePage(nPage);

	CMyScrollView::OnKillFocus(pNewWnd);
}

void CThumbnailsView::SelectPage(int nPage, bool bSelect)
{
	Page& page = m_pages[nPage];
	if (page.bSelected == bSelect)
		return;

	page.bSelected = bSelect;
	InvalidatePage(nPage);
	UpdateWindow();
}

void CThumbnailsView::ClearSelection()
{
	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		Page& page = m_pages[nPage];
		if (page.bSelected)
		{
			page.bSelected = false;
			InvalidatePage(nPage);
		}
	}

	UpdateWindow();
}

void CThumbnailsView::SetActivePage(int nPage, bool bSetSelection)
{
	if (m_nActivePage != nPage && m_nActivePage != -1)
		InvalidatePage(m_nActivePage);

	m_nActivePage = nPage;
	if (bSetSelection)
	{
		ClearSelection();
		SelectPage(m_nActivePage);
	}

	EnsureVisible(m_nActivePage);
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
	{
		InvalidatePage(m_nCurrentPage);
		EnsureVisible(m_nCurrentPage);
	}

	UpdateWindow();
}

void CThumbnailsView::OnLButtonDown(UINT nFlags, CPoint point)
{
	int nPage = GetPageFromPoint(point);

	if ((nFlags & MK_CONTROL) != 0)
	{
		if (nPage != -1 && m_nActivePage == -1)
		{
			SetActivePage(nPage);
		}
		else if (nPage != -1)
		{
			SetActivePage(nPage, false);
			SelectPage(nPage, !m_pages[nPage].bSelected);
		}
	}
	else if ((nFlags & MK_SHIFT) != 0)
	{
		if (nPage != -1 && m_nActivePage == -1)
		{
			SetActivePage(nPage);
		}
		else if (nPage != -1)
		{
			if (nPage > m_nActivePage)
			{
				int i = m_nActivePage;
				while (i < nPage && m_pages[i].bSelected && m_pages[i + 1].bSelected)
					SelectPage(i++, false);
				while (i <= nPage)
					SelectPage(i++);
			}
			else if (nPage < m_nActivePage)
			{
				int i = m_nActivePage;
				while (i > nPage && m_pages[i].bSelected && m_pages[i - 1].bSelected)
					SelectPage(i--, false);
				while (i >= nPage)
					SelectPage(i--);
			}
			SetActivePage(nPage, false);
		}
	}
	else
	{
		if (nPage != -1)
		{
			SetActivePage(nPage);
			UpdateObservers(PageMsg(THUMBNAIL_CLICKED, nPage));
		}
		else if (m_nActivePage != -1)
			ClearSelection();
	}

	CMyScrollView::OnLButtonDown(nFlags, point);
}

void CThumbnailsView::OnRButtonDown(UINT nFlags, CPoint point)
{
	int nPage = GetPageFromPoint(point);

	if (nPage == -1)
		ClearSelection();
	else if (!m_pages[nPage].bSelected)
	{
		ClearSelection();
		SetActivePage(nPage);
	}

	CRect rect(point, CSize(0, 0));
	ClientToScreen(rect);
	OnShowSettings(0, (LPARAM)(LPRECT) rect);

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
	if (!HasSelection())
	{
		pPopup->EnableMenuItem(ID_THUMBNAILS_PRINT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		pPopup->EnableMenuItem(ID_THUMBNAILS_EXPORT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	int nID = ::TrackPopupMenuEx(pPopup->m_hMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			rcButton.left, rcButton.bottom, m_hWnd, &tpm);

	if (nID == ID_THUMBNAILS_REDUCE)
		ResizeThumbnails(m_nThumbnailSize - 1);
	else if (nID == ID_THUMBNAILS_ENLARGE)
		ResizeThumbnails(m_nThumbnailSize + 1);
	else if (nID == ID_THUMBNAILS_PRINT)
		PrintSelectedPages();
	else if (nID == ID_THUMBNAILS_EXPORT)
		ExportSelectedPages();

	return 0;
}

void CThumbnailsView::ResizeThumbnails(int nThumbnailSize)
{
	nThumbnailSize = max(0, min(CAppSettings::ThumbnailSizes - 1, nThumbnailSize));
	theApp.GetAppSettings()->nThumbnailSize = nThumbnailSize;
	if (nThumbnailSize == m_nThumbnailSize)
		return;

	m_nThumbnailSize = nThumbnailSize;
	m_szThumbnail.cx = CAppSettings::thumbnailWidth[m_nThumbnailSize];
	m_szThumbnail.cy = CAppSettings::thumbnailHeight[m_nThumbnailSize];

	UpdateAllThumbnails();
}

bool CThumbnailsView::OnScroll(UINT nScrollCode, UINT nPos, bool bDoScroll)
{
	if (!CMyScrollView::OnScroll(nScrollCode, nPos, bDoScroll))
		return false;

	int nCode = HIBYTE(nScrollCode);  // Vertical scrollbar
	if (nCode == SB_THUMBTRACK)
		UpdateWindow();

	return true;
}

bool CThumbnailsView::OnScrollBy(CSize szScrollBy, bool bDoScroll)
{
	bool bVScroll = false;
	if (bDoScroll)
		bVScroll = CMyScrollView::OnScrollBy(CSize(0, szScrollBy.cy), false);

	if (!CMyScrollView::OnScrollBy(szScrollBy, bDoScroll))
		return false;

	if (bVScroll)
		UpdateVisiblePages();

	return true;
}

BOOL CThumbnailsView::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	CWnd* pWnd = WindowFromPoint(point);
	if (pWnd != this && !IsChild(pWnd) && IsFromCurrentProcess(pWnd) &&
			pWnd->SendMessage(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(point.x, point.y)) != 0)
		return true;

	if (CMyScrollView::OnMouseWheel(nFlags, zDelta, point))
		return true;

	if ((nFlags & MK_CONTROL) != 0)
	{
		// Resize thumbnails
		if (theApp.GetAppSettings()->bInvertWheelZoom == (zDelta < 0))
			ResizeThumbnails(m_nThumbnailSize - 1);
		else if (zDelta != 0)
			ResizeThumbnails(m_nThumbnailSize + 1);

		return true;
	}

	return false;
}

bool CThumbnailsView::InvalidatePage(int nPage)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);

	CRect rect(CPoint(0, 0), GetViewportSize());
	CPoint ptScroll = GetScrollPosition();
	if (rect.IntersectRect(CRect(rect), m_pages[nPage].rcDisplay - ptScroll))
	{
		InvalidateRect(rect, false);
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

	if (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN)
	{
		// set focus to this view, but don't notify the parent frame
		OnActivateView(true, this, this);
	}

	return nResult;
}

void CThumbnailsView::UpdateLayout(UpdateType updateType)
{
	if (m_bInsideUpdateLayout)
		return;

	m_bInsideUpdateLayout = true;

	// Save page and offset to restore after changes
	int nAnchorPage;
	CPoint ptAnchorOffset;
	CPoint ptScroll = GetScrollPosition();

	if (updateType == TOP)
	{
		int nTopPage = 0;
		while (nTopPage < m_nPageCount - 1 &&
				ptScroll.y >= m_pages[nTopPage].rcDisplay.bottom)
			++nTopPage;

		nAnchorPage = nTopPage;
		ptAnchorOffset = ptScroll - m_pages[nTopPage].rcPage.TopLeft();
	}

	int nThumbnailWidth = 2*nHorzMargin + 2*nFrameWidth + m_szThumbnail.cx;
	int nThumbnailHeight = 2*nVertMargin + 2*nFrameWidth + m_szThumbnail.cy
			+ nNumberSkip + nNumberHeight;

	// Get the full client rect without the scrollbars.
	CSize szClient = ::GetClientSize(this);
	CSize szViewport = szClient;
	bool bHScroll = false, bVScroll = false;

	do
	{
		m_nPagesInRow = max(1, (szViewport.cx - 2*nPadding) / nThumbnailWidth);
		m_nPagesInRow = min(m_nPagesInRow, m_nPageCount);
		int nRowCount = (m_nPageCount - 1) / m_nPagesInRow + 1;

		m_szDisplay = CSize(max(szViewport.cx, m_nPagesInRow*nThumbnailWidth + 2*nPadding),
				max(szViewport.cy, nThumbnailHeight*nRowCount + 2*nPadding));

		double nOffsetX = (szViewport.cx - 2*nPadding - nThumbnailWidth*m_nPagesInRow) / (2.0*m_nPagesInRow);
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
	} while (AdjustViewportSize(m_szDisplay, szViewport, bHScroll, bVScroll));

	CSize szDevPage(szViewport.cx*9/10, szViewport.cy*9/10);
	CSize szDevLine(15, 15);
	SetScrollSizes(m_szDisplay, szDevPage, szDevLine, false);

	if (updateType == TOP)
		ScrollToPosition(m_pages[nAnchorPage].rcPage.TopLeft() + ptAnchorOffset, false);

	m_bInsideUpdateLayout = false;
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

	m_pThread->RemoveAllJobs();
	m_pIdleThread->RemoveAllJobs();

	if (m_bVisible)
	{
		m_pThread->PauseJobs();
		m_pIdleThread->PauseJobs();

		CSize szViewport = GetViewportSize();
		int nTop = GetScrollPosition().y;

		int nTopPage = 0;
		while (nTopPage < m_nPageCount - 1 &&
				nTop >= m_pages[nTopPage].rcDisplay.bottom)
			++nTopPage;

		int nBottomPage = nTopPage + 1;
		while (nBottomPage < m_nPageCount &&
				m_pages[nBottomPage].rcDisplay.top < nTop + szViewport.cy)
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
			!(page.szBitmap.cx <= m_szThumbnail.cx && page.szBitmap.cy == m_szThumbnail.cy ||
			  page.szBitmap.cx == m_szThumbnail.cx && page.szBitmap.cy <= m_szThumbnail.cy))
	{
		pThread->AddJob(nPage, m_nRotate, m_szThumbnail, m_displaySettings);
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
		SetCurrentPage(msg->nPage);
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

		SetCurrentPage(pView->GetCurrentPage());

		m_bInitialized = true;
		UpdateLayout(RECALC);
	}
}

void CThumbnailsView::RecalcPageRects(int nPage)
{
	Page& page = m_pages[nPage];

	if (page.pBitmap != NULL)
	{
		page.szDisplay.cx = m_szThumbnail.cx;
		page.szDisplay.cy = static_cast<int>(1.0*m_szThumbnail.cx*page.szBitmap.cy/page.szBitmap.cx + 0.5);
		if (page.szDisplay.cy > m_szThumbnail.cy)
		{
			page.szDisplay.cy = m_szThumbnail.cy;
			page.szDisplay.cx = min(m_szThumbnail.cx, static_cast<int>(1.0*m_szThumbnail.cy*page.szBitmap.cx/page.szBitmap.cy + 0.5));
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
	ASSERT(nPage >= 0 && nPage < m_nPageCount);

	CSize szViewport = GetViewportSize();
	CPoint ptScroll = GetScrollPosition();
	Page& page = m_pages[nPage];

	int nScrollBy = 0;
	if (page.rcDisplay.top <= ptScroll.y)
		nScrollBy = page.rcDisplay.top - ptScroll.y;
	else if (page.rcDisplay.bottom > ptScroll.y + szViewport.cy)
		nScrollBy = page.rcDisplay.bottom - ptScroll.y - szViewport.cy;

	OnScrollBy(CSize(0, nScrollBy));
}

void CThumbnailsView::SettingsChanged()
{
	CDisplaySettings appSettings = *theApp.GetDisplaySettings();
	appSettings.bScaleColorPnm = false;
	appSettings.bScaleSubpix = false;

	if (m_displaySettings != appSettings)
	{
		m_displaySettings = appSettings;

		for (int nPage = 0; nPage < m_nPageCount; ++nPage)
			m_pages[nPage].DeleteBitmap();

		UpdateAllThumbnails();
	}
}

void CThumbnailsView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CMyScrollView::OnShowWindow(bShow, nStatus);

	if (bShow && m_nCurrentPage != -1)
		EnsureVisible(m_nCurrentPage);

	m_bVisible = bShow && GetParent()->IsWindowVisible();
	UpdateVisiblePages();
}

LRESULT CThumbnailsView::OnShowParent(WPARAM wParam, LPARAM lParam)
{
	m_bVisible = !!wParam;
	UpdateVisiblePages();

	return 0;
}

void CThumbnailsView::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	CMyScrollView::OnMenuSelect(nItemID, nFlags, hSysMenu);

	GetTopLevelFrame()->SendMessage(WM_MENUSELECT, MAKEWPARAM(nItemID, nFlags),
			(LPARAM) hSysMenu);
}

void CThumbnailsView::OnEnterIdle(UINT nWhy, CWnd* pWho)
{
	CMyScrollView::OnEnterIdle(nWhy, pWho);

	GetTopLevelFrame()->SendMessage(WM_ENTERIDLE, nWhy, (LPARAM) pWho->GetSafeHwnd());
}

bool CThumbnailsView::HasSelection() const
{
	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		if (m_pages[nPage].bSelected)
			return true;

	return false;
}

void CThumbnailsView::PrintSelectedPages()
{
	set<int> selection;
	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		if (m_pages[nPage].bSelected)
			selection.insert(nPage);

	UpdateObservers(PageRangeMsg(PRINT_PAGES, selection));
}

void CThumbnailsView::ExportSelectedPages()
{
	set<int> selection;
	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		if (m_pages[nPage].bSelected)
			selection.insert(nPage);

	UpdateObservers(PageRangeMsg(EXPORT_PAGES, selection));
}
