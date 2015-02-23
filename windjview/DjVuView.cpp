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
#include "Global.h"

#include "DjVuDoc.h"
#include "DjVuView.h"

#include "MainFrm.h"
#include "MDIChild.h"
#include "PrintDlg.h"
#include "Drawing.h"
#include "ProgressDlg.h"
#include "ZoomDlg.h"
#include "GotoPageDlg.h"
#include "FindDlg.h"
#include "BookmarksView.h"
#include "SearchResultsView.h"
#include "FullscreenWnd.h"
#include "ThumbnailsView.h"
#include "PageIndexWnd.h"
#include "MagnifyWnd.h"
#include "AnnotationDlg.h"
#include "BookmarkDlg.h"
#include "MyFileDialog.h"
#include "NavPane.h"
#include "MyGdiPlus.h"

#include "RenderThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Segment

struct Segment
{
	Segment(double x1_, double y1_, double x2_, double y2_)
		: x1(x1_), y1(y1_), x2(x2_), y2(y2_)
	{
		a = y2 - y1;
		b = x1 - x2;
		c = -a*x1 - b*y1;

		double mult = pow(a*a + b*b, 0.5);
		if (mult > 1e-9)
		{
			a /= mult;
			b /= mult;
			c /= mult;
		}
		else
		{
			a = b = 0;
			c = (c > 0 ? 1 : c < 0 ? -1 : 0);
		}
	}

	bool Contains(double x, double y) const
	{
		if (fabs(a*x + b*y + c) > 1e-6)
			return false;

		return (x - x1)*(x - x2) <= 1e-9 && (y - y1)*(y - y2) <= 1e-9;
	}

	bool Intersects(const Segment& rhs) const
	{
		double x, y;
		if (!IntersectionPoint(rhs, x, y))
			return false;

		return (x - x1)*(x - x2) <= 1e-9 && (y - y1)*(y - y2) <= 1e-9
				&& (x - rhs.x1)*(x - rhs.x2) <= 1e-9 && (y - rhs.y1)*(y - rhs.y2) <= 1e-9;
	}

	bool IntersectsDisc(double x, double y, double r) const
	{
		if (pow(pow(x - x1, 2) + pow(y - y1, 2), 0.5) <= r + 1e-6
				|| pow(pow(x - x2, 2) + pow(y - y2, 2), 0.5) <= r + 1e-6)
			return true;

		double xx, yy;
		Segment perp(x, y, x + (y1 - y2), y + (x2 - x1));
		if (!IntersectionPoint(perp, xx, yy))
			return false;

		return (xx - x1)*(xx - x2) <= 1e-9 && (yy - y1)*(yy - y2) <= 1e-9
				&& pow(pow(x - xx, 2) + pow(y - yy, 2), 0.5) <= r + 1e-6;
	}

private:
	bool IntersectionPoint(const Segment& rhs, double& x, double& y) const
	{
		double d = a*rhs.b - b*rhs.a;
		if (fabs(d) < 1e-9)
			return false;

		x = (b*rhs.c - c*rhs.b) / d;
		y = (c*rhs.a - a*rhs.c) / d;
		return true;
	}

	double x1, y1, x2, y2, a, b, c;
};


// CDjVuView

HCURSOR CDjVuView::hCursorHand = NULL;
HCURSOR CDjVuView::hCursorDrag = NULL;
HCURSOR CDjVuView::hCursorLink = NULL;
HCURSOR CDjVuView::hCursorText = NULL;
HCURSOR CDjVuView::hCursorMagnify = NULL;
HCURSOR CDjVuView::hCursorCross = NULL;
HCURSOR CDjVuView::hCursorZoomRect = NULL;

const int c_nDefaultMargin = 4;
const int c_nDefaultShadowMargin = 4;  // 3
const int c_nDefaultPageGap = 4;
const int c_nDefaultFacingGap = 4;
const int c_nDefaultPageBorder = 0;  // 1
const int c_nDefaultPageShadow = 0;  // 3

const int c_nFullscreenMargin = 0;
const int c_nFullscreenShadowMargin = 0;
const int c_nFullscreenPageGap = 4;
const int c_nFullscreenFacingGap = 4;
const int c_nFullscreenPageBorder = 0;
const int c_nFullscreenPageShadow = 0;

const int c_nHourglassWidth = 15;
const int c_nHourglassHeight = 24;

const int s_nCursorHideDelay = 3500;


// IMPORTANT: There seems to be a problem with the way TranslateAccelerator
// interacts with MFC. When an accelerator corresponds to an item in a submenu
// (like "View"->"Go To"->"First Page", the OnInitMenuPopup function is called
// with the "View" menu as an argument, not with the "Go To" menu. As a result,
// the update handler for a menu item in the "Go To" submenu is not called,
// and if this item was disabled when the menu was last displayed, it will stay
// disabled, even if it is enabled on the toolbar. TranslateAccelerator doesn't
// send a WM_COMMAND message if a corresponding menu item is disabled, so the
// accelerator doesn't work as expected.
//
// To fix this, for every item on a submenu with an accelerator, a second
// command ID is introduced, and the accelerator table specifies this second ID
// (so that it is not attached to a menu). Note that there is no update handler
// for this second ID, so the command handler should take this into account.
//
// This applies to the following items: "First Page", "Previous Page",
// "Next Page", "Last Page", "Previous View" and "Next View"
// (because they might become disabled).

IMPLEMENT_DYNCREATE(CDjVuView, CMyScrollView)

BEGIN_MESSAGE_MAP(CDjVuView, CMyScrollView)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_VIEW_NEXTPAGE, OnViewNextpage)
	ON_COMMAND(ID_VIEW_PREVIOUSPAGE, OnViewPreviouspage)
	ON_COMMAND(ID_VIEW_NEXTPAGE_SHORTCUT, OnViewNextpage)
	ON_COMMAND(ID_VIEW_PREVIOUSPAGE_SHORTCUT, OnViewPreviouspage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEXTPAGE, OnUpdateViewNextpage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PREVIOUSPAGE, OnUpdateViewPreviouspage)
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_COMMAND_RANGE(ID_ZOOM_50, ID_ZOOM_CUSTOM, OnViewZoom)
	ON_UPDATE_COMMAND_UI_RANGE(ID_ZOOM_50, ID_ZOOM_CUSTOM, OnUpdateViewZoom)
	ON_COMMAND_RANGE(ID_ROTATE_LEFT, ID_ROTATE_180, OnViewRotate)
	ON_COMMAND(ID_VIEW_FIRSTPAGE, OnViewFirstpage)
	ON_COMMAND(ID_VIEW_LASTPAGE, OnViewLastpage)
	ON_COMMAND(ID_VIEW_FIRSTPAGE_SHORTCUT, OnViewFirstpage)
	ON_COMMAND(ID_VIEW_LASTPAGE_SHORTCUT, OnViewLastpage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LASTPAGE, OnUpdateViewNextpage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIRSTPAGE, OnUpdateViewPreviouspage)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_CANCELMODE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_COMMAND_RANGE(ID_LAYOUT_CONTINUOUS, ID_LAYOUT_FACING, OnViewLayout)
	ON_UPDATE_COMMAND_UI_RANGE(ID_LAYOUT_CONTINUOUS, ID_LAYOUT_FACING, OnUpdateViewLayout)
	ON_MESSAGE(WM_PAGE_RENDERED, OnPageRendered)
	ON_MESSAGE(WM_PAGE_DECODED, OnPageDecoded)
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_FIND_STRING, OnFindString)
	ON_COMMAND(ID_FIND_ALL, OnFindAll)
	ON_COMMAND(ID_ZOOM_IN, OnViewZoomIn)
	ON_COMMAND(ID_ZOOM_OUT, OnViewZoomOut)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_IN, OnUpdateViewZoomIn)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_OUT, OnUpdateViewZoomOut)
	ON_COMMAND(ID_VIEW_FULLSCREEN, OnViewFullscreen)
	ON_WM_MOUSEACTIVATE()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_PAGE_INFORMATION, OnPageInformation)
	ON_COMMAND_RANGE(ID_EXPORT_PAGE, ID_EXPORT_SELECTION, OnExportPage)
	ON_UPDATE_COMMAND_UI(ID_EXPORT_SELECTION, OnUpdateExportSelection)
	ON_COMMAND_RANGE(ID_MODE_DRAG, ID_MODE_ZOOM_RECT, OnChangeMode)
	ON_UPDATE_COMMAND_UI_RANGE(ID_MODE_DRAG, ID_MODE_ZOOM_RECT, OnUpdateMode)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND_RANGE(ID_DISPLAY_COLOR, ID_DISPLAY_FOREGROUND, OnViewDisplay)
	ON_UPDATE_COMMAND_UI_RANGE(ID_DISPLAY_COLOR, ID_DISPLAY_FOREGROUND, OnUpdateViewDisplay)
	ON_COMMAND(ID_VIEW_GOTO_PAGE, OnViewGotoPage)
	ON_WM_TIMER()
	ON_COMMAND(ID_LAYOUT_FIRSTPAGE_ALONE, OnFirstPageAlone)
	ON_COMMAND(ID_LAYOUT_RTL_ORDER, OnRightToLeftOrder)
	ON_UPDATE_COMMAND_UI(ID_LAYOUT_FIRSTPAGE_ALONE, OnUpdateFirstPageAlone)
	ON_UPDATE_COMMAND_UI(ID_LAYOUT_RTL_ORDER, OnUpdateRightToLeftOrder)
	ON_MESSAGE_VOID(WM_MOUSELEAVE, OnMouseLeave)
	ON_COMMAND_RANGE(ID_HIGHLIGHT_SELECTION, ID_HIGHLIGHT_TEXT, OnHighlight)
	ON_UPDATE_COMMAND_UI_RANGE(ID_HIGHLIGHT_SELECTION, ID_HIGHLIGHT_TEXT, OnUpdateHighlight)
	ON_COMMAND(ID_ANNOTATION_DELETE, OnDeleteAnnotation)
	ON_COMMAND(ID_ANNOTATION_EDIT, OnEditAnnotation)
	ON_COMMAND(ID_BOOKMARK_ADD, OnAddBookmark)
	ON_COMMAND(ID_ZOOM_TO_SELECTION, OnZoomToSelection)
	ON_COMMAND_RANGE(ID_NEXT_PANE, ID_PREV_PANE, OnSwitchFocus)
	ON_UPDATE_COMMAND_UI_RANGE(ID_NEXT_PANE, ID_PREV_PANE, OnUpdateSwitchFocus)
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(WM_SHOWPARENT, OnShowParent)
	ON_COMMAND(ID_VIEW_BACK, OnViewBack)
	ON_COMMAND(ID_VIEW_FORWARD, OnViewForward)
	ON_COMMAND(ID_VIEW_BACK_SHORTCUT, OnViewBack)
	ON_COMMAND(ID_VIEW_FORWARD_SHORTCUT, OnViewForward)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BACK, OnUpdateViewBack)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FORWARD, OnUpdateViewForward)
END_MESSAGE_MAP()

// CDjVuView construction/destruction

CDjVuView::CDjVuView()
	: m_nPage(-1), m_nPageCount(0), m_nZoomType(ZoomPercent), m_fZoom(100.0),
	  m_nLayout(SinglePage), m_nRotate(0), m_bDragging(false),
	  m_bDraggingRight(false), m_pSource(NULL), m_pRenderThread(NULL),
	  m_bInsideUpdateLayout(false), m_bInsideMouseMove(false), m_bClick(false),
	  m_nPendingPage(-1), m_nClickedPage(-1), m_nMode(Drag),
	  m_bHasSelection(false), m_nDisplayMode(Color), m_bShiftDown(false),
	  m_bNeedUpdate(false), m_bCursorHidden(false), m_bDraggingPage(false),
	  m_bDraggingText(false), m_bFirstPageAlone(false), m_bRightToLeft(false),
	  m_bDraggingMagnify(false), m_bControlDown(false), m_nType(Normal),
	  m_bHoverIsCustom(false), m_bDraggingRect(false), m_nSelectionPage(-1),
	  m_pHoverAnno(NULL), m_pClickedAnno(NULL), m_bDraggingLink(false),
	  m_bPopupMenu(false), m_bClickedCustom(false), m_bUpdateBitmaps(false)
{
	m_historyPoint = m_history.end();

	m_nMargin = c_nDefaultMargin;
	m_nShadowMargin = c_nDefaultShadowMargin;
	m_nPageGap = c_nDefaultPageGap;
	m_nFacingGap = c_nDefaultFacingGap;
	m_nPageBorder = c_nDefaultPageBorder;
	m_nPageShadow = c_nDefaultPageShadow;

	CreateSystemIconFont(m_sampleFont);

	// Note: screen DPI is reported by Windows through the screen DC.
	// However, the default value of 96 causes the common zoom levels
	// of 50%, 100%, 150% look bad for common 300dpi and 600dpi documents,
	// because the scaling factor will be non-integer. However, by
	// explicitely setting dpi value to 100, we can cause common zoom
	// levels to require integer subsampling, and the resulting images
	// will look better and will be rendered faster.
	//
	// CScreenDC dcScreen;
	// m_nScreenDPI = dcScreen.GetDeviceCaps(LOGPIXELSY);

	m_nScreenDPI = 100;
}

CDjVuView::~CDjVuView()
{
	for (map<int, HFONT>::iterator it = m_fonts.begin(); it != m_fonts.end(); ++it)
		::DeleteObject(it->second);

	DeleteBitmaps();

	m_dataLock.Lock();

	for (set<CDIB*>::iterator it = m_bitmaps.begin(); it != m_bitmaps.end(); ++it)
		delete *it;
	m_bitmaps.clear();

	m_dataLock.Unlock();

	if (m_pSource != NULL)
		m_pSource->Release();
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

	CRect rcViewport(CPoint(0, 0), GetViewportSize());

	CRect rcClip;
	pDC->GetClipBox(rcClip);
	rcClip.IntersectRect(CRect(rcClip), rcViewport);

	m_offscreenDC.Create(pDC, rcClip.Size());
	m_offscreenDC.SetViewportOrg(-rcClip.TopLeft());
	m_offscreenDC.IntersectClipRect(rcClip);

	CPoint ptScroll = GetScrollPosition();
	rcClip.OffsetRect(ptScroll);

	CRect rcIntersect;

	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		Page& page = m_pages[nPage];
		if (m_nLayout == SinglePage || m_nLayout == Facing)
		{
			if (nPage != m_nPage && !(m_nLayout == Facing && HasFacingPage(m_nPage) && nPage == m_nPage + 1))
				continue;
		}
		else if (m_nLayout == Continuous || m_nLayout == ContinuousFacing)
		{
			if (!rcIntersect.IntersectRect(m_pages[nPage].rcDisplay, rcClip))
				continue;
		}

		DrawPage(&m_offscreenDC, nPage);

		if (page.pBitmap != NULL && page.pBitmap->m_hObject != NULL)
		{
			int nSaveDC = pDC->SaveDC();
			pDC->IntersectClipRect(CRect(page.ptOffset, page.szBitmap) - ptScroll);

			// Draw annotations
			for (list<Annotation>::iterator lit = page.info.anno.begin(); lit != page.info.anno.end(); ++lit)
				DrawAnnotation(&m_offscreenDC, *lit, nPage, &(*lit) == m_pHoverAnno);

			// Draw custom annotations
			map<int, PageSettings>::iterator it = m_pSource->GetSettings()->pageSettings.find(nPage);
			if (it != m_pSource->GetSettings()->pageSettings.end())
			{
				PageSettings& pageSettings = (*it).second;
				for (list<Annotation>::iterator lit = pageSettings.anno.begin(); lit != pageSettings.anno.end(); ++lit)
					DrawAnnotation(&m_offscreenDC, *lit, nPage, &(*lit) == m_pHoverAnno);
			}

			// Draw selection
			for (GPosition pos = page.selection; pos; ++pos)
			{
				GRect rect = page.selection[pos]->rect;
				CRect rcText = TranslatePageRect(nPage, rect);
				m_offscreenDC.InvertRect(rcText - ptScroll);
			}

			// Draw transparent text on top of the image to assist dictionaries and
			// other programs that hook to TextOut to find text under mouse pointer.
			DrawTransparentText(&m_offscreenDC, nPage);

			pDC->RestoreDC(nSaveDC);
		}
	}

	// Draw rectangle selection
	if (m_nSelectionPage != -1 && m_rcSelection.width() > 1 && m_rcSelection.height() > 1)
	{
		// All selection corner points are inside m_rcSelection, so it
		// needs to be adjusted so that the drawn selection precisely
		// follows the mouse on screen. 
		GRect rcDeflated(m_rcSelection.xmin, m_rcSelection.ymin + 1,
				m_rcSelection.width() - 1, m_rcSelection.height() - 1);
		CRect rcSel = TranslatePageRect(m_nSelectionPage, rcDeflated);
		rcSel.InflateRect(0, 0, 1, 1);
		InvertFrame(&m_offscreenDC, rcSel - ptScroll);
	}

	rcClip.OffsetRect(-ptScroll);
	pDC->BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(),
			&m_offscreenDC, rcClip.left, rcClip.top, SRCCOPY);

	m_offscreenDC.Release();
}

void CDjVuView::DrawAnnotation(CDC* pDC, const Annotation& anno, int nPage, bool bActive)
{
	Page& page = m_pages[nPage];
	CPoint ptScroll = GetScrollPosition();
	CRect rcBounds = TranslatePageRect(nPage, anno.rectBounds) - ptScroll;

	bool bShowAll = m_bShiftDown && (GetFocus() == this || m_nType == Magnify);
	bool bShowBounds = false, bAdjustBorder = false;

	// Border
	if (bActive || bShowAll || !anno.bHideInactiveBorder || anno.bIsLine)
	{
		bShowBounds = true;

		if (!anno.points.empty())
		{
			vector<POINT> points(anno.points.size() + 1);
			for (size_t i = 0; i < anno.points.size(); ++i)
			{
				GRect rect(anno.points[i].first, anno.points[i].second, 1, 1);
				points[i] = CPoint(TranslatePageRect(nPage, rect).TopLeft() - ptScroll);
			}

			if (anno.bIsLine)
			{
				if (anno.points.size() == 2)
				{
					CPen penSolid(PS_SOLID, anno.nLineWidth, anno.crForeground);
					CPen* pOldPen = pDC->SelectObject(&penSolid);
					pDC->MoveTo(points[0]);
					pDC->LineTo(points[1]);

					CSize szPage = page.GetSize(m_nRotate);
					if (CPoint(points[0]) != points[1] && anno.bHasArrow && szPage.cy > 0)
					{
						CScreenDC dcScreen;
						double fZoom = 1.0*page.szBitmap.cy*page.info.nDPI/(szPage.cy*m_nScreenDPI);
						double fArrowLength = 10*fZoom;
						double fLength = pow(pow(1.0*points[1].x - points[0].x, 2.0)
								+ pow(1.0*points[1].y - points[0].y, 2.0), 0.5);
						double fBaseX = points[1].x + (points[0].x - points[1].x)*fArrowLength/fLength;
						double fBaseY = points[1].y + (points[0].y - points[1].y)*fArrowLength/fLength;
						double fTopX = fBaseX + (points[0].y - points[1].y)*0.5*fArrowLength/fLength;
						double fTopY = fBaseY + (points[1].x - points[0].x)*0.5*fArrowLength/fLength;
						double fBottomX = fBaseX + (points[1].y - points[0].y)*0.5*fArrowLength/fLength;
						double fBottomY = fBaseY + (points[0].x - points[1].x)*0.5*fArrowLength/fLength;

						pDC->MoveTo(points[1]);
						pDC->LineTo(static_cast<int>(fTopX + 0.5), static_cast<int>(fTopY + 0.5));
						pDC->MoveTo(points[1]);
						pDC->LineTo(static_cast<int>(fBottomX + 0.5), static_cast<int>(fBottomY + 0.5));
					}

					pDC->SelectObject(pOldPen);
				}
			}
			else if (anno.points.size() >= 3)
			{
				points.back() = points.front();
				if (anno.nBorderType == Annotation::BorderXOR
						|| bShowAll && anno.nBorderType == Annotation::BorderNone)
					InvertPolyFrame(pDC, &points[0], (int)points.size());
				else if (anno.nBorderType == Annotation::BorderSolid)
					FramePoly(pDC, &points[0], (int)points.size(), anno.crBorder);
			}
		}
		else if (anno.bOvalShape)
		{
			if (anno.nBorderType == Annotation::BorderXOR
					|| bShowAll && anno.nBorderType == Annotation::BorderNone)
				InvertOvalFrame(pDC, rcBounds);
			else if (anno.nBorderType == Annotation::BorderSolid)
				FrameOval(pDC, rcBounds, anno.crBorder);
		}
		else
		{
			if (anno.nBorderType == Annotation::BorderXOR
					|| bShowAll && anno.nBorderType == Annotation::BorderNone)
			{
				InvertFrame(pDC, rcBounds);
				bAdjustBorder = true;
			}
			else if (anno.nBorderType == Annotation::BorderSolid)
			{
				FrameRect(pDC, rcBounds, anno.crBorder);
				bAdjustBorder = true;
			}
		}
	}

	// Fill
	if (!anno.bOvalShape && (bActive || !anno.bHideInactiveFill))
	{
		CRect rcInside = rcBounds;
		if (bAdjustBorder)
			rcInside.DeflateRect(1, 1);

		for (size_t i = 0; i < anno.rects.size(); ++i)
		{
			CPoint ptScroll = GetScrollPosition();
			CRect rect = TranslatePageRect(nPage, anno.rects[i]) - ptScroll;
			if (!rect.IntersectRect(CRect(rect), rcInside))
				continue;

			// Fill
			if (anno.nFillType == Annotation::FillXOR)
				pDC->InvertRect(rect);
			else if (anno.nFillType == Annotation::FillSolid)
				HighlightRect(pDC, rect, anno.crFill, anno.fTransparency);
		}
	}

	if (bShowBounds)
	{
		if (anno.nBorderType == Annotation::BorderShadowIn
				|| anno.nBorderType == Annotation::BorderShadowOut)
		{
			int nBorderWidth = min(anno.nBorderWidth,
					min(rcBounds.Width() / 2, rcBounds.Height() / 2));
			bool bShadowIn = (anno.nBorderType == Annotation::BorderShadowIn);
			COLORREF crWhite = RGB(255, 255, 255);
			COLORREF crBlack = RGB(0, 0, 0);

			CPoint points1[] = {
					CPoint(rcBounds.left, rcBounds.top),
					CPoint(rcBounds.right - 1, rcBounds.top),
					CPoint(rcBounds.right - nBorderWidth, rcBounds.top + nBorderWidth - 1),
					CPoint(rcBounds.left + nBorderWidth - 1, rcBounds.top + nBorderWidth - 1),
					CPoint(rcBounds.left + nBorderWidth - 1, rcBounds.bottom - nBorderWidth),
					CPoint(rcBounds.left, rcBounds.bottom - 1) };
			HighlightPolygon(pDC, points1, 6, bShadowIn ? crBlack : crWhite, bShadowIn ? 0.85 : 0.65);

			CPoint points2[] = {
					CPoint(rcBounds.right - 1, rcBounds.bottom - 1),
					CPoint(rcBounds.left + 1, rcBounds.bottom - 1),
					CPoint(rcBounds.left + nBorderWidth, rcBounds.bottom - nBorderWidth),
					CPoint(rcBounds.right - nBorderWidth, rcBounds.bottom - nBorderWidth),
					CPoint(rcBounds.right - nBorderWidth, rcBounds.top + nBorderWidth),
					CPoint(rcBounds.right - 1, rcBounds.top + 1) };
			HighlightPolygon(pDC, points2, 6, bShadowIn ? crWhite : crBlack, bShadowIn ? 0.65 : 0.85);
		}
		else if (anno.nBorderType == Annotation::BorderEtchedIn
				|| anno.nBorderType == Annotation::BorderEtchedOut)
		{
			int nBorderWidth = min(anno.nBorderWidth,
					min(rcBounds.Width() / 2, rcBounds.Height() / 2) - 1);
			bool bEtchedIn = (anno.nBorderType == Annotation::BorderEtchedIn);
			COLORREF crWhite = RGB(255, 255, 255);
			COLORREF crBlack = RGB(0, 0, 0);

			CRect rcOuterLeft(rcBounds.left, rcBounds.top, rcBounds.left + 1, rcBounds.bottom);
			CRect rcOuterTop(rcBounds.left + 1, rcBounds.top, rcBounds.right, rcBounds.top + 1);
			CRect rcOuterRight(rcBounds.right - 1, rcBounds.top + 1, rcBounds.right, rcBounds.bottom);
			CRect rcOuterBottom(rcBounds.left + 1, rcBounds.bottom - 1, rcBounds.right - 1, rcBounds.bottom);

			CRect rcInner = rcBounds;
			rcInner.DeflateRect(nBorderWidth, nBorderWidth);
			CRect rcInnerLeft(rcInner.left, rcInner.top, rcInner.left + 1, rcInner.bottom);
			CRect rcInnerTop(rcInner.left + 1, rcInner.top, rcInner.right, rcInner.top + 1);
			CRect rcInnerRight(rcInner.right - 1, rcInner.top + 1, rcInner.right, rcInner.bottom);
			CRect rcInnerBottom(rcInner.left + 1, rcInner.bottom - 1, rcInner.right - 1, rcInner.bottom);

			COLORREF crShadow = bEtchedIn ? crBlack : crWhite;
			COLORREF crHighlight = bEtchedIn ? crWhite : crBlack;
			double fTranspShadow = bEtchedIn ? 0.85 : 0.65;
			double fTranspHighlight = bEtchedIn ? 0.65 : 0.85;

			HighlightRect(pDC, rcOuterLeft, crShadow, fTranspShadow);
			HighlightRect(pDC, rcOuterTop, crShadow, fTranspShadow);
			HighlightRect(pDC, rcOuterRight, crHighlight, fTranspHighlight);
			HighlightRect(pDC, rcOuterBottom, crHighlight, fTranspHighlight);

			HighlightRect(pDC, rcInnerLeft, crHighlight, fTranspHighlight);
			HighlightRect(pDC, rcInnerTop, crHighlight, fTranspHighlight);
			HighlightRect(pDC, rcInnerRight, crShadow, fTranspShadow);
			HighlightRect(pDC, rcInnerBottom, crShadow, fTranspShadow);
		}
	}

	if (anno.bAlwaysShowComment)
	{
		CRect rcText = rcBounds;
		rcText.DeflateRect(5, 3);

		pDC->SetTextColor(anno.crForeground);
		pDC->SetBkMode(TRANSPARENT);

		CSize szPage = page.GetSize(m_nRotate);
		if (szPage.cy > 0)
		{
			CFont curFont;
			double fZoom = 1.0*page.szBitmap.cy*page.info.nDPI/(szPage.cy*m_nScreenDPI);

			LOGFONT lf;
			m_sampleFont.GetLogFont(&lf);
			int nFontSize = static_cast<int>(lf.lfHeight*fZoom + (lf.lfHeight > 0 ? 0.5 : -0.5));

			// Find or create the font with this size.
			map<int, HFONT>::iterator it = m_fonts.find(nFontSize);
			if (it == m_fonts.end())
			{
				lf.lfHeight = nFontSize;
				HFONT hFont = ::CreateFontIndirect(&lf);
				it = m_fonts.insert(make_pair(nFontSize, hFont)).first;
			}

			HGDIOBJ hOldFont = ::SelectObject(pDC->m_hDC, it->second);
			pDC->SetTextCharacterExtra(0);
			pDC->DrawText(MakeCString(anno.strComment), rcText,
					DT_NOPREFIX | DT_LEFT | DT_TOP | DT_WORDBREAK | DT_END_ELLIPSIS);
			::SelectObject(pDC->m_hDC, hOldFont);
		}
	}
}

void CDjVuView::DrawPage(CDC* pDC, int nPage)
{
	Page& page = m_pages[nPage];

	COLORREF clrFrame = ::GetSysColor(COLOR_WINDOWFRAME);
	COLORREF clrBtnshadow = ::GetSysColor(COLOR_BTNSHADOW);
	COLORREF clrBtnface = ::GetSysColor(COLOR_BTNFACE);

	COLORREF clrWindow = ::GetSysColor(COLOR_WINDOW);
	if (m_displaySettings.bInvertColors)
		clrWindow = InvertColor(clrWindow);

	COLORREF clrShadow = ChangeBrightness(clrBtnshadow, 0.75);
	//COLORREF clrBackground = ChangeBrightness(clrBtnshadow, 1.2);
	COLORREF clrBackground = RGB(64, 64, 64);
	if (m_nType == Fullscreen || m_nType == Magnify && GetMainFrame()->IsFullscreenMode())
		clrBackground = RGB(0, 0, 0);

	CPoint ptScroll = GetScrollPosition();

	CRect rcClip;
	pDC->GetClipBox(rcClip);
	rcClip.OffsetRect(ptScroll);
	rcClip.IntersectRect(CRect(rcClip), page.rcDisplay);

	if (rcClip.IsRectEmpty())
		return;

	if (page.szBitmap.cx <= 0 || page.szBitmap.cy <= 0)
	{
		pDC->FillSolidRect(rcClip - ptScroll, clrBackground);
		return;
	}

	if (page.pBitmap == NULL || page.pBitmap->m_hObject == NULL)
	{
		// Draw white rect
		CRect rcPage(page.ptOffset, page.szBitmap);
		pDC->FillSolidRect(rcPage - ptScroll, clrWindow);

		if (!page.bBitmapRendered &&
				page.szBitmap.cx >= c_nHourglassWidth && page.szBitmap.cy >= c_nHourglassHeight)
		{
			// Draw hourglass
			CPoint pt((page.szBitmap.cx - c_nHourglassWidth) / 2, (page.szBitmap.cy - c_nHourglassHeight) / 2);
			m_hourglass.Draw(pDC, m_displaySettings.bInvertColors ? 1 : 0,
					CPoint(pt + page.ptOffset - ptScroll), ILD_NORMAL);
		}
	}
	else if (page.pBitmap->GetSize() == page.szBitmap)
	{
		// Draw offscreen bitmap
		CPoint ptPartOffset(max(rcClip.left - page.ptOffset.x, 0),
				max(rcClip.top - page.ptOffset.y, 0));

		CSize szPageClip = page.szBitmap - ptPartOffset;
		CSize szPart(min(rcClip.Width(), szPageClip.cx), min(rcClip.Height(), szPageClip.cy));

		if (szPart.cx > 0 && szPart.cy > 0)
		{
			CRect rcPart(ptPartOffset, szPart);
			page.pBitmap->DrawDC(pDC,
					page.ptOffset + ptPartOffset - ptScroll, rcPart);
		}
	}
	else
	{
		// Offscreen bitmap has yet wrong size, stretch it
		page.pBitmap->Draw(pDC, page.ptOffset - ptScroll, page.szBitmap);
	}

	CRect rcBorder(page.ptOffset, page.szBitmap);
	int nBorder = m_nPageBorder;
	while (nBorder-- > 0)
	{
		// Draw border
		rcBorder.InflateRect(1, 1);
		FrameRect(pDC, rcBorder - ptScroll, clrFrame);
	}

	if (m_nPageShadow > 0)
	{
		// Draw shadow
		CRect rcWhiteBar(CPoint(rcBorder.left, rcBorder.bottom),
				CSize(m_nPageShadow + 1, m_nPageShadow));
		pDC->FillSolidRect(rcWhiteBar - ptScroll, clrBackground);
		rcWhiteBar = CRect(CPoint(rcBorder.right, rcBorder.top),
				CSize(m_nPageShadow, m_nPageShadow + 1));
		pDC->FillSolidRect(rcWhiteBar - ptScroll, clrBackground);

		CRect rcShadow(CPoint(rcBorder.left + m_nPageShadow + 1, rcBorder.bottom),
				CSize(rcBorder.Width() - 1, m_nPageShadow));
		pDC->FillSolidRect(rcShadow - ptScroll, clrShadow);
		rcShadow = CRect(CPoint(rcBorder.right, rcBorder.top + m_nPageShadow + 1),
				CSize(m_nPageShadow, rcBorder.Height() - 1));
		pDC->FillSolidRect(rcShadow - ptScroll, clrShadow);

		rcBorder.InflateRect(0, 0, m_nPageShadow, m_nPageShadow);
	}

	// Fill everything else with backgroundColor
	int nSaveDC = pDC->SaveDC();
	pDC->IntersectClipRect(page.rcDisplay - ptScroll);
	pDC->ExcludeClipRect(rcBorder - ptScroll);
	pDC->FillSolidRect(rcClip - ptScroll, clrBackground);
	pDC->RestoreDC(nSaveDC);
}

void GetZones(DjVuTXT::Zone* pZone, int nZoneType, const GRect* rect, DjVuSelection& zones)
{
	GRect temp;
	for (GPosition pos = pZone->children; pos; ++pos)
	{
		DjVuTXT::Zone* pChild = &pZone->children[pos];
		if (rect != NULL && !temp.intersect(pChild->rect, *rect))
			continue;

		if (pChild->ztype == nZoneType)
			zones.append(pChild);
		else if (pChild->ztype < nZoneType)
			GetZones(pChild, nZoneType, rect, zones);
	}
}

void CDjVuView::DrawTransparentText(CDC* pDC, int nPage)
{
	Page& page = m_pages[nPage];
	if (page.info.pText == NULL)
		return;

	CPoint ptScroll = GetScrollPosition();

	CRect rcClip;
	pDC->GetClipBox(rcClip);
	rcClip.OffsetRect(ptScroll);
	rcClip.IntersectRect(CRect(rcClip), page.rcDisplay);

	// Hide the displayed text by using raster operations.
	pDC->SetROP2(R2_NOP);
	pDC->SetBkMode(TRANSPARENT);
	CPen penNull(PS_NULL, 0, RGB(0, 0, 0));
	CPen* pOldPen = pDC->SelectObject(&penNull);

	pDC->BeginPath();

	CPoint ptTopLeft = ScreenToDjVu(nPage, rcClip.TopLeft() - page.ptOffset, true);
	CPoint ptBottomRight = ScreenToDjVu(nPage, rcClip.BottomRight() - page.ptOffset, true);
	GRect rect(min(ptTopLeft.x, ptBottomRight.x), min(ptTopLeft.y, ptBottomRight.y),
			abs(ptTopLeft.x - ptBottomRight.x), abs(ptTopLeft.y - ptBottomRight.y));

	DjVuSelection zones;
	GetZones(&page.info.pText->page_zone, DjVuTXT::WORD, &rect, zones);

	for (GPosition pos = zones; pos; ++pos)
	{
		DjVuTXT::Zone* pZone = zones[pos];

		CString strWord = MakeCString(page.info.pText->textUTF8.substr(pZone->text_start, pZone->text_length));
		strWord.TrimLeft();
		strWord.TrimRight();
		if (strWord.IsEmpty())
			continue;

		CRect rcWord = TranslatePageRect(nPage, pZone->rect) - ptScroll;

		// Round font size up to the next even number to reduce the
		// amount of fonts created.
		int nFontSize = rcWord.Height() + (rcWord.Height() % 2);

		// Find or create the font with this size.
		map<int, HFONT>::iterator it = m_fonts.find(nFontSize);
		if (it == m_fonts.end())
		{
			LOGFONT lf;
			m_sampleFont.GetLogFont(&lf);
			lf.lfHeight = nFontSize;
			HFONT hFont = ::CreateFontIndirect(&lf);
			it = m_fonts.insert(make_pair(nFontSize, hFont)).first;
		}

		HGDIOBJ hOldFont = ::SelectObject(pDC->m_hDC, it->second);

		pDC->SetTextCharacterExtra(0);
		CSize szWordExtent = pDC->GetTextExtent(strWord);
		if (strWord.GetLength() > 1)
			pDC->SetTextCharacterExtra(static_cast<int>((1.0*rcWord.Width() - szWordExtent.cx)/(strWord.GetLength() - 1)));

		pDC->ExtTextOut(rcWord.left, rcWord.top, ETO_CLIPPED, rcWord, strWord + _T(" "), NULL);

		::SelectObject(pDC->m_hDC, hOldFont);
	}

	pDC->EndPath();

	// No need to do anything with the path: ExtTextOut has already been called.
	// pDC->StrokePath();

	pDC->SelectObject(pOldPen);
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
#endif


// CDjVuView message handlers

void CDjVuView::OnInitialUpdate()
{
	CMyScrollView::OnInitialUpdate();

	SetTimer(1, 100, NULL);
	SetTimer(2, 50, NULL);

	m_pSource = GetDocument()->GetSource();
	m_pSource->AddRef();

	CAppSettings* pAppSettings = theApp.GetAppSettings();
	DocSettings* pDocSettings = m_pSource->GetSettings();

	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_HOURGLASS);
	m_hourglass.Create(c_nHourglassWidth, c_nHourglassHeight, ILC_COLOR24 | ILC_MASK, 0, 1);
	m_hourglass.Add(&bitmap, RGB(192, 0, 32));

	m_toolTip.Create(this, TTS_ALWAYSTIP | TTS_NOPREFIX);
	m_toolTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);
	m_toolTip.Activate(false);

	m_nPageCount = m_pSource->GetPageCount();
	m_pages.resize(m_nPageCount);

	m_displaySettings = *theApp.GetDisplaySettings();

	m_pRenderThread = new CRenderThread(m_pSource, this);
	ShowCursor();

	theApp.AddObserver(this);
	pDocSettings->AddObserver(this);

	Invalidate();

	if (m_nType == Normal)
	{
		int nStartupPage = m_nPage;
		CPoint ptStartupOffset(0, 0);
		if (theApp.GetAppSettings()->bRestoreView)
		{
			nStartupPage = pDocSettings->nPage;
			ptStartupOffset = pDocSettings->ptOffset;
		}

		nStartupPage = max(0, min(m_nPageCount - 1, nStartupPage));
		bool bValidStartupPage = (nStartupPage == pDocSettings->nPage);

		m_nMode = pAppSettings->nDefaultMode;
		m_nLayout = pAppSettings->nDefaultLayout;
		m_bFirstPageAlone = pAppSettings->bFirstPageAlone;
		m_bRightToLeft = false;
		m_nZoomType = pAppSettings->nDefaultZoomType;
		m_fZoom = pAppSettings->fDefaultZoom;
		if (m_nZoomType == ZoomPercent)
			m_fZoom = 100.0;

		Page& page = m_pages[0];
		page.Init(m_pSource, 0, false, true);
		if (page.info.pAnt != NULL)
		{
			ReadZoomSettings(page.info.pAnt);
			ReadDisplayMode(page.info.pAnt);
		}

		// Restore zoom
		if (pAppSettings->bRestoreView && pDocSettings->nZoomType >= ZoomStretch && pDocSettings->nZoomType <= ZoomPercent)
		{
			m_nZoomType = pDocSettings->nZoomType;
			m_fZoom = max(min(pDocSettings->fZoom, 800.0), 10.0);
			pAppSettings->nDefaultZoomType = m_nZoomType;
			pAppSettings->fDefaultZoom = m_fZoom;
		}

		// Restore layout
		if (pAppSettings->bRestoreView && pDocSettings->nLayout >= SinglePage && pDocSettings->nLayout <= ContinuousFacing)
		{
			m_nLayout = pDocSettings->nLayout;
			m_bFirstPageAlone = pDocSettings->bFirstPageAlone;
			m_bRightToLeft = pDocSettings->bRightToLeft;
			theApp.GetAppSettings()->nDefaultLayout = m_nLayout;
			theApp.GetAppSettings()->bFirstPageAlone = m_bFirstPageAlone;
		}

		// Restore mode
		if (pAppSettings->bRestoreView && pDocSettings->nDisplayMode >= Color && pDocSettings->nDisplayMode <= Foreground)
			m_nDisplayMode = pDocSettings->nDisplayMode;

		// Restore rotate
		if (pAppSettings->bRestoreView)
			m_nRotate = pDocSettings->nRotate;

		if (m_nLayout == Continuous || m_nLayout == ContinuousFacing)
			UpdateLayout(RECALC);

		if (theApp.GetAppSettings()->bRestoreView && bValidStartupPage)
			ScrollToPage(nStartupPage, ptStartupOffset);
		else
			RenderPage(nStartupPage);
		AddHistoryPoint();

		pDocSettings->nZoomType = m_nZoomType;
		pDocSettings->fZoom = m_fZoom;
		pDocSettings->nDisplayMode = m_nDisplayMode;
		pDocSettings->nLayout = m_nLayout;
		pDocSettings->bFirstPageAlone = m_bFirstPageAlone;
		pDocSettings->bRightToLeft = m_bRightToLeft;
		pDocSettings->nRotate = m_nRotate;

		AddObserver(GetMainFrame());

		GetMDIChild()->GetThumbnailsView()->AddObserver(this);
		AddObserver(GetMDIChild()->GetThumbnailsView());
		if (GetMDIChild()->GetContentsTree() != NULL)
			GetMDIChild()->GetContentsTree()->AddObserver(this);
		if (GetMDIChild()->GetPageIndex() != NULL)
			GetMDIChild()->GetPageIndex()->AddObserver(this);
		if (GetMDIChild()->HasBookmarksTree())
			GetMDIChild()->GetBookmarksTree(false)->AddObserver(this);

		UpdateObservers(VIEW_INITIALIZED);
	}
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
		m_nZoomType = ZoomPercent;
		m_fZoom = max(min(pAnt->zoom, 800.0), 10.0);
	}
}

void CDjVuView::ReadDisplayMode(GP<DjVuANT> pAnt)
{
	switch (pAnt->mode)
	{
	case DjVuANT::MODE_FORE:
		m_nDisplayMode = Foreground;
		break;

	case DjVuANT::MODE_BACK:
		m_nDisplayMode = Background;
		break;

	case DjVuANT::MODE_BW:
		m_nDisplayMode = BlackAndWhite;
		break;
	}
}

CMainFrame* CDjVuView::GetMainFrame() const
{
	if (m_nType == Normal)
		return (CMainFrame*) GetTopLevelFrame();
	else if (m_nType == Fullscreen)
		return ((CFullscreenWnd*) GetTopLevelParent())->GetOwner()->GetMainFrame();
	else if (m_nType == Magnify)
		return ((CMagnifyWnd*) GetTopLevelParent())->GetOwner()->GetMainFrame();

	ASSERT(false);
	return GetMainWnd();
}

CMDIChild* CDjVuView::GetMDIChild() const
{
	const CDjVuView* pView = this;
	if (m_nType == Fullscreen)
	{
		pView = ((CFullscreenWnd*) GetTopLevelParent())->GetOwner();
	}
	else if (m_nType == Magnify)
	{
		pView = ((CMagnifyWnd*) GetTopLevelParent())->GetOwner();
		if (pView->m_nType == Fullscreen)
			pView = ((CFullscreenWnd*) pView->GetTopLevelParent())->GetOwner();
	}

	ASSERT(pView != NULL);
	CWnd* pMDIChild = pView->GetParent();
	while (pMDIChild != NULL && !pMDIChild->IsKindOf(RUNTIME_CLASS(CMDIChild)))
		pMDIChild = pMDIChild->GetParent();
	return (CMDIChild*) pMDIChild;
}

BOOL CDjVuView::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void CDjVuView::RenderPage(int nPage, int nTimeout, bool bUpdateWindow)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);

	if (m_nLayout == Facing || m_nLayout == ContinuousFacing)
		nPage = FixPageNumber(nPage);
	Page& page = m_pages[nPage];

	if (nTimeout != -1)
		InterlockedExchange(&m_nPendingPage, nPage);

	if (m_nLayout == SinglePage || m_nLayout == Facing)
	{
		bool bSamePage = (m_nPage == nPage);
		m_nPage = nPage;

		if (!page.info.bDecoded)
			page.Init(m_pSource, nPage);

		if (m_nLayout == Facing && nPage < m_nPageCount - 1)
		{
			Page& nextPage = m_pages[nPage + 1];
			if (!nextPage.info.bDecoded)
				nextPage.Init(m_pSource, nPage + 1);
		}

		UpdateLayout();
		ScrollToPosition(CPoint(GetScrollPosition().x, 0), bUpdateWindow && bSamePage);
	}
	else if (m_nLayout == Continuous || m_nLayout == ContinuousFacing)
	{
		UpdatePageSizes(page.ptOffset.y, 0, RECALC);

		int nUpdatedPos = page.ptOffset.y - m_nPageBorder - (nPage == 0 ? m_nMargin : min(m_nMargin, m_nPageGap));
		ScrollToPosition(CPoint(GetScrollPosition().x, nUpdatedPos), bUpdateWindow);
	}

	if (nTimeout != -1 && !m_pages[nPage].bBitmapRendered)
	{
		// Wait until the page is rendered and dispatch the message immediately.
		HANDLE hEvents[] = { 0 };
		if (::MsgWaitForMultipleObjects(0, hEvents, false, nTimeout, QS_SENDMESSAGE)
				== WAIT_OBJECT_0)
		{
			MSG msg;
			if (::PeekMessage(&msg, m_hWnd, WM_PAGE_RENDERED, WM_PAGE_RENDERED, PM_REMOVE))
				::DispatchMessage(&msg);
		}
	}
	InterlockedExchange(&m_nPendingPage, -1);

	if (bUpdateWindow)
		UpdateWindow();
}

void CDjVuView::ScrollToPage(int nPage, const CPoint& ptOffset, bool bMargin)
{
	RenderPage(nPage, -1, false);

	GRect rect(ptOffset.x, ptOffset.y, 1, 1);
	CPoint ptPos = TranslatePageRect(nPage, rect, true, false).TopLeft() - m_pages[nPage].ptOffset;

	CPoint ptPageOffset = m_pages[nPage].ptOffset;

	if (bMargin)
	{
		// Offset top-left to 1/10 of the viewport
		CSize szViewport = GetViewportSize();
		ptPos.x -= static_cast<int>(0.1*szViewport.cx + 0.5);
		ptPos.y -= static_cast<int>(0.1*szViewport.cy + 0.5);
	}
	else
	{
		if (ptPos.x == 0)
			ptPos.x = -m_nMargin;
		if (ptPos.y == 0)
		{
			ptPos.y = -m_nPageBorder;
			if (m_nType == Normal)
				ptPos.y -= (nPage == 0 ? m_nMargin : min(m_nMargin, m_nPageGap));
		}
	}

	if (m_nLayout == Continuous || m_nLayout == ContinuousFacing)
	{
		UpdatePageSizes(ptPageOffset.y, ptPos.y, RECALC);
		ptPageOffset = m_pages[nPage].ptOffset;
	}

	ScrollToPosition(ptPageOffset + ptPos, false);
	UpdateWindow();
}

void CDjVuView::DeleteBitmaps()
{
	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		m_pages[nPage].DeleteBitmap();
	}
}

void CDjVuView::UpdateLayout(UpdateType updateType)
{
	if (m_bInsideUpdateLayout)
		return;

	m_bInsideUpdateLayout = true;

	// Get the full client rect without the scrollbars.
	CSize szClient = ::GetClientSize(this);

	// Save page and offset to restore after changes
	int nAnchorPage = -1;
	CPoint ptAnchorOffset;
	CPoint ptDjVuOffset;
	CSize szPrevBitmap;

	if (m_nLayout == Continuous || m_nLayout == ContinuousFacing)
	{
		if (updateType == TOP)
		{
			nAnchorPage = CalcTopPage();
			ptAnchorOffset = GetScrollPosition() - m_pages[nAnchorPage].ptOffset;
		}
		else if (updateType == BOTTOM)
		{
			CSize szViewport = GetViewportSize();
			CPoint ptBottom = GetScrollPosition() + szViewport;

			int nPage = CalcTopPage();
			while (nPage < m_nPageCount - 1 &&
					ptBottom.y > m_pages[nPage].rcDisplay.bottom)
				++nPage;

			nAnchorPage = nPage;
			ptAnchorOffset = ptBottom - m_pages[nAnchorPage].ptOffset;
		}

		if (nAnchorPage != -1)
		{
			ptDjVuOffset = ScreenToDjVu(nAnchorPage, ptAnchorOffset, false);
			szPrevBitmap = m_pages[nAnchorPage].szBitmap;
		}
	}

	CSize szViewport;
	if (m_nLayout == SinglePage)
		szViewport = UpdateLayoutSinglePage(szClient);
	else if (m_nLayout == Facing)
		szViewport = UpdateLayoutFacing(szClient);
	else if (m_nLayout == Continuous)
		szViewport = UpdateLayoutContinuous(szClient);
	else if (m_nLayout == ContinuousFacing)
		szViewport = UpdateLayoutContinuousFacing(szClient);

	CSize szPage(szViewport.cx*9/10, szViewport.cy*9/10);
	CSize szLine(15, 15);
	SetScrollSizes(m_szDisplay, szPage, szLine, false);

	if (nAnchorPage != -1)
	{
		if (m_pages[nAnchorPage].szBitmap != szPrevBitmap)
		{
			GRect rect(ptDjVuOffset.x, ptDjVuOffset.y, 1, 1);
			CPoint ptNewOffset = TranslatePageRect(nAnchorPage, rect, true, false).TopLeft()
					- m_pages[nAnchorPage].ptOffset;
			if (ptAnchorOffset.x >= 0)
				ptAnchorOffset.x = ptNewOffset.x;
			if (ptAnchorOffset.y >= 0)
				ptAnchorOffset.y = ptNewOffset.y;
		}

		if (updateType == TOP)
			ScrollToPosition(m_pages[nAnchorPage].ptOffset + ptAnchorOffset, false);
		else if (updateType == BOTTOM)
			ScrollToPosition(m_pages[nAnchorPage].ptOffset + ptAnchorOffset - szViewport, false);
	}

	if (updateType != RECALC)
	{
		UpdateVisiblePages();
		InvalidateViewport();
	}

	m_bNeedUpdate = false;
	m_bInsideUpdateLayout = false;
}

CSize CDjVuView::UpdateLayoutSinglePage(const CSize& szClient)
{
	// Update all page sizes so that cached bitmaps will be of right size.
	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		CSize szViewport = szClient;
		bool bHScroll = false, bVScroll = false;

		CSize szDisplay;
		do
		{
			PreparePageRect(szViewport, nPage);
			szDisplay = m_pages[nPage].rcDisplay.Size()
					+ CSize(m_nMargin + m_nShadowMargin, m_nMargin + m_nShadowMargin);
		} while (AdjustViewportSize(szDisplay, szViewport, bHScroll, bVScroll));
	}

	Page& page = m_pages[m_nPage];
	page.ptOffset.Offset(m_nMargin, m_nMargin);
	page.rcDisplay.InflateRect(0, 0, m_nMargin + m_nShadowMargin, m_nMargin + m_nShadowMargin);
	m_szDisplay = page.rcDisplay.Size();

	// Get the client size for the final display size.
	CSize szViewport = szClient;
	bool bHScroll = false, bVScroll = false;
	while (AdjustViewportSize(m_szDisplay, szViewport, bHScroll, bVScroll))
		EMPTY_LOOP;

	// Center horizontally
	if (page.rcDisplay.Width() < szViewport.cx)
	{
		page.ptOffset.x += (szViewport.cx - page.rcDisplay.Width())/2;
		page.rcDisplay.right = szViewport.cx;
		m_szDisplay.cx = szViewport.cx;
	}

	// Center vertically
	if (page.rcDisplay.Height() < szViewport.cy)
	{
		page.ptOffset.y += (szViewport.cy - page.rcDisplay.Height())/2;
		page.rcDisplay.bottom = szViewport.cy;
		m_szDisplay.cy = szViewport.cy;
	}

	return szViewport;
}

CSize CDjVuView::UpdateLayoutFacing(const CSize& szClient)
{
	// Update all page sizes so that cached bitmaps will be of right size.
	for (int nPage = 0; nPage < m_nPageCount; nPage = GetNextPage(nPage))
	{
		CSize szViewport = szClient;
		bool bHScroll = false, bVScroll = false;

		CSize szDisplay;
		do
		{
			PreparePageRectFacing(szViewport, nPage);

			int nRightmostPage = (HasFacingPage(nPage) && !m_bRightToLeft ? nPage + 1 : nPage);
			szDisplay = m_pages[nRightmostPage].rcDisplay.BottomRight() +
					CSize(m_nMargin + m_nShadowMargin, m_nMargin + m_nShadowMargin);
		} while (AdjustViewportSize(szDisplay, szViewport, bHScroll, bVScroll));
	}

	Page* pPage;
	Page* pNextPage;
	GetFacingPages(m_nPage, pPage, pNextPage);

	pPage->ptOffset.Offset(m_nMargin, m_nMargin);
	pPage->rcDisplay.InflateRect(0, 0, m_nMargin, m_nMargin + m_nShadowMargin);

	if (pNextPage != NULL)
	{
		pNextPage->ptOffset.Offset(m_nMargin, m_nMargin);
		pNextPage->rcDisplay.OffsetRect(m_nMargin, 0);
		pNextPage->rcDisplay.InflateRect(0, 0, m_nShadowMargin, m_nMargin + m_nShadowMargin);
		m_szDisplay = pNextPage->rcDisplay.BottomRight();
	}
	else
	{
		pPage->rcDisplay.right += m_nShadowMargin;
		m_szDisplay = pPage->rcDisplay.BottomRight();
	}

	// Get the client size for the final display size.
	CSize szViewport = szClient;
	bool bHScroll = false, bVScroll = false;
	while (AdjustViewportSize(m_szDisplay, szViewport, bHScroll, bVScroll))
		EMPTY_LOOP;

	// Center horizontally
	if (m_szDisplay.cx < szViewport.cx)
	{
		pPage->ptOffset.x += (szViewport.cx - m_szDisplay.cx)/2;
		pPage->rcDisplay.InflateRect(0, 0, (szViewport.cx - m_szDisplay.cx)/2, 0);
		if (pNextPage != NULL)
		{
			pNextPage->ptOffset.x += (szViewport.cx - m_szDisplay.cx)/2;
			pNextPage->rcDisplay.OffsetRect((szViewport.cx - m_szDisplay.cx)/2, 0);
			pNextPage->rcDisplay.right = szViewport.cx;
		}
		else
		{
			pPage->rcDisplay.right = szViewport.cx;
		}

		m_szDisplay.cx = szViewport.cx;
	}

	// Center vertically
	if (m_szDisplay.cy < szViewport.cy)
	{
		pPage->ptOffset.y += (szViewport.cy - m_szDisplay.cy)/2;
		pPage->rcDisplay.bottom = szViewport.cy;
		if (pNextPage != NULL)
		{
			pNextPage->ptOffset.y += (szViewport.cy - m_szDisplay.cy)/2;
			pNextPage->rcDisplay.bottom = szViewport.cy;
		}

		m_szDisplay.cy = szViewport.cy;
	}

	return szViewport;
}

CSize CDjVuView::UpdateLayoutContinuous(const CSize& szClient)
{
	CSize szViewport = szClient;
	bool bHScroll = false, bVScroll = false;

	do
	{
		m_szDisplay = CSize(0, 0);
		int nTop = m_nMargin;

		for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		{
			PreparePageRect(szViewport, nPage);

			Page& page = m_pages[nPage];
			page.ptOffset.Offset(m_nMargin, nTop);
			page.rcDisplay.OffsetRect(0, nTop);
			page.rcDisplay.InflateRect(0, nPage == 0 ? m_nMargin : m_nPageGap,
					m_nMargin + m_nShadowMargin, nPage == m_nPageCount - 1 ? m_nShadowMargin : 0);

			nTop = page.rcDisplay.bottom + m_nPageGap;

			if (page.rcDisplay.Width() > m_szDisplay.cx)
				m_szDisplay.cx = page.rcDisplay.Width();
		}

		m_szDisplay.cy = m_pages.back().rcDisplay.bottom;
	} while (AdjustViewportSize(m_szDisplay, szViewport, bHScroll, bVScroll));

	// Get the client size for the final display size.
	szViewport = szClient;
	bHScroll = bVScroll = false;
	while (AdjustViewportSize(m_szDisplay, szViewport, bHScroll, bVScroll))
		EMPTY_LOOP;

	if (m_szDisplay.cx < szViewport.cx)
		m_szDisplay.cx = szViewport.cx;

	// Center pages horizontally
	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		Page& page = m_pages[nPage];
		if (page.rcDisplay.Width() < m_szDisplay.cx)
		{
			page.ptOffset.x += (m_szDisplay.cx - page.rcDisplay.Width()) / 2;
			page.rcDisplay.right = m_szDisplay.cx;
		}
	}

	// Center pages vertically
	if (m_szDisplay.cy < szViewport.cy)
	{
		int nOffset = (szViewport.cy - m_szDisplay.cy) / 2;
		for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		{
			Page& page = m_pages[nPage];
			page.rcDisplay.OffsetRect(0, nOffset);
			page.ptOffset.Offset(0, nOffset);
		}

		m_szDisplay.cy = szViewport.cy;

		m_pages[0].rcDisplay.top = 0;
		m_pages.back().rcDisplay.bottom = m_szDisplay.cy;
	}

	return szViewport;
}

CSize CDjVuView::UpdateLayoutContinuousFacing(const CSize& szClient)
{
	CSize szViewport = szClient;
	bool bHScroll = false, bVScroll = false;

	do
	{
		m_szDisplay = CSize(0, 0);
		int nTop = m_nMargin;

		for (int nPage = 0; nPage < m_nPageCount; nPage = GetNextPage(nPage))
		{
			PreparePageRectFacing(szViewport, nPage);

			Page* pPage;
			Page* pNextPage;
			GetFacingPages(nPage, pPage, pNextPage);

			pPage->ptOffset.Offset(m_nMargin, nTop);
			pPage->rcDisplay.OffsetRect(0, nTop);
			pPage->rcDisplay.InflateRect(0, nPage == 0 ? m_nMargin : m_nPageGap,
				m_nMargin, GetNextPage(nPage) >= m_nPageCount ? m_nShadowMargin : 0);

			if (pNextPage != NULL)
			{
				pNextPage->ptOffset.Offset(m_nMargin, nTop);
				pNextPage->rcDisplay.OffsetRect(m_nMargin, nTop);
				pNextPage->rcDisplay.InflateRect(0, nPage == 0 ? m_nMargin : m_nPageGap,
					m_nShadowMargin, GetNextPage(nPage) >= m_nPageCount ? m_nShadowMargin : 0);

				// Align pages horizontally
				int nWidthDelta = pNextPage->szBitmap.cx - pPage->szBitmap.cx;
				if (nWidthDelta > 0)
				{
					pPage->ptOffset.x += nWidthDelta;
					pPage->rcDisplay.right += nWidthDelta;
					pNextPage->ptOffset.x += nWidthDelta;
					pNextPage->rcDisplay.OffsetRect(nWidthDelta, 0);
				}
				else if (nWidthDelta < 0)
				{
					pNextPage->rcDisplay.right -= nWidthDelta;
				}

				if (pNextPage->rcDisplay.right > m_szDisplay.cx)
					m_szDisplay.cx = pNextPage->rcDisplay.right;
			}
			else
			{
				pPage->rcDisplay.right += m_nShadowMargin;

				if (pPage->rcDisplay.right > m_szDisplay.cx)
					m_szDisplay.cx = pPage->rcDisplay.right;
			}

			nTop = pPage->rcDisplay.bottom + m_nPageGap;
		}

		m_szDisplay.cy = m_pages.back().rcDisplay.bottom;
	} while (AdjustViewportSize(m_szDisplay, szViewport, bHScroll, bVScroll));

	// Get the client size for the final display size.
	szViewport = szClient;
	bHScroll = bVScroll = false;
	while (AdjustViewportSize(m_szDisplay, szViewport, bHScroll, bVScroll))
		EMPTY_LOOP;

	if (m_szDisplay.cx < szViewport.cx)
		m_szDisplay.cx = szViewport.cx;

	// Center pages horizontally
	for (int nPage = 0; nPage < m_nPageCount; nPage = GetNextPage(nPage))
	{
		Page* pPage;
		Page* pNextPage;
		GetFacingPages(nPage, pPage, pNextPage);

		if (pNextPage != NULL)
		{
			int nOffset = m_szDisplay.cx - pNextPage->rcDisplay.right;
			if (nOffset > 0)
			{
				pPage->ptOffset.x += nOffset / 2;
				pPage->rcDisplay.right += nOffset / 2;
				pNextPage->ptOffset.x += nOffset / 2;
				pNextPage->rcDisplay.OffsetRect(nOffset / 2, 0);
				pNextPage->rcDisplay.right = m_szDisplay.cx;
			}
		}
		else
		{
			int nOffset = m_szDisplay.cx - pPage->rcDisplay.right;
			if (nOffset > 0)
			{
				pPage->ptOffset.x += nOffset / 2;
				pPage->rcDisplay.right = m_szDisplay.cx;
			}
		}
	}

	// Center pages vertically
	if (m_szDisplay.cy < szViewport.cy)
	{
		int nOffset = (szViewport.cy - m_szDisplay.cy) / 2;
		for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		{
			Page& page = m_pages[nPage];
			page.rcDisplay.OffsetRect(0, nOffset);
			page.ptOffset.Offset(0, nOffset);
		}

		m_szDisplay.cy = szViewport.cy;

		m_pages[0].rcDisplay.top = 0;
		if (HasFacingPage(0))
			m_pages[1].rcDisplay.top = 0;

		m_pages[m_nPageCount - 1].rcDisplay.bottom = m_szDisplay.cy;
		if (m_nPageCount >= 2 && IsValidPage(m_nPageCount - 2) && HasFacingPage(m_nPageCount - 2))
			m_pages[m_nPageCount - 2].rcDisplay.bottom = m_szDisplay.cy;
	}

	return szViewport;
}

void CDjVuView::UpdatePagesCacheSingle(bool bUpdateImages,
		vector<int>& add, vector<int>& remove)
{
	ASSERT(m_nLayout == SinglePage);

	for (int nDiff = m_nPageCount; nDiff >= 0; --nDiff)
	{
		if (m_nPage - nDiff >= 0)
			UpdatePageCacheSingle(m_nPage - nDiff, bUpdateImages, add, remove);
		if (m_nPage + nDiff < m_nPageCount && nDiff != 0)
			UpdatePageCacheSingle(m_nPage + nDiff, bUpdateImages, add, remove);
	}
}

void CDjVuView::UpdatePagesCacheFacing(bool bUpdateImages,
		vector<int>& add, vector<int>& remove)
{
	ASSERT(m_nLayout == Facing);

	for (int nDiff = m_nPageCount; nDiff >= 0; --nDiff)
	{
		if (m_nPage - nDiff >= 0)
			UpdatePageCacheFacing(m_nPage - nDiff, bUpdateImages, add, remove);
		if (m_nPage + nDiff < m_nPageCount && nDiff != 0)
			UpdatePageCacheFacing(m_nPage + nDiff, bUpdateImages, add, remove);
	}
}

void CDjVuView::UpdatePageCache(const CSize& szViewport, int nPage, bool bUpdateImages,
		vector<int>& add, vector<int>& remove)
{
	// Pages visible on screen are put to the front of the rendering queue.
	// Pages which are within 2 screens from the view are put to the back
	// of the rendering queue.
	// Pages which are within 10 screens from the view are put to the back
	// of the decoding queue.

	int nTop = GetScrollPosition().y;
	Page& page = m_pages[nPage];

	if (!page.info.bDecoded)
	{
		if (m_nType == Magnify)
			return;

		m_pRenderThread->AddReadInfoJob(nPage);
	}
	else if (page.rcDisplay.top < nTop + 3*szViewport.cy &&
			 page.rcDisplay.bottom > nTop - 2*szViewport.cy)
	{
		if (page.pBitmap == NULL || page.szBitmap != page.pBitmap->GetSize() && bUpdateImages)
		{
			if (m_nType == Magnify)
				CopyBitmapFrom(((CMagnifyWnd*) GetTopLevelParent())->GetOwner(), nPage);

			m_pRenderThread->AddJob(nPage, m_nRotate, page.szBitmap, m_displaySettings, m_nDisplayMode);
			InvalidatePage(nPage);
		}
		add.push_back(nPage);
	}
	else
	{
		page.DeleteBitmap();

		if (m_nType != Magnify && (page.rcDisplay.top < nTop + 11*szViewport.cy
				&& page.rcDisplay.bottom > nTop - 10*szViewport.cy
				|| nPage == 0 || nPage == m_nPageCount - 1))
		{
			m_pRenderThread->AddDecodeJob(nPage);
			add.push_back(nPage);
		}
		else if (m_pSource->IsPageCached(nPage, this))
		{
			m_pRenderThread->AddCleanupJob(nPage);
			remove.push_back(nPage);
		}
		else
		{
			remove.push_back(nPage);
		}
	}
}

void CDjVuView::UpdatePageCacheSingle(int nPage, bool bUpdateImages,
		vector<int>& add, vector<int>& remove)
{
	// Current page and adjacent are rendered, next +- 9 pages are decoded.
	Page& page = m_pages[nPage];
	long nPageSize = page.szBitmap.cx * page.szBitmap.cy;

	if (!page.info.bDecoded)
	{
		if (m_nType == Magnify)
			return;

		m_pRenderThread->AddReadInfoJob(nPage);
	}
	else if (nPageSize < 3000000 && abs(nPage - m_nPage) <= 2 ||
			 abs(nPage - m_nPage) <= 1)
	{
		if (page.pBitmap == NULL || page.szBitmap != page.pBitmap->GetSize() && bUpdateImages)
		{
			if (m_nType == Magnify)
				CopyBitmapFrom(((CMagnifyWnd*) GetTopLevelParent())->GetOwner(), nPage);

			m_pRenderThread->AddJob(nPage, m_nRotate, page.szBitmap, m_displaySettings, m_nDisplayMode);
			InvalidatePage(nPage);
		}
		add.push_back(nPage);
	}
	else
	{
		page.DeleteBitmap();

		if (m_nType != Magnify && (abs(nPage - m_nPage) <= 10 || nPage == 0 || nPage == m_nPageCount - 1))
		{
			m_pRenderThread->AddDecodeJob(nPage);
			add.push_back(nPage);
		}
		else if (m_pSource->IsPageCached(nPage, this))
		{
			m_pRenderThread->AddCleanupJob(nPage);
			remove.push_back(nPage);
		}
		else
		{
			remove.push_back(nPage);
		}
	}
}

void CDjVuView::UpdatePageCacheFacing(int nPage, bool bUpdateImages,
		vector<int>& add, vector<int>& remove)
{
	// Current page and adjacent are rendered, next +- 9 pages are decoded.
	Page& page = m_pages[nPage];
	long nPageSize = page.szBitmap.cx * page.szBitmap.cy;

	if (!page.info.bDecoded)
	{
		if (m_nType == Magnify)
			return;

		m_pRenderThread->AddReadInfoJob(nPage);
	}
	else if (nPageSize < 1500000 && nPage >= m_nPage - 4 && nPage <= m_nPage + 5 ||
			 nPage >= m_nPage - 2 && nPage <= m_nPage + 3)
	{
		if (page.pBitmap == NULL || page.szBitmap != page.pBitmap->GetSize() && bUpdateImages)
		{
			if (m_nType == Magnify)
				CopyBitmapFrom(((CMagnifyWnd*) GetTopLevelParent())->GetOwner(), nPage);

			m_pRenderThread->AddJob(nPage, m_nRotate, page.szBitmap, m_displaySettings, m_nDisplayMode);
			InvalidatePage(nPage);
		}
		add.push_back(nPage);
	}
	else
	{
		page.DeleteBitmap();

		if (m_nType != Magnify && (abs(nPage - m_nPage) <= 10 || nPage == 0 || nPage == m_nPageCount - 1))
		{
			m_pRenderThread->AddDecodeJob(nPage);
			add.push_back(nPage);
		}
		else if (m_pSource->IsPageCached(nPage, this))
		{
			m_pRenderThread->AddCleanupJob(nPage);
			remove.push_back(nPage);
		}
		else
		{
			remove.push_back(nPage);
		}
	}
}

void CDjVuView::UpdatePagesCacheContinuous(bool bUpdateImages,
		vector<int>& add, vector<int>& remove)
{
	ASSERT(m_nLayout == Continuous || m_nLayout == ContinuousFacing);

	CRect rcViewport(CPoint(0, 0), GetViewportSize());

	int nTopPage = CalcTopPage();
	int nBottomPage = CalcBottomPage(nTopPage);

	for (int nDiff = m_nPageCount; nDiff >= 1; --nDiff)
	{
		if (nTopPage - nDiff >= 0)
			UpdatePageCache(rcViewport.Size(), nTopPage - nDiff, bUpdateImages, add, remove);
		if (nBottomPage + nDiff < m_nPageCount)
			UpdatePageCache(rcViewport.Size(), nBottomPage + nDiff, bUpdateImages, add, remove);
	}

	int nLastPage = m_nPage;
	int nMaxSize = -1;
	for (int nPage = nBottomPage; nPage >= nTopPage; --nPage)
	{
		UpdatePageCache(rcViewport.Size(), nPage, bUpdateImages, add, remove);

		CRect rcBitmap(m_pages[nPage].ptOffset, m_pages[nPage].szBitmap);
		CPoint ptScroll = GetScrollPosition();
		CRect rcIntersect;
		if (rcIntersect.IntersectRect(rcViewport, rcBitmap - ptScroll)
				&& rcIntersect.Width() * rcIntersect.Height() >= nMaxSize)
		{
			nLastPage = nPage;
			nMaxSize = rcIntersect.Width() * rcIntersect.Height();
		}
	}

	// Push to the front of the queue a page with the largest visible area
	UpdatePageCache(rcViewport.Size(), nLastPage, bUpdateImages, add, remove);
}

void CDjVuView::UpdateVisiblePages()
{
	if (m_nPageCount == 0 || m_pRenderThread == NULL || m_pRenderThread->IsPaused())
		return;

	m_pRenderThread->PauseJobs();
	m_pRenderThread->RemoveAllJobs();

	// Collect page numbera that we want to be in cache.
	vector<int> add, remove;
	add.reserve(m_nPageCount);
	remove.reserve(m_nPageCount);

	if (m_nLayout == SinglePage)
		UpdatePagesCacheSingle(m_bUpdateBitmaps, add, remove);
	else if (m_nLayout == Facing)
		UpdatePagesCacheFacing(m_bUpdateBitmaps, add, remove);
	else if (m_nLayout == Continuous || m_nLayout == ContinuousFacing)
		UpdatePagesCacheContinuous(m_bUpdateBitmaps, add, remove);

	// Notify the source so that it will keep the page in cache
	// for us in case another thread requests it.
	m_pSource->ChangeObservedPages(this, add, remove);

	m_pRenderThread->ResumeJobs();
}

CSize CDjVuView::CalcPageSize(const CSize& szBounds, const CSize& szPage, int nDPI) const
{
	return CalcPageSize(szBounds, szPage, nDPI, m_nZoomType);
}

CSize CDjVuView::CalcPageSize(const CSize& szBounds, const CSize& szPage, int nDPI, int nZoomType) const
{
	CSize szDisplay(1, 1);
	if (szPage.cx <= 0 || szPage.cy <= 0)
		return szDisplay;

	if (nZoomType <= ZoomFitWidth && nZoomType >= ZoomStretch)
	{
		int nFrame = m_nMargin + m_nShadowMargin + 2*m_nPageBorder + m_nPageShadow;
		CSize szFrame(nFrame, nFrame);

		return CalcPageBitmapSize(szBounds - szFrame, szPage, nZoomType);
	}
	else
	{
		CSize szDisplay;
		szDisplay.cx = static_cast<int>(szPage.cx*m_nScreenDPI*m_fZoom*0.01/nDPI + 0.5);
		szDisplay.cy = static_cast<int>(szPage.cy*m_nScreenDPI*m_fZoom*0.01/nDPI + 0.5);
		return szDisplay;
	}
}

CSize CDjVuView::CalcPageBitmapSize(const CSize& szBounds, const CSize& szPage, int nZoomType) const
{
	CSize szNewBounds(szBounds);
	szNewBounds.cx = max(szNewBounds.cx, 10);
	szNewBounds.cy = max(szNewBounds.cy, 10);

	CSize szDisplay(szNewBounds);

	switch (nZoomType)
	{
	case ZoomFitWidth:
		szDisplay.cy = static_cast<int>(1.0*szDisplay.cx*szPage.cy/szPage.cx + 0.5);
		break;

	case ZoomFitHeight:
		szDisplay.cx = static_cast<int>(1.0*szDisplay.cy*szPage.cx/szPage.cy + 0.5);
		break;

	case ZoomFitPage:
		szDisplay.cx = static_cast<int>(1.0*szDisplay.cy*szPage.cx/szPage.cy + 0.5);

		if (szDisplay.cx > szNewBounds.cx)
		{
			szDisplay.cx = szNewBounds.cx;
			szDisplay.cy = min(szNewBounds.cy, static_cast<int>(1.0*szDisplay.cx*szPage.cy/szPage.cx + 0.5));
		}
		break;

	case ZoomActualSize:
		szDisplay = szPage;
		break;

	case ZoomStretch:
		break;

	case ZoomPercent:
	default:
		ASSERT(false);
	}

	return szDisplay;
}

void CDjVuView::CalcPageSizeFacing(const CSize& szBounds,
		const CSize& szPage1, int nDPI1, CSize& szDisplay1,
		const CSize& szPage2, int nDPI2, CSize& szDisplay2) const
{
	CalcPageSizeFacing(szBounds, szPage1, nDPI1, szDisplay1, szPage2,
			nDPI2, szDisplay2, m_nZoomType);
}

void CDjVuView::CalcPageSizeFacing(const CSize& szBounds,
		const CSize& szPage1, int nDPI1, CSize& szDisplay1,
		const CSize& szPage2, int nDPI2, CSize& szDisplay2, int nZoomType) const
{
	bool bFirstPageOk = (szPage1.cx > 0 && szPage1.cy > 0);
	bool bSecondPageOk = (szPage2.cx > 0 && szPage2.cy > 0);

	if (!bFirstPageOk)
		szDisplay1 = CSize(1, 1);
	if (!bSecondPageOk)
		szDisplay2 = CSize(1, 1);
	if (!bFirstPageOk && !bSecondPageOk)
		return;

	if (nZoomType <= ZoomFitWidth && nZoomType >= ZoomStretch)
	{
		int nFrame = m_nMargin + 2*m_nPageBorder + m_nPageShadow + m_nShadowMargin;
		CSize szFrame(nFrame + m_nFacingGap + 2*m_nPageBorder + m_nPageShadow, nFrame);

		CSize szBitmapBounds = szBounds - szFrame;
		CSize szHalf(szBitmapBounds.cx / 2, szBitmapBounds.cy);

		if (bFirstPageOk)
			szDisplay1 = CalcPageBitmapSize(szHalf, szPage1, nZoomType);
		if (bSecondPageOk)
			szDisplay2 = CalcPageBitmapSize(szHalf, szPage2, nZoomType);
	}
	else
	{
		if (bFirstPageOk)
		{
			szDisplay1.cx = static_cast<int>(szPage1.cx*m_nScreenDPI*m_fZoom*0.01/nDPI1 + 0.5);
			szDisplay1.cy = static_cast<int>(szPage1.cy*m_nScreenDPI*m_fZoom*0.01/nDPI1 + 0.5);
		}

		if (bSecondPageOk)
		{
			szDisplay2.cx = static_cast<int>(szPage2.cx*m_nScreenDPI*m_fZoom*0.01/nDPI2 + 0.5);
			szDisplay2.cy = static_cast<int>(szPage2.cy*m_nScreenDPI*m_fZoom*0.01/nDPI2 + 0.5);
		}
	}
}

void CDjVuView::PreparePageRect(const CSize& szBounds, int nPage)
{
	// Calculates page rectangle, but neither adds borders,
	// nor centers the page in the view, and puts it at (0, 0)

	Page& page = m_pages[nPage];
	CSize szPage = page.GetSize(m_nRotate);

	page.ptOffset = CPoint(m_nPageBorder, m_nPageBorder);
	page.szBitmap = CalcPageSize(szBounds, szPage, page.info.nDPI);
	page.rcDisplay = CRect(CPoint(0, 0),
		page.szBitmap + CSize(2*m_nPageBorder + m_nPageShadow, 2*m_nPageBorder + m_nPageShadow));

	if (page.info.bDecoded)
		page.bHasSize = true;
}

void CDjVuView::PreparePageRectFacing(const CSize& szBounds, int nPage)
{
	// Calculates page rectangles, but neither adds borders,
	// nor centers pages in the view, and puts them at (0, 0)

	Page* pPage;
	Page* pNextPage;
	GetFacingPages(nPage, pPage, pNextPage);

	if (pNextPage != NULL)
	{
		CalcPageSizeFacing(szBounds,
				pPage->GetSize(m_nRotate), pPage->info.nDPI, pPage->szBitmap,
				pNextPage->GetSize(m_nRotate), pNextPage->info.nDPI, pNextPage->szBitmap);
	}
	else
	{
		CalcPageSizeFacing(szBounds,
				pPage->GetSize(m_nRotate), pPage->info.nDPI, pPage->szBitmap,
				pPage->GetSize(m_nRotate), pPage->info.nDPI, pPage->szBitmap);
	}

	pPage->ptOffset = CPoint(m_nPageBorder, m_nPageBorder);
	pPage->rcDisplay = CRect(CPoint(0, 0),
		pPage->szBitmap + CSize(2*m_nPageBorder + m_nPageShadow, 2*m_nPageBorder + m_nPageShadow));

	if (pNextPage != NULL)
	{
		pNextPage->ptOffset = CPoint(pPage->rcDisplay.right + m_nFacingGap + m_nPageBorder, m_nPageBorder);
		pNextPage->rcDisplay = CRect(CPoint(pPage->rcDisplay.right, 0),
			pNextPage->szBitmap + CSize(m_nFacingGap + 2*m_nPageBorder + m_nPageShadow, 2*m_nPageBorder + m_nPageShadow));

		// Align pages vertically
		if (pPage->rcDisplay.bottom < pNextPage->rcDisplay.bottom)
		{
			pPage->ptOffset.y += (pNextPage->rcDisplay.bottom - pPage->rcDisplay.bottom) / 2;
			pPage->rcDisplay.bottom = pNextPage->rcDisplay.bottom;
		}
		else if (pPage->rcDisplay.bottom > pNextPage->rcDisplay.bottom)
		{
			pNextPage->ptOffset.y += (pPage->rcDisplay.bottom - pNextPage->rcDisplay.bottom) / 2;
			pNextPage->rcDisplay.bottom = pPage->rcDisplay.bottom;
		}
	}
	else if (nPage == 0 && m_bFirstPageAlone && !m_bRightToLeft
			|| nPage == m_nPageCount - 1 && m_bRightToLeft)
	{
		// Display this page on the right
		pPage->ptOffset.x += pPage->szBitmap.cx + m_nFacingGap + 2*m_nPageBorder + m_nPageShadow;
		pPage->rcDisplay.right = 2*pPage->rcDisplay.Width() + m_nFacingGap;
	}
	else
	{
		pPage->rcDisplay.right = 2*pPage->rcDisplay.Width() + m_nFacingGap;
	}

	if (pPage->info.bDecoded)
		pPage->bHasSize = true;
	if (pNextPage != NULL && pNextPage->info.bDecoded)
		pNextPage->bHasSize = true;
}

void CDjVuView::OnViewNextpage()
{
	int nPage = GetNextPage(m_nPage);

	if (nPage < m_nPageCount)
		RenderPage(nPage);
	else
		OnViewLastpage();
}

void CDjVuView::OnUpdateViewNextpage(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsViewNextpageEnabled());
}

bool CDjVuView::IsViewNextpageEnabled()
{
	if (m_nLayout == SinglePage || m_nLayout == Facing)
		return GetNextPage(m_nPage) < m_nPageCount;
	else if (m_nLayout == Continuous || m_nLayout == ContinuousFacing)
		return GetScrollPosition().y < GetScrollLimit().cy;
	else
		return false;
}

void CDjVuView::OnViewPreviouspage()
{
	int nPage = m_nPage - 1;
	if (m_nLayout == Facing || m_nLayout == ContinuousFacing)
		--nPage;

	nPage = max(0, nPage);
	RenderPage(nPage);
}

void CDjVuView::OnUpdateViewPreviouspage(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsViewPreviouspageEnabled());
}

bool CDjVuView::IsViewPreviouspageEnabled() const
{
	if (m_nLayout == SinglePage || m_nLayout == Facing)
		return m_nPage > 0;
	else if (m_nLayout == Continuous || m_nLayout == ContinuousFacing)
		return GetScrollPosition().y > 0;
	else
		return false;
}

void CDjVuView::OnSize(UINT nType, int cx, int cy)
{
	if (cx > 0 && cy > 0 && m_nPageCount > 0 && !m_bInsideUpdateLayout)
	{
		UpdateLayout();
		UpdateView(true, false, false);
		UpdateWindow();

		CView::OnSize(nType, cx, cy);
		return;
	}

	CMyScrollView::OnSize(nType, cx, cy);
}

void CDjVuView::UpdateView(bool bUpdateSizes, bool bUpdatePages, bool bUpdateCursor)
{
	UpdatePageNumber();

	if (m_nLayout == Continuous || m_nLayout == ContinuousFacing)
	{
		if (bUpdateSizes)
			UpdatePageSizes(GetScrollPosition().y);

		if (bUpdatePages)
			UpdateVisiblePages();
	}

	if (!m_bDragging && !m_bPanning)
		UpdateHoverAnnotation();

	UpdateDragAction();

	if (bUpdateCursor)
		UpdateCursor();
}

void CDjVuView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CSize szScroll(0, 0);
	bool bScroll = true;
	bool bNextPage = false, bPrevPage = false;
	bool bAlignTop = false, bAlignBottom = false;

	switch (nChar)
	{
	case 'G':
	case 'g':
		GetTopLevelParent()->SendMessage(WM_COMMAND, ID_VIEW_GOTO_PAGE);
		return;

	case 'F':
	case 'f':
		GetTopLevelParent()->SendMessage(WM_COMMAND, ID_EDIT_FIND);
		return;

	case VK_HOME:
		OnViewFirstpage();
		return;

	case VK_END:
		OnViewLastpage();
		return;

	case VK_ADD:
		OnViewZoomIn();
		return;

	case VK_SUBTRACT:
		OnViewZoomOut();
		return;

	case VK_MULTIPLY:
		OnViewZoom(ID_ZOOM_FITPAGE);
		return;

	case VK_DOWN:
		szScroll.cy = 3*m_szLine.cy;
		bNextPage = !HasVertScrollBar();
		break;

	case VK_UP:
		szScroll.cy = -3*m_szLine.cy;
		bPrevPage = !HasVertScrollBar();
		break;

	case VK_RIGHT:
		szScroll.cx = 3*m_szLine.cx;
		bNextPage = !HasHorzScrollBar();
		break;

	case VK_LEFT:
		szScroll.cx = -3*m_szLine.cx;
		bPrevPage = !HasHorzScrollBar();
		break;

	case VK_NEXT:
	case VK_SPACE:
		szScroll.cy = m_szPage.cy;
		bNextPage = true;
		bAlignTop = true;
		break;

	case VK_PRIOR:
	case VK_BACK:
		szScroll.cy = -m_szPage.cy;
		bPrevPage = true;
		bAlignBottom = true;
		break;

	default:
		bScroll = false;
		break;
	}

	if (!bScroll)
	{
		CMyScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	if ((m_nLayout == Continuous || m_nLayout == ContinuousFacing) && szScroll.cy != 0)
	{
		int nExtra = 2*m_nPageBorder + m_nPageShadow + m_nPageGap;
		if (bAlignTop)
		{
			CSize szViewport = GetViewportSize();

			ASSERT(szScroll.cy > 0 && szScroll.cy <= szViewport.cy);
			UpdatePageSizes(GetScrollPosition().y, szViewport.cy + nExtra);

			// Scroll position could have been changed by UpdatePageSizes
			CPoint ptScroll = GetScrollPosition();

			int nTopPage = CalcTopPage();
			int nBottomPage = CalcBottomPage(nTopPage);
			const Page& page = m_pages[nBottomPage];

			// Jump to the top of the next page, if the bottom of the last visible page
			// is on screen, and only a small part of the next page is visible
			// (less than client height minus page scroll).
			if (nBottomPage > nTopPage && page.ptOffset.y > ptScroll.y + szScroll.cy)
			{
				szScroll.cy = page.ptOffset.y - ptScroll.y - m_nPageBorder - min(m_nMargin, m_nPageGap);
			}
			else if (nBottomPage < m_nPageCount - 1)
			{
				const Page& pageNext = m_pages[nBottomPage + 1];
				if (page.ptOffset.y + page.szBitmap.cy <= ptScroll.y + szViewport.cy)
				{
					szScroll.cy = pageNext.ptOffset.y - ptScroll.y - m_nPageBorder - min(m_nMargin, m_nPageGap);
				}
			}
		}
		else if (bAlignBottom)
		{
			CSize szViewport = GetViewportSize();

			ASSERT(szScroll.cy < 0 && szScroll.cy >= -szViewport.cy);
			UpdatePageSizes(GetScrollPosition().y, -szViewport.cy - nExtra);

			// Scroll position could have been changed by UpdatePageSizes
			CPoint ptScroll = GetScrollPosition();

			int nTopPage = CalcTopPage();
			const Page& page = m_pages[nTopPage];

			// Jump to the bottom of the previous page, if the top of the first page
			// is on screen, and only a small part of the previous page is visible
			// (less than client height minus page scroll).
			if (page.ptOffset.y + page.szBitmap.cy < ptScroll.y + szViewport.cy + szScroll.cy)
			{
				szScroll.cy = page.ptOffset.y + page.szBitmap.cy - szViewport.cy -
						ptScroll.y + m_nPageBorder + m_nPageShadow + m_nShadowMargin;
			}
			else if (nTopPage > 0)
			{
				const Page& pagePrev = m_pages[nTopPage - 1];
				if (page.ptOffset.y >= ptScroll.y)
				{
					szScroll.cy = pagePrev.ptOffset.y + pagePrev.szBitmap.cy - szViewport.cy -
							ptScroll.y + m_nPageBorder + m_nPageShadow + m_nShadowMargin;
				}
			}
		}
	}

	if (!OnScrollBy(szScroll))
	{
		if (bNextPage && IsViewNextpageEnabled())
		{
			OnViewNextpage();
		}
		else if (bPrevPage && IsViewPreviouspageEnabled())
		{
			CPoint ptScroll = GetScrollPosition();
			OnViewPreviouspage();

			if (m_nLayout == SinglePage || m_nLayout == Facing)
				OnScrollBy(CPoint(ptScroll.x, m_szDisplay.cy) - GetScrollPosition());
		}
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

	int nTopPage = CalcTopPage();
	const Page& page = m_pages[nTopPage];
	CSize szPage = page.GetSize(m_nRotate);

	if (szPage.cx <= 0 || szPage.cy <= 0)
		return 100.0;

	// Calculate zoom from display area size
	return 100.0*page.szBitmap.cx*page.info.nDPI/(szPage.cx*m_nScreenDPI);
}

double CDjVuView::GetZoom(ZoomType nZoomType) const
{
	int nTopPage = CalcTopPage();
	const Page& page = m_pages[nTopPage];
	CSize szPage = page.GetSize(m_nRotate);
	CSize szViewport = GetViewportSize();

	// Calculate zoom from display area size
	CSize szDisplay;

	if (m_nLayout == SinglePage || m_nLayout == Continuous)
	{
		szDisplay = CalcPageSize(szViewport, szPage, page.info.nDPI, nZoomType);
	}
	else if (m_nLayout == Facing || m_nLayout == ContinuousFacing)
	{
		const Page* pNextPage = (HasFacingPage(nTopPage) ? &m_pages[nTopPage + 1] : NULL);
		CSize szPage = page.GetSize(m_nRotate);

		if (pNextPage != NULL)
		{
			CSize szPage2 = pNextPage->GetSize(m_nRotate);
			CSize szDisplay2;
			CalcPageSizeFacing(szViewport, szPage, page.info.nDPI, szDisplay,
					szPage2, pNextPage->info.nDPI, szDisplay2, nZoomType);

			if ((szPage.cx <= 0 || szPage.cy <= 0) && szPage2.cx > 0 && szPage2.cy > 0)
				return 100.0*szDisplay2.cx*pNextPage->info.nDPI/(szPage2.cx*m_nScreenDPI);
		}
		else
		{
			CalcPageSizeFacing(szViewport, szPage, page.info.nDPI, szDisplay,
					szPage, page.info.nDPI, szDisplay, nZoomType);
		}
	}

	if (szPage.cx <= 0 || szPage.cy <= 0)
		return 100.0;

	return 100.0*szDisplay.cx*page.info.nDPI/(szPage.cx*m_nScreenDPI);
}

void CDjVuView::OnViewLayout(UINT nID)
{
	int nPrevLayout = m_nLayout;

	int nAnchorPage = m_nPage;
	CPoint ptOffset = GetScrollPosition() - m_pages[nAnchorPage].ptOffset;

	switch (nID)
	{
	case ID_LAYOUT_CONTINUOUS:
		if (m_nLayout == SinglePage)
			m_nLayout = Continuous;
		else if (m_nLayout == Facing)
			m_nLayout = ContinuousFacing;
		else if (m_nLayout == Continuous)
			m_nLayout = SinglePage;
		else if (m_nLayout == ContinuousFacing)
			m_nLayout = Facing;
		break;

	case ID_LAYOUT_FACING:
		if (m_nLayout == SinglePage)
			m_nLayout = Facing;
		else if (m_nLayout == Facing)
			m_nLayout = SinglePage;
		else if (m_nLayout == Continuous)
			m_nLayout = ContinuousFacing;
		else if (m_nLayout == ContinuousFacing)
			m_nLayout = Continuous;
		break;
	}

	if (m_nType == Normal)
	{
		theApp.GetAppSettings()->nDefaultLayout = m_nLayout;
		m_pSource->GetSettings()->nLayout = m_nLayout;
	}
	else if (m_nType == Fullscreen && !theApp.GetAppSettings()->bFullscreenContinuousScroll)
	{
		m_nLayout = (m_nLayout == Facing || m_nLayout == ContinuousFacing ? Facing : SinglePage);
	}

	if (m_nLayout != nPrevLayout)
		SetLayout(m_nLayout, nAnchorPage, ptOffset);
}

void CDjVuView::OnUpdateViewLayout(CCmdUI* pCmdUI)
{
	if (m_nType != Normal && m_nType != Fullscreen)
	{
		pCmdUI->Enable(false);
		return;
	}

	switch (pCmdUI->m_nID)
	{
	case ID_LAYOUT_CONTINUOUS:
		pCmdUI->SetCheck(m_nLayout == Continuous || m_nLayout == ContinuousFacing);
		break;

	case ID_LAYOUT_FACING:
		pCmdUI->SetCheck(m_nLayout == Facing || m_nLayout == ContinuousFacing);
		break;
	}
}

void CDjVuView::OnViewRotate(UINT nID)
{
	switch (nID)
	{
	case ID_ROTATE_LEFT:
		m_nRotate = (m_nRotate + 1) % 4;
		break;

	case ID_ROTATE_RIGHT:
		m_nRotate = (m_nRotate + 3) % 4;
		break;

	case ID_ROTATE_180:
		m_nRotate = (m_nRotate + 2) % 4;
		break;
	}

	if (m_nType == Normal)
		m_pSource->GetSettings()->nRotate = m_nRotate;

	DeleteBitmaps();
	m_pRenderThread->RejectCurrentJob();

	UpdateLayout();
	AlignTopPage(false);
	UpdateView(true, false);
	UpdateWindow();

	UpdateObservers(RotateChanged(m_nRotate));
}

void CDjVuView::OnViewFirstpage()
{
	AddHistoryPoint();

	if (m_nLayout == SinglePage || m_nLayout == Facing)
	{
		RenderPage(0);
	}
	else
	{
		ScrollToPosition(CPoint(GetScrollPosition().x, 0));
		UpdateWindow();
	}

	AddHistoryPoint();
}

void CDjVuView::OnViewLastpage()
{
	AddHistoryPoint();

	if (m_nLayout == SinglePage || m_nLayout == Facing)
	{
		RenderPage(m_nPageCount - 1);
	}
	else if (m_nLayout == Continuous || m_nLayout == ContinuousFacing)
	{
		CSize szViewport = GetViewportSize();
		UpdatePagesFromBottom(m_szDisplay.cy - szViewport.cy, m_szDisplay.cy);

		ScrollToPosition(CPoint(GetScrollPosition().x, GetScrollLimit().cy));
		UpdateWindow();
	}

	AddHistoryPoint();
}

void CDjVuView::OnSetFocus(CWnd* pOldWnd)
{
	CMyScrollView::OnSetFocus(pOldWnd);

	if (m_nType == Normal)
		UpdateObservers(VIEW_ACTIVATED);

	m_nClickCount = 0;
}

void CDjVuView::OnKillFocus(CWnd* pNewWnd)
{
	CMyScrollView::OnKillFocus(pNewWnd);

	StopDragging();
}

void CDjVuView::OnCancelMode()
{
	CMyScrollView::OnCancelMode();

	StopDragging();
}

void CDjVuView::ZoomTo(int nZoomType, double fZoom, bool bRedraw)
{
	if (nZoomType < ZoomStretch || nZoomType > ZoomPercent)
	{
		nZoomType = ZoomPercent;
		fZoom = 100.0;
	}

	m_nZoomType = nZoomType;
	m_fZoom = fZoom;

	if (m_nZoomType == ZoomPercent)
	{
		// Leave two digits after decimal point
		m_fZoom = static_cast<int>(m_fZoom*100.0)*0.01;
		m_fZoom = max(min(m_fZoom, 800.0), 10.0);
	}

	if (m_nType == Normal)
	{
		theApp.GetAppSettings()->nDefaultZoomType = nZoomType;
		theApp.GetAppSettings()->fDefaultZoom = fZoom;
		m_pSource->GetSettings()->nZoomType = nZoomType;
		m_pSource->GetSettings()->fZoom = fZoom;
	}

	UpdateLayout();
	AlignTopPage(false);
	UpdateView(true, false);

	if (bRedraw)
		UpdateWindow();

	UpdateObservers(ZOOM_CHANGED);
}

void CDjVuView::AlignTopPage(bool bRepaint, int nPage)
{
	if (m_nLayout != Continuous && m_nLayout != ContinuousFacing)
		return;

	if (nPage == -1)
		nPage = CalcTopPage();

	int nWidth, nLeft;
	if (m_nLayout == Continuous)
	{
		const Page& page = m_pages[nPage];
		nWidth = page.szBitmap.cx + m_nMargin + m_nShadowMargin + 2*m_nPageBorder + m_nPageShadow;
		nLeft = page.ptOffset.x - m_nPageBorder - m_nMargin;
	}
	else if (m_nLayout == ContinuousFacing)
	{
		Page* pPage;
		Page* pNextPage;
		GetFacingPages(nPage, pPage, pNextPage);

		if (pPage != NULL && pNextPage != NULL)
		{
			nLeft = min(pPage->ptOffset.x, pNextPage->ptOffset.x) - m_nPageBorder - m_nMargin;
			nWidth = pPage->szBitmap.cx + pNextPage->szBitmap.cx + m_nMargin
					+ m_nShadowMargin + m_nFacingGap + 4*m_nPageBorder + 2*m_nPageShadow;
		}
		else
		{
			nLeft = pPage->ptOffset.x - m_nPageBorder - m_nMargin;
			nWidth = 2*pPage->szBitmap.cx + m_nMargin + m_nShadowMargin + m_nFacingGap
					+ 4*m_nPageBorder + 2*m_nPageShadow;

			if (nPage == 0 && m_bFirstPageAlone && !m_bRightToLeft
					|| nPage == m_nPageCount - 1 && m_bRightToLeft)
			{
				// This page is displayed on the right
				nLeft -= pPage->szBitmap.cx + 2*m_nPageBorder + m_nFacingGap + m_nPageShadow;
			}
		}
	}

	CPoint ptScroll = GetScrollPosition();
	CSize szViewport = GetViewportSize();
	if (nWidth < szViewport.cx)
		ptScroll.x = nLeft - (szViewport.cx - nWidth)/2;
	else if (ptScroll.x < nLeft)
		ptScroll.x = nLeft;
	else if (ptScroll.x + szViewport.cx > nLeft + nWidth)
		ptScroll.x = nLeft + nWidth - szViewport.cx;

	ScrollToPosition(ptScroll, bRepaint);
}

BOOL CDjVuView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_bPanning)
		return true;

	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	CRect rcViewport(CPoint(0, 0), GetViewportSize());

	if (nHitTest != HTCLIENT || !rcViewport.PtInRect(ptCursor))
		return CMyScrollView::OnSetCursor(pWnd, nHitTest, message);

	if (hCursorHand == NULL)
		hCursorHand = theApp.LoadCursor(IDC_CURSOR_HAND);
	if (hCursorDrag == NULL)
		hCursorDrag = theApp.LoadCursor(IDC_CURSOR_DRAG);
	if (hCursorLink == NULL)
		hCursorLink = ::LoadCursor(0, IDC_HAND);
	if (hCursorLink == NULL)
		hCursorLink = theApp.LoadCursor(IDC_CURSOR_LINK);
	if (hCursorText == NULL)
		hCursorText = ::LoadCursor(0, IDC_IBEAM);
	if (hCursorMagnify == NULL)
		hCursorMagnify = theApp.LoadCursor(IDC_CURSOR_MAGNIFY);
	if (hCursorCross == NULL)
		hCursorCross = ::LoadCursor(0, IDC_CROSS);
	if (hCursorZoomRect == NULL)
		hCursorZoomRect = theApp.LoadCursor(IDC_CURSOR_ZOOM_RECT);

	int nPage = GetPageFromPoint(ptCursor);

	HCURSOR hCursor = NULL;
	if (hCursor == NULL && (m_bDraggingMagnify || m_nType == Magnify))
		hCursor = hCursorMagnify;
	if (hCursor == NULL && m_bDraggingPage)
		hCursor = hCursorDrag;
	if (hCursor == NULL && m_bDraggingText)
		hCursor = hCursorText;
	if (hCursor == NULL && m_bDraggingRect)
		hCursor = (m_nMode == ZoomRect ? hCursorZoomRect : hCursorCross);
	if (hCursor == NULL && m_bDraggingLink)
		hCursor = hCursorLink;

	if (hCursor == NULL && m_nMode == MagnifyingGlass)
		hCursor = hCursorMagnify;
	if (hCursor == NULL && m_pHoverAnno != NULL && m_pHoverAnno->strURL.length() != 0)
		hCursor = hCursorLink;
	if (hCursor == NULL && m_nMode == Drag && HasScrollBars())
		hCursor = m_bDragging ? hCursorDrag : hCursorHand;
	if (hCursor == NULL && m_nMode == SelectRect)
		hCursor = hCursorCross;
	if (hCursor == NULL && m_nMode == ZoomRect)
		hCursor = hCursorZoomRect;
	if (hCursor == NULL && m_nMode == Select && nPage != -1)
	{
		const Page& page = m_pages[nPage];
		CPoint ptScroll = GetScrollPosition();
		CRect rcBitmap = CRect(page.ptOffset, page.szBitmap) - ptScroll;
		if (rcBitmap.PtInRect(ptCursor) && m_pages[nPage].info.bHasText)
			hCursor = hCursorText;
	}

	if (hCursor != NULL)
	{
		SetCursor(hCursor);
		return true;
	}

	return CMyScrollView::OnSetCursor(pWnd, nHitTest, message);
}

void CDjVuView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_bDraggingRight)
		return;

	if (m_bDragging)
	{
		StopDragging();
		m_nCursorTime = ::GetTickCount();
	}

	m_bClick = true;
	++m_nClickCount;
	::GetCursorPos(&m_ptClick);

	if (m_bControlDown || (m_nMode == MagnifyingGlass && !m_bShiftDown))
	{
		UpdateHoverAnnotation(CPoint(-1, -1));

		m_bDragging = true;
		m_bDraggingMagnify = true;

		m_ptPrevCursor = CPoint(-1, -1);

		StartMagnify();
		UpdateMagnifyWnd(true);

		SetCapture();
		ShowCursor();
		UpdateCursor();
	}
	else if (m_pHoverAnno != NULL && m_pHoverAnno->strURL.length() != 0)
	{
		m_bDragging = true;
		m_bDraggingLink = true;

		m_nStartPage = GetPageNearPoint(point);
		if (m_nStartPage != -1)
		{
			m_ptStartPos = GetScrollPosition() - m_pages[m_nStartPage].ptOffset;
			::GetCursorPos(&m_ptStart);

			if (m_nMode == Select)
			{
				m_nPrevPage = m_nStartPage;
				m_nSelStartPos = GetTextPosFromPoint(m_nStartPage, point);
			}
			else if (m_nMode == SelectRect || m_nMode == ZoomRect)
			{
				Page& page = m_pages[m_nStartPage];
				CPoint pt = point + GetScrollPosition() - page.ptOffset;
				m_ptStartSel = ScreenToDjVu(m_nStartPage, pt);
			}
		}

		SetCapture();
		ShowCursor();
		UpdateCursor();
	}
	else if ((m_bShiftDown || m_nMode == Drag) && HasScrollBars())
	{
		UpdateHoverAnnotation(CPoint(-1, -1));

		m_nStartPage = GetPageNearPoint(point);
		if (m_nStartPage == -1)
			return;

		m_bDragging = true;
		m_bDraggingPage = true;

		m_ptStartPos = GetScrollPosition() - m_pages[m_nStartPage].ptOffset;
		::GetCursorPos(&m_ptStart);

		SetCapture();
		ShowCursor();
		UpdateCursor();
	}
	else if (m_nMode == Select)
	{
		UpdateHoverAnnotation(CPoint(-1, -1));
		ClearSelection();

		m_nStartPage = GetPageNearPoint(point);
		if (m_nStartPage == -1)
			return;

		m_bDragging = true;
		m_bDraggingText = true;

		m_nPrevPage = m_nStartPage;
		m_nSelStartPos = GetTextPosFromPoint(m_nStartPage, point);

		SetCapture();
		ShowCursor();
		UpdateCursor();
	}
	else if (m_nMode == SelectRect || m_nMode == ZoomRect)
	{
		UpdateHoverAnnotation(CPoint(-1, -1));
		ClearSelection();

		m_nStartPage = GetPageNearPoint(point);
		if (m_nStartPage == -1)
			return;

		Page& page = m_pages[m_nStartPage];
		CPoint pt = point + GetScrollPosition() - page.ptOffset;
		m_ptStartSel = ScreenToDjVu(m_nStartPage, pt);

		m_nSelectionPage = m_nStartPage;
		m_rcSelection.xmin = m_ptStartSel.x;
		m_rcSelection.xmax = m_ptStartSel.x + 1;
		m_rcSelection.ymin = m_ptStartSel.y;
		m_rcSelection.ymax = m_ptStartSel.y + 1;

		m_bDragging = true;
		m_bDraggingRect = true;

		SetCapture();
		ShowCursor();
		UpdateCursor();
	}
	else if (m_nMode == NextPrev && !m_bShiftDown)
	{
		UpdateHoverAnnotation(CPoint(-1, -1));

		m_bDragging = true;
		SetCapture();

		OnViewNextpage();
	}

	CMyScrollView::OnLButtonDown(nFlags, point);
}

CPoint CDjVuView::ScreenToDjVu(int nPage, const CPoint& point, bool bClip)
{
	Page& page = m_pages[nPage];
	if (!page.info.bDecoded)
		page.Init(m_pSource, nPage);
	CSize szPage = page.GetSize(m_nRotate);

	double fRatioX = szPage.cx / (1.0*page.szBitmap.cx);
	double fRatioY = szPage.cy / (1.0*page.szBitmap.cy);

	CPoint ptResult(static_cast<int>(point.x*fRatioX + 0.5),
			static_cast<int>(szPage.cy - 1 - point.y*fRatioY + 0.5));

	GRect output(0, 0, page.info.szPage.cx, page.info.szPage.cy);

	int nRotate = (m_nRotate + page.info.nInitialRotate) % 4;
	if (nRotate != 0)
	{
		GRect rect(ptResult.x, ptResult.y, 1, 1);

		GRect input(0, 0, szPage.cx, szPage.cy);

		if ((page.info.nInitialRotate % 2) != 0)
			swap(output.xmax, output.ymax);

		GRectMapper mapper;
		mapper.clear();
		mapper.set_input(input);
		mapper.set_output(output);
		mapper.rotate(4 - nRotate);
		mapper.map(rect);

		ptResult = CPoint(rect.xmin, rect.ymin);
	}

	if (bClip)
	{
		ptResult.x = max(min(ptResult.x, output.xmax - 1), 0);
		ptResult.y = max(min(ptResult.y, output.ymax - 1), 0);
	}

	return ptResult;
}

void CDjVuView::GetTextPos(const DjVuTXT::Zone& zone, const CPoint& pt,
		int& nPos, double& fBest, bool bReturnBlockStart) const
{
	if (!zone.children.isempty())
	{
		for (GPosition pos = zone.children; pos; ++pos)
			GetTextPos(zone.children[pos], pt, nPos, fBest, bReturnBlockStart);
		return;
	}

	CPoint ptDiff(0, 0);
	if (zone.rect.xmin > pt.x)
		ptDiff.x = zone.rect.xmin - pt.x;
	else if (zone.rect.xmax <= pt.x)
		ptDiff.x = zone.rect.xmax - pt.x - 1;

	if (zone.rect.ymax <= pt.y)
		ptDiff.y = pt.y - zone.rect.ymax + 1;
	else if (zone.rect.ymin > pt.y)
		ptDiff.y = pt.y - zone.rect.ymin;

	double fDistance = pow(pow(ptDiff.x, 2.0) + pow(ptDiff.y, 2.0), 0.5);
	if (fDistance < fBest)
	{
		fBest = fDistance;
		if (!bReturnBlockStart && (ptDiff.x < 0 || ptDiff.x == 0 && ptDiff.y < 0))
		{
			const DjVuTXT::Zone* pZone = &zone;
			const DjVuTXT::Zone* pParent = pZone->get_parent();
			while (pParent != NULL && &pParent->children[pParent->children.lastpos()] == pZone)
			{
				pZone = pParent;
				pParent = pParent->get_parent();
			}

			nPos = pZone->text_start + pZone->text_length;
		}
		else
			nPos = zone.text_start;
	}
}

int CDjVuView::GetTextPosFromPoint(int nPage, const CPoint& point, bool bReturnBlockStart)
{
	Page& page = m_pages[nPage];
	if (page.info.bHasText && !page.info.bTextDecoded)
		page.Init(m_pSource, nPage, true);

	if (page.info.pText == NULL)
		return 0;

	CPoint ptPage = point + GetScrollPosition() - page.ptOffset;
	CPoint ptDjVu = ScreenToDjVu(nPage, ptPage);

	int nPos = -1;
	double fBest = 1e10;
	GetTextPos(page.info.pText->page_zone, ptDjVu, nPos, fBest, bReturnBlockStart);

	if (nPos == -1)
		nPos = page.info.pText->textUTF8.length();

	return nPos;
}

void CDjVuView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDraggingRight)
		return;

	GUTF8String strURL;

	if (m_bDraggingLink && m_pHoverAnno != NULL)
	{
		Annotation* pClickedAnno = m_pHoverAnno;

		UpdateHoverAnnotation(point);

		if (pClickedAnno == m_pHoverAnno && pClickedAnno->strURL.length() != 0)
			strURL = pClickedAnno->strURL;
	}

	StopDragging();
	m_nCursorTime = ::GetTickCount();

	if (m_bClick)
	{
		m_bClick = false;
		if (m_nClickCount > 1)
			ClearSelection();
	}

	UpdateHoverAnnotation(point);
	CMyScrollView::OnLButtonUp(nFlags, point);

	// GoToURL may result in destruction of this, if we are
	// in a fullscreen view. So make it the very last call.
	if (strURL.length() > 0)
		GoToURL(strURL);
}

void CDjVuView::StopDragging()
{
	if (m_bDragging || m_bDraggingRight)
	{
		m_bDragging = false;
		m_bDraggingRight = false;
		m_bDraggingPage = false;
		m_bDraggingText = false;
		m_bDraggingLink = false;

		if (m_bDraggingMagnify)
		{
			GetMainFrame()->GetMagnifyWnd()->Hide();
			m_bDraggingMagnify = false;
		}

		if (m_bDraggingRect)
		{
			if (m_rcSelection.width() <= 1 || m_rcSelection.height() <= 1)
				m_nSelectionPage = -1;
			m_bDraggingRect = false;

			if (m_nMode == ZoomRect && m_nSelectionPage != -1)
			{
				OnZoomToSelection();

				CPoint ptScroll = GetScrollPosition();
				CRect rcDisplay = TranslatePageRect(m_nSelectionPage, m_rcSelection) - ptScroll;
				rcDisplay.InflateRect(0, 0, 1, 1);
				InvalidateRect(rcDisplay, false);

				m_nSelectionPage = -1;
			}
		}

		ReleaseCapture();
	}
}

void CDjVuView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bInsideMouseMove)
		return;

	m_bInsideMouseMove = true;

	if (point != m_ptMouse)
	{
		m_ptMouse = point;
		ShowCursor();
	}

	if (m_bClick)
	{
		CPoint ptCursor;
		::GetCursorPos(&ptCursor);
		if (m_ptClick != ptCursor)
			m_bClick = false;
	}

	if (!m_bDragging && !m_bDraggingRight && !m_bPanning)
	{
		UpdateHoverAnnotation(point);
		m_bInsideMouseMove = false;
		return;
	}

	CPoint ptCursor;
	GetCursorPos(&ptCursor);
	if (m_bDraggingRight && !m_bDraggingPage && m_nStartPage != -1 && HasScrollBars()
			&& (abs(ptCursor.x - m_ptStart.x) >= 2 || abs(ptCursor.y - m_ptStart.y) >= 2))
	{
		m_bDraggingPage = true;
		UpdateHoverAnnotation(CPoint(-1, -1));
		UpdateCursor();
	}
	else if (m_bDraggingLink && m_nStartPage != -1
			&& (abs(ptCursor.x - m_ptStart.x) >= 2 || abs(ptCursor.y - m_ptStart.y) >= 2))
	{
		if ((m_bShiftDown || m_nMode == Drag) && HasScrollBars())
		{
			m_bDraggingLink = false;
			m_bDraggingPage = true;
			UpdateHoverAnnotation(CPoint(-1, -1));
			UpdateCursor();
		}
		else if (m_nMode == Select)
		{
			m_bDraggingLink = false;
			m_bDraggingText = true;
			UpdateHoverAnnotation(CPoint(-1, -1));
			ClearSelection();
			UpdateCursor();
		}
		else if (m_nMode == SelectRect || m_nMode == ZoomRect)
		{
			m_bDraggingLink = false;
			m_bDraggingRect = true;
			UpdateHoverAnnotation(CPoint(-1, -1));
			ClearSelection();

			m_nSelectionPage = m_nStartPage;
			m_rcSelection.xmin = m_ptStartSel.x;
			m_rcSelection.xmax = m_ptStartSel.x + 1;
			m_rcSelection.ymin = m_ptStartSel.y;
			m_rcSelection.ymax = m_ptStartSel.y + 1;

			UpdateCursor();
		}
	}

	if (m_bDraggingPage)
	{
		CPoint ptCursor;
		::GetCursorPos(&ptCursor);

		CPoint ptStartPos = m_pages[m_nStartPage].ptOffset + m_ptStartPos;
		CPoint ptScrollBy = ptStartPos + m_ptStart - ptCursor - GetScrollPosition();

		if (m_nLayout == Continuous || m_nLayout == ContinuousFacing)
		{
			UpdatePageSizes(GetScrollPosition().y, ptScrollBy.y);

			ptStartPos = m_pages[m_nStartPage].ptOffset + m_ptStartPos;
			ptScrollBy = ptStartPos + m_ptStart - ptCursor - GetScrollPosition();
		}

		if (ptScrollBy != CPoint(0, 0))
			m_bClick = false;

		OnScrollBy(ptScrollBy);
		UpdateWindow();
	}
	else if (m_bDraggingText)
	{
		int nCurPage = GetPageNearPoint(point);
		if (nCurPage == -1)
			return;

		int nSelPos = GetTextPosFromPoint(nCurPage, point);
		bool bInfoLoaded = false;
		CWaitCursor* pWaitCursor = NULL;

		int nPage;

		if (nCurPage < m_nStartPage)
		{
			if (m_nPrevPage >= m_nStartPage)
			{
				for (nPage = m_nStartPage; nPage <= m_nPrevPage; ++nPage)
					ClearSelection(nPage);

				SelectTextRange(nCurPage, nSelPos, -1, bInfoLoaded, pWaitCursor);
				for (nPage = nCurPage + 1; nPage < m_nStartPage; ++nPage)
					SelectTextRange(nPage, 0, -1, bInfoLoaded, pWaitCursor);
				SelectTextRange(m_nStartPage, 0, m_nSelStartPos, bInfoLoaded, pWaitCursor);
			}
			else if (m_nPrevPage <= nCurPage)
			{
				for (nPage = m_nPrevPage; nPage <= nCurPage; ++nPage)
					ClearSelection(nPage);

				SelectTextRange(nCurPage, nSelPos, -1, bInfoLoaded, pWaitCursor);
			}
			else
			{
				ClearSelection(m_nPrevPage);

				SelectTextRange(nCurPage, nSelPos, -1, bInfoLoaded, pWaitCursor);
				for (nPage = nCurPage + 1; nPage <= m_nPrevPage; ++nPage)
					SelectTextRange(nPage, 0, -1, bInfoLoaded, pWaitCursor);
			}
		}
		else if (nCurPage > m_nStartPage)
		{
			if (m_nPrevPage <= m_nStartPage)
			{
				for (nPage = m_nPrevPage; nPage <= m_nStartPage; ++nPage)
					ClearSelection(nPage);

				SelectTextRange(m_nStartPage, m_nSelStartPos, -1, bInfoLoaded, pWaitCursor);
				for (nPage = m_nStartPage + 1; nPage < nCurPage; ++nPage)
					SelectTextRange(nPage, 0, -1, bInfoLoaded, pWaitCursor);
				SelectTextRange(nCurPage, 0, nSelPos, bInfoLoaded, pWaitCursor);
			}
			else if (m_nPrevPage >= nCurPage)
			{
				for (nPage = nCurPage; nPage <= m_nPrevPage; ++nPage)
					ClearSelection(nPage);

				SelectTextRange(nCurPage, 0, nSelPos, bInfoLoaded, pWaitCursor);
			}
			else
			{
				ClearSelection(m_nPrevPage);

				for (nPage = m_nPrevPage; nPage < nCurPage; ++nPage)
					SelectTextRange(nPage, 0, -1, bInfoLoaded, pWaitCursor);
				SelectTextRange(nCurPage, 0, nSelPos, bInfoLoaded, pWaitCursor);
			}
		}
		else
		{
			if (m_nPrevPage < m_nStartPage)
			{
				for (nPage = m_nPrevPage; nPage <= m_nStartPage; ++nPage)
					ClearSelection(nPage);
			}
			else
			{
				for (nPage = m_nStartPage; nPage <= m_nPrevPage; ++nPage)
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
			UpdateLayout();

		if (pWaitCursor != NULL)
			delete pWaitCursor;
	}
	else if (m_bDraggingRect)
	{
		Page& page = m_pages[m_nSelectionPage];
		CPoint ptScroll = GetScrollPosition();
		CPoint pt = point + ptScroll - page.ptOffset;
		CPoint ptCurrent = ScreenToDjVu(m_nStartPage, pt, true);

		CRect rcDisplay = TranslatePageRect(m_nSelectionPage, m_rcSelection) - ptScroll;
		rcDisplay.InflateRect(0, 0, 1, 1);
		InvalidateRect(rcDisplay, false);

		m_rcSelection.xmin = min(ptCurrent.x, m_ptStartSel.x);
		m_rcSelection.xmax = max(ptCurrent.x, m_ptStartSel.x) + 1;
		m_rcSelection.ymin = min(ptCurrent.y, m_ptStartSel.y);
		m_rcSelection.ymax = max(ptCurrent.y, m_ptStartSel.y) + 1;

		rcDisplay = TranslatePageRect(m_nSelectionPage, m_rcSelection) - ptScroll;
		rcDisplay.InflateRect(0, 0, 1, 1);
		InvalidateRect(rcDisplay, false);

		UpdateWindow();
	}
	else if (m_bDraggingMagnify)
	{
		UpdateMagnifyWnd();
	}

	CMyScrollView::OnMouseMove(nFlags, point);

	m_bInsideMouseMove = false;
}

void CDjVuView::OnMouseLeave()
{
	UpdateHoverAnnotation(CPoint(-1, -1));
}

void CDjVuView::SelectTextRange(int nPage, int nStart, int nEnd,
	bool& bInfoLoaded, CWaitCursor*& pWaitCursor)
{
	Page& page = m_pages[nPage];
	if (!page.info.bDecoded)
	{
		if (pWaitCursor == NULL)
			pWaitCursor = new CWaitCursor();

		page.Init(m_pSource, nPage);
		bInfoLoaded = true;
	}

	if (page.info.bHasText && !page.info.bTextDecoded)
	{
		if (pWaitCursor == NULL)
			pWaitCursor = new CWaitCursor();

		page.Init(m_pSource, nPage, true);
	}

	if (page.info.pText == NULL)
		return;

	if (nEnd == -1)
		nEnd = page.info.pText->textUTF8.length();

	DjVuSelection selection;
	FindSelectionZones(selection, page.info.pText, nStart, nEnd);

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
	DoPrint();
}

void CDjVuView::DoPrint(const CString& strRange)
{
	CPrintDlg dlg(GetDocument()->GetSource(), m_nPage, m_nRotate, m_nDisplayMode);
	if (m_nSelectionPage != -1)
	{
		dlg.m_bHasSelection = true;
		dlg.m_rcSelection = m_rcSelection;
	}

	if (!strRange.IsEmpty())
		dlg.SetCustomRange(strRange);

	if (dlg.DoModal() == IDOK)
	{
		ASSERT(dlg.m_pPrinter != NULL && dlg.m_pPaper != NULL);

		CProgressDlg progress_dlg(PrintThreadProc);
		progress_dlg.SetUserData(reinterpret_cast<DWORD_PTR>(&dlg));

		if (progress_dlg.DoModal() == IDOK)
		{
			if (progress_dlg.GetResultCode() > 0)
				AfxMessageBox(IDS_PRINTING_FAILED);
		}
	}
}

int CDjVuView::GetPageFromPoint(CPoint point) const
{
	point += GetScrollPosition();

	CRect rcFrame(m_nPageBorder, m_nPageBorder, m_nPageBorder + m_nPageShadow, m_nPageBorder + m_nPageShadow);

	if (m_nLayout == SinglePage || m_nLayout == Facing)
	{
		CRect rcPage(m_pages[m_nPage].ptOffset, m_pages[m_nPage].szBitmap);
		rcPage.InflateRect(rcFrame);

		if (rcPage.PtInRect(point))
			return m_nPage;

		if (m_nLayout == Facing && HasFacingPage(m_nPage))
		{
			CRect rcPage2(m_pages[m_nPage + 1].ptOffset, m_pages[m_nPage + 1].szBitmap);
			rcPage2.InflateRect(rcFrame);
			return (rcPage2.PtInRect(point) ? m_nPage + 1 : -1);
		}
	}
	else if (m_nLayout == Continuous || m_nLayout == ContinuousFacing)
	{
		for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		{
			CRect rcPage(m_pages[nPage].ptOffset, m_pages[nPage].szBitmap);
			rcPage.InflateRect(rcFrame);

			if (rcPage.PtInRect(point))
				return nPage;
		}
	}

	return -1;
}

int CDjVuView::GetPageNearPoint(CPoint point) const
{
	CSize szViewport = GetViewportSize();

	point.x = max(0, min(point.x, szViewport.cx - 1));
	point.y = max(0, min(point.y, szViewport.cy - 1));
	point += GetScrollPosition();

	if (m_nLayout == SinglePage || m_nLayout == Facing)
	{
		if (m_pages[m_nPage].rcDisplay.PtInRect(point))
			return m_nPage;

		if (m_nLayout == Facing && HasFacingPage(m_nPage))
			return (m_pages[m_nPage + 1].rcDisplay.PtInRect(point) ? m_nPage + 1 : -1);
	}
	else if (m_nLayout == Continuous || m_nLayout == ContinuousFacing)
	{
		for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		{
			if (m_pages[nPage].rcDisplay.PtInRect(point))
				return nPage;
		}
	}

	return -1;
}

void CDjVuView::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_bDragging)
		return;

	if (m_nMode == NextPrev && !m_bDragging && !m_bShiftDown)
	{
		OnViewPreviouspage();
		return;
	}

	UpdateHoverAnnotation();

	m_bDraggingRight = true;
	m_nStartPage = GetPageNearPoint(point);
	if (m_nStartPage != -1)
	{
		m_ptStartPos = GetScrollPosition() - m_pages[m_nStartPage].ptOffset;
		::GetCursorPos(&m_ptStart);
	}

	SetCapture();
	ShowCursor();
	UpdateCursor();

	CMyScrollView::OnRButtonDown(nFlags, point);
}

void CDjVuView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDragging)
		return;

	if (m_bDraggingRight && !m_bDraggingPage)
		OnContextMenu();

	StopDragging();
	UpdateHoverAnnotation(point);
	CMyScrollView::OnRButtonUp(nFlags, point);
}

void CDjVuView::OnContextMenu()
{
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	m_nClickedPage = GetPageFromPoint(point);

	CRect rcViewport(CPoint(0, 0), GetViewportSize());
	if (!rcViewport.PtInRect(point) || m_nClickedPage == -1)
		return;

	m_pClickedAnno = m_pHoverAnno;
	m_bClickedCustom = m_bHoverIsCustom;

	CMenu menu;
	menu.LoadMenu(IDR_POPUP);

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	if (m_bHasSelection || m_nSelectionPage != -1)
	{
		pPopup->DeleteMenu(ID_ANNOTATION_DELETE, MF_BYCOMMAND);
		pPopup->DeleteMenu(ID_ANNOTATION_EDIT, MF_BYCOMMAND);

		if (!m_bHasSelection)
		{
			pPopup->ModifyMenu(ID_EDIT_COPY, MF_BYCOMMAND | MF_STRING,
					ID_EDIT_COPY, LoadString(IDS_COPY_SELECTION));
			pPopup->DeleteMenu(ID_HIGHLIGHT_TEXT, MF_BYCOMMAND);
		}
		else
		{
			pPopup->DeleteMenu(ID_HIGHLIGHT_SELECTION, MF_BYCOMMAND);
			pPopup->DeleteMenu(ID_EXPORT_SELECTION, MF_BYCOMMAND);
			pPopup->DeleteMenu(ID_ZOOM_TO_SELECTION, MF_BYCOMMAND);
		}
	}
	else
	{
		pPopup->DeleteMenu(ID_EDIT_COPY, MF_BYCOMMAND);
		pPopup->DeleteMenu(ID_HIGHLIGHT_TEXT, MF_BYCOMMAND);
		pPopup->DeleteMenu(ID_HIGHLIGHT_SELECTION, MF_BYCOMMAND);
		pPopup->DeleteMenu(ID_EXPORT_SELECTION, MF_BYCOMMAND);
		pPopup->DeleteMenu(ID_ZOOM_TO_SELECTION, MF_BYCOMMAND);

		if (m_pClickedAnno == NULL || !m_bClickedCustom)
		{
			pPopup->DeleteMenu(ID_ANNOTATION_DELETE, MF_BYCOMMAND);
			pPopup->DeleteMenu(ID_ANNOTATION_EDIT, MF_BYCOMMAND);
			pPopup->DeleteMenu(0, MF_BYPOSITION);
		}
	}

	TRACKMOUSEEVENT tme;
	ZeroMemory(&tme, sizeof(tme));
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE | TME_CANCEL;
	tme.hwndTrack = m_hWnd;
	tme.dwHoverTime = HOVER_DEFAULT;
	TrackMouseEvent(&tme);

	m_bPopupMenu = true;
	ShowCursor();

	ClientToScreen(&point);
	UINT nID = pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			point.x, point.y, GetTopLevelParent());

	m_bPopupMenu = false;

	if (nID != 0)
		GetTopLevelParent()->SendMessage(WM_COMMAND, nID);

	m_pClickedAnno = NULL;
	m_nClickedPage = -1;

	UpdateHoverAnnotation(CPoint(-1, -1));
	UpdateHoverAnnotation();
	UpdateCursor();
}

void CDjVuView::OnPageInformation()
{
	if (m_nClickedPage == -1)
		return;

	GP<DjVuImage> pImage = m_pSource->GetPage(m_nClickedPage, NULL);
	if (pImage == NULL)
	{
		AfxMessageBox(IDS_ERROR_DECODING_PAGE, MB_ICONERROR | MB_OK);
		return;
	}

	CString strDescr = MakeCString(pImage->get_long_description());
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
				fSize, szName, nShapes, (nShapes == 1 ? _T("") : _T("s")));
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
		else if (strLine.Find(_T("DjVuFile.JPEG_bg1_w")) == 0)
		{
			strLine = strLine.Mid(19);
			_stscanf(strLine, _T("%lf%s"), &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tJPEG background."), fSize, szName);
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
		else if (strLine.Find(_T("DjVuFile.JPEG_fg1_w")) == 0)
		{
			strLine = strLine.Mid(19);
			_stscanf(strLine, _T("%lf%s"), &fSize, szName);
			strFormatted.Format(_T(" %5.1f Kb\t'%s'\tJPEG foreground colors."), fSize, szName);
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

	CDjViewApp::MessageBoxOptions mbo;
	mbo.bVerbatim = true;
	theApp.DoMessageBox(strInfo, MB_ICONINFORMATION | MB_OK, 0, mbo);
}

LRESULT CDjVuView::OnPageRendered(WPARAM wParam, LPARAM lParam)
{
	int nPage = (int)wParam;
	CDIB* pBitmap = reinterpret_cast<CDIB*>(lParam);

	m_dataLock.Lock();
	m_bitmaps.erase(pBitmap);
	m_dataLock.Unlock();

	OnPageDecoded(nPage, true);

	Page& page = m_pages[nPage];
	page.DeleteBitmap();
	page.pBitmap = pBitmap;
	page.bBitmapRendered = true;

	long nPendingPage = InterlockedExchangeAdd(&m_nPendingPage, 0);
	if (InvalidatePage(nPage) && nPage != nPendingPage)
	{
		UpdateWindow();

		// Notify the magnify window, because it will not repaint
		// itself if is is layered.
		if (m_nType == Magnify)
		{
			CMagnifyWnd* pMagnifyWnd = GetMainFrame()->GetMagnifyWnd();
			pMagnifyWnd->CenterOnPoint(pMagnifyWnd->GetCenterPoint());
		}
	}

	return 0;
}

void CDjVuView::PageDecoded(int nPage)
{
	PostMessage(WM_PAGE_DECODED, nPage);
}

void CDjVuView::PageRendered(int nPage, CDIB* pDIB)
{
	LPARAM lParam = reinterpret_cast<LPARAM>(pDIB);
	if (pDIB != NULL)
	{
		m_dataLock.Lock();
		m_bitmaps.insert(pDIB);
		m_dataLock.Unlock();
	}

	long nPendingPage = InterlockedExchangeAdd(&m_nPendingPage, 0);
	if (nPendingPage == nPage)
		SendMessage(WM_PAGE_RENDERED, nPage, lParam);
	else
		PostMessage(WM_PAGE_RENDERED, nPage, lParam);
}

void CDjVuView::OnDestroy()
{
	if (m_pRenderThread != NULL)
	{
		m_pRenderThread->Stop();
		m_pRenderThread = NULL;
	}

	theApp.RemoveObserver(this);
	if (m_pSource != NULL)
	{
		m_pSource->GetSettings()->RemoveObserver(this);

		// Remove this view from all observed pages.
		vector<int> add, remove;
		remove.reserve(m_nPageCount);
		for (int i = 0; i < m_nPageCount; ++i)
			remove.push_back(i);
		m_pSource->ChangeObservedPages(this, add, remove);
	}

	if (m_nType == Normal)
	{
		if (GetMDIChild()->GetContentsTree() != NULL)
			GetMDIChild()->GetContentsTree()->RemoveObserver(this);
		if (GetMDIChild()->GetPageIndex() != NULL)
			GetMDIChild()->GetPageIndex()->RemoveObserver(this);
		if (GetMDIChild()->HasBookmarksTree())
			GetMDIChild()->GetBookmarksTree(false)->RemoveObserver(this);
	}

	KillTimer(1);
	KillTimer(2);
	ShowCursor();

	CMyScrollView::OnDestroy();
}

LRESULT CDjVuView::OnPageDecoded(WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_nPageCount == m_pSource->GetPageCount());

	int nPage = (int)wParam;

	Page& page = m_pages[nPage];
	bool bHadInfo = page.info.bDecoded;

	page.Init(m_pSource, nPage);

	if (!bHadInfo)
	{
		if (lParam && (m_nLayout == Continuous || m_nLayout == ContinuousFacing))
			UpdateLayout();
		else
			m_bNeedUpdate = true;
	}

	return 0;
}

void CDjVuView::ScrollToPosition(CPoint pt, bool bRepaint)
{
	if (m_nPageCount == 0)
	{
		CMyScrollView::ScrollToPosition(pt, bRepaint);
		return;
	}

	// We do not want to read missing page info during UpdateLayout()
	if (!m_bInsideUpdateLayout && (m_nLayout == Continuous || m_nLayout == ContinuousFacing))
		UpdatePageSizes(pt.y, 0, RECALC);

	CMyScrollView::ScrollToPosition(pt, bRepaint);

	// ScrollToPosition will be called from inside SetScrollSizes(),
	// when scroll position has not yet been updated to properly
	// retain the current page onscreen.  So if we call UpdateView(),
	// it will case a wrong page change notification to be sent out,
	// and thumbnails view will jump back and forward. Additionally,
	// page cache will be screwed up by UpdateVisiblePages(). Hence
	// the if statement.
	if (!m_bInsideUpdateLayout)
		UpdateView(false, true);
}

bool CDjVuView::OnScroll(UINT nScrollCode, UINT nPos, bool bDoScroll)
{
	int nCode = HIBYTE(nScrollCode);  // Vertical scrolling
	if (nCode == SB_THUMBTRACK
			&& (m_nLayout == Continuous || m_nLayout == ContinuousFacing))
	{
		SCROLLINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(si);
		si.fMask = SIF_TRACKPOS;
		::GetScrollInfo(m_vertScrollBar.m_hWnd, SB_CTL, &si);

		ScrollToPosition(CPoint(GetScrollPosition().x, si.nTrackPos));
		UpdateWindow();
		return true;
	}

	return CMyScrollView::OnScroll(nScrollCode, nPos, bDoScroll);
}

bool CDjVuView::OnScrollBy(CSize szScrollBy, bool bDoScroll)
{
	if (!bDoScroll || m_nPageCount == 0)
		return CMyScrollView::OnScrollBy(szScrollBy, bDoScroll);

	bool bUpdateVisible = false;

	if (HasVertScrollBar()
			&& (m_nLayout == Continuous || m_nLayout == ContinuousFacing))
	{
		int yOrig = GetScrollPosition().y;
		int yMax = GetScrollLimit().cy;
		int y = max(0, min(yOrig + szScrollBy.cy, yMax));

		if (y != yOrig)
		{
			UpdatePageSizes(yOrig, y - yOrig);
			bUpdateVisible = true;
		}
	}

	if (!CMyScrollView::OnScrollBy(szScrollBy, bDoScroll))
		return false;

	UpdateView(false, bUpdateVisible);
	return true;
}

void CDjVuView::UpdatePageSizes(int nTop, int nScroll, int nUpdateType)
{
	ASSERT(m_nLayout == Continuous || m_nLayout == ContinuousFacing);

	CSize szViewport = GetViewportSize();
	if (nScroll < 0)
	{
		UpdatePagesFromBottom(nTop + nScroll, nTop + szViewport.cy, nUpdateType);
	}
	else
	{
		if (!UpdatePagesFromTop(nTop, nTop + szViewport.cy + nScroll, nUpdateType))
		{
			int nBottom = m_szDisplay.cy;
			UpdatePagesFromBottom(nBottom - szViewport.cy - nScroll, nBottom, nUpdateType);
		}
	}
}

void CDjVuView::UpdatePagesFromBottom(int nTop, int nBottom, int nUpdateType)
{
	CSize szViewport = GetViewportSize();

	// Check that the pages which become visible as a result
	// of this operation have their szPage filled in

	int nPage = 0;
	while (nPage < m_nPageCount - 1 &&
		   nBottom > m_pages[nPage].rcDisplay.bottom)
		++nPage;

	if (m_nLayout == Continuous)
	{
		UpdatePageSize(szViewport, nPage);
		int nPageTop = m_pages[nPage].rcDisplay.top;

		while (nPage > 0 && nTop < nPageTop)
		{
			--nPage;
			UpdatePageSize(szViewport, nPage);

			nPageTop -= m_pages[nPage].szBitmap.cy + 2*m_nPageBorder + m_nPageGap + m_nPageShadow;
		}
	}
	else
	{
		nPage = FixPageNumber(nPage);
		UpdatePageSizeFacing(szViewport, nPage);
		int nPageTop = m_pages[nPage].rcDisplay.top;

		while (nPage > 0 && nTop < nPageTop)
		{
			nPage = max(0, nPage - 2);
			UpdatePageSizeFacing(szViewport, nPage);

			int nHeight = m_pages[nPage].szBitmap.cy;
			if (HasFacingPage(nPage))
				nHeight = max(nHeight, m_pages[nPage + 1].szBitmap.cy);
			nPageTop -= nHeight + 2*m_nPageBorder + m_nPageGap + m_nPageShadow;
		}
	}

	if (m_bNeedUpdate)
	{
		UpdateLayout(nUpdateType != -1 ? static_cast<UpdateType>(nUpdateType) : BOTTOM);
		InvalidateViewport();
	}
}

bool CDjVuView::UpdatePagesFromTop(int nTop, int nBottom, int nUpdateType)
{
	CSize szViewport = GetViewportSize();

	int nPage = 0;
	while (nPage < m_nPageCount - 1 &&
		   nTop >= m_pages[nPage].rcDisplay.bottom)
		++nPage;

	int nPageBottom;

	if (m_nLayout == Continuous)
	{
		UpdatePageSize(szViewport, nPage);
		nPageBottom = m_pages[nPage].rcDisplay.top + m_pages[nPage].szBitmap.cy
				+ m_nPageBorder + m_nPageShadow + m_nPageGap;

		while (++nPage < m_nPageCount && nBottom > nPageBottom)
		{
			UpdatePageSize(szViewport, nPage);
			nPageBottom += m_pages[nPage].szBitmap.cy + 2*m_nPageBorder
					+ m_nPageShadow + m_nPageGap;
		}
	}
	else
	{
		nPage = FixPageNumber(nPage);
		UpdatePageSizeFacing(szViewport, nPage);

		int nHeight = m_pages[nPage].szBitmap.cy;
		if (HasFacingPage(nPage))
			nHeight = max(nHeight, m_pages[nPage + 1].szBitmap.cy);
		nPageBottom = m_pages[nPage].rcDisplay.top + nHeight + m_nPageBorder + m_nPageShadow + m_nPageGap;

		while ((nPage = GetNextPage(nPage)) < m_nPageCount && nBottom > nPageBottom)
		{
			UpdatePageSizeFacing(szViewport, nPage);

			int nHeight = m_pages[nPage].szBitmap.cy;
			if (HasFacingPage(nPage))
				nHeight = max(nHeight, m_pages[nPage + 1].szBitmap.cy);
			nPageBottom += nHeight + m_nPageBorder + m_nPageShadow + m_nPageGap;
		}
	}

	bool bNeedScrollUp = false;
	if (nPage >= m_nPageCount)
		bNeedScrollUp = (nBottom > nPageBottom + m_nShadowMargin - m_nPageGap);

	if (m_bNeedUpdate)
	{
		UpdateLayout(nUpdateType != -1 ? static_cast<UpdateType>(nUpdateType) : TOP);
		InvalidateViewport();
	}

	return !bNeedScrollUp;
}

void CDjVuView::UpdatePageSize(const CSize& szBounds, int nPage)
{
	Page& page = m_pages[nPage];

	if (!page.info.bDecoded || !page.bHasSize)
	{
		if (!page.info.bDecoded)
			page.Init(m_pSource, nPage);

		page.szBitmap = CalcPageSize(szBounds, page.GetSize(m_nRotate), page.info.nDPI);
		m_bNeedUpdate = true;
	}
}

void CDjVuView::UpdatePageSizeFacing(const CSize& szBounds, int nPage)
{
	Page* pPage;
	Page* pNextPage;
	int nLeftPage, nRightPage;
	GetFacingPages(nPage, pPage, pNextPage, &nLeftPage, &nRightPage);

	bool bUpdated = false;
	if (!pPage->info.bDecoded || !pPage->bHasSize)
	{
		if (!pPage->info.bDecoded)
			pPage->Init(m_pSource, nLeftPage);

		bUpdated = true;
		m_bNeedUpdate = true;
	}

	if (pNextPage != NULL && (!pNextPage->info.bDecoded || !pNextPage->bHasSize))
	{
		if (!pNextPage->info.bDecoded)
			pNextPage->Init(m_pSource, nRightPage);

		bUpdated = true;
		m_bNeedUpdate = true;
	}

	if (bUpdated)
	{
		if (pNextPage != NULL)
		{
			CalcPageSizeFacing(szBounds,
					pPage->GetSize(m_nRotate), pPage->info.nDPI, pPage->szBitmap,
					pNextPage->GetSize(m_nRotate), pNextPage->info.nDPI, pNextPage->szBitmap);
		}
		else
		{
			CalcPageSizeFacing(szBounds,
					pPage->GetSize(m_nRotate), pPage->info.nDPI, pPage->szBitmap,
					pPage->GetSize(m_nRotate), pPage->info.nDPI, pPage->szBitmap);
		}
	}
}

int CDjVuView::CalcTopPage() const
{
	if (m_nLayout == SinglePage || m_nLayout == Facing)
		return m_nPage;

	int nTop = GetScrollPosition().y;

	int nPage = max(0, min(m_nPageCount - 1, m_nPage));
	while (nPage > 0 && nTop < m_pages[nPage].rcDisplay.top)
		--nPage;
	while (nPage < m_nPageCount - 1 && nTop >= m_pages[nPage].rcDisplay.bottom)
		++nPage;

	return FixPageNumber(nPage);
}

int CDjVuView::CalcCurrentPage() const
{
	if (m_nLayout == SinglePage || m_nLayout == Facing)
		return m_nPage;

	int nPage = CalcTopPage();
	const Page& page = m_pages[nPage];

	CSize szViewport = GetViewportSize();
	int nHeight = min(page.szBitmap.cy, szViewport.cy);

	if (page.rcDisplay.bottom - GetScrollPosition().y < 0.3*nHeight
			&& GetNextPage(nPage) < m_nPageCount)
		nPage = GetNextPage(nPage);

	return nPage;
}

int CDjVuView::CalcBottomPage(int nTopPage) const
{
	CSize szViewport = GetViewportSize();
	int nTop = GetScrollPosition().y;

	int nBottomPage = GetNextPage(nTopPage);
	while (nBottomPage < m_nPageCount
			&& m_pages[nBottomPage].rcDisplay.top < nTop + szViewport.cy)
		nBottomPage = GetNextPage(nBottomPage);

	--nBottomPage;
	if (nBottomPage >= m_nPageCount)
		nBottomPage = m_nPageCount - 1;

	return nBottomPage;
}

BOOL CDjVuView::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	if (m_bDraggingPage)
		return false;

	CWnd* pWnd = WindowFromPoint(point);
	if (pWnd != this && !IsChild(pWnd) && IsFromCurrentProcess(pWnd) && !m_bDraggingMagnify
			&& pWnd->SendMessage(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(point.x, point.y)) != 0)
		return true;

	if (CMyScrollView::OnMouseWheel(nFlags, zDelta, point))
		return true;

	if ((nFlags & MK_CONTROL) != 0)
	{
		// Zoom in/out
		if (theApp.GetAppSettings()->bInvertWheelZoom == (zDelta < 0))
			OnViewZoomOut();
		else if (zDelta != 0)
			OnViewZoomIn();

		return true;
	}

	if (!HasScrollBars() && (m_nLayout == SinglePage || m_nLayout == Facing))
	{
		if (zDelta < 0 && IsViewNextpageEnabled())
		{
			OnViewNextpage();
			return true;
		}
		else if (zDelta > 0 && IsViewPreviouspageEnabled())
		{
			CPoint ptScroll = GetScrollPosition();
			OnViewPreviouspage();
			OnScrollBy(CPoint(ptScroll.x, m_szDisplay.cy) - GetScrollPosition());
			UpdateWindow();
			return true;
		}
	}

	return false;
}

bool CDjVuView::InvalidatePage(int nPage)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);
	if (m_nLayout == SinglePage && nPage != m_nPage ||
			(m_nLayout == Facing && nPage != m_nPage && (!HasFacingPage(m_nPage) || nPage != m_nPage + 1)))
		return false;

	CRect rect(CPoint(0, 0), GetViewportSize());
	CPoint ptScroll = GetScrollPosition();
	if (rect.IntersectRect(CRect(rect), m_pages[nPage].rcDisplay - ptScroll))
	{
		InvalidateRect(rect, false);
		return true;
	}

	return false;
}

bool CDjVuView::InvalidateAnno(Annotation* pAnno, int nPage)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);
	if (m_nLayout == SinglePage && nPage != m_nPage ||
			(m_nLayout == Facing && nPage != m_nPage && (!HasFacingPage(m_nPage) || nPage != m_nPage + 1)))
		return false;

	CPoint ptScroll = GetScrollPosition();

	CRect rect = TranslatePageRect(nPage, pAnno->rectBounds) - ptScroll;
	rect.InflateRect(1, 1);
	CRect rcViewport(CPoint(0, 0), GetViewportSize());
	if (rect.IntersectRect(CRect(rect), rcViewport))
	{
		InvalidateRect(rect, false);
		return true;
	}

	return false;
}

void CDjVuView::OnExportPage(UINT nID)
{
	if (m_nSelectionPage == -1 && nID == ID_EXPORT_SELECTION)
		return;

	bool bCrop = (nID == ID_EXPORT_SELECTION);
	int nPage = (bCrop ? m_nSelectionPage : m_nClickedPage);
	DoExportPage(nPage, bCrop, m_rcSelection);
}

void CDjVuView::DoExportPage(int nPage, bool bCrop, GRect rect)
{
	CString strFileName = FormatString(_T("p%04d%s"), nPage + 1, bCrop ? _T("-sel") : _T(""));

	CString strFilter = LoadString(Gdip::IsLoaded() ? IDS_IMAGE_FILTER : IDS_BMP_FILTER);
	CMyFileDialog dlg(false, _T("png"), strFileName, OFN_OVERWRITEPROMPT |
		OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST, strFilter);

	CString strTitle;
	strTitle.LoadString(IDS_EXPORT_PAGE);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);
	dlg.m_ofn.nFilterIndex = (Gdip::IsLoaded() ? theApp.GetAppSettings()->nImageFormat : 1);

	UINT_PTR nResult = dlg.DoModal();
	SetFocus();
	if (nResult != IDOK)
		return;

	CWaitCursor wait;

	CString strPathName = dlg.GetPathName();
	CDIB::ImageFormat nFormat = (CDIB::ImageFormat) dlg.m_ofn.nFilterIndex;
	theApp.GetAppSettings()->nImageFormat = nFormat;

	Page& page = m_pages[nPage];
	if (!page.info.bDecoded)
		page.Init(m_pSource, nPage);

	GP<DjVuImage> pImage = m_pSource->GetPage(nPage, NULL);
	if (pImage == NULL)
	{
		AfxMessageBox(IDS_ERROR_DECODING_PAGE, MB_ICONERROR | MB_OK);
		return;
	}

	CSize size = page.GetSize(m_nRotate);
	CDIB* pBitmap = CRenderThread::Render(pImage, size, m_displaySettings, m_nDisplayMode, m_nRotate);

	if (bCrop && pBitmap != NULL && pBitmap->m_hObject != NULL)
	{
		CRect rcCrop = TranslatePageRect(nPage, rect, false);
		CDIB* pCropped = pBitmap->Crop(rcCrop);
		delete pBitmap;
		pBitmap = pCropped;
	}

	if (pBitmap != NULL && pBitmap->m_hObject != NULL)
	{
		bool bResult = false;

		CDIB* pNewBitmap = pBitmap->ReduceColors();
		if (pNewBitmap != NULL && pNewBitmap->m_hObject != NULL)
			bResult = pNewBitmap->Save(strPathName, nFormat);
		else
			bResult = pBitmap->Save(strPathName, nFormat);
		delete pNewBitmap;

		if (!bResult)
			AfxMessageBox(IDS_ERROR_EXPORTING_PAGE, MB_ICONERROR | MB_OK);
	}
	else
	{
		AfxMessageBox(IDS_ERROR_RENDERING_PAGE, MB_ICONERROR | MB_OK);
	}

	delete pBitmap;
}

int ParseFileTemplate(CString strPathName, CString& strPrefix, CString& strSuffix)
{
	// Extract prefix, suffix and integer format width from the
	// file name in the form c:\path\to\file\prefix####suffix.ext
	// Only the last block of ####'s before the extension counts.
	LPTSTR pszPathName = strPathName.GetBuffer(0);
	LPTSTR pszExt = PathFindExtension(pszPathName);
	CString strExtension = pszExt;
	*pszExt = '\0';

	LPTSTR pszName = PathFindFileName(pszPathName);
	LPTSTR pszBlock;
	for (pszBlock = pszExt; pszBlock > pszName; --pszBlock)
		if (*pszBlock == '#' && *(pszBlock - 1) != '#')
			break;

	LPTSTR pszSuffix;
	int nFormatWidth = 4;
	if (*pszBlock != '#')
	{
		pszSuffix = pszExt;
	}
	else
	{
		pszSuffix = pszBlock;
		*pszSuffix++ = '\0';
		while (pszSuffix < pszExt && *pszSuffix == '#')
			++pszSuffix;
		nFormatWidth = (int)(pszSuffix - pszBlock);
	}

	strPrefix = pszPathName;
	strSuffix = pszSuffix + strExtension;

	strPathName.ReleaseBuffer();
	return nFormatWidth;
}

int PromptOverwrite(const CString& strPrefix, const CString& strSuffix,
		int nFormatWidth, const set<int>& numbers)
{
	CString strMask = strPrefix;
	for (int i = 0; i < nFormatWidth; ++i)
		strMask += '?';
	strMask += strSuffix;

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(strMask, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return true;

	do
	{
		CString strNumber = fd.cFileName;
		if (strNumber.GetLength() >= strSuffix.GetLength() + nFormatWidth)
		{
			strNumber = strNumber.Mid(strNumber.GetLength() - strSuffix.GetLength() - nFormatWidth,
					nFormatWidth);
			TCHAR* pEndPtr = NULL;
			int nPage = _tcstol(strNumber.GetBuffer(0), &pEndPtr, 10);
			if (pEndPtr != NULL && *pEndPtr == '\0'
					&& numbers.find(nPage - 1) != numbers.end())
			{
				FindClose(hFind);
				return AfxMessageBox(IDS_EXPORT_PROMPT_OVERWRITE, MB_ICONEXCLAMATION | MB_YESNOCANCEL);
			}
		}
	} while (FindNextFile(hFind, &fd) != 0);

	FindClose(hFind);
	return IDYES;
}

unsigned int __stdcall CDjVuView::ExportThreadProc(void* pvData)
{
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	IProgressInfo* pProgress = reinterpret_cast<IProgressInfo*>(pvData);
	ExportData& data = *reinterpret_cast<ExportData*>(pProgress->GetUserData());
	CDjVuView* pView = data.pView;

	pProgress->SetRange(0, (int)data.pages.size());

	bool bSuccess = true;
	int i = 0;
	for (set<int>::const_iterator it = data.pages.begin(); it != data.pages.end(); ++it, ++i)
	{
		if (pProgress->IsCancelled())
			break;

		int nPage = *it;
		pProgress->SetStatus(FormatString(IDS_EXPORTING_PAGE, nPage + 1, i + 1, data.pages.size()));
		pProgress->SetPos(i);

		CString strPathName;
		strPathName.Format(_T("%s%0*d%s"), data.strPrefix, data.nFormatWidth, nPage + 1, data.strSuffix);
		if (!data.bOverwrite && PathFileExists(strPathName))
			continue;

		Page& page = pView->m_pages[nPage];
		if (!page.info.bDecoded)
			page.Init(pView->m_pSource, nPage);

		GP<DjVuImage> pImage = pView->m_pSource->GetPage(nPage, NULL);
		if (pImage == NULL)
		{
			bSuccess = false;
			continue;
		}

		CSize size = page.GetSize(pView->m_nRotate);
		CDIB* pBitmap = CRenderThread::Render(pImage, size,
				pView->m_displaySettings, pView->m_nDisplayMode, pView->m_nRotate);

		if (pBitmap != NULL && pBitmap->m_hObject != NULL)
		{
			CDIB* pNewBitmap = pBitmap->ReduceColors();
			if (pNewBitmap != NULL && pNewBitmap->m_hObject != NULL)
				bSuccess = pNewBitmap->Save(strPathName, data.nFormat);
			else
				bSuccess = pBitmap->Save(strPathName, data.nFormat);
			delete pNewBitmap;
		}
		else
			bSuccess = false;

		delete pBitmap;
	}

	pProgress->SetPos((int)data.pages.size());
	pProgress->StopProgress(bSuccess ? 0 : 1);

	::CoUninitialize();
	return 0;
}

void CDjVuView::ExportPages(const set<int>& pages)
{
	if (pages.empty())
		return;

	if (pages.size() == 1)
	{
		int nPage = *pages.begin();
		DoExportPage(nPage);
		return;
	}

	CString strFileName = _T("p####");

	CString strFilter = LoadString(Gdip::IsLoaded() ? IDS_IMAGE_FILTER : IDS_BMP_FILTER);
	CMyFileDialog dlg(false, _T("png"), strFileName,
		OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST, strFilter);

	CString strTitle;
	strTitle.LoadString(IDS_EXPORT_PAGES);
	dlg.m_ofn.lpstrTitle = strTitle.GetBuffer(0);
	dlg.m_ofn.nFilterIndex = (Gdip::IsLoaded() ? theApp.GetAppSettings()->nImageFormat : 1);

	UINT_PTR nResult = dlg.DoModal();
	SetFocus();
	if (nResult != IDOK)
		return;

	ExportData data(pages);
	data.pView = this;
	data.nFormat = (CDIB::ImageFormat) dlg.m_ofn.nFilterIndex;
	data.nFormatWidth = ParseFileTemplate(dlg.GetPathName(), data.strPrefix, data.strSuffix);

	// Check if any of these files exist
	int nOverwrite = PromptOverwrite(data.strPrefix, data.strSuffix, data.nFormatWidth, pages);
	if (nOverwrite == IDCANCEL)
		return;
	data.bOverwrite = (nOverwrite == IDYES);

	// Do export in a separate thread
	CWaitCursor wait;
	theApp.GetAppSettings()->nImageFormat = dlg.m_ofn.nFilterIndex;

	CProgressDlg progress_dlg(ExportThreadProc);
	progress_dlg.SetUserData(reinterpret_cast<DWORD_PTR>(&data));

	if (progress_dlg.DoModal() == IDOK)
	{
		if (progress_dlg.GetResultCode() > 0)
			AfxMessageBox(IDS_EXPORT_ERRORS);
	}
}

void CDjVuView::OnUpdateExportSelection(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_nSelectionPage != -1);
}

void CDjVuView::OnFindString()
{
	CWaitCursor wait;
	CFindDlg* pDlg = theApp.GetFindDlg(false);
	if (pDlg == NULL)
	{
		GetMainFrame()->SendMessage(WM_COMMAND, ID_EDIT_FIND);
		return;
	}

	GetMainFrame()->SetMessageText(AFX_IDS_IDLEMESSAGE);

	GUTF8String strFindUTF8 = MakeUTF8String(pDlg->m_strFind);
	GUTF8String strFindUp;
	if (!pDlg->m_bMatchCase)
		strFindUp = strFindUTF8.upcase();

	GUTF8String& strFind = (pDlg->m_bMatchCase ? strFindUTF8 : strFindUp);

	if (strFind.length() == 0)
	{
		::MessageBeep(MB_OK);
		return;
	}

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

	bool bWrapping = false;
	int nWrapPage = nStartPage;

	int nPage = nStartPage;
	for (;;)
	{
		Page& page = m_pages[nPage];
		if (!page.info.bDecoded || page.info.bHasText && !page.info.bTextDecoded)
		{
			if (!page.info.bDecoded)
				m_bNeedUpdate = true;

			page.Init(m_pSource, nPage, true);
		}

		if (page.info.pText != NULL)
		{
			GP<DjVuTXT> pText = page.info.pText;

			int nPos = -1;
			if (pDlg->m_bMatchCase)
			{
				GUTF8String& text = pText->textUTF8;
				nPos = text.search(strFind, nStartPos);
			}
			else
			{
				GUTF8String textUp = pText->textUTF8.upcase();
				nPos = textUp.search(strFind, nStartPos);
			}

			if (nPos != -1)
			{
				if (m_bNeedUpdate)
					UpdateLayout();

				int nSelEnd = nPos + strFind.length();
				DjVuSelection selection;
				FindSelectionZones(selection, pText, nPos, nSelEnd);

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
					EnsureSelectionVisible(nPage, page.selection, true);
				}

				if (bWrapping)
					GetMainFrame()->HilightStatusMessage(LoadString(IDS_SEARCH_WRAPPED));
				else
					GetMainFrame()->SetMessageText(AFX_IDS_IDLEMESSAGE);
				return;
			}
		}

		nStartPos = 0;
		++nPage;
		if (bWrapping && nPage > nWrapPage)
			break;

		if (nPage >= m_nPageCount)
		{
			nPage = 0;
			bWrapping = true;
		}
	}

	if (m_bNeedUpdate)
		UpdateLayout();

	GetMainFrame()->HilightStatusMessage(LoadString(IDS_STRING_NOT_FOUND));
	::MessageBeep(MB_OK);
}

void CDjVuView::OnFindAll()
{
	CWaitCursor wait;
	CFindDlg* pDlg = theApp.GetFindDlg();

	GUTF8String strFindUTF8 = MakeUTF8String(pDlg->m_strFind);
	GUTF8String strFindUp;
	if (!pDlg->m_bMatchCase)
		strFindUp = strFindUTF8.upcase();

	GUTF8String& strFind = (pDlg->m_bMatchCase ? strFindUTF8 : strFindUp);

	if (strFind.length() == 0)
	{
		::MessageBeep(MB_OK);
		return;
	}

	CSearchResultsView* pResults = NULL;
	int nResultCount = 0;

	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		pDlg->SetStatusText(FormatString(IDS_SEARCHING_PAGE, nPage + 1, m_nPageCount));

		Page& page = m_pages[nPage];
		if (!page.info.bDecoded || page.info.bHasText && !page.info.bTextDecoded)
		{
			if (!page.info.bDecoded)
				m_bNeedUpdate = true;

			page.Init(m_pSource, nPage, true);
		}

		if (page.info.pText == NULL)
			continue;

		GP<DjVuTXT> pText = page.info.pText;
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
				pResults = GetMDIChild()->GetSearchResults();
				pResults->AddObserver(this);
				pResults->Reset();
			}

			++nResultCount;
			CString strPreview = MakePreviewString(pText->textUTF8, nPos, nSelEnd);
			pResults->AddString(strPreview, nPage, nPos, nSelEnd);

			nPos = nSelEnd;
		} while (nPos != -1);
	}

	if (m_bNeedUpdate)
		UpdateLayout();

	if (nResultCount > 0)
	{
		GetMainFrame()->SetMessageText(FormatString(IDS_NUM_OCCURRENCES, nResultCount));
		pDlg->ShowWindow(SW_HIDE);
	}
	else
	{
		GetMainFrame()->HilightStatusMessage(LoadString(IDS_STRING_NOT_FOUND));
		::MessageBeep(MB_OK);
	}
}

CString MakePreviewString(const GUTF8String& text, int nStart, int nEnd)
{
	// Convert text to CString and add -1/+10 words to the string
	CString strPreview = MakeCString(text.substr(nStart, nEnd - nStart));

	CString strBefore = MakeCString(text.substr(0, nStart));

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

	CString strAfter = MakeCString(text.substr(nEnd, text.length() - nEnd));

	nPos = 0;
	for (int j = 0; j < 11 && nPos < strAfter.GetLength(); ++j)
	{
		if (j > 0)
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

void CDjVuView::FindSelectionZones(DjVuSelection& sel, DjVuTXT* pText, int nStart, int nEnd) const
{
	DjVuSelection selection;
	pText->page_zone.find_zones(selection, nStart, nEnd);

	for (GPosition pos = selection; pos; ++pos)
	{
		if (selection[pos]->ztype >= DjVuTXT::LINE)
			sel.append(selection[pos]);
		else
			GetZones(selection[pos], DjVuTXT::LINE, NULL, sel);
	}
}

CRect CDjVuView::GetSelectionRect(int nPage, const DjVuSelection& selection)
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

bool CDjVuView::IsSelectionVisible(int nPage, const DjVuSelection& selection)
{
	if (m_nLayout == SinglePage && nPage != m_nPage)
		return false;
	if (m_nLayout == Facing && nPage != m_nPage && (!HasFacingPage(m_nPage) || nPage != m_nPage + 1))
		return false;

	CPoint ptScroll = GetScrollPosition();
	CRect rcViewport(ptScroll, GetViewportSize());

	CRect rcSel = GetSelectionRect(nPage, selection);
	if (!rcViewport.IntersectRect(CRect(rcViewport), rcSel) || rcViewport != rcSel)
		return false;

	CFindDlg* pDlg = theApp.GetFindDlg(false);
	if (pDlg != NULL && pDlg->IsWindowVisible())
	{
		CRect rcFindDlg;
		pDlg->GetWindowRect(rcFindDlg);
		ScreenToClient(rcFindDlg);
		rcFindDlg.OffsetRect(ptScroll);

		if (rcFindDlg.IntersectRect(CRect(rcFindDlg), rcSel))
			return false;
	}

	return true;
}

void CDjVuView::EnsureSelectionVisible(int nPage,
		const DjVuSelection& selection, bool bWait)
{
	if (m_nPage != nPage)
	{
		if (bWait)
			RenderPage(nPage, 1000, false);
		else if (m_nLayout == SinglePage || m_nLayout == Facing)
			RenderPage(nPage, -1, false);
	}

	CPoint ptScroll = GetScrollPosition();
	CRect rcViewport(ptScroll, GetViewportSize());

	const Page& page = m_pages[nPage];
	int nHeight = min(page.szBitmap.cy, rcViewport.Height());
	int nWidth = min(page.szBitmap.cx, rcViewport.Width());

	CRect rcSel = GetSelectionRect(nPage, selection);

	// Scroll horizontally
	int nLeftPos = rcViewport.left;
	if (rcSel.left < rcViewport.left)
	{
		nLeftPos = max(page.rcDisplay.left, static_cast<int>(rcSel.left - 0.2*nWidth));
		if (nLeftPos + rcViewport.Width() < rcSel.right)
			nLeftPos = rcSel.right - rcViewport.Width();
		nLeftPos = min(rcSel.left, nLeftPos);
	}
	else if (rcSel.right > rcViewport.right)
	{
		int nRight = min(page.rcDisplay.right, static_cast<int>(rcSel.right + 0.2*nWidth));
		nLeftPos = min(rcSel.left, nRight - rcViewport.Width());
	}

	// Scroll vertically
	int nTopPos = rcViewport.top;
	if (rcSel.top < rcViewport.top)
	{
		nTopPos = max(page.rcDisplay.top, static_cast<int>(rcSel.top - 0.3*nHeight));
		if (nTopPos + rcViewport.Height() < rcSel.bottom)
			nTopPos = rcSel.bottom - rcViewport.Height();
		nTopPos = min(rcSel.top, nTopPos);
	}
	else if (rcSel.bottom > rcViewport.bottom)
	{
		int nBottom = min(m_szDisplay.cy, static_cast<int>(rcSel.bottom + 0.3*nHeight));
		nTopPos = min(rcSel.top, nBottom - rcViewport.Height());
	}

	rcViewport.OffsetRect(0, nTopPos - ptScroll.y);

	// Further scroll the document if selection is obscured by the Find dialog
	CFindDlg* pDlg = theApp.GetFindDlg(false);
	if (pDlg != NULL && pDlg->IsWindowVisible())
	{
		CRect rcFindDlg;
		pDlg->GetWindowRect(rcFindDlg);
		ScreenToClient(rcFindDlg);
		rcFindDlg.OffsetRect(0, nTopPos);

		if (rcSel.bottom > rcFindDlg.top && rcSel.top < rcFindDlg.bottom)
		{
			int nTopSpace = rcFindDlg.top - rcViewport.top;
			int nBottomSpace = rcViewport.bottom - rcFindDlg.bottom;
			if (nTopPos < m_szDisplay.cy - rcViewport.Height() - (rcSel.bottom - rcFindDlg.top)
					&& nTopSpace >= rcSel.Height())
			{
				// Enough space on top, and it is possible to scroll the content
				nTopPos = max(0, rcSel.top - (nTopSpace - rcSel.Height())/2);
				nTopPos = min(rcSel.top, nTopPos);
			}
			else if (nTopPos > rcSel.bottom - rcFindDlg.top && nBottomSpace >= rcSel.Height())
			{
				// Enough space on bottom, and it is possible to scroll the content
				int nBottom = rcSel.bottom + (nBottomSpace - rcSel.Height())/2;
				nTopPos = min(rcSel.top - (rcFindDlg.bottom - rcViewport.top), nBottom - rcViewport.Height());
			}
			else
			{
				// Too little space, or scroll range is not enough. Move the find dialog itself.
				CRect rcMonitor = GetMonitorWorkArea(this);
				ScreenToClient(rcMonitor);
				rcMonitor.OffsetRect(0, nTopPos);
				nTopSpace = rcFindDlg.top - rcMonitor.top;
				nBottomSpace = rcMonitor.bottom - rcFindDlg.bottom;
				int nOffset = 0;
				if (nTopSpace > nBottomSpace)
					nOffset = rcSel.top - 5 - rcFindDlg.bottom;
				else
					nOffset = rcSel.bottom + 5 - rcFindDlg.top;
				nOffset = max(nOffset, rcMonitor.top - rcFindDlg.top);
				nOffset = min(nOffset, rcMonitor.bottom - rcFindDlg.bottom);
				if (nOffset != 0)
				{
					rcFindDlg.OffsetRect(0, nOffset - nTopPos);
					ClientToScreen(rcFindDlg);
					pDlg->MoveWindow(rcFindDlg);
				}
			}
		}
	}

	ScrollToPosition(CPoint(nLeftPos, nTopPos), false);
	UpdateWindow();
}

CRect CDjVuView::TranslatePageRect(int nPage, GRect rect, bool bToDisplay, bool bClip)
{
	Page& page = m_pages[nPage];
	if (!page.info.bDecoded)
		page.Init(m_pSource, nPage);
	CSize szPage = page.GetSize(m_nRotate);

	int nRotate = m_nRotate + page.info.nInitialRotate;
	if (nRotate != 0)
	{
		GRect input(0, 0, szPage.cx, szPage.cy);
		GRect output(0, 0, page.info.szPage.cx, page.info.szPage.cy);

		if ((page.info.nInitialRotate % 2) != 0)
			swap(output.xmax, output.ymax);

		GRectMapper mapper;
		mapper.clear();
		mapper.set_input(input);
		mapper.set_output(output);
		mapper.rotate(4 - nRotate);
		mapper.unmap(rect);
	}

	CRect rcResult;

	if (bToDisplay)
	{
		double fRatioX = (1.0*page.szBitmap.cx) / szPage.cx;
		double fRatioY = (1.0*page.szBitmap.cy) / szPage.cy;

		rcResult = CRect(static_cast<int>(rect.xmin*fRatioX + 0.5),
			static_cast<int>((szPage.cy - rect.ymax)*fRatioY + 0.5),
			static_cast<int>(rect.xmax*fRatioX + 0.5),
			static_cast<int>((szPage.cy - rect.ymin)*fRatioY + 0.5));

		if (bClip)
		{
			rcResult.left = max(rcResult.left, 0);
			rcResult.top = max(rcResult.top, 0);
			rcResult.right = min(rcResult.right, page.szBitmap.cx);
			rcResult.bottom = min(rcResult.bottom, page.szBitmap.cy);
		}

		rcResult.OffsetRect(page.ptOffset);
	}
	else if (bClip)
	{
		rcResult.left = max(rect.xmin, 0);
		rcResult.top = max(szPage.cy - rect.ymax, 0);
		rcResult.right = min(rect.xmax, szPage.cx);
		rcResult.bottom = min(szPage.cy - rect.ymin, szPage.cy);
	}

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

		if (m_nSelectionPage != -1)
		{
			InvalidatePage(m_nSelectionPage);
			m_nSelectionPage = -1;
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

void CDjVuView::UpdateHoverAnnotation()
{
	CPoint ptCursor(-1, -1);
	if (m_nType != Magnify)
	{
		GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);
	}

	UpdateHoverAnnotation(ptCursor);
}

void CDjVuView::UpdateHoverAnnotation(const CPoint& point)
{
	int nPage;
	bool bCustom;
	Annotation* pAnno = GetAnnotationFromPoint(point, nPage, bCustom);

	if (pAnno != m_pHoverAnno)
	{
		if (m_pHoverAnno != NULL)
		{
			m_toolTip.Activate(false);
			m_toolTip.DelTool(this);
			InvalidateAnno(m_pHoverAnno, m_nHoverPage);
			GetMainFrame()->SetMessageText(AFX_IDS_IDLEMESSAGE);
		}

		m_pHoverAnno = pAnno;
		m_nHoverPage = nPage;
		m_bHoverIsCustom = bCustom;

		if (m_pHoverAnno != NULL)
		{
			CString strToolTip = MakeCString(m_pHoverAnno->strURL);
			if (!m_pHoverAnno->bAlwaysShowComment && m_pHoverAnno->strComment.length() > 0)
				strToolTip = MakeCString(m_pHoverAnno->strComment);
			m_toolTip.AddTool(this, strToolTip);
			m_toolTip.Activate(true);
			InvalidateAnno(m_pHoverAnno, m_nHoverPage);

			if (m_nType == Normal)
				GetMainFrame()->SetMessageText(MakeCString(m_pHoverAnno->strURL));
		}

		TRACKMOUSEEVENT tme;
		ZeroMemory(&tme, sizeof(tme));
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE | (m_pHoverAnno == NULL ? TME_CANCEL : 0);
		tme.hwndTrack = m_hWnd;
		tme.dwHoverTime = HOVER_DEFAULT;
		TrackMouseEvent(&tme);

		UpdateCursor();
	}
}

void CDjVuView::UpdateCursor()
{
	CPoint ptCursor;
	GetCursorPos(&ptCursor);

	if (WindowFromPoint(ptCursor) != this)
		return;

	ScreenToClient(&ptCursor);
	CRect rcViewport(CPoint(0, 0), GetViewportSize());
	if (rcViewport.PtInRect(ptCursor))
		SendMessage(WM_SETCURSOR, (WPARAM) m_hWnd, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
}

Annotation* CDjVuView::GetAnnotationFromPoint(const CPoint& point, int& nPage, bool& bCustom)
{
	bCustom = false;
	nPage = -1;

	CRect rcViewport(CPoint(0, 0), GetViewportSize());
	if (!rcViewport.PtInRect(point))
		return NULL;

	nPage = GetPageFromPoint(point);
	if (nPage == -1)
		return NULL;

	Page& page = m_pages[nPage];
	if (page.pBitmap == NULL || page.pBitmap->m_hObject == NULL)
		return NULL;

	map<int, PageSettings>::iterator it = m_pSource->GetSettings()->pageSettings.find(nPage);
	if (it != m_pSource->GetSettings()->pageSettings.end())
	{
		PageSettings& pageSettings = (*it).second;
		list<Annotation>::reverse_iterator rit;
		for (rit = pageSettings.anno.rbegin(); rit != pageSettings.anno.rend(); ++rit)
		{
			Annotation& anno = *rit;
			if (PtInAnnotation(anno, nPage, point))
			{
				bCustom = true;
				return &anno;
			}
		}
	}

	list<Annotation>::reverse_iterator rit;
	for (rit = page.info.anno.rbegin(); rit != page.info.anno.rend(); ++rit)
	{
		Annotation& anno = *rit;
		if (PtInAnnotation(anno, nPage, point))
			return &anno;
	}

	nPage = -1;
	return NULL;
}

bool CDjVuView::PtInAnnotation(const Annotation& anno, int nPage, const CPoint& point)
{
	CPoint pt = ScreenToDjVu(nPage, point + GetScrollPosition() - m_pages[nPage].ptOffset);
	if (!anno.rectBounds.contains(pt.x, pt.y))
		return false;

	if (anno.bOvalShape)
	{
		double fRadiusX = max(1.0, anno.rectBounds.width() / 2.0);
		double fRadiusY = max(1.0, anno.rectBounds.height() / 2.0);
		double fCenterX = (anno.rectBounds.xmax + anno.rectBounds.xmin) / 2.0;
		double fCenterY = (anno.rectBounds.ymax + anno.rectBounds.ymin) / 2.0;
		double fValue = pow((pt.x - fCenterX)/fRadiusX, 2.0) + pow((pt.y - fCenterY)/fRadiusY, 2.0);

		return fValue <= 1.001;
	}
	else if (anno.bIsLine)
	{
		if (anno.points.size() == 2)
		{
			CPoint point2(point.x, point.y + anno.nLineWidth / 2 + 4);
			CPoint pt2 = ScreenToDjVu(nPage, point2 + GetScrollPosition() - m_pages[nPage].ptOffset, false);
			double r = pow(pow(1.0*pt.x - pt2.x, 2.0) + pow(1.0*pt.y - pt2.y, 2.0), 0.5);
			Segment test(anno.points[0].first, anno.points[0].second,
					anno.points[1].first, anno.points[1].second);
			return test.IntersectsDisc(pt.x, pt.y, r);
		}
	}
	else if (anno.points.size() > 2)
	{
		// Construct a long segment with one endpoint pt and other endpoint in
		// the direction (sqrt(2), 1), and intersect all edges with this segment.
		int nBigLength = anno.rectBounds.width() + anno.rectBounds.height();
		Segment test(pt.x, pt.y, pt.x + nBigLength*pow(2.0, 0.5), pt.y + nBigLength);
		int nIntersections = 0;
		for (size_t i = 0; i < anno.points.size(); ++i)
		{
			const pair<int, int>& point2 = (i + 1 < anno.points.size() ? anno.points[i + 1] : anno.points[0]);
			Segment edge(anno.points[i].first, anno.points[i].second, point2.first, point2.second);
			if (edge.Contains(pt.x, pt.y))
				return true;
			if (test.Intersects(edge))
				++nIntersections;
		}
		return (nIntersections % 2) == 1;
	}
	else if (!anno.rects.empty())
	{
		for (size_t i = 0; i < anno.rects.size(); ++i)
		{
			if (anno.rects[i].contains(pt.x, pt.y))
				return true;
		}
	}

	return false;
}

BOOL CDjVuView::PreTranslateMessage(MSG* pMsg)
{
	if (::IsWindow(m_toolTip.m_hWnd))
		m_toolTip.RelayEvent(pMsg);

	return CMyScrollView::PreTranslateMessage(pMsg);
}

void CDjVuView::GoToBookmark(const Bookmark& bookmark, bool bAddHistoryPoint)
{
	if (bookmark.nLinkType == Bookmark::URL)
	{
		GoToURL(bookmark.strURL, bAddHistoryPoint);
	}
	else if (bookmark.nLinkType == Bookmark::Page)
	{
		GoToPage(bookmark.nPage, bAddHistoryPoint);
	}
	else if (bookmark.nLinkType == Bookmark::View)
	{
		if (bAddHistoryPoint)
			AddHistoryPoint();

		int nPage = max(0, min(bookmark.nPage, m_nPageCount - 1));

		if (bookmark.bZoom)
			ZoomTo(bookmark.nZoomType, bookmark.fZoom, false);

		if (bookmark.nPage == nPage)
			ScrollToPage(nPage, bookmark.ptOffset, bookmark.bMargin);
		else
			RenderPage(nPage);

		if (bAddHistoryPoint)
			AddHistoryPoint(bookmark);
	}
}

void CDjVuView::GoToURL(const GUTF8String& url, bool bAddHistoryPoint)
{
	if (url.length() == 0)
		return;

	if (url[0] == '#')
	{
		int nPage = -1;

		try
		{
			char base = '\0';
			const char *s = (const char*)url + 1;
			const char *q = s;
			if (*q == '+' || *q == '-')
				base = *q++;
			if (*q == '\0')
				return;

			GUTF8String str = q;
			if (str.is_int())
			{
				nPage = str.toInt();
				if (base == '+')
					nPage = m_nPage + nPage;
				else if (base == '-')
					nPage = m_nPage - nPage;
				else
					--nPage;
			}
			else
			{
				nPage = m_pSource->GetPageFromId(s);
				if (nPage == -1)
					return;
			}
		}
		catch (GException&)
		{
		}
		catch (...)
		{
			theApp.ReportFatalError();
		}

		GoToPage(nPage, bAddHistoryPoint);
		return;
	}

	// Try to open a .djvu file given by a path (relative or absolute)

	int nPos = url.rsearch('#');
	GUTF8String strPage, strURL = url;
	if (nPos != -1)
	{
		strPage = url.substr(nPos, url.length() - nPos);
		strURL = strURL.substr(0, nPos);
	}

	CString strPathName = MakeCString(strURL);
	bool bFile = false;
	if (_tcsnicmp(strPathName, _T("file:///"), 8) == 0)
	{
		strPathName = strPathName.Mid(8);
		bFile = true;
	}
	else if (_tcsnicmp(strPathName, _T("file://"), 7) == 0)
	{
		strPathName = strPathName.Mid(7);
		bFile = true;
	}
	else
		bFile = !PathIsURL(strPathName);

	// Check if the link leads to a local DjVu file
	LPTSTR pszExt = PathFindExtension(strPathName);
	if (bFile && pszExt != NULL
			&& (_tcsicmp(pszExt, _T(".djvu")) == 0 || _tcsicmp(pszExt, _T(".djv")) == 0))
	{
		if (PathIsRelative(strPathName))
		{
			CString strPath = m_pSource->GetFileName();
			PathRemoveFileSpec(strPath.GetBuffer(MAX_PATH));
			if (PathAppend(strPath.GetBuffer(MAX_PATH), strPathName))
			{
				strPath.ReleaseBuffer();
				strPathName = strPath;
			}
		}

		theApp.OpenDocument(strPathName, strPage, bAddHistoryPoint);
		return;
	}

	// Open a web link
	DWORD dwResult = (DWORD)::ShellExecute(NULL, _T("open"), MakeCString(url), NULL, NULL, SW_SHOW);
	if (dwResult <= 32) // Failure
	{
		AfxMessageBox(LoadString(IDS_FAILED_TO_OPEN) + MakeCString(url));
	}
}

void CDjVuView::GoToPage(int nPage, bool bAddHistoryPoint)
{
	if (bAddHistoryPoint)
		AddHistoryPoint();

	nPage = max(min(nPage, m_nPageCount - 1), 0);
	RenderPage(nPage);

	if (bAddHistoryPoint)
		AddHistoryPoint();
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
	for (nZoom = 0; nZoom < static_cast<int>(zoomLevels.size()) - 1; ++nZoom)
	{
		if (zoomLevels[nZoom] >= fCurrentZoom + 1e-2)
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
	for (nZoom = (int)zoomLevels.size() - 1; nZoom > 0; --nZoom)
	{
		if (zoomLevels[nZoom] <= fCurrentZoom - 1e-2)
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
	pCmdUI->Enable(GetZoom() < 800);
}

void CDjVuView::OnUpdateViewZoomOut(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetZoom() > 10);
}

void CDjVuView::OnChangeMode(UINT nID)
{
	switch (nID)
	{
	case ID_MODE_SELECT:
		m_nMode = Select;
		break;

	case ID_MODE_MAGNIFY:
		m_nMode = MagnifyingGlass;
		break;

	case ID_MODE_SELECT_RECT:
		m_nMode = SelectRect;
		break;

	case ID_MODE_ZOOM_RECT:
		m_nMode = ZoomRect;
		break;

	case ID_MODE_DRAG:
	default:
		m_nMode = Drag;
		break;
	}

	theApp.GetAppSettings()->nDefaultMode = m_nMode;
}

void CDjVuView::OnUpdateMode(CCmdUI* pCmdUI)
{
	int nID = 0;
	switch (m_nMode)
	{
	case Drag:
		nID = ID_MODE_DRAG;
		break;

	case Select:
		nID = ID_MODE_SELECT;
		break;

	case MagnifyingGlass:
		nID = ID_MODE_MAGNIFY;
		break;

	case SelectRect:
		nID = ID_MODE_SELECT_RECT;
		break;

	case ZoomRect:
		nID = ID_MODE_ZOOM_RECT;
		break;
	}

	pCmdUI->SetCheck(nID == pCmdUI->m_nID);
}

void CDjVuView::OnEditCopy()
{
	if (m_nSelectionPage == -1 && !m_bHasSelection)
		return;

	CWaitCursor wait;

	if (!OpenClipboard())
		return;
	EmptyClipboard();

	if (m_bHasSelection)
	{
		wstring text;
		GetNormalizedText(text, true);
		size_t nLength = text.length();

		HGLOBAL hText = NULL, hUnicodeText = NULL;

		// Prepare Unicode text
		hUnicodeText = ::GlobalAlloc(GMEM_MOVEABLE, (nLength + 1)*sizeof(WCHAR));
		if (hUnicodeText != NULL)
		{
			// Lock the handle and copy the text to the buffer.
			LPWSTR pszUnicodeText = (LPWSTR)::GlobalLock(hUnicodeText);
			memmove(pszUnicodeText, text.c_str(), (nLength + 1)*sizeof(WCHAR));

			// Prepare ANSI text
			int nSize = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DISCARDNS,
					pszUnicodeText, -1, NULL, 0, NULL, NULL);
			if (nSize > 0)
			{
				hText = ::GlobalAlloc(GMEM_MOVEABLE, nSize*sizeof(CHAR));
				if (hText != NULL)
				{
					LPSTR pszText = (LPSTR)::GlobalLock(hText);
					WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DISCARDNS,
						pszUnicodeText, -1, pszText, nSize, NULL, NULL);
					::GlobalUnlock(pszText);
				}
			}

			::GlobalUnlock(pszUnicodeText);
		}

		if (hUnicodeText != NULL)
			SetClipboardData(CF_UNICODETEXT, hUnicodeText);
		if (hText != NULL)
			SetClipboardData(CF_TEXT, hText);
	}
	else
	{
		Page& page = m_pages[m_nSelectionPage];
		if (!page.info.bDecoded)
			page.Init(m_pSource, m_nSelectionPage);

		GP<DjVuImage> pImage = m_pSource->GetPage(m_nSelectionPage, NULL);
		if (pImage == NULL)
		{
			AfxMessageBox(IDS_ERROR_DECODING_PAGE, MB_ICONERROR | MB_OK);
		}
		else
		{
			CSize size = page.GetSize(m_nRotate);
			CDIB* pBitmap = CRenderThread::Render(pImage, size, m_displaySettings, m_nDisplayMode, m_nRotate);

			CRect rcCrop = TranslatePageRect(m_nSelectionPage, m_rcSelection, false);
			CDIB* pCropped = pBitmap->Crop(rcCrop);
			delete pBitmap;
			pBitmap = pCropped;

			if (pBitmap == NULL || pBitmap->m_hObject == NULL)
			{
				AfxMessageBox(IDS_ERROR_RENDERING_PAGE, MB_ICONERROR | MB_OK);
			}
			else
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

				pBitmap->SetDPI(page.info.nDPI);
				HGLOBAL hData = pBitmap->SaveToMemory();
				if (hData != NULL)
					SetClipboardData(CF_DIB, hData);
			}

			delete pBitmap;
		}
	}

	CloseClipboard();
}

void CDjVuView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bHasSelection || m_nSelectionPage != -1);
}

void CDjVuView::GetNormalizedText(wstring& text, bool bSelected, int nMaxLength, bool bKeepHyphens)
{
	text.resize(0);
	int nLastHyphen = -1;
	bool bAgain = false;

	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		Page& page = m_pages[nPage];

		wstring chunk;
		if (bSelected)
		{
			if (page.nSelStart != -1)
			{
				ASSERT(page.info.pText != NULL);
				MakeWString(page.info.pText->textUTF8.substr(page.nSelStart, page.nSelEnd - page.nSelStart), chunk);
			}
		}
		else
		{
			if (!page.info.bDecoded || page.info.bHasText && !page.info.bTextDecoded)
			{
				page.Init(m_pSource, nPage, true);
				m_bNeedUpdate = true;
			}

			if (page.info.pText != NULL)
				MakeWString(page.info.pText->textUTF8, chunk);
		}

		if (text.length() + chunk.length() > text.capacity())
			text.reserve(max(2*text.length(), text.length() + chunk.length()));

		// Replace \n with \r\n
		size_t nLastPos = 0, nPos = 0;
		while ((nPos = chunk.find(L'\n', nLastPos)) != wstring::npos)
		{
			text.insert(text.length(), chunk, nLastPos, nPos - nLastPos);
			text += L"\r\n";
			nLastPos = nPos + 1;
		}
		text.insert(text.length(), chunk, nLastPos, chunk.length() - nLastPos);

		// If a word is hyphenated, both initial part and the full word
		// are usually stored in the text layer, and the latter is associated
		// with the part of the word after the hyphen. This is done to ensure
		// that hyphenated words can be located with the "Find" command.
		// The following code cuts the initial parts of hyphenated words,
		// to keep the extracted text clean.

		bAgain = false;
		size_t nHyphen = text.find(L'-', nLastHyphen + 1);
		while (nHyphen != string::npos)
		{
			int nPos;
			for (nPos = (int)nHyphen - 1; nPos >= 0 && text[nPos] > 0x20 && (nHyphen - nPos < 50); --nPos)
				;

			wstring strPrev = text.substr(nPos + 1, nHyphen - nPos - 1);

			bool bFoundEOL = false;
			size_t nWordPos;
			for (nWordPos = nHyphen + 1;
				 nWordPos < text.length() && text[nWordPos] <= 0x20;
				 ++nWordPos)
			{
				if (wcschr(L"\n\r\013\035\038", text[nWordPos]) != NULL)
					bFoundEOL = true;
			}

			if (bFoundEOL)
			{
				if (text.length() - nWordPos < strPrev.length())
				{
					nLastHyphen = (int)nHyphen - 1;
					bAgain = true;
					break;
				}

				if (text.substr(nWordPos, strPrev.length()) == strPrev)
				{
					if (bKeepHyphens)
					{
						// Cut the repeated part from the second line and keep the hyphen.
						text.erase(nWordPos, strPrev.length());
					}
					else
					{
						// Remove the first part together with the hyphen
						// (but keep the line feed).
						text.erase(nPos + 1, strPrev.length() + 1);
					}
				}
			}

			nHyphen = text.find(L'-', nHyphen + 1);
		}

		if (!bAgain)
			nLastHyphen = (int)text.length() - 1;

		if (nMaxLength >= 0 && static_cast<int>(text.length()) >= nMaxLength)
			break;
	}

	if (m_bNeedUpdate)
		UpdateLayout();
}

void CDjVuView::SettingsChanged()
{
	if (m_displaySettings != *theApp.GetDisplaySettings())
	{
		m_displaySettings = *theApp.GetDisplaySettings();

		DeleteBitmaps();
		m_pRenderThread->RejectCurrentJob();

		UpdateVisiblePages();
	}
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

	if (m_nType == Normal)
	{
		m_pSource->GetSettings()->nDisplayMode = nDisplayMode;
	}

	if (m_nDisplayMode != nDisplayMode)
	{
		m_nDisplayMode = nDisplayMode;

		DeleteBitmaps();
		m_pRenderThread->RejectCurrentJob();

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

void CDjVuView::GoToSelection(int nPage, int nStartPos, int nEndPos)
{
	AddHistoryPoint();

	Page& page = m_pages[nPage];
	bool bInfoLoaded;
	CWaitCursor* pWaitCursor = NULL;

	ClearSelection();
	SelectTextRange(nPage, nStartPos, nEndPos, bInfoLoaded, pWaitCursor);

	if (bInfoLoaded)
		UpdateLayout();

	if (!IsSelectionVisible(nPage, page.selection))
	{
		if (pWaitCursor == NULL)
			pWaitCursor = new CWaitCursor();

		EnsureSelectionVisible(nPage, page.selection);
	}

	if (pWaitCursor)
		delete pWaitCursor;

	AddHistoryPoint();
}

void CDjVuView::OnViewFullscreen()
{
	if (m_nType == Fullscreen)
	{
		CFullscreenWnd* pFullscreenWnd = GetMainFrame()->GetFullscreenWnd();
		pFullscreenWnd->Hide();
		return;
	}

	CFullscreenWnd* pWnd = GetMainFrame()->GetFullscreenWnd();

	CDjVuView* pView = new CDjVuView();
	pView->Create(NULL, NULL, WS_CHILD, CRect(0, 0, 0, 0), pWnd, 2);
	pView->m_pDocument = m_pDocument;

	pWnd->Show(this, pView);

	pView->m_nMargin = c_nFullscreenMargin;
	pView->m_nShadowMargin = c_nFullscreenShadowMargin;
	pView->m_nPageGap = c_nFullscreenPageGap;
	pView->m_nFacingGap = c_nFullscreenFacingGap;
	pView->m_nPageBorder = c_nFullscreenPageBorder;
	pView->m_nPageShadow = c_nFullscreenPageShadow;

	pView->m_nType = Fullscreen;
	pView->m_nMode = (theApp.GetAppSettings()->bFullscreenClicks ? NextPrev : Drag);
	pView->m_bFirstPageAlone = m_bFirstPageAlone;
	pView->m_bRightToLeft = m_bRightToLeft;
	pView->m_nZoomType = ZoomFitPage;
	pView->m_nDisplayMode = m_nDisplayMode;
	pView->m_nRotate = m_nRotate;
	pView->m_nPage = m_nPage;

	if (theApp.GetAppSettings()->bFullscreenContinuousScroll)
		pView->m_nLayout = m_nLayout;
	else
		pView->m_nLayout = (m_nLayout == Facing || m_nLayout == ContinuousFacing ? Facing : SinglePage);

	if (theApp.GetAppSettings()->bFullscreenHideScroll)
		pView->ShowScrollBars(false);

	pView->OnInitialUpdate();

	pView->UpdatePageInfoFrom(this);
	pView->CopyBitmapsFrom(this);

	if (pView->m_nLayout == Continuous || pView->m_nLayout == ContinuousFacing)
		pView->UpdateLayout(RECALC);
	pView->RenderPage(m_nPage);
	pView->AddHistoryPoint();

	pView->ShowWindow(SW_SHOW);
	pView->SetFocus();
	pView->UpdateWindow();
}

int CDjVuView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	if (m_nType != Normal)
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

	return CMyScrollView::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

bool CDjVuView::UpdatePageInfoFrom(CDjVuView* pFrom)
{
	ASSERT(m_nPageCount == pFrom->m_nPageCount);

	bool bUpdated = false;
	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		Page& page = m_pages[nPage];
		Page& srcPage = pFrom->m_pages[nPage];

		if (!page.info.bDecoded && srcPage.info.bDecoded)
			bUpdated = true;

		page.info.Update(srcPage.info);
	}

	return bUpdated;
}

void CDjVuView::CopyBitmapsFrom(CDjVuView* pFrom, bool bMove)
{
	ASSERT(m_nPageCount == pFrom->m_nPageCount);

	for (int nPage = 0; nPage < m_nPageCount; ++nPage)
	{
		Page& page = m_pages[nPage];
		Page& srcPage = pFrom->m_pages[nPage];

		if (page.pBitmap == NULL && srcPage.pBitmap != NULL)
		{
			if (bMove)
			{
				page.pBitmap = srcPage.pBitmap;
				page.bBitmapRendered = true;
				srcPage.pBitmap = NULL;
				srcPage.bBitmapRendered = false;
			}
			else
			{
				page.pBitmap = CDIB::CreateDIB(srcPage.pBitmap);
				page.bBitmapRendered = true;
			}
		}
	}
}

void CDjVuView::CopyBitmapFrom(CDjVuView* pFrom, int nPage)
{
	ASSERT(nPage >= 0 && nPage < pFrom->m_nPageCount);

	Page& page = m_pages[nPage];
	Page& srcPage = pFrom->m_pages[nPage];

	if (page.pBitmap == NULL && srcPage.pBitmap != NULL)
	{
		page.pBitmap = CDIB::CreateDIB(srcPage.pBitmap);
		page.bBitmapRendered = true;
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

		int nPos = GetTextPosFromPoint(nPage, point, true);
		bool bInfoLoaded = false;
		CWaitCursor* pWaitCursor = NULL;
		SelectTextRange(nPage, nPos, nPos + 1, bInfoLoaded, pWaitCursor);

		if (bInfoLoaded)
			UpdateLayout();

		if (pWaitCursor != NULL)
			delete pWaitCursor;
	}

	CMyScrollView::OnLButtonDblClk(nFlags, point);
}

void CDjVuView::UpdateKeyboard(UINT nKey, bool bDown)
{
	bool bUpdate = false;
	bool bUpdateWindow = false;
	if (nKey == VK_SHIFT)
	{
		bUpdateWindow = (m_bShiftDown != bDown);
		bUpdate = bUpdateWindow;
		m_bShiftDown = bDown;
	}
	else if (nKey == VK_CONTROL)
	{
		bUpdate = (m_bControlDown != bDown);
		m_bControlDown = bDown;
	}

	if (bUpdateWindow)
	{
		InvalidateViewport();
		UpdateWindow();

		// Notify the magnify window, because it will not repaint
		// itself if is is layered.
		if (m_nType == Magnify)
		{
			CMagnifyWnd* pMagnifyWnd = GetMainFrame()->GetMagnifyWnd();
			pMagnifyWnd->CenterOnPoint(pMagnifyWnd->GetCenterPoint());
		}
	}

	if (bUpdate)
		UpdateCursor();
}

void CDjVuView::OnViewGotoPage()
{
	CGotoPageDlg dlg(m_nPage, m_nPageCount);
	if (dlg.DoModal() == IDOK)
		GoToPage(dlg.m_nPage - 1);
}

void CDjVuView::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case 1:
		if (m_bNeedUpdate)
			UpdateLayout();

		if (m_nType == Fullscreen && !m_bCursorHidden)
		{
			if (GetFocus() == this && !m_bDragging && !m_bPanning && !m_bPopupMenu)
			{
				int nTickCount = ::GetTickCount();
				if (nTickCount - m_nCursorTime > s_nCursorHideDelay)
				{
					m_bCursorHidden = true;
					::ShowCursor(false);
				}
			}
			else
			{
				m_nCursorTime = ::GetTickCount();
			}
		}
		break;

	case 2:
		if (m_bDraggingText || m_bDraggingRect)
		{
			// Autoscroll
			const int nDelta = 25;

			CPoint ptCursor;
			::GetCursorPos(&ptCursor);

			CRect rcViewport(CPoint(0, 0), GetViewportSize());
			ClientToScreen(rcViewport);

			CSize szOffset(0, 0);
			if (ptCursor.x < rcViewport.left)
				szOffset.cx = -nDelta;
			if (ptCursor.x >= rcViewport.right)
				szOffset.cx = nDelta;

			if (ptCursor.y < rcViewport.top)
				szOffset.cy = -nDelta;
			if (ptCursor.y >= rcViewport.bottom)
				szOffset.cy = nDelta;

			OnScrollBy(szOffset);
		}
		break;

	default:
		CMyScrollView::OnTimer(nIDEvent);
		break;
	}
}

void CDjVuView::ShowCursor()
{
	if (m_bCursorHidden)
	{
		::ShowCursor(true);
		m_bCursorHidden = false;
	}

	m_nCursorTime = ::GetTickCount();
}

void CDjVuView::OnFirstPageAlone()
{
	int nAnchorPage = m_nPage;
	CPoint ptOffset = GetScrollPosition() - m_pages[nAnchorPage].ptOffset;

	m_bFirstPageAlone = !m_bFirstPageAlone;

	if (m_nType == Normal)
	{
		theApp.GetAppSettings()->bFirstPageAlone = m_bFirstPageAlone;
		m_pSource->GetSettings()->bFirstPageAlone = m_bFirstPageAlone;
	}

	if (m_nLayout == Facing || m_nLayout == ContinuousFacing)
		SetLayout(m_nLayout, nAnchorPage, ptOffset);
}

void CDjVuView::OnRightToLeftOrder()
{
	int nAnchorPage = m_nPage;
	CPoint ptOffset = GetScrollPosition() - m_pages[nAnchorPage].ptOffset;

	m_bRightToLeft = !m_bRightToLeft;

	if (m_nType == Normal)
		m_pSource->GetSettings()->bRightToLeft = m_bRightToLeft;

	if (m_nLayout == Facing || m_nLayout == ContinuousFacing)
		SetLayout(m_nLayout, nAnchorPage, ptOffset);
}

void CDjVuView::OnUpdateFirstPageAlone(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bFirstPageAlone);
}

void CDjVuView::OnUpdateRightToLeftOrder(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bRightToLeft);
}

bool CDjVuView::IsValidPage(int nPage) const
{
	ASSERT(m_nLayout == Facing || m_nLayout == ContinuousFacing);

	if (m_bFirstPageAlone)
		return (nPage == 0 || nPage % 2 == 1);
	else
		return (nPage % 2 == 0);
}

bool CDjVuView::HasFacingPage(int nPage) const
{
	ASSERT(IsValidPage(nPage));

	if (m_bFirstPageAlone)
		return (nPage > 0 && nPage < m_nPageCount - 1);
	else
		return (nPage < m_nPageCount - 1);
}

void CDjVuView::GetFacingPages(int nPage, Page*& pLeftOrSingle, Page*& pOther,
		int* pnLeftOrSingle, int* pnOther)
{
	ASSERT(IsValidPage(nPage));

	int nLeftPage = nPage;
	int nRightPage = HasFacingPage(nPage) ? nPage + 1 : -1;
	if (nRightPage != -1 && m_bRightToLeft)
		swap(nLeftPage, nRightPage);

	pLeftOrSingle = &m_pages[nLeftPage];
	pOther = (nRightPage != -1 ? &m_pages[nRightPage] : NULL);

	if (pnLeftOrSingle != NULL)
		*pnLeftOrSingle = nLeftPage;
	if (pnOther != NULL)
		*pnOther = nRightPage;
}

int CDjVuView::FixPageNumber(int nPage) const
{
	if (m_nLayout == SinglePage || m_nLayout == Continuous)
		return nPage;

	if (m_bFirstPageAlone)
		return (nPage == 0 ? 0 : nPage - ((nPage - 1) % 2));
	else
		return (nPage - nPage % 2);
}

int CDjVuView::GetNextPage(int nPage) const
{
	if (m_nLayout == SinglePage || m_nLayout == Continuous)
		return nPage + 1;

	ASSERT(IsValidPage(nPage));
	if (m_bFirstPageAlone)
		return (nPage == 0 ? 1 : nPage + 2);
	else
		return (nPage + 2);
}

void CDjVuView::SetLayout(int nLayout, int nPage, const CPoint& ptOffset)
{
	m_nLayout = nLayout;

	if (m_nLayout == SinglePage || m_nLayout == Facing)
	{
		RenderPage(nPage, -1, false);
	}
	else
	{
		UpdateLayout(RECALC);
		UpdatePageSizes(m_pages[nPage].ptOffset.y, ptOffset.y, RECALC);
	}

	CPoint ptTop = m_pages[nPage].ptOffset + ptOffset;
	ScrollToPosition(ptTop, false);
	UpdateWindow();
}

void CDjVuView::UpdateDragAction()
{
	if (m_bDragging && !m_bInsideMouseMove)
	{
		m_ptPrevCursor = CPoint(-1, -1);

		CPoint ptCursor;
		::GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);

		OnMouseMove(MK_LBUTTON, ptCursor);
	}
}

void CDjVuView::StartMagnify()
{
	CMagnifyWnd* pMagnifyWnd = GetMainFrame()->GetMagnifyWnd();

	CDjVuView* pView = new CDjVuView();
	pView->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), pMagnifyWnd, 2);
	pView->m_pDocument = m_pDocument;

	pMagnifyWnd->Init(this, pView);

	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);
	int nPage = GetPageNearPoint(ptCursor);
	if (nPage == -1)
		nPage = m_nPage;
	nPage = FixPageNumber(nPage);

	CSize szView = pMagnifyWnd->GetViewSize();
	int nExtraMargin = max(szView.cx, szView.cy);

	pView->m_nMargin = m_nMargin + nExtraMargin;
	pView->m_nShadowMargin = m_nShadowMargin + nExtraMargin;
	pView->m_nPageGap = m_nPageGap;
	pView->m_nFacingGap = m_nFacingGap;
	pView->m_nPageBorder = m_nPageBorder;
	pView->m_nPageShadow = m_nPageShadow;

	pView->m_nType = Magnify;
	pView->m_nMode = Drag;
	pView->m_nLayout = m_nLayout;
	pView->m_bFirstPageAlone = m_bFirstPageAlone;
	pView->m_bRightToLeft = m_bRightToLeft;
	pView->m_nDisplayMode = m_nDisplayMode;
	pView->m_nRotate = m_nRotate;
	pView->m_nPage = nPage;

	double fZoom = GetZoom();
	double fZoomActualSize = GetZoom(ZoomActualSize);
	if (m_nZoomType == ZoomActualSize || fZoom < fZoomActualSize)
	{
		pView->m_nZoomType = ZoomActualSize;
	}
	else
	{
		pView->m_nZoomType = ZoomPercent;
		pView->m_fZoom = fZoom;
	}

	pView->ShowScrollBars(false);

	pView->OnInitialUpdate();

	pView->UpdatePageInfoFrom(this);
	if (m_nLayout == Continuous || m_nLayout == ContinuousFacing)
		pView->UpdateLayout(RECALC);
	pView->RenderPage(nPage);

	UpdateCursor();
	pView->UpdateCursor();
}

void CDjVuView::UpdateMagnifyWnd(bool bInitial)
{
	CMagnifyWnd* pMagnifyWnd = GetMainFrame()->GetMagnifyWnd();
	if (!bInitial && !pMagnifyWnd->IsWindowVisible())
	{
		StopDragging();
		return;
	}

	ASSERT(pMagnifyWnd->GetOwner() == this);

	CPoint ptCursor;
	::GetCursorPos(&ptCursor);

	if (!bInitial && ptCursor == m_ptPrevCursor)
		return;
	m_ptPrevCursor = ptCursor;

	CDjVuView* pView = pMagnifyWnd->GetView();
	pView->SetRedraw(false);

	CPoint ptCenter = ptCursor;

	ScreenToClient(&ptCursor);
	int nPage = GetPageNearPoint(ptCursor);
	if (nPage == -1)
		nPage = m_nPage;

	if (pView->UpdatePageInfoFrom(this))
		pView->UpdateLayout();

	if (pView->m_nLayout == SinglePage || pView->m_nLayout == Facing)
	{
		int nNewPage = FixPageNumber(nPage);
		if (nNewPage != pView->m_nPage)
			pView->RenderPage(nNewPage);
	}

	const Page& page = m_pages[nPage];
	CSize szPage = page.GetSize(m_nRotate);
	double fRatioX = pView->m_pages[nPage].szBitmap.cx / (1.0*page.szBitmap.cx);
	double fRatioY = pView->m_pages[nPage].szBitmap.cy / (1.0*page.szBitmap.cy);

	ptCursor += GetScrollPosition() - page.ptOffset;
	CPoint ptResult(static_cast<int>(ptCursor.x*fRatioX + 0.5),
			static_cast<int>(ptCursor.y*fRatioY + 0.5));

	CSize szView = pMagnifyWnd->GetViewSize();
	CPoint ptOffset = pView->m_pages[nPage].ptOffset + ptResult -
			CPoint(szView.cx / 2, szView.cy / 2);
	pView->OnScrollBy(ptOffset - pView->GetScrollPosition());

	pView->SetRedraw(true);

	pMagnifyWnd->CenterOnPoint(ptCenter);
}

bool CDjVuView::OnStartPan()
{
	if (m_bDragging)
		return false;

	ShowCursor();
	return true;
}

void CDjVuView::OnEndPan()
{
	ShowCursor();
}

CString FormatRange(const set<int>& pages)
{
	if (pages.empty())
		return _T("");

	CString strRange;
	set<int>::const_iterator it = pages.begin();
	int nStart = *it, nEnd = *it;
	++it;
	for (;;)
	{
		if (it == pages.end() || *it > nEnd + 1)
		{
			strRange += (strRange.IsEmpty() ? _T("") : _T(","));
			if (nStart == nEnd)
				strRange += FormatString(_T("%d"), nStart + 1);
			else
				strRange += FormatString(_T("%d-%d"), nStart + 1, nEnd + 1);

			if (it != pages.end())
				nStart = *it;
		}

		if (it != pages.end())
			nEnd = *it++;
		else
			break;
	}

	return strRange;
}

void CDjVuView::OnUpdate(const Observable* source, const Message* message)
{
	if (message->code == PAGE_RENDERED)
	{
		const BitmapMsg* msg = (const BitmapMsg*) message;
		PageRendered(msg->nPage, msg->pDIB);
	}
	else if (message->code == PAGE_DECODED)
	{
		const PageMsg* msg = (const PageMsg*) message;
		PageDecoded(msg->nPage);
	}
	else if (message->code == THUMBNAIL_CLICKED)
	{
		const PageMsg* msg = (const PageMsg*) message;
		GoToPage(msg->nPage);
	}
	else if (message->code == LINK_CLICKED)
	{
		const LinkClicked* msg = (const LinkClicked*) message;
		GoToURL(msg->url);
	}
	else if (message->code == BOOKMARK_CLICKED)
	{
		const BookmarkMsg* msg = (const BookmarkMsg*) message;
		GoToBookmark(*msg->pBookmark);
	}
	else if (message->code == SEARCH_RESULT_CLICKED)
	{
		const SearchResultClicked* msg = (const SearchResultClicked*) message;
		GoToSelection(msg->nPage, msg->nSelStart, msg->nSelEnd);
	}
	else if (message->code == APP_SETTINGS_CHANGED)
	{
		SettingsChanged();
	}
	else if (message->code == ANNOTATION_DELETED)
	{
		const AnnotationMsg* msg = (const AnnotationMsg*) message;

		if (m_pClickedAnno == msg->pAnno)
			m_pClickedAnno = NULL;

		if (!m_bDragging && !m_bPanning)
			UpdateHoverAnnotation();
	}
	else if (message->code == ANNOTATION_ADDED)
	{
		const AnnotationMsg* msg = (const AnnotationMsg*) message;
		InvalidateAnno(msg->pAnno, msg->nPage);

		if (!m_bDragging && !m_bPanning)
			UpdateHoverAnnotation();
	}
	else if (message->code == KEY_STATE_CHANGED)
	{
		const KeyStateChanged* msg = (const KeyStateChanged*) message;
		UpdateKeyboard(msg->nKey, msg->bPressed);
	}
	else if (message->code == ANNOTATIONS_CHANGED)
	{
		InvalidateViewport();
		if (!m_bDragging && !m_bPanning)
			UpdateHoverAnnotation();
	}
	else if (message->code == BOOKMARKS_CHANGED)
	{
		if (m_nType == Normal)
		{
			CBookmarksView* pBookmarks = GetMDIChild()->GetBookmarksTree(true);
			pBookmarks->AddObserver(this);
			pBookmarks->LoadUserBookmarks();
		}
	}
	else if (message->code == PRINT_PAGES)
	{
		const PageRangeMsg* msg = (const PageRangeMsg*) message;
		CString strRange = FormatRange(msg->pages);
		DoPrint(strRange);
	}
	else if (message->code == EXPORT_PAGES)
	{
		const PageRangeMsg* msg = (const PageRangeMsg*) message;
		ExportPages(msg->pages);
	}
}

void CDjVuView::UpdatePageNumber()
{
	if (m_nLayout == SinglePage || m_nLayout == Facing)
	{
		UpdateObservers(PageMsg(CURRENT_PAGE_CHANGED, m_nPage));
	}
	else
	{
		int nCurrentPage = CalcCurrentPage();
		if (nCurrentPage != m_nPage)
		{
			m_nPage = nCurrentPage;
			UpdateObservers(PageMsg(CURRENT_PAGE_CHANGED, m_nPage));
		}
	}

	if (m_nType == Normal)
	{
		Bookmark bmView;
		CreateBookmarkFromView(bmView);

		m_pSource->GetSettings()->nPage = bmView.nPage;
		m_pSource->GetSettings()->ptOffset = bmView.ptOffset;
	}
}

void CDjVuView::OnHighlight(UINT nID)
{
	if (m_nSelectionPage == -1 && !m_bHasSelection)
		return;

	CAnnotationDlg dlg(IDS_CREATE_ANNOTATION);

	dlg.m_nBorderType = theApp.GetAnnoTemplate()->nBorderType;
	dlg.m_crBorder = theApp.GetAnnoTemplate()->crBorder;
	dlg.m_bHideInactiveBorder = theApp.GetAnnoTemplate()->bHideInactiveBorder;
	dlg.m_nFillType = theApp.GetAnnoTemplate()->nFillType;
	dlg.m_crFill = theApp.GetAnnoTemplate()->crFill;
	dlg.m_nTransparency = static_cast<int>(theApp.GetAnnoTemplate()->fTransparency*100.0 + 0.5);
	dlg.m_bHideInactiveFill = theApp.GetAnnoTemplate()->bHideInactiveFill;
	dlg.m_bAlwaysShowComment = !m_bHasSelection ? theApp.GetAnnoTemplate()->bAlwaysShowComment : false;
	dlg.m_bDisableShowComment = m_bHasSelection;

	if (dlg.DoModal() != IDOK)
		return;

	theApp.GetAnnoTemplate()->nBorderType = dlg.m_nBorderType;
	theApp.GetAnnoTemplate()->crBorder = dlg.m_crBorder;
	theApp.GetAnnoTemplate()->bHideInactiveBorder = !!dlg.m_bHideInactiveBorder;
	theApp.GetAnnoTemplate()->nFillType = dlg.m_nFillType;
	theApp.GetAnnoTemplate()->crFill = dlg.m_crFill;
	theApp.GetAnnoTemplate()->fTransparency = dlg.m_nTransparency / 100.0;
	theApp.GetAnnoTemplate()->bHideInactiveFill = !!dlg.m_bHideInactiveFill;
	theApp.GetAnnoTemplate()->bAlwaysShowComment = !!dlg.m_bAlwaysShowComment;

	Annotation annoTemplate = *theApp.GetAnnoTemplate();
	annoTemplate.strComment = MakeUTF8String(dlg.m_strComment);

	Annotation* pNewAnno = NULL;
	int nAnnoPage = -1;

	if (m_nSelectionPage != -1)
	{
		Annotation anno = annoTemplate;
		anno.rects.push_back(m_rcSelection);
		anno.UpdateBounds();

		nAnnoPage = m_nSelectionPage;
		pNewAnno = m_pSource->GetSettings()->AddAnnotation(anno, m_nSelectionPage);
	}
	else if (m_bHasSelection)
	{
		for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		{
			Page& page = m_pages[nPage];

			if (!page.selection.isempty())
			{
				Annotation anno = annoTemplate;
				for (GPosition pos = page.selection; pos; ++pos)
					anno.rects.push_back(page.selection[pos]->rect);
				anno.UpdateBounds();

				Annotation* pAnno = m_pSource->GetSettings()->AddAnnotation(anno, nPage);
				if (pNewAnno == NULL)
				{
					nAnnoPage = nPage;
					pNewAnno = pAnno;
				}
			}
		}
	}

	ClearSelection();

	if (dlg.m_bAddBookmark && pNewAnno != NULL)
	{
		Bookmark bookmark;
		CreateBookmarkFromAnnotation(bookmark, pNewAnno, nAnnoPage);
		bookmark.strTitle = MakeUTF8String(dlg.m_strBookmark);

		m_pSource->GetSettings()->bookmarks.push_back(bookmark);
		Bookmark& bmNew = m_pSource->GetSettings()->bookmarks.back();

		CBookmarksView* pBookmarks = GetMDIChild()->GetBookmarksTree();
		pBookmarks->AddObserver(this);
		pBookmarks->AddBookmark(bmNew);
	}
}

void CDjVuView::OnUpdateHighlight(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bHasSelection || m_nSelectionPage != -1);
}

void CDjVuView::OnDeleteAnnotation()
{
	if (m_pClickedAnno == NULL || !m_bClickedCustom || m_nClickedPage == -1)
		return;

	if (AfxMessageBox(IDS_PROMPT_ANNOTATION_DELETE, MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
	{
		m_pSource->GetSettings()->DeleteAnnotation(m_pClickedAnno, m_nClickedPage);
	}
}

void CDjVuView::OnEditAnnotation()
{
	if (m_pClickedAnno == NULL || !m_bClickedCustom || m_nClickedPage == -1)
		return;

	CAnnotationDlg dlg(IDS_EDIT_ANNOTATION);

	dlg.m_bEnableBookmark = false;
	dlg.m_nBorderType = m_pClickedAnno->nBorderType;
	dlg.m_crBorder = m_pClickedAnno->crBorder;
	dlg.m_bHideInactiveBorder = m_pClickedAnno->bHideInactiveBorder;
	dlg.m_nFillType = m_pClickedAnno->nFillType;
	dlg.m_crFill = m_pClickedAnno->crFill;
	dlg.m_nTransparency = static_cast<int>(m_pClickedAnno->fTransparency*100.0 + 0.5);
	dlg.m_bHideInactiveFill = m_pClickedAnno->bHideInactiveFill;
	dlg.m_strComment = MakeCString(m_pClickedAnno->strComment);
	dlg.m_bAlwaysShowComment = m_pClickedAnno->bAlwaysShowComment;
	dlg.m_bDisableShowComment = m_pClickedAnno->rects.size() > 1;

	if (dlg.DoModal() == IDOK)
	{
		theApp.GetAnnoTemplate()->nBorderType = dlg.m_nBorderType;
		theApp.GetAnnoTemplate()->crBorder = dlg.m_crBorder;
		theApp.GetAnnoTemplate()->bHideInactiveBorder = !!dlg.m_bHideInactiveBorder;
		theApp.GetAnnoTemplate()->nFillType = dlg.m_nFillType;
		theApp.GetAnnoTemplate()->crFill = dlg.m_crFill;
		theApp.GetAnnoTemplate()->fTransparency = dlg.m_nTransparency / 100.0;
		theApp.GetAnnoTemplate()->bHideInactiveFill = !!dlg.m_bHideInactiveFill;
		theApp.GetAnnoTemplate()->bAlwaysShowComment = !!dlg.m_bAlwaysShowComment;

		m_pClickedAnno->nBorderType = dlg.m_nBorderType;
		m_pClickedAnno->crBorder = dlg.m_crBorder;
		m_pClickedAnno->bHideInactiveBorder = !!dlg.m_bHideInactiveBorder;
		m_pClickedAnno->nFillType = dlg.m_nFillType;
		m_pClickedAnno->crFill = dlg.m_crFill;
		m_pClickedAnno->fTransparency = dlg.m_nTransparency / 100.0;
		m_pClickedAnno->bHideInactiveFill = !!dlg.m_bHideInactiveFill;
		m_pClickedAnno->strComment = MakeUTF8String(dlg.m_strComment);
		m_pClickedAnno->bAlwaysShowComment = !!dlg.m_bAlwaysShowComment;

		InvalidateAnno(m_pClickedAnno, m_nClickedPage);
	}
}

void CDjVuView::OnAddBookmark()
{
	Bookmark bookmark;
	UINT nDlgTitle;

	if (m_nSelectionPage != -1 || m_bHasSelection)
	{
		CreateBookmarkFromSelection(bookmark);
		nDlgTitle = IDS_BOOKMARK_SELECTION;
	}
	else if (m_pClickedAnno != NULL)
	{
		CreateBookmarkFromAnnotation(bookmark, m_pClickedAnno, m_nClickedPage);
		nDlgTitle = IDS_BOOKMARK_ANNOTATION;
	}
	else
	{
		CreateBookmarkFromView(bookmark);
		nDlgTitle = IDS_BOOKMARK_VIEW;
	}

	CBookmarkDlg dlg(nDlgTitle);
	dlg.m_strTitle = MakeCString(bookmark.strTitle);

	if (dlg.DoModal() == IDOK)
	{
		bookmark.strTitle = MakeUTF8String(dlg.m_strTitle);

		m_pSource->GetSettings()->bookmarks.push_back(bookmark);
		Bookmark& bmNew = m_pSource->GetSettings()->bookmarks.back();

		CBookmarksView* pBookmarks = GetMDIChild()->GetBookmarksTree();
		if (m_nType == Normal) {
			pBookmarks->AddObserver(this);
		}
		pBookmarks->AddBookmark(bmNew);
	}
}

void MakeBookmarkTitle(const wstring& strText, wstring& result)
{
	result.resize(0);
	result.reserve(strText.length());

	bool bSkipSpaces = true;
	for (size_t i = 0; i < strText.length(); ++i)
	{
		if (strText[i] <= 0x20)
		{
			if (!bSkipSpaces)
				result += L' ';
			bSkipSpaces = true;
		}
		else
		{
			result += strText[i];
			bSkipSpaces = false;
		}
	}

	while (!result.empty() && result[result.length() - 1] <= 0x20)
		result.resize(result.length() - 1);

	if (result.length() > 100)
	{
		result.erase(100);
		while (!result.empty() && result[result.length() - 1] <= 0x20)
			result.resize(result.length() - 1);
		result += L"...";
	}
}

bool CDjVuView::CreateBookmarkFromSelection(Bookmark& bookmark)
{
	if (m_nSelectionPage != -1)
	{
		bookmark.nLinkType = Bookmark::View;
		bookmark.nPage = m_nSelectionPage;
		bookmark.ptOffset = CPoint(m_rcSelection.xmin, m_rcSelection.ymax);
		bookmark.bMargin = true;
		return true;
	}
	else if (m_bHasSelection)
	{
		for (int nPage = 0; nPage < m_nPageCount; ++nPage)
		{
			const Page& page = m_pages[nPage];

			if (!page.selection.isempty())
			{
				GPosition pos = page.selection;
				GRect rect = page.selection[pos]->rect;

				bookmark.nLinkType = Bookmark::View;
				bookmark.nPage = nPage;
				bookmark.ptOffset = CPoint(rect.xmin, rect.ymax);
				bookmark.bMargin = true;

				wstring strText, strTitle;
				GetNormalizedText(strText, true, 200, false);
				MakeBookmarkTitle(strText, strTitle);
				bookmark.strTitle = MakeUTF8String(strTitle);

				return true;
			}
		}
	}

	return false;
}

void CDjVuView::CreateBookmarkFromView(Bookmark& bookmark)
{
	int nPage;
	if (m_nLayout == SinglePage || m_nLayout == Facing)
		nPage = m_nPage;
	else
		nPage = CalcTopPage();

	CPoint ptOffset = GetScrollPosition() - m_pages[nPage].ptOffset;
	CPoint ptDjVuOffset = ScreenToDjVu(nPage, ptOffset);

	bookmark.nLinkType = Bookmark::View;
	bookmark.nPage = nPage;
	bookmark.ptOffset = ptDjVuOffset;
	bookmark.bMargin = false;
}

void CDjVuView::CreateBookmarkFromAnnotation(Bookmark& bookmark, const Annotation* pAnno, int nPage)
{
	bookmark.nLinkType = Bookmark::View;
	bookmark.nPage = nPage;
	bookmark.ptOffset = CPoint(pAnno->rectBounds.xmin, pAnno->rectBounds.ymax);
	bookmark.bMargin = true;

	wstring strComment, strTitle;
	MakeWString(pAnno->strComment, strComment);
	MakeBookmarkTitle(strComment, strTitle);
	bookmark.strTitle = MakeUTF8String(strTitle);
}

void CDjVuView::CreateBookmarkFromPage(Bookmark& bookmark, int nPage)
{
	bookmark.nLinkType = Bookmark::Page;
	bookmark.nPage = nPage;
}

void CDjVuView::OnZoomToSelection()
{
	if (m_nSelectionPage == -1)
		return;

	AddHistoryPoint();
	int nPrevZoomType = m_nZoomType;
	double fPrevZoom = GetZoom();

	CSize szViewport = ::GetClientSize(this);
	if (szViewport.cx > m_szScrollBars.cx && szViewport.cy > m_szScrollBars.cy)
	{
		szViewport.cx -= m_szScrollBars.cx;
		szViewport.cy -= m_szScrollBars.cy;
	}

	const Page& page = m_pages[m_nSelectionPage];
	CSize szPage = page.GetSize(m_nRotate);

	double fScale = min(1.0*szViewport.cx/m_rcSelection.width(),
			1.0*szViewport.cy/m_rcSelection.height());
	double fZoom = 100.0*fScale*page.info.nDPI/m_nScreenDPI;

	ZoomTo(ZoomPercent, fZoom, false);
	szViewport = GetViewportSize();

	CRect rcSel = TranslatePageRect(m_nSelectionPage, m_rcSelection);
	CPoint ptOffset = rcSel.TopLeft() - page.ptOffset;

	if (rcSel.Width() < szViewport.cx)
		ptOffset.x -= (szViewport.cx - rcSel.Width()) / 2;
	if (rcSel.Height() < szViewport.cy)
		ptOffset.y -= (szViewport.cy - rcSel.Height()) / 2;

	ScrollToPage(m_nSelectionPage, ScreenToDjVu(m_nSelectionPage, ptOffset));

	Bookmark bookmark;
	CreateBookmarkFromView(bookmark);
	bookmark.bZoom = true;
	bookmark.nZoomType = ZoomPercent;
	bookmark.fZoom = fZoom;
	bookmark.nPrevZoomType = nPrevZoomType;
	bookmark.fPrevZoom = fPrevZoom;

	AddHistoryPoint(bookmark, true);
}

void CDjVuView::OnSwitchFocus(UINT nID)
{
	if (m_nType != Normal)
		return;

	if (GetFocus() != this)
	{
		SetFocus();
		return;
	}

	CMDIChild* pMDIChild = GetMDIChild();
	if (!pMDIChild->IsNavPaneHidden() && !pMDIChild->IsNavPaneCollapsed())
		pMDIChild->GetNavPane()->SetFocus();
}

void CDjVuView::OnUpdateSwitchFocus(CCmdUI* pCmdUI)
{
	pCmdUI->Enable();
}

void CDjVuView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CMyScrollView::OnShowWindow(bShow, nStatus);

	m_bUpdateBitmaps = bShow && GetParent()->IsWindowVisible();
	UpdateVisiblePages();
}

LRESULT CDjVuView::OnShowParent(WPARAM wParam, LPARAM lParam)
{
	m_bUpdateBitmaps = !!wParam;
	UpdateVisiblePages();

	return 0;
}

void CDjVuView::GoToHistoryPoint(const HistoryPoint& pt, const HistoryPoint* pCurPt)
{
	if (pCurPt != NULL && pCurPt->bookmark.bZoom)
		ZoomTo(pCurPt->bookmark.nPrevZoomType, pCurPt->bookmark.fPrevZoom, false);

	GoToBookmark(pt.bookmark, false);
}

void CDjVuView::OnViewBack()
{
	HistoryPoint pt;
	CreateBookmarkFromView(pt.bmView);
	pt.bookmark = pt.bmView;

	if (m_history.empty() || m_historyPoint == m_history.begin() && *m_historyPoint == pt)
		return;

	ASSERT(m_historyPoint != m_history.end());

	// Account for view changes not reflected in the history.
	// If we are positioned at a history point, this will do nothing
	// and we will proceed to the preious history entry as planned.
	AddHistoryPoint(pt);

	const HistoryPoint* pCurPt = &(*m_historyPoint);
	const HistoryPoint& prevPt = *(--m_historyPoint);
	GoToHistoryPoint(prevPt, pCurPt);
}

void CDjVuView::OnUpdateViewBack(CCmdUI* pCmdUI)
{
	HistoryPoint pt;
	CreateBookmarkFromView(pt.bmView);
	pt.bookmark = pt.bmView;

	pCmdUI->Enable(!m_history.empty()
		&& (m_historyPoint != m_history.begin() || *m_historyPoint != pt));
}

void CDjVuView::OnViewForward()
{
	if (m_history.empty())
		return;

	ASSERT(m_historyPoint != m_history.end());
	list<HistoryPoint>::iterator it = m_history.end();
	if (m_historyPoint == --it)
		return;

	const HistoryPoint& pt = *(++m_historyPoint);
	GoToHistoryPoint(pt);
}

void CDjVuView::OnUpdateViewForward(CCmdUI* pCmdUI)
{
	list<HistoryPoint>::iterator it = m_history.end();
	pCmdUI->Enable(!m_history.empty() && m_historyPoint != --it);
}

bool CDjVuView::AddHistoryPoint()
{
	HistoryPoint pt;
	CreateBookmarkFromView(pt.bmView);
	pt.bookmark = pt.bmView;

	return AddHistoryPoint(pt);
}

bool CDjVuView::AddHistoryPoint(const Bookmark& bookmark, bool bForce)
{
	HistoryPoint pt;
	CreateBookmarkFromView(pt.bmView);
	pt.bookmark = bookmark;

	return AddHistoryPoint(pt, bForce);
}

bool CDjVuView::AddHistoryPoint(const HistoryPoint& pt, bool bForce)
{
	ASSERT(pt.bmView.nLinkType == Bookmark::View);

	if (!m_history.empty())
	{
		ASSERT(m_historyPoint != m_history.end());
		if (pt == *m_historyPoint && !bForce)
			return false;

		++m_historyPoint;
		m_history.erase(m_historyPoint, m_history.end());
	}

	m_history.push_back(pt);

	m_historyPoint = m_history.end();
	--m_historyPoint;
	return true;
}
