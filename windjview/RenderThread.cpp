//	WinDjView 0.1
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

#include "RenderThread.h"
#include "Drawing.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLock class

void CLock::Lock()
{
	while (::InterlockedCompareExchange(&m_nLock, 1, 0) == 1)
		::Sleep(1);
}

void CLock::Unlock()
{
	ASSERT(m_nLock == 1);
	::InterlockedExchange(&m_nLock, 0);
}


// CSignal class

bool CSignal::IsSet() const
{
	return ::InterlockedCompareExchange(&m_nSignal, 0, 0) == 1;
}

void CSignal::Set()
{
	::InterlockedExchange(&m_nSignal, 1);
}

void CSignal::Reset()
{
	::InterlockedExchange(&m_nSignal, 0);
}

void CSignal::Wait()
{
	while (::InterlockedCompareExchange(&m_nSignal, 1, 1) == 0)
		::Sleep(1);
}

// CRenderThread class

CRenderThread::CRenderThread(CWnd* pOwner)
	: m_pOwner(pOwner)
{
	DWORD dwThreadId;
	HANDLE hThread = ::CreateThread(NULL, 0, RenderThreadProc, this, 0, &dwThreadId);
	::SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
}

CRenderThread::~CRenderThread()
{
	m_stop.Set();
	while (m_stop.IsSet())
		::Sleep(10);
}

DWORD WINAPI CRenderThread::RenderThreadProc(LPVOID pvData)
{
	CRenderThread* pData = reinterpret_cast<CRenderThread*>(pvData);

	while (!pData->m_stop.IsSet())
	{
		pData->m_lock.Lock();
		if (pData->m_jobs.empty())
		{
			pData->m_lock.Unlock();
			::Sleep(10);
			continue;
		}

		Job job = pData->m_jobs.front();
		pData->m_jobs.pop_front();

		pData->m_lock.Unlock();

		pData->Render(job);
	}

	pData->m_stop.Reset();
	return 0;
}

DWORD CRenderThread::AddJob(GP<DjVuImage> pImage, CRect rcAll, CRect rcClip, bool bClearQueue)
{
	static DWORD nCode = 0;

	m_lock.Lock();

	if (bClearQueue)
		m_jobs.clear();

	Job job;
	job.pImage = pImage;
	job.rcAll = rcAll;
	job.rcClip = rcClip;
	job.nCode = nCode;

	m_jobs.push_back(job);

	m_lock.Unlock();

	return nCode++;
}

void CRenderThread::Render(Job& job)
{
	GP<GBitmap> pGBitmap;
	GP<GPixmap> pGPixmap;

	GRect rcAll(job.rcAll.left, job.rcAll.top, job.rcAll.Width(), job.rcAll.Height());
	GRect rcClip(job.rcClip.left, job.rcClip.top, job.rcClip.Width(), job.rcClip.Height());

	if (job.pImage->is_legal_photo() || job.pImage->is_legal_compound())
	{
		pGPixmap = job.pImage->get_pixmap(rcClip, rcAll);
	}
	else if (job.pImage->is_legal_bilevel())
	{
		pGBitmap = job.pImage->get_bitmap(rcClip, rcAll, 4);
	}
	else
	{
		// Try to get both
		pGPixmap = job.pImage->get_pixmap(rcClip, rcAll);
		if (pGPixmap == NULL)
			pGBitmap = job.pImage->get_bitmap(rcClip, rcAll, 4);
	}

	CDIB* pBitmap = NULL;

	if (pGPixmap != NULL)
		pBitmap = RenderPixmap(*pGPixmap);
	else if (pGBitmap != NULL)
		pBitmap = RenderBitmap(*pGBitmap);

	if (pBitmap == NULL || pBitmap->m_hObject == NULL)
	{
		delete pBitmap;
		return;
	}

	m_pOwner->PostMessage(WM_RENDER_FINISHED, job.nCode, reinterpret_cast<LPARAM>(pBitmap));
}
