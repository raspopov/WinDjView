//	WinDjView
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

#include "StdAfx.h"
#include "WinDjView.h"

#include "DecodeThread.h"
#include "DjVuDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDecodeThread class

CDecodeThread::CDecodeThread(CDjVuDoc* pDoc)
	: m_pDoc(pDoc)
{
	for (int i = 0; i < pDoc->m_pDjVuDoc->get_pages_num(); ++i)
		m_jobs.push_back(i);

	DWORD dwThreadId;
	HANDLE hThread = ::CreateThread(NULL, 0, DecodeThreadProc, this, 0, &dwThreadId);
	::SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
}

CDecodeThread::~CDecodeThread()
{
	m_stop.SetEvent();
	::WaitForSingleObject(m_finished.m_hObject, INFINITE);
}

DWORD WINAPI CDecodeThread::DecodeThreadProc(LPVOID pvData)
{
	CDecodeThread* pData = reinterpret_cast<CDecodeThread*>(pvData);

	while (::WaitForSingleObject(pData->m_stop, 0) != WAIT_OBJECT_0)
	{
		pData->m_lock.Lock();
		if (pData->m_jobs.empty())
		{
			pData->m_lock.Unlock();
			pData->m_finished.SetEvent();
			return 0;
		}

		int nPage = pData->m_jobs.front();
		pData->m_jobs.pop_front();

		pData->m_lock.Unlock();

		GP<DjVuImage> pImage = pData->m_pDoc->m_pDjVuDoc->get_page(nPage);
		pData->m_pDoc->PageDecoded(nPage, pImage);
	}

	pData->m_finished.SetEvent();
	return 0;
}

void CDecodeThread::MoveToFront(int nPage)
{
	m_lock.Lock();
	list<int>::iterator it = find(m_jobs.begin(), m_jobs.end(), nPage);
	if (it != m_jobs.end())
	{
		m_jobs.erase(it);
		m_jobs.push_front(nPage);
	}
	m_lock.Unlock();
}
