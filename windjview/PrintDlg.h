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

#include "MyDialog.h"
#include "MyEdit.h"
#include "Drawing.h"

class DjVuSource;


class CPrintDlg : public CMyDialog
{
	DECLARE_DYNAMIC(CPrintDlg)

public:
	CPrintDlg(DjVuSource* pSource, int nPage, int nRotate, int nMode, CWnd* pParent = NULL);
	virtual ~CPrintDlg();

// Dialog Data
	enum { IDD = IDD_PRINT };

public:
	struct Printer
	{
		CString strPrinterName;
		CString strDriverName;
		CString strPortName;
		CString strComment;

		// Device capabilities
		bool bCanCollate;
		DWORD nMaxCopies;

		// Device capabilities that depend on current paper type
		int nPhysicalWidth;
		int nPhysicalHeight;
		int nOffsetLeft;
		int nOffsetTop;
		int nUserWidth;
		int nUserHeight;
	};

	struct Paper
	{
		Paper(const CString& name, WORD code, const CSize& sz)
			: strName(name), nCode(code), size(sz) {}

		CString strName;
		WORD nCode;
		CSize size;
	};

	Printer* m_pPrinter;
	DEVMODE* m_pDevMode;
	Paper* m_pPaper;
	HANDLE m_hPrinter;

	typedef vector<pair<int, int> > PageArr;
	PageArr m_arrPages;

	bool m_bHasSelection;
	GRect m_rcSelection;
	bool IsPrintSelection() const { return m_bHasSelection && m_nRangeType == CurrentSelection; }
	void SetCustomRange(const CString& strRange);

	DjVuSource* GetSource() const { return m_pSource; }
	int GetRotate() const { return m_nRotate; }
	int GetMode() const { return m_nMode; }

public:
	CPrintSettings m_settings;
	BOOL m_bPrintToFile;

protected:
	enum RangeType
	{
		AllPages = 0,
		CurrentPage = 1,
		CurrentSelection = 2,
		CustomRange = 3
	};

	// Dialog data
	CString m_strPages;
	int m_nRangeType;
	BOOL m_bReverse;
	CComboBox m_cboPrinter;
	CComboBox m_cboPaper;
	CComboBox m_cboPagesPerSheet;
	CComboBox m_cboPagesInRange;
	CMyEdit m_edtCopies;
	CMyEdit m_edtScale;
	CMyEdit m_edtMarginBottom;
	CMyEdit m_edtMarginTop;
	CMyEdit m_edtMarginLeft;
	CMyEdit m_edtMarginRight;
	CMyEdit m_edtPosLeft;
	CMyEdit m_edtPosTop;

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();

// Implementation
protected:
	vector<BYTE> m_printerData;
	vector<BYTE> m_devModeData;
	vector<Paper> m_paperSizes;

	CRect m_rcPreview;
	COffscreenDC m_offscreenDC;

	DjVuSource* m_pSource;
	GP<DjVuImage> m_pCurPage, m_pNextPage;
	bool m_bDrawPreview;
	int m_nCurPage;
	int m_nRotate;
	int m_nMode;
	vector<int> m_pages;

	static map<CString, vector<byte> > s_devModes;
	static LPDEVMODE GetCachedDevMode(const CString& strPrinter);
	static void UpdateDevModeCache(const CString& strPrinter, LPDEVMODE pDevMode, size_t nSize);

	void LoadPaperTypes();
	bool ParseRange();
	void SaveSettings();
	void UpdateDevMode();
	void PreviewTwoPages(CDC* pDC, const CRect& rcPage, const CSize& szPaper, double fScreenMM);

	// Message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnChangePagesPerSheet();
	afx_msg void OnChangePaper();
	afx_msg void OnChangePrinter();
	afx_msg void OnPrintRange(UINT nID);
	afx_msg void OnCopiesUpDown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateControls();
	afx_msg void OnProperties();
	afx_msg void OnUpdateDialogData();
	DECLARE_MESSAGE_MAP()
};
