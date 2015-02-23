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
#include "PageIndexWnd.h"
#include "DjVuSource.h"
#include "XMLParser.h"

static int s_nRightGap = 4;
static int s_nVertGap = 6;
static int s_nMinTextWidth = 10;
static int s_nMinListHeight = 10;

// CPageIndexWnd

IMPLEMENT_DYNAMIC(CPageIndexWnd, CWnd)

BEGIN_MESSAGE_MAP(CPageIndexWnd, CWnd)
	ON_NOTIFY(TVN_SELCHANGED, CPageIndexWnd::ID_LIST, OnSelChanged)
	ON_NOTIFY(TVN_ITEMCLICKED, CPageIndexWnd::ID_LIST, OnSelChanged)
	ON_NOTIFY(TVN_ITEMEXPANDING, CPageIndexWnd::ID_LIST, OnItemExpanding)
	ON_NOTIFY(TVN_KEYDOWN, CPageIndexWnd::ID_LIST, OnKeyDownList)
	ON_CBN_SETFOCUS(CPageIndexWnd::ID_TEXT, OnLookupFocus)
	ON_CBN_EDITCHANGE(CPageIndexWnd::ID_TEXT, OnChangeText)
	ON_CONTROL(CBN_FINISHEDIT, CPageIndexWnd::ID_TEXT, OnFinishEditText)
	ON_WM_CREATE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_MOUSEACTIVATE()
END_MESSAGE_MAP()

CPageIndexWnd::CPageIndexWnd()
	: m_bChangeInternal(false)
{
	m_pList = new CMyTreeView();
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

	m_cboLookup.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL
			| CBS_DROPDOWN | CBS_AUTOHSCROLL, CRect(0, 0, 0, 180), this, ID_TEXT);
	m_cboLookup.SetExtendedStyle(CBES_EX_CASESENSITIVE | CBES_EX_NOEDITIMAGE,
			CBES_EX_CASESENSITIVE | CBES_EX_NOEDITIMAGE);
	m_cboLookup.GetComboBoxCtrl()->ModifyStyle(CBS_SORT | CBS_NOINTEGRALHEIGHT,
			CBS_AUTOHSCROLL);
	m_cboLookup.SetFont(&m_font);
	m_cboLookup.SetItemHeight(-1, 16);

	m_pList->Create(NULL, NULL, WS_VISIBLE | WS_TABSTOP | WS_CHILD
			| TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS | TVS_TRACKSELECT,
			CRect(), this, ID_LIST);

	m_imageList.Create(16, 16, ILC_COLOR24 | ILC_MASK, 0, 1);
	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_BOOKMARKS);
	m_imageList.Add(&bitmap, RGB(192, 64, 32));
	m_pList->SetImageList(&m_imageList, TVSIL_NORMAL);

	m_pList->SetItemHeight(20);
	m_pList->SetWrapLabels(false);

	UpdateControls();
	Invalidate();

	return 0;
}

inline wstring& tolower(wstring& str)
{
	for (wstring::iterator it = str.begin(); it != str.end(); ++it)
		*it = towlower(*it);
	return str;
}

static const TCHAR pszTagEntry[] = _T("entry");
static const TCHAR pszAttrStartPage[] = _T("first");
static const TCHAR pszAttrLastPage[] = _T("last");
static const TCHAR pszAttrURL[] = _T("url");

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

		CString strTitle;
		if (!entry.strFirst.empty())
		{
			entry.strText = MakeCString(entry.strFirst);
			strTitle = MakeCString(entry.strFirst);
			if (!entry.strLast.empty())
				strTitle += _T(" - ") + MakeCString(entry.strLast);
		}
		else
		{
			entry.strText = MakeCString(entry.strLast);
			strTitle = MakeCString(entry.strLast);
			if (m_entries.size() > 1)
			{
				int nPrev = static_cast<int>(m_entries.size()) - 2;
				while (nPrev >= 0 && m_entries[nPrev].strLink.empty())
					--nPrev;

				if (nPrev >= 0)
				{
					IndexEntry& prev = m_entries[nPrev];
					if (!prev.strLast.empty() && prev.strLast <= entry.strLast)
						entry.strFirst = prev.strLast;
					else if (!prev.strFirst.empty() && prev.strFirst <= entry.strLast)
						entry.strFirst = prev.strFirst;
				}
			}
		}

		++nCount;
		HTREEITEM hItem = m_pList->InsertItem(strTitle, 0, 1, hParent);
		m_pList->SetItemData(hItem, m_entries.size() - 1);
		entry.hItem = hItem;

		entry.strFirst = MapCharacters(tolower(entry.strFirst));
		entry.strLast = MapCharacters(tolower(entry.strLast));

		if (AddEntries(node, hItem) > 0)
			m_pList->Expand(hItem, TVE_EXPAND);
	}

	return nCount;
}

bool CPageIndexWnd::InitPageIndex(DjVuSource* pSource)
{
	GUTF8String strPageIndex = pSource->GetDictionaryInfo()->strPageIndex;
	if (strPageIndex.length() == 0)
		return false;

	stringstream sin((const char*) strPageIndex);
	XMLParser parser;
	if (!parser.Parse(sin))
		return false;

	InitCharacterMap(pSource->GetDictionaryInfo()->strCharMap);

	m_pList->BeginBatchUpdate();

	AddEntries(*parser.GetRoot(), TVI_ROOT);

	m_sorted.resize(m_entries.size());

	// Create a sorted list of items with nonempty links
	size_t k = 0;
	for (size_t i = 0; i < m_sorted.size(); ++i)
		if (!m_entries[i].strLink.empty())
			m_sorted[k++] = &m_entries[i];
	m_sorted.resize(k);

	stable_sort(m_sorted.begin(), m_sorted.end(), CompareEntries);

	m_pList->EndBatchUpdate();

	return !m_entries.empty();
}

static const TCHAR pszAttrFrom[] = _T("from");
static const TCHAR pszAttrTo[] = _T("to");

bool CPageIndexWnd::InitCharacterMap(const GUTF8String& strCharMap)
{
	if (strCharMap.length() == 0)
		return false;

	stringstream sin((const char*) strCharMap);
	XMLParser parser;
	if (!parser.Parse(sin))
		return false;

	const XMLNode& root = *parser.GetRoot();
	list<XMLNode>::const_iterator it;
	for (it = root.childElements.begin(); it != root.childElements.end(); ++it)
	{
		const XMLNode& node = *it;
		if (MakeCString(node.tagName) != pszTagEntry)
			continue;

		wstring strFrom, strTo;
		node.GetAttribute(pszAttrFrom, strFrom);
		node.GetAttribute(pszAttrTo, strTo);

		if (strFrom.length() == 0)
			continue;

		strFrom = tolower(strFrom);
		strTo = tolower(strTo);
		m_charMap.push_back(make_pair(strFrom, strTo));
	}

	sort(m_charMap.begin(), m_charMap.end());

	return true;
}

wstring& CPageIndexWnd::MapCharacters(wstring& str)
{
	if (m_charMap.empty())
		return str;

	wstring result;
	const wchar_t* cur = str.c_str();
	const wchar_t* last = cur + str.length();

	while (*cur != '\0')
	{
		int l = 0, r = static_cast<int>(m_charMap.size());
		if (m_charMap[l].first > cur)
		{
			result += *cur++;
			continue;
		}

		while (r - l > 1)
		{
			int mid = (l + r) / 2;
			if (m_charMap[mid].first <= cur)
				l = mid;
			else
				r = mid;
		}

		bool bFound = false;
		while (l >= 0 && m_charMap[l].first[0] == *cur)
		{
			size_t len = m_charMap[l].first.length();
			if (last - cur >= static_cast<int>(len)
					&& wcsncmp(m_charMap[l].first.c_str(), cur, len) == 0)
			{
				cur += len;
				result += m_charMap[l].second;
				bFound = true;
				break;
			}
			--l;
		}

		if (!bFound)
			result += *cur++;
	}

	str.swap(result);
	return str;
}

void CPageIndexWnd::GoToItem(HTREEITEM hItem)
{
	size_t nEntry = m_pList->GetItemData(hItem);
	IndexEntry& entry = m_entries[nEntry];

	if (!entry.strLink.empty())
	{
		UpdateObservers(LinkClicked(MakeUTF8String(entry.strLink)));
	}
}

void CPageIndexWnd::OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	if (hItem != NULL)
	{
		if (!m_bChangeInternal)
		{
			m_bChangeInternal = true;

			size_t nEntry = m_pList->GetItemData(hItem);
			IndexEntry& entry = m_entries[nEntry];
			m_cboLookup.SetWindowText(entry.strText);
			m_cboLookup.GetEditCtrl()->SetSel(entry.strText.GetLength(), -1);

			m_bChangeInternal = false;
		}

		if (pNMTreeView->action == TVC_BYMOUSE)
			GoToItem(hItem);
	}

	*pResult = 0;
}

void CPageIndexWnd::OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	if (pNMTreeView->action == TVE_COLLAPSE)
	{
		// Cancel the operation
		*pResult = 1;
		return;
	}

	*pResult = 0;
}

void CPageIndexWnd::OnKeyDownList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);

	if (pTVKeyDown->wVKey == VK_RETURN || pTVKeyDown->wVKey == VK_SPACE)
	{
		HTREEITEM hItem = m_pList->GetSelectedItem();
		if (hItem != NULL)
			GoToItem(hItem);

		*pResult = 0;
	}
	else
		*pResult = 1;
}

void CPageIndexWnd::OnFinishEditText()
{
	m_bChangeInternal = true;
	theApp.UpdateSearchHistory(m_cboLookup);
	m_cboLookup.GetEditCtrl()->SetSel(-1, -1);
	m_bChangeInternal = false;

	HTREEITEM hItem = m_pList->GetSelectedItem();
	if (hItem != NULL)
		GoToItem(hItem);
}

void CPageIndexWnd::OnChangeText()
{
	if (m_bChangeInternal)
		return;

	CString strText;
	m_cboLookup.GetWindowText(strText);

	if (strText.IsEmpty())
		return;

	wstring text;
	MakeWString(strText, text);

	text = MapCharacters(tolower(text));

	int left = 0, right = (int)m_sorted.size();
	while (left < right - 1)
	{
		int mid = (left + right) / 2;
		if (m_sorted[mid]->strFirst <= text)
			left = mid;
		else
			right = mid;
	}

	while (left > 0 && m_sorted[left - 1]->strFirst == text)
		--left;
	while (left > 0 && m_sorted[left - 1]->strLast == text)
		--left;

	m_bChangeInternal = true;
	if (left >= 0 && left < static_cast<int>(m_sorted.size()))
		m_pList->SelectItem(m_sorted[left]->hItem);
	else
		m_pList->SelectItem(NULL);
	m_bChangeInternal = false;
}

void CPageIndexWnd::Lookup(const CString& strLookup)
{
	m_bChangeInternal = true;
	m_cboLookup.SetWindowText(strLookup);
	m_bChangeInternal = false;

	OnChangeText();

	HTREEITEM hItem = m_pList->GetSelectedItem();
	if (hItem != NULL)
		GoToItem(hItem);
}

void CPageIndexWnd::OnLookupFocus()
{
	theApp.InitSearchHistory(m_cboLookup);
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

void CPageIndexWnd::UpdateControls()
{
	CSize szClient = ::GetClientSize(this);

	CRect rcCombo;
	m_cboLookup.GetWindowRect(rcCombo);

	CSize szText(max(s_nMinTextWidth, szClient.cx - s_nRightGap), rcCombo.Height());
	CPoint ptText(0, 0);

	CPoint ptList(1, szText.cy + s_nVertGap + 1);
	CSize szList(max(s_nMinTextWidth, szClient.cx - 1), max(s_nMinListHeight, szClient.cy - ptList.y));

	m_cboLookup.SetWindowPos(NULL, ptText.x, ptText.y, szText.cx, szText.cy, SWP_NOACTIVATE);
	m_pList->SetWindowPos(NULL, ptList.x, ptList.y, szList.cx, szList.cy, SWP_NOACTIVATE);

	m_rcText = CRect(ptText, szText);
	m_rcList = CRect(ptList, szList);
	m_rcList.InflateRect(1, 1);

	m_rcGap = CRect(0, m_rcText.bottom, szClient.cx, m_rcList.top);
}

BOOL CPageIndexWnd::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void CPageIndexWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	COLORREF clrBtnface = ::GetSysColor(COLOR_BTNFACE);
	COLORREF clrShadow = ::GetSysColor(COLOR_BTNSHADOW);
	CBrush brushShadow(clrShadow);

	CRect rcClient = ::GetClientRect(this);
	CRect rcRightGap(rcClient.right - s_nRightGap, 0, rcClient.right, m_rcGap.bottom);
	dc.FillSolidRect(rcRightGap, clrBtnface);
	dc.FillSolidRect(m_rcGap, clrBtnface);

	dc.FrameRect(m_rcList, &brushShadow);
}

BOOL CPageIndexWnd::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			HTREEITEM hItem = m_pList->GetSelectedItem();
			if (hItem != NULL)
				GoToItem(hItem);

			return true;
		}
		if (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN ||
			pMsg->wParam == VK_PRIOR || pMsg->wParam == VK_NEXT)
		{
			m_pList->SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
			return true;
		}
	}
	else if (pMsg->message == WM_MOUSEWHEEL)
	{
		CPoint point((DWORD) pMsg->lParam);

		CWnd* pWnd = WindowFromPoint(point);
		if (pWnd != this && IsFromCurrentProcess(pWnd) && 
				pWnd->SendMessage(WM_MOUSEWHEEL, pMsg->wParam, pMsg->lParam) != 0)
			return true;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CPageIndexWnd::OnSetFocus(CWnd* pOldWnd)
{
	m_cboLookup.SetFocus();
}

int CPageIndexWnd::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	int nResult = CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
	if (nResult == MA_NOACTIVATE || nResult == MA_NOACTIVATEANDEAT)
		return nResult;

	return MA_NOACTIVATE;
}
