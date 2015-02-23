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
#include "MyTreeView.h"
#include "Drawing.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


int s_nOffsetLeft = 3;
int s_nTextOffset = 5;
int s_nOffsetRight = 2;

static HCURSOR hCursorLink = NULL;


// CMyTreeView control

IMPLEMENT_DYNAMIC(CMyTreeView, CMyScrollView)

CMyTreeView::CMyTreeView()
	: m_nItemHeight(20), m_pImageList(NULL), m_bWrapLabels(true), m_hTheme(NULL),
	  m_pSelection(NULL), m_pHoverNode(NULL), m_szDisplay(0, 0),
	  m_bMouseInTooltip(false), m_bRedirectWheel(true), m_bLinesAtRoot(false),
	  m_bHasLines(false), m_bHasGlyphs(false), m_bBatchUpdate(false),
	  m_bInsideOnSize(false)
{
	m_pRoot = new TreeNode(NULL, -1, -1, NULL);
	m_pRoot->bCollapsed = false;

	CreateSystemIconFont(m_font);

	LOGFONT lf;
	m_font.GetLogFont(&lf);

	lf.lfUnderline = true;
	m_fontHover.CreateFontIndirect(&lf);
}

CMyTreeView::~CMyTreeView()
{
	delete m_pRoot;
}

CMyTreeView::TreeNode::~TreeNode()
{
	TreeNode* pNode = pChild;
	while (pNode != NULL)
	{
		TreeNode* pNext = pNode->pNext;
		delete pNode;
		pNode = pNext;
	}
}

BEGIN_MESSAGE_MAP(CMyTreeView, CMyScrollView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE_VOID(WM_THEMECHANGED, OnThemeChanged)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SETCURSOR()
	ON_MESSAGE_VOID(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_KEYDOWN()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_STYLECHANGED()
END_MESSAGE_MAP()


// CMyTreeView message handlers

void CMyTreeView::OnDraw(CDC* pDC)
{
	CRect rcViewport(CPoint(0, 0), GetViewportSize());

	CRect rcClip;
	pDC->GetClipBox(rcClip);
	rcClip.IntersectRect(CRect(rcClip), rcViewport);

	m_offscreenDC.Create(pDC, rcClip.Size());
	m_offscreenDC.SetViewportOrg(-rcClip.TopLeft());
	m_offscreenDC.IntersectClipRect(rcClip);

	CPoint ptScroll = GetScrollPosition();

	CFont* pOldFont = m_offscreenDC.SelectObject(&m_font);
	COLORREF crBackground = ::GetSysColor(COLOR_WINDOW);
	m_offscreenDC.SetBkColor(crBackground);

	int nBottom = PaintNode(&m_offscreenDC, m_pRoot, rcClip);

	if (nBottom < m_szDisplay.cy)
	{
		CRect rcBottom(0, nBottom, m_szDisplay.cx, m_szDisplay.cy);
		m_offscreenDC.FillSolidRect(rcBottom - ptScroll, crBackground);
	}

	m_offscreenDC.SelectObject(pOldFont);

	pDC->BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(),
			&m_offscreenDC, rcClip.left, rcClip.top, SRCCOPY);

	m_offscreenDC.Release();
}

int CMyTreeView::PaintNode(CDC* pDC, TreeNode* pNode, const CRect& rcClip)
{
	CPoint ptScroll = GetScrollPosition();
	bool bFocus = (GetFocus() == this);

	int nBottom = (pNode == m_pRoot ? 0 : pNode->rcNode.bottom);

	CRect rcLine(pNode->rcNode);
	rcLine.right = m_szDisplay.cx;

	CRect rcIntersect;
	if (pNode != m_pRoot && rcIntersect.IntersectRect(rcClip, rcLine - ptScroll))
	{
		COLORREF crBackground = ::GetSysColor(COLOR_WINDOW);
		pDC->FillSolidRect(rcLine - ptScroll, crBackground);

		CFont* pOldFont = NULL;
		if (pNode == m_pHoverNode)
			pOldFont = pDC->SelectObject(&m_fontHover);

		UINT nFlags = DT_LEFT | DT_NOPREFIX | DT_TOP | (m_bWrapLabels ? DT_WORDBREAK : DT_SINGLELINE);
		if (pNode == m_pSelection)
		{
			CRect rcSel(pNode->rcLabel);
			rcSel.DeflateRect(s_nTextOffset - 3, 0, 0, 0);

			COLORREF crSel = ::GetSysColor(bFocus ? COLOR_HIGHLIGHT : COLOR_BTNFACE);
			pDC->FillSolidRect(rcSel - ptScroll, crSel);

			COLORREF crSelText = ::GetSysColor(bFocus ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT);
			pDC->SetTextColor(crSelText);

			pDC->DrawText(pNode->strLabel, pNode->rcText - ptScroll, nFlags);

			if (bFocus)
				DrawDottedRect(pDC, rcSel - ptScroll, crSelText);
		}
		else
		{
			COLORREF crText = ::GetSysColor(pNode == m_pHoverNode ? COLOR_HOTLIGHT : COLOR_WINDOWTEXT);
			pDC->SetTextColor(crText);
			pDC->DrawText(pNode->strLabel, pNode->rcText - ptScroll, DT_LEFT | DT_NOPREFIX | DT_WORDBREAK);
		}

		if (pNode == m_pHoverNode)
			pDC->SelectObject(pOldFont);

		if (m_pImageList != NULL && pNode->nImage != -1)
		{
			m_pImageList->Draw(pDC, pNode == m_pSelection ? pNode->nSelectedImage : pNode->nImage,
					pNode->rcImage.TopLeft() + (-ptScroll), ILD_NORMAL);
		}

		COLORREF crLines = ::GetSysColor(COLOR_BTNSHADOW);
		if (m_hTheme != NULL)
			XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_EDGESHADOWCOLOR, &crLines);

		if (m_bHasLines && (pNode->pParent != m_pRoot || m_bLinesAtRoot))
		{
			// Draw lines
			if (m_pRoot->pChild != pNode)
			{
				// This node is not the first child of the root, so there should be a vertical top half-line here
				DrawDottedLine(pDC, CPoint(pNode->nLineX, pNode->rcNode.top) + (-ptScroll),
					CPoint(pNode->nLineX, pNode->nLineY) + (-ptScroll), crLines);
			}

			if (pNode->HasSibling())
			{
				// This node is not the last child, so there should be a vertical bottom half-line here
				DrawDottedLine(pDC,	CPoint(pNode->nLineX, pNode->nLineY) + (-ptScroll),
					CPoint(pNode->nLineX, pNode->rcNode.bottom) + (-ptScroll), crLines);
			}

			// Horizontal line
			DrawDottedLine(pDC, CPoint(pNode->nLineX, pNode->nLineY) + (-ptScroll),
				CPoint(pNode->nLineStopX, pNode->nLineY) + (-ptScroll), crLines);

			if (pNode->HasChildren() && !pNode->bCollapsed)
			{
				// Vertical line to the first child
				DrawDottedLine(pDC, CPoint(pNode->pChild->nLineX, pNode->nLineY) + (-ptScroll),
					CPoint(pNode->pChild->nLineX, pNode->rcNode.bottom) + (-ptScroll), crLines);
			}

			TreeNode* pParent = pNode->pParent;
			while (pParent != m_pRoot)
			{
				if (pParent->HasSibling())
				{
					// This node has a sibling node, so there should be a full vertical line here
					DrawDottedLine(pDC, CPoint(pParent->nLineX, pNode->rcNode.top) + (-ptScroll),
						CPoint(pParent->nLineX, pNode->rcNode.bottom) + (-ptScroll), crLines);
				}

				pParent = pParent->pParent;
			}
		}

		if (pNode->HasChildren() && m_bHasGlyphs && (pNode->pParent != m_pRoot || m_bLinesAtRoot))
		{
			// Draw glyph
			if (m_hTheme != NULL)
			{
				int iGlyph = (pNode->bCollapsed ? GLPS_CLOSED : GLPS_OPENED);

				XPDrawThemeBackground(m_hTheme, pDC->m_hDC, TVP_GLYPH, iGlyph, pNode->rcGlyph - ptScroll, NULL);
			}
			else
			{
				pDC->FillSolidRect(pNode->rcGlyph - ptScroll, crBackground);
				FrameRect(pDC, pNode->rcGlyph - ptScroll, ::GetSysColor(COLOR_BTNSHADOW));

				CRect rcHorz(pNode->rcGlyph.left + 2, pNode->rcGlyph.CenterPoint().y,
					pNode->rcGlyph.right - 2, pNode->rcGlyph.CenterPoint().y + 1);
				pDC->FillSolidRect(rcHorz - ptScroll, RGB(0, 0, 0));

				if (pNode->bCollapsed)
				{
					CRect rcVert(pNode->rcGlyph.CenterPoint().x, pNode->rcGlyph.top + 2,
						pNode->rcGlyph.CenterPoint().x + 1, pNode->rcGlyph.bottom - 2);
					pDC->FillSolidRect(rcVert - ptScroll, RGB(0, 0, 0));
				}
			}
		}
	}

	if (pNode->HasChildren() && !pNode->bCollapsed)
	{
		for (TreeNode* pChild = pNode->pChild; pChild != NULL; pChild = pChild->pNext)
			nBottom = PaintNode(pDC, pChild, rcClip);
	}

	return nBottom;
}

void CMyTreeView::SetItemHeight(int nHeight)
{
	m_nItemHeight = nHeight;
	RecalcLayout();
}

void CMyTreeView::SetImageList(CImageList* pImageList, DWORD dwStyle)
{
	m_pImageList = pImageList;
	RecalcLayout();
}

void CMyTreeView::SetWrapLabels(bool bWrapLabels)
{
	if (m_bWrapLabels != bWrapLabels)
	{
		m_bWrapLabels = bWrapLabels;
		RecalcLayout();
	}
}

void CMyTreeView::SetRedirectWheel(bool bRedirectWheel)
{
	m_bRedirectWheel = bRedirectWheel;
}

HTREEITEM CMyTreeView::InsertItem(LPCTSTR pszItem, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	return InsertItem(pszItem, -1, -1, hParent, hInsertAfter);
}

HTREEITEM CMyTreeView::InsertItem(LPCTSTR pszItem, int nImage, int nSelectedImage, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	TreeNode* pParent = (hParent == TVI_ROOT ? m_pRoot : reinterpret_cast<TreeNode*>(hParent));
	ASSERT_POINTER(pParent, TreeNode);

	TreeNode* pInsertAfter = NULL;
	if (hInsertAfter == TVI_LAST)
	{
		pInsertAfter = pParent->pLastChild;
	}
	else if (hInsertAfter != TVI_FIRST)
	{
		pInsertAfter = reinterpret_cast<TreeNode*>(hInsertAfter);
		ASSERT_POINTER(pInsertAfter, TreeNode);
		ASSERT(pInsertAfter->pParent == pParent);
	}

	TreeNode* pNode = new TreeNode(pszItem, nImage, nSelectedImage, pParent);
	if (pInsertAfter != NULL)
	{
		pNode->pNext = pInsertAfter->pNext;
		pInsertAfter->pNext = pNode;
	}
	else
	{
		pNode->pNext = pParent->pChild;
		pParent->pChild = pNode;
	}

	if (pParent->pLastChild == pInsertAfter)
		pParent->pLastChild = pNode;

	if (!m_bBatchUpdate)
		RecalcLayout();

	return reinterpret_cast<HTREEITEM>(pNode);
}

bool CMyTreeView::DeleteItem(HTREEITEM hItem)
{
	if (hItem == TVI_ROOT)
		return false;

	TreeNode* pNode = reinterpret_cast<TreeNode*>(hItem);
	ASSERT_POINTER(pNode, TreeNode);

	while (pNode->pChild != NULL)
	{
		bool bBatchUpdate = m_bBatchUpdate;
		m_bBatchUpdate = true;
		DeleteItem(reinterpret_cast<HTREEITEM>(pNode->pChild));
		m_bBatchUpdate = bBatchUpdate;
	}

	TreeNode* pParent = pNode->pParent;
	if (pParent->pChild == pNode)
	{
		pParent->pChild = pNode->pNext;
		if (pParent->pLastChild == pNode)
			pParent->pLastChild = NULL;
	}
	else
	{
		TreeNode* pPrev = pParent->pChild;
		while (pPrev != NULL && pPrev->pNext != pNode)
			pPrev = pPrev->pNext;

		if (pPrev == NULL)
			return false;

		pPrev->pNext = pNode->pNext;
		if (pParent->pLastChild == pNode)
			pParent->pLastChild = pPrev;
	}

	if (m_pSelection == pNode)
		m_pSelection = NULL;

	NMTREEVIEW nmtv;
	InitNotification(nmtv, TVN_DELETEITEM);

	nmtv.itemOld.mask = TVIF_HANDLE | TVIF_PARAM;
	nmtv.itemOld.hItem = (HTREEITEM) pNode;
	nmtv.itemOld.lParam = (LPARAM) pNode->dwUserData;

	GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM) &nmtv);

	if (!m_bBatchUpdate)
		RecalcLayout();

	return true;
}

void CMyTreeView::DeleteAllItems()
{
	bool bBatchUpdate = m_bBatchUpdate;

	m_bBatchUpdate = true;
	while (m_pRoot->pChild != NULL)
		DeleteItem(reinterpret_cast<HTREEITEM>(m_pRoot->pChild));
	m_bBatchUpdate = bBatchUpdate;

	if (!m_bBatchUpdate)
		RecalcLayout();
}

void CMyTreeView::InitNotification(NMTREEVIEW& nmtv, UINT nCode)
{
	ZeroMemory(&nmtv, sizeof(nmtv));

	nmtv.hdr.hwndFrom = GetSafeHwnd();
	nmtv.hdr.idFrom = GetDlgCtrlID();
	nmtv.hdr.code = nCode;
}

void CMyTreeView::BeginBatchUpdate()
{
	m_bBatchUpdate = true;
}

void CMyTreeView::EndBatchUpdate()
{
	m_bBatchUpdate = false;

	RecalcLayout();

	if (m_pSelection != NULL && m_pSelection->nIndex == -1)
		SelectNode(NULL);
}

void CMyTreeView::RecalcLayout()
{
	CScreenDC dcScreen;
	CDC dc;
	dc.CreateCompatibleDC(&dcScreen);
	CFont* pOldFont = dc.SelectObject(&m_font);

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int nFontHeight = tm.tmHeight;
	int nTextOffsetTop = max(0, (m_nItemHeight - nFontHeight) / 2);

	CSize szGlyph(9, 9);
	if (m_hTheme != NULL)
	{
		CSize sz(9, 9);
		XPGetThemePartSize(m_hTheme, dc.m_hDC, TVP_GLYPH, GLPS_CLOSED, NULL, TS_TRUE, &sz);
		XPGetThemePartSize(m_hTheme, dc.m_hDC, TVP_GLYPH, GLPS_OPENED, NULL, TS_TRUE, &szGlyph);

		szGlyph.cx = max(szGlyph.cx, sz.cx);
		szGlyph.cy = max(szGlyph.cy, sz.cy);
	}

	int nChildOffset = szGlyph.cx + 10;

	CSize szViewport = ::GetClientSize(this);
	bool bHScroll = false, bVScroll = false;

	do
	{
		int nTop = 0;
		int nLevel = 0;
		stack<pair<TreeNode*, bool> > s;
		s.push(make_pair(m_pRoot, true));

		m_items.clear();

		m_szDisplay = CSize(0, 0);
		while (!s.empty())
		{
			TreeNode* pNode = s.top().first;
			if (pNode != m_pRoot && s.top().second)
			{
				pNode->nIndex = (int)m_items.size();
				m_items.push_back(pNode);

				pNode->rcNode.left = s_nOffsetLeft + (nLevel - 1)*nChildOffset;
				pNode->rcNode.top = nTop;
				pNode->rcNode.bottom = nTop + m_nItemHeight;

				bool bHasLine = nLevel > 1 || m_bLinesAtRoot;
				if (nLevel > 1 && !m_bLinesAtRoot)
					pNode->rcNode.left -= nChildOffset;

				pNode->rcText.left = pNode->rcNode.left + (bHasLine ? nChildOffset : 0);
				pNode->rcText.top = nTop + nTextOffsetTop;

				pNode->rcGlyph.left = pNode->rcNode.left + 4;
				pNode->rcGlyph.right = pNode->rcGlyph.left + szGlyph.cx;
				pNode->rcGlyph.top = nTop + (m_nItemHeight + 1 - szGlyph.cy) / 2;
				pNode->rcGlyph.bottom = pNode->rcGlyph.top + szGlyph.cy;

				pNode->nLineX = pNode->rcGlyph.left + szGlyph.cx / 2;
				pNode->nLineY = pNode->rcGlyph.top + szGlyph.cy / 2;
				pNode->nLineStopX = pNode->rcNode.left + nChildOffset;

				if (m_pImageList != NULL && pNode->nImage != -1)
				{
					int cx = 0, cy = 0;
					::ImageList_GetIconSize(m_pImageList->m_hImageList, &cx, &cy);

					pNode->rcImage.top = pNode->rcNode.top + (m_nItemHeight - cy) / 2;
					pNode->rcImage.bottom = pNode->rcImage.top + cy;
					pNode->rcImage.left = pNode->rcText.left;
					pNode->rcImage.right = pNode->rcImage.left + cx;

					pNode->rcText.left += cx;
				}

				pNode->rcText.left += s_nTextOffset;
				pNode->rcText.right = pNode->rcText.left + 2;
				
				if (!pNode->strLabel.IsEmpty())
				{
					pNode->rcText.right = max(pNode->rcText.right, szViewport.cx - s_nOffsetRight);
					pNode->rcText.bottom = nTop + m_nItemHeight;

					UINT nFlags = DT_CALCRECT | DT_LEFT | DT_NOPREFIX | DT_TOP |
						(m_bWrapLabels ? DT_WORDBREAK : DT_SINGLELINE);
					int nHeight = dc.DrawText(pNode->strLabel, pNode->rcText, nFlags);
					pNode->rcText.bottom = pNode->rcText.top + nHeight;
				}
				else
					pNode->rcText.bottom = nTop + m_nItemHeight - 2;

				pNode->rcNode.bottom = max(pNode->rcNode.bottom, pNode->rcText.bottom + 2);
				pNode->rcNode.bottom += (pNode->rcNode.bottom % 2);

				pNode->rcLabel = pNode->rcText;
				pNode->rcLabel.left -= s_nTextOffset;
				pNode->rcLabel.right += s_nOffsetRight;
				pNode->rcLabel.top = pNode->rcNode.top;
				pNode->rcLabel.bottom = pNode->rcNode.bottom;

				pNode->rcNode.left = 0;
				pNode->rcNode.right = pNode->rcLabel.right;

				m_szDisplay.cx = max(m_szDisplay.cx, pNode->rcNode.Width());
				nTop = pNode->rcNode.bottom;
			}
			else
			{
				pNode->nIndex = -1;
			}

			if (pNode->HasChildren())
			{
				s.push(make_pair(pNode->pChild, s.top().second && !pNode->bCollapsed));
				++nLevel;
			}
			else
			{
				while (!s.empty() && s.top().first->pNext == NULL)
				{
					s.pop();
					--nLevel;
				}
				if (!s.empty())
					s.top().first = s.top().first->pNext;
			}
		}

		m_szDisplay.cy = nTop;
	} while (AdjustViewportSize(m_szDisplay, szViewport, bHScroll, bVScroll));

	m_szDisplay.cx = max(m_szDisplay.cx, szViewport.cx);
	m_szDisplay.cy = max(m_szDisplay.cy, szViewport.cy);

	CSize szPage(szViewport.cx*9/10, szViewport.cy*9/10);
	CSize szLine(15, 15);
	SetScrollSizes(m_szDisplay, szPage, szLine, false);

	dc.SelectObject(pOldFont);

	SetHoverNode(NULL);
	if (!m_bInsideOnSize)
		UpdateHoverNode();

	Invalidate();
	UpdateWindow();
}

int CMyTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_bHasLines = (lpCreateStruct->style & TVS_HASLINES) != 0;
	m_bLinesAtRoot = (lpCreateStruct->style & TVS_LINESATROOT) != 0;
	m_bHasGlyphs = (lpCreateStruct->style & TVS_HASBUTTONS) != 0;

	if (CMyScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (IsThemed())
		m_hTheme = XPOpenThemeData(m_hWnd, L"TREEVIEW");

	m_toolTip.Create(this);

	return 0;
}

void CMyTreeView::OnDestroy()
{
	if (m_hTheme != NULL)
	{
		XPCloseThemeData(m_hTheme);
		m_hTheme = NULL;
	}

	m_toolTip.DestroyWindow();

	CMyScrollView::OnDestroy();
}

void CMyTreeView::OnSize(UINT nType, int cx, int cy) 
{
	if (cx > 0 && cy > 0)
	{
		m_bInsideOnSize = true;
		RecalcLayout();
		m_bInsideOnSize = false;
	}

	CMyScrollView::OnSize(nType, cx, cy);
}

BOOL CMyTreeView::OnEraseBkgnd(CDC* pDC) 
{
	return true;
}

void CMyTreeView::OnThemeChanged()
{
	if (m_hTheme != NULL)
		XPCloseThemeData(m_hTheme);

	m_hTheme = NULL;

	if (IsThemed())
		m_hTheme = XPOpenThemeData(m_hWnd, L"TREEVIEW");

	m_font.DeleteObject();
	CreateSystemIconFont(m_font);

	LOGFONT lf;
	m_font.GetLogFont(&lf);
	lf.lfUnderline = true;

	m_fontHover.DeleteObject();
	m_fontHover.CreateFontIndirect(&lf);

	RecalcLayout();
}

void CMyTreeView::OnSysColorChange()
{
	Invalidate();
}

void CMyTreeView::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	UpdateHoverNode(point);

	int nArea;
	TreeNode* pNode = HitTest(point, &nArea);

	if (pNode != NULL)
	{
		if (nArea == HT_IMAGE || nArea == HT_LABEL)
		{
			SelectNode(pNode, TVC_BYMOUSE);
		}
		else if (nArea == HT_GLYPH && pNode->HasChildren())
		{
			bool bAllChildren = (nFlags & (MK_CONTROL | MK_SHIFT)) != 0;
			ToggleNode(pNode, bAllChildren);
		}
	}

	CMyScrollView::OnLButtonDown(nFlags, point);
}

void CMyTreeView::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	UpdateHoverNode(point);

	int nArea;
	TreeNode* pNode = HitTest(point, &nArea);

	if (pNode != NULL)
	{
		if (nArea == HT_IMAGE || nArea == HT_LABEL)
		{
			SelectNode(pNode, TVC_UNKNOWN);
		}
	}

	CMyScrollView::OnRButtonDown(nFlags, point);
}

CMyTreeView::TreeNode* CMyTreeView::HitTest(CPoint point, int* pnArea)
{
	return HitTest(m_pRoot, point, pnArea);
}

CMyTreeView::TreeNode* CMyTreeView::HitTest(TreeNode* pNode, CPoint point, int* pnArea)
{
	CPoint ptScroll = GetScrollPosition();

	if (pNode != m_pRoot && pNode->rcNode.PtInRect(point + ptScroll))
	{
		if (pNode->rcLabel.PtInRect(point + ptScroll))
			*pnArea = HT_LABEL;
		else if (pNode->rcImage.PtInRect(point + ptScroll))
			*pnArea = HT_IMAGE;
		else if (pNode->rcGlyph.PtInRect(point + ptScroll))
			*pnArea = HT_GLYPH;
		else
			*pnArea = HT_OTHER;

		return pNode;
	}

	if (pNode->HasChildren() && !pNode->bCollapsed)
	{
		for (TreeNode* pChild = pNode->pChild; pChild != NULL; pChild = pChild->pNext)
		{
			TreeNode* pResult = HitTest(pChild, point, pnArea);
			if (pResult != NULL)
				return pResult;
		}
	}

	return NULL;
}

void CMyTreeView::InvalidateNode(TreeNode* pNode)
{
	CPoint ptScroll = GetScrollPosition();
	InvalidateRect(pNode->rcNode - ptScroll);
}

void CMyTreeView::OnMouseMove(UINT nFlags, CPoint point)
{
	bool bDragging = (nFlags & (MK_LBUTTON | MK_RBUTTON)) != 0;
	if (bDragging)
		return;

	UpdateHoverNode(point);

	CMyScrollView::OnMouseMove(nFlags, point);
}

void CMyTreeView::OnMouseLeave()
{
	if (!m_bMouseInTooltip)
		SetHoverNode(NULL);
}

void CMyTreeView::UpdateHoverNode()
{
	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);
	UpdateHoverNode(ptCursor);
}

void CMyTreeView::UpdateHoverNode(const CPoint& point)
{
	CRect rcViewport(CPoint(0, 0), GetViewportSize());
	if (!rcViewport.PtInRect(point))
	{
		SetHoverNode(NULL);
		return;
	}

	int nArea;
	TreeNode* pNode = HitTest(point, &nArea);

	if (pNode != NULL && nArea != HT_IMAGE && nArea != HT_LABEL)
		pNode = NULL;

	SetHoverNode(pNode);
	SendMessage(WM_SETCURSOR, (WPARAM) m_hWnd, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
}

void CMyTreeView::SetHoverNode(TreeNode* pNode)
{
	if (pNode == m_pHoverNode)
		return;

	if (m_pHoverNode != NULL)
	{
		InvalidateNode(m_pHoverNode);
		m_toolTip.Hide();

		m_bMouseInTooltip = false;
		++m_toolTip.m_nNextCode;
	}

	m_pHoverNode = pNode;

	if (pNode != NULL)
	{
		InvalidateNode(pNode);

		CPoint ptScroll = GetScrollPosition();
		CRect rcLabel = pNode->rcLabel - ptScroll;

		CRect rcViewport(CPoint(0, 0), GetViewportSize());
		CRect rcIntersect;
		if (rcIntersect.IntersectRect(rcLabel, rcViewport) && rcIntersect != rcLabel)
		{
			CPoint ptTooltip = pNode->rcLabel.TopLeft() + (-ptScroll);
			ptTooltip.Offset(s_nTextOffset - 3, 0);

			CRect rcTooltip(pNode->rcLabel);
			rcTooltip.DeflateRect(s_nTextOffset - 3, 0, 0, 0);

			CRect rcTooltipText(pNode->rcText - rcTooltip.TopLeft());
			rcTooltip -= ptScroll;
			ClientToScreen(rcTooltip);

			m_bMouseInTooltip = true;
			++m_toolTip.m_nNextCode;

			m_toolTip.Show(pNode->strLabel, m_bWrapLabels, rcTooltip, rcTooltipText);
		}
	}

	GetTopLevelParent()->UpdateWindow();

	TRACKMOUSEEVENT tme;

	ZeroMemory(&tme, sizeof(tme));
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE | (m_pHoverNode == NULL || m_toolTip.IsWindowVisible() ? TME_CANCEL : 0);
	tme.hwndTrack = m_hWnd;
	tme.dwHoverTime = HOVER_DEFAULT;

	::TrackMouseEvent(&tme);
}

void CMyTreeView::OnSetFocus(CWnd* pOldWnd)
{
	CMyScrollView::OnSetFocus(pOldWnd);

	if (m_pSelection != NULL)
	{
		InvalidateNode(m_pSelection);
		UpdateWindow();
	}
}

void CMyTreeView::OnKillFocus(CWnd* pNewWnd)
{
	CMyScrollView::OnKillFocus(pNewWnd);

	SetHoverNode(NULL);

	if (m_pSelection != NULL)
	{
		InvalidateNode(m_pSelection);
		UpdateWindow();
	}
}

BOOL CMyTreeView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_bPanning)
		return true;

	if (hCursorLink == NULL)
		hCursorLink = ::LoadCursor(0, IDC_HAND);
	if (hCursorLink == NULL)
		hCursorLink = AfxGetApp()->LoadCursor(IDC_CURSOR_LINK);

	if (nHitTest == HTCLIENT && m_pHoverNode != NULL)
	{
		SetCursor(hCursorLink);
		return true;
	}

	return CMyScrollView::OnSetCursor(pWnd, nHitTest, message);
}

bool CMyTreeView::OnScrollBy(CSize szScrollBy, bool bDoScroll)
{
	if (!CMyScrollView::OnScrollBy(szScrollBy, bDoScroll))
		return false;

	SetHoverNode(NULL);
	if (!m_bPanning)
		UpdateHoverNode();

	UpdateWindow();
	return true;
}

bool CMyTreeView::OnStartPan()
{
	SetHoverNode(NULL);
	return true;
}

void CMyTreeView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Notify parent
	NMTVKEYDOWN tvkd;
	tvkd.hdr.hwndFrom = GetSafeHwnd();
	tvkd.hdr.idFrom = GetDlgCtrlID();
	tvkd.hdr.code = TVN_KEYDOWN;
	tvkd.wVKey = (WORD) nChar;
	tvkd.flags = 0;
	GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM) &tvkd);

	TreeNode* pSelectNode = NULL;

	switch (nChar)
	{
	case VK_DOWN:
		pSelectNode = FindNextNode(m_pSelection);
		break;

	case VK_UP:
		pSelectNode = FindPrevNode(m_pSelection);
		break;

	case VK_RIGHT:
		if (m_pSelection != NULL)
		{
			if (m_pSelection->HasChildren())
			{
				if (m_pSelection->bCollapsed)
					ToggleNode(m_pSelection);
				else
					pSelectNode = m_pSelection->pChild;
			}
		}
		else
		{
			pSelectNode = FindNextNode(NULL);
		}
		break;

	case VK_LEFT:
		if (m_pSelection != NULL)
		{
			if (m_pSelection->HasChildren() && !m_pSelection->bCollapsed)
				ToggleNode(m_pSelection);
			else if (m_pSelection->pParent != m_pRoot)
				pSelectNode = m_pSelection->pParent;
		}
		else
		{
			pSelectNode = FindPrevNode(NULL);
		}
		break;

	case VK_ADD:
		if (m_pSelection != NULL && m_pSelection->HasChildren() && m_pSelection->bCollapsed)
			ToggleNode(m_pSelection);
		break;

	case VK_SUBTRACT:
		if (m_pSelection != NULL && m_pSelection->HasChildren() && !m_pSelection->bCollapsed)
			ToggleNode(m_pSelection);
		break;

	case VK_NEXT:
		pSelectNode = FindNextPageNode(m_pSelection);
		break;

	case VK_PRIOR:
		pSelectNode = FindPrevPageNode(m_pSelection);
		break;
	}

	if (pSelectNode != NULL && pSelectNode != m_pSelection)
		SelectNode(pSelectNode, TVC_BYKEYBOARD);

	CMyScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMyTreeView::SelectItem(HTREEITEM hItem)
{
	if (hItem == TVI_ROOT)
	{
		SelectNode(NULL);
		return;
	}

	TreeNode* pNode = (TreeNode*)hItem;
	SelectNode(pNode);
}

void CMyTreeView::SelectNode(TreeNode* pNode, UINT nAction)
{
	if (m_pSelection != pNode)
	{
		NMTREEVIEW nmtv;
		InitNotification(nmtv, TVN_SELCHANGING);
		nmtv.action = nAction;

		if (m_pSelection != NULL)
		{
			nmtv.itemOld.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
			nmtv.itemOld.hItem = (HTREEITEM) m_pSelection;
			nmtv.itemOld.state = TVIS_SELECTED | (!m_pSelection->bCollapsed ? TVIS_EXPANDED : 0);
			nmtv.itemOld.lParam = (LPARAM) m_pSelection->dwUserData;
		}

		if (pNode != NULL)
		{
			nmtv.itemNew.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
			nmtv.itemNew.hItem = (HTREEITEM) pNode;
			nmtv.itemNew.state = (!pNode->bCollapsed ? TVIS_EXPANDED : 0);
			nmtv.itemNew.lParam = (LPARAM) pNode->dwUserData;
		}

		// The parent can cancel selection change by returning true
		if (GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM) &nmtv))
			return;

		TreeNode* pOldSel = m_pSelection;

		if (m_pSelection != NULL)
			InvalidateNode(m_pSelection);
		if (pNode != NULL)
			InvalidateNode(pNode);

		m_pSelection = pNode;

		if (pNode != NULL)
			EnsureVisible(pNode);

		InitNotification(nmtv, TVN_SELCHANGED);
		nmtv.action = nAction;

		if (pOldSel != NULL)
		{
			nmtv.itemOld.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
			nmtv.itemOld.hItem = (HTREEITEM) pOldSel;
			nmtv.itemOld.state = (!pOldSel->bCollapsed ? TVIS_EXPANDED : 0);
			nmtv.itemOld.lParam = (LPARAM) pOldSel->dwUserData;
		}

		if (pNode != NULL)
		{
			nmtv.itemNew.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
			nmtv.itemNew.hItem = (HTREEITEM) pNode;
			nmtv.itemNew.state = TVIS_SELECTED | (!pNode->bCollapsed ? TVIS_EXPANDED : 0);
			nmtv.itemNew.lParam = (LPARAM) pNode->dwUserData;
		}

		GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM) &nmtv);
	}
	else if (pNode != NULL && nAction == TVC_BYMOUSE)
	{
		// Still send a notification message
		NMTREEVIEW nmtv;
		InitNotification(nmtv, TVN_ITEMCLICKED);
		nmtv.action = nAction;

		nmtv.itemNew.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
		nmtv.itemNew.hItem = (HTREEITEM) pNode;
		nmtv.itemNew.state = TVIS_SELECTED | (!pNode->bCollapsed ? TVIS_EXPANDED : 0);
		nmtv.itemNew.lParam = (LPARAM) pNode->dwUserData;

		GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM) &nmtv);
	}

	UpdateWindow();
}

void CMyTreeView::ExpandNode(TreeNode* pNode, bool bExpand, bool bAllChildren)
{
	if (bAllChildren)
	{
		bool bBatchUpdate = m_bBatchUpdate;
		m_bBatchUpdate = true;
		for (TreeNode* pChild = pNode->pChild; pChild != NULL; pChild = pChild->pNext)
			ExpandNode(pChild, bExpand, true);
		m_bBatchUpdate = bBatchUpdate;
	}

	if (pNode->bCollapsed == !bExpand)
		return;

	NMTREEVIEW nmtv;
	InitNotification(nmtv, TVN_ITEMEXPANDING);
	nmtv.action = (bExpand ? TVE_EXPAND : TVE_COLLAPSE);

	nmtv.itemNew.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
	nmtv.itemNew.hItem = (HTREEITEM) pNode;
	nmtv.itemNew.state = (pNode == m_pSelection ? TVIS_SELECTED : 0) | (!pNode->bCollapsed ? TVIS_EXPANDED : 0);
	nmtv.itemNew.lParam = (LPARAM) pNode->dwUserData;

	// The parent can cancel expanding/collapsing by returning true
	if (GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM) &nmtv))
		return;

	pNode->bCollapsed = !bExpand;

	InitNotification(nmtv, TVN_ITEMEXPANDED);
	nmtv.action = (bExpand ? TVE_EXPAND : TVE_COLLAPSE);

	nmtv.itemNew.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
	nmtv.itemNew.hItem = (HTREEITEM) pNode;
	nmtv.itemNew.state = (pNode == m_pSelection ? TVIS_SELECTED : 0) | (!pNode->bCollapsed ? TVIS_EXPANDED : 0);
	nmtv.itemNew.lParam = (LPARAM) pNode->dwUserData;

	GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM) &nmtv);

	if (!m_bBatchUpdate)
	{
		RecalcLayout();

		if (m_pSelection != NULL && m_pSelection->nIndex == -1)
			SelectNode(pNode);
	}
}

void CMyTreeView::ToggleNode(TreeNode* pNode, bool bAllChildren)
{
	ExpandNode(pNode, pNode->bCollapsed, bAllChildren);
}

bool CMyTreeView::Expand(HTREEITEM hItem, UINT nCode)
{
	if (hItem == TVI_ROOT)
		return false;

	TreeNode* pNode = reinterpret_cast<TreeNode*>(hItem);
	ASSERT_POINTER(pNode, TreeNode);

	switch (nCode)
	{
	case TVE_COLLAPSE:
		ExpandNode(pNode, false);
		return true;

	case TVE_COLLAPSERESET:
		return false;

	case TVE_EXPAND:
		ExpandNode(pNode, true);
		return true;

	case TVE_TOGGLE:
		ToggleNode(pNode);
		return true;
   	}

	return false;
}

CMyTreeView::TreeNode* CMyTreeView::FindNextNode(TreeNode* pNode)
{
	if (pNode == NULL)
		return (m_items.empty() ? NULL : m_items.front());

	if (pNode->nIndex == -1 || pNode->nIndex + 1 == m_items.size())
		return NULL;

	return m_items[pNode->nIndex + 1];
}

CMyTreeView::TreeNode* CMyTreeView::FindPrevNode(TreeNode* pNode)
{
	if (pNode == NULL)
		return (m_items.empty() ? NULL : m_items.back());

	if (pNode->nIndex == -1 || pNode->nIndex == 0)
		return NULL;

	return m_items[pNode->nIndex - 1];
}

CMyTreeView::TreeNode* CMyTreeView::FindNextPageNode(TreeNode* pNode)
{
	if (pNode == NULL)
		return (m_items.empty() ? NULL : m_items.front());

	if (pNode->nIndex == -1 || pNode->nIndex + 1 == m_items.size())
		return NULL;

	CSize szViewport = GetViewportSize();

	int nIndex = pNode->nIndex + 1;
	while (nIndex + 1 < static_cast<int>(m_items.size()) &&
			m_items[nIndex + 1]->rcNode.bottom <= pNode->rcNode.top + szViewport.cy)
		++nIndex;

	return m_items[nIndex];
}

CMyTreeView::TreeNode* CMyTreeView::FindPrevPageNode(TreeNode* pNode)
{
	if (pNode == NULL)
		return (m_items.empty() ? NULL : m_items.back());

	if (pNode->nIndex == -1 || pNode->nIndex == 0)
		return NULL;

	CSize szViewport = GetViewportSize();

	int nIndex = pNode->nIndex - 1;
	while (nIndex >= 1 && m_items[nIndex - 1]->rcNode.top >= pNode->rcNode.bottom - szViewport.cy)
		--nIndex;

	return m_items[nIndex];
}

void CMyTreeView::SetItemData(HTREEITEM hItem, DWORD_PTR dwData)
{
	TreeNode* pNode = (TreeNode*) hItem;
	pNode->dwUserData = dwData;
}

DWORD_PTR CMyTreeView::GetItemData(HTREEITEM hItem)
{
	TreeNode* pNode = (TreeNode*) hItem;
	return pNode->dwUserData;
}

HTREEITEM CMyTreeView::GetSelectedItem()
{
	return (HTREEITEM) m_pSelection;
}

BOOL CMyTreeView::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	if (m_bRedirectWheel)
	{
		CWnd* pWnd = WindowFromPoint(point);
		if (pWnd != NULL && pWnd != this && pWnd->m_hWnd != m_toolTip.m_hWnd
				&& !IsChild(pWnd) && IsFromCurrentProcess(pWnd)
				&& pWnd->SendMessage(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(point.x, point.y)) != 0)
			return true;
	}

	return CMyScrollView::OnMouseWheel(nFlags, zDelta, point);
}

void CMyTreeView::EnsureVisible(TreeNode* pNode)
{
	CPoint ptScroll = GetScrollPosition();
	CRect rcNode = pNode->rcNode - ptScroll;

	CRect rcViewport(CPoint(0, 0), GetViewportSize());

	int nScrollBy = 0;
	if (rcNode.bottom > rcViewport.bottom)
		nScrollBy = rcNode.bottom - rcViewport.bottom;
	if (rcNode.top - nScrollBy < rcViewport.top)
		nScrollBy = rcNode.top - rcViewport.top;

	OnScrollBy(CSize(0, nScrollBy));
}

void CMyTreeView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	int nArea;
	TreeNode* pNode = HitTest(point, &nArea);

	if (pNode != NULL && pNode->HasChildren()
			&& (nArea == HT_IMAGE || nArea == HT_LABEL || nArea == HT_GLYPH))
	{
		bool bAllChildren = (nFlags & (MK_CONTROL | MK_SHIFT)) != 0;
		ToggleNode(pNode, bAllChildren);
		return;
	}

	CMyScrollView::OnLButtonDblClk(nFlags, point);
}

bool CMyTreeView::UpdateParameters()
{
	bool bChanged = false;
	DWORD dwStyle = GetStyle();

	if (m_bHasLines != ((dwStyle & TVS_HASLINES) != 0))
	{
		m_bLinesAtRoot = (dwStyle & TVS_HASLINES) != 0;
		bChanged = true;
	}

	if (m_bLinesAtRoot != ((dwStyle & TVS_LINESATROOT) != 0))
	{
		m_bLinesAtRoot = (dwStyle & TVS_LINESATROOT) != 0;
		bChanged = true;
	}

	if (m_bHasGlyphs != ((dwStyle & TVS_HASBUTTONS) != 0))
	{
		m_bHasGlyphs = (dwStyle & TVS_HASBUTTONS) != 0;
		bChanged = true;
	}

	return bChanged;
}

void CMyTreeView::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	if (UpdateParameters())
	{
		RecalcLayout();
	}
}

BOOL CMyTreeView::CTreeToolTip::Create(CMyTreeView* pTree)
{
	m_pTree = pTree;

	static CString strWndClass = AfxRegisterWndClass(CS_DBLCLKS,
			::LoadCursor(NULL, IDC_ARROW));

	return CreateEx(WS_EX_TOOLWINDOW, strWndClass, NULL, WS_POPUP,
		CRect(0, 0, 0, 0), pTree, 0);
}

void CMyTreeView::CTreeToolTip::Show(const CString& strText,
	bool bWrap, const CRect& rcWindow, const CRect& rcText)
{
	m_strText = strText;
	m_bWrap = bWrap;
	m_rcText = rcText;

	SetWindowPos(&wndTop, rcWindow.left, rcWindow.top,
			rcWindow.Width(), rcWindow.Height(), SWP_NOACTIVATE);
	ShowWindow(SW_SHOWNA);

	Invalidate();
	UpdateWindow();
}

void CMyTreeView::CTreeToolTip::Hide()
{
	if (IsWindowVisible())
	{
		ShowWindow(SW_HIDE);
		m_pTree->UpdateHoverNode();
	}
}

BOOL CMyTreeView::CTreeToolTip::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
		{
			m_pTree->m_bMouseInTooltip = true;

			TRACKMOUSEEVENT tme;

			ZeroMemory(&tme, sizeof(tme));
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = m_hWnd;
			tme.dwHoverTime = HOVER_DEFAULT;

			m_nMouseLeaveCode = m_nNextCode;
			::TrackMouseEvent(&tme);
		}
		break;

	case WM_MOUSEACTIVATE:
		if (pResult != NULL)
			*pResult = MA_NOACTIVATE;
		return true;

	case WM_MOUSELEAVE:
		if (m_nNextCode == m_nMouseLeaveCode)
		{
			m_pTree->m_bMouseInTooltip = false;

			// GetAsyncKeyState always returns state of physical buttons, even
			// if they are reversed. We only want to check if any of the mouse
			// buttons is pressed, so there are no extra checks here
			bool bLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
			bool bRightDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
			bool bMiddleDown = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;

			if (!bLeftDown && !bRightDown && !bMiddleDown)
				m_pTree->UpdateHoverNode();
		}
		break;

	case WM_MOUSEWHEEL:
		{
			UINT nFlags = LOWORD(wParam);
			short zDelta = (short) HIWORD(wParam);
			CPoint point((DWORD) lParam);

			LRESULT lResult = (LRESULT) m_pTree->OnMouseWheel(nFlags, zDelta, point);
			if (pResult != NULL)
				*pResult = lResult;
		}
		return true;

	case WM_LBUTTONDOWN:
		m_pTree->SelectNode(m_pTree->m_pHoverNode, TVC_BYMOUSE);
		return true;

	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
		{
			m_pTree->m_bMouseInTooltip = false;
			Hide();

			CPoint point((DWORD) lParam);
			ClientToScreen(&point);
			m_pTree->ScreenToClient(&point);
			m_pTree->SendMessage(message, wParam, MAKELPARAM(point.x, point.y));
		}
		return true;

	case WM_MBUTTONDOWN:
		{
			m_pTree->m_bMouseInTooltip = false;
			Hide();

			CPoint point((DWORD) lParam);
			ClientToScreen(&point);
			CWnd* pWnd = WindowFromPoint(point);
			if (pWnd == m_pTree)
			{
				m_pTree->ScreenToClient(&point);
				m_pTree->SendMessage(message, wParam, MAKELPARAM(point.x, point.y));
			}
		}
		return true;

	case WM_SETCURSOR:
		{
			if (hCursorLink == NULL)
				hCursorLink = ::LoadCursor(0, IDC_HAND);
			if (hCursorLink == NULL)
				hCursorLink = AfxGetApp()->LoadCursor(IDC_CURSOR_LINK);

			SetCursor(hCursorLink);

			if (pResult != NULL)
				*pResult = true;
		}
		return true;

	case WM_PAINT:
		{
			CPaintDC dcPaint(this);
			CFont* pOldFont = dcPaint.SelectObject(&m_pTree->m_font);

			CRect rcClient = ::GetClientRect(this);
			FrameRect(&dcPaint, rcClient, ::GetSysColor(COLOR_INFOTEXT));

			rcClient.DeflateRect(1, 1);
			dcPaint.FillSolidRect(rcClient, ::GetSysColor(COLOR_INFOBK));

			UINT nFlags = DT_LEFT | DT_NOPREFIX | DT_TOP | (m_bWrap ? DT_WORDBREAK : DT_SINGLELINE);
			dcPaint.SetTextColor(::GetSysColor(COLOR_INFOTEXT));
			dcPaint.DrawText(m_strText, m_rcText, nFlags);

			dcPaint.SelectObject(pOldFont);
		}
		return true;
	}

	return CWnd::OnWndMsg(message, wParam, lParam, pResult);
}
