//	Google Desktop Search DjVu Indexer Plugin
//	Copyright (C) 2005 Andrew Zhezherun
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
//	http://www.gnu.org/copyleft/gpl.html

// $Id$

#include "stdafx.h"
#include "resource.h"
#include "DjVuIndexerObj.h"

// The module attribute causes DllMain, DllRegisterServer and DllUnregisterServer to be automatically implemented for you
[
	module(dll,
	uuid = "{E18803D6-7195-4BEA-BAEC-A92C77E5E306}", 
	name = "GDSDjVuIndexer", 
	helpstring = "GDSDjVuIndexer 1.0 Type Library",
	resource_name = "IDR_DJVUINDEXER")
]
class CDjVuIndexerModule
{
	typedef CAtlDllModuleT<CDjVuIndexerModule> Base;

public:
	HRESULT DllRegisterServer(BOOL bRegTypeLib = TRUE) throw();
	HRESULT DllUnregisterServer(BOOL bUnRegTypeLib = TRUE) throw();
};


HRESULT RegisterComponent()
{
	REFCLSID clsid = __uuidof(CDjVuIndexerObj);
	IGoogleDesktopSearchComponentRegisterPtr spRegister;

	try
	{
		HRESULT hr = spRegister.CreateInstance(__uuidof(GoogleDesktopSearchRegister));
		if (FAILED(hr))
			return hr;

		CStringW strModuleName;
		::GetModuleFileNameW(_AtlBaseModule.GetModuleInstance(), CStrBufW(strModuleName, 1024), 1023);
		CStringA strIconIndex;
		strIconIndex.Format("%d", IDI_DJVUICON);
		CStringW strIcon = strModuleName + L"," + CStringW(strIconIndex);

		// Component description is 6 strings
		CComSafeArray<VARIANT> arr_descr(6);
		arr_descr.SetAt(0, CComVariant(L"Title"));
		arr_descr.SetAt(1, CComVariant(L"DjVu Indexer 1.0 by Andrew Zhezherun"));
		arr_descr.SetAt(2, CComVariant(L"Description"));
		arr_descr.SetAt(3, CComVariant(L"Indexes DjVu files"));
		arr_descr.SetAt(4, CComVariant(L"Icon"));
		arr_descr.SetAt(5, CComVariant(strIcon));

		// Wrap description array in variant
		IGoogleDesktopSearchComponentRegistrationPtr spRegistration =
				spRegister->RegisterComponent((BSTR)CComBSTR(clsid), CComVariant(arr_descr.m_psa));

		// register extensions
		spRegistration->RegisterExtension(L"djvu");
		spRegistration->RegisterExtension(L"djv");

		return S_OK;
	}
	catch (_com_error& e)
	{
		// revoke our registration in case of any error
		if (spRegister != NULL)
			spRegister->UnregisterComponent((BSTR)CComBSTR(clsid));

		return e.Error();
	}
}

HRESULT UnregisterComponent()
{
	REFCLSID clsid = __uuidof(CDjVuIndexerObj);

	try
	{
		IGoogleDesktopSearchComponentRegisterPtr spRegister(__uuidof(GoogleDesktopSearchRegister));
		return spRegister->UnregisterComponent((BSTR)CComBSTR(clsid));
	}
	catch (_com_error& e)
	{
		return e.Error();
	}
}


// CDjVuIndexerModule

HRESULT CDjVuIndexerModule::DllRegisterServer(BOOL bRegTypeLib) throw()
{
	HRESULT hr = Base::DllRegisterServer(bRegTypeLib);

	if (SUCCEEDED(hr))
		hr = RegisterComponent();

	return hr;
}

HRESULT CDjVuIndexerModule::DllUnregisterServer(BOOL bUnRegTypeLib) throw()
{
	UnregisterComponent();

	return Base::DllUnregisterServer(bUnRegTypeLib);
}
