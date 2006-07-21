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

#include "MyTheme.h"


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
	void RecalcLayout();

	HTREEITEM InsertItem(LPCTSTR pszItem, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
	HTREEITEM InsertItem(LPCTSTR pszItem, int nImage, int nSelectedImage, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);

	void SetItemData(HTREEITEM hItem, DWORD_PTR dwData);
	DWORD_PTR GetItemData(HTREEITEM hItem);
	HTREEITEM GetSelectedItem();

// Overrides
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
protected:
	class CTreeToolTip : public CToolTipCtrl
	{
	public:
		CTreeToolTip() : m_pTree(NULL), m_nNextCode(0), m_nMouseLeaveCode(0) {}
		void SetTree(CMyTreeCtrl* pTree) { m_pTree = pTree; }
		CMyTreeCtrl* m_pTree;
		int m_nNextCode, m_nMouseLeaveCode;

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
			  pChild(NULL), pNext(NULL), bCollapsed(true), dwUserData(0) {}
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
		TreeNode* pNext;

		bool HasChildren() const { return pChild != NULL; }
		bool HasSibling() const { return pNext != NULL; }
	};

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
	CFont m_font, m_fontHover;
	CSize m_szDisplay;
	CBitmap* m_pOffscreenBitmap;
	CSize m_szOffscreen;

	TreeNode* m_pSelection;
	TreeNode* m_pHoverNode;

	CPoint m_ptScrollOffset;
	CSize m_szLine, m_szPage;

	void SetHoverNode(TreeNode* pNode);
	void SelectNode(TreeNode* pNode, UINT nAction = TVC_UNKNOWN);
	void ToggleNode(TreeNode* pNode);
	TreeNode* FindNextNode(TreeNode* pNode);
	TreeNode* FindPrevNode(TreeNode* pNode);

	int PaintNode(CDC* pDC, TreeNode* pNode, const CRect& rcClip);
	TreeNode* HitTest(CPoint point, int* pnArea);
	TreeNode* HitTest(TreeNode* pNode, CPoint point, int* pnArea);
	void InvalidateNode(TreeNode* pNode);
	void EnsureVisible(TreeNode* pNode);

	CPoint GetScrollPosition();
	bool OnScroll(UINT nScrollCode, UINT nPos);
	bool OnScrollBy(CSize sz);
	void CheckScrollBars(bool& bHasHorzBar, bool& bHasVertBar) const;

	// Generated message map functions
	afx_msg void OnPaint();
	afx_msg void OnEditAddnode();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	LRESULT OnThemeChanged(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnToolTipNeedText(UINT nID, NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseLeave();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	afx_msg void OnSysColorChange();
	DECLARE_MESSAGE_MAP()
};
