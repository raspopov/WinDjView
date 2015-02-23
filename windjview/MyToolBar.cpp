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
#include "MyToolBar.h"
#include "Drawing.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMyToolBar

IMPLEMENT_DYNAMIC(CMyToolBar, CControlBar)

CMyToolBar::CMyToolBar()
{
	m_cxLeftBorder = m_cxRightBorder = m_cyBottomBorder = m_cyTopBorder = 0;
}

CMyToolBar::~CMyToolBar()
{
}


BEGIN_MESSAGE_MAP(CMyToolBar, CControlBar)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_NOTIFY(NM_CUSTOMDRAW, 1, OnCustomDraw)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()


// CMyToolBar message handlers

BOOL CMyToolBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	VERIFY(AfxDeferRegisterClass(AFX_WNDCONTROLBAR_REG));

	DWORD dwNewStyle = (dwStyle & CBRS_ALIGN_ANY) | (dwStyle & CBRS_BORDER_ANY)
			| WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	// Save the style
	m_dwStyle = (dwNewStyle & CBRS_ALL);

	if (!CControlBar::Create(_afxWndControlBar, NULL,
			dwNewStyle, CRect(0, 0, 0, 0), pParentWnd, nID))
		return false;

	DWORD dwToolBarStyle = dwStyle | WS_VISIBLE | WS_CLIPCHILDREN;
	if (!m_toolBar.Create(this, dwToolBarStyle, 1))
		return false;

	m_toolBar.GetToolBarCtrl().SetExtendedStyle(TBSTYLE_EX_DOUBLEBUFFER);
	return true;
}

BOOL CMyToolBar::CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle,
	DWORD dwStyle, CRect rcBorders, UINT nID)
{
	VERIFY(AfxDeferRegisterClass(AFX_WNDCONTROLBAR_REG));

	DWORD dwNewStyle = (dwStyle & CBRS_ALIGN_ANY) | (dwStyle & CBRS_BORDER_ANY)
			| WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	// Save the style
	m_dwStyle = (dwNewStyle & CBRS_ALL);

	if (!CControlBar::Create(_afxWndControlBar, NULL,
			dwNewStyle, CRect(0, 0, 0, 0), pParentWnd, nID))
		return false;

	DWORD dwToolBarStyle = dwStyle | WS_VISIBLE | WS_CLIPCHILDREN;
	if (!m_toolBar.CreateEx(this, dwCtrlStyle, dwToolBarStyle, rcBorders, 1))
		return false;

	m_toolBar.GetToolBarCtrl().SetExtendedStyle(TBSTYLE_EX_DOUBLEBUFFER);
	return true;
}

int CMyToolBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	theApp.AddObserver(this);

	return 0;
}

void CMyToolBar::OnDestroy()
{
	theApp.RemoveObserver(this);

	CControlBar::OnDestroy();
}

void CMyToolBar::OnNcPaint()
{
	CWindowDC dc(this);

	CRect rcClient = ::GetClientRect(this);
	CRect rcWindow;
	GetWindowRect(rcWindow);
	ScreenToClient(rcWindow);

	CRect rcToolBar = ::GetClientRect(m_toolBar);
	m_toolBar.ClientToScreen(rcToolBar);
	ScreenToClient(rcToolBar);

	dc.ExcludeClipRect(rcToolBar - rcWindow.TopLeft());
	dc.FillSolidRect(rcClient - rcWindow.TopLeft(), ::GetSysColor(COLOR_BTNFACE));
}

void CMyToolBar::OnPaint()
{
	// Do nothing, but we still must create a paint DC in this handler,
	// otherwise Windows goes crazy.
	CPaintDC paintDC(this);
}

void CMyToolBar::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTBCUSTOMDRAW pTBCD = (LPNMTBCUSTOMDRAW) pNMHDR;

	if (pTBCD->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
		return;
	}

	if (pTBCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		if (m_controls.find(pTBCD->nmcd.dwItemSpec) != m_controls.end())
		{
			*pResult = CDRF_SKIPDEFAULT;
			return;
		}

		for (size_t nLabel = 0; nLabel < m_labels.size(); ++nLabel)
		{
			if (pTBCD->nmcd.dwItemSpec == m_labels[nLabel].nID)
			{
				Label& label = m_labels[nLabel];

				CDC dc;
				dc.Attach(pTBCD->nmcd.hdc);

				CFont* pOldFont = dc.SelectObject(label.pFont);
				COLORREF crTextColor = dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
				int nBkMode = dc.SetBkMode(TRANSPARENT);

				UINT nFlags = DT_LEFT | DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE;
				CRect rect(pTBCD->nmcd.rc);
				rect.left += 3;
				dc.DrawText(label.strText, rect, nFlags);

				dc.SetTextColor(crTextColor);
				dc.SetBkMode(nBkMode);
				dc.SelectObject(pOldFont);

				dc.Detach();

				*pResult = CDRF_SKIPDEFAULT;
				return;
			}
		}
	}

	*pResult = 0;
}

void CMyToolBar::InsertLabel(int nPos, UINT nID, CFont* pFont)
{
	Label label;
	label.nID = nID;
	label.strText = LoadString(nID);
	label.pFont = pFont;
	m_labels.push_back(label);

	CScreenDC dcScreen;
	CFont* pOldFont = dcScreen.SelectObject(pFont);
	int nWidth = dcScreen.GetTextExtent(label.strText).cx + 3;
	dcScreen.SelectObject(pOldFont);

	TBBUTTON btn;
	ZeroMemory(&btn, sizeof(btn));
	btn.idCommand = nID;
	btn.fsStyle = TBSTYLE_BUTTON;

	TBBUTTONINFO info;
	ZeroMemory(&info, sizeof(info));
	info.cbSize = sizeof(info);
	info.dwMask = TBIF_SIZE;
	info.cx = nWidth;

	GetToolBarCtrl().InsertButton(nPos, &btn);
	GetToolBarCtrl().SetButtonInfo(nID, &info);
}

void CMyToolBar::OnUpdate(const Observable* source, const Message* message)
{
	if (message->code == APP_LANGUAGE_CHANGED)
	{
		CScreenDC dcScreen;
		for (size_t nLabel = 0; nLabel < m_labels.size(); ++nLabel)
		{
			Label& label = m_labels[nLabel];
			label.strText = LoadString(label.nID);

			CFont* pOldFont = dcScreen.SelectObject(label.pFont);
			int nWidth = dcScreen.GetTextExtent(label.strText).cx + 3;
			dcScreen.SelectObject(pOldFont);

			TBBUTTONINFO info;
			ZeroMemory(&info, sizeof(info));
			info.cbSize = sizeof(info);
			info.dwMask = TBIF_SIZE;
			info.cx = nWidth;

			GetToolBarCtrl().SetButtonInfo(label.nID, &info);
		}

		if (!m_labels.empty())
		{
			for (int nButton = 0; nButton < GetToolBarCtrl().GetButtonCount(); ++nButton)
			{
				TBBUTTON btn;
				ZeroMemory(&btn, sizeof(btn));
				GetToolBarCtrl().GetButton(nButton, &btn);
				CWnd* pWnd = m_toolBar.GetDlgItem(btn.idCommand);

				if (btn.fsStyle == TBSTYLE_BUTTON && pWnd != NULL)
				{
					CRect rcItem;
					m_toolBar.GetItemRect(nButton, rcItem);
					rcItem.DeflateRect(3, 0);

					pWnd->MoveWindow(rcItem);
				}
			}

			GetToolBarCtrl().AutoSize();
			Invalidate(false);
		}
	}
}

void CMyToolBar::OnWindowPosChanging(LPWINDOWPOS lpWndPos)
{
	if (!(lpWndPos->flags & SWP_NOSIZE))
	{
		m_toolBar.MoveWindow(0, 0, lpWndPos->cx, lpWndPos->cy);
	}

	CControlBar::OnWindowPosChanging(lpWndPos);
}

CSize CMyToolBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	return m_toolBar.CalcFixedLayout(bStretch, bHorz);
}

CSize CMyToolBar::CalcDynamicLayout(int nLength, DWORD nMode)
{
	return m_toolBar.CalcDynamicLayout(nLength, nMode);
}

void CMyToolBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
}

void CMyToolBar::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	Invalidate();

	if (m_pDockSite != NULL)
		m_pDockSite->RecalcLayout();
}

void CMyToolBar::OnSysColorChange()
{
	Invalidate();
	m_toolBar.Invalidate();
}

void CMyToolBar::SetControl(int nPos, UINT nID, int nWidth)
{
	m_controls.insert(nID);

	TBBUTTON btn;
	ZeroMemory(&btn, sizeof(btn));
	btn.idCommand = nID;
	btn.fsStyle = TBSTYLE_BUTTON;

	TBBUTTONINFO info;
	ZeroMemory(&info, sizeof(info));
	info.cbSize = sizeof(info);
	info.dwMask = TBIF_SIZE;
	info.cx = nWidth;

	GetToolBarCtrl().DeleteButton(nPos);
	GetToolBarCtrl().InsertButton(nPos, &btn);
	GetToolBarCtrl().SetButtonInfo(nID, &info);
}

BOOL CMyToolBar::CAuxToolBar::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (message == WM_RBUTTONDOWN || message == WM_RBUTTONUP)
	{
		// Eat the message. Standard windows toolbar does some strange
		// things after right-clicking.
		if (pResult != NULL)
			*pResult = 0;
		return true;
	}

	return CToolBar::OnWndMsg(message, wParam, lParam, pResult);
}


// Override MFC to allow loading full color bitmaps into the toolbar

struct CToolBarData
{
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	//WORD aItems[wItemCount]

	WORD* items()
	{
		return (WORD*)(this + 1);
	}
};

bool CMyToolBar::CAuxToolBar::LoadToolBar(UINT nIDResource)
{
	return LoadToolBar(MAKEINTRESOURCE(nIDResource));
}

bool CMyToolBar::CAuxToolBar::LoadToolBar(LPCTSTR lpszResourceName)
{
	ASSERT_VALID(this);
	ASSERT(lpszResourceName != NULL);

	// determine location of the bitmap in resource fork
	HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_TOOLBAR);
	HRSRC hRsrc = ::FindResource(hInst, lpszResourceName, RT_TOOLBAR);
	if (hRsrc == NULL)
		return FALSE;

	HGLOBAL hGlobal = LoadResource(hInst, hRsrc);
	if (hGlobal == NULL)
		return FALSE;

	CToolBarData* pData = (CToolBarData*)LockResource(hGlobal);
	if (pData == NULL)
		return FALSE;
	ASSERT(pData->wVersion == 1);

	UINT* pItems = new UINT[pData->wItemCount];
	for (int i = 0; i < pData->wItemCount; i++)
		pItems[i] = pData->items()[i];
	bool bResult = !!SetButtons(pItems, pData->wItemCount);
	delete[] pItems;

	if (bResult)
	{
		// set new sizes of the buttons
		CSize sizeImage(pData->wWidth, pData->wHeight);
		CSize sizeButton(pData->wWidth + 7, pData->wHeight + 7);
		SetSizes(sizeButton, sizeImage);

		// load bitmap now that sizes are known by the toolbar control
		bResult = LoadBitmap(lpszResourceName);
	}

	UnlockResource(hGlobal);
	FreeResource(hGlobal);

	return bResult;
}

#define AFX_RGB_TO_RGBQUAD(r,g,b)   (RGB(b,g,r))
#define AFX_CLR_TO_RGBQUAD(clr)     (RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr)))

struct AFX_COLORMAP
{
	// use DWORD instead of RGBQUAD so we can compare two RGBQUADs easily
	DWORD rgbqFrom;
	int iSysColorTo;
};

static const AFX_COLORMAP _afxSysColorMap[] =
{
	// mapping from color in DIB to system color
	{ AFX_RGB_TO_RGBQUAD(0x00, 0x00, 0x00), COLOR_BTNTEXT },       // black
	{ AFX_RGB_TO_RGBQUAD(0x80, 0x80, 0x80), COLOR_BTNSHADOW },     // dark gray
	{ AFX_RGB_TO_RGBQUAD(0xC0, 0xC0, 0xC0), COLOR_BTNFACE },       // bright gray
	{ AFX_RGB_TO_RGBQUAD(0xFF, 0xFF, 0xFF), COLOR_BTNHIGHLIGHT }   // white
};

static HBITMAP MyAfxLoadSysColorBitmap(HINSTANCE hInst, HRSRC hRsrc, BOOL bMono = false)
{
	HGLOBAL hglb;
	if ((hglb = LoadResource(hInst, hRsrc)) == NULL)
	{
		return NULL;
	}

	LPBITMAPINFOHEADER lpBitmap = (LPBITMAPINFOHEADER)LockResource(hglb);
	if ((lpBitmap == NULL)/* || (lpBitmap->biBitCount > 8)*/)
	{
		return NULL;
	}

	// make copy of BITMAPINFOHEADER so we can modify the color table
	const int nColorTableSize = 16;
	UINT nSize = lpBitmap->biSize + nColorTableSize * sizeof(RGBQUAD);
	LPBITMAPINFOHEADER lpBitmapInfo = (LPBITMAPINFOHEADER)::malloc(nSize);
	if (lpBitmapInfo == NULL)
	{
		return NULL;
	}

	Checked::memcpy_s(lpBitmapInfo, nSize, lpBitmap, nSize);

	// color table is in RGBQUAD DIB format
	DWORD* pColorTable =
		(DWORD*)(((LPBYTE)lpBitmapInfo) + (UINT)lpBitmapInfo->biSize);

	for (int iColor = 0; iColor < nColorTableSize; iColor++)
	{
		// look for matching RGBQUAD color in original
		for (int i = 0; i < _countof(_afxSysColorMap); i++)
		{
			if (pColorTable[iColor] == _afxSysColorMap[i].rgbqFrom)
			{
				if (bMono)
				{
					// all colors except text become white
					if (_afxSysColorMap[i].iSysColorTo != COLOR_BTNTEXT)
						pColorTable[iColor] = AFX_RGB_TO_RGBQUAD(255, 255, 255);
				}
				else
					pColorTable[iColor] =
					AFX_CLR_TO_RGBQUAD(::GetSysColor(_afxSysColorMap[i].iSysColorTo));
				break;
			}
		}
	}

	int nWidth = (int)lpBitmapInfo->biWidth;
	int nHeight = (int)lpBitmapInfo->biHeight;
	HDC hDCScreen = ::GetDC(NULL);
	HBITMAP hbm = ::CreateCompatibleBitmap(hDCScreen, nWidth, nHeight);

	if (hbm != NULL)
	{
		HDC hDCGlyphs = ::CreateCompatibleDC(hDCScreen);
		HBITMAP hbmOld = (HBITMAP)::SelectObject(hDCGlyphs, hbm);

		LPBYTE lpBits;
		lpBits = (LPBYTE)(lpBitmap + 1);
		lpBits += ((size_t)1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD);

		StretchDIBits(hDCGlyphs, 0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight,
			lpBits, (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
		SelectObject(hDCGlyphs, hbmOld);
		::DeleteDC(hDCGlyphs);
	}
	::ReleaseDC(NULL, hDCScreen);

	// free copy of bitmap info struct and resource itself
	::free(lpBitmapInfo);
	::FreeResource(hglb);

	return hbm;
}

bool CMyToolBar::CAuxToolBar::LoadBitmap(UINT nIDResource)
{
	return LoadBitmap(MAKEINTRESOURCE(nIDResource));
}

bool CMyToolBar::CAuxToolBar::LoadBitmap(LPCTSTR lpszResourceName)
{
	ASSERT_VALID(this);
	ASSERT(lpszResourceName != NULL);

	// determine location of the bitmap in resource fork
	HINSTANCE hInstImageWell = AfxFindResourceHandle(lpszResourceName, RT_BITMAP);
	HRSRC hRsrcImageWell = ::FindResource(hInstImageWell, lpszResourceName, RT_BITMAP);
	if (hRsrcImageWell == NULL)
		return FALSE;

	// load the bitmap
	HBITMAP hbmImageWell;
	hbmImageWell = MyAfxLoadSysColorBitmap(hInstImageWell, hRsrcImageWell);

	// tell common control toolbar about the new bitmap
	if (!AddReplaceBitmap(hbmImageWell))
		return FALSE;

	// remember the resource handles so the bitmap can be recolored if necessary
	m_hInstImageWell = hInstImageWell;
	m_hRsrcImageWell = hRsrcImageWell;
	return TRUE;
}
