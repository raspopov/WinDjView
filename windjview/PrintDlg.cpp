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

#include "stdafx.h"
#include "WinDjView.h"
#include "PrintDlg.h"

#include "DjVuDoc.h"
#include "DjVuView.h"

#include "AppSettings.h"

CString GetStatusString(DWORD dwStatus)
{
	vector<CString> status;
	if (dwStatus & PRINTER_STATUS_BUSY) // The printer is busy.
		status.push_back(_T("Busy"));
	if (dwStatus & PRINTER_STATUS_DOOR_OPEN) // The printer door is open.
		status.push_back(_T("Door open"));
	if (dwStatus & PRINTER_STATUS_ERROR) // The printer is in an error state.
		status.push_back(_T("Error"));
	if (dwStatus & PRINTER_STATUS_INITIALIZING) // The printer is initializing.
		status.push_back(_T("Initializing"));
	if (dwStatus & PRINTER_STATUS_IO_ACTIVE) // The printer is in an active input/output state.
		status.push_back(_T("Printing"));
	if (dwStatus & PRINTER_STATUS_MANUAL_FEED) // The printer is in a manual feed state.
		status.push_back(_T("Manual feed"));
	if (dwStatus & PRINTER_STATUS_NO_TONER) // The printer is out of toner.
		status.push_back(_T("Out of toner"));
	if (dwStatus & PRINTER_STATUS_NOT_AVAILABLE) // The printer is not available for printing.
		status.push_back(_T("Not available"));
	if (dwStatus & PRINTER_STATUS_OFFLINE) // The printer is offline.
		status.push_back(_T("Offline"));
	if (dwStatus & PRINTER_STATUS_OUT_OF_MEMORY) // The printer has run out of memory.
		status.push_back(_T("Out of memory"));
	if (dwStatus & PRINTER_STATUS_OUTPUT_BIN_FULL) // The printer's output bin is full.
		status.push_back(_T("Output bin full"));
	if (dwStatus & PRINTER_STATUS_PAGE_PUNT) // The printer cannot print the current page.
		status.push_back(_T("Page punted"));
	if (dwStatus & PRINTER_STATUS_PAPER_JAM) // Paper is jammed in the printer.
		status.push_back(_T("Paper jam"));
	if (dwStatus & PRINTER_STATUS_PAPER_OUT) // The printer is out of paper.
		status.push_back(_T("Out of paper"));
	if (dwStatus & PRINTER_STATUS_PAPER_PROBLEM) // The printer has a paper problem.
		status.push_back(_T("Paper problem"));
	if (dwStatus & PRINTER_STATUS_PAUSED) // The printer is paused.
		status.push_back(_T("Paused"));
	if (dwStatus & PRINTER_STATUS_PENDING_DELETION) // The printer is being deleted.
		status.push_back(_T("Deleted"));
	if (dwStatus & PRINTER_STATUS_POWER_SAVE) // The printer is in power save mode.
		status.push_back(_T("Power saving"));
	if (dwStatus & PRINTER_STATUS_PRINTING) // The printer is printing.
		status.push_back(_T("Printing"));
	if (dwStatus & PRINTER_STATUS_PROCESSING) // The printer is processing a print job.
		status.push_back(_T("Processing"));
	if (dwStatus & PRINTER_STATUS_SERVER_UNKNOWN) // The printer status is unknown.
		status.push_back(_T("Unknown"));
	if (dwStatus & PRINTER_STATUS_TONER_LOW) // The printer is low on toner.
		status.push_back(_T("Low on toner"));
	if (dwStatus & PRINTER_STATUS_USER_INTERVENTION) // The printer has an error that requires the user to do something.
		status.push_back(_T("Fatal error"));
	if (dwStatus & PRINTER_STATUS_WAITING) // The printer is waiting.
		status.push_back(_T("Waiting"));
	if (dwStatus & PRINTER_STATUS_WARMING_UP) // The printer is warming up.
		status.push_back(_T("Warming up"));

	CString strStatus;
	for (size_t i = 0; i < status.size(); ++i)
		strStatus += (i > 0 ? _T("; ") : _T("")) + status[i];
	return strStatus.IsEmpty() ? _T("Ready") : strStatus;
}

CString FormatDouble(double fValue)
{
	CString strResult;
	strResult.Format(_T("%.6f"), fValue);
	while (!strResult.IsEmpty() && strResult[strResult.GetLength() - 1] == '0')
		strResult = strResult.Left(strResult.GetLength() - 1);

	if (!strResult.IsEmpty() && strResult[strResult.GetLength() - 1] == '.')
		strResult = strResult.Left(strResult.GetLength() - 1);

	if (strResult.IsEmpty())
		strResult = _T("0");

	return strResult;
}

void AFXAPI DDX_MyText(CDataExchange* pDX, int nIDC, double& value, double def, LPCTSTR pszSuffix)
{
	CString strText = FormatDouble(value) + pszSuffix;
	DDX_Text(pDX, nIDC, strText);

	if (pDX->m_bSaveAndValidate)
	{
		if (_stscanf(strText, _T("%lf"), &value) != 1)
			value = def;
	}
}

void AFXAPI DDX_MyText(CDataExchange* pDX, int nIDC, DWORD& value, DWORD def, LPCTSTR pszSuffix)
{
	CString strText;
	strText.Format(_T("%u%s"), value, (LPCTSTR)CString(pszSuffix));
	DDX_Text(pDX, nIDC, strText);

	if (pDX->m_bSaveAndValidate)
	{
		if (_stscanf(strText, _T("%u"), &value) != 1)
			value = def;
	}
}

// CPrintDlg dialog

IMPLEMENT_DYNAMIC(CPrintDlg, CDialog)

CPrintDlg::CPrintDlg(CDjVuDoc* pDoc, int nPage, int nRotate, CWnd* pParent)
	: CDialog(CPrintDlg::IDD, pParent),
	  m_bCollate(FALSE), m_nCopies(1), m_strPages(_T("")), m_bPrintToFile(FALSE),
	  m_strStatus(_T("")), m_strType(_T("")), m_strLocation(_T("")),
	  m_strComment(_T("")), m_bLandscape(FALSE), m_nRangeType(0),
	  m_pPrinter(NULL), m_hPrinter(NULL), m_pPaper(NULL), m_bReverse(false),
	  m_pDoc(pDoc), m_nCurPage(nPage), m_nRotate(nRotate), m_bTwoPages(false)
{
}

CPrintDlg::~CPrintDlg()
{
}

void CPrintDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CENTER_IMAGE, m_settings.bCenterImage);
	DDX_Check(pDX, IDC_CLIP_CONTENT, m_settings.bClipContent);
	DDX_Check(pDX, IDC_SHRINK_OVERSIZED, m_settings.bShrinkOversized);
	DDX_Check(pDX, IDC_SCALE_TO_FIT, m_settings.bScaleToFit);
	DDX_Check(pDX, IDC_COLLATE, m_bCollate);
	DDX_Text(pDX, IDC_PAGE_RANGES, m_strPages);
	DDX_Check(pDX, IDC_PRINT_TO_FILE, m_bPrintToFile);
	DDX_Text(pDX, IDC_STATIC_STATUS, m_strStatus);
	DDX_Text(pDX, IDC_STATIC_TYPE, m_strType);
	DDX_Text(pDX, IDC_STATIC_LOCATION, m_strLocation);
	DDX_Text(pDX, IDC_STATIC_COMMENT, m_strComment);
	DDX_Radio(pDX, IDC_PORTRAIT, m_bLandscape);
	DDX_Radio(pDX, IDC_RANGE_ALL, m_nRangeType);
	DDX_Check(pDX, IDC_REVERSE, m_bReverse);
	DDX_Control(pDX, IDC_COMBO_PRINTER, m_cboPrinter);
	DDX_Control(pDX, IDC_COMBO_PAPER, m_cboPaper);
	DDX_Control(pDX, IDC_COMBO_PAGESPERSHEET, m_cboPagesPerSheet);
	DDX_Control(pDX, IDC_COMBO_PAGESINRANGE, m_cboPagesInRange);
	DDX_Control(pDX, IDC_EDIT_COPIES, m_edtCopies);
	DDX_Control(pDX, IDC_MARGIN_BOTTOM, m_edtMarginBottom);
	DDX_Control(pDX, IDC_MARGIN_TOP, m_edtMarginTop);
	DDX_Control(pDX, IDC_MARGIN_LEFT, m_edtMarginLeft);
	DDX_Control(pDX, IDC_MARGIN_RIGHT, m_edtMarginRight);
	DDX_Control(pDX, IDC_POSITION_LEFT, m_edtPosLeft);
	DDX_Control(pDX, IDC_POSITION_TOP, m_edtPosTop);
	DDX_Control(pDX, IDC_SCALE, m_edtScale);

	DDX_MyText(pDX, IDC_SCALE, m_settings.fScale, 100.0, "%");
	DDX_MyText(pDX, IDC_POSITION_LEFT, m_settings.fPosLeft);
	DDX_MyText(pDX, IDC_POSITION_TOP, m_settings.fPosTop);
	DDX_MyText(pDX, IDC_MARGIN_BOTTOM, m_settings.fMarginBottom);
	DDX_MyText(pDX, IDC_MARGIN_TOP, m_settings.fMarginTop);
	DDX_MyText(pDX, IDC_MARGIN_LEFT, m_settings.fMarginLeft);
	DDX_MyText(pDX, IDC_MARGIN_RIGHT, m_settings.fMarginRight);
	DDX_MyText(pDX, IDC_EDIT_COPIES, m_nCopies, 1);

	if (!pDX->m_bSaveAndValidate)
	{
		CString strPaperSize;
		if (m_pPaper != NULL)
		{
			CSize szPaper = m_pPaper->size;
			if (m_bLandscape)
				swap(szPaper.cx, szPaper.cy);

			strPaperSize.Format(_T("%s x %s mm"),
				(LPCTSTR)FormatDouble(szPaper.cx / 10.0),
				(LPCTSTR)FormatDouble(szPaper.cy / 10.0));
		}

		DDX_Text(pDX, IDC_STATIC_PAPER, strPaperSize);
	}
}


BEGIN_MESSAGE_MAP(CPrintDlg, CDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_CENTER_IMAGE, OnUpdateDialogData)
	ON_BN_CLICKED(IDC_CLIP_CONTENT, OnUpdateDialogData)
	ON_CBN_SELCHANGE(IDC_COMBO_PAGESPERSHEET, OnChangePagesPerSheet)
	ON_CBN_SELCHANGE(IDC_COMBO_PAPER, OnChangePaper)
	ON_CBN_SELCHANGE(IDC_COMBO_PRINTER, OnChangePrinter)
	ON_BN_CLICKED(IDC_LANDSCAPE, OnUpdateDialogData)
	ON_BN_CLICKED(IDC_PORTRAIT, OnUpdateDialogData)
	ON_BN_CLICKED(IDC_RANGE_ALL, OnPrintRange)
	ON_BN_CLICKED(IDC_RANGE_CURRENT, OnPrintRange)
	ON_BN_CLICKED(IDC_RANGE_PAGES, OnPrintRange)
	ON_BN_CLICKED(IDC_SCALE_TO_FIT, OnUpdateDialogData)
	ON_BN_CLICKED(IDC_SHRINK_OVERSIZED, OnUpdateDialogData)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_COPIES, OnCopiesUpDown)
	ON_BN_CLICKED(IDC_PROPERTIES, OnProperties)
	ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
	ON_EN_KILLFOCUS(IDC_EDIT_COPIES, OnUpdateDialogData)
	ON_EN_KILLFOCUS(IDC_MARGIN_LEFT, OnUpdateDialogData)
	ON_EN_KILLFOCUS(IDC_MARGIN_TOP, OnUpdateDialogData)
	ON_EN_KILLFOCUS(IDC_MARGIN_RIGHT, OnUpdateDialogData)
	ON_EN_KILLFOCUS(IDC_MARGIN_BOTTOM, OnUpdateDialogData)
	ON_EN_KILLFOCUS(IDC_POSITION_LEFT, OnUpdateDialogData)
	ON_EN_KILLFOCUS(IDC_POSITION_TOP, OnUpdateDialogData)
	ON_EN_KILLFOCUS(IDC_SCALE, OnUpdateDialogData)
	ON_BN_CLICKED(IDC_COLLATE, OnUpdateDialogData)
END_MESSAGE_MAP()


// CPrintDlg message handlers

BOOL CPrintDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ASSERT(m_hPrinter == NULL);

	m_edtCopies.SetInteger();
	m_edtPosLeft.SetReal();
	m_edtPosTop.SetReal();
	m_edtMarginLeft.SetReal();
	m_edtMarginTop.SetReal();
	m_edtMarginRight.SetReal();
	m_edtMarginBottom.SetReal();
	m_edtScale.SetReal();
	m_edtScale.SetPercent();

	// Get preview rect
	GetDlgItem(IDC_PREVIEW_AREA)->GetWindowRect(m_rcPreview);
	ScreenToClient(m_rcPreview);

	// Get default printer
	CString strDefault, strTemp;
	::GetProfileString(_T("windows"), _T("device"), strTemp.GetBufferSetLength(1),
		strDefault.GetBufferSetLength(1000), 1000);
	strTemp.ReleaseBuffer();
	strDefault.ReleaseBuffer();

	// Quick enum all printers
	DWORD cbNeeded, nPrinters;
	
	OSVERSIONINFO vi;
	if (::GetVersionEx(&vi) && vi.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		// use PRINTER_INFO_4
		::EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
			NULL, 4, NULL, 0, &cbNeeded, &nPrinters);

		vector<BYTE> buf(cbNeeded + 1);
		::EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
			NULL, 4, &buf[0], cbNeeded, &cbNeeded, &nPrinters);

		PRINTER_INFO_4* pPrinter = reinterpret_cast<PRINTER_INFO_4*>(&buf[0]);
		for (size_t i = 0; i < nPrinters; ++i, ++pPrinter)
			m_cboPrinter.AddString(pPrinter->pPrinterName);
	}
	else
	{
		// use PRINTER_INFO_5
		::EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
			NULL, 5, NULL, 0, &cbNeeded, &nPrinters);

		vector<BYTE> buf(cbNeeded + 1);
		::EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
			NULL, 5, &buf[0], cbNeeded, &cbNeeded, &nPrinters);

		PRINTER_INFO_5* pPrinter = reinterpret_cast<PRINTER_INFO_5*>(&buf[0]);
		for (size_t i = 0; i < nPrinters; ++i, ++pPrinter)
			m_cboPrinter.AddString(pPrinter->pPrinterName);
	}

	for (int i = 0; i < m_cboPrinter.GetCount(); ++i)
	{
		CString strPrinter;
		m_cboPrinter.GetLBText(i, strPrinter);
		if (CAppSettings::strPrinter == strPrinter)
		{
			m_cboPrinter.SetCurSel(i);
			break;
		}
	}

	if (m_cboPrinter.GetCurSel() == -1)
	{
		for (int i = 0; i < m_cboPrinter.GetCount(); ++i)
		{
			CString strPrinter;
			m_cboPrinter.GetLBText(i, strPrinter);
			if (strDefault.Find(strPrinter) == 0)
			{
				m_cboPrinter.SetCurSel(i);
				break;
			}
		}
	}

	if (m_cboPrinter.GetCurSel() == -1)
		m_cboPrinter.SetCurSel(0);

	m_bLandscape = CAppSettings::bLandscape;
	m_nCopies = CAppSettings::nCopies;
	m_bCollate = CAppSettings::bCollate;
	m_nPaperCode = CAppSettings::nPaper;
	m_bTwoPages = CAppSettings::bTwoPages;
	m_settings = CAppSettings::printSettings;

	UpdateData(false);

	OnChangePrinter();

	m_cboPagesInRange.AddString(_T("All Pages In Range"));
	m_cboPagesInRange.AddString(_T("Odd Pages"));
	m_cboPagesInRange.AddString(_T("Even Pages"));
	m_cboPagesInRange.SetCurSel(0);

	m_cboPagesPerSheet.AddString(_T("One Page Per Sheet"));
	m_cboPagesPerSheet.AddString(_T("Two Pages Per Sheet"));
	m_cboPagesPerSheet.SetCurSel(m_bTwoPages ? 1 : 0);

	return true;
}

void CPrintDlg::OnOK()
{
	OnUpdateDialogData();

	SaveSettings();

	if (m_hPrinter == NULL)
		EndDialog(IDCANCEL);

	// Printer handle and info struct are returned to the caller
	ASSERT(m_hPrinter != NULL && m_pPrinter != NULL && m_pPaper != NULL);

	// Parse page range
	if (m_nRangeType == 0)
	{
		for (int i = 1; i <= m_pDoc->m_pDjVuDoc->get_pages_num(); ++i)
			m_pages.insert(i);
	}
	else if (m_nRangeType == 1)
	{
		m_pages.insert(m_nCurPage + 1);
	}
	else
	{
		if (!ParseRange())
			return;
	}

	bool bAllPages = (m_cboPagesInRange.GetCurSel() == 0);
	bool bOddPages = (m_cboPagesInRange.GetCurSel() == 1);
	bool bEvenPages = (m_cboPagesInRange.GetCurSel() == 2);
	int i = 1;

	for (set<int>::iterator it = m_pages.begin(); it != m_pages.end(); ++it, ++i)
	{
		if (!m_bTwoPages)
		{
			if (bAllPages || bOddPages && (i % 2) == 1 || bEvenPages && (i % 2) == 0)
				m_arrPages.push_back(make_pair(*it, 0));
		}
		else
		{
			int nFirst = *it++;
			if (it == m_pages.end())
			{
				if (bAllPages || bOddPages && (i % 2) == 1 || bEvenPages && (i % 2) == 0)
					m_arrPages.push_back(make_pair(nFirst, 0));
				break;
			}
			else
			{
				if (bAllPages || bOddPages && (i % 2) == 1 || bEvenPages && (i % 2) == 0)
					m_arrPages.push_back(make_pair(nFirst, *it));
			}
		}
	}

	if (m_bReverse)
		reverse(m_arrPages.begin(), m_arrPages.end());

	CDialog::OnOK();
}

void CPrintDlg::OnCancel()
{
	SaveSettings();

	if (m_hPrinter != NULL)
		::ClosePrinter(m_hPrinter);

	CDialog::OnCancel();
}

void CPrintDlg::OnPaint()
{
	CPaintDC paintDC(this);

	if (m_bitmap.m_hObject == NULL)
	{
		m_bitmap.CreateCompatibleBitmap(&paintDC,
			m_rcPreview.Width(), m_rcPreview.Height());
	}

	CDC dc;
	dc.CreateCompatibleDC(&paintDC);
	CBitmap* pBmpOld = dc.SelectObject(&m_bitmap);

	CRect rcPreview = m_rcPreview - m_rcPreview.TopLeft();
	dc.FillSolidRect(rcPreview, ::GetSysColor(COLOR_BTNFACE));

	if (m_pPaper != NULL && m_pPaper->size.cx != 0 && m_pPaper->size.cy != 0)
	{
		CSize szPaper = m_pPaper->size;
		if (m_bLandscape)
			swap(szPaper.cx, szPaper.cy);

		rcPreview.DeflateRect(1, 1, 6, 6);

		CSize szPage;
		szPage.cx = rcPreview.Width();
		szPage.cy = szPage.cx * szPaper.cy / szPaper.cx;
		if (szPage.cy > rcPreview.Height())
		{
			szPage.cy = rcPreview.Height();
			szPage.cx = szPage.cy * szPaper.cx / szPaper.cy;
		}

		CPoint ptTopLeft = rcPreview.TopLeft();
		ptTopLeft.Offset((rcPreview.Width() - szPage.cx) / 2, (rcPreview.Height() - szPage.cy) / 2);
		CRect rcPage(ptTopLeft, szPage);

		dc.FillSolidRect(rcPage + CPoint(6, 6), ::GetSysColor(COLOR_BTNSHADOW));
		dc.FillSolidRect(rcPage, ::GetSysColor(COLOR_WINDOW));

		CRect rcFrame = rcPage;
		rcFrame.InflateRect(1, 1);
		dc.FrameRect(rcFrame, &CBrush(::GetSysColor(COLOR_WINDOWFRAME)));

		if (m_pCurPage == NULL)
		{
			GP<DjVuDocument> pDoc = m_pDoc->m_pDjVuDoc;
			m_pCurPage = pDoc->get_page(m_nCurPage);
			if (m_pCurPage != NULL)
				m_pCurPage->set_rotate(m_nRotate);
		}

		if (!m_bTwoPages)
		{
			double fScreenMM = rcPage.Width()*10.0 / szPaper.cx;
			PrintPage(&dc, m_pCurPage, rcPage, fScreenMM, fScreenMM, m_settings, true);
		}
		else
		{
			if (m_pNextPage == NULL)
			{
				GP<DjVuDocument> pDoc = m_pDoc->m_pDjVuDoc;
				m_pNextPage = pDoc->get_page(m_nCurPage + 1);
				if (m_pNextPage != NULL)
					m_pNextPage->set_rotate(m_nRotate);
			}

			PreviewTwoPages(&dc, rcPage, szPaper);
		}
	}

	paintDC.BitBlt(m_rcPreview.left, m_rcPreview.top, m_rcPreview.Width(), m_rcPreview.Height(),
		&dc, 0, 0, SRCCOPY);

	dc.SelectObject(pBmpOld);
}

void CPrintDlg::PreviewTwoPages(CDC* pDC, const CRect& rcPage, const CSize& szPaper)
{
	CSize szHalf = szPaper;
	CRect rcFirstHalf = rcPage, rcSecondHalf = rcPage;
	if (m_bLandscape)
	{
		szHalf.cx = szHalf.cx / 2;
		rcFirstHalf.right = rcPage.CenterPoint().x;
		rcSecondHalf.left = rcFirstHalf.right;
	}
	else
	{
		szHalf.cy = szHalf.cy / 2;
		rcFirstHalf.bottom = rcPage.CenterPoint().y;
		rcSecondHalf.top = rcFirstHalf.bottom;
	}

	double fScreenMM = rcPage.Width()*10.0 / szPaper.cx;
	PrintPage(pDC, m_pCurPage, rcFirstHalf, fScreenMM, fScreenMM, m_settings, true);
	PrintPage(pDC, m_pNextPage, rcSecondHalf, fScreenMM, fScreenMM, m_settings, true);
}

void CPrintDlg::OnChangePagesPerSheet()
{
	UpdateData();

	bool bTwoPages = m_cboPagesPerSheet.GetCurSel() == 1;
	if (bTwoPages == m_bTwoPages)
		return;

	m_bTwoPages = bTwoPages;
	m_bLandscape = !m_bLandscape;

	UpdateData(false);
	UpdateDevMode();

	InvalidateRect(m_rcPreview, 0);
	UpdateWindow();
}

void CPrintDlg::OnChangePaper()
{
	int nItem = m_cboPaper.GetCurSel();
	if (nItem == -1)
		return;

	m_nPaperCode = (WORD)m_cboPaper.GetItemData(nItem);
	m_pPaper = &m_paperSizes[nItem];

	UpdateData(false);
	UpdateDevMode();

	InvalidateRect(m_rcPreview, 0);
	UpdateWindow();
}

void CPrintDlg::OnChangePrinter()
{
	if (!UpdateData())
		return;

	if (m_hPrinter != NULL)
		::ClosePrinter(m_hPrinter);
	m_hPrinter = NULL;
	m_pPrinter = NULL;
	m_pPaper = NULL;

	int nPrinter = m_cboPrinter.GetCurSel();
	if (nPrinter == -1)
	{
		m_strStatus.Empty();
		m_strLocation.Empty();
		m_strComment.Empty();
		m_strType.Empty();

		UpdateData(false);
		return;
	}

	CString strPrinter;
	m_cboPrinter.GetLBText(nPrinter, strPrinter);

	// Get full information about the printer into PRINTER_INFO_2 member variable
	::OpenPrinter(strPrinter.GetBuffer(), &m_hPrinter, NULL);

	DWORD cbNeeded;
	::GetPrinter(m_hPrinter, 2, NULL, 0, &cbNeeded);

	m_printerData.resize(cbNeeded + 1);
	::GetPrinter(m_hPrinter, 2, &m_printerData[0], cbNeeded, &cbNeeded);
	m_pPrinter = reinterpret_cast<PRINTER_INFO_2*>(&m_printerData[0]);

	m_strComment = m_pPrinter->pComment;
	m_strLocation = m_pPrinter->pPortName;
	m_strType = m_pPrinter->pDriverName;
	m_strStatus = GetStatusString(m_pPrinter->Status);

	LoadPaperTypes();

	m_nMaxCopies = ::DeviceCapabilities(m_pPrinter->pPrinterName, m_pPrinter->pPortName,
		DC_COPIES, NULL, m_pPrinter->pDevMode);
	if (m_nMaxCopies <= 1)
		m_nMaxCopies = 9999;

	if ((m_pPrinter->pDevMode->dmFields & DM_COPIES) == 0)
		m_nCopies = 1;
	else
		m_nCopies = min(m_nCopies, m_nMaxCopies);

	m_bCanCollate = ::DeviceCapabilities(m_pPrinter->pPrinterName, m_pPrinter->pPortName,
		DC_COLLATE, NULL, m_pPrinter->pDevMode) > 0;
	if (!m_bCanCollate)
		m_bCollate = false;

	UpdateData(false);
	UpdateDevMode();

	OnChangePaper();
}

void CPrintDlg::LoadPaperTypes()
{
	int nPapers = ::DeviceCapabilities(m_pPrinter->pPrinterName, m_pPrinter->pPortName,
		DC_PAPERNAMES, NULL, m_pPrinter->pDevMode);
	CString strPaperNames;
	::DeviceCapabilities(m_pPrinter->pPrinterName, m_pPrinter->pPortName,
		DC_PAPERNAMES, strPaperNames.GetBufferSetLength(nPapers*64), m_pPrinter->pDevMode);

	int nSizeNames = ::DeviceCapabilities(m_pPrinter->pPrinterName, m_pPrinter->pPortName,
		DC_PAPERS, NULL, m_pPrinter->pDevMode);
	vector<WORD> size_names(nSizeNames);
	::DeviceCapabilities(m_pPrinter->pPrinterName, m_pPrinter->pPortName,
		DC_PAPERS, (LPTSTR)&size_names[0], m_pPrinter->pDevMode);

	int nSizes = ::DeviceCapabilities(m_pPrinter->pPrinterName, m_pPrinter->pPortName,
		DC_PAPERSIZE, NULL, m_pPrinter->pDevMode);
	vector<POINT> sizes(nSizes);
	::DeviceCapabilities(m_pPrinter->pPrinterName, m_pPrinter->pPortName,
		DC_PAPERSIZE, (LPTSTR)&sizes[0], m_pPrinter->pDevMode);

	// Retain selected paper size
	for (int i = 0; i < nPapers && i < nSizeNames && i < nSizes; ++i)
	{
		if (size_names[i] == m_nPaperCode)
		{
			m_pPrinter->pDevMode->dmPaperSize = m_nPaperCode;
			break;
		}
	}

	m_cboPaper.ResetContent();
	m_paperSizes.clear();
	TCHAR szPaperName[65];
	for (int i = 0; i < nPapers && i < nSizeNames && i < nSizes; ++i)
	{
		ZeroMemory(szPaperName, sizeof(szPaperName));
		_tcsncpy(szPaperName, (LPCTSTR)strPaperNames + i*64, 64);

		if (size_names[i] != DMPAPER_USER)
		{
			int nItem = m_cboPaper.AddString(szPaperName);
			m_cboPaper.SetItemData(nItem, size_names[i]);

			if (size_names[i] == m_pPrinter->pDevMode->dmPaperSize)
				m_cboPaper.SetCurSel(nItem);

			m_paperSizes.push_back(Paper(szPaperName, size_names[i], sizes[i]));
		}
	}

	if (m_cboPaper.GetCurSel() == -1)
		m_cboPaper.SetCurSel(0);
}

void CPrintDlg::OnPrintRange()
{
	if (m_pPrinter == NULL)
		return;

	UpdateData();
}


void CPrintDlg::OnCopiesUpDown(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_pPrinter == NULL)
		return;

	UpdateData();

	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	if (pNMUpDown->iDelta < 0)
		++m_nCopies;
	else
		--m_nCopies;

	if (m_nCopies < 1)
		m_nCopies = 1;
	else if (m_nCopies > m_nMaxCopies)
		m_nCopies = m_nMaxCopies;

	UpdateData(false);
	UpdateDevMode();

	*pResult = 0;
}

void CPrintDlg::OnProperties()
{
	if (m_hPrinter == NULL)
		return;

	::DocumentProperties(m_hWnd, m_hPrinter, m_pPrinter->pPrinterName,
			m_pPrinter->pDevMode, m_pPrinter->pDevMode,
			DM_IN_PROMPT | DM_IN_BUFFER | DM_OUT_BUFFER);

	m_bLandscape = (m_pPrinter->pDevMode->dmOrientation == DMORIENT_LANDSCAPE);
	m_nCopies = m_pPrinter->pDevMode->dmCopies;
	m_bCollate = (m_pPrinter->pDevMode->dmCollate == DMCOLLATE_TRUE);

	UpdateData(false);

	m_pPaper = NULL;
	m_nPaperCode = m_pPrinter->pDevMode->dmPaperSize;
	LoadPaperTypes();
	OnChangePaper();
}

void CPrintDlg::OnKickIdle()
{
	bool bOk = (m_pPrinter != NULL);

	m_cboPaper.EnableWindow(bOk);
	GetDlgItem(IDC_PORTRAIT)->EnableWindow(bOk && (m_pPrinter->pDevMode->dmFields & DM_ORIENTATION));
	GetDlgItem(IDC_LANDSCAPE)->EnableWindow(bOk && (m_pPrinter->pDevMode->dmFields & DM_ORIENTATION));

	GetDlgItem(IDC_EDIT_COPIES)->EnableWindow(bOk && (m_pPrinter->pDevMode->dmFields & DM_COPIES));
	GetDlgItem(IDC_SPIN_COPIES)->EnableWindow(bOk && (m_pPrinter->pDevMode->dmFields & DM_COPIES));

	GetDlgItem(IDC_COLLATE)->EnableWindow(bOk && m_nCopies >= 2 && m_bCanCollate);

	GetDlgItem(IDC_RANGE_ALL)->EnableWindow(bOk);
	GetDlgItem(IDC_RANGE_CURRENT)->EnableWindow(bOk);
	GetDlgItem(IDC_RANGE_PAGES)->EnableWindow(bOk);

	GetDlgItem(IDC_PAGE_RANGES)->EnableWindow(bOk && m_nRangeType == 2);

	GetDlgItem(IDC_SCALE)->EnableWindow(bOk && !m_settings.bScaleToFit);
	GetDlgItem(IDC_SHRINK_OVERSIZED)->EnableWindow(bOk && !m_settings.bScaleToFit);

	GetDlgItem(IDC_POSITION_TOP)->EnableWindow(bOk && !m_settings.bCenterImage);
	GetDlgItem(IDC_POSITION_LEFT)->EnableWindow(bOk && !m_settings.bCenterImage);

	GetDlgItem(IDC_MARGIN_LEFT)->EnableWindow(bOk);
	GetDlgItem(IDC_MARGIN_TOP)->EnableWindow(bOk);
	GetDlgItem(IDC_MARGIN_RIGHT)->EnableWindow(bOk);
	GetDlgItem(IDC_MARGIN_BOTTOM)->EnableWindow(bOk);

	m_cboPaper.EnableWindow(bOk);
	m_cboPagesInRange.EnableWindow(bOk);
	m_cboPagesPerSheet.EnableWindow(bOk);

	GetDlgItem(IDOK)->EnableWindow(bOk);
}

void CPrintDlg::OnUpdateDialogData()
{
	UpdateData();

	if (m_nCopies > m_nMaxCopies)
		m_nCopies = m_nMaxCopies;
	else if (m_nCopies < 1)
		m_nCopies = 1;

	UpdateData(false);
	UpdateDevMode();

	InvalidateRect(m_rcPreview, 0);
	UpdateWindow();
}

void CPrintDlg::UpdateDevMode()
{
	if (m_pPrinter == NULL)
		return;

	m_pPrinter->pDevMode->dmPaperSize = m_nPaperCode;
	m_pPrinter->pDevMode->dmOrientation = (m_bLandscape ? DMORIENT_LANDSCAPE : DMORIENT_PORTRAIT);
	m_pPrinter->pDevMode->dmCollate = (m_bCollate ? DMCOLLATE_TRUE : DMCOLLATE_FALSE);
	m_pPrinter->pDevMode->dmCopies = (WORD)m_nCopies;
}

void CPrintDlg::SaveSettings()
{
	if (m_pPrinter == NULL)
		return;

	CAppSettings::strPrinter = m_pPrinter->pPrinterName;
	CAppSettings::bTwoPages = m_bTwoPages;
	CAppSettings::nPaper = m_nPaperCode;
	CAppSettings::bLandscape = !!m_bLandscape;
	CAppSettings::nCopies = m_nCopies;
	CAppSettings::bCollate = !!m_bCollate;

	CAppSettings::printSettings = m_settings;
}

bool CPrintDlg::ParseRange()
{
	int i = 0;
	while (i < m_strPages.GetLength())
	{
		if (m_strPages[i] < '0' || m_strPages[i] > '9')
			return false;

		int num = 0;
		while (i < m_strPages.GetLength() && m_strPages[i] >= '0' && m_strPages[i] <= '9')
			num = 10*num + m_strPages[i++] - '0';

		int num2 = num;
		if (i < m_strPages.GetLength())
		{
			if (m_strPages[i] == '-')
			{
				++i;

				if (m_strPages[i] < '0' || m_strPages[i] > '9')
					return false;

				num2 = 0;
				while (i < m_strPages.GetLength() && m_strPages[i] >= '0' && m_strPages[i] <= '9')
					num2 = 10*num2 + m_strPages[i++] - '0';

				if (i < m_strPages.GetLength())
				{
					if (m_strPages[i] == ',' || m_strPages[i] == ';')
						++i;
					else
						return false;
				}
			}
			else if (m_strPages[i] == ',' || m_strPages[i] == ';')
				++i;
			else
				return false;
		}

		for (int j = num; j <= num2; ++j)
			m_pages.insert(j);
	}

	return true;
}
