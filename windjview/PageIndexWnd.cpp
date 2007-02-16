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
#include "PageIndexWnd.h"
#include "DjVuSource.h"
#include "XMLParser.h"


static int s_nTextHeight = 11;

static int s_nRightGap = 4;
static int s_nVertGap = 6;
static int s_nMinTextWidth = 10;
static int s_nMinListHeight = 10;

// CPageIndexWnd

IMPLEMENT_DYNAMIC(CPageIndexWnd, CWnd)

BEGIN_MESSAGE_MAP(CPageIndexWnd, CWnd)
	ON_NOTIFY(TVN_SELCHANGED, CPageIndexWnd::ID_LIST, OnSelChanged)
	ON_NOTIFY(TVN_ITEMCLICKED, CPageIndexWnd::ID_LIST, OnItemClicked)
	ON_NOTIFY(TVN_KEYDOWN, CPageIndexWnd::ID_LIST, OnKeyDownList)
	ON_EN_CHANGE(CPageIndexWnd::ID_TEXT, OnChangeText)
	ON_NOTIFY(NM_KEYDOWN, CPageIndexWnd::ID_TEXT, OnKeyDownText)
	ON_WM_CREATE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

CPageIndexWnd::CPageIndexWnd()
{
}

CPageIndexWnd::~CPageIndexWnd()
{
}


// CPageIndexWnd message handlers

BOOL CPageIndexWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs))
		return false;

	static CString strWndClass = AfxRegisterWndClass(CS_DBLCLKS,
			::LoadCursor(NULL, IDC_ARROW));
	cs.lpszClass = strWndClass;

	return true;
}

int CPageIndexWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CreateSystemDialogFont(m_font);

	m_edtText.Create(WS_VISIBLE | WS_TABSTOP | WS_CHILD,
		CRect(), this, ID_TEXT);
	m_edtText.SetFont(&m_font);

	m_list.Create(NULL, NULL, WS_VISIBLE | WS_TABSTOP | WS_CHILD
		| TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS | TVS_TRACKSELECT,
		CRect(), this, ID_LIST);

	m_imageList.Create(16, 16, ILC_COLOR24 | ILC_MASK, 0, 1);
	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_BOOKMARKS);
	m_imageList.Add(&bitmap, RGB(192, 64, 32));
	m_list.SetImageList(&m_imageList, TVSIL_NORMAL);

	m_list.SetItemHeight(20);
	m_list.SetWrapLabels(false);

	UpdateControls();
	Invalidate();

	return 0;
}

inline wstring tolower(wstring str)
{
	for (wstring::iterator it = str.begin(); it != str.end(); ++it)
		*it = towlower(*it);
	return str;
}

const TCHAR pszTagEntry[] = _T("entry");
const TCHAR pszAttrStartPage[] = _T("first");
const TCHAR pszAttrLastPage[] = _T("last");
const TCHAR pszAttrURL[] = _T("url");

int CPageIndexWnd::AddEntries(const XMLNode& parent, HTREEITEM hParent)
{
	list<XMLNode>::const_iterator it;
	int nCount = 0;
	for (it = parent.childElements.begin(); it != parent.childElements.end(); ++it)
	{
		const XMLNode& node = *it;
		if (MakeCString(node.tagName) != pszTagEntry)
			continue;

		m_entries.push_back(IndexEntry());
		IndexEntry& entry = m_entries.back();

		node.GetAttribute(pszAttrStartPage, entry.strFirst);
		node.GetAttribute(pszAttrLastPage, entry.strLast);
		node.GetAttribute(pszAttrURL, entry.strLink);

		CString strTitle = MakeCString(entry.strFirst);
		if (!entry.strLast.empty())
			strTitle += _T(" - ") + MakeCString(entry.strLast);

		++nCount;
		HTREEITEM hItem = m_list.InsertItem(strTitle, 0, 1, hParent);
		m_list.SetItemData(hItem, m_entries.size() - 1);
		entry.hItem = hItem;

		entry.strFirst = tolower(entry.strFirst);
		entry.strLast = tolower(entry.strLast);

		if (AddEntries(node, hItem) > 0)
			m_list.Expand(hItem, TVE_EXPAND);
	}

	return nCount;
}

bool CPageIndexWnd::InitPageIndex(DjVuSource* pSource)
{
	GUTF8String strPageIndex = pSource->GetPageIndex();
	if (strPageIndex.length() == 0)
		return false;

	stringstream sin((const char*) strPageIndex);
	XMLParser parser;
	if (!parser.Parse(sin))
		return false;

	m_list.BeginBatchUpdate();

	AddEntries(*parser.GetRoot(), TVI_ROOT);

	m_sorted.resize(m_entries.size());

	// Create a sorted list of items with nonempty links
	size_t k = 0;
	for (size_t i = 0; i < m_sorted.size(); ++i)
		if (!m_entries[i].strLink.empty())
			m_sorted[k++] = &m_entries[i];
	m_sorted.resize(k);

	sort(m_sorted.begin(), m_sorted.end(), CompareEntries);

	m_list.EndBatchUpdate();

	return !m_entries.empty();
}

void CPageIndexWnd::GoToItem(HTREEITEM hItem)
{
	size_t nEntry = m_list.GetItemData(hItem);
	IndexEntry& entry = m_entries[nEntry];

	if (!entry.strLink.empty())
	{
		UpdateObservers(LinkClicked(MakeUTF8String(entry.strLink)));
	}
}

void CPageIndexWnd::OnSelChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if (pNMTreeView->action == TVC_BYMOUSE)
	{
		HTREEITEM hItem = pNMTreeView->itemNew.hItem;
		if (hItem != NULL)
			GoToItem(hItem);
	}

	*pResult = 0;
}

void CPageIndexWnd::OnItemClicked(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if (pNMTreeView->action == TVC_BYMOUSE)
	{
		HTREEITEM hItem = pNMTreeView->itemNew.hItem;
		if (hItem != NULL)
			GoToItem(hItem);
	}

	*pResult = 0;
}

void CPageIndexWnd::OnKeyDownList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);

	if (pTVKeyDown->wVKey == VK_RETURN || pTVKeyDown->wVKey == VK_SPACE)
	{
		HTREEITEM hItem = m_list.GetSelectedItem();
		if (hItem != NULL)
			GoToItem(hItem);

		*pResult = 0;
	}
	else
		*pResult = 1;
}

void CPageIndexWnd::OnKeyDownText(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMKEY pKeyDown = reinterpret_cast<LPNMKEY>(pNMHDR);

	if (pKeyDown->nVKey == VK_RETURN)
	{
		HTREEITEM hItem = m_list.GetSelectedItem();
		if (hItem != NULL)
			GoToItem(hItem);

		*pResult = 0;
	}
	else
		*pResult = 1;
}

void CPageIndexWnd::OnChangeText()
{
	CString strText;
	GetDlgItem(ID_TEXT)->GetWindowText(strText);

	wstring text;
	MakeWString(strText, text);
	text = tolower(text);

	int left = 0, right = m_sorted.size();
	while (left < right - 1)
	{
		int mid = (left + right) / 2;
		if (m_sorted[mid]->strFirst.compare(text) <= 0)
			left = mid;
		else
			right = mid;
	}

	if (left > 0 && m_sorted[left - 1]->strLast.compare(text) == 0)
		--left;

	if (left >= 0 && left < static_cast<int>(m_sorted.size()))
		m_list.SelectItem(m_sorted[left]->hItem);
	else
		m_list.SelectItem(NULL);
}

void CPageIndexWnd::PostNcDestroy()
{
	// Should be created on heap
	delete this;
}

void CPageIndexWnd::OnWindowPosChanged(WINDOWPOS* lpwndpos) 
{
	CWnd::OnWindowPosChanged(lpwndpos);

	UpdateControls();
}

inline CPoint ConvertDialogPoint(const CPoint& point, const CSize& szBaseUnits)
{
	CPoint result;

	result.x = MulDiv(point.x, szBaseUnits.cx, 4);
	result.y = MulDiv(point.y, szBaseUnits.cy, 8);

	return result;
}

void CPageIndexWnd::UpdateControls()
{
	CRect rcClient;
	GetClientRect(rcClient);

	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	CFont* pOldFont = dcScreen.SelectObject(&m_font);

	TEXTMETRIC tm;
	dcScreen.GetTextMetrics(&tm);
	dcScreen.SelectObject(pOldFont);

	CSize szBaseUnit(tm.tmAveCharWidth, tm.tmHeight);

	CPoint ptText = ConvertDialogPoint(CPoint(0, s_nTextHeight), szBaseUnit);
	CSize szText(max(s_nMinTextWidth, rcClient.Width() - s_nRightGap - 2), ptText.y - 2);
	ptText = CPoint(1, 1);

	CPoint ptList(1, szText.cy + s_nVertGap + 3);
	CSize szList(max(s_nMinTextWidth, rcClient.Width() - 1/*s_nRightGap - 2*/),
		max(s_nMinListHeight, rcClient.Height() - ptList.y));

	m_edtText.SetWindowPos(NULL, ptText.x, ptText.y, szText.cx, szText.cy, SWP_NOACTIVATE);
	m_list.SetWindowPos(NULL, ptList.x, ptList.y, szList.cx, szList.cy, SWP_NOACTIVATE);

	m_rcText = CRect(ptText, szText);
	m_rcText.InflateRect(1, 1);
	m_rcList = CRect(ptList, szList);
	m_rcList.InflateRect(1, 1);

	m_rcGap = CRect(0, m_rcText.bottom, rcClient.Width(), m_rcList.top);
}

BOOL CPageIndexWnd::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void CPageIndexWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rcClient;
	GetClientRect(rcClient);

	COLORREF clrBtnface = ::GetSysColor(COLOR_BTNFACE);
	COLORREF clrShadow = ::GetSysColor(COLOR_BTNSHADOW);
	CBrush brushShadow(clrShadow);

	CRect rcRightGap(rcClient.Width() - s_nRightGap, 0, rcClient.Width(), m_rcGap.bottom);
	dc.FillSolidRect(rcRightGap, clrBtnface);
	dc.FillSolidRect(m_rcGap, clrBtnface);

	dc.FrameRect(m_rcText, &brushShadow);
	dc.FrameRect(m_rcList, &brushShadow);
}

BOOL CPageIndexWnd::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			HTREEITEM hItem = m_list.GetSelectedItem();
			if (hItem != NULL)
				GoToItem(hItem);

			return true;
		}
		if (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN ||
			pMsg->wParam == VK_PRIOR || pMsg->wParam == VK_NEXT)
		{
			m_list.SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
			return true;
		}
	}
	else if (pMsg->message == WM_MOUSEWHEEL)
	{
		CPoint point((DWORD) pMsg->lParam);

		CWnd* pWnd = WindowFromPoint(point);
		if (pWnd != this && !IsChild(pWnd) && IsFromCurrentProcess(pWnd))
			pWnd->SendMessage(WM_MOUSEWHEEL, pMsg->wParam, pMsg->lParam);

		return true;
	}

	return CWnd::PreTranslateMessage(pMsg);
}
