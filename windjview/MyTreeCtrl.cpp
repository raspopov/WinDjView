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

#include "stdafx.h"
#include "WinDjView.h"
#include "MyTreeCtrl.h"
#include "Drawing.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


int s_nOffsetLeft = 3;
int s_nTextOffset = 5;
int s_nOffsetRight = 2;

static HCURSOR hCursorLink = NULL;


// CMyTreeCtrl control

IMPLEMENT_DYNAMIC(CMyTreeCtrl, CWnd)

CMyTreeCtrl::CMyTreeCtrl()
	: m_nItemHeight(20), m_pImageList(NULL), m_bWrapLabels(true), m_hTheme(NULL),
	  m_pSelection(NULL), m_pHoverNode(NULL), m_ptScrollOffset(0, 0), m_szDisplay(0, 0),
	  m_szLine(15, 15), m_szPage(0, 0), m_bMouseInTooltip(false), m_bRedirectWheel(true),
	  m_bLinesAtRoot(false), m_bHasLines(false), m_bHasGlyphs(false), m_bBatchUpdate(false)
{
	m_pRoot = new TreeNode(NULL, -1, -1, NULL);
	m_pRoot->bCollapsed = false;

	CreateSystemIconFont(m_font);

	LOGFONT lf;
	::GetObject(m_font.m_hObject, sizeof(LOGFONT), &lf);

	lf.lfUnderline = true;
	m_fontHover.CreateFontIndirect(&lf);
}

CMyTreeCtrl::~CMyTreeCtrl()
{
	delete m_pRoot;
}

CMyTreeCtrl::TreeNode::~TreeNode()
{
	TreeNode* pNode = pChild;
	while (pNode != NULL)
	{
		TreeNode* pNext = pNode->pNext;
		delete pNode;
		pNode = pNext;
	}
}

BEGIN_MESSAGE_MAP(CMyTreeCtrl, CWnd)
	ON_WM_PAINT()
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
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_MESSAGE_VOID(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_STYLECHANGED()
END_MESSAGE_MAP()


// CMyTreeCtrl message handlers

BOOL CMyTreeCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs))
		return false;

	static CString strWndClass = AfxRegisterWndClass(CS_DBLCLKS,
			::LoadCursor(NULL, IDC_ARROW));

	cs.style |= WS_HSCROLL | WS_VSCROLL;
	cs.lpszClass = strWndClass;

	return true;
}

void CMyTreeCtrl::OnPaint()
{
	CPaintDC dcPaint(this);
	dcPaint.SetViewportOrg(CPoint(0, 0));

	CRect rcClient;
	GetClientRect(rcClient);

	CRect rcClip;
	dcPaint.GetClipBox(rcClip);
	rcClip.IntersectRect(rcClip, rcClient);

	m_offscreenDC.Create(&dcPaint, rcClip.Size());
	m_offscreenDC.SetViewportOrg(-rcClip.TopLeft());
	m_offscreenDC.IntersectClipRect(rcClip);

	CPoint ptOffset = GetScrollPosition();

	CFont* pOldFont = m_offscreenDC.SelectObject(&m_font);
	COLORREF crBackground = ::GetSysColor(COLOR_WINDOW);
	m_offscreenDC.SetBkColor(crBackground);

	int nBottom = PaintNode(&m_offscreenDC, m_pRoot, rcClip);

	if (nBottom < m_szDisplay.cy)
	{
		CRect rcBottom(0, nBottom, m_szDisplay.cx, m_szDisplay.cy);
		m_offscreenDC.FillSolidRect(rcBottom - ptOffset, crBackground);
	}

	m_offscreenDC.SelectObject(pOldFont);

	dcPaint.BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(),
			&m_offscreenDC, rcClip.left, rcClip.top, SRCCOPY);

	m_offscreenDC.Release();
}

int CMyTreeCtrl::PaintNode(CDC* pDC, TreeNode* pNode, const CRect& rcClip)
{
	CPoint ptOffset = GetScrollPosition();

	int nBottom = pNode->rcNode.bottom;

	CRect rcLine(pNode->rcNode);
	rcLine.right = m_szDisplay.cx;

	CRect rcIntersect;
	if (pNode != m_pRoot && rcIntersect.IntersectRect(rcClip, rcLine - ptOffset))
	{
		COLORREF crBackground = ::GetSysColor(COLOR_WINDOW);
		pDC->FillSolidRect(rcLine - ptOffset, crBackground);

		CFont* pOldFont = NULL;
		if (pNode == m_pHoverNode)
			pOldFont = pDC->SelectObject(&m_fontHover);

		UINT nFlags = DT_LEFT | DT_NOPREFIX | DT_TOP | (m_bWrapLabels ? DT_WORDBREAK : DT_SINGLELINE);
		if (pNode == m_pSelection)
		{
			CRect rcSel(pNode->rcLabel);
			rcSel.DeflateRect(s_nTextOffset - 3, 0, 0, 0);

			COLORREF crSel = ::GetSysColor(GetFocus() == this ? COLOR_HIGHLIGHT : COLOR_BTNFACE);
			pDC->FillSolidRect(rcSel - ptOffset, crSel);

			COLORREF crSelText = ::GetSysColor(GetFocus() == this ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT);
			pDC->SetTextColor(crSelText);

			pDC->DrawText(pNode->strLabel, pNode->rcText - ptOffset, nFlags);

			if (GetFocus() == this)
				DrawDottedRect(pDC, rcSel - ptOffset, crSelText);
		}
		else
		{
			COLORREF crText = ::GetSysColor(pNode == m_pHoverNode ? COLOR_HOTLIGHT : COLOR_WINDOWTEXT);
			pDC->SetTextColor(crText);
			pDC->DrawText(pNode->strLabel, pNode->rcText - ptOffset, DT_LEFT | DT_NOPREFIX | DT_WORDBREAK);
		}

		if (pNode == m_pHoverNode)
			pDC->SelectObject(pOldFont);

		if (m_pImageList != NULL && pNode->nImage != -1)
		{
			m_pImageList->Draw(pDC, pNode == m_pSelection ? pNode->nSelectedImage : pNode->nImage,
					pNode->rcImage.TopLeft() + (-ptOffset), ILD_NORMAL);
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
				DrawDottedLine(pDC, CPoint(pNode->nLineX, pNode->rcNode.top) + (-ptOffset),
					CPoint(pNode->nLineX, pNode->nLineY) + (-ptOffset), crLines);
			}

			if (pNode->HasSibling())
			{
				// This node is not the last child, so there should be a vertical bottom half-line here
				DrawDottedLine(pDC,	CPoint(pNode->nLineX, pNode->nLineY) + (-ptOffset),
					CPoint(pNode->nLineX, pNode->rcNode.bottom) + (-ptOffset), crLines);
			}

			// Horizontal line
			DrawDottedLine(pDC, CPoint(pNode->nLineX, pNode->nLineY) + (-ptOffset),
				CPoint(pNode->nLineStopX, pNode->nLineY) + (-ptOffset), crLines);

			if (pNode->HasChildren() && !pNode->bCollapsed)
			{
				// Vertical line to the first child
				DrawDottedLine(pDC, CPoint(pNode->pChild->nLineX, pNode->nLineY) + (-ptOffset),
					CPoint(pNode->pChild->nLineX, pNode->rcNode.bottom) + (-ptOffset), crLines);
			}

			TreeNode* pParent = pNode->pParent;
			while (pParent != m_pRoot)
			{
				if (pParent->HasSibling())
				{
					// This node has a sibling node, so there should be a full vertical line here
					DrawDottedLine(pDC, CPoint(pParent->nLineX, pNode->rcNode.top) + (-ptOffset),
						CPoint(pParent->nLineX, pNode->rcNode.bottom) + (-ptOffset), crLines);
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

				XPDrawThemeBackground(m_hTheme, pDC->m_hDC, TVP_GLYPH, iGlyph, pNode->rcGlyph - ptOffset, NULL);
			}
			else
			{
				pDC->FillSolidRect(pNode->rcGlyph - ptOffset, crBackground);
				FrameRect(pDC, pNode->rcGlyph - ptOffset, ::GetSysColor(COLOR_BTNSHADOW));

				CRect rcHorz(pNode->rcGlyph.left + 2, pNode->rcGlyph.CenterPoint().y,
					pNode->rcGlyph.right - 2, pNode->rcGlyph.CenterPoint().y + 1);
				pDC->FillSolidRect(rcHorz - ptOffset, RGB(0, 0, 0));

				if (pNode->bCollapsed)
				{
					CRect rcVert(pNode->rcGlyph.CenterPoint().x, pNode->rcGlyph.top + 2,
						pNode->rcGlyph.CenterPoint().x + 1, pNode->rcGlyph.bottom - 2);
					pDC->FillSolidRect(rcVert - ptOffset, RGB(0, 0, 0));
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

void CMyTreeCtrl::SetItemHeight(int nHeight)
{
	m_nItemHeight = nHeight;
	RecalcLayout();
}

void CMyTreeCtrl::SetImageList(CImageList* pImageList, DWORD dwStyle)
{
	m_pImageList = pImageList;
	RecalcLayout();
}

void CMyTreeCtrl::SetWrapLabels(bool bWrapLabels)
{
	if (m_bWrapLabels != bWrapLabels)
	{
		m_bWrapLabels = bWrapLabels;
		RecalcLayout();
	}
}

void CMyTreeCtrl::SetRedirectWheel(bool bRedirectWheel)
{
	m_bRedirectWheel = bRedirectWheel;
}

CPoint CMyTreeCtrl::GetScrollPosition()
{
	return CPoint(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
}

HTREEITEM CMyTreeCtrl::InsertItem(LPCTSTR pszItem, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	return InsertItem(pszItem, -1, -1, hParent, hInsertAfter);
}

HTREEITEM CMyTreeCtrl::InsertItem(LPCTSTR pszItem, int nImage, int nSelectedImage, HTREEITEM hParent, HTREEITEM hInsertAfter)
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

bool CMyTreeCtrl::DeleteItem(HTREEITEM hItem)
{
	if (hItem == TVI_ROOT)
		return false;

	TreeNode* pNode = reinterpret_cast<TreeNode*>(hItem);
	ASSERT_POINTER(pNode, TreeNode);

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

	if (!m_bBatchUpdate)
		RecalcLayout();

	return true;
}

void CMyTreeCtrl::BeginBatchUpdate()
{
	m_bBatchUpdate = true;
}

void CMyTreeCtrl::EndBatchUpdate()
{
	m_bBatchUpdate = false;

	RecalcLayout();

	if (m_pSelection != NULL && m_pSelection->nIndex == -1)
		SelectNode(NULL);
}

void CMyTreeCtrl::RecalcLayout()
{
	CRect rcClient;
	GetClientRect(rcClient);

	if ((GetStyle() & WS_VSCROLL) != 0)
		rcClient.right += ::GetSystemMetrics(SM_CXVSCROLL);
	if ((GetStyle() & WS_HSCROLL) != 0)
		rcClient.bottom += ::GetSystemMetrics(SM_CYHSCROLL);

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

	bool bVertScroll = false;
	bool bHorzScroll = false;
	bool bContinue;

	do
	{
		int nTop = 0;
		int nLevel = 0;
		stack<pair<TreeNode*, bool> > s;
		s.push(make_pair(m_pRoot, true));

		m_items.clear();

		bContinue = false;
		m_szDisplay = CSize(0, 0);
		while (!s.empty())
		{
			TreeNode* pNode = s.top().first;
			if (pNode != m_pRoot && s.top().second)
			{
				pNode->nIndex = m_items.size();
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
					pNode->rcText.right = max(pNode->rcText.right, rcClient.right - s_nOffsetRight);
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

		if (m_szDisplay.cx > rcClient.Width() && !bHorzScroll)
		{
			bHorzScroll = true;
			rcClient.bottom -= ::GetSystemMetrics(SM_CYHSCROLL);
		}
		if (m_szDisplay.cy > rcClient.Height() && !bVertScroll)
		{
			bVertScroll = true;
			bContinue = true;
			rcClient.right -= ::GetSystemMetrics(SM_CXVSCROLL);
		}
	} while (bContinue);

	ShowScrollBar(SB_HORZ, bHorzScroll);
	ShowScrollBar(SB_VERT, bVertScroll);

	m_szDisplay.cx = max(m_szDisplay.cx, rcClient.Width());
	m_szDisplay.cy = max(m_szDisplay.cy, rcClient.Height());
	m_szPage = CSize(rcClient.Width()*3/4, rcClient.Height()*3/4);

	dc.SelectObject(pOldFont);

	// this structure needed to update the scrollbar page range
	SCROLLINFO info;
	info.fMask = SIF_PAGE | SIF_RANGE;
	info.nMin = 0;

	// now update the bars as appropriate
	if (bHorzScroll)
	{
		info.nPage = rcClient.Width();
		info.nMax = m_szDisplay.cx - 1;
		if (!SetScrollInfo(SB_HORZ, &info, true))
			SetScrollRange(SB_HORZ, 0, m_szDisplay.cx - rcClient.Width(), true);

		int nPos = max(0, min(m_szDisplay.cx - rcClient.Width(), GetScrollPos(SB_HORZ)));
		SetScrollPos(SB_HORZ, nPos, true);
	}
	else
	{
		SetScrollPos(SB_HORZ, 0, false);
	}

	if (bVertScroll)
	{
		info.nPage = rcClient.Height();
		info.nMax = m_szDisplay.cy - 1;
		if (!SetScrollInfo(SB_VERT, &info, true))
			SetScrollRange(SB_VERT, 0, m_szDisplay.cy - rcClient.Height(), true);

		int nPos = max(0, min(m_szDisplay.cy - rcClient.Height(), GetScrollPos(SB_VERT)));
		SetScrollPos(SB_VERT, nPos, true);
	}
	else
	{
		SetScrollPos(SB_VERT, 0, false);
	}

	SetHoverNode(NULL);

	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);
	UpdateHoverNode(ptCursor);

	Invalidate();
}

int CMyTreeCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_bHasLines = (lpCreateStruct->style & TVS_HASLINES) != 0;
	m_bLinesAtRoot = (lpCreateStruct->style & TVS_LINESATROOT) != 0;
	m_bHasGlyphs = (lpCreateStruct->style & TVS_HASBUTTONS) != 0;

	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (IsThemed())
		m_hTheme = XPOpenThemeData(m_hWnd, L"TREEVIEW");

	m_toolTip.Create(this);

	return 0;
}

void CMyTreeCtrl::OnDestroy()
{
	if (m_hTheme != NULL)
	{
		XPCloseThemeData(m_hTheme);
		m_hTheme = NULL;
	}

	m_toolTip.DestroyWindow();

	CWnd::OnDestroy();
}

void CMyTreeCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	RecalcLayout();
}

BOOL CMyTreeCtrl::OnEraseBkgnd(CDC* pDC) 
{
	return true;
}

void CMyTreeCtrl::OnThemeChanged()
{
	if (m_hTheme != NULL)
		XPCloseThemeData(m_hTheme);

	m_hTheme = NULL;

	if (IsThemed())
		m_hTheme = XPOpenThemeData(m_hWnd, L"TREEVIEW");

	m_font.DeleteObject();
	CreateSystemIconFont(m_font);

	RecalcLayout();
	Invalidate();
}

void CMyTreeCtrl::OnSysColorChange()
{
	Invalidate();
}

void CMyTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	int nArea;
	TreeNode* pNode = HitTest(point, &nArea);

	if (pNode != NULL)
	{
		if (nArea == HT_IMAGE || nArea == HT_LABEL)
		{
			SelectNode(pNode, TVC_BYMOUSE);
		}
		else if (nArea == HT_GLYPH)
		{
			if (pNode->HasChildren())
			{
				ToggleNode(pNode);
			}
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CMyTreeCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	int nArea;
	TreeNode* pNode = HitTest(point, &nArea);

	if (pNode != NULL)
	{
		if (nArea == HT_IMAGE || nArea == HT_LABEL)
		{
			SelectNode(pNode, TVC_UNKNOWN);
		}
	}

	CWnd::OnRButtonDown(nFlags, point);
}

CMyTreeCtrl::TreeNode* CMyTreeCtrl::HitTest(CPoint point, int* pnArea)
{
	return HitTest(m_pRoot, point, pnArea);
}

CMyTreeCtrl::TreeNode* CMyTreeCtrl::HitTest(TreeNode* pNode, CPoint point, int* pnArea)
{
	CPoint ptOffset = GetScrollPosition();

	if (pNode != m_pRoot && pNode->rcNode.PtInRect(point + ptOffset))
	{
		if (pNode->rcLabel.PtInRect(point + ptOffset))
			*pnArea = HT_LABEL;
		else if (pNode->rcImage.PtInRect(point + ptOffset))
			*pnArea = HT_IMAGE;
		else if (pNode->rcGlyph.PtInRect(point + ptOffset))
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

void CMyTreeCtrl::InvalidateNode(TreeNode* pNode)
{
	CPoint ptOffset = GetScrollPosition();
	InvalidateRect(pNode->rcNode + (-ptOffset));
}

void CMyTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	bool bDragging = (nFlags & (MK_MBUTTON | MK_LBUTTON | MK_RBUTTON)) != 0;
	if (bDragging)
		return;

	UpdateHoverNode(point);

	CWnd::OnMouseMove(nFlags, point);
}

void CMyTreeCtrl::OnMouseLeave()
{
	if (!m_bMouseInTooltip)
		SetHoverNode(NULL);
}

void CMyTreeCtrl::UpdateHoverNode(const CPoint& point)
{
	CRect rcClient;
	GetClientRect(rcClient);

	if (!rcClient.PtInRect(point))
	{
		SetHoverNode(NULL);
		return;
	}

	int nArea;
	TreeNode* pNode = HitTest(point, &nArea);

	if (pNode != NULL && nArea != HT_IMAGE && nArea != HT_LABEL)
		pNode = NULL;

	TreeNode* pOldHoverNode = m_pHoverNode;
	SetHoverNode(pNode);

	if (pNode != NULL && pOldHoverNode != pNode)
	{
		TRACKMOUSEEVENT tme;

		::ZeroMemory(&tme, sizeof(tme));
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		tme.dwHoverTime = HOVER_DEFAULT;

		::TrackMouseEvent(&tme);
	}
}

void CMyTreeCtrl::SetHoverNode(TreeNode* pNode)
{
	if (pNode != m_pHoverNode)
	{
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

			CPoint ptOffset = GetScrollPosition();
			CRect rcLabel = pNode->rcLabel - ptOffset;

			CRect rcClient, rcIntersect;
			GetClientRect(rcClient);

			if (rcIntersect.IntersectRect(rcLabel, rcClient) && rcIntersect != rcLabel)
			{
				CPoint ptTooltip = pNode->rcLabel.TopLeft() + (-ptOffset);
				ptTooltip.Offset(s_nTextOffset - 3, 0);

				CRect rcTooltip(pNode->rcLabel);
				rcTooltip.DeflateRect(s_nTextOffset - 3, 0, 0, 0);

				CRect rcTooltipText(pNode->rcText - rcTooltip.TopLeft());
				rcTooltip -= ptOffset;
				ClientToScreen(rcTooltip);

				m_bMouseInTooltip = true;
				++m_toolTip.m_nNextCode;

				m_toolTip.Show(pNode->strLabel, m_bWrapLabels, rcTooltip, rcTooltipText);
			}
		}

		GetTopLevelParent()->UpdateWindow();
	}
}

void CMyTreeCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	if (m_pSelection != NULL)
	{
		InvalidateNode(m_pSelection);
		UpdateWindow();
	}
}

void CMyTreeCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);

	SetHoverNode(NULL);

	if (m_pSelection != NULL)
	{
		InvalidateNode(m_pSelection);
		UpdateWindow();
	}
}

BOOL CMyTreeCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (hCursorLink == NULL)
		hCursorLink = ::LoadCursor(0, IDC_HAND);
	if (hCursorLink == NULL)
		hCursorLink = AfxGetApp()->LoadCursor(IDC_CURSOR_LINK);

	if (nHitTest == HTCLIENT && m_pHoverNode != NULL)
	{
		SetCursor(hCursorLink);
		return true;
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CMyTreeCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return;     // eat it

	// ignore scroll bar msgs from other controls
	if (pScrollBar != GetScrollBarCtrl(SB_HORZ))
		return;

	OnScroll(MAKEWORD(nSBCode, -1), nPos);
}

void CMyTreeCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return;     // eat it

	// ignore scroll bar msgs from other controls
	if (pScrollBar != GetScrollBarCtrl(SB_VERT))
		return;

	OnScroll(MAKEWORD(-1, nSBCode), nPos);
}

bool CMyTreeCtrl::OnScroll(UINT nScrollCode, UINT nPos)
{
	// calc new x position
	int x = GetScrollPos(SB_HORZ);
	int xOrig = x;

	switch (LOBYTE(nScrollCode))
	{
	case SB_TOP:
		x = 0;
		break;
	case SB_BOTTOM:
		x = INT_MAX;
		break;
	case SB_LINEUP:
		x -= m_szLine.cx;
		break;
	case SB_LINEDOWN:
		x += m_szLine.cx;
		break;
	case SB_PAGEUP:
		x -= m_szPage.cx;
		break;
	case SB_PAGEDOWN:
		x += m_szPage.cx;
		break;
	case SB_THUMBTRACK:
		{
			SCROLLINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cbSize = sizeof(si);

			if (!GetScrollInfo(SB_HORZ, &si, SIF_TRACKPOS))
				return true;

			x = si.nTrackPos;
		}
		break;
	}

	// calc new y position
	int y = GetScrollPos(SB_VERT);
	int yOrig = y;

	switch (HIBYTE(nScrollCode))
	{
	case SB_TOP:
		y = 0;
		break;
	case SB_BOTTOM:
		y = INT_MAX;
		break;
	case SB_LINEUP:
		y -= m_szLine.cy;
		break;
	case SB_LINEDOWN:
		y += m_szLine.cy;
		break;
	case SB_PAGEUP:
		y -= m_szPage.cy;
		break;
	case SB_PAGEDOWN:
		y += m_szPage.cy;
		break;
	case SB_THUMBTRACK:
		{
			SCROLLINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cbSize = sizeof(si);

			if (!GetScrollInfo(SB_VERT, &si, SIF_TRACKPOS))
				return true;

			y = si.nTrackPos;
		}
		break;
	}

	return OnScrollBy(CSize(x - xOrig, y - yOrig));
}

bool CMyTreeCtrl::OnScrollBy(CSize sz)
{
	DWORD dwStyle = GetStyle();

	int xOrig = GetScrollPos(SB_HORZ);
	int yOrig = GetScrollPos(SB_VERT);
	int x = xOrig + sz.cx;
	int y = yOrig + sz.cy;

	CScrollBar* pBar = GetScrollBarCtrl(SB_HORZ);
	if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
		(pBar == NULL && !(dwStyle & WS_HSCROLL)))
	{
		// horizontal scroll bar not enabled
		x = xOrig;
	}
	else
	{
		// adjust current x position
		int xMax = GetScrollLimit(SB_HORZ);
		x = min(xMax, max(0, x));
	}

	pBar = GetScrollBarCtrl(SB_VERT);
	if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
		(pBar == NULL && !(dwStyle & WS_VSCROLL)))
	{
		// horizontal scroll bar not enabled
		y = yOrig;
	}
	else
	{
		// adjust current y position
		int yMax = GetScrollLimit(SB_VERT);
		y = min(yMax, max(0, y));
	}

	if (x != xOrig || y != yOrig)
	{
		ScrollWindow(xOrig - x, yOrig - y);
		if (x != xOrig)
			SetScrollPos(SB_HORZ, x);
		if (y != yOrig)
			SetScrollPos(SB_VERT, y);

		SetHoverNode(NULL);

		CPoint ptCursor;
		::GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);
		UpdateHoverNode(ptCursor);

		UpdateWindow();
		return true;
	}

	return false;
}

void CMyTreeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMyTreeCtrl::SelectItem(HTREEITEM hItem)
{
	if (hItem == TVI_ROOT)
	{
		SelectNode(NULL);
		return;
	}

	TreeNode* pNode = (TreeNode*)hItem;
	SelectNode(pNode);
}

void CMyTreeCtrl::SelectNode(TreeNode* pNode, UINT nAction)
{
	if (m_pSelection != pNode)
	{
		TreeNode* pOldSel = m_pSelection;

		if (m_pSelection != NULL)
			InvalidateNode(m_pSelection);
		if (pNode != NULL)
			InvalidateNode(pNode);

		m_pSelection = pNode;

		if (pNode != NULL)
			EnsureVisible(pNode);

		// Notify parent
		NMTREEVIEW nmtv;
		::ZeroMemory(&nmtv, sizeof(nmtv));

		nmtv.hdr.hwndFrom = GetSafeHwnd();
		nmtv.hdr.idFrom = GetDlgCtrlID();
		nmtv.hdr.code = TVN_SELCHANGED;
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
		::ZeroMemory(&nmtv, sizeof(nmtv));

		nmtv.hdr.hwndFrom = GetSafeHwnd();
		nmtv.hdr.idFrom = GetDlgCtrlID();
		nmtv.hdr.code = TVN_ITEMCLICKED;
		nmtv.action = nAction;

		nmtv.itemNew.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
		nmtv.itemNew.hItem = (HTREEITEM) pNode;
		nmtv.itemNew.state = TVIS_SELECTED | (!pNode->bCollapsed ? TVIS_EXPANDED : 0);
		nmtv.itemNew.lParam = (LPARAM) pNode->dwUserData;

		GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM) &nmtv);
	}

	UpdateWindow();
}

void CMyTreeCtrl::ExpandNode(TreeNode* pNode, bool bExpand)
{
	pNode->bCollapsed = !bExpand;

	if (!m_bBatchUpdate)
	{
		RecalcLayout();

		if (m_pSelection != NULL && m_pSelection->nIndex == -1)
			SelectNode(pNode);
	}
}

void CMyTreeCtrl::ToggleNode(TreeNode* pNode)
{
	ExpandNode(pNode, pNode->bCollapsed);
}

bool CMyTreeCtrl::Expand(HTREEITEM hItem, UINT nCode)
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

CMyTreeCtrl::TreeNode* CMyTreeCtrl::FindNextNode(TreeNode* pNode)
{
	if (pNode == NULL)
		return (m_items.empty() ? NULL : m_items.front());

	if (pNode->nIndex == -1 || pNode->nIndex + 1 == m_items.size())
		return NULL;

	return m_items[pNode->nIndex + 1];
}

CMyTreeCtrl::TreeNode* CMyTreeCtrl::FindPrevNode(TreeNode* pNode)
{
	if (pNode == NULL)
		return (m_items.empty() ? NULL : m_items.back());

	if (pNode->nIndex == -1 || pNode->nIndex == 0)
		return NULL;

	return m_items[pNode->nIndex - 1];
}

CMyTreeCtrl::TreeNode* CMyTreeCtrl::FindNextPageNode(TreeNode* pNode)
{
	if (pNode == NULL)
		return (m_items.empty() ? NULL : m_items.front());

	if (pNode->nIndex == -1 || pNode->nIndex + 1 == m_items.size())
		return NULL;

	CRect rcClient;
	GetClientRect(rcClient);

	int nIndex = pNode->nIndex + 1;
	while (nIndex + 1 < static_cast<int>(m_items.size()) &&
			m_items[nIndex + 1]->rcNode.bottom <= pNode->rcNode.top + rcClient.Height())
		++nIndex;

	return m_items[nIndex];
}

CMyTreeCtrl::TreeNode* CMyTreeCtrl::FindPrevPageNode(TreeNode* pNode)
{
	if (pNode == NULL)
		return (m_items.empty() ? NULL : m_items.back());

	if (pNode->nIndex == -1 || pNode->nIndex == 0)
		return NULL;

	CRect rcClient;
	GetClientRect(rcClient);

	int nIndex = pNode->nIndex - 1;
	while (nIndex >= 1 && m_items[nIndex - 1]->rcNode.top >= pNode->rcNode.bottom - rcClient.Height())
		--nIndex;

	return m_items[nIndex];
}

void CMyTreeCtrl::SetItemData(HTREEITEM hItem, DWORD_PTR dwData)
{
	TreeNode* pNode = (TreeNode*) hItem;
	pNode->dwUserData = dwData;
}

DWORD_PTR CMyTreeCtrl::GetItemData(HTREEITEM hItem)
{
	TreeNode* pNode = (TreeNode*) hItem;
	return pNode->dwUserData;
}

HTREEITEM CMyTreeCtrl::GetSelectedItem()
{
	return (HTREEITEM) m_pSelection;
}

BOOL CMyTreeCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	if (m_bRedirectWheel)
	{
		CWnd* pWnd = WindowFromPoint(point);
		if (pWnd != this && !IsChild(pWnd) && IsFromCurrentProcess(pWnd) &&
				pWnd->SendMessage(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(point.x, point.y)) != 0)
			return true;
	}

	bool bHasHorzBar, bHasVertBar;
	CheckScrollBars(bHasHorzBar, bHasVertBar);

	if (!bHasVertBar && !bHasHorzBar)
		return false;

	UINT uWheelScrollLines = GetMouseScrollLines();
	int nToScroll = ::MulDiv(-zDelta, uWheelScrollLines, WHEEL_DELTA);
	int nDisplacement;

	if (bHasVertBar && (!bHasHorzBar || (nFlags & MK_SHIFT) == 0))
	{
		if (uWheelScrollLines == WHEEL_PAGESCROLL)
		{
			nDisplacement = m_szPage.cy;
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
		}
		else
		{
			nDisplacement = nToScroll * m_szLine.cy;
			nDisplacement = min(nDisplacement, m_szPage.cy);
		}

		OnScrollBy(CSize(0, nDisplacement));
	}
	else if (bHasHorzBar)
	{
		if (uWheelScrollLines == WHEEL_PAGESCROLL)
		{
			nDisplacement = m_szPage.cx;
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
		}
		else
		{
			nDisplacement = nToScroll * m_szLine.cx;
			nDisplacement = min(nDisplacement, m_szPage.cx);
		}

		OnScrollBy(CSize(nDisplacement, 0));
	}

	return true;
}

void CMyTreeCtrl::CheckScrollBars(bool& bHasHorzBar, bool& bHasVertBar) const
{
	DWORD dwStyle = GetStyle();
	CScrollBar* pBar = GetScrollBarCtrl(SB_VERT);
	bHasVertBar = (pBar != NULL && pBar->IsWindowEnabled())
			|| (dwStyle & WS_VSCROLL) != 0;

	pBar = GetScrollBarCtrl(SB_HORZ);
	bHasHorzBar = (pBar != NULL && pBar->IsWindowEnabled())
			|| (dwStyle & WS_HSCROLL) != 0;
}

void CMyTreeCtrl::EnsureVisible(TreeNode* pNode)
{
	CPoint ptOffset = GetScrollPosition();
	CRect rcNode = pNode->rcNode - ptOffset;

	CRect rcClient;
	GetClientRect(rcClient);

	int nScroll = 0;
	if (rcNode.bottom > rcClient.bottom)
		nScroll = rcNode.bottom - rcClient.bottom;
	if (rcNode.top - nScroll < rcClient.top)
		nScroll = rcNode.top - rcClient.top;

	OnScrollBy(CSize(0, nScroll));
}

void CMyTreeCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	int nArea;
	TreeNode* pNode = HitTest(point, &nArea);

	if (pNode != NULL && (nArea == HT_IMAGE || nArea == HT_LABEL) && pNode->HasChildren())
	{
		ToggleNode(pNode);
		return;
	}

	CWnd::OnLButtonDblClk(nFlags, point);
}

bool CMyTreeCtrl::UpdateParameters()
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

void CMyTreeCtrl::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	if (UpdateParameters())
	{
		RecalcLayout();
	}
}

BOOL CMyTreeCtrl::CTreeToolTip::Create(CMyTreeCtrl* pTree)
{
	m_pTree = pTree;

	static CString strWndClass = AfxRegisterWndClass(CS_DBLCLKS,
			::LoadCursor(NULL, IDC_ARROW));

	return CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, strWndClass, NULL, WS_POPUP,
		CRect(0, 0, 0, 0), pTree, 0);
}

void CMyTreeCtrl::CTreeToolTip::Show(const CString& strText,
	bool bWrap, const CRect& rcWindow, const CRect& rcText)
{
	m_strText = strText;
	m_bWrap = bWrap;
	m_rcText = rcText;

	MoveWindow(rcWindow.left, rcWindow.top, rcWindow.Width(), rcWindow.Height());
	ShowWindow(SW_SHOWNOACTIVATE);
	SetWindowPos(&wndTopMost, 0, 0, 0, 0,
			SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER);

	Invalidate();
	UpdateWindow();
}

void CMyTreeCtrl::CTreeToolTip::Hide()
{
	ShowWindow(SW_HIDE);
}

BOOL CMyTreeCtrl::CTreeToolTip::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
		{
			TRACKMOUSEEVENT tme;

			::ZeroMemory(&tme, sizeof(tme));
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
			Hide();

			CPoint ptCursor;
			::GetCursorPos(&ptCursor);
			m_pTree->ScreenToClient(&ptCursor);

			// GetAsyncKeyState always returns state of physical buttons, even
			// if they are reversed. We only want to check if any of the mouse
			// buttons is pressed, so there are no extra checks here
			bool bLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
			bool bRightDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
			bool bMiddleDown = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;

			if (!bLeftDown && !bRightDown && !bMiddleDown)
				m_pTree->UpdateHoverNode(ptCursor);
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

			CRect rcClient;
			GetClientRect(rcClient);

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
