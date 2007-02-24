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
#include "MySplitterWnd.h"
#include "NavPane.h"
#include "Drawing.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMySplitterWnd

IMPLEMENT_DYNAMIC(CMySplitterWnd, CSplitterWnd)
CMySplitterWnd::CMySplitterWnd()
	: m_bAllowTracking(true), m_bNavHidden(false)
{
	m_cxOrigSplitter = m_cxSplitter;
	m_cxOrigSplitterGap = m_cxSplitterGap;
	m_cyOrigSplitter = m_cySplitter;
	m_cyOrigSplitterGap = m_cySplitterGap;

	m_nNavPaneWidth = theApp.GetAppSettings()->nNavPaneWidth;
	m_bCollapsed = theApp.GetAppSettings()->bNavPaneCollapsed;
	HideNavPane(theApp.GetAppSettings()->bNavPaneHidden);
}

CMySplitterWnd::~CMySplitterWnd()
{
}


BEGIN_MESSAGE_MAP(CMySplitterWnd, CSplitterWnd)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CMySplitterWnd message handlers

void CMySplitterWnd::StopTracking(BOOL bAccept)
{
	CSplitterWnd::StopTracking(bAccept);

	if (bAccept)
	{
		int cxMin, cxCur;
		GetColumnInfo(0, cxCur, cxMin);
		if (cxCur > CNavPaneWnd::s_nMinExpandedWidth)
		{
			m_nNavPaneWidth = cxCur;
			theApp.GetAppSettings()->nNavPaneWidth = m_nNavPaneWidth;

			m_bCollapsed = false;
			theApp.GetAppSettings()->bNavPaneCollapsed = m_bCollapsed;
		}
		else
		{
			m_bCollapsed = true;
			theApp.GetAppSettings()->bNavPaneCollapsed = m_bCollapsed;
		}

		UpdateNavPaneWidth(cxCur);
	}
}

int CMySplitterWnd::HitTest(CPoint pt) const
{
	if (!m_bAllowTracking)
		return 0;

	return CSplitterWnd::HitTest(pt);
}

void CMySplitterWnd::HideNavPane(bool bHide)
{
	if (bHide)
	{
		m_cxSplitter = 0;
		m_cxSplitterGap = 1;
		m_cySplitter = 0;
		m_cySplitterGap = 1;
	}
	else
	{
		m_cxSplitter = m_cxOrigSplitter;
		m_cxSplitterGap = m_cxOrigSplitterGap;
		m_cySplitter = m_cyOrigSplitter;
		m_cySplitterGap = m_cyOrigSplitterGap;
	}

	m_bAllowTracking = !bHide;
	m_bNavHidden = bHide;

	if (::IsWindow(m_hWnd))
		UpdateNavPane();
}

void CMySplitterWnd::TrackRowSize(int y, int row)
{
	if (GetPane(row, 0)->IsKindOf(RUNTIME_CLASS(CSplitterWnd)))
		y -= 2;

	CSplitterWnd::TrackRowSize(y, row);
}

void CMySplitterWnd::TrackColumnSize(int x, int col)
{
	if (GetPane(0, col)->IsKindOf(RUNTIME_CLASS(CSplitterWnd)))
		x -= 2;

	CSplitterWnd::TrackColumnSize(x, col);
}

void CMySplitterWnd::DrawAllSplitBars(CDC* pDC, int cxInside, int cyInside)
{
	ASSERT_VALID(this);

	// draw column split bar
	CRect rect;
	GetClientRect(rect);
	rect.left += m_cxBorder + m_pColInfo[0].nCurSize;
	rect.right = rect.left + m_cxSplitter;
	if (!m_bNavHidden && rect.left <= cxInside)
		OnDrawSplitter(pDC, splitBar, rect);

	// draw pane borders
	GetClientRect(rect);
	int x = rect.left;
	for (int nCol = 0; nCol < m_nCols; nCol++)
	{
		int cx = m_pColInfo[nCol].nCurSize + 2*m_cxBorder;
		if (nCol == m_nCols - 1 && m_bHasVScroll)
			cx += afxData.cxVScroll - CX_BORDER;
		int y = rect.top;

		int cy = m_pRowInfo[0].nCurSize + 2*m_cyBorder;
		if (m_bHasHScroll)
			cy += afxData.cyHScroll - CX_BORDER;

		if (nCol == 0)
		{
			if (!m_bNavHidden)
				DrawLeftPaneBorder(pDC, CRect(x, y, x + cx, y + cy));
			else
				OnDrawSplitter(pDC, splitBar, CRect(x, y, x + 1, y + cy));
		}
		else
			OnDrawSplitter(pDC, splitBorder, CRect(x, y, x + cx, y + cy));

		x += cx + m_cxSplitterGap - 2*m_cxBorder;
	}
}

void CMySplitterWnd::DrawLeftPaneBorder(CDC* pDC, const CRect& rcArg)
{
	if (pDC == NULL)
	{
		RedrawWindow(rcArg, NULL, RDW_INVALIDATE|RDW_NOCHILDREN);
		return;
	}
	ASSERT_VALID(pDC);

	COLORREF clrBtnface = ::GetSysColor(COLOR_BTNFACE);
	COLORREF clrTabBg = ChangeBrightness(clrBtnface, 0.85);

	CPoint pointsTop[] = { rcArg.TopLeft(), CPoint(rcArg.right - 1, rcArg.top) };
	CPoint pointsTop2[] = { CPoint(rcArg.left + 1, rcArg.top + 1), CPoint(rcArg.right - 2, rcArg.top + 1) };
	CPoint pointsTop3[] = { CPoint(rcArg.left + 1, rcArg.top + 1), CPoint(min(rcArg.left + 3 + CNavPaneWnd::s_nTabsWidth, rcArg.right - 2), rcArg.top + 1) };
	CPoint pointsBottom[] = { CPoint(rcArg.left, rcArg.bottom - 1), CPoint(rcArg.right - 1, rcArg.bottom - 1) };
	CPoint pointsBottom2[] = { CPoint(rcArg.left, rcArg.bottom - 2), CPoint(rcArg.right - 2, rcArg.bottom - 2) };
	CPoint pointsLeft[] = { CPoint(rcArg.left, rcArg.top + 1), CPoint(rcArg.left, rcArg.bottom - 2) };
	CPoint pointsLeft2[] = { CPoint(rcArg.left + 1, rcArg.top + 1), CPoint(rcArg.left + 1, rcArg.bottom - 2) };
	CPoint pointsRight[] = { CPoint(rcArg.right - 1, rcArg.top), CPoint(rcArg.right - 1, rcArg.bottom - 1) };
	CPoint pointsRight2[] = { CPoint(rcArg.right - 2, rcArg.top), CPoint(rcArg.right - 2, rcArg.bottom - 1) };

	CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNSHADOW));
	CPen* pOldPen = pDC->SelectObject(&pen);
	pDC->Polyline((LPPOINT)pointsTop, 2);
	pDC->Polyline((LPPOINT)pointsBottom2, 2);
	pDC->Polyline((LPPOINT)pointsRight2, 2);
	pDC->SelectObject(pOldPen);

	CPen pen3(PS_SOLID, 1, clrBtnface);
	pOldPen = pDC->SelectObject(&pen3);
	pDC->Polyline((LPPOINT)pointsRight, 2);
	pDC->Polyline((LPPOINT)pointsBottom, 2);
	pDC->Polyline((LPPOINT)pointsTop2, 2);
	pDC->SelectObject(pOldPen);

	CPen pen2(PS_SOLID, 1, clrTabBg);
	pOldPen = pDC->SelectObject(&pen2);
	pDC->Polyline((LPPOINT)pointsLeft, 2);
	pDC->Polyline((LPPOINT)pointsLeft2, 2);
	pDC->Polyline((LPPOINT)pointsTop3, 2);
	pDC->SelectObject(pOldPen);
}

void CMySplitterWnd::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	// From MFC: CSplitterWnd::OnMouseMove

	if (GetCapture() != this)
		StopTracking(FALSE);

	if (m_bTracking)
	{
		// move tracker to current cursor position
		if (m_htTrack == 201 /*hSplitterBar1*/)
		{
			point.Offset(m_ptTrackOffset); // pt is the upper right of hit detect

			// limit the point to the valid split range
			if (point.y < m_rectLimit.top)
				point.y = m_rectLimit.top;
			else if (point.y > m_rectLimit.bottom)
				point.y = m_rectLimit.bottom;
			if (point.x < m_rectLimit.left)
				point.x = m_rectLimit.left;
			else if (point.x > m_rectLimit.right)
				point.x = m_rectLimit.right;

			CPoint ptSize(point);
			ClientToScreen(&ptSize);
			GetNavPane()->ScreenToClient(&ptSize);

			if (ptSize.x < CNavPaneWnd::s_nMinExpandedWidth + CX_BORDER)
			{
				ptSize.x = CNavPaneWnd::s_nTabsWidth + CX_BORDER;
				GetNavPane()->ClientToScreen(&ptSize);
				ScreenToClient(&ptSize);

				point.x = ptSize.x;
			}

			if (m_rectTracker.left != point.x)
			{
				OnInvertTracker(m_rectTracker);
				m_rectTracker.OffsetRect(point.x - m_rectTracker.left, 0);
				OnInvertTracker(m_rectTracker);
			}
		}
	}
	else
	{
		// simply hit-test and set appropriate cursor
		int ht = HitTest(point);
		SetSplitCursor(ht);
	}
}

CNavPaneWnd* CMySplitterWnd::GetNavPane()
{
	return static_cast<CNavPaneWnd*>(GetPane(0, 0));
}

void CMySplitterWnd::UpdateNavPane()
{
	UpdateNavPaneWidth(m_bCollapsed ? 0 : m_nNavPaneWidth);
}

void CMySplitterWnd::UpdateNavPaneWidth(int nWidth)
{
	if (m_bNavHidden)
		nWidth = 0;
	else
		nWidth = max(nWidth, CNavPaneWnd::s_nTabsWidth);

	SetColumnInfo(0, nWidth, 0);
	RecalcLayout();
}

void CMySplitterWnd::CollapseNavPane(bool bCollapse)
{
	m_bCollapsed = bCollapse;
	theApp.GetAppSettings()->bNavPaneCollapsed = m_bCollapsed;

	UpdateNavPane();
}
