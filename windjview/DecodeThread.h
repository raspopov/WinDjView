//	WinDjView
//	Copyright (C) 2004-2005 Andrew Zhezherun
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

#pragma once

#include "DjVuDoc.h"

class CDecodeThread
{
public:
	CDecodeThread(CDjVuDoc* pDoc);
	~CDecodeThread();

	void AddJob(int nPage, bool bReadInfo = false);

	void StartDecodePage(int nPage);
	static PageInfo ReadPageInfo(GP<DjVuDocument> pDjVuDoc, int nPage);

private:
	HANDLE m_hThread;
	CEvent m_stop;
	CEvent m_finished;
	CEvent m_jobReady;
	CDjVuDoc* m_pDoc;

	CCriticalSection m_lock;

	struct Job
	{
		Job(int page, bool readInfo) : nPage(page), bReadInfo(readInfo) {}

		int nPage;
		bool bReadInfo;

		bool operator==(int page) const { return nPage == page; }
	};

	list<Job> m_jobs;
	Job currentJob;

	static DWORD WINAPI DecodeThreadProc(LPVOID pvData);
};
