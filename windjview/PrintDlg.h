//	WinDjView 0.1
//	Copyright (C) 2004 Andrew Zhezherun
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
//  http://www.gnu.org/copyleft/gpl.html

// $Id$

#pragma once

#include "MyEdit.h"
#include "Drawing.h"

class CDjVuDoc;


class CPrintDlg : public CDialog
{
	DECLARE_DYNAMIC(CPrintDlg)

public:
	CPrintDlg(CDjVuDoc* pDoc, int nPage, int nRotate, CWnd* pParent = NULL);   // standard constructor
	virtual ~CPrintDlg();

// Dialog Data
	enum { IDD = IDD_PRINT };

public:
	struct Paper
	{
		Paper(const CString& name, WORD code, const CSize& sz)
			: strName(name), nCode(code), size(sz) {}

		CString strName;
		WORD nCode;
		CSize size;
	};

	PRINTER_INFO_2* m_pPrinter;
	Paper* m_pPaper;
	HANDLE m_hPrinter;
	bool m_bTwoPages;

	typedef vector<pair<int, int> > PageArr;
	PageArr m_arrPages;

	CDjVuDoc* GetDocument() const { return m_pDoc; }
	int GetRotate() const { return m_nRotate; }

public:
	CPrintSettings m_settings;
	BOOL m_bLandscape;
	BOOL m_bPrintToFile;

protected:
	BOOL m_bCollate;
	DWORD m_nCopies;
	CString m_strStatus;
	CString m_strType;
	CString m_strLocation;
	CString m_strComment;
	CString m_strPages;
	int m_nRangeType;
	BOOL m_bReverse;
	CComboBox m_cboPrinter;
	CComboBox m_cboPaper;
	CComboBox m_cboPagesPerSheet;
	CComboBox m_cboPagesInRange;
	CMyEdit m_edtCopies;
	CMyEdit m_edtMarginBottom;
	CMyEdit m_edtMarginTop;
	CMyEdit m_edtMarginLeft;
	CMyEdit m_edtMarginRight;
	CMyEdit m_edtPosLeft;
	CMyEdit m_edtPosTop;
	CMyEdit m_edtScale;

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();

// Implementation
protected:
	bool m_bCanCollate;
	DWORD m_nMaxCopies;

	vector<BYTE> m_printerData;
	vector<Paper> m_paperSizes;
	WORD m_nPaperCode;

	CRect m_rcPreview;
	CBitmap m_bitmap;

	CDjVuDoc* m_pDoc;
	GP<DjVuImage> m_pCurPage, m_pNextPage;
	int m_nCurPage, m_nRotate;
	set<int> m_pages;

	void LoadPaperTypes();
	bool ParseRange();
	void SaveSettings();
	void UpdateDevMode();
	void PreviewTwoPages(CDC* pDC, const CRect& rcPage, const CSize& szPaper);

// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnChangePagesPerSheet();
	afx_msg void OnChangePaper();
	afx_msg void OnChangePrinter();
	afx_msg void OnPrintRange();
	afx_msg void OnCopiesUpDown(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKickIdle();
	afx_msg void OnProperties();
	afx_msg void OnUpdateDialogData();
	DECLARE_MESSAGE_MAP()
};
