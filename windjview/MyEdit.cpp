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
#include "MyEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMyEdit

IMPLEMENT_DYNAMIC(CMyEdit, CEdit)
CMyEdit::CMyEdit()
	: m_nType(EditNormal), m_nUnits(theApp.GetAppSettings()->nUnits), m_fValue(0.0)
{
}

CMyEdit::~CMyEdit()
{
}


BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
	ON_WM_CHAR()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()



// CMyEdit message handlers

void AFXAPI DDX_Units(CDataExchange* pDX, int nIDC, double& fValue, int nUnits)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	ASSERT(hWndCtrl != NULL);                
    
	CMyEdit* pEdit = (CMyEdit*) CWnd::FromHandle(hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		ASSERT(pEdit->GetType() == CMyEdit::EditUnits);

		pEdit->UpdateValue();
		int nDisplayUnits = pEdit->GetUnits();
		fValue = pEdit->GetValue() * CAppSettings::unitsPerInch[nUnits] / CAppSettings::unitsPerInch[nDisplayUnits];
    }
	else
	{
		pEdit->SetUnits();

		int nDisplayUnits = pEdit->GetUnits();
		double fConvValue = fValue * CAppSettings::unitsPerInch[nDisplayUnits] / CAppSettings::unitsPerInch[nUnits];
		pEdit->SetValue(fConvValue, nDisplayUnits);
	}
}

void CMyEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	char nDecimalPoint = localeconv()->decimal_point[0];

	if (m_nType == EditReal)
	{
		if ((nChar < '0' || nChar > '9') && nChar != nDecimalPoint && nChar != VK_BACK &&
				(!m_bPercent || nChar != '%'))
			return;
	}
	else if (m_nType == EditInteger)
	{
		if ((nChar < '0' || nChar > '9') && nChar != VK_BACK &&
				(!m_bPercent || nChar != '%'))
			return;
	}

	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

void CMyEdit::OnKillFocus(CWnd* pNewWnd)
{
	if (m_nType == EditUnits)
		UpdateValue();

	CEdit::OnKillFocus(pNewWnd);
}

void CMyEdit::OnSetFocus(CWnd* pOldWnd)
{
	SetSel(0, -1);

	CEdit::OnSetFocus(pOldWnd);
}

void CMyEdit::UpdateValue()
{
	CString strText;
	GetWindowText(strText);

	CString strAllUnitsLoc = LoadString(IDS_UNITS_SHORT);
	CString strAllUnitsEng;
	::LoadString(AfxGetInstanceHandle(), IDS_UNITS_SHORT, strAllUnitsEng.GetBuffer(256), 256);
	strAllUnitsEng.ReleaseBuffer();

	TCHAR* pEnd = NULL;
	double fValue = _tcstod(strText, &pEnd);
	if (pEnd == (LPCTSTR) strText)
	{
		SetValue(0, theApp.GetAppSettings()->nUnits);
		return;
	}

	CString strUnits = pEnd;
	strUnits.TrimLeft();
	strUnits.TrimRight();

	for (int nUnits = CAppSettings::Centimeters; nUnits <= CAppSettings::Inches; ++nUnits)
	{
		CString strUnitsLoc, strUnitsEng;
		AfxExtractSubString(strUnitsLoc, strAllUnitsLoc, nUnits, ',');
		AfxExtractSubString(strUnitsEng, strAllUnitsEng, nUnits, ',');

		if (_tcsicmp(strUnitsLoc, strUnits) == 0 || _tcsicmp(strUnitsEng, strUnits) == 0)
		{
			SetValue(fValue, nUnits);
			return;
		}
	}

	SetValue(fValue, theApp.GetAppSettings()->nUnits);
}

void CMyEdit::SetValue(double fValue, int nUnits)
{
	m_fValue = static_cast<int>(fValue * 100.0 + 0.5) / 100.0;
	m_nUnits = nUnits;
	if (m_nUnits < CAppSettings::Centimeters || m_nUnits > CAppSettings::Inches)
		m_nUnits = theApp.GetAppSettings()->nUnits;

	CString strUnits;
	AfxExtractSubString(strUnits, LoadString(IDS_UNITS_SHORT), nUnits, ',');
	SetWindowText(FormatString(_T("%s %s"), FormatDouble(m_fValue), strUnits));
}
