//	WinDjView 0.1
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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDjVuView

HCURSOR CDjVuView::hCursorHand = NULL;
HCURSOR CDjVuView::hCursorDrag = NULL;

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
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CDjVuView construction/destruction

CDjVuView::CDjVuView()
	: m_nPage(-1), m_nPageCount(0), m_pBitmap(NULL), m_bRendered(false),
	  m_nZoomType(ZoomPercent), m_fZoom(100.0), m_nLayout(SinglePage),
	  m_nRotate(0), m_bDragging(false)
{
	m_pRenderThread = NULL;
}

CDjVuView::~CDjVuView()
{
	delete m_pBitmap;
}

BOOL CDjVuView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CScrollView::PreCreateWindow(cs);
}

// CDjVuView drawing

void CDjVuView::OnDraw(CDC* pDC)
{
	if (m_pImage != NULL)
	{
		if (pDC->IsPrinting())
		{
			// TODO: Print preview
			if (m_pBitmap != NULL)
			{
				int nOffsetX = pDC->GetDeviceCaps(PHYSICALOFFSETX);
				int nOffsetY = pDC->GetDeviceCaps(PHYSICALOFFSETY);

				pDC->SetViewportOrg(-nOffsetX, -nOffsetY);

				m_pBitmap->Draw(pDC, CPoint(0, 0), m_szPage);
			}
			return;
		}

		if (m_pBitmap != NULL)
		{
			if (m_bRendered)
			{
				DrawOffscreen(pDC);
			}
			else
			{
				DrawStretch(pDC);
			}
		}
		else
		{
			DrawWhite(pDC);
		}
	}
}

void CDjVuView::DrawWhite(CDC* pDC)
{
	CPoint ptOffset = GetScrollPosition();

	CRect rcClient;
	GetClientRect(rcClient);
	rcClient.OffsetRect(ptOffset);

	COLORREF clrWindow = ::GetSysColor(COLOR_WINDOW);
	pDC->FillSolidRect(rcClient, clrWindow);

	CRect rcBorder(m_ptPageOffset.x - 1, m_ptPageOffset.y - 1,
		m_ptPageOffset.x + m_szDisplayPage.cx,
		m_ptPageOffset.y + m_szDisplayPage.cy);

	CPoint points[] = { rcBorder.TopLeft(), CPoint(rcBorder.right, rcBorder.top),
		rcBorder.BottomRight(), CPoint(rcBorder.left, rcBorder.bottom),
		rcBorder.TopLeft() };

	CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_3DDKSHADOW));
	CPen* pOldPen = pDC->SelectObject(&pen);
	pDC->Polyline((LPPOINT)points, 5);
	pDC->SelectObject(pOldPen);
}

void CDjVuView::DrawOffscreen(CDC* pDC)
{
	ASSERT(m_pImage != NULL);
	ASSERT(m_pBitmap != NULL && m_pBitmap->m_hObject != NULL);

	CPoint ptOffset = GetScrollPosition();

	CRect rcClip;
	pDC->GetClipBox(rcClip);

	CPoint ptPartOffset(max(rcClip.left - m_ptPageOffset.x, 0),
		max(rcClip.top - m_ptPageOffset.y, 0));

	CSize szPageClip = m_szDisplayPage - ptPartOffset;
	CSize szPart(min(rcClip.Width(), szPageClip.cx), min(rcClip.Height(), szPageClip.cy));
	CRect rcPart(ptPartOffset, szPart);

	m_pBitmap->DrawDC(pDC, m_ptPageOffset + ptPartOffset, rcPart);

	CRect rcBorder(m_ptPageOffset.x - 1, m_ptPageOffset.y - 1,
		m_ptPageOffset.x + m_szDisplayPage.cx,
		m_ptPageOffset.y + m_szDisplayPage.cy);

	CPoint points[] = { rcBorder.TopLeft(), CPoint(rcBorder.right, rcBorder.top),
		rcBorder.BottomRight(), CPoint(rcBorder.left, rcBorder.bottom),
		rcBorder.TopLeft() };

	CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_3DDKSHADOW));
	CPen* pOldPen = pDC->SelectObject(&pen);
	pDC->Polyline((LPPOINT)points, 5);
	pDC->SelectObject(pOldPen);

	int nSaveDC = pDC->SaveDC();
	rcBorder.InflateRect(0, 0, 1, 1);
	pDC->ExcludeClipRect(rcBorder);

	COLORREF clrWindow = ::GetSysColor(COLOR_WINDOW);
	pDC->FillSolidRect(0, 0, m_szDisplay.cx, m_szDisplay.cy, clrWindow);

	pDC->RestoreDC(nSaveDC);
}

void CDjVuView::DrawStretch(CDC* pDC)
{
	ASSERT(m_pImage != NULL);
	ASSERT(m_pBitmap != NULL && m_pBitmap->m_hObject != NULL);

	m_pBitmap->Draw(pDC, m_ptPageOffset, m_szDisplayPage);

	CRect rcBorder(m_ptPageOffset.x - 1, m_ptPageOffset.y - 1,
		m_ptPageOffset.x + m_szDisplayPage.cx,
		m_ptPageOffset.y + m_szDisplayPage.cy);

	CPoint points[] = { rcBorder.TopLeft(), CPoint(rcBorder.right, rcBorder.top),
		rcBorder.BottomRight(), CPoint(rcBorder.left, rcBorder.bottom),
		rcBorder.TopLeft() };

	CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_3DDKSHADOW));
	CPen* pOldPen = pDC->SelectObject(&pen);
	pDC->Polyline((LPPOINT)points, 5);
	pDC->SelectObject(pOldPen);

	int nSaveDC = pDC->SaveDC();
	rcBorder.InflateRect(0, 0, 1, 1);
	pDC->ExcludeClipRect(rcBorder);

	COLORREF clrWindow = ::GetSysColor(COLOR_WINDOW);
	pDC->FillSolidRect(0, 0, m_szDisplay.cx, m_szDisplay.cy, clrWindow);

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

	m_pRenderThread = new CRenderThread(this);

	m_nZoomType = CAppSettings::nDefaultZoomType;
	m_fZoom = CAppSettings::fDefaultZoom;
	if (m_nZoomType == ZoomPercent)
		m_fZoom = 100.0;

	m_nLayout = CAppSettings::nDefaultLayout;

	m_nPageCount = GetDocument()->GetPageCount();
	m_nPage = 0;

	RenderPage(m_nPage);
}

BOOL CDjVuView::OnEraseBkgnd(CDC* pDC)
{
	CBrush br(GetSysColor(COLOR_WINDOW));
	FillOutsideRect(pDC, &br);
	return true;
}

void CDjVuView::RenderPage(int nPage)
{
	m_nPage = nPage;

	G_TRY
	{
		m_pImage = GetDocument()->GetPage(m_nPage);

		if (m_pImage == NULL)
		{
			m_szPage = CSize(0, 0);
		}
		else
		{
			m_pImage->set_rotate(m_nRotate);
			m_szPage = CSize(m_pImage->get_width(), m_pImage->get_height());
		}
	}
	G_CATCH(ex)
	{
		ex;
		m_szPage = CSize(0, 0);
	}
	G_ENDCATCH;

	delete m_pBitmap;
	m_pBitmap = NULL;

	UpdateView();
	ScrollToPosition(CPoint(0, 0));

	GetMainFrame()->UpdatePageCombo(m_nPage);

	Invalidate();
	UpdateWindow();
}

void CDjVuView::UpdateView()
{
	if (m_nLayout == SinglePage)
	{
		CSize szDisplayPage = m_szDisplayPage;

		CRect rcClient;
		GetClientRect(rcClient);

		// Update size 2 times to allow for scrollbars
		for (int i = 0; i < 2; ++i)
		{
			UpdatePageSize();

			CSize szPage(rcClient.Width()*3/4, rcClient.Height()*3/4);
			CSize szLine(15, 15);
			SetScrollSizes(MM_TEXT, m_szDisplay, szPage, szLine);
		}

		if (m_pBitmap == NULL || szDisplayPage != m_szDisplayPage)
		{
			m_bRendered = false;

			m_nImageCode = m_pRenderThread->AddJob(m_pImage,
				CRect(CPoint(0, 0), m_szDisplayPage),
				CRect(CPoint(0, 0), m_szDisplayPage), true);
		}
	}
	else if (m_nLayout == Continuous)
	{
	}
}

CSize CDjVuView::CalcPageSize(GP<DjVuImage> pImage)
{
	CSize szDisplay(0, 0);
	if (pImage == NULL)
		return szDisplay;

	CSize szPage(pImage->get_width(), pImage->get_height());
	if (szPage.cx <= 0 || szPage.cy <= 0)
		return szDisplay;

	CRect rcClient;
	GetClientRect(rcClient);

	switch (m_nZoomType)
	{
	case ZoomFitWidth:
		szDisplay = rcClient.Size();
		szDisplay.cy = szDisplay.cx * szPage.cy / szPage.cx;
		break;

	case ZoomFitHeight:
		szDisplay = rcClient.Size();
		szDisplay.cx = szDisplay.cy * szPage.cx / szPage.cy;
		break;

	case ZoomFitPage:
		szDisplay = rcClient.Size();
		szDisplay.cx = szDisplay.cy * szPage.cx / szPage.cy;

		if (szDisplay.cx > rcClient.Width())
		{
			szDisplay.cx = rcClient.Width();
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

		int nImgPixels = pImage->get_dpi();

		szDisplay.cx = static_cast<int>(szPage.cx*nLogPixelsX*m_fZoom*0.01/nImgPixels);
		szDisplay.cy = static_cast<int>(szPage.cy*nLogPixelsY*m_fZoom*0.01/nImgPixels);
	}

	return szDisplay;
}

void CDjVuView::UpdatePageSize()
{
	m_ptPageOffset = CPoint(0, 0);
	m_szDisplay = m_szDisplayPage = CalcPageSize(m_pImage);

	CRect rcClient;
	GetClientRect(rcClient);

	if (m_szDisplayPage.cx < rcClient.Width())
	{
		m_szDisplay.cx = rcClient.Width();
		m_ptPageOffset.x = (m_szDisplay.cx - m_szDisplayPage.cx)/2;
	}

	if (m_szDisplayPage.cy < rcClient.Height())
	{
		m_szDisplay.cy = rcClient.Height();
		m_ptPageOffset.y = (m_szDisplay.cy - m_szDisplayPage.cy)/2;
	}
}

void CDjVuView::OnViewNextpage()
{
	if (m_nPage < m_nPageCount - 1)
		RenderPage(m_nPage + 1);
}

void CDjVuView::OnUpdateViewNextpage(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_nPage < m_nPageCount - 1);
}

void CDjVuView::OnViewPreviouspage()
{
	if (m_nPage > 0)
		RenderPage(m_nPage - 1);
}

void CDjVuView::OnUpdateViewPreviouspage(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_nPage > 0);
}

void CDjVuView::OnSize(UINT nType, int cx, int cy)
{
	if (m_nPage != -1)
	{
		UpdateView();
	}

	CScrollView::OnSize(nType, cx, cy);
}

void CDjVuView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_DOWN:
		OnScrollBy(CSize(0, 3*m_lineDev.cy), TRUE);
		break;

	case VK_UP:
		OnScrollBy(CSize(0, -3*m_lineDev.cy), TRUE);
		break;

	case VK_RIGHT:
		OnScrollBy(CSize(3*m_lineDev.cx, 0), TRUE);
		break;

	case VK_LEFT:
		OnScrollBy(CSize(-3*m_lineDev.cx, 0), TRUE);
		break;

	case VK_NEXT:
	case VK_SPACE:
		if (!OnScrollBy(CSize(0, m_pageDev.cy), TRUE) && m_nPage < m_nPageCount - 1)
		{
			CPoint ptScroll = GetScrollPosition();
			OnViewNextpage();
			OnScrollBy(CPoint(ptScroll.x, 0) - GetScrollPosition(), TRUE);
			Invalidate();
		}
		break;

	case VK_PRIOR:
	case VK_BACK:
		if (!OnScrollBy(CSize(0, -m_pageDev.cy), TRUE) && m_nPage > 0)
		{
			CPoint ptScroll = GetScrollPosition();
			OnViewPreviouspage();
			OnScrollBy(CPoint(ptScroll.x, m_szDisplay.cy) - GetScrollPosition(), TRUE);
			Invalidate();
		}
		break;
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

	// Calc zoom from display area size
	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	int nLogPixels = dcScreen.GetDeviceCaps(LOGPIXELSX);
	int nImgPixels = m_pImage->get_dpi();

	return 100.0*m_szDisplayPage.cx*nImgPixels/(m_szPage.cx*nLogPixels);
}

void CDjVuView::OnViewLayout(UINT nID)
{
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
		Invalidate();
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
	if (m_pImage == NULL)
		return;

	m_nRotate = (m_nRotate + 1) % 4;
	RenderPage(m_nPage);
}

void CDjVuView::OnRotateRight()
{
	if (m_pImage == NULL)
		return;

	m_nRotate = (m_nRotate + 3) % 4;
	RenderPage(m_nPage);
}

void CDjVuView::OnRotate180()
{
	if (m_pImage == NULL)
		return;

	m_nRotate = (m_nRotate + 2) % 4;
	RenderPage(m_nPage);
}

void CDjVuView::OnViewFirstpage()
{
	RenderPage(0);
}

void CDjVuView::OnViewLastpage()
{
	RenderPage(m_nPageCount - 1);
}

void CDjVuView::OnSetFocus(CWnd* pOldWnd)
{
	CScrollView::OnSetFocus(pOldWnd);

	GetMainFrame()->UpdatePageCombo(m_nPage);
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
	Invalidate();

	GetMainFrame()->UpdateZoomCombo(m_nZoomType, m_fZoom);
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

		m_ptStart = point;
		m_ptStartPos = GetScrollPosition();

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
		OnScrollBy(m_ptStartPos + m_ptStart - point - GetScrollPosition(), TRUE);
		return;
	}

	CScrollView::OnMouseMove(nFlags, point);
}

void CDjVuView::OnFilePrint()
{
	int nRotate = (m_pImage != NULL ? m_pImage->get_rotate() : 0);

	CPrintDlg dlg(GetDocument(), m_nPage, nRotate);
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

void CDjVuView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (point.x < 0 || point.y < 0)
	{
		point = CPoint(0, 0);
		ClientToScreen(&point);
	}

	CMenu menu;
	menu.LoadMenu(IDR_POPUP);

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y,
		GetMainFrame());
}

void CDjVuView::OnPageInformation()
{
	if (m_pImage == NULL)
		return;

	CString strDescr = m_pImage->get_long_description();
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

	if (m_pBitmap != NULL)
		delete m_pBitmap;

	m_pBitmap = pBitmap;

	if (wParam == m_nImageCode)
		m_bRendered = true;

	Invalidate();
	UpdateWindow();
	return 0;
}

void CDjVuView::OnDestroy()
{
	delete m_pRenderThread;
	m_pRenderThread = NULL;

	CScrollView::OnDestroy();
}
