//	WinDjView
//	Copyright (C) 2004 Andrew Zhezherun
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
#include "WinDjView.h"

#include "DjVuDoc.h"
#include "DjVuView.h"
#include "AppSettings.h"

#include "MainFrm.h"
#include "PrintDlg.h"
#include "Drawing.h"
#include "ProgressDlg.h"
#include "ZoomDlg.h"

#include "RenderThread.h"

#include <../src/mfc/afximpl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDjVuView

HCURSOR CDjVuView::hCursorHand = NULL;
HCURSOR CDjVuView::hCursorDrag = NULL;

const int c_nPageGap = 8;
const int c_nPageShadow = 4;

IMPLEMENT_DYNCREATE(CDjVuView, CScrollView)

BEGIN_MESSAGE_MAP(CDjVuView, CScrollView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_VIEW_NEXTPAGE, OnViewNextpage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEXTPAGE, OnUpdateViewNextpage)
	ON_COMMAND(ID_VIEW_PREVIOUSPAGE, OnViewPreviouspage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PREVIOUSPAGE, OnUpdateViewPreviouspage)
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_COMMAND_RANGE(ID_ZOOM_50, ID_ZOOM_CUSTOM, OnViewZoom)
	ON_UPDATE_COMMAND_UI_RANGE(ID_ZOOM_50, ID_ZOOM_CUSTOM, OnUpdateViewZoom)
	ON_COMMAND(ID_ROTATE_LEFT, OnRotateLeft)
	ON_COMMAND(ID_ROTATE_RIGHT, OnRotateRight)
	ON_COMMAND(ID_ROTATE_180, OnRotate180)
	ON_COMMAND(ID_VIEW_FIRSTPAGE, OnViewFirstpage)
	ON_COMMAND(ID_VIEW_LASTPAGE, OnViewLastpage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LASTPAGE, OnUpdateViewNextpage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIRSTPAGE, OnUpdateViewPreviouspage)
	ON_WM_SETFOCUS()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PAGE_INFORMATION, OnPageInformation)
	ON_COMMAND_RANGE(ID_LAYOUT_SINGLEPAGE, ID_LAYOUT_CONTINUOUS, OnViewLayout)
	ON_UPDATE_COMMAND_UI_RANGE(ID_LAYOUT_SINGLEPAGE, ID_LAYOUT_CONTINUOUS, OnUpdateViewLayout)
	ON_MESSAGE(WM_RENDER_FINISHED, OnRenderFinished)
	ON_MESSAGE(WM_PAGE_DECODED, OnPageDecoded)
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// CDjVuView construction/destruction

CDjVuView::CDjVuView()
	: m_nPage(-1), m_nPageCount(0), m_nZoomType(ZoomPercent), m_fZoom(100.0),
	  m_nLayout(SinglePage), m_nRotate(0), m_bDragging(false),
	  m_pRenderThread(NULL)
{
}

CDjVuView::~CDjVuView()
{
}

BOOL CDjVuView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CScrollView::PreCreateWindow(cs);
}

// CDjVuView drawing

void CDjVuView::OnDraw(CDC* pDC)
{
	if (pDC->IsPrinting())
	{
		// TODO: Print preview
		Page& page = m_pages[m_nPage];

		if (page.pBitmap != NULL)
		{
			int nOffsetX = pDC->GetDeviceCaps(PHYSICALOFFSETX);
			int nOffsetY = pDC->GetDeviceCaps(PHYSICALOFFSETY);

			pDC->SetViewportOrg(-nOffsetX, -nOffsetY);

			CSize szPage = page.GetSize(m_nRotate);
			page.pBitmap->Draw(pDC, CPoint(0, 0), szPage);
		}
		return;
	}

	if (m_nLayout == SinglePage)
	{
        DrawPage(pDC, m_nPage);
	}
	else if (m_nLayout == Continuous)
	{
		CRect rcClip;
		pDC->GetClipBox(rcClip);

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
}

void CDjVuView::DrawPage(CDC* pDC, int nPage)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);
	Page& page = m_pages[nPage];

	if (page.pBitmap != NULL)
	{
		if (page.pBitmap->GetSize() == page.szDisplay)
		{
			DrawOffscreen(pDC, nPage);
		}
		else
		{
			DrawStretch(pDC, nPage);
		}
	}
	else
	{
		DrawWhite(pDC, nPage);
	}
}

void CDjVuView::DrawWhite(CDC* pDC, int nPage)
{
	Page& page = m_pages[nPage];

	CRect rcClip;
	pDC->GetClipBox(rcClip);

	if (m_nLayout == Continuous)
	{
		rcClip.top = max(rcClip.top, page.rcDisplay.top);
		rcClip.bottom = min(rcClip.bottom, page.rcDisplay.bottom);
	}

	if (!rcClip.IsRectEmpty())
	{
		COLORREF clrWindow = ::GetSysColor(COLOR_WINDOW);
		pDC->FillSolidRect(rcClip, clrWindow);
	}

	if (page.szDisplay.cx > 0 && page.szDisplay.cy > 0)
	{
		CRect rcBorder(page.ptOffset.x - 1, page.ptOffset.y - 1,
			page.ptOffset.x + page.szDisplay.cx,
			page.ptOffset.y + page.szDisplay.cy);

		CPoint points[] = { rcBorder.TopLeft(), CPoint(rcBorder.right, rcBorder.top),
			rcBorder.BottomRight(), CPoint(rcBorder.left, rcBorder.bottom),
			rcBorder.TopLeft() };

		CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_3DDKSHADOW));
		CPen* pOldPen = pDC->SelectObject(&pen);
		pDC->Polyline((LPPOINT)points, 5);
		pDC->SelectObject(pOldPen);

		CRect rcShadow(page.ptOffset.x + c_nPageShadow - 1, page.ptOffset.y + page.szDisplay.cy + 1,
			page.ptOffset.x + page.szDisplay.cx + c_nPageShadow, page.ptOffset.y + page.szDisplay.cy + c_nPageShadow);
		pDC->FillSolidRect(rcShadow, ::GetSysColor(COLOR_BTNSHADOW));

		rcShadow = CRect(page.ptOffset.x + page.szDisplay.cx + 1, page.ptOffset.y + c_nPageShadow - 1,
			page.ptOffset.x + page.szDisplay.cx + c_nPageShadow, page.ptOffset.y + page.szDisplay.cy + 1);
		pDC->FillSolidRect(rcShadow, ::GetSysColor(COLOR_BTNSHADOW));

//		CString strPage;
//		strPage.Format(_T("%d"), nPage + 1);
//		pDC->DrawText(strPage, rcBorder, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
}

void CDjVuView::DrawOffscreen(CDC* pDC, int nPage)
{
	Page& page = m_pages[nPage];
	ASSERT(page.pBitmap != NULL && page.pBitmap->m_hObject != NULL);

	CPoint ptOffset = GetScrollPosition();

	CRect rcClip;
	pDC->GetClipBox(rcClip);

	CPoint ptPartOffset(max(rcClip.left - page.ptOffset.x, 0),
		max(rcClip.top - page.ptOffset.y, 0));

	CSize szPageClip = page.szDisplay - ptPartOffset;
	CSize szPart(min(rcClip.Width(), szPageClip.cx), min(rcClip.Height(), szPageClip.cy));

	if (szPart.cx > 0 && szPart.cy > 0)
	{
		CRect rcPart(ptPartOffset, szPart);
		page.pBitmap->DrawDC(pDC, page.ptOffset + ptPartOffset, rcPart);
	}

	CRect rcBorder(page.ptOffset.x - 1, page.ptOffset.y - 1,
		page.ptOffset.x + page.szDisplay.cx,
		page.ptOffset.y + page.szDisplay.cy);

	CPoint points[] = { rcBorder.TopLeft(), CPoint(rcBorder.right, rcBorder.top),
		rcBorder.BottomRight(), CPoint(rcBorder.left, rcBorder.bottom),
		rcBorder.TopLeft() };

	CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_3DDKSHADOW));
	CPen* pOldPen = pDC->SelectObject(&pen);
	pDC->Polyline((LPPOINT)points, 5);
	pDC->SelectObject(pOldPen);

	if (m_nLayout == Continuous)
	{
		rcClip.top = max(rcClip.top, page.rcDisplay.top);
		rcClip.bottom = min(rcClip.bottom, page.rcDisplay.bottom);
	}

	if (!rcClip.IsRectEmpty())
	{
		int nSaveDC = pDC->SaveDC();
		rcBorder.InflateRect(0, 0, 1, 1);
		pDC->ExcludeClipRect(rcBorder);

		COLORREF clrWindow = ::GetSysColor(COLOR_WINDOW);
		pDC->FillSolidRect(rcClip, clrWindow);

		pDC->RestoreDC(nSaveDC);
	}

	CRect rcShadow(page.ptOffset.x + c_nPageShadow - 1, page.ptOffset.y + page.szDisplay.cy + 1,
		page.ptOffset.x + page.szDisplay.cx + c_nPageShadow, page.ptOffset.y + page.szDisplay.cy + c_nPageShadow);
	pDC->FillSolidRect(rcShadow, ::GetSysColor(COLOR_BTNSHADOW));

	rcShadow = CRect(page.ptOffset.x + page.szDisplay.cx + 1, page.ptOffset.y + c_nPageShadow - 1,
		page.ptOffset.x + page.szDisplay.cx + c_nPageShadow, page.ptOffset.y + page.szDisplay.cy + 1);
	pDC->FillSolidRect(rcShadow, ::GetSysColor(COLOR_BTNSHADOW));
}

void CDjVuView::DrawStretch(CDC* pDC, int nPage)
{
	Page& page = m_pages[nPage];
	ASSERT(page.pBitmap != NULL && page.pBitmap->m_hObject != NULL);

	CRect rcClip;
	pDC->GetClipBox(rcClip);

	page.pBitmap->Draw(pDC, page.ptOffset, page.szDisplay);

	CRect rcBorder(page.ptOffset.x - 1, page.ptOffset.y - 1,
		page.ptOffset.x + page.szDisplay.cx,
		page.ptOffset.y + page.szDisplay.cy);

	CPoint points[] = { rcBorder.TopLeft(), CPoint(rcBorder.right, rcBorder.top),
		rcBorder.BottomRight(), CPoint(rcBorder.left, rcBorder.bottom),
		rcBorder.TopLeft() };

	CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_3DDKSHADOW));
	CPen* pOldPen = pDC->SelectObject(&pen);
	pDC->Polyline((LPPOINT)points, 5);
	pDC->SelectObject(pOldPen);

	if (m_nLayout == Continuous)
	{
		rcClip.top = max(rcClip.top, page.rcDisplay.top);
		rcClip.bottom = min(rcClip.bottom, page.rcDisplay.bottom);
	}

	if (!rcClip.IsRectEmpty())
	{
		int nSaveDC = pDC->SaveDC();
		rcBorder.InflateRect(0, 0, 1, 1);
		pDC->ExcludeClipRect(rcBorder);

		COLORREF clrWindow = ::GetSysColor(COLOR_WINDOW);
		pDC->FillSolidRect(rcClip, clrWindow);

		pDC->RestoreDC(nSaveDC);
	}

	CRect rcShadow(page.ptOffset.x + c_nPageShadow - 1, page.ptOffset.y + page.szDisplay.cy + 1,
		page.ptOffset.x + page.szDisplay.cx + c_nPageShadow, page.ptOffset.y + page.szDisplay.cy + c_nPageShadow);
	pDC->FillSolidRect(rcShadow, ::GetSysColor(COLOR_BTNSHADOW));

	rcShadow = CRect(page.ptOffset.x + page.szDisplay.cx + 1, page.ptOffset.y + c_nPageShadow - 1,
		page.ptOffset.x + page.szDisplay.cx + c_nPageShadow, page.ptOffset.y + page.szDisplay.cy + 1);
	pDC->FillSolidRect(rcShadow, ::GetSysColor(COLOR_BTNSHADOW));
}


// CDjVuView printing

BOOL CDjVuView::OnPreparePrinting(CPrintInfo* pInfo)
{
	pInfo->SetMaxPage(-1);
	return DoPreparePrinting(pInfo);
}

// CDjVuView diagnostics

#ifdef _DEBUG
void CDjVuView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CDjVuView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CDjVuDoc* CDjVuView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDjVuDoc)));
	return (CDjVuDoc*)m_pDocument;
}
#endif //_DEBUG


// CDjVuView message handlers

void CDjVuView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	m_pRenderThread = new CRenderThread(GetDocument(), this);

	m_nZoomType = CAppSettings::nDefaultZoomType;
	m_fZoom = CAppSettings::fDefaultZoom;
	if (m_nZoomType == ZoomPercent)
		m_fZoom = 100.0;

	m_nLayout = CAppSettings::nDefaultLayout;

	m_nPageCount = GetDocument()->GetPageCount();
	m_pages.resize(m_nPageCount);
	m_nPage = 0;

	UpdateView(RECALC);
	RenderPage(0);
}

BOOL CDjVuView::OnEraseBkgnd(CDC* pDC)
{
	CBrush br(GetSysColor(COLOR_WINDOW));
	return true;
}

void CDjVuView::RenderPage(int nPage)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);
	Page& page = m_pages[nPage];

	if (m_nLayout == SinglePage)
	{
		DeleteBitmaps(nPage);

		m_nPage = nPage;

		if (!page.bSizeLoaded)
		{
			GetDocument()->GetPageInfo(nPage, page.szPage, page.nDPI);
			page.bSizeLoaded = true;
		}

		UpdateView();
		ScrollToPosition(CPoint(GetScrollPos(SB_HORZ), 0));

		GetMainFrame()->UpdatePageCombo(m_nPage);
	}
	else
	{
		UpdatePageSizes(page.ptOffset.y);

		int nUpdatedPos = page.ptOffset.y - 1;
		ScrollToPosition(CPoint(GetScrollPos(SB_HORZ), nUpdatedPos));
		UpdateVisiblePages();

		GetMainFrame()->UpdatePageCombo(GetTopPage());
	}

	Invalidate();
	UpdateWindow();
}

void CDjVuView::DeleteBitmaps(int nKeep)
{
	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		if (nPage != nKeep)
			m_pages[nPage].DeleteBitmap();
	}
}

void CDjVuView::UpdateView(UpdateType updateType)
{
	CRect rcClient;

	if (m_nLayout == SinglePage)
	{
		Page& page = m_pages[m_nPage];

		// Update size 2 times to allow for scrollbars
		for (int i = 0; i < 2; ++i)
		{
			UpdatePageSize();

			GetClientRect(rcClient);
			CSize szDevPage(rcClient.Width()*3/4, rcClient.Height()*3/4);
			CSize szDevLine(15, 15);

			SetScrollSizes(MM_TEXT, m_szDisplay, szDevPage, szDevLine);
		}

		if (page.pBitmap == NULL || page.szDisplay != page.pBitmap->GetSize())
		{
			m_pRenderThread->AddJob(m_nPage, m_nRotate,
				CRect(CPoint(0, 0), page.szDisplay),
				CRect(CPoint(0, 0), page.szDisplay));

			Invalidate();
		}
	}
	else if (m_nLayout == Continuous)
	{
		GetClientRect(rcClient);

		int nHorzPos = GetScrollPos(SB_HORZ);

		// Save page and offset to restore after changes
		int nAnchorPage;
		int nAnchorOffset;

		int nTop = GetScrollPos(SB_VERT);
		int nTopPage = GetTopPage();

		if (updateType == TOP)
		{
			nAnchorPage = nTopPage;
			nAnchorOffset = nTop - m_pages[nTopPage].ptOffset.y;
		}
		else if (updateType == BOTTOM)
		{
			int nBottom = nTop + rcClient.Height();

			int nPage = GetTopPage();
			while (nPage < m_nPageCount - 1 &&
				nBottom > m_pages[nPage].rcDisplay.bottom)
				++nPage;

			nAnchorPage = nPage;
			nAnchorOffset = nBottom - m_pages[nPage].ptOffset.y;
		}

		for (int i = 0; i < 2; ++i)
		{
			m_szDisplay = CSize(0, 0);
			m_pages[0].ptOffset = CPoint(0, 1);

			for (int nPage = 0; nPage < m_nPageCount; ++nPage)
			{
				Page& page = m_pages[nPage];
				CSize szPage = page.GetSize(m_nRotate);

				page.szDisplay = CalcPageSize(szPage, page.nDPI);
				if (nPage < m_nPageCount - 1)
				{
					m_pages[nPage + 1].ptOffset = CPoint(0,
							page.ptOffset.y + page.szDisplay.cy + c_nPageGap);
				}

				page.rcDisplay = CRect(page.ptOffset, page.szDisplay);
				page.rcDisplay.InflateRect(1, 1, c_nPageShadow, c_nPageGap - 1);

				if (page.rcDisplay.Width() > m_szDisplay.cx)
					m_szDisplay.cx = page.rcDisplay.Width();
			}

			m_szDisplay.cy = m_pages.back().ptOffset.y + 
				m_pages.back().szDisplay.cy + c_nPageShadow + 1;

			GetClientRect(rcClient);
			CSize szDevPage(rcClient.Width()*3/4, rcClient.Height()*3/4);
			CSize szDevLine(15, 15);

			SetScrollSizesNoRepaint(m_szDisplay, szDevPage, szDevLine);
		}

		GetClientRect(rcClient);
		if (m_szDisplay.cx < rcClient.Width())
			m_szDisplay.cx = rcClient.Width();

		for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		{
			Page& page = m_pages[nPage];

			if (page.rcDisplay.Width() < m_szDisplay.cx)
				page.ptOffset.x = (m_szDisplay.cx - page.rcDisplay.Width())/2;
			page.ptOffset.Offset(1, 0);

			page.rcDisplay.OffsetRect(page.ptOffset.x, 0);
		}

		if (updateType == TOP)
		{
			ScrollToPositionNoRepaint(CPoint(nHorzPos,
				m_pages[nAnchorPage].ptOffset.y + nAnchorOffset));
		}
		else if (updateType == BOTTOM)
		{
			ScrollToPositionNoRepaint(CPoint(nHorzPos,
				m_pages[nAnchorPage].ptOffset.y + nAnchorOffset - rcClient.Height()));
		}

		if (updateType != RECALC)
		{
			UpdateVisiblePages();
		}
	}
}

void CDjVuView::UpdateVisiblePages()
{
	CRect rcClient;
	GetClientRect(rcClient);

	int nTop = GetScrollPos(SB_VERT);

	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		Page& page = m_pages[nPage];

		if (page.rcDisplay.top < nTop + rcClient.Height() &&
			page.rcDisplay.bottom > nTop)
		{
			if (page.pBitmap == NULL || page.szDisplay != page.pBitmap->GetSize())
			{
				m_pRenderThread->AddJob(nPage, m_nRotate,
					CRect(CPoint(0, 0), page.szDisplay),
					CRect(CPoint(0, 0), page.szDisplay));

				InvalidatePage(nPage);
			}
		}
		else
		{
			page.DeleteBitmap();
			m_pRenderThread->RemoveFromQueue(nPage);
		}
	}
}

CSize CDjVuView::CalcPageSize(const CSize& szPage, int nDPI)
{
	CSize szDisplay(0, 0);
	if (szPage.cx <= 0 || szPage.cy <= 0)
		return szDisplay;

	CRect rcClient;
	GetClientRect(rcClient);

	switch (m_nZoomType)
	{
	case ZoomFitWidth:
		szDisplay = rcClient.Size() - CSize(c_nPageShadow + 1, 0);
		szDisplay.cy = szDisplay.cx * szPage.cy / szPage.cx;
		break;

	case ZoomFitHeight:
		szDisplay = rcClient.Size() - CSize(0, c_nPageShadow + 1);
		szDisplay.cx = szDisplay.cy * szPage.cx / szPage.cy;
		break;

	case ZoomFitPage:
		szDisplay = rcClient.Size() - CSize(0, c_nPageShadow + 1);
		szDisplay.cx = szDisplay.cy * szPage.cx / szPage.cy;

		if (szDisplay.cx > rcClient.Width() - c_nPageShadow - 1)
		{
			szDisplay.cx = rcClient.Width() - c_nPageShadow - 1;
			szDisplay.cy = szDisplay.cx * szPage.cy / szPage.cx;
		}
		break;

	case ZoomActualSize:
		szDisplay = szPage;
		break;

	case ZoomStretch:
		szDisplay = rcClient.Size();
		break;

	case ZoomPercent:
	default:
		CDC dcScreen;
		dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

		int nLogPixelsX = dcScreen.GetDeviceCaps(LOGPIXELSX);
		int nLogPixelsY = dcScreen.GetDeviceCaps(LOGPIXELSX);

		szDisplay.cx = static_cast<int>(szPage.cx*nLogPixelsX*m_fZoom*0.01/nDPI);
		szDisplay.cy = static_cast<int>(szPage.cy*nLogPixelsY*m_fZoom*0.01/nDPI);
	}

	return szDisplay;
}

void CDjVuView::UpdatePageSize()
{
	Page& page = m_pages[m_nPage];
	CSize szPage = page.GetSize(m_nRotate);

	page.ptOffset = CPoint(0, 0);
	m_szDisplay = page.szDisplay = CalcPageSize(szPage, page.nDPI);

	page.rcDisplay = CRect(page.ptOffset, page.szDisplay);
	page.rcDisplay.InflateRect(1, 1, c_nPageShadow, c_nPageShadow);

	CRect rcClient;
	GetClientRect(rcClient);

	if (page.rcDisplay.Width() < rcClient.Width())
	{
		m_szDisplay.cx = rcClient.Width();
		page.ptOffset.x = (m_szDisplay.cx - page.rcDisplay.Width())/2;
	}

	if (page.rcDisplay.Height() < rcClient.Height())
	{
		m_szDisplay.cy = rcClient.Height();
		page.ptOffset.y = (m_szDisplay.cy - page.rcDisplay.Height())/2;
	}

	page.ptOffset.Offset(1, 1);
	page.rcDisplay.OffsetRect(page.ptOffset);
}

void CDjVuView::OnViewNextpage()
{
	int nPage = 0;
	if (m_nLayout == SinglePage)
		nPage = m_nPage;
	else if (m_nLayout == Continuous)
		nPage = GetTopPage();

	++nPage;
	if (nPage < m_nPageCount - 1)
		RenderPage(nPage);
	else
		OnViewLastpage();
}

void CDjVuView::OnUpdateViewNextpage(CCmdUI* pCmdUI)
{
	if (m_nLayout == SinglePage)
		pCmdUI->Enable(m_nPage < m_nPageCount - 1);
	else if (m_nLayout == Continuous)
		pCmdUI->Enable(GetScrollPos(SB_VERT) < GetScrollLimit(SB_VERT));
}

void CDjVuView::OnViewPreviouspage()
{
	int nPage = 0;
	if (m_nLayout == SinglePage)
		nPage = m_nPage;
	else if (m_nLayout == Continuous)
		nPage = GetTopPage();

	--nPage;
	if (nPage < 0)
		nPage = 0;

	RenderPage(nPage);
}

void CDjVuView::OnUpdateViewPreviouspage(CCmdUI* pCmdUI)
{
	if (m_nLayout == SinglePage)
		pCmdUI->Enable(m_nPage > 0);
	else if (m_nLayout == Continuous)
		pCmdUI->Enable(GetScrollPos(SB_VERT) > 0);
}

void CDjVuView::OnSize(UINT nType, int cx, int cy)
{
	if (m_nPage != -1)
	{
		if (m_nLayout == Continuous)
			UpdatePageSizes(GetScrollPos(SB_VERT));

		UpdateView();
		GetMainFrame()->UpdatePageCombo(GetTopPage());
	}

	CScrollView::OnSize(nType, cx, cy);
}

void CDjVuView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_nLayout == SinglePage)
	{
		switch (nChar)
		{
		case VK_DOWN:
			OnScrollBy(CSize(0, 3*m_lineDev.cy));
			break;

		case VK_UP:
			OnScrollBy(CSize(0, -3*m_lineDev.cy));
			break;

		case VK_RIGHT:
			OnScrollBy(CSize(3*m_lineDev.cx, 0));
			break;

		case VK_LEFT:
			OnScrollBy(CSize(-3*m_lineDev.cx, 0));
			break;

		case VK_NEXT:
		case VK_SPACE:
			if (!OnScrollBy(CSize(0, m_pageDev.cy)))
			{
				if (m_nPage < m_nPageCount - 1 && m_nLayout == SinglePage)
				{
					CPoint ptScroll = GetScrollPosition();
					OnViewNextpage();
					OnScrollBy(CPoint(ptScroll.x, 0) - GetScrollPosition());
					Invalidate();
				}
			}
			break;

		case VK_PRIOR:
		case VK_BACK:
			if (!OnScrollBy(CSize(0, -m_pageDev.cy)))
			{
				if (m_nPage > 0 && m_nLayout == SinglePage)
				{
					CPoint ptScroll = GetScrollPosition();
					OnViewPreviouspage();
					OnScrollBy(CPoint(ptScroll.x, m_szDisplay.cy) - GetScrollPosition());
					Invalidate();
				}
			}
			break;
		}
	}
	else if (m_nLayout == Continuous)
	{
		int y = GetScrollPos(SB_VERT);
		int nScroll = 0;

		switch (nChar)
		{
		case VK_RIGHT:
			OnScrollBy(CSize(3*m_lineDev.cx, 0));
			return;

		case VK_LEFT:
			OnScrollBy(CSize(-3*m_lineDev.cx, 0));
			return;

		case VK_DOWN:
			nScroll = 3*m_lineDev.cy;
			break;

		case VK_UP:
			nScroll = -3*m_lineDev.cy;
			break;

		case VK_NEXT:
		case VK_SPACE:
			nScroll = m_pageDev.cy;
			break;

		case VK_PRIOR:
		case VK_BACK:
			nScroll = -m_pageDev.cy;
			break;
		}

		UpdatePageSizes(y, nScroll);

		OnScrollBy(CSize(0, nScroll));
		UpdateVisiblePages();
		GetMainFrame()->UpdatePageCombo(GetTopPage());
	}

	CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CDjVuView::OnViewZoom(UINT nID)
{
	switch (nID)
	{
	case ID_ZOOM_50:
		ZoomTo(ZoomPercent, 50.0);
		break;

	case ID_ZOOM_75:
		ZoomTo(ZoomPercent, 75.0);
		break;

	case ID_ZOOM_100:
		ZoomTo(ZoomPercent, 100.0);
		break;

	case ID_ZOOM_200:
		ZoomTo(ZoomPercent, 200.0);
		break;

	case ID_ZOOM_400:
		ZoomTo(ZoomPercent, 400.0);
		break;

	case ID_ZOOM_FITWIDTH:
		ZoomTo(ZoomFitWidth);
		break;

	case ID_ZOOM_FITHEIGHT:
		ZoomTo(ZoomFitHeight);
		break;

	case ID_ZOOM_FITPAGE:
		ZoomTo(ZoomFitPage);
		break;

	case ID_ZOOM_ACTUALSIZE:
		ZoomTo(ZoomActualSize);
		break;

	case ID_ZOOM_STRETCH:
		ZoomTo(ZoomStretch);
		break;

	case ID_ZOOM_CUSTOM:
		{
			// Leave two digits after decimal point
			double fZoom = static_cast<int>(GetZoom()*100.0)*0.01;

			CCustomZoomDlg dlg(fZoom);
			if (dlg.DoModal() == IDOK)
				ZoomTo(ZoomPercent, dlg.m_fZoom);
		}
		break;
	}
}

void CDjVuView::OnUpdateViewZoom(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_ZOOM_50:
		pCmdUI->SetCheck(m_nZoomType == ZoomPercent && m_fZoom == 50.0);
		break;

	case ID_ZOOM_75:
		pCmdUI->SetCheck(m_nZoomType == ZoomPercent && m_fZoom == 75.0);
		break;

	case ID_ZOOM_100:
		pCmdUI->SetCheck(m_nZoomType == ZoomPercent && m_fZoom == 100.0);
		break;

	case ID_ZOOM_200:
		pCmdUI->SetCheck(m_nZoomType == ZoomPercent && m_fZoom == 200.0);
		break;

	case ID_ZOOM_400:
		pCmdUI->SetCheck(m_nZoomType == ZoomPercent && m_fZoom == 400.0);
		break;

	case ID_ZOOM_FITWIDTH:
		pCmdUI->SetCheck(m_nZoomType == ZoomFitWidth);
		break;

	case ID_ZOOM_FITHEIGHT:
		pCmdUI->SetCheck(m_nZoomType == ZoomFitHeight);
		break;

	case ID_ZOOM_FITPAGE:
		pCmdUI->SetCheck(m_nZoomType == ZoomFitPage);
		break;

	case ID_ZOOM_ACTUALSIZE:
		pCmdUI->SetCheck(m_nZoomType == ZoomActualSize);
		break;

	case ID_ZOOM_STRETCH:
		pCmdUI->SetCheck(m_nZoomType == ZoomStretch);
		break;

	case ID_ZOOM_CUSTOM:
		pCmdUI->SetCheck(!IsStandardZoom(m_nZoomType, m_fZoom));
		break;
	}
}

double CDjVuView::GetZoom() const
{
	if (m_nZoomType == ZoomPercent)
		return m_fZoom;

	const Page& page = m_pages[GetTopPage()];
	CSize szPage = page.GetSize(m_nRotate);

	if (szPage.cx == 0 || szPage.cy == 0)
		return 100.0;

	// Calculate zoom from display area size
	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	int nLogPixels = dcScreen.GetDeviceCaps(LOGPIXELSX);

	return 100.0*page.szDisplay.cx*page.nDPI/(szPage.cx*nLogPixels);
}

void CDjVuView::OnViewLayout(UINT nID)
{
	int nPage = GetTopPage();
	int nOffset = GetScrollPos(SB_VERT) - m_pages[nPage].ptOffset.y;
	if (nOffset < 0)
		nOffset = 0;

	int nPrevLayout = m_nLayout;

	switch (nID)
	{
	case ID_LAYOUT_SINGLEPAGE:
		m_nLayout = SinglePage;
		break;

	case ID_LAYOUT_CONTINUOUS:
		m_nLayout = Continuous;
		break;
	}

	CAppSettings::nDefaultLayout = m_nLayout;

	if (m_nLayout != nPrevLayout)
	{
		UpdateView();

		int nTop = m_pages[nPage].ptOffset.y + nOffset;
		if (m_nLayout == Continuous)
		{
			UpdatePageSizes(nTop);
			nTop = m_pages[nPage].ptOffset.y + nOffset;
		}

		ScrollToPosition(CPoint(GetScrollPos(SB_HORZ), nTop));
	}
}

void CDjVuView::OnUpdateViewLayout(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_LAYOUT_SINGLEPAGE:
		pCmdUI->SetCheck(m_nLayout == SinglePage);
		break;

	case ID_LAYOUT_CONTINUOUS:
		pCmdUI->SetCheck(m_nLayout == Continuous);
		break;
	}
}

void CDjVuView::OnRotateLeft()
{
	m_nRotate = (m_nRotate + 1) % 4;

	DeleteBitmaps();
	UpdateView();

	GetMainFrame()->UpdatePageCombo(GetTopPage());
}

void CDjVuView::OnRotateRight()
{
	m_nRotate = (m_nRotate + 3) % 4;

	DeleteBitmaps();
	UpdateView();

	GetMainFrame()->UpdatePageCombo(GetTopPage());
}

void CDjVuView::OnRotate180()
{
	m_nRotate = (m_nRotate + 2) % 4;

	DeleteBitmaps();
	UpdateView();

	GetMainFrame()->UpdatePageCombo(GetTopPage());
}

void CDjVuView::OnViewFirstpage()
{
	if (m_nLayout == SinglePage)
	{
		RenderPage(0);
	}
	else if (m_nLayout == Continuous)
	{
		CRect rcClient;
		GetClientRect(rcClient);
		UpdatePagesFromTop(0, rcClient.Height());

		ScrollToPosition(CPoint(GetScrollPos(SB_HORZ), 0));
		UpdateVisiblePages();
		GetMainFrame()->UpdatePageCombo(GetTopPage());
	}
}

void CDjVuView::OnViewLastpage()
{
	if (m_nLayout == SinglePage)
	{
		RenderPage(m_nPageCount - 1);
	}
	else if (m_nLayout == Continuous)
	{
		CRect rcClient;
		GetClientRect(rcClient);
		UpdatePagesFromBottom(m_szDisplay.cy - rcClient.Height(), m_szDisplay.cy);

		OnScrollBy(CSize(0, m_szDisplay.cy));
		UpdateVisiblePages();
		GetMainFrame()->UpdatePageCombo(GetTopPage());
	}
}

void CDjVuView::OnSetFocus(CWnd* pOldWnd)
{
	CScrollView::OnSetFocus(pOldWnd);

	GetMainFrame()->UpdatePageCombo(GetTopPage());
	GetMainFrame()->UpdateZoomCombo(m_nZoomType, m_fZoom);
}

void CDjVuView::ZoomTo(int nZoomType, double fZoom)
{
	m_nZoomType = nZoomType;
	m_fZoom = fZoom;

	if (m_nZoomType == ZoomPercent)
	{
		// Leave two digits after decimal point
		m_fZoom = static_cast<int>(m_fZoom*100.0)*0.01;
		if (m_fZoom < 10.0)
			m_fZoom = 10.0;
		else if (m_fZoom > 800.0)
			m_fZoom = 800.0;
	}

	CAppSettings::nDefaultZoomType = nZoomType;
	CAppSettings::fDefaultZoom = fZoom;

	UpdateView();

	if (m_nLayout == Continuous)
		UpdatePageSizes(GetScrollPos(SB_VERT));

	GetMainFrame()->UpdateZoomCombo(m_nZoomType, m_fZoom);
	GetMainFrame()->UpdatePageCombo(GetTopPage());
}

BOOL CDjVuView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (hCursorHand == NULL)
		hCursorHand = AfxGetApp()->LoadCursor(IDC_CURSOR_HAND);
	if (hCursorDrag == NULL)
		hCursorDrag = AfxGetApp()->LoadCursor(IDC_CURSOR_DRAG);

	CRect rcClient;
	GetClientRect(rcClient);

	if (nHitTest == HTCLIENT &&
	   (m_totalDev.cx > rcClient.Width() || m_totalDev.cy > rcClient.Height()))
	{
		SetCursor(m_bDragging ? hCursorDrag : hCursorHand);
		return true;
	}

	return CScrollView::OnSetCursor(pWnd, nHitTest, message);
}

void CDjVuView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rcClient;
	GetClientRect(rcClient);

	if (m_totalDev.cx > rcClient.Width() || m_totalDev.cy > rcClient.Height())
	{
		SetCapture();
		m_bDragging = true;

		m_nStartPage = GetTopPage();
		m_ptStartPos = GetScrollPosition() - m_pages[m_nStartPage].ptOffset;

		::GetCursorPos(&m_ptStart);

		if (hCursorDrag == NULL)
			hCursorDrag = AfxGetApp()->LoadCursor(IDC_CURSOR_DRAG);
		SetCursor(hCursorDrag);
	}

	CScrollView::OnLButtonDown(nFlags, point);
}

void CDjVuView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDragging)
	{
		m_bDragging = false;
		ReleaseCapture();
	}

	CScrollView::OnLButtonUp(nFlags, point);
}

void CDjVuView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bDragging)
	{
		CPoint ptCursor;
		::GetCursorPos(&ptCursor);

		CPoint ptStartPos = m_pages[m_nStartPage].ptOffset + m_ptStartPos;
		CPoint ptScroll = ptStartPos + m_ptStart - ptCursor - GetScrollPosition();

		if (m_nLayout == Continuous)
		{
			int y = GetScrollPos(SB_VERT);
			UpdatePageSizes(y, ptScroll.y);

			ptStartPos = m_pages[m_nStartPage].ptOffset + m_ptStartPos;
			ptScroll = ptStartPos + m_ptStart - ptCursor - GetScrollPosition();
		}

		OnScrollBy(ptScroll);
		UpdateVisiblePages();
		GetMainFrame()->UpdatePageCombo(GetTopPage());
		return;
	}

	CScrollView::OnMouseMove(nFlags, point);
}

void CDjVuView::OnFilePrint()
{
	CPrintDlg dlg(GetDocument(), GetTopPage(), m_nRotate);
	if (dlg.DoModal() == IDOK)
	{
		ASSERT(dlg.m_hPrinter != NULL && dlg.m_pPrinter != NULL && dlg.m_pPaper != NULL);

		CProgressDlg progress_dlg(PrintThreadProc);
		progress_dlg.SetUserData(reinterpret_cast<DWORD_PTR>(&dlg));

		if (progress_dlg.DoModal() == IDOK)
		{
			if (progress_dlg.GetResultCode() > 0)
				AfxMessageBox("Error while sending data to printer.");
		}
	}
}

int CDjVuView::GetPageFromPoint(CPoint point)
{
	if (m_nLayout == SinglePage)
	{
		return m_nPage;
	}
	else
	{
		ScreenToClient(&point);
		point += GetScrollPosition();

		int nPage = 0;
		while (nPage < m_nPageCount - 1 &&
				point.y >= m_pages[nPage].rcDisplay.bottom)
			++nPage;

		return nPage;
	}
}

void CDjVuView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (point.x < 0 || point.y < 0)
	{
		point = CPoint(0, 0);
		ClientToScreen(&point);
	}

	CRect rcClient;
	GetClientRect(rcClient);
	ClientToScreen(rcClient);

	if (!rcClient.PtInRect(point))
	{
		CScrollView::OnContextMenu(pWnd, point);
		return;
	}

	m_nClickedPage = GetPageFromPoint(point);

	CMenu menu;
	menu.LoadMenu(IDR_POPUP);

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y,
		GetMainFrame());
}

void CDjVuView::OnPageInformation()
{
	GP<DjVuImage> pImage = GetDocument()->GetPage(m_nClickedPage);

	CString strDescr = pImage->get_long_description();
	CString strInfo, strLine;

	int nLine = 0;
	while (AfxExtractSubString(strLine, strDescr, nLine++, '\n'))
	{
		if (!strLine.IsEmpty() && strLine[0] == '\003')
			strLine = strLine.Mid(1);

		TCHAR szName[1000], szFile[1000];
		int nWidth, nHeight, nDPI, nVersion, nPart, nColors, nCCS, nShapes;
		double fSize, fRatio;
		CString strFormatted;

		if (strLine.Find(_T("DjVuFile.djvu_header")) == 0)
		{
			strLine = strLine.Mid(20);
			_stscanf(strLine, _T("%d%d%d%d"), &nWidth, &nHeight, &nDPI, &nVersion);
			strFormatted.Format(_T("DJVU Image (%dx%d, %d dpi) version %d:\n"),
				nWidth, nHeight, nDPI, nVersion);
		}
		else if (strLine.Find(_T("DjVuFile.IW44_header")) == 0)
		{
			strLine = strLine.Mid(20);
			_stscanf(strLine, _T("%d%d%d"), &nWidth, &nHeight, &nDPI);
			strFormatted.Format(_T("IW44 Image (%dx%d, %d dpi):\n"),
				nWidth, nHeight, nDPI);
		}
		else if (strLine.Find(_T("DjVuFile.page_info")) == 0)
		{
			strLine = strLine.Mid(18);
			_stscanf(strLine, _T("%lf%s"), &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tPage information."),
				fSize, szName);
		}
		else if (strLine.Find(_T("DjVuFile.indir_chunk1")) == 0)
		{
			strLine = strLine.Mid(21);
			_stscanf(strLine, _T("%s%lf%s"), szFile, &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tIndirection chunk (%s)."),
				fSize, szName, szFile);
		}
		else if (strLine.Find(_T("DjVuFile.indir_chunk2")) == 0)
		{
			strLine = strLine.Mid(21);
			_stscanf(strLine, _T("%lf%s"), &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tIndirection chunk."),
				fSize, szName);
		}
		else if (strLine.Find(_T("DjVuFile.JB2_fg")) == 0)
		{
			strLine = strLine.Mid(15);
			_stscanf(strLine, _T("%d%d%lf%s"), &nColors, &nCCS, &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tJB2 foreground colors (%d color%s, %d ccs)."),
				fSize, szName, nColors, (nColors == 1 ? _T("") : _T("s")), nCCS);
		}
		else if (strLine.Find(_T("DjVuFile.shape_dict")) == 0)
		{
			strLine = strLine.Mid(19);
			_stscanf(strLine, _T("%d%lf%s"), &nShapes, &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tJB2 shapes dictionary (%d shape%s)."),
				fSize, szName, nShapes, (nColors == 1 ? _T("") : _T("s")));
		}
		else if (strLine.Find(_T("DjVuFile.fg_mask")) == 0)
		{
			strLine = strLine.Mid(16);
			_stscanf(strLine, _T("%d%d%d%lf%s"), &nWidth, &nHeight, &nDPI, &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tJB2 foreground mask (%dx%d, %d dpi)."),
				fSize, szName, nWidth, nHeight, nDPI);
		}
		else if (strLine.Find(_T("DjVuFile.G4_mask")) == 0)
		{
			strLine = strLine.Mid(16);
			_stscanf(strLine, _T("%d%d%d%lf%s"), &nWidth, &nHeight, &nDPI, &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tG4 foreground mask (%dx%d, %d dpi)."),
				fSize, szName, nWidth, nHeight, nDPI);
		}
		else if (strLine.Find(_T("DjVuFile.IW44_bg1")) == 0)
		{
			strLine = strLine.Mid(17);
			_stscanf(strLine, _T("%d%d%d%lf%s"), &nWidth, &nHeight, &nDPI, &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tIW44 background (%dx%d, %d dpi)."),
				fSize, szName, nWidth, nHeight, nDPI);
		}
		else if (strLine.Find(_T("DjVuFile.IW44_bg2")) == 0)
		{
			strLine = strLine.Mid(17);
			_stscanf(strLine, _T("%d%d%lf%s"), &nPart, &nDPI, &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tIW44 background (part %d, %d dpi)."),
				fSize, szName, nPart, nDPI);
		}
		else if (strLine.Find(_T("DjVuFile.IW44_data1")) == 0)
		{
			strLine = strLine.Mid(19);
			_stscanf(strLine, _T("%d%d%d%lf%s"), &nWidth, &nHeight, &nDPI, &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tIW44 data (%dx%d, %d dpi)."),
				fSize, szName, nWidth, nHeight, nDPI);
		}
		else if (strLine.Find(_T("DjVuFile.IW44_data2")) == 0)
		{
			strLine = strLine.Mid(19);
			_stscanf(strLine, _T("%d%d%lf%s"), &nPart, &nDPI, &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tIW44 data (part %d, %d dpi)."),
				fSize, szName, nPart, nDPI);
		}
		else if (strLine.Find(_T("DjVuFile.IW44_fg")) == 0)
		{
			strLine = strLine.Mid(16);
			_stscanf(strLine, _T("%d%d%d%lf%s"), &nWidth, &nHeight, &nDPI, &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tIW44 foreground colors (%dx%d, %d dpi)."),
				fSize, szName, nWidth, nHeight, nDPI);
		}
		else if (strLine.Find(_T("DjVuFile.ratio")) == 0)
		{
			strLine = strLine.Mid(14);
			_stscanf(strLine, _T("%lf%lf"), &fRatio, &fSize);
			strFormatted.Format(_T("\nCompression ratio: %.1f (%.1f Kb)"),
				fRatio, fSize);
		}
		else if (strLine.Find(_T("DjVuFile.text")) == 0)
		{
			strLine = strLine.Mid(13);
			_stscanf(strLine, _T("%lf%s"), &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tText (text, etc.)."),
				fSize, szName);
		}
		else
			strFormatted = strLine;

		// TODO: Display information about the following chunks:
		//   DjVuFile.color_import1
		//   DjVuFile.color_import2
		//   DjVuFile.JPEG_bg1
		//   DjVuFile.JPEG_bg2
		//   DjVuFile.JPEG_fg1
		//   DjVuFile.JPEG_fg2
		//   DjVuFile.JPEG2K_bg
		//   DjVuFile.JPEG2K_fg
		//   DjVuFile.nav_dir
		//   DjVuFile.anno1
		//   DjVuFile.anno2

		strInfo += (!strInfo.IsEmpty() ? _T("\n") : _T("")) + strFormatted;
	}

	AfxMessageBox(strInfo, MB_ICONINFORMATION | MB_OK);
}

LRESULT CDjVuView::OnRenderFinished(WPARAM wParam, LPARAM lParam)
{
	CDIB* pBitmap = reinterpret_cast<CDIB*>(lParam);
	int nPage = (int)wParam;
	Page& page = m_pages[nPage];

	if (page.pBitmap != NULL && page.szDisplay == page.pBitmap->GetSize())
	{
		// Bitmap is too old, ignore it
		delete pBitmap;
		return 0;
	}

	page.DeleteBitmap();
	page.pBitmap = pBitmap;

	InvalidatePage(nPage);
	UpdateWindow();

	return 0;
}

void CDjVuView::OnDestroy()
{
	delete m_pRenderThread;
	m_pRenderThread = NULL;

	CScrollView::OnDestroy();
}

LRESULT CDjVuView::OnPageDecoded(WPARAM wParam, LPARAM lParam)
{
	int nPage = (int)wParam;
	GP<DjVuImage> pImage = GetDocument()->GetPage(nPage);
	pImage->set_rotate(0);

	Page& page = m_pages[nPage];
	page.szPage = CSize(pImage->get_width(), pImage->get_height());
	page.nDPI = pImage->get_dpi();
	page.bSizeLoaded = true;

	UpdateView();
	return 0;
}

BOOL CDjVuView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
	if (m_nLayout == Continuous)
	{
		CRect rcClient;
		GetClientRect(rcClient);

		int nCode = HIBYTE(nScrollCode);
		if (nCode == SB_TOP)
		{
			UpdatePagesFromTop(0, rcClient.Height());
		}
		else if (nCode == SB_BOTTOM)
		{
			UpdatePagesFromBottom(m_szDisplay.cy - rcClient.Height(), m_szDisplay.cy);
		}
		else if (nCode == SB_THUMBTRACK)
		{
			SCROLLINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cbSize = sizeof(si);

			if (!GetScrollInfo(SB_VERT, &si, SIF_TRACKPOS))
				return true;

			UpdatePageSizes(si.nTrackPos);

			int yOrig = GetScrollPos(SB_VERT);

			BOOL bResult = OnScrollBy(CSize(0, si.nTrackPos - yOrig), bDoScroll);
			if (bResult && bDoScroll)
				UpdateWindow();

			return bResult;
		}
		else
		{
			int y = GetScrollPos(SB_VERT);
			int nScroll = 0;

			// Calculate new x position
			switch (nCode)
			{
			case SB_LINEUP:
				nScroll = m_lineDev.cy;
				break;
			case SB_LINEDOWN:
				nScroll = m_lineDev.cy;
				break;
			case SB_PAGEUP:
				nScroll = m_pageDev.cy;
				break;
			case SB_PAGEDOWN:
				nScroll = m_pageDev.cy;
				break;
			}

			UpdatePageSizes(y, nScroll);
		}
	}

	BOOL bResult = CScrollView::OnScroll(nScrollCode, nPos, bDoScroll);
	UpdateVisiblePages();
	GetMainFrame()->UpdatePageCombo(GetTopPage());

	return bResult;
}

void CDjVuView::UpdatePageSizes(int nTop, int nScroll)
{
	CRect rcClient;
	GetClientRect(rcClient);

	if (nScroll < 0)
	{
		UpdatePagesFromBottom(nTop + nScroll, nTop + rcClient.Height());
	}
	else
	{
		if (!UpdatePagesFromTop(nTop, nTop + rcClient.Height() + nScroll))
		{
			int nBottom = m_szDisplay.cy;
			UpdatePagesFromBottom(nBottom - rcClient.Height() - nScroll, nBottom);
		}
	}
}

void CDjVuView::UpdatePagesFromBottom(int nTop, int nBottom)
{
	// Check that the pages which become visible as a result
	// of this operation have their szPage filled in

	bool bUpdateView = false;

	int nPage = 0;
	while (nPage < m_nPageCount - 1 &&
		   nBottom > m_pages[nPage].rcDisplay.bottom)
		++nPage;

	if (!m_pages[nPage].bSizeLoaded)
	{
		Page& page = m_pages[nPage];

		GetDocument()->GetPageInfo(nPage, page.szPage, page.nDPI);
		page.bSizeLoaded = true;

		page.szDisplay = CalcPageSize(page.GetSize(m_nRotate), page.nDPI);
	}

	while (nPage > 0 && nTop < m_pages[nPage].ptOffset.y)
	{
		--nPage;
		if (!m_pages[nPage].bSizeLoaded)
		{
			Page& page = m_pages[nPage];

			GetDocument()->GetPageInfo(nPage, page.szPage, page.nDPI);
			page.bSizeLoaded = true;

			page.szDisplay = CalcPageSize(page.GetSize(m_nRotate), page.nDPI);
			page.ptOffset.y = m_pages[nPage + 1].ptOffset.y -
				page.szDisplay.cy - c_nPageGap;

			bUpdateView = true;
		}
	}

	if (bUpdateView)
		UpdateView(BOTTOM);
}

bool CDjVuView::UpdatePagesFromTop(int nTop, int nBottom)
{
	bool bUpdateView = false;

	int nPage = 0;
	while (nPage < m_nPageCount - 1 &&
		   nTop >= m_pages[nPage].rcDisplay.bottom)
		++nPage;

	if (!m_pages[nPage].bSizeLoaded)
	{
		Page& page = m_pages[nPage];

		GetDocument()->GetPageInfo(nPage, page.szPage, page.nDPI);
		page.bSizeLoaded = true;

		page.szDisplay = CalcPageSize(page.GetSize(m_nRotate), page.nDPI);
	}

	while (nPage < m_nPageCount - 1 &&
		   nBottom > m_pages[nPage].ptOffset.y + m_pages[nPage].szDisplay.cy + c_nPageGap - 1)
	{
		++nPage;
		if (!m_pages[nPage].bSizeLoaded)
		{
			Page& page = m_pages[nPage];

			GetDocument()->GetPageInfo(nPage, page.szPage, page.nDPI);
			page.bSizeLoaded = true;

			page.szDisplay = CalcPageSize(page.GetSize(m_nRotate), page.nDPI);
			page.ptOffset.y = m_pages[nPage - 1].ptOffset.y +
				m_pages[nPage - 1].szDisplay.cy + c_nPageGap;

			bUpdateView = true;
		}
	}

	bool bNeedScrollUp = (nBottom > m_pages[nPage].ptOffset.y +
		m_pages[nPage].szDisplay.cy + c_nPageGap - 1);

	if (bUpdateView)
		UpdateView(TOP);

	return !bNeedScrollUp;
}

int CDjVuView::GetTopPage() const
{
	if (m_nLayout == SinglePage)
	{
		return m_nPage;
	}
	else
	{
		int nTop = GetScrollPos(SB_VERT);

		int nPage = 0;
		while (nPage < m_nPageCount - 1 &&
				nTop >= m_pages[nPage].rcDisplay.bottom)
			++nPage;

		return nPage;
	}
}


UINT GetMouseScrollLines()
{
	static UINT uCachedScrollLines;
	static bool bGotScrollLines;

	// If we've already got it and we're not refreshing,
	// return what we've already got

	if (bGotScrollLines)
		return uCachedScrollLines;

	// see if we can find the mouse window

	bGotScrollLines = TRUE;

	static UINT msgGetScrollLines;
	static WORD nRegisteredMessage;

	if (afxData.bWin95)
	{
		if (nRegisteredMessage == 0)
		{
			msgGetScrollLines = ::RegisterWindowMessage(MSH_SCROLL_LINES);
			if (msgGetScrollLines == 0)
				nRegisteredMessage = 1;     // couldn't register!  never try again
			else
				nRegisteredMessage = 2;     // it worked: use it
		}

		if (nRegisteredMessage == 2)
		{
			HWND hwMouseWheel = NULL;
			hwMouseWheel = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);
			if (hwMouseWheel && msgGetScrollLines)
			{
				uCachedScrollLines = (UINT)
					::SendMessage(hwMouseWheel, msgGetScrollLines, 0, 0);
			}
		}
	}
	else
	{
		// couldn't use the window -- try system settings
		uCachedScrollLines = 3; // reasonable default
		if (!afxData.bWin95)
			::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uCachedScrollLines, 0);
	}

	return uCachedScrollLines;
}

BOOL CDjVuView::OnMouseWheel(UINT nFlags, short zDelta, CPoint /*point*/)
{
	// we don't handle anything but scrolling
	if ((nFlags & MK_CONTROL) != 0)
		return FALSE;

	// if the parent is a splitter, it will handle the message
	if (GetParentSplitter(this, TRUE))
		return FALSE;

	BOOL bHasHorzBar, bHasVertBar;
	CheckScrollBars(bHasHorzBar, bHasVertBar);
	if (!bHasVertBar && !bHasHorzBar)
		return false;

	BOOL bResult = FALSE;
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

		UpdatePageSizes(GetScrollPos(SB_VERT), nDisplacement);

		bResult = OnScrollBy(CSize(0, nDisplacement), TRUE);
		UpdateVisiblePages();
		GetMainFrame()->UpdatePageCombo(GetTopPage());
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

		bResult = OnScrollBy(CSize(nDisplacement, 0), TRUE);
	}

	if (bResult)
		UpdateWindow();

	return bResult;
}

void CDjVuView::InvalidatePage(int nPage)
{
	ASSERT(nPage >= 0 && nPage <= m_nPageCount);

	CRect rect;
	GetClientRect(rect);

	int y = GetScrollPos(SB_VERT);
	rect.top = max(rect.top, m_pages[nPage].rcDisplay.top - y);
	rect.bottom = min(rect.bottom, m_pages[nPage].rcDisplay.bottom - y);

	if (!rect.IsRectEmpty())
		InvalidateRect(rect);
}

void CDjVuView::SetScrollSizesNoRepaint(const CSize& szTotal,
	const CSize& szPage, const CSize& szLine)
{
	ASSERT(szTotal.cx >= 0 && szTotal.cy >= 0);

	int nOldMapMode = m_nMapMode;
	m_nMapMode = MM_TEXT;
	m_totalLog = szTotal;

	//BLOCK: convert logical coordinate space to device coordinates
	{
		CWindowDC dc(NULL);
		ASSERT(m_nMapMode > 0);
		dc.SetMapMode(m_nMapMode);

		// total size
		m_totalDev = m_totalLog;
		dc.LPtoDP((LPPOINT)&m_totalDev);
		m_pageDev = szPage;
		dc.LPtoDP((LPPOINT)&m_pageDev);
		m_lineDev = szLine;
		dc.LPtoDP((LPPOINT)&m_lineDev);
		if (m_totalDev.cy < 0)
			m_totalDev.cy = -m_totalDev.cy;
		if (m_pageDev.cy < 0)
			m_pageDev.cy = -m_pageDev.cy;
		if (m_lineDev.cy < 0)
			m_lineDev.cy = -m_lineDev.cy;
	} // release DC here

	// now adjust device specific sizes
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);
	if (m_pageDev.cx == 0)
		m_pageDev.cx = m_totalDev.cx / 10;
	if (m_pageDev.cy == 0)
		m_pageDev.cy = m_totalDev.cy / 10;
	if (m_lineDev.cx == 0)
		m_lineDev.cx = m_pageDev.cx / 10;
	if (m_lineDev.cy == 0)
		m_lineDev.cy = m_pageDev.cy / 10;

	if (m_hWnd != NULL)
	{
		// window has been created, invalidate now
		UpdateBarsNoRepaint();
	}
}

void CDjVuView::UpdateBarsNoRepaint()
{
	// update the horizontal to reflect reality
	// NOTE: turning on/off the scrollbars will cause 'OnSize' callbacks
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);

	CRect rectClient;
	bool bCalcClient = true;

	// allow parent to do inside-out layout first
	CWnd* pParentWnd = GetParent();
	if (pParentWnd != NULL)
	{
		// if parent window responds to this message, use just
		//  client area for scroll bar calc -- not "true" client area
		if (pParentWnd->SendMessage(WM_RECALCPARENT, 0,
			(LPARAM)(LPCRECT)&rectClient) != 0)
		{
			// use rectClient instead of GetTrueClientSize for
			//  client size calculation.
			bCalcClient = false;
		}
	}

	CSize sizeClient;
	CSize sizeSb;

	if (bCalcClient)
	{
		// get client rect
		if (!GetTrueClientSize(sizeClient, sizeSb))
		{
			// no room for scroll bars (common for zero sized elements)
			CRect rect;
			GetClientRect(&rect);
			if (rect.right > 0 && rect.bottom > 0)
			{
				// if entire client area is not invisible, assume we have
				//  control over our scrollbars
				EnableScrollBarCtrl(SB_BOTH, false);
			}
			return;
		}
	}
	else
	{
		// let parent window determine the "client" rect
		GetScrollBarSizes(sizeSb);
		sizeClient.cx = rectClient.right - rectClient.left;
		sizeClient.cy = rectClient.bottom - rectClient.top;
	}

	// enough room to add scrollbars
	CSize sizeRange;
	CPoint ptMove;
	CSize needSb;

	// get the current scroll bar state given the true client area
	GetScrollBarState(sizeClient, needSb, sizeRange, ptMove, bCalcClient);
	if (needSb.cx)
		sizeClient.cy -= sizeSb.cy;
	if (needSb.cy)
		sizeClient.cx -= sizeSb.cx;

	// first scroll the window as needed
	SetScrollPos(SB_HORZ, ptMove.x);
	SetScrollPos(SB_VERT, ptMove.y);

	// this structure needed to update the scrollbar page range
	SCROLLINFO info;
	info.fMask = SIF_PAGE|SIF_RANGE;
	info.nMin = 0;

	// now update the bars as appropriate
	EnableScrollBarCtrl(SB_HORZ, needSb.cx);
	if (needSb.cx)
	{
		info.nPage = sizeClient.cx;
		info.nMax = m_totalDev.cx-1;
		if (!SetScrollInfo(SB_HORZ, &info, true))
			SetScrollRange(SB_HORZ, 0, sizeRange.cx, true);
	}

	EnableScrollBarCtrl(SB_VERT, needSb.cy);
	if (needSb.cy)
	{
		info.nPage = sizeClient.cy;
		info.nMax = m_totalDev.cy-1;
		if (!SetScrollInfo(SB_VERT, &info, true))
			SetScrollRange(SB_VERT, 0, sizeRange.cy, true);
	}
}

void CDjVuView::ScrollToPositionNoRepaint(CPoint pt)
{
	ASSERT(m_nMapMode > 0);     // not allowed for shrink to fit
	if (m_nMapMode != MM_TEXT)
	{
		CWindowDC dc(NULL);
		dc.SetMapMode(m_nMapMode);
		dc.LPtoDP((LPPOINT)&pt);
	}

	// now in device coordinates - limit if out of range
	int xMax = GetScrollLimit(SB_HORZ);
	int yMax = GetScrollLimit(SB_VERT);
	if (pt.x < 0)
		pt.x = 0;
	else if (pt.x > xMax)
		pt.x = xMax;
	if (pt.y < 0)
		pt.y = 0;
	else if (pt.y > yMax)
		pt.y = yMax;

	SetScrollPos(SB_HORZ, pt.x);
	SetScrollPos(SB_VERT, pt.y);
}
