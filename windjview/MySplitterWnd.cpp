//	WinDjView
//	Copyright (C) 2004-2005 Andrew Zhezherun
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
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  http://www.gnu.org/copyleft/gpl.html

// $Id$

#include "stdafx.h"
#include "MySplitterWnd.h"

#ifndef ELIBRA_READER
#include "resource.h"
#else
#include "Elibra.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMySplitterWnd

IMPLEMENT_DYNAMIC(CMySplitterWnd, CSplitterWnd)
CMySplitterWnd::CMySplitterWnd()
{
	m_bAllowTracking = true;

	m_cxOrigSplitter = m_cxSplitter;
	m_cxOrigSplitterGap = m_cxSplitterGap;
	m_cyOrigSplitter = m_cySplitter;
	m_cyOrigSplitterGap = m_cySplitterGap;
}

CMySplitterWnd::~CMySplitterWnd()
{
}


BEGIN_MESSAGE_MAP(CMySplitterWnd, CSplitterWnd)
END_MESSAGE_MAP()


// CMySplitterWnd message handlers

void CMySplitterWnd::StopTracking(BOOL bAccept)
{
	CSplitterWnd::StopTracking(bAccept);

	if (bAccept)
		GetParentFrame()->SendMessage(ID_STOP_TRACKING);
}

int CMySplitterWnd::HitTest(CPoint pt) const
{
	if (!m_bAllowTracking)
		return 0;

	return CSplitterWnd::HitTest(pt);
}

void CMySplitterWnd::HideSplitter(bool bHide)
{
	if (bHide)
	{
		m_cxSplitter = 0;
		m_cxSplitterGap = 2;
		m_cySplitter = 0;
		m_cySplitterGap = 2;
	}
	else
	{
		m_cxSplitter = m_cxOrigSplitter;
		m_cxSplitterGap = m_cxOrigSplitterGap;
		m_cySplitter = m_cyOrigSplitter;
		m_cySplitterGap = m_cyOrigSplitterGap;
	}

	RecalcLayout();
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
	rect.left += m_cxBorder;
	rect.left += m_pColInfo[0].nCurSize + m_cxBorderShare;
	rect.right = rect.left + m_cxSplitter;
	if (rect.left <= cxInside)
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
		OnDrawSplitter(pDC, nCol == 0 ? splitBar : splitBorder,
			CRect(x, y, x + cx, y + cy));

		x += cx + m_cxSplitterGap - 2*m_cxBorder;
	}
}
