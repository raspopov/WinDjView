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

#pragma once


// CMyDocTemplate

class CMyDocTemplate : public CMultiDocTemplate
{
	DECLARE_DYNAMIC(CMyDocTemplate)

public:
	CMyDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass, 
			CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);
	virtual ~CMyDocTemplate();

	void UpdateTemplate();

// Overrides
public:
    virtual Confidence MatchDocType(LPCTSTR lpszPathName, CDocument*& rpDocMatch);
	virtual void InitialUpdateFrame(CFrameWnd* pFrame, CDocument* pDoc, BOOL bMakeVisible = true);
};
