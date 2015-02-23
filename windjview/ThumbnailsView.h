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

#pragma once

#include "Global.h"
#include "MyScrollView.h"
#include "Drawing.h"
#include "AppSettings.h"
class DjVuSource;
class CThumbnailsThread;


// CThumbnailsView

class CThumbnailsView : public CMyScrollView, public Observable, public Observer
{
	DECLARE_DYNAMIC(CThumbnailsView)

public:
	CThumbnailsView(DjVuSource* pSource);
	virtual ~CThumbnailsView();

// Attributes
public:
	DjVuSource* GetSource() const { return m_pSource; }
	int GetCurrentPage() const { return m_nCurrentPage; }

	void SetCurrentPage(int nPage);
	void SetActivePage(int nPage, bool bSetSelection = true);
	void EnsureVisible(int nPage);
	void SelectPage(int nPage, bool bSelect = true);
	void ClearSelection();
	bool HasSelection() const;

	virtual void OnUpdate(const Observable* source, const Message* message);

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();

	virtual bool OnScrollBy(CSize szScrollBy, bool bDoScroll = true);
	virtual bool OnScroll(UINT nScrollCode, UINT nPos, bool bDoScroll = true);

// Implementation
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CThumbnailsThread* m_pThread;
	CThumbnailsThread* m_pIdleThread;
	int m_nPageCount;
	DjVuSource* m_pSource;
	bool m_bInsideUpdateLayout;
	CFont m_font;
	int m_nActivePage, m_nCurrentPage;
	bool m_bVisible, m_bInitialized;
	int m_nRotate;
	int m_nPagesInRow;
	int m_nThumbnailSize;
	CSize m_szThumbnail;

	CCriticalSection m_dataLock;
	set<CDIB*> m_bitmaps;

	CDisplaySettings m_displaySettings;
	void SettingsChanged();

	enum UpdateType
	{
		TOP = 0,
		RECALC = 1
	};
	void UpdateLayout(UpdateType updateType = TOP);
	void RecalcPageRects(int nPage);
	void UpdateVisiblePages();
	void UpdatePage(int nPage, CThumbnailsThread* pThread);
	bool InvalidatePage(int nPage);
	void DrawPage(CDC* pDC, int nPage);
	int GetPageFromPoint(CPoint point);
	void ResizeThumbnails(int nThumbnailSize);
	void UpdateAllThumbnails();
	void PrintSelectedPages();
	void ExportSelectedPages();

	struct Page
	{
		Page() : pBitmap(NULL), bRendered(false), bSelected(false) {}
		~Page() { delete pBitmap; }

		CRect rcDisplay, rcPage, rcBitmap, rcNumber;
		CLightweightDIB* pBitmap;
		CSize szBitmap, szDisplay;
		bool bRendered;
		bool bSelected;

		void DeleteBitmap()
		{
			delete pBitmap;
			pBitmap = NULL;
			bRendered = false;
		}
	};
	vector<Page> m_pages;
	CSize m_szDisplay;

	// Generated message map functions
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThumbnailRendered(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnShowSettings(WPARAM wParam, LPARAM lParam);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg LRESULT OnShowParent(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
	DECLARE_MESSAGE_MAP()
};
