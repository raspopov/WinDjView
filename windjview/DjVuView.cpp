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
//	http://www.gnu.org/copyleft/gpl.html

// $Id$

#include "stdafx.h"
#include "WinDjView.h"

#include "DjVuDoc.h"
#include "DjVuView.h"

#include "MainFrm.h"
#include "PrintDlg.h"
#include "Drawing.h"
#include "ProgressDlg.h"
#include "ZoomDlg.h"
#include "FindDlg.h"
#include "ChildFrm.h"
#include "SearchResultsView.h"
#include "FullscreenWnd.h"

#include "RenderThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDjVuView

HCURSOR CDjVuView::hCursorHand = NULL;
HCURSOR CDjVuView::hCursorDrag = NULL;
HCURSOR CDjVuView::hCursorLink = NULL;
HCURSOR CDjVuView::hCursorText = NULL;

const int c_nPageGap = 8;
const int c_nPageShadow = 3;

IMPLEMENT_DYNCREATE(CDjVuView, CMyScrollView)

BEGIN_MESSAGE_MAP(CDjVuView, CMyScrollView)
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
	ON_WM_RBUTTONDOWN()
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
	ON_COMMAND(ID_FIND_ALL, OnFindAll)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNeedText)
	ON_COMMAND(ID_ZOOM_IN, OnViewZoomIn)
	ON_COMMAND(ID_ZOOM_OUT, OnViewZoomOut)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_IN, OnUpdateViewZoomIn)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_OUT, OnUpdateViewZoomOut)
	ON_COMMAND_RANGE(ID_MODE_DRAG, ID_MODE_SELECT, OnChangeMode)
	ON_UPDATE_COMMAND_UI_RANGE(ID_MODE_DRAG, ID_MODE_SELECT, OnUpdateMode)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_FILE_EXPORT_TEXT, OnFileExportText)
	ON_UPDATE_COMMAND_UI(ID_FILE_EXPORT_TEXT, OnUpdateFileExportText)
	ON_COMMAND_RANGE(ID_DISPLAY_COLOR, ID_DISPLAY_FOREGROUND, OnViewDisplay)
	ON_UPDATE_COMMAND_UI_RANGE(ID_DISPLAY_COLOR, ID_DISPLAY_FOREGROUND, OnUpdateViewDisplay)
	ON_COMMAND(ID_VIEW_FULLSCREEN, OnViewFullscreen)
	ON_WM_MOUSEACTIVATE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

// CDjVuView construction/destruction

CDjVuView::CDjVuView()
	: m_nPage(-1), m_nPageCount(0), m_nZoomType(ZoomPercent), m_fZoom(100.0),
	  m_nLayout(SinglePage), m_nRotate(0), m_bDragging(false),
	  m_pRenderThread(NULL), m_bInsideUpdateView(false), m_bClick(false),
	  m_evtRendered(false, true), m_nPendingPage(-1), m_nClickedPage(-1),
	  m_nMode(Drag), m_pOffscreenBitmap(NULL), m_szOffscreen(0, 0),
	  m_bHasSelection(false), m_nDisplayMode(Color), m_bShowAllLinks(false)
{
}

CDjVuView::~CDjVuView()
{
}

BOOL CDjVuView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CMyScrollView::PreCreateWindow(cs);
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
	// explicitly.
	pDC->SetViewportOrg(CPoint(0, 0));

	CRect rcClient;
	GetClientRect(rcClient);
	if (m_pOffscreenBitmap == NULL
			|| m_szOffscreen.cx < rcClient.Width() || m_szOffscreen.cy < rcClient.Height())
	{
		m_szOffscreen.cx = max(m_szOffscreen.cx, static_cast<int>(rcClient.Width()*1.1));
		m_szOffscreen.cy = max(m_szOffscreen.cy, static_cast<int>(rcClient.Height()*1.1));

		delete m_pOffscreenBitmap;
		m_pOffscreenBitmap = new CBitmap();
		m_pOffscreenBitmap->CreateCompatibleBitmap(pDC, m_szOffscreen.cx, m_szOffscreen.cy);
	}

	CDC dcOffscreen;
	dcOffscreen.CreateCompatibleDC(pDC);
	dcOffscreen.SetViewportOrg(CPoint(0, 0));

	CBitmap* pOldBitmap = dcOffscreen.SelectObject(m_pOffscreenBitmap);

	CRect rcClip;
	pDC->GetClipBox(rcClip);
	rcClip.IntersectRect(rcClip, rcClient);

	dcOffscreen.IntersectClipRect(rcClip);

	CPoint ptScrollPos = GetDeviceScrollPosition();
	rcClip.OffsetRect(ptScrollPos);

	if (m_nLayout == SinglePage)
	{
		DrawPage(&dcOffscreen, m_nPage);
	}
	else if (m_nLayout == Continuous)
	{
		for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		{
			Page& page = m_pages[nPage];
			if (page.rcDisplay.top >= rcClip.bottom ||
					page.rcDisplay.bottom <= rcClip.top)
				continue;

			DrawPage(&dcOffscreen, nPage);
		}
	}

	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		Page& page = m_pages[nPage];
		if (m_nLayout == SinglePage)
		{
			if (nPage != m_nPage)
				continue;
		}
		else if (page.rcDisplay.top >= rcClip.bottom ||
				page.rcDisplay.bottom <= rcClip.top)
		{
			continue;
		}

		if (page.pBitmap == NULL || page.pBitmap->m_hObject == NULL)
			continue;

		// Draw hyperlinks
		if (page.pAnt != NULL)
		{
			for (GPosition pos = page.pAnt->map_areas; pos; ++pos)
			{
				GP<GMapArea> pArea = page.pAnt->map_areas[pos];
				DrawMapArea(&dcOffscreen, pArea, nPage, !!(pArea == m_pActiveLink) || m_bShowAllLinks);
			}
		}

		// Draw selection
		for (GPosition pos = page.selection; pos; ++pos)
		{
			GRect rect = page.selection[pos]->rect;
			CRect rcText = TranslatePageRect(nPage, rect);

			rcText.OffsetRect(-ptScrollPos);
			dcOffscreen.InvertRect(rcText);
		}
	}

	rcClip.OffsetRect(-ptScrollPos);
	pDC->BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(),
			&dcOffscreen, rcClip.left, rcClip.top, SRCCOPY);
	dcOffscreen.SelectObject(pOldBitmap);
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

void CDjVuView::DrawPage(CDC* pDC, int nPage)
{
	Page& page = m_pages[nPage];

	COLORREF clrFrame = ::GetSysColor(COLOR_WINDOWFRAME);
	COLORREF clrShadow = ::GetSysColor(COLOR_BTNSHADOW);
	COLORREF clrWindow = ::GetSysColor(COLOR_WINDOW);

	CPoint ptScrollPos = GetDeviceScrollPosition();

	CRect rcClip;
	pDC->GetClipBox(rcClip);
	rcClip.OffsetRect(ptScrollPos);

	if (m_nLayout == Continuous)
	{
		rcClip.top = max(rcClip.top, page.rcDisplay.top);
		rcClip.bottom = min(rcClip.bottom, page.rcDisplay.bottom);
	}

	if (rcClip.IsRectEmpty())
		return;

	if (page.szDisplay.cx <= 0 || page.szDisplay.cy <= 0)
	{
		pDC->FillSolidRect(rcClip - ptScrollPos, clrWindow);
		return;
	}

	if (page.pBitmap == NULL || page.pBitmap->m_hObject == NULL)
	{
		// Draw white rect
		CRect rcPage(page.ptOffset, page.szDisplay);
		pDC->FillSolidRect(rcPage - ptScrollPos, clrWindow);

		//CString strPage;
		//strPage.Format(_T("%d"), nPage + 1);
		//pDC->DrawText(strPage, rcPage - ptScrollPos,
		//		DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
	else if (page.pBitmap->GetSize() == page.szDisplay)
	{
		// Draw offscreen bitmap
		CPoint ptPartOffset(max(rcClip.left - page.ptOffset.x, 0),
			max(rcClip.top - page.ptOffset.y, 0));

		CSize szPageClip = page.szDisplay - ptPartOffset;
		CSize szPart(min(rcClip.Width(), szPageClip.cx), min(rcClip.Height(), szPageClip.cy));

		if (szPart.cx > 0 && szPart.cy > 0)
		{
			CRect rcPart(ptPartOffset, szPart);
			page.pBitmap->DrawDC(pDC,
				page.ptOffset + ptPartOffset - ptScrollPos, rcPart);
		}
	}
	else
	{
		// Offscreen bitmap has yet wrong size, stretch it
		page.pBitmap->Draw(pDC, page.ptOffset - ptScrollPos, page.szDisplay);
	}

	// Draw border
	CRect rcBorder(page.ptOffset, page.szDisplay);
	rcBorder.InflateRect(1, 1);
	FrameRect(pDC, rcBorder - ptScrollPos, clrFrame);

	if (m_nMode != Fullscreen)
	{
		// Draw shadow
		CRect rcWhiteBar(CPoint(rcBorder.left, rcBorder.bottom),
				CSize(c_nPageShadow + 1, c_nPageShadow));
		pDC->FillSolidRect(rcWhiteBar - ptScrollPos, clrWindow);
		rcWhiteBar = CRect(CPoint(rcBorder.right, rcBorder.top),
				CSize(c_nPageShadow, c_nPageShadow + 1));
		pDC->FillSolidRect(rcWhiteBar - ptScrollPos, clrWindow);

		CRect rcShadow(CPoint(rcBorder.left + c_nPageShadow + 1, rcBorder.bottom),
				CSize(rcBorder.Width() - 1, c_nPageShadow));
		pDC->FillSolidRect(rcShadow - ptScrollPos, clrShadow);
		rcShadow = CRect(CPoint(rcBorder.right, rcBorder.top + c_nPageShadow + 1),
				CSize(c_nPageShadow, rcBorder.Height() - 1));
		pDC->FillSolidRect(rcShadow - ptScrollPos, clrShadow);

		rcBorder.InflateRect(0, 0, c_nPageShadow, c_nPageShadow);
	}

	// Fill everything else with backgroundColor
	int nSaveDC = pDC->SaveDC();
	pDC->ExcludeClipRect(rcBorder - ptScrollPos);
	pDC->FillSolidRect(rcClip - ptScrollPos, m_nMode != Fullscreen ? clrWindow : RGB(0, 0, 0));
	pDC->RestoreDC(nSaveDC);
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
	CMyScrollView::AssertValid();
}

void CDjVuView::Dump(CDumpContext& dc) const
{
	CMyScrollView::Dump(dc);
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
	CMyScrollView::OnInitialUpdate();

	if (m_toolTip.Create(this, TTS_ALWAYSTIP) && m_toolTip.AddTool(this))
	{
		m_toolTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);
		m_toolTip.Activate(false);
	}

	m_nPageCount = GetDocument()->GetPageCount();
	m_pages.resize(m_nPageCount);

	if (m_nMode != Fullscreen)
	{
		m_nZoomType = CAppSettings::nDefaultZoomType;
		m_fZoom = CAppSettings::fDefaultZoom;
		if (m_nZoomType == ZoomPercent)
			m_fZoom = 100.0;

		m_nLayout = CAppSettings::nDefaultLayout;
#ifndef ELIBRA_READER
		m_nMode = CAppSettings::nDefaultMode;
#endif

		m_nPage = 0;
		Page& page = m_pages[0];
		page.Init(GetDocument()->GetPageInfo(0));
		if (page.pAnt != NULL)
			ReadZoomSettings(page.pAnt);
	}

#ifndef ELIBRA_READER
	m_displaySettings = CAppSettings::displaySettings;
#endif

	m_pRenderThread = new CRenderThread(GetDocument(), this);

	UpdateView(RECALC);
	RenderPage(m_nPage);
}

void CDjVuView::ReadZoomSettings(GP<DjVuANT> pAnt)
{
	switch (pAnt->zoom)
	{
	case DjVuANT::ZOOM_STRETCH:
		m_nZoomType = ZoomStretch;
		break;

	case DjVuANT::ZOOM_ONE2ONE:
		m_nZoomType = ZoomActualSize;
		break;

	case DjVuANT::ZOOM_WIDTH:
		m_nZoomType = ZoomFitWidth;
		break;

	case DjVuANT::ZOOM_PAGE:
		m_nZoomType = ZoomFitPage;
		break;
	}

	if (pAnt->zoom > 0)
	{
		m_fZoom = pAnt->zoom;

		if (m_fZoom < 10.0)
			m_fZoom = 10.0;
		if (m_fZoom > 800.0)
			m_fZoom = 800.0;
	}
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
	else if (m_nLayout == Continuous)
	{
		UpdatePageSizes(page.ptOffset.y);

		int nUpdatedPos = page.ptOffset.y - 1;
		ScrollToPositionNoRepaint(CPoint(GetScrollPos(SB_HORZ), nUpdatedPos));
		UpdateVisiblePages();
	}

	if (nTimeout != -1 && m_pages[nPage].pBitmap == NULL)
		::WaitForSingleObject(m_evtRendered, nTimeout);
	m_nPendingPage = -1;

	GetMainFrame()->UpdatePageCombo(this);

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
		Page& page = m_pages[m_nPage];

		// Update current page size 3 times to allow for scrollbars
		for (int i = 0; i < 3; ++i)
		{
			// First update all page sizes so that cached bitmaps
			// will be of right size
			for (int nPage = 0; nPage < m_nPageCount; ++nPage)
				UpdatePageSize(nPage);

			UpdatePageSize(m_nPage);

			m_szDisplay = page.rcDisplay.Size();

			CRect rcClient;
			GetClientRect(rcClient);
			CSize szDevPage(rcClient.Width()*3/4, rcClient.Height()*3/4);
			CSize szDevLine(15, 15);

			SetScrollSizes(m_szDisplay, szDevPage, szDevLine);
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
			nAnchorPage = GetCurrentPage();
			ptAnchorOffset = ptTop - m_pages[nAnchorPage].ptOffset;
		}
		else if (updateType == BOTTOM)
		{
			CPoint ptBottom = ptTop + rcClient.Size();

			int nPage = GetCurrentPage();
			while (nPage < m_nPageCount - 1 &&
				ptBottom.y > m_pages[nPage].rcDisplay.bottom)
				++nPage;

			nAnchorPage = nPage;
			ptAnchorOffset = ptBottom - m_pages[nAnchorPage].ptOffset;
		}

		for (int i = 0; i < 3; ++i)
		{
			GetClientRect(rcClient);

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
				page.rcDisplay.InflateRect(1, 1, c_nPageShadow + 1, c_nPageGap - 1);

				if (page.rcDisplay.Width() > m_szDisplay.cx)
					m_szDisplay.cx = page.rcDisplay.Width();
			}

			m_szDisplay.cy = m_pages.back().ptOffset.y + 
				m_pages.back().szDisplay.cy + c_nPageShadow + 1;

			GetClientRect(rcClient);
			if (m_szDisplay.cx < rcClient.Width())
				m_szDisplay.cx = rcClient.Width();

			// Center pages horizontally
			for (int nPage = 0; nPage < m_nPageCount; ++nPage)
			{
				Page& page = m_pages[nPage];

				if (page.rcDisplay.Width() < m_szDisplay.cx)
					page.ptOffset.x = (m_szDisplay.cx - page.rcDisplay.Width())/2;
				page.ptOffset.Offset(1, 0);

				page.rcDisplay.OffsetRect(page.ptOffset.x, 0);
			}

			// Center pages vertically
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

			CSize szDevPage(rcClient.Width()*3/4, rcClient.Height()*3/4);
			CSize szDevLine(15, 15);
			SetScrollSizesNoRepaint(m_szDisplay, szDevPage, szDevLine);
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
				CRect(CPoint(0, 0), page.szDisplay), m_nDisplayMode);
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
				CRect(CPoint(0, 0), page.szDisplay), m_nDisplayMode);
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
		if (nSize >= nMaxSize)
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

	CSize szFrame(2, 2);
	if (m_nMode != Fullscreen)
		szFrame += CSize(c_nPageShadow, c_nPageShadow);

	CRect rcClient;
	GetClientRect(rcClient);

	switch (m_nZoomType)
	{
	case ZoomFitWidth:
		szDisplay = rcClient.Size() - szFrame;
		szDisplay.cy = szDisplay.cx * szPage.cy / szPage.cx;
		break;

	case ZoomFitHeight:
		szDisplay = rcClient.Size() - szFrame;
		szDisplay.cx = szDisplay.cy * szPage.cx / szPage.cy;
		break;

	case ZoomFitPage:
		szDisplay = rcClient.Size() - szFrame;
		szDisplay.cx = szDisplay.cy * szPage.cx / szPage.cy;

		if (szDisplay.cx > rcClient.Width() - szFrame.cx)
		{
			szDisplay.cx = rcClient.Width() - szFrame.cx;
			szDisplay.cy = szDisplay.cx * szPage.cy / szPage.cx;
		}
		break;

	case ZoomActualSize:
		szDisplay = szPage;
		break;

	case ZoomStretch:
		szDisplay = rcClient.Size() - szFrame;
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

	page.ptOffset = CPoint(1, 1);
	page.szDisplay = CalcPageSize(szPage, page.info.nDPI);
	page.rcDisplay = CRect(CPoint(0, 0), page.szDisplay + CSize(2, 2));
	if (m_nMode != Fullscreen)
		page.rcDisplay.InflateRect(0, 0, c_nPageShadow, c_nPageShadow);

	CRect rcClient;
	GetClientRect(rcClient);

	if (page.rcDisplay.Width() < rcClient.Width())
	{
		page.ptOffset.x += (rcClient.Width() - page.rcDisplay.Width())/2;
		page.rcDisplay.right = rcClient.Width();
	}

	if (page.rcDisplay.Height() < rcClient.Height())
	{
		page.ptOffset.y += (rcClient.Height() - page.rcDisplay.Height())/2;
		page.rcDisplay.bottom = rcClient.Height();
	}
}

void CDjVuView::OnViewNextpage()
{
	int nPage = GetCurrentPage() + 1;

	if (nPage < m_nPageCount)
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
	int nPage = max(0, GetCurrentPage() - 1);
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
		Invalidate();

		GetMainFrame()->UpdatePageCombo(this);
	}

	CMyScrollView::OnSize(nType, cx, cy);
}

void CDjVuView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CSize szScroll(0, 0);
	bool bNextPage = false;
	bool bPrevPage = false;

	BOOL bHasHorzBar, bHasVertBar;
	CheckScrollBars(bHasHorzBar, bHasVertBar);

	switch (nChar)
	{
	case VK_DOWN:
		szScroll.cy = 3*m_lineDev.cy;
		bNextPage = (m_nLayout == SinglePage && !bHasVertBar);
		break;

	case VK_UP:
		szScroll.cy = -3*m_lineDev.cy;
		bPrevPage = (m_nLayout == SinglePage && !bHasVertBar);
		break;

	case VK_RIGHT:
		szScroll.cx = 3*m_lineDev.cx;
		bNextPage = (m_nLayout == SinglePage && !bHasHorzBar);
		break;

	case VK_LEFT:
		szScroll.cx = -3*m_lineDev.cx;
		bPrevPage = (m_nLayout == SinglePage && !bHasHorzBar);
		break;

	case VK_NEXT:
	case VK_SPACE:
		szScroll.cy = m_pageDev.cy;
		bNextPage = (m_nLayout == SinglePage && m_nPage < m_nPageCount - 1);
		break;

	case VK_PRIOR:
	case VK_BACK:
		szScroll.cy = -m_pageDev.cy;
		bPrevPage = (m_nLayout == SinglePage && m_nPage > 0);
		break;
	}

	if (m_nLayout == Continuous && szScroll.cy != 0)
		UpdatePageSizes(GetScrollPos(SB_VERT), szScroll.cy);

	if (!OnScrollBy(szScroll))
	{
		if (bNextPage)
		{
			OnViewNextpage();
		}
		else if (bPrevPage)
		{
			CPoint ptScroll = GetScrollPosition();
			OnViewPreviouspage();
			OnScrollBy(CPoint(ptScroll.x, m_szDisplay.cy) - GetScrollPosition());
			Invalidate();
		}
	}

	if (m_nLayout == Continuous)
	{
		UpdateVisiblePages();
		GetMainFrame()->UpdatePageCombo(this);
	}

	CMyScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
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
	if (m_nMode == Fullscreen)
	{
		pCmdUI->Enable(false);
		return;
	}

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

	const Page& page = m_pages[GetCurrentPage()];
	CSize szPage = page.GetSize(m_nRotate);

	if (szPage.cx == 0 || szPage.cy == 0)
		return 100.0;

	// Calculate zoom from display area size
	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	int nLogPixels = dcScreen.GetDeviceCaps(LOGPIXELSX);

	return 100.0*page.szDisplay.cx*page.info.nDPI/(szPage.cx*nLogPixels);
}

double CDjVuView::GetZoom(ZoomType nZoomType) const
{
	const Page& page = m_pages[GetCurrentPage()];
	CSize szPage = page.GetSize(m_nRotate);

	if (szPage.cx == 0 || szPage.cy == 0)
		return 100.0;

	// Calculate zoom from display area size
	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	int nLogPixels = dcScreen.GetDeviceCaps(LOGPIXELSX);

	CRect rcClient;
	GetClientRect(rcClient);

	CSize szDisplay;
	CSize szFrame(c_nPageShadow + 2, c_nPageShadow + 2);
	switch (nZoomType)
	{
	case ZoomFitWidth:
		szDisplay = rcClient.Size() - szFrame;
		szDisplay.cy = szDisplay.cx * szPage.cy / szPage.cx;
		break;

	case ZoomFitHeight:
		szDisplay = rcClient.Size() - szFrame;
		szDisplay.cx = szDisplay.cy * szPage.cx / szPage.cy;
		break;

	case ZoomFitPage:
		szDisplay = rcClient.Size() - szFrame;
		szDisplay.cx = szDisplay.cy * szPage.cx / szPage.cy;

		if (szDisplay.cx > rcClient.Width() - szFrame.cx)
		{
			szDisplay.cx = rcClient.Width() - szFrame.cx;
			szDisplay.cy = szDisplay.cx * szPage.cy / szPage.cx;
		}
		break;

	case ZoomActualSize:
		szDisplay = szPage;
		break;

	case ZoomStretch:
		szDisplay = rcClient.Size() - szFrame;
		break;

	default:
		ASSERT(false);
		break;
	}

	return 100.0*szDisplay.cx*page.info.nDPI/(szPage.cx*nLogPixels);
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

		if (m_nLayout == SinglePage)
		{
			UpdatePagesCache();
		}
		else if (m_nLayout == Continuous)
		{
			UpdateVisiblePages();
			GetMainFrame()->UpdatePageCombo(this);
		}
	}
}

void CDjVuView::OnUpdateViewLayout(CCmdUI* pCmdUI)
{
	if (m_nMode == Fullscreen)
	{
		pCmdUI->Enable(false);
		return;
	}

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

	if (m_nLayout == SinglePage)
		UpdatePagesCache();
	else if (m_nLayout == Continuous)
		UpdatePageSizes(GetScrollPos(SB_VERT));

	GetMainFrame()->UpdatePageCombo(this);
}

void CDjVuView::OnRotateRight()
{
	m_nRotate = (m_nRotate + 3) % 4;

	DeleteBitmaps();
	UpdateView();

	if (m_nLayout == SinglePage)
		UpdatePagesCache();
	else if (m_nLayout == Continuous)
		UpdatePageSizes(GetScrollPos(SB_VERT));

	GetMainFrame()->UpdatePageCombo(this);
}

void CDjVuView::OnRotate180()
{
	m_nRotate = (m_nRotate + 2) % 4;

	DeleteBitmaps();
	UpdateView();

	if (m_nLayout == SinglePage)
		UpdatePagesCache();
	else if (m_nLayout == Continuous)
		UpdatePageSizes(GetScrollPos(SB_VERT));

	GetMainFrame()->UpdatePageCombo(this);
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
		GetMainFrame()->UpdatePageCombo(this);
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
		GetMainFrame()->UpdatePageCombo(this);
	}
}

void CDjVuView::OnSetFocus(CWnd* pOldWnd)
{
	CMyScrollView::OnSetFocus(pOldWnd);

	GetMainFrame()->UpdatePageCombo(this);
	GetMainFrame()->UpdateZoomCombo(m_nZoomType, m_fZoom);

	m_nClickCount = 0;

	if (m_nMode == Fullscreen)
	{
		GetParent()->SetWindowPos(&wndTopMost, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
}

void CDjVuView::OnKillFocus(CWnd* pNewWnd)
{
	CMyScrollView::OnKillFocus(pNewWnd);

	if (m_nMode == Fullscreen && (!::IsWindow(pNewWnd->GetSafeHwnd())
			|| !GetMainFrame()->IsChild(pNewWnd)))
	{
		GetParent()->SetWindowPos(&wndNoTopMost, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		GetParent()->SetWindowPos(&wndTop, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
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

	if (m_nLayout == SinglePage)
		UpdatePagesCache();
	else if (m_nLayout == Continuous)
		UpdatePageSizes(GetScrollPos(SB_VERT));

	Invalidate();

	GetMainFrame()->UpdateZoomCombo(m_nZoomType, m_fZoom);
	GetMainFrame()->UpdatePageCombo(this);
}

BOOL CDjVuView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (hCursorHand == NULL)
		hCursorHand = AfxGetApp()->LoadCursor(IDC_CURSOR_HAND);
	if (hCursorDrag == NULL)
		hCursorDrag = AfxGetApp()->LoadCursor(IDC_CURSOR_DRAG);
	if (hCursorLink == NULL)
		hCursorLink = ::LoadCursor(0, IDC_HAND);
	if (hCursorText == NULL)
		hCursorText = ::LoadCursor(0, IDC_IBEAM);

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

	if (nHitTest == HTCLIENT)
	{
		if (m_nMode == Drag)
		{
			if (m_totalDev.cx > rcClient.Width() || m_totalDev.cy > rcClient.Height())
			{
				SetCursor(m_bDragging ? hCursorDrag : hCursorHand);
				return true;
			}
		}
		else if (m_nMode == Select && nPage != -1)
		{
			const Page& page = m_pages[nPage];
			CRect rcBitmap(CPoint(page.ptOffset - GetDeviceScrollPosition()), page.szDisplay);

			m_pages[nPage].DecodeText();
			if (rcBitmap.PtInRect(ptCursor) && m_pages[nPage].pText != NULL)
			{
				SetCursor(hCursorText);
				return true;
			}
		}
	}

	return CMyScrollView::OnSetCursor(pWnd, nHitTest, message);
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
		return;
	}

	if (m_nMode == Drag
			&& (m_totalDev.cx > rcClient.Width() || m_totalDev.cy > rcClient.Height()))
	{
		UpdateActiveHyperlink(CPoint(-1, -1));

		SetCapture();
		m_bDragging = true;

		m_nStartPage = GetCurrentPage();
		m_ptStartPos = GetScrollPosition() - m_pages[m_nStartPage].ptOffset;

		::GetCursorPos(&m_ptStart);

		if (hCursorDrag == NULL)
			hCursorDrag = AfxGetApp()->LoadCursor(IDC_CURSOR_DRAG);
		SetCursor(hCursorDrag);
	}
	else if (m_nMode == Select)
	{
		UpdateActiveHyperlink(CPoint(-1, -1));
		ClearSelection();

		m_nStartPage = GetPageFromPoint(point);
		if (m_nStartPage == -1)
			return;

		SetCapture();
		m_bDragging = true;
		m_nPrevPage = m_nStartPage;

		m_nSelStartPos = GetTextPosFromPoint(m_nStartPage, point);
	}
	else if (m_nMode == Fullscreen)
	{
		UpdateActiveHyperlink(CPoint(-1, -1));
		SetCapture();
		m_bDragging = true;

		OnViewNextpage();
	}

	CMyScrollView::OnLButtonDown(nFlags, point);
}

CPoint CDjVuView::TranslateToDjVuCoord(int nPage, const CPoint& point)
{
	const Page& page = m_pages[nPage];
	CSize szPage = page.GetSize(m_nRotate);

	double fRatioX = szPage.cx / (1.0*page.szDisplay.cx);
	double fRatioY = szPage.cy / (1.0*page.szDisplay.cy);

	CPoint ptResult(static_cast<int>(point.x * fRatioX),
			static_cast<int>(szPage.cy - point.y * fRatioY));

	if (m_nRotate != 0)
	{
		GRect rect(ptResult.x, ptResult.y);

		GRect input(0, 0, szPage.cx, szPage.cy);
		GRect output(0, 0, page.info.szPage.cx, page.info.szPage.cy);

		GRectMapper mapper;
		mapper.clear();
		mapper.set_input(input);
		mapper.set_output(output);               
		mapper.rotate(4 - m_nRotate);
		mapper.map(rect);

		ptResult = CPoint(rect.xmin, rect.ymin);
	}

	return ptResult;
}

void CDjVuView::GetTextPosFromTop(DjVuTXT::Zone& zone,  const CPoint& pt, int& nPos)
{
	if (zone.rect.xmin > pt.x || zone.rect.ymax < pt.y)
		return;

	if (zone.rect.xmax < pt.x || zone.rect.ymin > pt.y)
	{
		int nEnd = zone.text_start + zone.text_length;
		if (nPos == -1 || nPos < nEnd)
			nPos = nEnd;
		return;
	}

	if (zone.children.isempty()) 
	{
		nPos = zone.text_start;
	}
	else
	{
		for (GPosition pos = zone.children; pos; ++pos)
			GetTextPosFromTop(zone.children[pos], pt, nPos);
	}
}

void CDjVuView::GetTextPosFromBottom(DjVuTXT::Zone& zone,  const CPoint& pt, int& nPos)
{
	if (zone.rect.xmax < pt.x || zone.rect.ymin > pt.y)
		return;

	if (zone.rect.xmin > pt.x || zone.rect.ymax < pt.y)
	{
		if (nPos == -1 || nPos > zone.text_start)
			nPos = zone.text_start;
		return;
	}

	if (zone.children.isempty()) 
	{
		nPos = zone.text_start;
	}
	else
	{
		for (GPosition pos = zone.children; pos; ++pos)
			GetTextPosFromBottom(zone.children[pos], pt, nPos);
	}
}

int CDjVuView::GetTextPosFromPoint(int nPage, const CPoint& point)
{
	Page& page = m_pages[nPage];
	page.DecodeText();
	if (page.pText == NULL)
		return 0;

	CPoint pt = TranslateToDjVuCoord(nPage,
			point + GetScrollPosition() - page.ptOffset);

	int nPos = -1;
	GetTextPosFromTop(page.pText->page_zone, pt, nPos);

	if (nPos == -1)
		GetTextPosFromBottom(page.pText->page_zone, pt, nPos);

	if (nPos == -1)
		nPos = page.pText->textUTF8.length();

	return nPos;
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

	CMyScrollView::OnLButtonUp(nFlags, point);
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

	if (!m_bDragging)
	{
		UpdateActiveHyperlink(point);
		return;
	}

	if (m_nMode == Drag)
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
			GetMainFrame()->UpdatePageCombo(this);
		}

		return;
	}
	else if (m_nMode == Select)
	{
		if (m_pActiveLink != NULL)
			return;

		int nCurPage = GetPageFromPoint(point);
		if (nCurPage == -1)
			return;

		int nSelPos = GetTextPosFromPoint(nCurPage, point);
		bool bInfoLoaded = false;
		CWaitCursor* pWaitCursor = NULL;

		if (nCurPage < m_nStartPage)
		{
			if (m_nPrevPage >= m_nStartPage)
			{
				for (int nPage = m_nStartPage; nPage <= m_nPrevPage; ++nPage)
					ClearSelection(nPage);

				SelectTextRange(nCurPage, nSelPos, -1, bInfoLoaded, pWaitCursor);
				for (int nPage = nCurPage + 1; nPage < m_nStartPage; ++nPage)
					SelectTextRange(nPage, 0, -1, bInfoLoaded, pWaitCursor);
				SelectTextRange(m_nStartPage, 0, m_nSelStartPos, bInfoLoaded, pWaitCursor);
			}
			else if (m_nPrevPage <= nCurPage)
			{
				for (int nPage = m_nPrevPage; nPage <= nCurPage; ++nPage)
					ClearSelection(nPage);

				SelectTextRange(nCurPage, nSelPos, -1, bInfoLoaded, pWaitCursor);
			}
			else
			{
				ClearSelection(m_nPrevPage);

				SelectTextRange(nCurPage, nSelPos, -1, bInfoLoaded, pWaitCursor);
				for (int nPage = nCurPage + 1; nPage <= m_nPrevPage; ++nPage)
					SelectTextRange(nPage, 0, -1, bInfoLoaded, pWaitCursor);
			}
		}
		else if (nCurPage > m_nStartPage)
		{
			if (m_nPrevPage <= m_nStartPage)
			{
				for (int nPage = m_nPrevPage; nPage <= m_nStartPage; ++nPage)
					ClearSelection(nPage);

				SelectTextRange(m_nStartPage, m_nSelStartPos, -1, bInfoLoaded, pWaitCursor);
				for (int nPage = m_nStartPage + 1; nPage < nCurPage; ++nPage)
					SelectTextRange(nPage, 0, -1, bInfoLoaded, pWaitCursor);
				SelectTextRange(nCurPage, 0, nSelPos, bInfoLoaded, pWaitCursor);
			}
			else if (m_nPrevPage >= nCurPage)
			{
				for (int nPage = nCurPage; nPage <= m_nPrevPage; ++nPage)
					ClearSelection(nPage);

				SelectTextRange(nCurPage, 0, nSelPos, bInfoLoaded, pWaitCursor);
			}
			else
			{
				ClearSelection(m_nPrevPage);

				for (int nPage = m_nPrevPage; nPage < nCurPage; ++nPage)
					SelectTextRange(nPage, 0, -1, bInfoLoaded, pWaitCursor);
				SelectTextRange(nCurPage, 0, nSelPos, bInfoLoaded, pWaitCursor);
			}
		}
		else
		{
			if (m_nPrevPage < m_nStartPage)
			{
				for (int nPage = m_nPrevPage; nPage <= m_nStartPage; ++nPage)
					ClearSelection(nPage);
			}
			else
			{
				for (int nPage = m_nStartPage; nPage <= m_nPrevPage; ++nPage)
					ClearSelection(nPage);
			}

			if (nSelPos < m_nSelStartPos)
			{
				SelectTextRange(nCurPage, nSelPos, m_nSelStartPos, bInfoLoaded, pWaitCursor);
			}
			else if (nSelPos > m_nSelStartPos)
			{
				SelectTextRange(nCurPage, m_nSelStartPos, nSelPos, bInfoLoaded, pWaitCursor);
			}
		}

		m_nPrevPage = nCurPage;
		UpdateSelectionStatus();

		if (bInfoLoaded)
			UpdateView();

		if (pWaitCursor != NULL)
			delete pWaitCursor;
	}

	CMyScrollView::OnMouseMove(nFlags, point);
}

void CDjVuView::SelectTextRange(int nPage, int nStart, int nEnd,
	bool& bInfoLoaded, CWaitCursor*& pWaitCursor)
{
	Page& page = m_pages[nPage];
	if (!page.bInfoLoaded)
	{
		if (pWaitCursor == NULL)
			pWaitCursor = new CWaitCursor();

		PageInfo info = GetDocument()->GetPageInfo(nPage);
		page.Init(info);
		bInfoLoaded = true;
	}

	if (page.info.pTextStream != NULL && !page.bTextDecoded && pWaitCursor == NULL)
		pWaitCursor = new CWaitCursor();

	page.DecodeText();
	if (page.pText == NULL)
		return;

	if (nEnd == -1)
		nEnd = page.pText->textUTF8.length();

	DjVuSelection selection;
	FindSelectionZones(selection, page.pText, nStart, nEnd);

	page.selection = selection;
	if (!selection.isempty())
	{
		nStart = -1;
		nEnd = -1;
		for (GPosition pos = selection; pos; ++pos)
		{
			if (nStart == -1 || nStart > selection[pos]->text_start)
				nStart = selection[pos]->text_start;

			int nZoneEnd = selection[pos]->text_start + selection[pos]->text_length;
			if (nEnd == -1 || nEnd < nZoneEnd)
				nEnd = nZoneEnd;
		}

		page.nSelStart = nStart;
		page.nSelEnd = nEnd;
		m_bHasSelection = true;
	}
	else
	{
		page.nSelStart = -1;
		page.nSelEnd = -1;
	}

	InvalidatePage(nPage);
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

void CDjVuView::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_nMode == Fullscreen)
	{
		OnViewPreviouspage();
		return;
	}

	CMyScrollView::OnRButtonDown(nFlags, point);
}

void CDjVuView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (m_nMode == Fullscreen)
		return;

#ifndef ELIBRA_READER
	if (point.x < 0 || point.y < 0)
		point = CPoint(0, 0);

	CRect rcClient;
	GetClientRect(rcClient);
	ClientToScreen(rcClient);

	if (!rcClient.PtInRect(point))
	{
		CMyScrollView::OnContextMenu(pWnd, point);
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
	
	if (InvalidatePage(nPage))
		UpdateWindow();

	return 0;
}

void CDjVuView::OnDestroy()
{
	delete m_pRenderThread;
	m_pRenderThread = NULL;

	CMyScrollView::OnDestroy();
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
				GetMainFrame()->UpdatePageCombo(this);

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

	BOOL bResult = CMyScrollView::OnScroll(nScrollCode, nPos, bDoScroll);

	if (m_nLayout == Continuous)
	{
		UpdateVisiblePages();
		GetMainFrame()->UpdatePageCombo(this);
	}

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

BOOL CDjVuView::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	// we don't handle anything but scrolling
	if ((nFlags & MK_CONTROL) != 0)
		return false;

	CWnd* pWnd = WindowFromPoint(point);
	if (pWnd != this && !IsChild(pWnd) && GetMainFrame()->IsChild(pWnd) &&
			pWnd->SendMessage(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(point.x, point.y)) != 0)
		return true;

	BOOL bHasHorzBar, bHasVertBar;
	CheckScrollBars(bHasHorzBar, bHasVertBar);

	if (!bHasVertBar && !bHasHorzBar && m_nLayout != SinglePage)
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

		if (m_nLayout == Continuous)
			UpdatePageSizes(GetScrollPos(SB_VERT), nDisplacement);

		OnScrollBy(CSize(0, nDisplacement));

		if (m_nLayout == Continuous)
		{
			UpdateVisiblePages();
			GetMainFrame()->UpdatePageCombo(this);
		}
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
	else if (m_nLayout == SinglePage && m_nPageCount > 0)
	{
		if (zDelta < 0 && m_nPage < m_nPageCount - 1)
		{
			OnViewNextpage();
		}
		else if (zDelta > 0 && m_nPage > 0)
		{
			CPoint ptScroll = GetScrollPosition();
			OnViewPreviouspage();
			OnScrollBy(CPoint(ptScroll.x, m_szDisplay.cy) - GetScrollPosition());
			Invalidate();
		}
	}

	UpdateWindow();
	return true;
}

bool CDjVuView::InvalidatePage(int nPage)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);
	CRect rect;
	GetClientRect(rect);

	int y = GetScrollPos(SB_VERT);
	rect.top = max(rect.top, m_pages[nPage].rcDisplay.top - y);
	rect.bottom = min(rect.bottom, m_pages[nPage].rcDisplay.bottom - y);

	if (!rect.IsRectEmpty() && (m_nLayout != SinglePage || nPage == m_nPage))
	{
		InvalidateRect(rect);
		return true;
	}

	return false;
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
	CDIB* pBitmap = CRenderThread::Render(pImage, rect);

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

	bool bNeedUpdate = false;

	int nPage = nStartPage;
	while (nPage < m_nPageCount)
	{
		Page& page = m_pages[nPage];
		if (!page.bInfoLoaded)
		{
			PageInfo info = GetDocument()->GetPageInfo(nPage);
			page.Init(info);
			bNeedUpdate = true;
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
				if (bNeedUpdate)
				{
					UpdateView();
					bNeedUpdate = false;
				}

				int nSelEnd = nPos + strFind.length();
				DjVuSelection selection;
				FindSelectionZones(selection, pText, nPos, nSelEnd);

				if (!bHasSelection && nPage == nStartPage &&
						!IsSelectionBelowTop(nPage, selection))
				{
					nStartPos = nPos + strFind.length();
					continue;
				}

				ClearSelection();

				int nStart = -1;
				int nEnd = -1;
				for (GPosition pos = selection; pos; ++pos)
				{
					if (nStart == -1 || nStart > selection[pos]->text_start)
						nStart = selection[pos]->text_start;

					int nZoneEnd = selection[pos]->text_start + selection[pos]->text_length;
					if (nEnd == -1 || nEnd < nZoneEnd)
						nEnd = nZoneEnd;
				}

				page.nSelStart = nStart;
				page.nSelEnd = nEnd;
				page.selection = selection;
				m_bHasSelection = !selection.isempty();

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

	if (bNeedUpdate)
		UpdateView();

	GetMainFrame()->HilightStatusMessage(_T("Search string not found"));
	::MessageBeep(MB_OK);
}

void CDjVuView::OnFindAll()
{
	CWaitCursor wait;

	CFindDlg* pDlg = GetMainFrame()->m_pFindDlg;
	ASSERT(pDlg != NULL);

	GUTF8String strFindUTF8 = GNativeString(pDlg->m_strFind).NativeToUTF8();
	GUTF8String strFindUp;
	if (!pDlg->m_bMatchCase)
		strFindUp = strFindUTF8.upcase();

	GUTF8String& strFind = (pDlg->m_bMatchCase ? strFindUTF8 : strFindUp);

	CSearchResultsView* pResults = NULL;
	int nResultCount = 0;
	bool bNeedUpdate = false;

	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		CString strStatus;
		strStatus.Format(_T("Searching page %d of %d"), nPage + 1, m_nPageCount);
		pDlg->SetStatusText(strStatus);

		Page& page = m_pages[nPage];
		if (!page.bInfoLoaded)
		{
			PageInfo info = GetDocument()->GetPageInfo(nPage);
			page.Init(info);
			bNeedUpdate = true;
		}

		page.DecodeText();
		if (page.pText == NULL)
			continue;

		GP<DjVuTXT> pText = page.pText;
		GUTF8String textUp;
		if (!pDlg->m_bMatchCase)
			textUp = pText->textUTF8.upcase();

		GUTF8String& text = (pDlg->m_bMatchCase ? pText->textUTF8 : textUp);

		int nPos = 0;
		do
		{
			nPos = text.search(strFind, nPos);
			if (nPos == -1)
				break;

			int nSelEnd = nPos + strFind.length();

			if (pResults == NULL)
			{
				pResults = ((CChildFrame*)GetParentFrame())->GetResultsView();
				pResults->Reset();
			}

			++nResultCount;
			CString strPreview = MakePreviewString(pText->textUTF8, nPos, nSelEnd);
			pResults->AddString(strPreview, nPage, nPos, nSelEnd);

			nPos = nSelEnd;
		} while (nPos != -1);
	}

	if (bNeedUpdate)
		UpdateView();

	if (nResultCount > 0)
	{
		CString strMessage;
		strMessage.Format(_T("%d occurrences found"), nResultCount);
		GetMainFrame()->SetMessageText(strMessage);
		pDlg->ShowWindow(SW_HIDE);
	}
	else
	{
		GetMainFrame()->HilightStatusMessage(_T("Search string not found"));
		::MessageBeep(MB_OK);
	}
}

CString MakePreviewString(const GUTF8String& text, int nStart, int nEnd)
{
	// Convert text to ANSI encoding and add -1/+10 words to the string
	CString strPreview = MakeANSIString(text.substr(nStart, nEnd - nStart));

	CString strBefore = MakeANSIString(text.substr(0, nStart));

	int nPos = strBefore.GetLength() - 1;
	for (int i = 0; i < 2 && nPos >= 0; ++i)
	{
		if (i > 0)
			strPreview = _T(" ") + strPreview;

		while (nPos >= 0 && static_cast<unsigned int>(strBefore[nPos]) > 0x20
			&& strBefore.GetLength() - nPos < 30)
		{
			strPreview = strBefore[nPos--] + strPreview;
		}

		while (nPos >= 0 && static_cast<unsigned int>(strBefore[nPos]) <= 0x20)
			--nPos;
	}

	CString strAfter = MakeANSIString(text.substr(nEnd, text.length() - nEnd));

	nPos = 0;
	for (int i = 0; i < 11 && nPos < strAfter.GetLength(); ++i)
	{
		if (i > 0)
			strPreview += _T(" ");

		while (nPos < strAfter.GetLength() && static_cast<unsigned int>(strAfter[nPos]) > 0x20
			&& nPos < 200)
		{
			strPreview += strAfter[nPos++];
		}

		while (nPos < strAfter.GetLength() && static_cast<unsigned int>(strAfter[nPos]) <= 0x20)
			++nPos;
	}

	return strPreview;
}

void CDjVuView::FindSelectionZones(DjVuSelection& list, DjVuTXT* pText, int nStart, int nEnd)
{
	for (GPosition pos = pText->page_zone.children; pos; ++pos)
		pText->page_zone.children[pos].find_zones(list, nStart, nEnd);
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
	int nWidth = min(page.szDisplay.cx, rcClient.Width());

	CRect rcSel = GetSelectionRect(nPage, selection);

	// Scroll horizontally
	int nLeftPos = rcClient.left;
	if (rcSel.left < rcClient.left)
	{
		nLeftPos = max(page.rcDisplay.left, static_cast<int>(rcSel.left - 0.2*nWidth));
		if (nLeftPos + rcClient.Width() < rcSel.right)
			nLeftPos = rcSel.right - rcClient.Width();
		nLeftPos = min(rcSel.left, nLeftPos);
	}
	else if (rcSel.right > rcClient.right)
	{
		int nRight = min(page.rcDisplay.right, static_cast<int>(rcSel.right + 0.2*nWidth));
		nLeftPos = min(rcSel.left, nRight - rcClient.Width());
	}

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

	CPoint ptScrollBy(nLeftPos - ptScroll.x, nTopPos - GetScrollPos(SB_VERT));

	if (m_nLayout == Continuous)
		UpdatePageSizes(GetScrollPos(SB_VERT), ptScrollBy.y);

	OnScrollBy(ptScrollBy);

	if (m_nLayout == Continuous)
	{
		UpdateVisiblePages();
		GetMainFrame()->UpdatePageCombo(this);
	}
}

CRect CDjVuView::TranslatePageRect(int nPage, GRect rect) const
{
	const Page& page = m_pages[nPage];
	CSize szPage = page.GetSize(m_nRotate);

	if (m_nRotate != 0)
	{
		GRect input(0, 0, szPage.cx, szPage.cy);
		GRect output(0, 0, page.info.szPage.cx, page.info.szPage.cy);

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

void CDjVuView::ClearSelection(int nPage)
{
	if (nPage != -1)
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
	else
	{
		for (nPage = 0; nPage < m_nPageCount; ++nPage)
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

		m_bHasSelection = false;
	}
}

void CDjVuView::UpdateSelectionStatus()
{
	m_bHasSelection = false;

	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		Page& page = m_pages[nPage];

		if (page.nSelStart != -1)
		{
			m_bHasSelection = true;
			break;
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
			m_toolTip.Activate(false);
			CRect rcArea = TranslatePageRect(m_nLinkPage, m_pActiveLink->get_bound_rect());
			rcArea.InflateRect(0, 0, 1, 1);
			rcArea.OffsetRect(-GetScrollPosition());
			InvalidateRect(rcArea);
		}

		m_pActiveLink = pHyperlink;
		m_nLinkPage = nPage;

		if (m_pActiveLink != NULL)
		{
			m_toolTip.Activate(true);
			CRect rcArea = TranslatePageRect(nPage, m_pActiveLink->get_bound_rect());
			rcArea.InflateRect(0, 0, 1, 1);
			rcArea.OffsetRect(-GetScrollPosition());
			InvalidateRect(rcArea);
		}
	}
}

GP<GMapArea> CDjVuView::GetHyperlinkFromPoint(CPoint point, int* pnPage)
{
	if (pnPage != NULL)
		*pnPage = -1;

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
	if (::IsWindow(m_toolTip.m_hWnd))
		m_toolTip.RelayEvent(pMsg);

	return CMyScrollView::PreTranslateMessage(pMsg);
}

void CDjVuView::GoToURL(const GUTF8String& url, int nLinkPage, int nAddToHistory)
{
	if (nLinkPage == -1)
		nLinkPage = GetCurrentPage();

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

		GoToPage(nPage, nLinkPage, nAddToHistory);
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
	if (_tcsicmp(szExt, _T(".djvu")) == 0 || _tcsicmp(szExt, _T(".djv")) == 0
#ifdef ELIBRA_READER
			|| _tcsicmp(szExt, _T(".elib")) == 0
#endif
		)
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

			if (m_nMode == Fullscreen)
			{
				CDjVuView* pOwner = ((CFullscreenWnd*)GetParent())->GetOwner();
				GetParent()->DestroyWindow();

				if (nAddToHistory & AddSource)
					GetMainFrame()->AddToHistory(pOwner, nLinkPage);
			}
			else
			{
				if (nAddToHistory & AddSource)
					GetMainFrame()->AddToHistory(this, nLinkPage);
			}

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

			if (m_nMode == Fullscreen)
			{
				CDjVuView* pOwner = ((CFullscreenWnd*)GetParent())->GetOwner();
				GetParent()->DestroyWindow();

				if (nAddToHistory & AddSource)
					GetMainFrame()->AddToHistory(pOwner, nLinkPage);
			}
			else
			{
				if (nAddToHistory & AddSource)
					GetMainFrame()->AddToHistory(this, nLinkPage);
			}

			theApp.OpenDocument(strPathName, strPage);
			return;
		}
	}

	// Open a web link
	::ShellExecute(NULL, "open", (const char*)url, NULL, NULL, SW_SHOWNORMAL);
}

void CDjVuView::GoToPage(int nPage, int nLinkPage, int nAddToHistory)
{
	if (nLinkPage == -1)
		nLinkPage = GetCurrentPage();

	if (m_nMode != Fullscreen)
	{
		if (nAddToHistory & AddSource)
			GetMainFrame()->AddToHistory(this, nLinkPage);

		if (nAddToHistory & AddTarget)
			GetMainFrame()->AddToHistory(this, nPage);
	}

	RenderPage(nPage);
}

void CDjVuView::OnViewZoomIn()
{
	double fCurrentZoom = GetZoom();
	static const double levels[] = { 10.0, 12.5, 25, 33.33, 50, 66.66, 75,
		100, 125, 150, 200, 300, 400, 600, 800 };
	double fFitWidth, fFitPage, fActualSize;

	vector<double> zoomLevels(levels, levels + sizeof(levels)/sizeof(double));
	zoomLevels.push_back(fFitWidth = GetZoom(ZoomFitWidth));
	zoomLevels.push_back(fFitPage = GetZoom(ZoomFitPage));
	zoomLevels.push_back(fActualSize = GetZoom(ZoomActualSize));

	sort(zoomLevels.begin(), zoomLevels.end());

	int nZoom;
	for (nZoom = 0; nZoom < static_cast<int>(zoomLevels.size() - 1); ++nZoom)
	{
		if (zoomLevels[nZoom] > fCurrentZoom)
			break;
	}

	if (zoomLevels[nZoom] == fFitWidth)
		ZoomTo(ZoomFitWidth);
	else if (zoomLevels[nZoom] == fFitPage)
		ZoomTo(ZoomFitPage);
	else if (zoomLevels[nZoom] == fActualSize)
		ZoomTo(ZoomActualSize);
	else
		ZoomTo(ZoomPercent, zoomLevels[nZoom]);
}

void CDjVuView::OnViewZoomOut()
{
	double fCurrentZoom = GetZoom();
	static const double levels[] = { 10.0, 12.5, 25, 33.33, 50, 66.66, 75,
		100, 125, 150, 200, 300, 400, 600, 800 };
	double fFitWidth, fFitPage, fActualSize;

	vector<double> zoomLevels(levels, levels + sizeof(levels)/sizeof(double));
	zoomLevels.push_back(fFitWidth = GetZoom(ZoomFitWidth));
	zoomLevels.push_back(fFitPage = GetZoom(ZoomFitPage));
	zoomLevels.push_back(fActualSize = GetZoom(ZoomActualSize));

	sort(zoomLevels.begin(), zoomLevels.end());

	int nZoom;
	for (nZoom = zoomLevels.size() - 1; nZoom > 0; --nZoom)
	{
		if (zoomLevels[nZoom] < fCurrentZoom)
			break;
	}

	if (zoomLevels[nZoom] == fFitWidth)
		ZoomTo(ZoomFitWidth);
	else if (zoomLevels[nZoom] == fFitPage)
		ZoomTo(ZoomFitPage);
	else if (zoomLevels[nZoom] == fActualSize)
		ZoomTo(ZoomActualSize);
	else
		ZoomTo(ZoomPercent, zoomLevels[nZoom]);
}

void CDjVuView::OnUpdateViewZoomIn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_nMode != Fullscreen && GetZoom() < 800);
}

void CDjVuView::OnUpdateViewZoomOut(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_nMode != Fullscreen && GetZoom() > 10);
}

void CDjVuView::StopDecoding()
{
	m_pRenderThread->Stop();
}

void CDjVuView::OnChangeMode(UINT nID)
{
	m_nMode = (nID == ID_MODE_DRAG ? Drag : Select);
	CAppSettings::nDefaultMode = m_nMode;
}

void CDjVuView::OnUpdateMode(CCmdUI* pCmdUI)
{
	int nID = (m_nMode == Drag ? ID_MODE_DRAG : ID_MODE_SELECT);
	pCmdUI->SetCheck(nID == pCmdUI->m_nID);
}

void CDjVuView::OnEditCopy()
{
	if (!m_bHasSelection)
		return;

	CWaitCursor wait;

	if (!OpenClipboard())
		return;
	EmptyClipboard();

	GUTF8String text = GetSelectedText();
	HGLOBAL hText = NULL, hUnicodeText = NULL;

	// Prepare Unicode text
	int nSize = MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)text, -1, NULL, 0);
	hUnicodeText = ::GlobalAlloc(GMEM_MOVEABLE, nSize*sizeof(WCHAR));
	if (hUnicodeText != NULL)
	{
		// Lock the handle and copy the text to the buffer.
		LPWSTR pszUnicodeText = (LPWSTR)::GlobalLock(hUnicodeText);
		MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)text, -1, pszUnicodeText, nSize);

		// Prepare ANSI text
		nSize = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DISCARDNS,
				pszUnicodeText, -1, NULL, 0, NULL, NULL);
		hText = ::GlobalAlloc(GMEM_MOVEABLE, nSize*sizeof(CHAR));
		if (hText != NULL)
		{
			LPSTR pszText = (LPSTR)::GlobalLock(hText);
			WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DISCARDNS,
				pszUnicodeText, -1, pszText, nSize, NULL, NULL);
			::GlobalUnlock(pszText);
		}

		::GlobalUnlock(pszUnicodeText);
	}

	if (hUnicodeText != NULL)
		SetClipboardData(CF_UNICODETEXT, hUnicodeText);
	if (hText != NULL)
		SetClipboardData(CF_TEXT, hText);

	CloseClipboard();
}

void CDjVuView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bHasSelection);
}

GUTF8String CDjVuView::GetSelectedText()
{
	GUTF8String selection;
	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		Page& page = m_pages[nPage];
		if (page.nSelStart != -1)
		{
			ASSERT(page.pText != NULL);
			selection += page.pText->textUTF8.substr(page.nSelStart, page.nSelEnd - page.nSelStart);
		}
	}

	return selection;
}

GUTF8String CDjVuView::GetFullText()
{
	GUTF8String text;
	bool bNeedUpdate = false;

	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		Page& page = m_pages[nPage];
		if (!page.bInfoLoaded)
		{
			PageInfo info = GetDocument()->GetPageInfo(nPage);
			page.Init(info);
			bNeedUpdate = true;
		}

		page.DecodeText();
		if (page.pText != NULL)
			text += page.pText->textUTF8;
	}

	if (bNeedUpdate)
		UpdateView();

	return text;
}

CString MakeANSIString(const GUTF8String& text)
{
	// Prepare Unicode text
	int nSize = ::MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)text, -1, NULL, 0);
	LPWSTR pszUnicodeText = new WCHAR[nSize];
	::MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)text, -1, pszUnicodeText, nSize);

	// Prepare ANSI text
      nSize = ::WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DISCARDNS,
		pszUnicodeText, -1, NULL, 0, NULL, NULL);
	LPSTR pszText = new CHAR[nSize];
	::WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DISCARDNS,
		pszUnicodeText, -1, pszText, nSize, NULL, NULL);

	CString strResult(pszText);

	delete[] pszText;
	delete[] pszUnicodeText;

	return strResult;
}

void CDjVuView::OnFileExportText()
{
	CString strPathName = GetDocument()->GetPathName();
	TCHAR szDrive[_MAX_DRIVE], szPath[_MAX_PATH], szName[_MAX_FNAME], szExt[_MAX_EXT];
	_tsplitpath(strPathName, szDrive, szPath, szName, szExt);
	CString strFileName = szName + CString(".txt");

	const TCHAR szFilter[] = "Text Documents (*.txt)|*.txt|All Files (*.*)|*.*||";

	CFileDialog dlg(false, "txt", strFileName, OFN_OVERWRITEPROMPT |
		OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST, szFilter);
	CString strTitle(_T("Export Text"));
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer();

	if (dlg.DoModal() != IDOK)
		return;

	CWaitCursor wait;

	strFileName = dlg.GetPathName();
	GUTF8String text = GetFullText();
	CString strANSIText = MakeANSIString(text);

	CFile file;
	if (file.Open(strFileName, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive))
	{
		file.Write(strANSIText, strANSIText.GetLength());
		file.Close();
	}
}

void CDjVuView::OnUpdateFileExportText(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetDocument()->HasText());
}

void CDjVuView::OnSettingsChanged()
{
#ifndef ELIBRA_READER
	if (m_displaySettings != CAppSettings::displaySettings)
	{
		m_displaySettings = CAppSettings::displaySettings;

		DeleteBitmaps();

		if (m_nLayout == SinglePage)
			UpdatePagesCache();
		else if (m_nLayout == Continuous)
			UpdateVisiblePages();
	}
#endif
}

void CDjVuView::OnViewDisplay(UINT nID)
{
	int nDisplayMode = Color;

	switch (nID)
	{
	case ID_DISPLAY_COLOR:
		nDisplayMode = Color;
		break;

	case ID_DISPLAY_BW:
		nDisplayMode = BlackAndWhite;
		break;

	case ID_DISPLAY_BACKGROUND:
		nDisplayMode = Background;
		break;

	case ID_DISPLAY_FOREGROUND:
		nDisplayMode = Foreground;
		break;
	}

	if (m_nDisplayMode != nDisplayMode)
	{
		m_nDisplayMode = nDisplayMode;

		DeleteBitmaps();
		if (m_nLayout == SinglePage)
			UpdatePagesCache();
		else if (m_nLayout == Continuous)
			UpdateVisiblePages();
	}
}

void CDjVuView::OnUpdateViewDisplay(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_DISPLAY_COLOR:
		pCmdUI->SetCheck(m_nDisplayMode == Color);
		break;

	case ID_DISPLAY_BW:
		pCmdUI->SetCheck(m_nDisplayMode == BlackAndWhite);
		break;

	case ID_DISPLAY_BACKGROUND:
		pCmdUI->SetCheck(m_nDisplayMode == Background);
		break;

	case ID_DISPLAY_FOREGROUND:
		pCmdUI->SetCheck(m_nDisplayMode == Foreground);
		break;
	}
}

void CDjVuView::GoToSelection(int nPage, int nStartPos, int nEndPos, int nLinkPage, int nAddToHistory)
{
	Page& page = m_pages[nPage];
	bool bInfoLoaded;
	CWaitCursor* pWaitCursor;

	ClearSelection();
	SelectTextRange(nPage, nStartPos, nEndPos, bInfoLoaded, pWaitCursor);

	if (bInfoLoaded)
		UpdateView();

	if (!IsSelectionVisible(nPage, page.selection))
	{
		if (pWaitCursor == NULL)
			pWaitCursor = new CWaitCursor();

		EnsureSelectionVisible(nPage, page.selection);
	}

	if (pWaitCursor)
		delete pWaitCursor;

	if (nLinkPage == -1)
		nLinkPage = GetCurrentPage();

	if (m_nMode != Fullscreen)
	{
		if (nAddToHistory & AddSource)
			GetMainFrame()->AddToHistory(this, nLinkPage);

		if (nAddToHistory & AddTarget)
			GetMainFrame()->AddToHistory(this, nPage);
	}
}

void CDjVuView::OnViewFullscreen()
{
	if (m_nMode == Fullscreen)
	{
		GetParent()->DestroyWindow();
		return;
	}

	m_pRenderThread->Stop();

	CFullscreenWnd* pWnd = new CFullscreenWnd(this);
	pWnd->Create();

	CDjVuView* pView = (CDjVuView*)RUNTIME_CLASS(CDjVuView)->CreateObject();
	pView->Create(NULL, NULL, WS_CHILD, CRect(0, 0, 0, 0), pWnd, 2);
	pView->SetDocument(GetDocument());
	pWnd->SetView(pView);
	GetMainFrame()->SetFullscreenWnd(pWnd);

	pView->m_nMode = Fullscreen;
	pView->m_nLayout = SinglePage;
	pView->m_nZoomType = ZoomFitPage;
	pView->m_nDisplayMode = m_nDisplayMode;
	pView->m_nRotate = m_nRotate;
	pView->m_nPage = GetCurrentPage();
	pView->OnInitialUpdate();

	UpdatePageInfo(pView);

	pView->ShowWindow(SW_SHOW);
	pWnd->SetForegroundWindow();
	pView->SetFocus();
}

void CDjVuView::RestartThread()
{
	if (m_pRenderThread != NULL)
		delete m_pRenderThread;

	m_pRenderThread = new CRenderThread(GetDocument(), this);

	UpdateView();
	if (m_nLayout == SinglePage)
		UpdatePagesCache();
	else if (m_nLayout == Continuous)
		UpdateVisiblePages();
}

int CDjVuView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	if (m_nMode == Fullscreen)
	{
		// From MFC: CView::OnMouseActivate
		// Don't call CFrameWnd::SetActiveView

		int nResult = CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
		if (nResult == MA_NOACTIVATE || nResult == MA_NOACTIVATEANDEAT)
			return nResult;

		// set focus to this view, but don't notify the parent frame
		OnActivateView(TRUE, this, this);
		return nResult;
	}

	return CScrollView::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

void CDjVuView::UpdatePageInfo(CDjVuView* pView)
{
	ASSERT(m_nPageCount == pView->m_nPageCount);

	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		Page& page = m_pages[nPage];
		Page& rhsPage = pView->m_pages[nPage];

		if (page.bInfoLoaded && !rhsPage.bInfoLoaded)
			rhsPage.Init(page.info);
	}
}

void CDjVuView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_nMode == Select)
	{
		ClearSelection();

		int nPage = GetPageFromPoint(point);
		if (nPage == -1)
			return;

		int nPos = GetTextPosFromPoint(nPage, point);
		bool bInfoLoaded = false;
		CWaitCursor* pWaitCursor = NULL;
		SelectTextRange(nPage, nPos, nPos + 1, bInfoLoaded, pWaitCursor);

		if (bInfoLoaded)
			UpdateView();

		if (pWaitCursor != NULL)
			delete pWaitCursor;
	}

	CMyScrollView::OnLButtonDblClk(nFlags, point);
}

void CDjVuView::ShowAllLinks(bool bShowAll)
{
	m_bShowAllLinks = bShowAll;
	Invalidate();
	UpdateWindow();
}
