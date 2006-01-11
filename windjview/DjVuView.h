//	WinDjView
//	Copyright (C) 2004-2006 Andrew Zhezherun
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

#pragma once

#include "MyScrollView.h"
#include "AppSettings.h"
#include "Drawing.h"
#include "DjVuDoc.h"

class CPrintDlg;
class CRenderThread;

inline bool IsStandardZoom(int nZoomType, double fZoom)
{
	return (nZoomType < 0 || fZoom == 50.0 || fZoom == 75.0 ||
			fZoom == 100.0 || fZoom == 200.0 || fZoom == 400.0);
}

typedef GList<DjVuTXT::Zone*> DjVuSelection;

CString MakeCString(const GUTF8String& text);
CString MakePreviewString(const GUTF8String& text, int nStart, int nEnd);


class CDjVuView : public CMyScrollView
{
protected: // create from serialization only
	CDjVuView();
	DECLARE_DYNCREATE(CDjVuView)

// Attributes
public:
	CDjVuDoc* GetDocument() const;
	void SetDocument(CDjVuDoc* pDoc) { m_pDocument = pDoc; }

// Operations
public:
	enum AddToHistory
	{
		DoNotAdd = 0,
		AddSource = 1,
		AddTarget = 2
	};
	void GoToPage(int nPage, int nLinkPage = -1, int nAddToHistory = AddSource | AddTarget);
	void GoToURL(const GUTF8String& url, int nLinkPage = -1, int nAddToHistory = AddSource | AddTarget);
	void GoToSelection(int nPage, int nStartPos, int nEndPos,
		int nLinkPage = -1, int nAddToHistory = AddSource | AddTarget);

	int GetPageCount() const { return m_nPageCount; }
	int GetCurrentPage() const;
	int GetZoomType() const { return m_nZoomType; }
	double GetZoom() const;
	void ZoomTo(int nZoomType, double fZoom = 100.0);
	int GetLayout() const { return m_nLayout; }
	int GetRotate() const { return m_nRotate; }

	GUTF8String GetFullText();
	void StopDecoding();
	void RestartThread();
	void UpdatePageInfo(CDjVuView* pView);

	CSize GetPageSize(int nPage) const { return m_pages[nPage].GetSize(m_nRotate); }
	int GetPageDPI(int nPage) const { return m_pages[nPage].info.nDPI; }

	void OnSettingsChanged();
	void ShowAllLinks(bool bShowAll);

	enum ZoomType
	{
		ZoomPercent = 0,
		ZoomFitWidth = -1,
		ZoomFitHeight = -2,
		ZoomFitPage = -3,
		ZoomActualSize = -4,
		ZoomStretch = -5
	};

	enum LayoutType
	{
		SinglePage = 0,
		Continuous = 1,
		Facing = 2
	};

	double GetZoom(ZoomType nZoomType) const;

	enum DisplayMode
	{
		Color = 0,
		BlackAndWhite = 1,
		Background = 2,
		Foreground = 3
	};

	int GetDisplayMode() const { return m_nDisplayMode; }

	enum Mode
	{
		Drag = 0,
		Select = 1,
		Fullscreen = 2
	};

	int GetMode() const { return m_nMode; }

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CDjVuView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CToolTipCtrl m_toolTip;
	CRenderThread* m_pRenderThread;
	CEvent m_evtRendered;
	bool m_bShowAllLinks;
	bool m_bNeedUpdate;
	UINT m_nTimerID;

	CBitmap* m_pOffscreenBitmap;
	CSize m_szOffscreen;

	int m_nPage, m_nPageCount;
	CSize m_szDisplay;
	void CalcTopPage();
	void RenderPage(int nPage, int nTimeout = -1);

	bool InvalidatePage(int nPage);
	void DrawPage(CDC* pDC, int nPage);
	void DrawMapArea(CDC* pDC, GP<GMapArea> pArea, int nPage, bool bActive);

	int m_nZoomType;
	double m_fZoom;
	int m_nLayout;
	int m_nDisplayMode;
	int m_nRotate;

	CDisplaySettings m_displaySettings;

	struct Page
	{
		Page() :
			bInfoLoaded(false), szDisplay(0, 0), ptOffset(0, 0),
			pBitmap(NULL), bTextDecoded(false), bAnnoDecoded(false),
			nSelStart(-1), nSelEnd(-1) {}
		~Page() { delete pBitmap; }

		CSize GetSize(int nRotate) const
		{
			CSize sz(info.szPage);
			if ((nRotate % 2) == 1)
				swap(sz.cx, sz.cy);
			return sz;
		}

		void Init(const PageInfo& info)
		{
			this->info = info;
			bInfoLoaded = true;
			DecodeAnno();
		}

		bool bInfoLoaded;
		PageInfo info;

		CPoint ptOffset;
		CSize szDisplay;
		CRect rcDisplay;
		CDIB* pBitmap;

		bool bTextDecoded;
		GP<DjVuTXT> pText;
		DjVuSelection selection;
		int nSelStart, nSelEnd;

		bool bAnnoDecoded;
		GP<DjVuANT> pAnt;

		void DecodeText()
		{
			if (info.pTextStream != NULL && !bTextDecoded)
			{
				info.pTextStream->seek(0);
				GP<DjVuText> pDjVuText = DjVuText::create();
				pDjVuText->decode(info.pTextStream);
				pText = pDjVuText->txt;
				bTextDecoded = true;
			}
		}

		void DecodeAnno()
		{
			if (info.pAnnoStream != NULL && !bAnnoDecoded)
			{
				info.pAnnoStream->seek(0);
				GP<DjVuAnno> pDjVuAnno = DjVuAnno::create();
				pDjVuAnno->decode(info.pAnnoStream);
				pAnt = pDjVuAnno->ant;
				bAnnoDecoded = true;
			}
		}

		void DeleteBitmap()
		{
			delete pBitmap;
			pBitmap = NULL;
		}
	};
	vector<Page> m_pages;

	void UpdatePageSize(int nPage);
	CSize UpdatePageSizeFacing(int nPage);
	CSize CalcPageSize(const CSize& szPage, int nDPI);
	CSize CalcPageSize(const CSize& szPage, int nDPI, int nZoomType) const;
	CSize CalcZoomedPageSize(const CSize& szPage, const CSize& szFrame, int nZoomType) const;
	void CalcPageSizeFacing(const CSize& szPage1, int nDPI1, CSize& szDisplay1,
			const CSize& szPage2, int nDPI2, CSize& szDisplay2);
	void CalcPageSizeFacing(const CSize& szPage1, int nDPI1, CSize& szDisplay1,
			const CSize& szPage2, int nDPI2, CSize& szDisplay2, int nZoomType) const;
	void UpdatePageSizes(int nTop, int nScroll = 0);
	bool UpdatePagesFromTop(int nTop, int nBottom);
	void UpdatePagesFromBottom(int nTop, int nBottom);
	void DeleteBitmaps();
	int GetPageFromPoint(CPoint point);
	void ReadZoomSettings(GP<DjVuANT> pAnt);
	void ReadDisplayMode(GP<DjVuANT> pAnt);

	enum UpdateType
	{
		TOP = 0,
		BOTTOM = 1,
		RECALC = 2
	};
	void UpdateView(UpdateType updateType = TOP);
	void UpdateVisiblePages();
	void UpdatePagesCacheSingle();
	void UpdatePagesCacheFacing();
	void UpdatePagesCacheContinuous();
	void UpdatePageCache(int nPage, const CRect& rcClient);
	void UpdatePageCacheSingle(int nPage);
	void UpdatePageCacheFacing(int nPage);
	bool IsViewNextpageEnabled();
	bool IsViewPreviouspageEnabled();
	void ClearSelection(int nPage = -1);
	void UpdateSelectionStatus();
	bool IsSelectionBelowTop(int nPage, const DjVuSelection& selection);
	bool IsSelectionVisible(int nPage, const DjVuSelection& selection);
	void EnsureSelectionVisible(int nPage, const DjVuSelection& selection);
	CRect GetSelectionRect(int nPage, const DjVuSelection& selection) const;
	CRect TranslatePageRect(int nPage, GRect rect) const;
	bool m_bInsideUpdateView;

	GP<GMapArea> m_pActiveLink;
	int m_nLinkPage;
	GP<GMapArea> GetHyperlinkFromPoint(CPoint point, int* pnPage = NULL);
	void UpdateActiveHyperlink(CPoint point);

	int m_nMode;
	int m_nSelStartPos;
	CPoint TranslateToDjVuCoord(int nPage, const CPoint& point);
	int GetTextPosFromPoint(int nPage, const CPoint& point);
	void GetTextPosFromTop(DjVuTXT::Zone& zone,  const CPoint& pt, int& nPos);
	void GetTextPosFromBottom(DjVuTXT::Zone& zone,  const CPoint& pt, int& nPos);
	void FindSelectionZones(DjVuSelection& list, DjVuTXT* pText, int nStart, int nEnd);
	void SelectTextRange(int nPage, int nStart, int nEnd, bool& bInfoLoaded, CWaitCursor*& pWaitCursor);
	GUTF8String GetSelectedText();
	bool m_bHasSelection;

	int m_nClickedPage;
	bool m_bDragging;
	CPoint m_ptStart, m_ptStartPos;
	int m_nStartPage, m_nPrevPage;
	bool m_bClick;
	CPoint m_ptClick;
	int m_nClickCount;
	static HCURSOR hCursorHand;
	static HCURSOR hCursorDrag;
	static HCURSOR hCursorLink;
	static HCURSOR hCursorText;

public:
	int m_nPendingPage;
	afx_msg LRESULT OnRenderFinished(WPARAM wParam, LPARAM lParam);

	// Generated message map functions
protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPageInformation();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnViewZoom(UINT nID);
	afx_msg void OnUpdateViewZoom(CCmdUI *pCmdUI);
	afx_msg void OnViewNextpage();
	afx_msg void OnUpdateViewNextpage(CCmdUI *pCmdUI);
	afx_msg void OnViewPreviouspage();
	afx_msg void OnUpdateViewPreviouspage(CCmdUI *pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnRotateLeft();
	afx_msg void OnRotateRight();
	afx_msg void OnRotate180();
	afx_msg void OnViewFirstpage();
	afx_msg void OnViewLastpage();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnFilePrint();
	afx_msg void OnViewLayout(UINT nID);
	afx_msg void OnUpdateViewLayout(CCmdUI* pCmdUI);
	afx_msg LRESULT OnPageDecoded(WPARAM wParam, LPARAM lParam = 0);
	afx_msg void OnDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnExportPage();
	afx_msg void OnFindString();
	afx_msg void OnFindAll();
	afx_msg BOOL OnToolTipNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnViewZoomIn();
	afx_msg void OnViewZoomOut();
	afx_msg void OnUpdateViewZoomIn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewZoomOut(CCmdUI* pCmdUI);
	afx_msg void OnChangeMode(UINT nID);
	afx_msg void OnUpdateMode(CCmdUI* pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnFileExportText();
	afx_msg void OnUpdateFileExportText(CCmdUI* pCmdUI);
	afx_msg void OnViewDisplay(UINT nID);
	afx_msg void OnUpdateViewDisplay(CCmdUI* pCmdUI);
	afx_msg void OnViewFullscreen();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnViewGotoPage();
	afx_msg void OnTimer(UINT nIDEvent);
	DECLARE_MESSAGE_MAP()
};


#ifndef _DEBUG  // debug version in DjVuView.cpp
inline CDjVuDoc* CDjVuView::GetDocument() const
   { return reinterpret_cast<CDjVuDoc*>(m_pDocument); }
#endif

