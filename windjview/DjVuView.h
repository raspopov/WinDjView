//	WinDjView
//	Copyright (C) 2004-2008 Andrew Zhezherun
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

// $Id$

#pragma once

#include "Global.h"
#include "MyScrollView.h"
#include "AppSettings.h"
#include "Drawing.h"
#include "DjVuDoc.h"
#include "DjVuSource.h"

class CMainFrame;
class CMDIChild;
class CPrintDlg;
class CRenderThread;

inline bool IsStandardZoom(int nZoomType, double fZoom)
{
	return (nZoomType < 0 || fZoom == 50.0 || fZoom == 75.0 ||
			fZoom == 100.0 || fZoom == 200.0 || fZoom == 400.0);
}

CString MakePreviewString(const GUTF8String& text, int nStart, int nEnd);


class CDjVuView : public CMyScrollView, public Observer, public Observable
{
protected: // create from serialization only
	CDjVuView();
	DECLARE_DYNCREATE(CDjVuView)

// Attributes
public:
	CDjVuDoc* GetDocument() const;
	void SetDocument(CDjVuDoc* pDocument) { m_pDocument = pDocument; }
	CMainFrame* GetMainFrame() const;
	CMDIChild* GetMDIChild() const;

// Operations
public:
	enum AddToHistory
	{
		DoNotAdd = 0,
		AddSource = 1,
		AddTarget = 2
	};
	void GoToPage(int nPage, int nAddToHistory = AddSource | AddTarget);
	void GoToBookmark(const Bookmark& bookmark, int nAddToHistory = AddSource | AddTarget);
	void GoToURL(const GUTF8String& url, int nAddToHistory = AddSource | AddTarget);
	void GoToSelection(int nPage, int nStartPos, int nEndPos, int nAddToHistory = AddSource | AddTarget);
	void ScrollToPage(int nPage, const CPoint& ptOffset, bool bMargin = false);

	int GetPageCount() const { return m_nPageCount; }
	int GetCurrentPage() const { return m_nPage; }
	int GetZoomType() const { return m_nZoomType; }
	double GetZoom() const;
	void ZoomTo(int nZoomType, double fZoom = 100.0);
	int GetLayout() const { return m_nLayout; }
	int GetRotate() const { return m_nRotate; }

	bool UpdatePageInfoFrom(CDjVuView* pFrom);
	void CopyBitmapsFrom(CDjVuView* pFrom, bool bMove = false);
	void CopyBitmapFrom(CDjVuView* pFrom, int nPage);

	CSize GetPageSize(int nPage) const { return m_pages[nPage].GetSize(m_nRotate); }
	int GetPageDPI(int nPage) const { return m_pages[nPage].info.nDPI; }
	void GetNormalizedText(wstring& text, bool bSelected = false, int nMaxLength = -1);

	void UpdateKeyboard(UINT nKey, bool bDown);
	void UpdateVisiblePages();

	bool CreateBookmarkFromSelection(Bookmark& bookmark);
	void CreateBookmarkFromAnnotation(Bookmark& bookmark, const Annotation* pAnno, int nPage);
	void CreateBookmarkFromView(Bookmark& bookmark);
	void CreateBookmarkFromPage(Bookmark& bookmark, int nPage);

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
		ZoomRect = 4,
		NextPrev = 5
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
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;

	virtual void ScrollToPosition(CPoint pt, bool bRepaint = true);
	virtual bool OnScroll(UINT nScrollCode, UINT nPos, bool bDoScroll = true);
	virtual bool OnScrollBy(CSize szScrollBy, bool bDoScroll = true);

	virtual bool OnStartPan();
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
	CEvent m_pageRendered;
	bool m_bShiftDown, m_bControlDown;
	bool m_bNeedUpdate;
	bool m_bUpdateBitmaps;
	UINT m_nTimerID;

	COffscreenDC m_offscreenDC;

	CCriticalSection m_dataLock;
	set<CDIB*> m_bitmaps;

	int m_nPage, m_nPageCount;
	int m_nPendingPage;
	CSize m_szDisplay;
	int CalcTopPage() const;
	int CalcCurrentPage() const;
	int CalcBottomPage(int nTopPage) const;
	void RenderPage(int nPage, int nTimeout = -1, bool bUpdateWindow = true);

	bool InvalidatePage(int nPage);
	void DrawPage(CDC* pDC, int nPage);
	void DrawAnnotation(CDC* pDC, const Annotation& anno, int nPage, bool bActive);
	void DrawTransparentText(CDC* pDC, int nPage);

	int m_nZoomType;
	double m_fZoom;
	int m_nLayout;
	int m_nDisplayMode;
	int m_nRotate;
	bool m_bFirstPageAlone;
	bool m_bRightToLeft;

	CDisplaySettings m_displaySettings;

	struct Page
	{
		Page() :
			szBitmap(0, 0), ptOffset(0, 0), pBitmap(NULL), nSelStart(-1), nSelEnd(-1),
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
		CSize szBitmap;
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

	void PreparePageRect(const CSize& szBounds, int nPage);
	void PreparePageRectFacing(const CSize& szBounds, int nPage);
	CSize CalcPageSize(const CSize& szBounds, const CSize& szPage, int nDPI) const;
	CSize CalcPageSize(const CSize& szBounds, const CSize& szPage, int nDPI, int nZoomType) const;
	CSize CalcPageBitmapSize(const CSize& szBounds, const CSize& szPage, int nZoomType) const;
	void CalcPageSizeFacing(const CSize& szBounds,
			const CSize& szPage1, int nDPI1, CSize& szDisplay1,
			const CSize& szPage2, int nDPI2, CSize& szDisplay2) const;
	void CalcPageSizeFacing(const CSize& szBounds,
			const CSize& szPage1, int nDPI1, CSize& szDisplay1,
			const CSize& szPage2, int nDPI2, CSize& szDisplay2, int nZoomType) const;
	void UpdatePageSizes(int nTop, int nScroll = 0);
	bool UpdatePagesFromTop(int nTop, int nBottom);
	void UpdatePagesFromBottom(int nTop, int nBottom);
	void UpdatePageSize(const CSize& szBounds, int nPage);
	void UpdatePageSizeFacing(const CSize& szBounds, int nPage);
	void DeleteBitmaps();
	int GetPageFromPoint(CPoint point) const;
	int GetPageNearPoint(CPoint point) const;
	void ReadZoomSettings(GP<DjVuANT> pAnt);
	void ReadDisplayMode(GP<DjVuANT> pAnt);
	bool IsValidPage(int nPage) const;
	bool HasFacingPage(int nPage) const;
	void GetFacingPages(int nPage, Page*& pLeftOrSingle, Page*& pOther,
			int* pnLeftOrSingle = NULL, int* pnOther = NULL);
	int FixPageNumber(int nPage) const;
	int GetNextPage(int nPage) const;
	void SetLayout(int nLayout, int nPage, int nOffset);
	void UpdatePageNumber();
	void PageRendered(int nPage, CDIB* pDIB);
	void PageDecoded(int nPage);
	void SettingsChanged();
	void UpdateCursor();
	bool HasScrollBars() const;

	virtual void OnUpdate(const Observable* source, const Message* message);

	enum UpdateType
	{
		TOP = 0,
		BOTTOM = 1,
		RECALC = 2
	};
	void UpdateLayout(UpdateType updateType = TOP);
	CSize UpdateLayoutSinglePage(const CSize& szTrueClient);
	CSize UpdateLayoutFacing(const CSize& szTrueClient);
	CSize UpdateLayoutContinuous(const CSize& szTrueClient);
	CSize UpdateLayoutContinuousFacing(const CSize& szTrueClient);
	void UpdatePagesCacheSingle(bool bUpdateImages);
	void UpdatePagesCacheFacing(bool bUpdateImages);
	void UpdatePagesCacheContinuous(bool bUpdateImages);
	void UpdatePageCache(const CSize& szClient, int nPage, bool bUpdateImages);
	void UpdatePageCacheSingle(int nPage, bool bUpdateImages);
	void UpdatePageCacheFacing(int nPage, bool bUpdateImages);
	bool IsViewNextpageEnabled();
	bool IsViewPreviouspageEnabled() const;
	void ClearSelection(int nPage = -1);
	void UpdateSelectionStatus();
	bool IsSelectionVisible(int nPage, const DjVuSelection& selection);
	void EnsureSelectionVisible(int nPage, const DjVuSelection& selection);
	CRect GetSelectionRect(int nPage, const DjVuSelection& selection);
	CRect TranslatePageRect(int nPage, GRect rect, bool bToDisplay = true, bool bClip = true);
	bool m_bInsideUpdateLayout, m_bInsideMouseMove;

	Annotation* m_pHoverAnno;
	Annotation* m_pClickedAnno;
	bool m_bHoverIsCustom, m_bClickedCustom;
	int m_nHoverPage;
	bool m_bIgnoreMouseLeave;
	Annotation* GetAnnotationFromPoint(const CPoint& point, int& nPage, bool& bCustom);
	bool PtInAnnotation(const Annotation& anno, int nPage, const CPoint& point);
	void UpdateHoverAnnotation(const CPoint& point);
	void UpdateHoverAnnotation();
	bool InvalidateAnno(Annotation* pAnno, int nPage);

	int m_nMode, m_nType;
	int m_nSelStartPos;
	CPoint ScreenToDjVu(int nPage, const CPoint& point, bool bClip = true);
	void UpdateDragAction();
	int GetTextPosFromPoint(int nPage, const CPoint& point);
	void GetTextPos(const DjVuTXT::Zone& zone, const CPoint& pt, int& nPos, double& fBest) const;
	void FindSelectionZones(DjVuSelection& list, DjVuTXT* pText, int nStart, int nEnd) const;
	void SelectTextRange(int nPage, int nStart, int nEnd, bool& bInfoLoaded, CWaitCursor*& pWaitCursor);
	bool m_bHasSelection;
	int m_nSelectionPage;
	GRect m_rcSelection;

	CFont m_sampleFont;
	map<int, HFONT> m_fonts;

	int m_nClickedPage;
	bool m_bDragging, m_bDraggingRight;
	bool m_bDraggingPage, m_bDraggingText, m_bDraggingRect, m_bDraggingLink;
	bool m_bPanning;
	bool m_bPopupMenu;
	void StopDragging();
	void OnContextMenu();
	CPoint m_ptStart, m_ptStartPos, m_ptStartSel, m_ptPrevCursor;
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
	static HCURSOR hCursorZoomRect;
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
	afx_msg void OnViewRotate(UINT nID);
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
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnFilePrint();
	afx_msg void OnViewLayout(UINT nID);
	afx_msg void OnUpdateViewLayout(CCmdUI* pCmdUI);
	afx_msg LRESULT OnPageDecoded(WPARAM wParam, LPARAM lParam = 0);
	afx_msg LRESULT OnPageRendered(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnExportPage(UINT nID);
	afx_msg void OnUpdateExportSelection(CCmdUI* pCmdUI);
	afx_msg void OnFindString();
	afx_msg void OnFindAll();
	afx_msg void OnViewZoomIn();
	afx_msg void OnViewZoomOut();
	afx_msg void OnUpdateViewZoomIn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewZoomOut(CCmdUI* pCmdUI);
	afx_msg void OnChangeMode(UINT nID);
	afx_msg void OnUpdateMode(CCmdUI* pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnViewDisplay(UINT nID);
	afx_msg void OnUpdateViewDisplay(CCmdUI* pCmdUI);
	afx_msg void OnViewFullscreen();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnViewGotoPage();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnFirstPageAlone();
	afx_msg void OnRightToLeftOrder();
	afx_msg void OnUpdateFirstPageAlone(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRightToLeftOrder(CCmdUI* pCmdUI);
	afx_msg void OnMouseLeave();
	afx_msg void OnHighlight(UINT nID);
	afx_msg void OnUpdateHighlight(CCmdUI* pCmdUI);
	afx_msg void OnEditAnnotation();
	afx_msg void OnDeleteAnnotation();
	afx_msg void OnAddBookmark();
	afx_msg void OnZoomToSelection();
	afx_msg void OnSwitchFocus(UINT nID);
	afx_msg void OnUpdateSwitchFocus(CCmdUI* pCmdUI);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg LRESULT OnShowParent(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};


#ifndef _DEBUG  // debug version in DjVuView.cpp
inline CDjVuDoc* CDjVuView::GetDocument() const
   { return reinterpret_cast<CDjVuDoc*>(m_pDocument); }
#endif

