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


void AFXAPI DDX_Units(CDataExchange* pDX, int nIDC, double& fValue, int nUnits);

// CMyEdit

class CMyEdit : public CEdit
{
	DECLARE_DYNAMIC(CMyEdit)

public:
	CMyEdit();
	virtual ~CMyEdit();

// Operations
public:
	void SetNormal() { m_nType = EditNormal; }
	void SetInteger() { m_nType = EditInteger; }
	void SetReal() { m_nType = EditReal; }
	void SetPercent(bool bPercent = true) { m_bPercent = bPercent; }
	void SetUnits() { m_nType = EditUnits; }

	enum EditType
	{
		EditNormal,
		EditInteger,
		EditReal,
		EditUnits
	};

	int GetType() const { return m_nType; }

	void UpdateValue();
	int GetUnits() const { return m_nUnits; }
	double GetValue() const { return m_fValue; }
	void SetValue(double fValue, int nUnits);

// Implementation
protected:
	EditType m_nType;
	bool m_bPercent;

	double m_fValue;
	int m_nUnits;

	// Message map functions
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	DECLARE_MESSAGE_MAP()
};

