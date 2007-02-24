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
#include "PrintDlg.h"

#include "DjVuDoc.h"
#include "DjVuView.h"

#include "AppSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPrintDlg dialog

map<CString, vector<byte> > CPrintDlg::s_devModes;

IMPLEMENT_DYNAMIC(CPrintDlg, CDialog)

CPrintDlg::CPrintDlg(CDjVuDoc* pDoc, int nPage, int nRotate, int nMode, CWnd* pParent)
	: CDialog(CPrintDlg::IDD, pParent),
	  m_strPages(_T("")), m_bPrintToFile(FALSE), m_strType(_T("")), m_strLocation(_T("")),
	  m_strComment(_T("")), m_nRangeType(0), m_pPrinter(NULL), m_hPrinter(NULL),
	  m_pPaper(NULL), m_bReverse(false), m_pDoc(pDoc), m_pSource(pDoc->GetSource()),
	  m_nCurPage(nPage), m_nRotate(nRotate), m_nMode(nMode), m_bPrinterCanCollate(false)
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
	DDX_Check(pDX, IDC_IGNORE_MARGINS, m_settings.bIgnorePrinterMargins);
	DDX_Radio(pDX, IDC_PORTRAIT, m_settings.bLandscape);
	DDX_Check(pDX, IDC_COLLATE, m_settings.bCollate);
	DDX_Text(pDX, IDC_PAGE_RANGES, m_strPages);
	DDX_Check(pDX, IDC_PRINT_TO_FILE, m_bPrintToFile);
	DDX_Text(pDX, IDC_STATIC_TYPE, m_strType);
	DDX_Text(pDX, IDC_STATIC_LOCATION, m_strLocation);
	DDX_Text(pDX, IDC_STATIC_COMMENT, m_strComment);
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

	DDX_MyText(pDX, IDC_SCALE, m_settings.fScale, 100.0, _T("%"));
	DDX_MyText(pDX, IDC_POSITION_LEFT, m_settings.fPosLeft);
	DDX_MyText(pDX, IDC_POSITION_TOP, m_settings.fPosTop);
	DDX_MyText(pDX, IDC_MARGIN_BOTTOM, m_settings.fMarginBottom);
	DDX_MyText(pDX, IDC_MARGIN_TOP, m_settings.fMarginTop);
	DDX_MyText(pDX, IDC_MARGIN_LEFT, m_settings.fMarginLeft);
	DDX_MyText(pDX, IDC_MARGIN_RIGHT, m_settings.fMarginRight);
	DDX_MyText(pDX, IDC_EDIT_COPIES, m_settings.nCopies, 1);

	if (!pDX->m_bSaveAndValidate)
	{
		CString strPaperSize;
		if (m_pPaper != NULL)
		{
			CSize szPaper = m_pPaper->size;
			if (m_settings.bLandscape)
				swap(szPaper.cx, szPaper.cy);

			int nUnits = theApp.GetAppSettings()->nUnits;
			double fUnits = CAppSettings::unitsPerInch[nUnits] / 254.0;

			CString strUnits;
			AfxExtractSubString(strUnits, LoadString(IDS_UNITS_SHORT), nUnits, ',');

			strPaperSize.Format(IDS_PAPER_SIZE,
				FormatDouble(static_cast<int>(szPaper.cx * fUnits * 100.0 + 0.5) * 0.01),
				FormatDouble(static_cast<int>(szPaper.cy * fUnits * 100.0 + 0.5) * 0.01),
				strUnits);
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
	ON_BN_CLICKED(IDC_IGNORE_MARGINS, OnUpdateDialogData)
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

	if (IsWinNT())
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
		if (m_settings.strPrinter == strPrinter)
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

	m_settings = *theApp.GetPrintSettings();

	UpdateData(false);

	OnChangePrinter();

	m_cboPagesInRange.AddString(LoadString(IDS_PRINT_ALL_PAGES));
	m_cboPagesInRange.AddString(LoadString(IDS_PRINT_ODD_PAGES));
	m_cboPagesInRange.AddString(LoadString(IDS_PRINT_EVEN_PAGES));
	m_cboPagesInRange.SetCurSel(0);

	m_cboPagesPerSheet.AddString(LoadString(IDS_PRINT_ONE_PAGE));
	m_cboPagesPerSheet.AddString(LoadString(IDS_PRINT_TWO_PAGES));
	m_cboPagesPerSheet.SetCurSel(m_settings.bTwoPages ? 1 : 0);

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
	m_pages.clear();
	if (m_nRangeType == 0)
	{
		for (int i = 1; i <= m_pSource->GetPageCount(); ++i)
			m_pages.push_back(i);
	}
	else if (m_nRangeType == 1)
	{
		m_pages.push_back(m_nCurPage + 1);
		if (m_settings.bTwoPages && m_nCurPage < m_pSource->GetPageCount() - 1)
			m_pages.push_back(m_nCurPage + 2);
	}
	else
	{
		if (!ParseRange())
			return;
	}

	bool bAllPages = (m_cboPagesInRange.GetCurSel() == 0) || m_nRangeType == 1;
	bool bOddPages = (m_cboPagesInRange.GetCurSel() == 1) && m_nRangeType != 1;
	bool bEvenPages = (m_cboPagesInRange.GetCurSel() == 2) && m_nRangeType != 1;
	int i = 1;
	int inc = (m_settings.bTwoPages ? 2 : 1);

	for (size_t j = 0; j < m_pages.size(); j += inc, ++i)
	{
		int nFirst = m_pages[j];
		if (!m_settings.bTwoPages)
		{
			if (bAllPages || bOddPages && (nFirst % 2) == 1 || bEvenPages && (nFirst % 2) == 0)
				m_arrPages.push_back(make_pair(nFirst, 0));
		}
		else
		{
			if (j + 1 == m_pages.size())
			{
				if (bAllPages || bOddPages && (i % 2) == 1 || bEvenPages && (i % 2) == 0)
					m_arrPages.push_back(make_pair(nFirst, 0));
				break;
			}
			else
			{
				if (bAllPages || bOddPages && (i % 2) == 1 || bEvenPages && (i % 2) == 0)
					m_arrPages.push_back(make_pair(nFirst, m_pages[j + 1]));
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

	m_offscreenDC.Create(&paintDC, m_rcPreview.Size());

	CRect rcPreview = m_rcPreview - m_rcPreview.TopLeft();
	m_offscreenDC.FillSolidRect(rcPreview, ::GetSysColor(COLOR_BTNFACE));

	if (m_pPaper != NULL && m_pPaper->size.cx != 0 && m_pPaper->size.cy != 0)
	{
		CSize szPaper = m_pPaper->size;
		if (m_settings.bLandscape)
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

		m_offscreenDC.FillSolidRect(rcPage + CPoint(6, 6), ::GetSysColor(COLOR_BTNSHADOW));
		m_offscreenDC.FillSolidRect(rcPage, ::GetSysColor(COLOR_WINDOW));

		CRect rcFrame = rcPage;
		rcFrame.InflateRect(1, 1);
		FrameRect(&m_offscreenDC, rcFrame, ::GetSysColor(COLOR_WINDOWFRAME));

		if (m_pCurPage == NULL)
			m_pCurPage = m_pSource->GetPage(m_nCurPage, NULL);

		double fScreenMM = rcPage.Width()*10.0 / szPaper.cx;

		if (!m_settings.bIgnorePrinterMargins)
		{
			CDC infoDC;
			infoDC.Attach(::CreateIC(m_pPrinter->pDriverName, m_pPrinter->pPrinterName, NULL, m_pDevMode));
			ASSERT(infoDC.m_hDC);

			int nPhysicalWidth = infoDC.GetDeviceCaps(PHYSICALWIDTH);
			int nPhysicalHeight = infoDC.GetDeviceCaps(PHYSICALHEIGHT);
			int nOffsetLeft = infoDC.GetDeviceCaps(PHYSICALOFFSETX);
			int nOffsetTop = infoDC.GetDeviceCaps(PHYSICALOFFSETY);
			int nOffsetRight = nPhysicalWidth - nOffsetLeft - infoDC.GetDeviceCaps(HORZRES);
			int nOffsetBottom = nPhysicalHeight - nOffsetTop - infoDC.GetDeviceCaps(VERTRES);

			rcPage.DeflateRect(nOffsetLeft * szPage.cx / nPhysicalWidth,
				nOffsetTop * szPage.cy / nPhysicalHeight,
				nOffsetRight * szPage.cx / nPhysicalWidth,
				nOffsetBottom * szPage.cy / nPhysicalHeight);
		}

		if (!m_settings.bTwoPages)
		{
			PrintPage(&m_offscreenDC, m_pCurPage, m_nRotate, m_nMode,
				rcPage, fScreenMM, fScreenMM, m_settings, true);
		}
		else
		{
			if (m_pNextPage == NULL && m_nCurPage < m_pSource->GetPageCount() - 1)
				m_pNextPage = m_pSource->GetPage(m_nCurPage + 1, NULL);

			PreviewTwoPages(&m_offscreenDC, rcPage, szPaper, fScreenMM);
		}
	}

	paintDC.BitBlt(m_rcPreview.left, m_rcPreview.top, m_rcPreview.Width(), m_rcPreview.Height(),
		&m_offscreenDC, 0, 0, SRCCOPY);

	m_offscreenDC.Release();
}

void CPrintDlg::PreviewTwoPages(CDC* pDC, const CRect& rcPage, const CSize& szPaper, double fScreenMM)
{
	CSize szHalf = szPaper;
	CRect rcFirstHalf = rcPage, rcSecondHalf = rcPage;
	if (m_settings.bLandscape)
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

	PrintPage(pDC, m_pCurPage, m_nRotate, m_nMode, rcFirstHalf, fScreenMM, fScreenMM, m_settings, true);

	if (m_pNextPage != NULL)
		PrintPage(pDC, m_pNextPage, m_nRotate, m_nMode, rcSecondHalf, fScreenMM, fScreenMM, m_settings, true);
}

void CPrintDlg::OnChangePagesPerSheet()
{
	UpdateData();

	BOOL bTwoPages = m_cboPagesPerSheet.GetCurSel() == 1;
	if (bTwoPages == m_settings.bTwoPages)
		return;

	m_settings.bTwoPages = bTwoPages;
	m_settings.bLandscape = !m_settings.bLandscape;

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

	m_settings.nPaperCode = (WORD)m_cboPaper.GetItemData(nItem);
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
	{
		UpdateDevMode();
		SaveSettings();
		::ClosePrinter(m_hPrinter);
	}

	m_hPrinter = NULL;
	m_pPrinter = NULL;
	m_pDevMode = NULL;
	m_pPaper = NULL;

	m_strLocation.Empty();
	m_strComment.Empty();
	m_strType.Empty();

	int nPrinter = m_cboPrinter.GetCurSel();
	if (nPrinter == -1)
	{
		UpdateData(false);
		return;
	}

	CString strPrinter;
	m_cboPrinter.GetLBText(nPrinter, strPrinter);
	theApp.GetPrintSettings()->strPrinter = strPrinter;

	PRINTER_DEFAULTS defaults;
	defaults.pDatatype = NULL;
	defaults.pDevMode = GetCachedDevMode(strPrinter);
	defaults.DesiredAccess = PRINTER_ACCESS_USE;

	// Get full information about the printer into PRINTER_INFO_2 member variable
	if (!::OpenPrinter(strPrinter.GetBuffer(0), &m_hPrinter, &defaults))
	{
		UpdateData(false);
		return;
	}

	DWORD cbNeeded;
	::GetPrinter(m_hPrinter, 2, NULL, 0, &cbNeeded);

	m_printerData.resize(cbNeeded + 1);
	::GetPrinter(m_hPrinter, 2, &m_printerData[0], cbNeeded, &cbNeeded);
	m_pPrinter = reinterpret_cast<PRINTER_INFO_2*>(&m_printerData[0]);

	DWORD cbDevMode = ::DocumentProperties(NULL, m_hPrinter,
		m_pPrinter->pPrinterName, NULL, NULL, 0);
	m_devModeData.resize(cbDevMode);
	m_pDevMode = reinterpret_cast<DEVMODE*>(&m_devModeData[0]);

	if (defaults.pDevMode != NULL)
	{
		::DocumentProperties(NULL, m_hPrinter, m_pPrinter->pPrinterName,
			m_pDevMode, defaults.pDevMode, DM_IN_BUFFER | DM_OUT_BUFFER);
	}
	else
	{
		::DocumentProperties(NULL, m_hPrinter, m_pPrinter->pPrinterName,
			m_pDevMode, NULL, DM_OUT_BUFFER);
	}

	m_strComment = m_pPrinter->pComment;
	m_strLocation = m_pPrinter->pPortName;
	m_strType = m_pPrinter->pDriverName;

	LoadPaperTypes();

	m_nMaxCopies = ::DeviceCapabilities(m_pPrinter->pPrinterName, m_pPrinter->pPortName,
		DC_COPIES, NULL, m_pDevMode);
	if (m_nMaxCopies <= 1)
		m_nMaxCopies = 9999;

	if ((m_pDevMode->dmFields & DM_COPIES) == 0)
		m_settings.nCopies = 1;
	else
		m_settings.nCopies = min(m_settings.nCopies, m_nMaxCopies);

	m_bPrinterCanCollate = ::DeviceCapabilities(m_pPrinter->pPrinterName, m_pPrinter->pPortName,
		DC_COLLATE, NULL, m_pDevMode) > 0;

	UpdateData(false);
	UpdateDevMode();

	OnChangePaper();
}

void CPrintDlg::LoadPaperTypes()
{
	int nPapers = ::DeviceCapabilities(m_pPrinter->pPrinterName, m_pPrinter->pPortName,
		DC_PAPERNAMES, NULL, m_pDevMode);
	CString strPaperNames;
	::DeviceCapabilities(m_pPrinter->pPrinterName, m_pPrinter->pPortName,
		DC_PAPERNAMES, strPaperNames.GetBufferSetLength(nPapers*64), m_pDevMode);

	int nSizeNames = ::DeviceCapabilities(m_pPrinter->pPrinterName, m_pPrinter->pPortName,
		DC_PAPERS, NULL, m_pDevMode);
	vector<WORD> size_names(nSizeNames);
	::DeviceCapabilities(m_pPrinter->pPrinterName, m_pPrinter->pPortName,
		DC_PAPERS, (LPTSTR)&size_names[0], m_pDevMode);

	int nSizes = ::DeviceCapabilities(m_pPrinter->pPrinterName, m_pPrinter->pPortName,
		DC_PAPERSIZE, NULL, m_pDevMode);
	vector<POINT> sizes(nSizes);
	::DeviceCapabilities(m_pPrinter->pPrinterName, m_pPrinter->pPortName,
		DC_PAPERSIZE, (LPTSTR)&sizes[0], m_pDevMode);

	// Retain selected paper size
	int i;
	for (i = 0; i < nPapers && i < nSizeNames && i < nSizes; ++i)
	{
		if (size_names[i] == m_settings.nPaperCode)
		{
			m_pDevMode->dmPaperSize = m_settings.nPaperCode;
			break;
		}
	}

	m_cboPaper.ResetContent();
	m_paperSizes.clear();
	TCHAR szPaperName[65];
	for (i = 0; i < nPapers && i < nSizeNames && i < nSizes; ++i)
	{
		ZeroMemory(szPaperName, sizeof(szPaperName));
		_tcsncpy(szPaperName, (LPCTSTR)strPaperNames + i*64, 64);

		if (size_names[i] != DMPAPER_USER)
		{
			int nItem = m_cboPaper.AddString(szPaperName);
			m_cboPaper.SetItemData(nItem, size_names[i]);

			if (size_names[i] == m_pDevMode->dmPaperSize)
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
		++m_settings.nCopies;
	else
		--m_settings.nCopies;

	if (m_settings.nCopies < 1)
		m_settings.nCopies = 1;
	else if (m_settings.nCopies > m_nMaxCopies)
		m_settings.nCopies = m_nMaxCopies;

	UpdateData(false);
	UpdateDevMode();

	*pResult = 0;
}

void CPrintDlg::OnProperties()
{
	if (m_hPrinter == NULL)
		return;

	UpdateData();

	::DocumentProperties(m_hWnd, m_hPrinter, m_pPrinter->pPrinterName,
			m_pDevMode, m_pDevMode,
			DM_IN_PROMPT | DM_IN_BUFFER | DM_OUT_BUFFER);

	m_settings.bLandscape = (m_pDevMode->dmOrientation == DMORIENT_LANDSCAPE);
	m_settings.nCopies = m_pDevMode->dmCopies;

	if (m_bPrinterCanCollate)
		m_settings.bCollate = (m_pDevMode->dmCollate == DMCOLLATE_TRUE);

	UpdateData(false);

	m_pPaper = NULL;
	m_settings.nPaperCode = m_pDevMode->dmPaperSize;
	LoadPaperTypes();
	OnChangePaper();
}

void CPrintDlg::OnKickIdle()
{
	bool bOk = (m_pPrinter != NULL);

	GetDlgItem(IDC_PORTRAIT)->EnableWindow(bOk && (m_pDevMode->dmFields & DM_ORIENTATION));
	GetDlgItem(IDC_LANDSCAPE)->EnableWindow(bOk && (m_pDevMode->dmFields & DM_ORIENTATION));

	GetDlgItem(IDC_EDIT_COPIES)->EnableWindow(bOk && (m_pDevMode->dmFields & DM_COPIES));
	GetDlgItem(IDC_SPIN_COPIES)->EnableWindow(bOk && (m_pDevMode->dmFields & DM_COPIES));

	GetDlgItem(IDC_COLLATE)->EnableWindow(bOk && m_settings.nCopies >= 2);

	GetDlgItem(IDC_RANGE_ALL)->EnableWindow(bOk);
	GetDlgItem(IDC_RANGE_CURRENT)->EnableWindow(bOk);
	GetDlgItem(IDC_RANGE_PAGES)->EnableWindow(bOk);

	GetDlgItem(IDC_PAGE_RANGES)->EnableWindow(bOk && m_nRangeType == 2);

	GetDlgItem(IDC_SCALE)->EnableWindow(bOk && !m_settings.bScaleToFit);
	GetDlgItem(IDC_SHRINK_OVERSIZED)->EnableWindow(bOk && !m_settings.bScaleToFit);
	GetDlgItem(IDC_SCALE_TO_FIT)->EnableWindow(bOk);
	GetDlgItem(IDC_IGNORE_MARGINS)->EnableWindow(bOk);

	GetDlgItem(IDC_POSITION_TOP)->EnableWindow(bOk && !m_settings.bCenterImage);
	GetDlgItem(IDC_POSITION_LEFT)->EnableWindow(bOk && !m_settings.bCenterImage);
	GetDlgItem(IDC_CENTER_IMAGE)->EnableWindow(bOk);
	GetDlgItem(IDC_CLIP_CONTENT)->EnableWindow(bOk);

	GetDlgItem(IDC_MARGIN_LEFT)->EnableWindow(bOk);
	GetDlgItem(IDC_MARGIN_TOP)->EnableWindow(bOk);
	GetDlgItem(IDC_MARGIN_RIGHT)->EnableWindow(bOk);
	GetDlgItem(IDC_MARGIN_BOTTOM)->EnableWindow(bOk);

	m_cboPaper.EnableWindow(bOk);
	m_cboPagesInRange.EnableWindow(bOk);
	m_cboPagesPerSheet.EnableWindow(bOk);

	GetDlgItem(IDOK)->EnableWindow(bOk);
	GetDlgItem(IDC_REVERSE)->EnableWindow(bOk);
	GetDlgItem(IDC_PROPERTIES)->EnableWindow(bOk);
}

void CPrintDlg::OnUpdateDialogData()
{
	UpdateData();

	if (m_settings.nCopies > m_nMaxCopies)
		m_settings.nCopies = m_nMaxCopies;
	else if (m_settings.nCopies < 1)
		m_settings.nCopies = 1;

	UpdateData(false);
	UpdateDevMode();

	InvalidateRect(m_rcPreview, 0);
	UpdateWindow();
}

void CPrintDlg::UpdateDevMode()
{
	if (m_pPrinter == NULL)
		return;

	m_pDevMode->dmFields |= DM_PAPERSIZE | DM_ORIENTATION | DM_COPIES | DM_COLLATE;

	m_pDevMode->dmPaperSize = m_settings.nPaperCode;
	m_pDevMode->dmOrientation = (m_settings.bLandscape ? DMORIENT_LANDSCAPE : DMORIENT_PORTRAIT);

	m_pDevMode->dmCollate = (m_settings.bCollate && m_bPrinterCanCollate ? DMCOLLATE_TRUE : DMCOLLATE_FALSE);
	m_pDevMode->dmCopies = (WORD)m_settings.nCopies;

	UpdateDevModeCache(m_pPrinter->pPrinterName, m_pDevMode, m_devModeData.size());
}

void CPrintDlg::SaveSettings()
{
	if (m_pPrinter == NULL)
		return;

	*theApp.GetPrintSettings() = m_settings;
}

bool CPrintDlg::ParseRange()
{
	CString strPages = m_strPages;
	strPages.TrimLeft();
	strPages.TrimRight();

	int i = 0;
	while (i < strPages.GetLength())
	{
		while (i < strPages.GetLength() && strPages[i] <= ' ')
			++i;

		if (i >= strPages.GetLength() || strPages[i] < '0' || strPages[i] > '9')
			return false;

		int num = 0;
		while (i < strPages.GetLength() && strPages[i] >= '0' && strPages[i] <= '9')
			num = 10*num + strPages[i++] - '0';
		int num2 = num;

		while (i < strPages.GetLength() && strPages[i] <= ' ')
			++i;
		if (i < strPages.GetLength())
		{
			if (strPages[i] == '-')
			{
				++i;
				while (i < strPages.GetLength() && strPages[i] <= ' ')
					++i;

				if (i >= strPages.GetLength() || strPages[i] < '0' || strPages[i] > '9')
					return false;

				num2 = 0;
				while (i < strPages.GetLength() && strPages[i] >= '0' && strPages[i] <= '9')
					num2 = 10*num2 + strPages[i++] - '0';

				while (i < strPages.GetLength() && strPages[i] <= ' ')
					++i;

				if (i < strPages.GetLength())
				{
					if (strPages[i] == ',' || strPages[i] == ';')
					{
						++i;
						while (i < strPages.GetLength() && strPages[i] <= ' ')
							++i;
					}
					else
						return false;
				}
			}
			else if (strPages[i] == ',' || strPages[i] == ';')
			{
				++i;
				while (i < strPages.GetLength() && strPages[i] <= ' ')
					++i;
			}
			else
				return false;
		}

		if (num > num2)
			return false;

		for (int j = num; j <= num2; ++j)
			m_pages.push_back(j);
	}

	return true;
}

LPDEVMODE CPrintDlg::GetCachedDevMode(const CString& strPrinter)
{
	map<CString, vector<byte> >::iterator it = s_devModes.find(strPrinter);
	if (it != s_devModes.end())
		return (LPDEVMODE) &(it->second[0]);

	return NULL;
}

void CPrintDlg::UpdateDevModeCache(const CString& strPrinter, LPDEVMODE pDevMode, UINT nSize)
{
	map<CString, vector<byte> >::iterator it = s_devModes.find(strPrinter);
	if (it == s_devModes.end())
		it = s_devModes.insert(make_pair(strPrinter, vector<byte>())).first;

	it->second.resize(nSize);
	memcpy(&(it->second[0]), pDevMode, it->second.size());
}
