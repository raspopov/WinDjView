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

#pragma once

#include "Global.h"
#include "MyScrollView.h"
#include "AppSettings.h"
#include "Drawing.h"
#include "DjVuDoc.h"
#include "DjVuSource.h"

class CPrintDlg;
class CRenderThread;

inline bool IsStandardZoom(int nZoomType, double fZoom)
{
	return (nZoomType < 0 || fZoom == 50.0 || fZoom == 75.0 ||
			fZoom == 100.0 || fZoom == 200.0 || fZoom == 400.0);
}

CString MakePreviewString(const GUTF8String& text, int nStart, int nEnd);


#define WM_PAGE_DECODED (WM_APP + 1)
#define WM_PAGE_RENDERED (WM_APP + 2)

class CDjVuView : public CMyScrollView, public Observer, public Observable
{
protected: // create from serialization only
	CDjVuView();
	DECLARE_DYNCREATE(CDjVuView)

// Attributes
public:
	CDjVuDoc* GetDocument() const;
	void SetDocument(CDjVuDoc* pDocument) { m_pDocument = pDocument; }

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
	void ScrollToPage(int nPage, const CPoint& ptOffset);

	int GetPageCount() const { return m_nPageCount; }
	int GetCurrentPage() const { return m_nPage; }
	int GetZoomType() const { return m_nZoomType; }
	double GetZoom() const;
	void ZoomTo(int nZoomType, double fZoom = 100.0);
	int GetLayout() const { return m_nLayout; }
	int GetRotate() const { return m_nRotate; }

	GUTF8String GetFullText();
	void StopDecoding();
	void RestartThread();
	bool UpdatePageInfoFrom(CDjVuView* pFrom);
	void CopyBitmapsFrom(CDjVuView* pFrom, bool bMove = false);
	void CopyBitmapFrom(CDjVuView* pFrom, int nPage);

	CSize GetPageSize(int nPage) const { return m_pages[nPage].GetSize(m_nRotate); }
	int GetPageDPI(int nPage) const { return m_pages[nPage].info.nDPI; }

	void UpdateKeyboard(UINT nKey, bool bDown);
	void UpdateVisiblePages();

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
		Facing = 2,
		ContinuousFacing = 3
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
		MagnifyingGlass = 2,
		SelectRect = 3,
		NextPrev = 4
	};

	int GetMode() const { return m_nMode; }

	enum Type
	{
		Normal = 0,
		Fullscreen = 1,
		Magnify = 2
	};

	int GetType() const { return m_nType; }

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;

	virtual void OnStartPan();
	virtual void OnPan(CSize szScroll);
	virtual void OnEndPan();

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
	DjVuSource* m_pSource;

	CToolTipCtrl m_toolTip;
	CString m_strToolTip;
	CRenderThread* m_pRenderThread;
	CEvent m_evtRendered;
	bool m_bShiftDown, m_bControlDown;
	bool m_bNeedUpdate;
	UINT m_nTimerID;
	bool m_bInitialized;

	COffscreenDC m_offscreenDC;

	CCriticalSection m_dataLock;
	list<CDIB*> m_bitmaps;

	int m_nPage, m_nPageCount;
	int m_nPendingPage;
	CSize m_szDisplay;
	int CalcTopPage() const;
	int CalcCurrentPage() const;
	void RenderPage(int nPage, int nTimeout = -1, bool bUpdateWindow = true);

	bool InvalidatePage(int nPage);
	void DrawPage(CDC* pDC, int nPage);
	void DrawAnnotation(CDC* pDC, const Annotation& anno, int nPage, bool bActive);

	int m_nZoomType;
	double m_fZoom;
	int m_nLayout;
	int m_nDisplayMode;
	int m_nRotate;
	bool m_bFirstPageAlone;

	CDisplaySettings m_displaySettings;

	struct Page
	{
		Page() :
			szDisplay(0, 0), ptOffset(0, 0), pBitmap(NULL), nSelStart(-1), nSelEnd(-1),
			bHasSize(false), bBitmapRendered(false) {}
		~Page() { delete pBitmap; }

		CSize GetSize(int nRotate) const
		{
			CSize sz(info.szPage);
			if ((nRotate % 2) == 1)
				swap(sz.cx, sz.cy);
			return sz;
		}

		void Init(DjVuSource* pSource, int nPage, bool bNeedText = false, bool bNeedAnno = false)
		{
			info.Update(pSource->GetPageInfo(nPage, bNeedText, bNeedAnno));
		}

		PageInfo info;

		bool bHasSize;
		CPoint ptOffset;
		CSize szDisplay;
		CRect rcDisplay;
		CDIB* pBitmap;
		bool bBitmapRendered;

		DjVuSelection selection;
		int nSelStart, nSelEnd;

		void DeleteBitmap()
		{
			delete pBitmap;
			pBitmap = NULL;
			bBitmapRendered = false;
		}
	};
	vector<Page> m_pages;

	CSize UpdatePageRect(int nPage);
	CSize UpdatePageRectFacing(int nPage);
	void PreparePageRect(int nPage);
	void PreparePageRectFacing(int nPage);
	CSize CalcPageSize(const CSize& szPage, int nDPI) const;
	CSize CalcPageSize(const CSize& szPage, int nDPI, int nZoomType) const;
	CSize CalcZoomedPageSize(const CSize& szPage, CSize szBounds, int nZoomType) const;
	void CalcPageSizeFacing(const CSize& szPage1, int nDPI1, CSize& szDisplay1,
			const CSize& szPage2, int nDPI2, CSize& szDisplay2) const;
	void CalcPageSizeFacing(const CSize& szPage1, int nDPI1, CSize& szDisplay1,
			const CSize& szPage2, int nDPI2, CSize& szDisplay2, int nZoomType) const;
	void UpdatePageSizes(int nTop, int nScroll = 0);
	bool UpdatePagesFromTop(int nTop, int nBottom);
	void UpdatePagesFromBottom(int nTop, int nBottom);
	void UpdatePageSize(int nPage);
	void UpdatePageSizeFacing(int nPage);
	void DeleteBitmaps();
	int GetPageFromPoint(CPoint point) const;
	int GetPageNearPoint(CPoint point) const;
	void ReadZoomSettings(GP<DjVuANT> pAnt);
	void ReadDisplayMode(GP<DjVuANT> pAnt);
	bool IsValidPage(int nPage) const;
	bool HasFacingPage(int nPage) const;
	int FixPageNumber(int nPage) const;
	int GetNextPage(int nPage) const;
	void SetLayout(int nLayout, int nPage, int nOffset);
	void UpdatePageNumber();
	void PageRendered(int nPage, CDIB* pDIB);
	void PageDecoded(int nPage);
	void SettingsChanged();
	void UpdateCursor();

	virtual void OnUpdate(const Observable* source, const Message* message);

	enum UpdateType
	{
		TOP = 0,
		BOTTOM = 1,
		RECALC = 2
	};
	void UpdateLayout(UpdateType updateType = TOP);
	void UpdatePagesCacheSingle(bool bUpdateImages);
	void UpdatePagesCacheFacing(bool bUpdateImages);
	void UpdatePagesCacheContinuous(bool bUpdateImages);
	void UpdatePageCache(int nPage, const CRect& rcClient, bool bUpdateImages);
	void UpdatePageCacheSingle(int nPage, bool bUpdateImages);
	void UpdatePageCacheFacing(int nPage, bool bUpdateImages);
	bool IsViewNextpageEnabled();
	bool IsViewPreviouspageEnabled() const;
	void ClearSelection(int nPage = -1);
	void UpdateSelectionStatus();
	bool IsSelectionBelowTop(int nPage, const DjVuSelection& selection) const;
	bool IsSelectionVisible(int nPage, const DjVuSelection& selection) const;
	void EnsureSelectionVisible(int nPage, const DjVuSelection& selection);
	CRect GetSelectionRect(int nPage, const DjVuSelection& selection) const;
	CRect TranslatePageRect(int nPage, GRect rect, bool bToDisplay = true) const;
	bool m_bInsideUpdateLayout;

	Annotation* m_pHoverAnno;
	Annotation* m_pClickedAnno;
	int m_nHoverPage;
	bool m_bIgnoreMouseLeave;
	bool m_bHoverIsCustom;
	Annotation* GetAnnotationFromPoint(const CPoint& point, int& nPage, bool& bCustom);
	bool PtInAnnotation(const Annotation& anno, int nPage, const CPoint& point) const;
	void UpdateHoverAnnotation(const CPoint& point);
	void UpdateHoverAnnotation();
	bool InvalidateAnno(Annotation* pAnno, int nPage);

	int m_nMode, m_nType;
	int m_nSelStartPos;
	CPoint ScreenToDjVu(int nPage, const CPoint& point) const;
	void UpdateDragAction();
	int GetTextPosFromPoint(int nPage, const CPoint& point);
	void GetTextPosFromTop(DjVuTXT::Zone& zone,  const CPoint& pt, int& nPos) const;
	void GetTextPosFromBottom(DjVuTXT::Zone& zone,  const CPoint& pt, int& nPos) const;
	void FindSelectionZones(DjVuSelection& list, DjVuTXT* pText, int nStart, int nEnd) const;
	void SelectTextRange(int nPage, int nStart, int nEnd, bool& bInfoLoaded, CWaitCursor*& pWaitCursor);
	GUTF8String GetSelectedText() const;
	bool m_bHasSelection;
	int m_nSelectionPage;
	GRect m_rcSelectionRect;

	int m_nClickedPage;
	bool m_bDragging, m_bDraggingPage, m_bDraggingText, m_bDraggingRect, m_bDraggingLink;
	bool m_bPanning;
	void StopDragging();
	CPoint m_ptStart, m_ptStartPos, m_ptPrevCursor;
	int m_nStartPage, m_nPrevPage;
	bool m_bClick;
	CPoint m_ptClick, m_ptMouse;
	int m_nClickCount;
	DWORD m_nCursorTime;
	bool m_bCursorHidden;
	void ShowCursor();
	static HCURSOR hCursorHand;
	static HCURSOR hCursorDrag;
	static HCURSOR hCursorLink;
	static HCURSOR hCursorText;
	static HCURSOR hCursorMagnify;
	static HCURSOR hCursorCross;
	CImageList m_hourglass;

	// Dummy invisible scrollbars
	CScrollBar* m_pHScrollBar;
	CScrollBar* m_pVScrollBar;
	void CreateScrollbars();

	bool m_bDraggingMagnify;
	void StartMagnify();
	void UpdateMagnifyWnd();

	int m_nMargin, m_nShadowMargin;
	int m_nPageGap, m_nFacingGap;
	int m_nPageBorder, m_nPageShadow;

protected:
	// Generated message map functions
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
	afx_msg void OnCancelMode();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnFilePrint();
	afx_msg void OnViewLayout(UINT nID);
	afx_msg void OnUpdateViewLayout(CCmdUI* pCmdUI);
	afx_msg LRESULT OnPageDecoded(WPARAM wParam, LPARAM lParam = 0);
	afx_msg LRESULT OnPageRendered(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnExportPage(UINT nID);
	afx_msg void OnFindString();
	afx_msg void OnFindAll();
	afx_msg BOOL OnToolTipNeedText(UINT nID, NMHDR* pNMHDR, LRESULT* pResult);
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
	afx_msg void OnFirstPageAlone();
	afx_msg void OnUpdateFirstPageAlone(CCmdUI* pCmdUI);
	afx_msg void OnMouseLeave();
	afx_msg void OnHighlightSelection();
	afx_msg void OnUpdateHighlightSelection(CCmdUI* pCmdUI);
	afx_msg void OnHighlightDelete();
	afx_msg void OnHighlightEdit();
	DECLARE_MESSAGE_MAP()
};


#ifndef _DEBUG  // debug version in DjVuView.cpp
inline CDjVuDoc* CDjVuView::GetDocument() const
   { return reinterpret_cast<CDjVuDoc*>(m_pDocument); }
#endif

