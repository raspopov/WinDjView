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
	m_stop.SetEvent();
	::WaitForSingleObject(m_finished.m_hObject, INFINITE);
}

DWORD WINAPI CRenderThread::RenderThreadProc(LPVOID pvData)
{
	CRenderThread* pData = reinterpret_cast<CRenderThread*>(pvData);

	HANDLE hEvents[] = { pData->m_jobReady.m_hObject, pData->m_stop.m_hObject };
	while (::WaitForMultipleObjects(2, hEvents, false, INFINITE) == WAIT_OBJECT_0)
	{
		pData->m_lock.Lock();

		if (pData->m_jobs.empty())
		{
			pData->m_lock.Unlock();
			continue;
		}

		Job job = pData->m_jobs.front();
		pData->m_jobs.pop_front();

		if (!pData->m_jobs.empty())
			pData->m_jobReady.SetEvent();

		pData->m_lock.Unlock();

		pData->Render(job);
	}

	pData->m_finished.SetEvent();
	return 0;
}

DWORD CRenderThread::AddJob(GP<DjVuImage> pImage, CRect rcAll, CRect rcClip, bool bClearQueue)
{
	static DWORD nCode = 0;

	Job job;
	job.pImage = pImage;
	job.rcAll = rcAll;
	job.rcClip = rcClip;
	job.nCode = nCode;

	m_lock.Lock();

	if (bClearQueue)
		m_jobs.clear();
	m_jobs.push_back(job);

	m_lock.Unlock();

	m_jobReady.SetEvent();
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
