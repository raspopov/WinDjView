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

#include "MyTheme.h"
#include "Drawing.h"

#define TVN_ITEMCLICKED (WM_USER + 30)


class CMyTreeCtrl : public CWnd
{
	DECLARE_DYNAMIC(CMyTreeCtrl)

public:
	CMyTreeCtrl();
	virtual ~CMyTreeCtrl();

// Attributes
public:
	void SetItemHeight(int nHeight);
	void SetImageList(CImageList* pImageList, DWORD dwStyle);
	void SetWrapLabels(bool bWrapLabels);
	void SetRedirectWheel(bool bRedirectWheel);
	void RecalcLayout();

	HTREEITEM InsertItem(LPCTSTR pszItem, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
	HTREEITEM InsertItem(LPCTSTR pszItem, int nImage, int nSelectedImage, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);

	void SetItemData(HTREEITEM hItem, DWORD_PTR dwData);
	DWORD_PTR GetItemData(HTREEITEM hItem);
	HTREEITEM GetSelectedItem();
	void SelectItem(HTREEITEM hItem);
	bool Expand(HTREEITEM hItem, UINT nCode);

	void BeginBatchUpdate();
	void EndBatchUpdate();

// Overrides
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
protected:
	class CTreeToolTip : public CWnd
	{
	public:
		CTreeToolTip() : m_pTree(NULL), m_nNextCode(0), m_nMouseLeaveCode(0) {}
		BOOL Create(CMyTreeCtrl* pTree);
		void Show(const CString& strText, bool bWrap, const CRect& rcWindow, const CRect& rcText);
		void Hide();

		CMyTreeCtrl* m_pTree;
		CString m_strText;
		CRect m_rcText;
		int m_nNextCode, m_nMouseLeaveCode;
		bool m_bWrap;

		virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	};
	friend class CTreeToolTip;

	bool m_bMouseInTooltip;
	CTreeToolTip m_toolTip;
	CString m_strToolTip;

	struct TreeNode
	{
		TreeNode(LPCTSTR pszItem, int nImageIndex, int nSelectedIndex, TreeNode* pParentNode)
			: strLabel(pszItem), nImage(nImageIndex), nSelectedImage(nSelectedIndex), pParent(pParentNode),
			  pChild(NULL), pLastChild(NULL), pNext(NULL), bCollapsed(true), dwUserData(0) {}
		~TreeNode();

		CString strLabel;
		int nImage;
		int nSelectedImage;
		bool bCollapsed;
		CRect rcNode, rcText, rcLabel, rcGlyph, rcImage;
		int nLineX, nLineY, nLineStopX;
		DWORD_PTR dwUserData;
		TreeNode* pParent;
		TreeNode* pChild;
		TreeNode* pLastChild;
		TreeNode* pNext;

		int nIndex;

		bool HasChildren() const { return pChild != NULL; }
		bool HasSibling() const { return pNext != NULL; }
	};

	vector<TreeNode*> m_items;

	enum HitTestArea
	{
		HT_LABEL,
		HT_IMAGE,
		HT_GLYPH,
		HT_OTHER
	};

	HTHEME m_hTheme;
	TreeNode* m_pRoot;
	int m_nItemHeight;
	CImageList* m_pImageList;

	bool m_bWrapLabels;
	bool m_bRedirectWheel;
	bool m_bLinesAtRoot;
	bool m_bHasLines;
	bool m_bHasGlyphs;
	CFont m_font, m_fontHover;
	CSize m_szDisplay;
	bool m_bBatchUpdate;

	COffscreenDC m_offscreenDC;

	TreeNode* m_pSelection;
	TreeNode* m_pHoverNode;

	CPoint m_ptScrollOffset;
	CSize m_szLine, m_szPage;

	void SetHoverNode(TreeNode* pNode);
	void SelectNode(TreeNode* pNode, UINT nAction = TVC_UNKNOWN);
	void ToggleNode(TreeNode* pNode);
	void ExpandNode(TreeNode* pNode, bool bExpand = true);
	TreeNode* FindNextNode(TreeNode* pNode);
	TreeNode* FindPrevNode(TreeNode* pNode);
	TreeNode* FindNextPageNode(TreeNode* pNode);
	TreeNode* FindPrevPageNode(TreeNode* pNode);

	int PaintNode(CDC* pDC, TreeNode* pNode, const CRect& rcClip);
	TreeNode* HitTest(CPoint point, int* pnArea);
	TreeNode* HitTest(TreeNode* pNode, CPoint point, int* pnArea);
	void InvalidateNode(TreeNode* pNode);
	void EnsureVisible(TreeNode* pNode);

	CPoint GetScrollPosition();
	bool OnScroll(UINT nScrollCode, UINT nPos);
	bool OnScrollBy(CSize sz);
	void CheckScrollBars(bool& bHasHorzBar, bool& bHasVertBar) const;
	bool UpdateParameters();

	// Generated message map functions
	afx_msg void OnPaint();
	afx_msg void OnEditAddnode();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnThemeChanged();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseLeave();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	afx_msg void OnSysColorChange();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	DECLARE_MESSAGE_MAP()
};
