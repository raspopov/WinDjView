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

#include "ChildFrm.h"
#include "MainFrm.h"
#include "DjVuView.h"
#include "NavPane.h"
#include "AppSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	ON_WM_MDIACTIVATE()
	ON_WM_WINDOWPOSCHANGED()
	ON_MESSAGE_VOID(ID_STOP_TRACKING, OnStopTracking)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	m_nNavPaneWidth = CAppSettings::nNavPaneWidth;

	m_bCreated = false;
}

CChildFrame::~CChildFrame()
{
	CAppSettings::nNavPaneWidth = m_nNavPaneWidth;
}


BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIChildWnd::PreCreateWindow(cs))
		return FALSE;

	return TRUE;
}


// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG


// CChildFrame message handlers

void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	CComboBox& cboPage = GetMainFrame()->m_cboPage;
	CComboBox& cboZoom = GetMainFrame()->m_cboZoom;

	if (bActivate)
	{
		CDjVuView* pView = GetDjVuView();

		cboPage.EnableWindow(true);
		int nPages = pView->GetPageCount();
		int nPage = pView->GetCurrentPage();
		GetMainFrame()->UpdatePageCombo(nPage, nPages);

		cboZoom.EnableWindow(true);
		int nZoomType = pView->GetZoomType();
		double fZoom = pView->GetZoom();
		GetMainFrame()->UpdateZoomCombo(nZoomType, fZoom);
	}
	else if (pActivateWnd == NULL)
	{
		cboPage.ResetContent();
		cboPage.EnableWindow(false);

		cboZoom.SetWindowText(_T("100%"));
		cboZoom.EnableWindow(false);
	}
}

void CChildFrame::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CMDIChildWnd::OnWindowPosChanged(lpwndpos);

	if (IsWindowVisible() && !IsIconic())
		CAppSettings::bChildMaximized = !!IsZoomed();

	if (m_bCreated)
		UpdateNavPane();
}

void CChildFrame::ActivateFrame(int nCmdShow)
{
	if (CAppSettings::bChildMaximized)
		nCmdShow = SW_SHOWMAXIMIZED;

	CMDIChildWnd::ActivateFrame(nCmdShow);
}

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	m_wndSplitter.CreateStatic(this, 1, 2);

	m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CNavPaneWnd),
		CSize(m_nNavPaneWidth, 0), pContext);
	m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CDjVuView),
		CSize(0, 0), pContext);

	SetActiveView(GetDjVuView());

	m_bCreated = true;
	UpdateNavPane();

	return TRUE;
}

void CChildFrame::UpdateNavPane()
{
	m_wndSplitter.AllowTracking(CAppSettings::bShowNavPane);
	m_wndSplitter.HideSplitter(!CAppSettings::bShowNavPane);

	UpdateNavPaneWidth();
}

void CChildFrame::UpdateNavPaneWidth()
{
	int nWidth = (CAppSettings::bShowNavPane ? m_nNavPaneWidth : 0);

	m_wndSplitter.SetColumnInfo(0, nWidth, 0);
	m_wndSplitter.RecalcLayout();
}

void CChildFrame::OnStopTracking()
{
	if (CAppSettings::bShowNavPane)
	{
		int cxMin;
		m_wndSplitter.GetColumnInfo(0, m_nNavPaneWidth, cxMin);
	}
}

CDjVuView* CChildFrame::GetDjVuView()
{
	return static_cast<CDjVuView*>(m_wndSplitter.GetPane(0, 1));
}

CNavPaneWnd* CChildFrame::GetNavPane()
{
	return static_cast<CNavPaneWnd*>(m_wndSplitter.GetPane(0, 0));
}

void CChildFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// update our parent window first
	GetMDIFrame()->OnUpdateFrameTitle(bAddToTitle);

	if ((GetStyle() & FWS_ADDTOTITLE) == 0)
		return;     // leave child window alone!

	CDocument* pDocument = GetActiveDocument();
	if (bAddToTitle)
	{
		TCHAR szText[256 + _MAX_PATH];
		if (pDocument == NULL)
			_tcsncpy(szText, m_strTitle, _countof(szText));
		else
			_tcsncpy(szText, pDocument->GetTitle(), _countof(szText));

#ifdef ELIBRA_READER
		// Strip extension from the document name
		TCHAR* pPos = _tcsrchr(szText, '\\');
		TCHAR* pPos2 = _tcsrchr(szText, '/');

		TCHAR* pDotPos = _tcschr(szText, '.');
		if (pDotPos != NULL && pDotPos > pPos && pDotPos > pPos2)
			*pDotPos = '\0';
#endif

		// set title if changed, but don't remove completely
		AfxSetWindowText(m_hWnd, szText);
	}
}

CDocument* CChildFrame::GetActiveDocument()
{
	return GetDjVuView()->GetDocument();
}

BOOL CChildFrame::OnEraseBkgnd(CDC* pDC)
{
	return true;
}
