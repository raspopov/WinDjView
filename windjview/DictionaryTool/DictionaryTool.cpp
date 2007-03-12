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

#include "stdafx.h"
#include "DictionaryTool.h"
#include "DictionaryDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDictionaryApp

BEGIN_MESSAGE_MAP(CDictionaryApp, CWinApp)
END_MESSAGE_MAP()


// CDictionaryApp construction

CDictionaryApp::CDictionaryApp()
{
}


// The one and only CDictionaryApp object
CDictionaryApp theApp;


// CDictionaryApp initialization

BOOL CDictionaryApp::InitInstance()
{
	_tsetlocale(LC_ALL, _T(""));

	AfxOleInit();

	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization

	// Change the registry key under which our settings are stored
	SetRegistryKey(_T("Andrew Zhezherun"));

	InitLocalizations();
	InitLanguages();

	CDictionaryDlg dlg;
	m_pMainWnd = &dlg;

	dlg.DoModal();

	AfxOleTerm();

	// Since the dialog has been closed, return FALSE so that we exit the
	// application, rather than start the application's message pump.
	return false;
}

void CDictionaryApp::InitLocalizations()
{
	m_loc.push_back(Localization(_T("English"), MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT)));
	m_loc.push_back(Localization(_T("Russian"), MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT)));
	m_loc.push_back(Localization(_T("Ukrainian"), MAKELANGID(LANG_UKRAINIAN, SUBLANG_DEFAULT)));
	m_loc.push_back(Localization(_T("French"), MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH)));
	m_loc.push_back(Localization(_T("Portuguese"), MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE)));
	m_loc.push_back(Localization(_T("Chinese Simplified"), MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)));
	m_loc.push_back(Localization(_T("Tatarish"), MAKELANGID(LANG_TURKISH, 3)));
	m_loc.push_back(Localization(_T("Hungarian"), MAKELANGID(LANG_HUNGARIAN, SUBLANG_DEFAULT)));
	m_loc.push_back(Localization(_T("Polish"), MAKELANGID(LANG_POLISH, SUBLANG_DEFAULT)));
}

void CDictionaryApp::InitLanguages()
{
	HRSRC hRes = ::FindResource(NULL, MAKEINTRESOURCE(IDR_ISO_CODES), _T("RT_RCDATA"));
	ASSERT(hRes != NULL);
	DWORD dwSize = ::SizeofResource(NULL, hRes);

	HGLOBAL hGlobal = ::LoadResource(NULL, hRes);
	ASSERT(hGlobal != NULL);
	LPVOID pvData = ::LockResource(hGlobal);

	vector<char> data((char*) pvData, (char*) pvData + dwSize);
	data.push_back('\r');
	data.push_back('\n');
	data.push_back('\0');

	CString strLanguages(&data[0]);
	int nStart = 0;

	while (nStart < strLanguages.GetLength())
	{
		int nPos = strLanguages.Find('\r', nStart);
		int nPos2 = strLanguages.Find('\n', nStart);
		if (nPos == -1 && nPos2 != -1 || nPos != -1 && nPos2 != -1 && nPos2 < nPos)
			nPos = nPos2;

		CString strData = strLanguages.Mid(nStart, nPos - nStart);

		int nComma = strData.Find(',');
		CString strCode, strName;
		if (nComma != -1)
		{
			strCode = strData.Left(nComma);
			strName = strData.Mid(nComma + 1);
		}

		if (strCode.GetLength() > 0 && strCode[0] == '\"')
			strCode.Delete(0);
		if (strCode.GetLength() > 0 && strCode[strCode.GetLength() - 1] == '\"')
			strCode.Delete(strCode.GetLength() - 1);

		if (strName.GetLength() > 0 && strName[0] == '\"')
			strName.Delete(0);
		if (strName.GetLength() > 0 && strName[strName.GetLength() - 1] == '\"')
			strName.Delete(strName.GetLength() - 1);

		if (!strCode.IsEmpty() && !strName.IsEmpty())
			m_lang.push_back(Language(strCode, strName));

		nStart = nPos;
		while (nStart < strLanguages.GetLength()
				&& (strLanguages[nStart] == '\n' || strLanguages[nStart] == '\r'))
			++nStart;
	}
}
