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
#include "WinDjView.h"

#include "DjVuDoc.h"
#include "DjVuView.h"
#include "AppSettings.h"

#include "MainFrm.h"
#include "PrintDlg.h"
#include "Drawing.h"
#include "ProgressDlg.h"
#include "ZoomDlg.h"
#include "FindDlg.h"

#include "RenderThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDjVuView

HCURSOR CDjVuView::hCursorHand = NULL;
HCURSOR CDjVuView::hCursorDrag = NULL;
HCURSOR CDjVuView::hCursorLink = NULL;

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
	ON_COMMAND(ID_EXPORT_PAGE, OnExportPage)
	ON_COMMAND(ID_FIND_STRING, OnFindString)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNeedText)
	ON_COMMAND(ID_VIEW_BACK, OnViewBack)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BACK, OnUpdateViewBack)
	ON_COMMAND(ID_VIEW_FORWARD, OnViewForward)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FORWARD, OnUpdateViewForward)
END_MESSAGE_MAP()

// CDjVuView construction/destruction

CDjVuView::CDjVuView()
	: m_nPage(-1), m_nPageCount(0), m_nZoomType(ZoomPercent), m_fZoom(100.0),
	  m_nLayout(SinglePage), m_nRotate(0), m_bDragging(false),
	  m_pRenderThread(NULL), m_bInsideUpdateView(false), m_bClick(false),
	  m_historyPos(m_history.end()), m_evtRendered(false, true),
	  m_nPendingPage(-1), m_nClickedPage(-1)
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

	// CScrollView sets viewport to minus scroll position, so that
	// all drawing can be done in natural coordinates. Unfortunately,
	// this does not work in Win98, because in this OS coordinates
	// cannot be larger than 32767. So we will subtract scroll position
	// explicitely.
	pDC->SetViewportOrg(CPoint(0, 0));

	if (m_nLayout == SinglePage)
	{
        DrawPage(pDC, m_nPage);
	}
	else if (m_nLayout == Continuous)
	{
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

		// Draw hyperlinks
		if (page.pAnt != NULL)
		{
			for (GPosition pos = page.pAnt->map_areas; pos; ++pos)
			{
				GP<GMapArea> pArea = page.pAnt->map_areas[pos];
				DrawMapArea(pDC, pArea, nPage, !!(pArea == m_pActiveLink));
			}
		}

		// Draw selection
		for (GPosition pos = page.selection; pos; ++pos)
		{
			GRect rect = page.selection[pos]->rect;
			CRect rcText = TranslatePageRect(nPage, rect);

			rcText.OffsetRect(-GetDeviceScrollPosition());
			pDC->InvertRect(rcText);
		}
	}
	else
	{
		DrawWhite(pDC, nPage);
	}
}

void CDjVuView::DrawMapArea(CDC* pDC, GP<GMapArea> pArea, int nPage, bool bActive)
{
	if (pArea->border_type == GMapArea::NO_BORDER ||
			!bActive && !pArea->border_always_visible)
		return;

	CRect rcArea = TranslatePageRect(nPage, pArea->get_bound_rect());
	rcArea.OffsetRect(-GetDeviceScrollPosition());
	CPoint points[] = { rcArea.TopLeft(), CPoint(rcArea.right, rcArea.top),
		rcArea.BottomRight(), CPoint(rcArea.left, rcArea.bottom), rcArea.TopLeft() };

	if (pArea->border_type == GMapArea::SOLID_BORDER)
	{
		DWORD dwColor = pArea->border_color;
		COLORREF crBorder = RGB(GetBValue(dwColor), GetGValue(dwColor), GetRValue(dwColor));
		CPen pen(PS_SOLID, 1, crBorder);
		CPen* pOldPen = pDC->SelectObject(&pen);
		pDC->Polyline((LPPOINT)points, 5);
		pDC->SelectObject(pOldPen);
	}
	else
	{
		// Draw XOR border
		CRect rcHorz(rcArea.TopLeft(), CSize(rcArea.Width() + 1, 1));
		pDC->InvertRect(rcHorz);
		rcHorz.OffsetRect(0, rcArea.Height());
		pDC->InvertRect(rcHorz);

		CRect rcVert(rcArea.TopLeft() + CPoint(0, 1), CSize(1, rcArea.Height() - 1));
		pDC->InvertRect(rcVert);
		rcVert.OffsetRect(rcArea.Width(), 0);
		pDC->InvertRect(rcVert);
	}
}

void CDjVuView::DrawWhite(CDC* pDC, int nPage)
{
	Page& page = m_pages[nPage];

	CPoint ptDeviceScrollPos = GetDeviceScrollPosition();

	CRect rcClip;
	pDC->GetClipBox(rcClip);
	rcClip.OffsetRect(ptDeviceScrollPos);

	if (m_nLayout == Continuous)
	{
		rcClip.top = max(rcClip.top, page.rcDisplay.top);
		rcClip.bottom = min(rcClip.bottom, page.rcDisplay.bottom);
	}

	if (!rcClip.IsRectEmpty())
	{
		rcClip.OffsetRect(-ptDeviceScrollPos);

		COLORREF clrWindow = ::GetSysColor(COLOR_WINDOW);
		pDC->FillSolidRect(rcClip, clrWindow);
	}

	if (page.szDisplay.cx > 0 && page.szDisplay.cy > 0)
	{
		CRect rcBorder(page.ptOffset.x - 1, page.ptOffset.y - 1,
			page.ptOffset.x + page.szDisplay.cx,
			page.ptOffset.y + page.szDisplay.cy);
		rcBorder.OffsetRect(-ptDeviceScrollPos);

		CPoint points[] = { rcBorder.TopLeft(), CPoint(rcBorder.right, rcBorder.top),
			rcBorder.BottomRight(), CPoint(rcBorder.left, rcBorder.bottom),
			rcBorder.TopLeft() };

		CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_3DDKSHADOW));
		CPen* pOldPen = pDC->SelectObject(&pen);
		pDC->Polyline((LPPOINT)points, 5);
		pDC->SelectObject(pOldPen);

		CRect rcShadow(page.ptOffset.x + c_nPageShadow - 1, page.ptOffset.y + page.szDisplay.cy + 1,
			page.ptOffset.x + page.szDisplay.cx + c_nPageShadow, page.ptOffset.y + page.szDisplay.cy + c_nPageShadow);
		rcShadow.OffsetRect(-ptDeviceScrollPos);
		pDC->FillSolidRect(rcShadow, ::GetSysColor(COLOR_BTNSHADOW));

		rcShadow = CRect(page.ptOffset.x + page.szDisplay.cx + 1, page.ptOffset.y + c_nPageShadow - 1,
			page.ptOffset.x + page.szDisplay.cx + c_nPageShadow, page.ptOffset.y + page.szDisplay.cy + 1);
		rcShadow.OffsetRect(-ptDeviceScrollPos);
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
	CPoint ptDeviceScrollPos = GetDeviceScrollPosition();

	CRect rcClip;
	pDC->GetClipBox(rcClip);
	rcClip.OffsetRect(ptDeviceScrollPos);

	CPoint ptPartOffset(max(rcClip.left - page.ptOffset.x, 0),
		max(rcClip.top - page.ptOffset.y, 0));

	CSize szPageClip = page.szDisplay - ptPartOffset;
	CSize szPart(min(rcClip.Width(), szPageClip.cx), min(rcClip.Height(), szPageClip.cy));

	if (szPart.cx > 0 && szPart.cy > 0)
	{
		CRect rcPart(ptPartOffset, szPart);
		page.pBitmap->DrawDC(pDC, page.ptOffset + ptPartOffset
			- ptDeviceScrollPos, rcPart);
	}

	CRect rcBorder(page.ptOffset.x - 1, page.ptOffset.y - 1,
		page.ptOffset.x + page.szDisplay.cx,
		page.ptOffset.y + page.szDisplay.cy);
	rcBorder.OffsetRect(-ptDeviceScrollPos);

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
		rcClip.OffsetRect(-ptDeviceScrollPos);
		int nSaveDC = pDC->SaveDC();
		rcBorder.InflateRect(0, 0, 1, 1);
		pDC->ExcludeClipRect(rcBorder);

		COLORREF clrWindow = ::GetSysColor(COLOR_WINDOW);
		pDC->FillSolidRect(rcClip, clrWindow);

		pDC->RestoreDC(nSaveDC);
	}

	CRect rcShadow(page.ptOffset.x + c_nPageShadow - 1, page.ptOffset.y + page.szDisplay.cy + 1,
		page.ptOffset.x + page.szDisplay.cx + c_nPageShadow, page.ptOffset.y + page.szDisplay.cy + c_nPageShadow);
	rcShadow.OffsetRect(-ptDeviceScrollPos);
	pDC->FillSolidRect(rcShadow, ::GetSysColor(COLOR_BTNSHADOW));

	rcShadow = CRect(page.ptOffset.x + page.szDisplay.cx + 1, page.ptOffset.y + c_nPageShadow - 1,
		page.ptOffset.x + page.szDisplay.cx + c_nPageShadow, page.ptOffset.y + page.szDisplay.cy + 1);
	rcShadow.OffsetRect(-ptDeviceScrollPos);
	pDC->FillSolidRect(rcShadow, ::GetSysColor(COLOR_BTNSHADOW));
}

void CDjVuView::DrawStretch(CDC* pDC, int nPage)
{
	Page& page = m_pages[nPage];
	ASSERT(page.pBitmap != NULL && page.pBitmap->m_hObject != NULL);

	CPoint ptDeviceScrollPos = GetDeviceScrollPosition();

	CRect rcClip;
	pDC->GetClipBox(rcClip);
	rcClip.OffsetRect(ptDeviceScrollPos);

	page.pBitmap->Draw(pDC, page.ptOffset - ptDeviceScrollPos, page.szDisplay);

	CRect rcBorder(page.ptOffset.x - 1, page.ptOffset.y - 1,
		page.ptOffset.x + page.szDisplay.cx,
		page.ptOffset.y + page.szDisplay.cy);
	rcBorder.OffsetRect(-ptDeviceScrollPos);

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
		rcClip.OffsetRect(-ptDeviceScrollPos);

		int nSaveDC = pDC->SaveDC();
		rcBorder.InflateRect(0, 0, 1, 1);
		pDC->ExcludeClipRect(rcBorder);

		COLORREF clrWindow = ::GetSysColor(COLOR_WINDOW);
		pDC->FillSolidRect(rcClip, clrWindow);

		pDC->RestoreDC(nSaveDC);
	}

	CRect rcShadow(page.ptOffset.x + c_nPageShadow - 1, page.ptOffset.y + page.szDisplay.cy + 1,
		page.ptOffset.x + page.szDisplay.cx + c_nPageShadow, page.ptOffset.y + page.szDisplay.cy + c_nPageShadow);
	rcShadow.OffsetRect(-ptDeviceScrollPos);
	pDC->FillSolidRect(rcShadow, ::GetSysColor(COLOR_BTNSHADOW));

	rcShadow = CRect(page.ptOffset.x + page.szDisplay.cx + 1, page.ptOffset.y + c_nPageShadow - 1,
		page.ptOffset.x + page.szDisplay.cx + c_nPageShadow, page.ptOffset.y + page.szDisplay.cy + 1);
	rcShadow.OffsetRect(-ptDeviceScrollPos);
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

	if (m_toolTip.Create(this, TTS_ALWAYSTIP) && m_toolTip.AddTool(this))
	{
		m_toolTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);
		m_toolTip.Activate(FALSE);
	}

	m_nZoomType = CAppSettings::nDefaultZoomType;
	m_fZoom = CAppSettings::fDefaultZoom;
	if (m_nZoomType == ZoomPercent)
		m_fZoom = 100.0;

	m_nLayout = CAppSettings::nDefaultLayout;

	m_nPageCount = GetDocument()->GetPageCount();
	m_pages.resize(m_nPageCount);
	m_nPage = 0;

	m_pRenderThread = new CRenderThread(GetDocument(), this);

	UpdateView(RECALC);
	RenderPage(0);
}

BOOL CDjVuView::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void CDjVuView::RenderPage(int nPage, int nTimeout)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);
	Page& page = m_pages[nPage];

	m_nPendingPage = nPage;
	m_evtRendered.ResetEvent();

	if (m_nLayout == SinglePage)
	{
		m_nPage = nPage;

		if (!page.bInfoLoaded)
		{
			PageInfo info = GetDocument()->GetPageInfo(nPage);
			page.Init(info);
		}

		UpdateView();
		ScrollToPositionNoRepaint(CPoint(GetScrollPos(SB_HORZ), 0));
		UpdatePagesCache();
	}
	else
	{
		UpdatePageSizes(page.ptOffset.y);

		int nUpdatedPos = page.ptOffset.y - 1;
		ScrollToPositionNoRepaint(CPoint(GetScrollPos(SB_HORZ), nUpdatedPos));
		UpdateVisiblePages();
	}

	if (nTimeout != -1 && m_pages[nPage].pBitmap == NULL)
		::WaitForSingleObject(m_evtRendered, nTimeout);
	m_nPendingPage = -1;

	GetMainFrame()->UpdatePageCombo(GetCurrentPage());

	Invalidate();
	UpdateWindow();
}

void CDjVuView::DeleteBitmaps()
{
	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		m_pages[nPage].DeleteBitmap();
	}
}

void CDjVuView::UpdateView(UpdateType updateType)
{
	if (m_bInsideUpdateView)
		return;

	m_bInsideUpdateView = true;

	if (m_nLayout == SinglePage)
	{
		for (int nPage = 0; nPage < m_nPageCount; ++nPage)
			UpdatePageSize(nPage);

		Page& page = m_pages[m_nPage];

		// Update current page size 2 times to allow for scrollbars
		for (int i = 0; i < 2; ++i)
		{
			UpdatePageSize(m_nPage);

			m_szDisplay = page.rcDisplay.Size();

			CRect rcClient;
			GetClientRect(rcClient);
			CSize szDevPage(rcClient.Width()*3/4, rcClient.Height()*3/4);
			CSize szDevLine(15, 15);

			SetScrollSizes(m_szDisplay, szDevPage, szDevLine);
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
		CRect rcClient;
		GetClientRect(rcClient);

		// Save page and offset to restore after changes
		int nAnchorPage;
		CPoint ptAnchorOffset;
		CPoint ptTop = GetDeviceScrollPosition();

		if (updateType == TOP)
		{
			nAnchorPage = m_nPage;
			ptAnchorOffset = ptTop - m_pages[m_nPage].ptOffset;
		}
		else if (updateType == BOTTOM)
		{
			CPoint ptBottom = ptTop + rcClient.Size();

			int nPage = m_nPage;
			while (nPage < m_nPageCount - 1 &&
				ptBottom.y > m_pages[nPage].rcDisplay.bottom)
				++nPage;

			nAnchorPage = nPage;
			ptAnchorOffset = ptBottom - m_pages[nPage].ptOffset;
		}

		for (int i = 0; i < 2; ++i)
		{
			m_szDisplay = CSize(0, 0);
			m_pages[0].ptOffset = CPoint(0, 1);

			for (int nPage = 0; nPage < m_nPageCount; ++nPage)
			{
				Page& page = m_pages[nPage];
				CSize szPage = page.GetSize(m_nRotate);

				page.szDisplay = CalcPageSize(szPage, page.info.nDPI);
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

		// Center pages vertically
		for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		{
			Page& page = m_pages[nPage];

			if (page.rcDisplay.Width() < m_szDisplay.cx)
				page.ptOffset.x = (m_szDisplay.cx - page.rcDisplay.Width())/2;
			page.ptOffset.Offset(1, 0);

			page.rcDisplay.OffsetRect(page.ptOffset.x, 0);
		}

		if (m_szDisplay.cy < rcClient.Height())
		{
			int nOffset = (rcClient.Height() - m_szDisplay.cy) / 2;
			for (int nPage = 0; nPage < m_nPageCount; ++nPage)
			{
				Page& page = m_pages[nPage];
				page.rcDisplay.OffsetRect(0, nOffset);
				page.ptOffset.Offset(0, nOffset);
			}

			m_pages[0].rcDisplay.top = 0;
			m_pages.back().rcDisplay.bottom = rcClient.Height();
			m_szDisplay.cy = rcClient.Height();
		}

		if (updateType == TOP)
		{
			ScrollToPositionNoRepaint(m_pages[nAnchorPage].ptOffset + ptAnchorOffset);
		}
		else if (updateType == BOTTOM)
		{
			ScrollToPositionNoRepaint(m_pages[nAnchorPage].ptOffset + ptAnchorOffset - rcClient.Size());
		}

		if (updateType != RECALC)
		{
			UpdateVisiblePages();
		}
	}

	m_bInsideUpdateView = false;
}

void CDjVuView::UpdatePagesCache()
{
	ASSERT(m_nLayout == SinglePage);

	m_pRenderThread->PauseJobs();

	for (int nDiff = m_nPageCount; nDiff >= 0; --nDiff)
	{
		if (m_nPage - nDiff >= 0)
			UpdatePageCacheSingle(m_nPage - nDiff);
		if (m_nPage + nDiff < m_nPageCount && nDiff != 0)
			UpdatePageCacheSingle(m_nPage + nDiff);
	}

	m_pRenderThread->ResumeJobs();
}

void CDjVuView::UpdatePageCache(int nPage, const CRect& rcClient)
{
	// Pages visible on screen are put to the front of the rendering queue.
	// Pages which are within 2 screens from the view are put to the back
	// of the rendering queue.
	// Pages which are within 10 screens from the view are put to the back
	// of the decoding queue.

	int nTop = GetScrollPos(SB_VERT);
	Page& page = m_pages[nPage];

	if (page.rcDisplay.top < nTop + 3*rcClient.Height() &&
		page.rcDisplay.bottom > nTop - 2*rcClient.Height())
	{
		if (!page.bInfoLoaded)
		{
			m_pRenderThread->AddDecodeJob(nPage);
		}
		else if (page.pBitmap == NULL || page.szDisplay != page.pBitmap->GetSize())
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

		if (page.rcDisplay.top < nTop + 11*rcClient.Height() &&
			page.rcDisplay.bottom > nTop - 10*rcClient.Height())
		{
			m_pRenderThread->AddDecodeJob(nPage);
		}
		else if (!page.bInfoLoaded)
		{
			m_pRenderThread->AddReadInfoJob(nPage);
		}
		else if (GetDocument()->IsPageCached(nPage))
		{
			m_pRenderThread->AddCleanupJob(nPage);
		}
	}
}

void CDjVuView::UpdatePageCacheSingle(int nPage)
{
	// Current page and adjacent are rendered, next +- 9 pages are decoded.

	Page& page = m_pages[nPage];
	long nPageSize = page.szDisplay.cx * page.szDisplay.cy;

	if (!page.bInfoLoaded)
		return;

	if (nPageSize < 4000000 && abs(nPage - m_nPage) <= 2 ||
			abs(nPage - m_nPage) <= 1)
	{
		if (!page.bInfoLoaded)
		{
			m_pRenderThread->AddDecodeJob(nPage);
		}
		else if (page.pBitmap == NULL || page.szDisplay != page.pBitmap->GetSize())
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

		if (abs(nPage - m_nPage) <= 10)
		{
			m_pRenderThread->AddDecodeJob(nPage);
		}
		else if (!page.bInfoLoaded)
		{
			m_pRenderThread->AddReadInfoJob(nPage);
		}
		else
		{
			m_pRenderThread->AddCleanupJob(nPage);
		}
	}
}

void CDjVuView::UpdateVisiblePages()
{
	CRect rcClient;
	GetClientRect(rcClient);
	int nTop = GetScrollPos(SB_VERT);

	CalcTopPage();

	m_pRenderThread->PauseJobs();

	int nBottomPage = m_nPage + 1;
	while (nBottomPage < m_nPageCount &&
		m_pages[nBottomPage].rcDisplay.top < nTop + rcClient.Height())
	{
		++nBottomPage;
	}

	for (int nDiff = m_nPageCount; nDiff >= 1; --nDiff)
	{
		if (m_nPage - nDiff >= 0)
			UpdatePageCache(m_nPage - nDiff, rcClient);
		if (nBottomPage + nDiff - 1 < m_nPageCount)
			UpdatePageCache(nBottomPage + nDiff - 1, rcClient);
	}

	int nLastPage = m_nPage;
	int nMaxSize = -1;
	for (int nPage = nBottomPage - 1; nPage >= m_nPage; --nPage)
	{
		UpdatePageCache(nPage, rcClient);

		int nSize = min(nTop + rcClient.Height(), m_pages[nPage].rcDisplay.bottom) -
			max(nTop, m_pages[nPage].rcDisplay.top);
		if (nSize > nMaxSize)
		{
			nLastPage = nPage;
			nMaxSize = nSize;
		}
	}

	// Push to the front of the queue a page with the largest visible area.
	UpdatePageCache(nLastPage, rcClient);

	m_pRenderThread->ResumeJobs();
}

CSize CDjVuView::CalcPageSize(const CSize& szPage, int nDPI)
{
	CSize szDisplay(0, 0);
	if (szPage.cx <= 0 || szPage.cy <= 0)
		return szDisplay;

	CSize szShadow(c_nPageShadow + 1, c_nPageShadow + 1);

	CRect rcClient;
	GetClientRect(rcClient);

	switch (m_nZoomType)
	{
	case ZoomFitWidth:
		szDisplay = rcClient.Size() - szShadow;
		szDisplay.cy = szDisplay.cx * szPage.cy / szPage.cx;
		break;

	case ZoomFitHeight:
		szDisplay = rcClient.Size() - szShadow;
		szDisplay.cx = szDisplay.cy * szPage.cx / szPage.cy;
		break;

	case ZoomFitPage:
		szDisplay = rcClient.Size() - szShadow;
		szDisplay.cx = szDisplay.cy * szPage.cx / szPage.cy;

		if (szDisplay.cx > rcClient.Width() - szShadow.cx)
		{
			szDisplay.cx = rcClient.Width() - szShadow.cx;
			szDisplay.cy = szDisplay.cx * szPage.cy / szPage.cx;
		}
		break;

	case ZoomActualSize:
		szDisplay = szPage;
		break;

	case ZoomStretch:
		szDisplay = rcClient.Size() - szShadow;
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

void CDjVuView::UpdatePageSize(int nPage)
{
	Page& page = m_pages[nPage];
	CSize szPage = page.GetSize(m_nRotate);

	page.ptOffset = CPoint(0, 0);
	page.szDisplay = CalcPageSize(szPage, page.info.nDPI);

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
	int nPage = m_nPage + 1;

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
	int nPage = m_nPage - 1;

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
		GetMainFrame()->UpdatePageCombo(GetCurrentPage());
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
					OnViewNextpage();
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
		GetMainFrame()->UpdatePageCombo(GetCurrentPage());
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

	const Page& page = m_pages[m_nPage];
	CSize szPage = page.GetSize(m_nRotate);

	if (szPage.cx == 0 || szPage.cy == 0)
		return 100.0;

	// Calculate zoom from display area size
	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	int nLogPixels = dcScreen.GetDeviceCaps(LOGPIXELSX);

	return 100.0*page.szDisplay.cx*page.info.nDPI/(szPage.cx*nLogPixels);
}

void CDjVuView::OnViewLayout(UINT nID)
{
	int nPage = GetCurrentPage();
	int nOffset = GetScrollPos(SB_VERT) - m_pages[nPage].ptOffset.y;

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
		if (m_nLayout == SinglePage)
		{
			RenderPage(nPage);
		}
		else
		{
			UpdateView();
			UpdatePageSizes(m_pages[nPage].ptOffset.y);

			if (nOffset < 0)
			{
				CRect rcClient;
				GetClientRect(rcClient);

				int nTop = m_pages[nPage].ptOffset.y + nOffset;
				int nBottom = min(m_szDisplay.cy, nTop + rcClient.Height());

				UpdatePagesFromBottom(nBottom - rcClient.Height() + nOffset, nBottom);
			}
		}

		int nTop = m_pages[nPage].ptOffset.y + nOffset;
		OnScrollBy(CPoint(GetScrollPos(SB_HORZ), nTop) - GetScrollPosition());

		if (m_nLayout == Continuous)
		{
			UpdateVisiblePages();
			GetMainFrame()->UpdatePageCombo(GetCurrentPage());
		}
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

	if (m_nLayout == Continuous)
		UpdatePageSizes(GetScrollPos(SB_VERT));

	GetMainFrame()->UpdatePageCombo(GetCurrentPage());
}

void CDjVuView::OnRotateRight()
{
	m_nRotate = (m_nRotate + 3) % 4;

	DeleteBitmaps();
	UpdateView();

	if (m_nLayout == Continuous)
		UpdatePageSizes(GetScrollPos(SB_VERT));

	GetMainFrame()->UpdatePageCombo(GetCurrentPage());
}

void CDjVuView::OnRotate180()
{
	m_nRotate = (m_nRotate + 2) % 4;

	DeleteBitmaps();
	UpdateView();

	if (m_nLayout == Continuous)
		UpdatePageSizes(GetScrollPos(SB_VERT));

	GetMainFrame()->UpdatePageCombo(GetCurrentPage());
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

		OnScrollBy(CPoint(GetScrollPos(SB_HORZ), 0) - GetScrollPosition());
		UpdateVisiblePages();
		GetMainFrame()->UpdatePageCombo(GetCurrentPage());
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
		GetMainFrame()->UpdatePageCombo(GetCurrentPage());
	}
}

void CDjVuView::OnSetFocus(CWnd* pOldWnd)
{
	CScrollView::OnSetFocus(pOldWnd);

	GetMainFrame()->UpdatePageCombo(GetCurrentPage());
	GetMainFrame()->UpdateZoomCombo(m_nZoomType, m_fZoom);

	m_nClickCount = 0;
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
	GetMainFrame()->UpdatePageCombo(GetCurrentPage());
}

BOOL CDjVuView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (hCursorHand == NULL)
		hCursorHand = AfxGetApp()->LoadCursor(IDC_CURSOR_HAND);
	if (hCursorDrag == NULL)
		hCursorDrag = AfxGetApp()->LoadCursor(IDC_CURSOR_DRAG);
	if (hCursorLink == NULL)
		hCursorLink = ::LoadCursor(0, IDC_HAND);

	CRect rcClient;
	GetClientRect(rcClient);

	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	int nPage = GetPageFromPoint(ptCursor);
	if (nPage != -1 && m_pages[nPage].pAnt != NULL)
	{
		GP<DjVuANT> pAnt = m_pages[nPage].pAnt;
		for (GPosition pos = pAnt->map_areas; pos; ++pos)
		{
			GP<GMapArea> pArea = pAnt->map_areas[pos];
			CRect rcArea = TranslatePageRect(nPage, pArea->get_bound_rect());
			rcArea.OffsetRect(-GetDeviceScrollPosition());

			if (rcArea.PtInRect(ptCursor))
			{
				SetCursor(hCursorLink);
				return true;
			}
		}
	}


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

	m_bClick = true;
	++m_nClickCount;
	::GetCursorPos(&m_ptClick);

	if (m_pActiveLink != NULL)
	{
		SetCapture();
		m_bDragging = true;
	}
	else if (m_totalDev.cx > rcClient.Width() || m_totalDev.cy > rcClient.Height())
	{
		UpdateActiveHyperlink(CPoint(-1, -1));

		SetCapture();
		m_bDragging = true;

		m_nStartPage = m_nPage;
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

	if (m_pActiveLink != NULL)
	{
		GP<GMapArea> pClickedLink = m_pActiveLink;
		UpdateActiveHyperlink(point);

		if (pClickedLink == m_pActiveLink)
		{
			GUTF8String url = m_pActiveLink->url;
			if (url.length() > 0)
				GoToURL(url, m_nLinkPage);
		}
	}

	UpdateActiveHyperlink(point);

	if (m_bClick)
	{
		m_bClick = false;

		if (m_nClickCount > 1)
		{
			ClearSelection();
		}
	}

	CScrollView::OnLButtonUp(nFlags, point);
}

void CDjVuView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bClick)
	{
		CPoint ptCursor;
		::GetCursorPos(&ptCursor);
		if (m_ptClick != ptCursor)
			m_bClick = false;
	}

	if (m_bDragging)
	{
		if (m_pActiveLink != NULL)
			return;

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

		if (ptScroll != CPoint(0, 0))
			m_bClick = false;

		OnScrollBy(ptScroll);

		if (m_nLayout == Continuous)
		{
			UpdateVisiblePages();
			GetMainFrame()->UpdatePageCombo(GetCurrentPage());
		}

		return;
	}

	UpdateActiveHyperlink(point);

	CScrollView::OnMouseMove(nFlags, point);
}

void CDjVuView::OnFilePrint()
{
	CPrintDlg dlg(GetDocument(), GetCurrentPage(), m_nRotate);
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
		point += GetScrollPosition();

		int nPage = m_nPage;
		if (nPage == 0)
		{
			int nTop = m_pages[nPage].ptOffset.y - 1;
			if (point.y < nTop)
				return -1;
		}

		while (nPage < m_nPageCount - 1 &&
				point.y >= m_pages[nPage].rcDisplay.bottom)
			++nPage;

		if (nPage == m_nPageCount - 1)
		{
			int nBottom = m_pages[nPage].ptOffset.y + m_pages[nPage].szDisplay.cy
					+ c_nPageGap - 1;
			if (point.y >= nBottom)
				return -1;
		}

		return nPage;
	}
}

void CDjVuView::OnContextMenu(CWnd* pWnd, CPoint point)
{
#ifndef ELIBRA_READER
	if (point.x < 0 || point.y < 0)
		point = CPoint(0, 0);

	CRect rcClient;
	GetClientRect(rcClient);
	ClientToScreen(rcClient);

	if (!rcClient.PtInRect(point))
	{
		CScrollView::OnContextMenu(pWnd, point);
		return;
	}

	ScreenToClient(&point);
	m_nClickedPage = GetPageFromPoint(point);
	if (m_nClickedPage == -1)
		return;

	CMenu menu;
	menu.LoadMenu(IDR_POPUP);

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	ClientToScreen(&point);
	pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y,
		GetMainFrame());
#endif
}

void CDjVuView::OnPageInformation()
{
	if (m_nClickedPage == -1)
		return;

	GP<DjVuImage> pImage = GetDocument()->GetPage(m_nClickedPage);
	if (pImage == NULL)
	{
		AfxMessageBox(_T("Error decoding page"), MB_ICONERROR | MB_OK);
		return;
	}

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
		else if (strLine.Find(_T("DjVuFile.JPEG_bg1")) == 0)
		{
			strLine = strLine.Mid(17);
			_stscanf(strLine, _T("%d%d%d%lf%s"), &nWidth, &nHeight, &nDPI, &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tJPEG background (%dx%d, %d dpi)."),
				fSize, szName, nWidth, nHeight, nDPI);
		}
		else if (strLine.Find(_T("DjVuFile.JPEG_bg2")) == 0)
		{
			strLine = strLine.Mid(17);
			_stscanf(strLine, _T("%lf%s"), &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tJPEG background (unimplemented)."),
				fSize, szName);
		}
		else if (strLine.Find(_T("DjVuFile.JPEG_fg1")) == 0)
		{
			strLine = strLine.Mid(17);
			_stscanf(strLine, _T("%d%d%d%lf%s"), &nWidth, &nHeight, &nDPI, &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tJPEG foreground colors (%dx%d, %d dpi)."),
				fSize, szName, nWidth, nHeight, nDPI);
		}
		else if (strLine.Find(_T("DjVuFile.JPEG_fg2")) == 0)
		{
			strLine = strLine.Mid(17);
			_stscanf(strLine, _T("%lf%s"), &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tJPEG foreground colors (unimplemented)."),
				fSize, szName);
		}
		else if (strLine.Find(_T("DjVuFile.JPEG2K_bg")) == 0)
		{
			strLine = strLine.Mid(17);
			_stscanf(strLine, _T("%lf%s"), &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tJPEG-2000 background (unimplemented)."),
				fSize, szName);
		}
		else if (strLine.Find(_T("DjVuFile.JPEG2K_fg")) == 0)
		{
			strLine = strLine.Mid(17);
			_stscanf(strLine, _T("%lf%s"), &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tJPEG-2000 foreground colors (unimplemented)."),
				fSize, szName);
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
		else if (strLine.Find(_T("DjVuFile.unrecog_chunk")) == 0)
		{
			strLine = strLine.Mid(22);
			_stscanf(strLine, _T("%lf%s"), &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tUnrecognized chunk."),
				fSize, szName);
		}
		else if (strLine.Find(_T("DjVuFile.nav_dir")) == 0)
		{
			strLine = strLine.Mid(16);
			_stscanf(strLine, _T("%lf%s"), &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tNavigation directory (obsolete)."),
				fSize, szName);
		}
		else if (strLine.Find(_T("DjVuFile.color_import1")) == 0)
		{
			strLine = strLine.Mid(22);
			_stscanf(strLine, _T("%d%d%d%lf%s"), &nWidth, &nHeight, &nDPI, &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tLINK Color Import (%dx%d, %d dpi)."),
				fSize, szName, nWidth, nHeight, nDPI);
		}
		else if (strLine.Find(_T("DjVuFile.color_import2")) == 0)
		{
			strLine = strLine.Mid(22);
			_stscanf(strLine, _T("%lf%s"), &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tLINK Color Import (unimplemented)."),
				fSize, szName);
		}
		else if (strLine.Find(_T("DjVuFile.anno1")) == 0)
		{
			strLine = strLine.Mid(14);
			_stscanf(strLine, _T("%lf%s"), &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tAnnotations (bundled)."),
				fSize, szName);
		}
		else if (strLine.Find(_T("DjVuFile.anno2")) == 0)
		{
			strLine = strLine.Mid(14);
			_stscanf(strLine, _T("%lf%s"), &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tAnnotations (hyperlinks, etc.)."),
				fSize, szName);
		}
		else
			strFormatted = strLine;

		strInfo += (!strInfo.IsEmpty() ? _T("\n") : _T("")) + strFormatted;
	}

	AfxMessageBox(strInfo, MB_ICONINFORMATION | MB_OK);
}

LRESULT CDjVuView::OnRenderFinished(WPARAM wParam, LPARAM lParam)
{
	int nPage = (int)wParam;
	OnPageDecoded(nPage);

	Page& page = m_pages[nPage];
	CDIB* pBitmap = reinterpret_cast<CDIB*>(lParam);

	if (page.pBitmap != NULL && page.szDisplay == page.pBitmap->GetSize())
	{
		// Bitmap is too old, ignore it
		delete pBitmap;
		return 0;
	}

	page.DeleteBitmap();
	page.pBitmap = pBitmap;
	m_evtRendered.SetEvent();
	
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
	PageInfo info = GetDocument()->GetPageInfo(nPage);

	if (m_pages.size() == 0)
	{
		m_nPageCount = GetDocument()->GetPageCount();
		m_pages.resize(m_nPageCount);
	}

	Page& page = m_pages[nPage];
	bool bHadInfo = page.bInfoLoaded;

	page.Init(info);
	if (!bHadInfo && m_nLayout == Continuous)
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
			{
				UpdateVisiblePages();
				GetMainFrame()->UpdatePageCombo(GetCurrentPage());

				UpdateWindow();
			}

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
				nScroll = -m_lineDev.cy;
				break;
			case SB_LINEDOWN:
				nScroll = m_lineDev.cy;
				break;
			case SB_PAGEUP:
				nScroll = -m_pageDev.cy;
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
	GetMainFrame()->UpdatePageCombo(GetCurrentPage());

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

	if (!m_pages[nPage].bInfoLoaded)
	{
		Page& page = m_pages[nPage];

		PageInfo info = GetDocument()->GetPageInfo(nPage);
		page.Init(info);

		page.szDisplay = CalcPageSize(page.GetSize(m_nRotate), page.info.nDPI);
		bUpdateView = true;
	}

	while (nPage > 0 && nTop < m_pages[nPage].ptOffset.y)
	{
		--nPage;
		if (!m_pages[nPage].bInfoLoaded)
		{
			Page& page = m_pages[nPage];

			PageInfo info = GetDocument()->GetPageInfo(nPage);
			page.Init(info);

			page.szDisplay = CalcPageSize(page.GetSize(m_nRotate), page.info.nDPI);
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

	if (!m_pages[nPage].bInfoLoaded)
	{
		Page& page = m_pages[nPage];

		PageInfo info = GetDocument()->GetPageInfo(nPage);
		page.Init(info);

		page.szDisplay = CalcPageSize(page.GetSize(m_nRotate), page.info.nDPI);
		bUpdateView = true;
	}

	while (nPage < m_nPageCount - 1 &&
		   nBottom > m_pages[nPage].ptOffset.y + m_pages[nPage].szDisplay.cy + c_nPageGap - 1)
	{
		++nPage;
		if (!m_pages[nPage].bInfoLoaded)
		{
			Page& page = m_pages[nPage];

			PageInfo info = GetDocument()->GetPageInfo(nPage);
			page.Init(info);

			page.szDisplay = CalcPageSize(page.GetSize(m_nRotate), page.info.nDPI);
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

void CDjVuView::CalcTopPage()
{
	if (m_nLayout == Continuous)
	{
		int nTop = GetScrollPos(SB_VERT);

		int nPage = 0;
		while (nPage < m_nPageCount - 1 &&
				nTop >= m_pages[nPage].rcDisplay.bottom)
			++nPage;

		m_nPage = nPage;
	}
}

int CDjVuView::GetCurrentPage() const
{
	if (m_nLayout == SinglePage)
		return m_nPage;

	CRect rcClient;
	GetClientRect(rcClient);

	int nPage = m_nPage;
	const Page& page = m_pages[nPage];
	int nHeight = min(page.szDisplay.cy, rcClient.Height());

	if (page.rcDisplay.bottom - GetScrollPos(SB_VERT) < 0.3*nHeight &&
		nPage < m_nPageCount)
	{
		++nPage;
	}

	return nPage;
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

	bGotScrollLines = true;

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

	if (uCachedScrollLines == 0)
	{
		::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uCachedScrollLines, 0);

		if (uCachedScrollLines == 0)
		{
			// couldn't use the window -- try system settings
			uCachedScrollLines = 3; // reasonable default
		}
	}

	return uCachedScrollLines;
}

BOOL CDjVuView::OnMouseWheel(UINT nFlags, short zDelta, CPoint /*point*/)
{
	// we don't handle anything but scrolling
	if ((nFlags & MK_CONTROL) != 0)
		return false;

	BOOL bHasHorzBar, bHasVertBar;
	CheckScrollBars(bHasHorzBar, bHasVertBar);

	BOOL bResult = false;
	UINT uWheelScrollLines = GetMouseScrollLines();
	int nToScroll = ::MulDiv(-zDelta, uWheelScrollLines, WHEEL_DELTA);
	int nDisplacement;

	bool bScrollPages = true;

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

		bResult = OnScrollBy(CSize(0, nDisplacement));
		bScrollPages = false;

		UpdateVisiblePages();
		GetMainFrame()->UpdatePageCombo(GetCurrentPage());
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

		bResult = OnScrollBy(CSize(nDisplacement, 0));
		bScrollPages = false;
	}

	if (!bResult && bScrollPages)
	{
		if (zDelta < 0 && m_nPage < m_nPageCount - 1 &&
			m_nLayout == SinglePage)
		{
			OnViewNextpage();
		}
		else if (zDelta > 0 && m_nPage > 0 &&
			m_nLayout == SinglePage)
		{
			CPoint ptScroll = GetScrollPosition();
			OnViewPreviouspage();
			OnScrollBy(CPoint(ptScroll.x, m_szDisplay.cy) - GetScrollPosition());
			Invalidate();
		}
		return true;
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

	m_nMapMode = MM_TEXT;
	m_totalLog = szTotal;

	m_totalDev = m_totalLog;
	m_pageDev = szPage;
	m_lineDev = szLine;
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);

	if (m_hWnd != NULL)
	{
		// window has been created, invalidate now
		UpdateBarsNoRepaint();
	}
}

void CDjVuView::SetScrollSizes(const CSize& szTotal, const CSize& szPage, const CSize& szLine)
{
	SetScrollSizesNoRepaint(szTotal, szPage, szLine);

	if (m_hWnd != NULL)
	{
		// window has been created, invalidate now
		UpdateBars();
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

	BOOL bHasHorzBar, bHasVertBar;
	CheckScrollBars(bHasHorzBar, bHasVertBar);

	if (!bHasHorzBar)
		xMax = 0;
	if (!bHasVertBar)
		yMax = 0;

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

void CDjVuView::OnExportPage()
{
	CString strFileName;
	strFileName.Format(_T("p%04d.bmp"), m_nClickedPage);

	const TCHAR szFilter[] = "Bitmap Files (*.bmp)|*.bmp|All Files (*.*)|*.*||";

	CFileDialog dlg(false, "bmp", strFileName, OFN_OVERWRITEPROMPT |
		OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST, szFilter);

	if (dlg.DoModal() != IDOK)
		return;

	CWaitCursor wait;
	strFileName = dlg.GetPathName();

	GP<DjVuImage> pImage = GetDocument()->GetPage(m_nClickedPage);
	if (pImage == NULL)
	{
		AfxMessageBox(_T("Error decoding page"), MB_ICONERROR | MB_OK);
		return;
	}

	pImage->set_rotate(m_nRotate);

	GRect rect(0, 0, pImage->get_width(), pImage->get_height());
	CDIB* pBitmap = CRenderThread::Render(pImage, rect, rect);

	if (pBitmap != NULL && pBitmap->m_hObject != NULL)
	{
		CDIB* pNewBitmap = pBitmap->ReduceColors();
		if (pNewBitmap != NULL && pNewBitmap->m_hObject != NULL)
		{
			delete pBitmap;
			pBitmap = pNewBitmap;
		}
		else
		{
			delete pNewBitmap;
		}

		pBitmap->Save(strFileName);
	}

	delete pBitmap;
}

void CDjVuView::OnFindString()
{
	CWaitCursor wait;

	CFindDlg* pDlg = GetMainFrame()->m_pFindDlg;
	ASSERT(pDlg != NULL);
	GUTF8String strFind = GNativeString(pDlg->m_strFind).NativeToUTF8();

	int nStartPage, nStartPos;
	bool bHasSelection = false;

	// Check if there is a selection
	for (int i = 0; i < m_nPageCount; ++i)
	{
		Page& page = m_pages[i];
		if (page.nSelEnd != -1)
		{
			bHasSelection = true;
			nStartPage = i;
			nStartPos = page.nSelEnd;
			break;
		}
	}

	if (!bHasSelection)
	{
		nStartPage = m_nPage;
		nStartPos = 0;
	}

	int nPage = nStartPage;
	while (nPage < m_nPageCount)
	{
		Page& page = m_pages[nPage];
		if (!page.bInfoLoaded)
		{
			PageInfo info = GetDocument()->GetPageInfo(nPage);
			page.Init(info);
			UpdateView();
		}

		page.DecodeText();
		if (page.pText != NULL)
		{
			GP<DjVuTXT> pText = page.pText;

			int nPos = -1;
			if (pDlg->m_bMatchCase)
			{
				GUTF8String& text = pText->textUTF8;
				nPos = text.search(strFind, nStartPos);
			}
			else
			{
				GUTF8String textUp = pText->textUTF8.upcase();
				GUTF8String strFindUp = strFind.upcase();
				nPos = textUp.search(strFindUp, nStartPos);
			}

			if (nPos != -1)
			{
				int nSelEnd = nPos + strFind.length();
				DjVuSelection selection;
				pText->page_zone.find_zones(selection, nPos, nSelEnd);

				if (!bHasSelection && nPage == nStartPage &&
						!IsSelectionBelowTop(nPage, selection))
				{
					nStartPos = nPos + strFind.length();
					continue;
				}

				ClearSelection();

				page.nSelStart = nPos;
				page.nSelEnd = nSelEnd;
				page.selection = selection;

				InvalidatePage(nPage);

				if (!IsSelectionVisible(nPage, page.selection))
				{
					EnsureSelectionVisible(nPage, page.selection);
				}

				GetMainFrame()->SetMessageText(AFX_IDS_IDLEMESSAGE);
				return;
			}
		}

		nStartPos = 0;
		++nPage;
	}

	GetMainFrame()->HilightStatusMessage(_T("Search string not found"));
	::MessageBeep(MB_OK);
}

CRect CDjVuView::GetSelectionRect(int nPage, const DjVuSelection& selection) const
{
	CRect rcSel;
	bool bFirst = true;

	for (GPosition pos = selection; pos; ++pos)
	{
		GRect rect = selection[pos]->rect;
		CRect rcText = TranslatePageRect(nPage, rect);

		if (bFirst)
		{
			rcSel = rcText;
			bFirst = false;
		}
		else
			rcSel.UnionRect(rcSel, rcText);
	}

	return rcSel;
}

bool CDjVuView::IsSelectionBelowTop(int nPage, const DjVuSelection& selection)
{
	int nTop = GetScrollPos(SB_VERT);

	CRect rcSel = GetSelectionRect(nPage, selection);
	return (rcSel.top >= nTop);
}

bool CDjVuView::IsSelectionVisible(int nPage, const DjVuSelection& selection)
{
	CPoint ptScroll = GetScrollPosition();

	CRect rcClient;
	GetClientRect(rcClient);
	rcClient.OffsetRect(ptScroll);

	CRect rcSel = GetSelectionRect(nPage, selection);
	rcClient.IntersectRect(rcClient, rcSel);

	if (rcClient != rcSel)
		return false;

	CFindDlg* pDlg = GetMainFrame()->m_pFindDlg;
	if (pDlg != NULL && pDlg->IsWindowVisible())
	{
		CRect rcFindDlg;
		pDlg->GetWindowRect(rcFindDlg);
		ScreenToClient(rcFindDlg);
		rcFindDlg.OffsetRect(ptScroll);

		if (rcFindDlg.IntersectRect(rcFindDlg, rcSel))
			return false;
	}

	return true;
}

void CDjVuView::EnsureSelectionVisible(int nPage, const DjVuSelection& selection)
{
	if (m_nPage != nPage)
	{
		RenderPage(nPage, 1000);
	}

	CRect rcClient;
	GetClientRect(rcClient);
	CPoint ptScroll = GetScrollPosition();
	rcClient.OffsetRect(ptScroll);

	const Page& page = m_pages[nPage];
	int nHeight = min(page.szDisplay.cy, rcClient.Height());

	CRect rcSel = GetSelectionRect(nPage, selection);

	// Scroll vertically
	int nTopPos = rcClient.top;
	if (rcSel.top < rcClient.top)
	{
		nTopPos = max(page.rcDisplay.top, static_cast<int>(rcSel.top - 0.3*nHeight));
		if (nTopPos + rcClient.Height() < rcSel.bottom)
			nTopPos = rcSel.bottom - rcClient.Height();
		nTopPos = min(rcSel.top, nTopPos);
	}
	else if (rcSel.bottom > rcClient.bottom)
	{
		int nBottom = min(m_szDisplay.cy, static_cast<int>(rcSel.bottom + 0.3*nHeight));
		nTopPos = min(rcSel.top, nBottom - rcClient.Height());
	}

	rcClient.OffsetRect(0, nTopPos - ptScroll.y);

	// Further scroll the document if selection is obscured by the Find dialog
	CFindDlg* pDlg = GetMainFrame()->m_pFindDlg;
	ASSERT(pDlg != NULL);
	CRect rcFindDlg;
	pDlg->GetWindowRect(rcFindDlg);
	ScreenToClient(rcFindDlg);
	rcFindDlg.OffsetRect(0, nTopPos);

	if (rcSel.bottom > rcFindDlg.top && rcSel.top < rcFindDlg.bottom)
	{
		int nTopSpace = rcFindDlg.top - rcClient.top;
		int nBottomSpace = rcClient.bottom - rcFindDlg.bottom;
		if (nTopSpace >= rcSel.Height() || nTopSpace >= nBottomSpace && nTopSpace > 0)
		{
			nTopPos = max(0, rcSel.top - (nTopSpace - rcSel.Height())/2);
			nTopPos = min(rcSel.top, nTopPos);
		}
		else if (nBottomSpace > 0)
		{
			int nBottom = rcSel.bottom + (nBottomSpace - rcSel.Height())/2;
			nTopPos = min(rcSel.top - (rcFindDlg.bottom - rcClient.top), nBottom - rcClient.Height());
		}
	}

	CPoint ptScrollBy(0, nTopPos - GetScrollPos(SB_VERT));

	UpdatePageSizes(GetScrollPos(SB_VERT), ptScrollBy.y);
	OnScrollBy(ptScrollBy);
	UpdateVisiblePages();
	GetMainFrame()->UpdatePageCombo(GetCurrentPage());
}

CRect CDjVuView::TranslatePageRect(int nPage, GRect rect) const
{
	const Page& page = m_pages[nPage];
	CSize szPage = page.GetSize(m_nRotate);

	GRect input, output;
	if (m_nRotate != 0)
	{
		input = GRect(0, 0, szPage.cx, szPage.cy);
		output = GRect(0, 0, page.info.szPage.cx, page.info.szPage.cy);

		GRectMapper mapper;
		mapper.clear();
		mapper.set_input(input);
		mapper.set_output(output);               
		mapper.rotate(4 - m_nRotate);
		mapper.unmap(rect);
	}

	double fRatioX = (1.0*page.szDisplay.cx) / szPage.cx;
	double fRatioY = (1.0*page.szDisplay.cy) / szPage.cy;

	CRect rcResult(static_cast<int>(rect.xmin * fRatioX),
		static_cast<int>((szPage.cy - rect.ymax) * fRatioY),
		static_cast<int>(rect.xmax * fRatioX),
		static_cast<int>((szPage.cy - rect.ymin) * fRatioY));

	rcResult.OffsetRect(page.ptOffset);
	return rcResult;
}

void CDjVuView::ClearSelection()
{
	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		Page& page = m_pages[nPage];

		if (page.nSelStart != -1)
		{
			page.selection.empty();
			page.nSelStart = -1;
			page.nSelEnd = -1;
			InvalidatePage(nPage);
		}
	}
}

void CDjVuView::UpdateActiveHyperlink(CPoint point)
{
	int nPage;
	GP<GMapArea> pHyperlink = GetHyperlinkFromPoint(point, &nPage);
	if (pHyperlink != m_pActiveLink)
	{
		if (m_pActiveLink != NULL)
		{
			m_toolTip.Activate(FALSE);
			CRect rcArea = TranslatePageRect(m_nLinkPage, m_pActiveLink->get_bound_rect());
			rcArea.InflateRect(0, 0, 1, 1);
			rcArea.OffsetRect(-GetScrollPosition());
			InvalidateRect(rcArea);
		}

		m_pActiveLink = pHyperlink;
		m_nLinkPage = nPage;

		if (m_pActiveLink != NULL)
		{
			m_toolTip.Activate(TRUE);
			CRect rcArea = TranslatePageRect(nPage, m_pActiveLink->get_bound_rect());
			rcArea.InflateRect(0, 0, 1, 1);
			rcArea.OffsetRect(-GetScrollPosition());
			InvalidateRect(rcArea);
		}
	}
}

GP<GMapArea> CDjVuView::GetHyperlinkFromPoint(CPoint point, int* pnPage)
{
	int nPage = GetPageFromPoint(point);
	if (nPage == -1)
		return NULL;

	const Page& page = m_pages[nPage];

	CRect rcClient;
	GetClientRect(rcClient);

	if (!rcClient.PtInRect(point))
		return NULL;

	if (page.pAnt != NULL)
	{
		for (GPosition pos = page.pAnt->map_areas; pos; ++pos)
		{
			GP<GMapArea> pArea = page.pAnt->map_areas[pos];
			CRect rcArea = TranslatePageRect(nPage, pArea->get_bound_rect());
			rcArea.OffsetRect(-GetDeviceScrollPosition());

			if (rcArea.PtInRect(point))
			{
				if (pnPage != NULL)
					*pnPage = nPage;

				return pArea;
			}
		}
	}

	return NULL;
}

BOOL CDjVuView::OnToolTipNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pNMHDR;
	BOOL bHandledNotify = FALSE;

	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	CRect rcClient;
	GetClientRect(rcClient);

	// Make certain that the cursor is in the client rect, because the
	// mainframe also wants these messages to provide tooltips for the toolbar.
	if (!rcClient.PtInRect(ptCursor))
		return FALSE;

	if (m_pActiveLink != NULL)
	{
		CString strTip = m_pActiveLink->url;

		ASSERT(strTip.GetLength() < sizeof(pTTT->szText));
		::strcpy(pTTT->szText, strTip);
	}
	else
        pTTT->szText[0] = '\0';

	return TRUE;
}

BOOL CDjVuView::PreTranslateMessage(MSG* pMsg)
{
	// if (::IsWindow(m_toolTip.m_hWnd) && pMsg->hwnd == m_hWnd)
	if (::IsWindow(m_toolTip.m_hWnd))
		m_toolTip.RelayEvent(pMsg);

	return CScrollView::PreTranslateMessage(pMsg);
}

void CDjVuView::GoToURL(const GUTF8String& url, int nLinkPage, bool bAddToHistory)
{
	if (url[0] == '#')
	{
		int nPage = -1;

		G_TRY
		{
			char base = '\0';
			const char *s = (const char*)url + 1;
			const char *q = s;
			if (*q == '+' || *q == '-')
				base = *q++;

			GUTF8String str = q;
			if (str.is_int())
			{
				nPage = str.toInt();
				if (base == '+')
					nPage = nLinkPage + nPage;
				else if (base=='-')
					nPage = nLinkPage - nPage;
				else
					--nPage;
			}
			else
			{
				nPage = GetDocument()->GetPageFromId(s);
				if (nPage == -1)
					return;
			}
		}
		G_CATCH(ex)
		{
			ex;
		}
		G_ENDCATCH;

		if (nPage >= m_nPageCount)
			nPage = m_nPageCount - 1;
		if (nPage < 0)
			nPage = 0;

		GoToPage(nPage, nLinkPage, bAddToHistory);
		return;
	}

	// Try to open a .djvu file given by a path (relative or absolute)

	int nPos = url.search('#');
	GUTF8String strPage, strURL = url;
	if (nPos != -1)
	{
		strPage = url.substr(nPos, url.length() - nPos);
		strURL = strURL.substr(0, nPos);
	}

	CString strPathName = (const char*)strURL;
	TCHAR szDrive[_MAX_DRIVE + 1] = {0};
	TCHAR szDir[_MAX_DIR + 1] = {0};
	TCHAR szExt[_MAX_EXT + 1] = {0};
	_tsplitpath(strPathName, NULL, NULL, NULL, szExt);
	if (_tcsicmp(szExt, _T(".djvu")) == 0 || _tcsicmp(szExt, _T(".djv")) == 0)
	{
		// Check if the link leads to a local DjVu file

		if (_tcsnicmp(strPathName, _T("file:///"), 8) == 0)
			strPathName = strPathName.Mid(8);
		else if (_tcsnicmp(strPathName, _T("file://"), 7) == 0)
			strPathName = strPathName.Mid(7);

		// Try as absolute path
		CFile file;
		if (file.Open(strPathName, CFile::modeRead | CFile::shareDenyWrite))
		{
			file.Close();
			theApp.OpenDocument(strPathName, strPage);
			return;
		}

		// Try as relative path
		CString strCurrentPath = GetDocument()->GetPathName();
		_tsplitpath(strCurrentPath, szDrive, szDir, NULL, NULL);

		strPathName = CString(szDrive) + CString(szDir) + strPathName;
		if (file.Open(strPathName, CFile::modeRead | CFile::shareDenyWrite))
		{
			file.Close();
			theApp.OpenDocument(strPathName, strPage);
			return;
		}
	}

	// Open a web link
	::ShellExecute(NULL, "open", (const char*)url, NULL, NULL, SW_SHOWNORMAL);
}

void CDjVuView::GoToPage(int nPage, int nLinkPage, bool bAddToHistory)
{
	if (bAddToHistory)
	{
		if (!m_history.empty())
		{
			++m_historyPos;
			m_history.erase(m_historyPos, m_history.end());
		}

		View entry;
		entry.nPage = nLinkPage;
		if (m_history.empty() || m_history.back() != entry)
			m_history.push_back(entry);

		entry.nPage = nPage;
		m_history.push_back(entry);

		m_historyPos = m_history.end();
		--m_historyPos;
	}

	RenderPage(nPage);
}

void CDjVuView::OnViewBack()
{
	if (m_history.empty() || m_historyPos == m_history.begin())
		return;

	const View& view = *(--m_historyPos);
	RenderPage(view.nPage);
}

void CDjVuView::OnUpdateViewBack(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_history.empty() && m_historyPos != m_history.begin());
}

void CDjVuView::OnViewForward()
{
	if (m_history.empty())
		return;

	list<View>::iterator it = m_history.end();
	if (m_historyPos == m_history.end() || m_historyPos == --it)
		return;

	const View& view = *(++m_historyPos);
	RenderPage(view.nPage);
}

void CDjVuView::OnUpdateViewForward(CCmdUI* pCmdUI)
{
	if (m_history.empty())
	{
		pCmdUI->Enable(false);
	}
	else
	{
		list<View>::iterator it = m_history.end();
		pCmdUI->Enable(m_historyPos != m_history.end() && m_historyPos != --it);
	}
}
