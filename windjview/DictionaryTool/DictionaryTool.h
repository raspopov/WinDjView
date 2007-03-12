//	DjVu Dictionary Tool
//	Copyright (C) 2006-2007 Andrew Zhezherun
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

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


struct Localization
{
	Localization(const CString& strName_, UINT nCode_)
		: strName(strName_), nCode(nCode_) {}

	CString strName;
	int nCode;
};

struct Language
{
	Language(const CString& strCode_, const CString& strName_)
		: strCode(strCode_), strName(strName_) {}

	CString strCode;
	CString strName;
};

// CDictionaryApp

class CDictionaryApp : public CWinApp
{
public:
	CDictionaryApp();

	size_t GetLocalizationCount() const { return m_loc.size(); }
	const Localization& GetLocalization(size_t nIndex) { return m_loc[nIndex]; }
	const Localization& GetEnglishLoc() const { return m_loc[0]; }

	size_t GetLanguageCount() const { return m_lang.size(); }
	const Language& GetLanguage(size_t nIndex) { return m_lang[nIndex]; }

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
protected:
	vector<Localization> m_loc;
	vector<Language> m_lang;
	void InitLocalizations();
	void InitLanguages();

	DECLARE_MESSAGE_MAP()
};

extern CDictionaryApp theApp;
