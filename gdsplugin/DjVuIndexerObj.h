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

#pragma once

#include "resource.h"       // main symbols


// IDjVuIndexerObj
[
	object,
	uuid("859108E9-F0DD-4756-8C99-A3BE44D846EF"),
	dual,
	helpstring("IDjVuIndexerObj Interface"),
	pointer_default(unique)
]
__interface IDjVuIndexerObj : IDispatch
{
	[id(1), helpstring("method HandleFile")] HRESULT HandleFile(BSTR fullPath, IDispatch* pFactory);
};


// CDjVuIndexerObj
[
	coclass,
	threading("both"),
	support_error_info("IDjVuIndexerObj"),
	vi_progid("DjVuIndexer.DjVuIndexerObj"),
	progid("DjVuIndexer.DjVuIndexerObj.1"),
	version(1.0),
	uuid("10688B37-0DA9-450E-A704-03066165838B"),
	helpstring("DjVuIndexerObj Class")
]
class ATL_NO_VTABLE CDjVuIndexerObj : 	public IDjVuIndexerObj
{
public:
	CDjVuIndexerObj() {}
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct() { return S_OK; }
	void FinalRelease()  {}

public:
	STDMETHOD(HandleFile)(BSTR fullPath, IDispatch* pFactory);
};
