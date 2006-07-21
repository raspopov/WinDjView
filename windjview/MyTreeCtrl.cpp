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

#include "stdafx.h"
#include "WinDjView.h"
#include "MyTreeCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


int s_nOffsetLeft = 3;
int s_nTextOffset = 5;
int s_nOffsetRight = 2;


// CMyTreeCtrl control

IMPLEMENT_DYNAMIC(CMyTreeCtrl, CWnd)

CMyTreeCtrl::CMyTreeCtrl()
	: m_nItemHeight(20), m_pImageList(NULL), m_bWrapLabels(true), m_hTheme(NULL),
	  m_pOffscreenBitmap(NULL), m_szOffscreen(0, 0), m_pSelection(NULL), m_pHoverNode(NULL),
	  m_ptScrollOffset(0, 0), m_szDisplay(0, 0), m_szLine(15, 15), m_szPage(0, 0),
	  m_bMouseInTooltip(false)
{
	m_pRoot = new TreeNode(NULL, -1, -1, NULL);
	m_pRoot->bCollapsed = false;

	CreateSystemDialogFont(m_font);

	LOGFONT lf;
	::GetObject(m_font.m_hObject, sizeof(LOGFONT), &lf);

	lf.lfUnderline = true;
	m_fontHover.CreateFontIndirect(&lf);

	m_toolTip.SetTree(this);
}

CMyTreeCtrl::~CMyTreeCtrl()
{
	delete m_pRoot;
	delete m_pOffscreenBitmap;
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
	ON_MESSAGE(WM_THEMECHANGED, OnThemeChanged)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNeedText)
	ON_WM_SETCURSOR()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_MESSAGE_VOID(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()


// CMyTreeCtrl message handlers

BOOL CMyTreeCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs))
		return false;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style |= WS_HSCROLL | WS_VSCROLL;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW + 1), NULL);

	return true;
}

void CMyTreeCtrl::OnPaint()
{
	CPaintDC dcPaint(this);
	CPoint ptOffset(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	CRect rcClient;
	GetClientRect(rcClient);
	if (m_pOffscreenBitmap == NULL
			|| m_szOffscreen.cx < rcClient.Width() || m_szOffscreen.cy < rcClient.Height())
	{
		m_szOffscreen.cx = max(m_szOffscreen.cx, static_cast<int>(rcClient.Width()*1.1));
		m_szOffscreen.cy = max(m_szOffscreen.cy, static_cast<int>(rcClient.Height()*1.1));

		delete m_pOffscreenBitmap;
		m_pOffscreenBitmap = new CBitmap();
		m_pOffscreenBitmap->CreateCompatibleBitmap(&dcPaint, m_szOffscreen.cx, m_szOffscreen.cy);
	}

	CDC dc;
	dc.CreateCompatibleDC(&dcPaint);
	CBitmap* pOldBitmap = dc.SelectObject(m_pOffscreenBitmap);

	CRect rcClip;
	dcPaint.GetClipBox(rcClip);
	rcClip.IntersectRect(rcClip, rcClient);
	dc.IntersectClipRect(rcClip);

	CFont* pOldFont = dc.SelectObject(&m_font);
	dc.SetBkColor(RGB(255, 255, 255));

	int nBottom = PaintNode(&dc, m_pRoot);

	if (nBottom < m_szDisplay.cy)
	{
		CRect rcBottom(0, nBottom, m_szDisplay.cx, m_szDisplay.cy);
		dc.FillSolidRect(rcBottom - ptOffset, RGB(255, 255, 255));
	}

	dc.SelectObject(pOldFont);

	dcPaint.BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(),
			&dc, rcClip.left, rcClip.top, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

int CMyTreeCtrl::PaintNode(CDC* pDC, TreeNode* pNode)
{
	CPoint ptOffset(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	int nBottom = pNode->rcNode.bottom;

	if (pNode != m_pRoot)
	{
		CRect rcLine(pNode->rcNode);
		rcLine.right = m_szDisplay.cx;

		pDC->FillSolidRect(rcLine - ptOffset, RGB(255, 255, 255));

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
			{
				LOGBRUSH brush;
				brush.lbColor = crSelText;
				brush.lbStyle = BS_SOLID;

				CPen pen(PS_COSMETIC | PS_ALTERNATE, 1, &brush, 0, NULL);
				CPen* pOldPen = pDC->SelectObject(&pen);

				pDC->MoveTo(rcSel.TopLeft() + (-ptOffset));
				pDC->LineTo(CPoint(rcSel.right, rcSel.top) + (-ptOffset));
				pDC->MoveTo(CPoint(rcSel.right - 1, rcSel.top + (rcSel.Width() + 1) % 2) + (-ptOffset));
				pDC->LineTo(CPoint(rcSel.right - 1, rcSel.bottom) + (-ptOffset));
				pDC->MoveTo(rcSel.TopLeft() + (-ptOffset));
				pDC->LineTo(CPoint(rcSel.left, rcSel.bottom) + (-ptOffset));
				pDC->MoveTo(CPoint(rcSel.left + 1, rcSel.bottom - 1) + (-ptOffset));
				pDC->LineTo(CPoint(rcSel.right, rcSel.bottom - 1) + (-ptOffset));
			}
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

		LOGBRUSH brush;
		brush.lbColor = crLines;
		brush.lbStyle = BS_SOLID;

		CPen pen(PS_COSMETIC | PS_ALTERNATE, 1, &brush, 0, NULL);
		CPen* pOldPen = pDC->SelectObject(&pen);

		// Draw lines
		if (m_pRoot->pChild != pNode)
		{
			// This node is not the first child of the root, so there should be a vertical top half-line here
			pDC->MoveTo(CPoint(pNode->nLineX, pNode->rcNode.top) + (-ptOffset));
			pDC->LineTo(CPoint(pNode->nLineX, pNode->nLineY) + (-ptOffset));
		}

		if (pNode->HasSibling())
		{
			// This node is not the last child, so there should be a vertical bottom half-line here
			pDC->MoveTo(CPoint(pNode->nLineX, pNode->nLineY) + (-ptOffset));
			pDC->LineTo(CPoint(pNode->nLineX, pNode->rcNode.bottom) + (-ptOffset));
		}

		// Horizontal line
		pDC->MoveTo(CPoint(pNode->nLineX, pNode->nLineY) + (-ptOffset));
		pDC->LineTo(CPoint(pNode->nLineStopX, pNode->nLineY) + (-ptOffset));

		if (pNode->HasChildren() && !pNode->bCollapsed)
		{
			// Vertical line to the first child
			pDC->MoveTo(CPoint(pNode->pChild->nLineX, pNode->nLineY) + (-ptOffset));
			pDC->LineTo(CPoint(pNode->pChild->nLineX, pNode->rcNode.bottom) + (-ptOffset));
		}

		TreeNode* pParent = pNode->pParent;
		while (pParent != m_pRoot)
		{
			if (pParent->HasSibling())
			{
				// This node has a sibling node, so there should be a full vertical line here
				pDC->MoveTo(CPoint(pParent->nLineX, pNode->rcNode.top) + (-ptOffset));
				pDC->LineTo(CPoint(pParent->nLineX, pNode->rcNode.bottom) + (-ptOffset));
			}

			pParent = pParent->pParent;
		}

		pDC->SelectObject(pOldPen);

		if (pNode->HasChildren())
		{
			// Draw glyph
			if (m_hTheme != NULL)
			{
/*				COLORREF color;
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_BORDERCOLOR, &color);
				TRACE(_T("Color TMT_BORDERCOLOR: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_FILLCOLOR, &color);
				TRACE(_T("Color TMT_FILLCOLOR: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_TEXTCOLOR, &color);
				TRACE(_T("Color TMT_TEXTCOLOR: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_EDGELIGHTCOLOR, &color);
				TRACE(_T("Color TMT_EDGELIGHTCOLOR: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_EDGEHIGHLIGHTCOLOR, &color);
				TRACE(_T("Color TMT_EDGEHIGHLIGHTCOLOR: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_EDGESHADOWCOLOR, &color);
				TRACE(_T("Color TMT_EDGESHADOWCOLOR: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_EDGEDKSHADOWCOLOR, &color);
				TRACE(_T("Color TMT_EDGEDKSHADOWCOLOR: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_EDGEFILLCOLOR, &color);
				TRACE(_T("Color TMT_EDGEFILLCOLOR: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_TRANSPARENTCOLOR, &color);
				TRACE(_T("Color TMT_TRANSPARENTCOLOR: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_GRADIENTCOLOR1, &color);
				TRACE(_T("Color TMT_GRADIENTCOLOR1: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_GRADIENTCOLOR2, &color);
				TRACE(_T("Color TMT_GRADIENTCOLOR2: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_GRADIENTCOLOR3, &color);
				TRACE(_T("Color TMT_GRADIENTCOLOR3: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_GRADIENTCOLOR4, &color);
				TRACE(_T("Color TMT_GRADIENTCOLOR4: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_GRADIENTCOLOR5, &color);
				TRACE(_T("Color TMT_GRADIENTCOLOR5: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_SHADOWCOLOR, &color);
				TRACE(_T("Color TMT_SHADOWCOLOR: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_GLOWCOLOR, &color);
				TRACE(_T("Color TMT_GLOWCOLOR: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_TEXTBORDERCOLOR, &color);
				TRACE(_T("Color TMT_TEXTBORDERCOLOR: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_TEXTSHADOWCOLOR, &color);
				TRACE(_T("Color TMT_TEXTSHADOWCOLOR: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_GLYPHTEXTCOLOR, &color);
				TRACE(_T("Color TMT_GLYPHTEXTCOLOR: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_GLYPHTRANSPARENTCOLOR, &color);
				TRACE(_T("Color TMT_GLYPHTRANSPARENTCOLOR: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_FILLCOLORHINT, &color);
				TRACE(_T("Color TMT_FILLCOLORHINT: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_BORDERCOLORHINT, &color);
				TRACE(_T("Color TMT_BORDERCOLORHINT: %08x\n"), color);
				XPGetThemeColor(m_hTheme, TVP_BRANCH, 0, TMT_ACCENTCOLORHINT, &color);
				TRACE(_T("Color TMT_ACCENTCOLORHINT: %08x\n"), color);

				color = XPGetThemeSysColor(m_hTheme, TMT_BTNFACE);
				TRACE(_T("Color TMT_BTNFACE: %08x\n"), color);
				color = XPGetThemeSysColor(m_hTheme, TMT_BTNSHADOW);
				TRACE(_T("Color TMT_BTNSHADOW: %08x\n"), color);
				color = XPGetThemeSysColor(m_hTheme, TMT_HOTTRACKING);
				TRACE(_T("Color TMT_HOTTRACKING: %08x\n"), color);
*/
				int iGlyph = (pNode->bCollapsed ? GLPS_CLOSED : GLPS_OPENED);

				XPDrawThemeBackground(m_hTheme, pDC->m_hDC, TVP_GLYPH, iGlyph, pNode->rcGlyph - ptOffset, NULL);
			}
			else
			{
			}
		}
	}

	if (pNode->HasChildren() && !pNode->bCollapsed)
	{
		for (TreeNode* pChild = pNode->pChild; pChild != NULL; pChild = pChild->pNext)
			nBottom = PaintNode(pDC, pChild);
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
		pInsertAfter = pParent->pChild;
		while (pInsertAfter != NULL && pInsertAfter->pNext != NULL)
			pInsertAfter = pInsertAfter->pNext;
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

	RecalcLayout();

	return reinterpret_cast<HTREEITEM>(pNode);
}

void CMyTreeCtrl::RecalcLayout()
{
	CRect rcClient;
	GetClientRect(rcClient);

	if ((GetStyle() & WS_VSCROLL) != 0)
		rcClient.right += ::GetSystemMetrics(SM_CXVSCROLL);
	if ((GetStyle() & WS_HSCROLL) != 0)
		rcClient.bottom += ::GetSystemMetrics(SM_CYHSCROLL);

	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

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
		stack<TreeNode*> s;
		s.push(m_pRoot);

		bContinue = false;
		m_szDisplay = CSize(0, 0);
		while (!s.empty())
		{
			TreeNode* pNode = s.top();
			if (pNode != m_pRoot)
			{
				pNode->rcNode.left = s_nOffsetLeft + (nLevel - 1)*nChildOffset;
				pNode->rcNode.top = nTop;
				pNode->rcNode.bottom = nTop + m_nItemHeight;

				pNode->rcText.left = pNode->rcNode.left + nChildOffset;
				pNode->rcText.top = nTop + nTextOffsetTop;
				pNode->rcText.bottom = nTop + m_nItemHeight;

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
				pNode->rcText.right = max(rcClient.right - s_nOffsetRight, pNode->rcText.left + 1);

				UINT nFlags = DT_CALCRECT | DT_LEFT | DT_NOPREFIX | DT_TOP |
					(m_bWrapLabels ? DT_WORDBREAK : DT_SINGLELINE);
				int nHeight = dc.DrawText(pNode->strLabel, pNode->rcText, nFlags);
				pNode->rcText.bottom = pNode->rcText.top + nHeight;

				pNode->rcNode.left = 0;
				pNode->rcNode.right = pNode->rcText.right;
				pNode->rcNode.bottom = max(pNode->rcNode.bottom, pNode->rcText.bottom + 2);
				pNode->rcNode.bottom += (pNode->rcNode.bottom % 2);

				pNode->rcLabel = pNode->rcText;
				pNode->rcLabel.left -= s_nTextOffset;
				pNode->rcLabel.top = pNode->rcNode.top;
				pNode->rcLabel.bottom = pNode->rcNode.bottom;

				m_szDisplay.cx = max(m_szDisplay.cx, pNode->rcNode.Width());
				nTop = pNode->rcNode.bottom;
			}

			if (pNode->HasChildren() && !pNode->bCollapsed)
			{
				s.push(pNode->pChild);
				++nLevel;
			}
			else
			{
				while (!s.empty() && s.top()->pNext == NULL)
				{
					s.pop();
					--nLevel;
				}
				if (!s.empty())
					s.top() = s.top()->pNext;
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

	Invalidate();
}

int CMyTreeCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (XPIsAppThemed() && XPIsThemeActive())
		m_hTheme = XPOpenThemeData(m_hWnd, L"TREEVIEW");

	if (m_toolTip.Create(this, TTS_ALWAYSTIP | TTS_NOPREFIX))
	{
		TOOLINFO info;
		info.cbSize = sizeof(TOOLINFO);
		info.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_ABSOLUTE | TTF_TRACK/* | TTF_TRANSPARENT*/;
		info.hwnd = m_hWnd;
		info.uId = (UINT)m_hWnd;
		info.hinst = AfxGetInstanceHandle();
		info.lpszText = LPSTR_TEXTCALLBACK;
		info.rect = CRect(0, 0, 0, 0);
		m_toolTip.SendMessage(TTM_ADDTOOL, 0, (LPARAM)&info);

		m_toolTip.SetFont(&m_font);
		m_toolTip.Activate(true);
	}

	return 0;
}

void CMyTreeCtrl::OnDestroy()
{
	if (m_hTheme != NULL)
	{
		XPCloseThemeData(m_hTheme);
		m_hTheme = NULL;
	}

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

LRESULT CMyTreeCtrl::OnThemeChanged(WPARAM wParam, LPARAM lParam)
{
	if (m_hTheme != NULL)
		XPCloseThemeData(m_hTheme);

	m_hTheme = NULL;

	if (XPIsAppThemed() && XPIsThemeActive())
		m_hTheme = XPOpenThemeData(m_hWnd, L"TREEVIEW");

	return 0;
}

void CMyTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	int nArea;
	TreeNode* pNode = HitTest(point, &nArea);

	TreeNode* pNewSel = m_pSelection;

	if (pNode != NULL)
	{
		if (nArea == HT_IMAGE || nArea == HT_LABEL)
		{
			pNewSel = pNode;
		}
		else if (nArea == HT_GLYPH)
		{
			if (pNode->HasChildren())
			{
				ToggleNode(pNode);
			}
		}
	}

	SelectNode(pNewSel, TVC_BYMOUSE);

	CWnd::OnLButtonDown(nFlags, point);
}

CMyTreeCtrl::TreeNode* CMyTreeCtrl::HitTest(CPoint point, int* pnArea)
{
	return HitTest(m_pRoot, point, pnArea);
}

CMyTreeCtrl::TreeNode* CMyTreeCtrl::HitTest(TreeNode* pNode, CPoint point, int* pnArea)
{
	CPoint ptOffset(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

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
	CPoint ptOffset(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
	InvalidateRect(pNode->rcNode + (-ptOffset));
}

BOOL CMyTreeCtrl::OnToolTipNeedText(UINT nID, NMHDR* pNMHDR, LRESULT* pResult)
{
	TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pNMHDR;

	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	CRect rcClient;
	GetClientRect(rcClient);

	// Ensure that the cursor is in the client rect, because main frame
	// also wants these messages to provide tooltips for the toolbar
	if (!rcClient.PtInRect(ptCursor))
		return false;

	CPoint ptOffset(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	if (m_pHoverNode != NULL)
	{
		m_strToolTip = m_pHoverNode->strLabel;
		pTTT->lpszText = m_strToolTip.GetBuffer(0);

		m_toolTip.SetWindowPos(&wndTopMost, 0, 0, 0, 0,
				SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER);
	}
	else
	{
		pTTT->lpszText = pTTT->szText;
		pTTT->szText[0] = '\0';
	}

	return true;
}

BOOL CMyTreeCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (::IsWindow(m_toolTip.m_hWnd))
		m_toolTip.RelayEvent(pMsg);

	return CWnd::PreTranslateMessage(pMsg);
}

void CMyTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
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

	CWnd::OnMouseMove(nFlags, point);
}

void CMyTreeCtrl::OnMouseLeave()
{
	if (!m_bMouseInTooltip)
		SetHoverNode(NULL);
}

void CMyTreeCtrl::SetHoverNode(TreeNode* pNode)
{
	if (pNode != m_pHoverNode)
	{
		if (m_pHoverNode != NULL)
		{
			InvalidateNode(m_pHoverNode);

			TOOLINFO info;
			info.cbSize = sizeof(TOOLINFO);
			info.hwnd = m_hWnd;
			info.uId = (UINT)m_hWnd;
			m_toolTip.SendMessage(TTM_TRACKACTIVATE, false, reinterpret_cast<LPARAM>(&info));

			m_bMouseInTooltip = false;
			++m_toolTip.m_nNextCode;
		}

		m_pHoverNode = pNode;

		if (pNode != NULL)
		{
			InvalidateNode(pNode);

			CPoint ptOffset(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
			CRect rcLabel = pNode->rcLabel - ptOffset;

			CRect rcClient, rcIntersect;
			GetClientRect(rcClient);

			if (rcIntersect.IntersectRect(rcLabel, rcClient) && rcIntersect != rcLabel)
			{
				CPoint ptTooltip = pNode->rcLabel.TopLeft() + (-ptOffset);
				ptTooltip.Offset(s_nTextOffset - 3, 0);

				ClientToScreen(&ptTooltip);
				m_toolTip.SendMessage(TTM_TRACKPOSITION, 0, MAKELPARAM(ptTooltip.x, ptTooltip.y));

				m_toolTip.SetMaxTipWidth(pNode->rcLabel.Width() - s_nOffsetRight - s_nTextOffset);
				m_toolTip.SetMargin(CRect(0, 1, 0, max(0, pNode->rcNode.bottom - pNode->rcText.bottom - 2)));

				m_bMouseInTooltip = true;
				++m_toolTip.m_nNextCode;

				TOOLINFO info;
				info.cbSize = sizeof(TOOLINFO);
				info.hwnd = m_hWnd;
				info.uId = (UINT)m_hWnd;
				m_toolTip.SendMessage(TTM_TRACKACTIVATE, true, reinterpret_cast<LPARAM>(&info));
			}
		}
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
	static HCURSOR hCursorLink = NULL;
	if (hCursorLink == NULL)
		hCursorLink = ::LoadCursor(0, IDC_HAND);
	if (hCursorLink == NULL)
		hCursorLink = AfxGetApp()->LoadCursor(IDC_CURSOR_LINK);

	if (m_pHoverNode != NULL)
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

BOOL CMyTreeCtrl::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
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

	DWORD dwStyle = GetStyle();

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
		break;

	case VK_PRIOR:
		break;
	}

	if (pSelectNode != NULL && pSelectNode != m_pSelection)
		SelectNode(pSelectNode, TVC_BYKEYBOARD);

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
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

	UpdateWindow();
}

void CMyTreeCtrl::ToggleNode(TreeNode* pNode)
{
	pNode->bCollapsed = !pNode->bCollapsed;
	RecalcLayout();
}

CMyTreeCtrl::TreeNode* CMyTreeCtrl::FindNextNode(TreeNode* pNode)
{
	if (pNode == NULL)
		pNode = m_pRoot;

	if (pNode->HasChildren() && !pNode->bCollapsed)
		return pNode->pChild;
	else if (pNode->pNext != NULL)
		return pNode->pNext;
	else
	{
		for (pNode = pNode->pParent; pNode != NULL && pNode != m_pRoot; pNode = pNode->pParent)
		{
			if (pNode->pNext != NULL)
				return pNode->pNext;
		}
	}

	return NULL;
}

CMyTreeCtrl::TreeNode* CMyTreeCtrl::FindPrevNode(TreeNode* pNode)
{
	if (pNode == m_pRoot || m_pRoot->pChild == NULL)
		return NULL;

	if (pNode == NULL)
	{
		pNode = m_pRoot->pChild;
		while (pNode->HasSibling() || pNode->HasChildren() && !pNode->bCollapsed)
			pNode = pNode->HasSibling() ? pNode->pNext : pNode->pChild;
		return pNode;
	}

	TreeNode* pParent = pNode->pParent;
	if (pParent->pChild == pNode)
		return pParent == m_pRoot ? NULL : pParent;

	TreeNode* pPrev = pParent->pChild;
	while (pPrev->pNext != pNode)
		pPrev = pPrev->pNext;

	if (pPrev->HasChildren() && !pPrev->bCollapsed)
	{
		pPrev = pPrev->pChild;
		while (pPrev->HasSibling() || pPrev->HasChildren() && !pPrev->bCollapsed)
			pPrev = pPrev->HasSibling() ? pPrev->pNext : pPrev->pChild;
	}

	return pPrev;
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

BOOL CMyTreeCtrl::CTreeToolTip::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (message == WM_MOUSEMOVE)
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
	else if (message == WM_NCMOUSEMOVE)
	{
		CPoint ptCursor;
		::GetCursorPos(&ptCursor);
		m_pTree->ScreenToClient(&ptCursor);

		m_pTree->OnMouseMove(0, ptCursor);
	}
	else if (message == WM_MOUSEACTIVATE)
	{
		if (pResult != NULL)
			*pResult = MA_NOACTIVATEANDEAT;

		UINT nMsg = (UINT) HIWORD(lParam);
		if (nMsg == WM_LBUTTONDOWN)
			m_pTree->SelectNode(m_pTree->m_pHoverNode, TVC_BYMOUSE);

		return TRUE;
	}
	else if (message == WM_MOUSELEAVE)
	{
		if (m_nNextCode == m_nMouseLeaveCode)
		{
			m_pTree->m_bMouseInTooltip = false;

			CPoint ptCursor;
			::GetCursorPos(&ptCursor);
			m_pTree->ScreenToClient(&ptCursor);

			m_pTree->OnMouseMove(0, ptCursor);
		}
	}

	return CToolTipCtrl::OnWndMsg(message, wParam, lParam, pResult);
}
