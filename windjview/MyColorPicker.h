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

#include "MyTheme.h"
#include "Drawing.h"


// CMyColorPopup messages

#define CPN_SELCHANGE (WM_USER + 1001)     // Color Picker selection change
#define CPN_DROPDOWN (WM_USER + 1002)      // Color Picker drop down
#define CPN_CLOSEUP (WM_USER + 1003)       // Color Picker close up
#define CPN_SELENDOK (WM_USER + 1004)      // Color Picker end OK
#define CPN_SELENDCANCEL (WM_USER + 1005)  // Color Picker end (cancelled)

void AFXAPI DDX_Color(CDataExchange* pDX, int nIDC, COLORREF& crColor);


// CMyColorPopup window

class CMyColorPopup : public CWnd
{
// Construction
public:
	CMyColorPopup();
	virtual ~CMyColorPopup();

	bool Create(CWnd* pParent);
	void Show(const CRect& rcParent, COLORREF color);
	void Hide();

	void SetColorNames(UINT nID = 0);
	virtual void SetCustomText(LPCTSTR pszText);
	virtual void SetDefaultText(LPCTSTR pszText);
	void SetRegSection(LPCTSTR pszRegSection);

	size_t GetColorCount() const { return m_colors.size(); }
	COLORREF GetColor(size_t i) const { return m_colors[i].color; }
	CString GetColorName(size_t i) const { return m_colors[i].strName; }

// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	void InitColorTable();
	void InitLayout();
	void InitToolTips();

	void SetPosition(const CRect& rcParent);
	void SetOrigColor(COLORREF color);
	void Select(int nIndex);
	CRect GetCellRect(int nIndex);

	void EndSelection(int nMessage);
	void DrawCell(CDC* pDC, int nIndex);

	void DrawBorder(CDC* pDC, CRect rect, UINT nEdge, UINT nBorder);

// Attributes
protected:
	struct ColorEntry
	{
		COLORREF color;
		CString strName;
	};
	vector<ColorEntry> m_colors;

    int m_nColumns, m_nRows;
    int m_nBoxSize, m_nMargin;
    int m_nCurSel, m_nOrigSel;
    CString m_strDefaultText, m_strCustomText;
    CRect m_rcCustomText, m_rcDefaultText, m_rcBoxes;
	CSize m_size;
    CFont m_font;
    COLORREF m_colorOrig, m_color;
    CToolTipCtrl m_toolTip;

	bool m_bFlatMenus;

	CString	m_strRegSection;

protected:
    // Message map functions
    afx_msg void OnNcDestroy();
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    DECLARE_MESSAGE_MAP()

	friend class CMyColorPicker;
};


// CMyColorPicker button

class CMyColorPicker : public CButton
{
	DECLARE_DYNCREATE(CMyColorPicker);

public:
	CMyColorPicker();
	virtual ~CMyColorPicker();

	// Returns the current color selected in the control.
	virtual COLORREF GetColor(bool bTranslateDefault = false) const;

	// Sets the current color selected in the control.
	virtual void SetColor(COLORREF Color);

	// Returns the color associated with the 'default' selection.
	virtual COLORREF GetDefaultColor() const;

	// Sets the color associated with the 'default' selection.
	// The default value is COLOR_APPWORKSPACE.
	virtual void SetDefaultColor(COLORREF Color);

	// Sets the text to display in the 'Custom' selection of the
	// CMyColorPicker control, the default text is "More Colors...".
	virtual void SetCustomText(LPCTSTR pszText);

	// Sets the text to display in the 'Default' selection of the
	// CMyColorPicker control, the default text is "Automatic".
	// If this value is set to "", the 'Default' selection will not be shown.
	virtual void SetDefaultText(LPCTSTR pszText);

	// Sets the text of the tooltips from the resource string.
	// Set this to 0 to use original English names.
	virtual void SetColorNames(UINT nID = 0);

	// Sets the registry section where to write custom colors.
	// Set this to _T("") to disable.
	virtual void SetRegSection(LPCTSTR pszRegSection = _T(""));

	// Turns on/off the 'Track Selection' option of the control
	virtual void SetTrackSelection(bool bTrack);

	// Returns the state of the 'Track Selection' option.
	virtual bool IsTrackSelection() const;

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

protected:
	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	COLORREF m_color;
	COLORREF m_colorDefault;
	CString m_strDefaultText;
	CString m_strCustomText;
	bool m_bPopupActive;
	bool m_bTrackSelection;
	bool m_bMouseOver;
	bool m_bFlatMenus;

	CMyColorPopup m_popup;

	COffscreenDC m_offscreenDC;
	HTHEME m_hThemeButton;
	bool m_bIsDefault;

	// Message map functions
	afx_msg BOOL OnClicked();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnSelEndOK(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSelEndCancel(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSelChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnThemeChanged();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg UINT OnGetDlgCode();
	afx_msg LRESULT OnSetStyle(WPARAM wParam, LPARAM lParam);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnCancelMode();
	DECLARE_MESSAGE_MAP()
};
