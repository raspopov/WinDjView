//	WinDjView
//	Copyright (C) 2004-2006 Andrew Zhezherun
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

#include "StdAfx.h"
#include "WinDjView.h"

#include "ThumbnailsThread.h"
#include "Drawing.h"
#include "DjVuDoc.h"
#include "ThumbnailsView.h"
#include "RenderThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CThumbnailsThread class

CThumbnailsThread::CThumbnailsThread(CDjVuDoc* pDoc, CThumbnailsView* pOwner, bool bIdle)
	: m_pOwner(pOwner), m_pDoc(pDoc), m_bPaused(false), m_bRejectCurrentJob(false)
{
	m_currentJob.nPage = -1;

	DWORD dwThreadId;
	m_hThread = ::CreateThread(NULL, 0, RenderThreadProc, this, 0, &dwThreadId);
	::SetThreadPriority(m_hThread,
			bIdle ? THREAD_PRIORITY_IDLE : THREAD_PRIORITY_BELOW_NORMAL);
}

CThumbnailsThread::~CThumbnailsThread()
{
	m_stop.SetEvent();
	::WaitForSingleObject(m_finished.m_hObject, INFINITE);
	::CloseHandle(m_hThread);
}

void CThumbnailsThread::Stop()
{
	m_stop.SetEvent();
}

DWORD WINAPI CThumbnailsThread::RenderThreadProc(LPVOID pvData)
{
	CThumbnailsThread* pData = reinterpret_cast<CThumbnailsThread*>(pvData);

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
		pData->m_currentJob = job;
		pData->m_jobs.pop_front();
		bool bHasMoreJobs = !pData->m_jobs.empty();
		pData->m_bRejectCurrentJob = false;
		pData->m_lock.Unlock();

		pData->Render(job);

		pData->m_lock.Lock();
		pData->m_currentJob.nPage = -1;
		if (bHasMoreJobs && !pData->m_bPaused)
			pData->m_jobReady.SetEvent();
		pData->m_lock.Unlock();

		if (::WaitForSingleObject(pData->m_stop.m_hObject, 0) == WAIT_OBJECT_0)
			break;
	}

	pData->m_finished.SetEvent();
	return 0;
}

void CThumbnailsThread::PauseJobs()
{
	m_bPaused = true;
}

void CThumbnailsThread::ResumeJobs()
{
	m_bPaused = false;

	m_lock.Lock();
	bool bHasJobs = !m_jobs.empty();
	m_lock.Unlock();

	if (bHasJobs)
		m_jobReady.SetEvent();
}

void CThumbnailsThread::ClearQueue()
{
	m_lock.Lock();
	m_jobs.clear();
	m_lock.Unlock();
}

void CThumbnailsThread::Render(Job& job)
{
	CDIB* pBitmap = NULL;

	GP<DjVuImage> pImage = m_pDoc->GetPage(job.nPage, false);
	if (pImage != NULL)
	{
		RotateImage(pImage, job.nRotate);
		CSize szPage(pImage->get_width(), pImage->get_height());
		if (szPage.cx > 0 && szPage.cy > 0)
		{
			CSize szDisplay = m_szThumbnail;
			szDisplay.cx = szDisplay.cy * szPage.cx / szPage.cy;

			if (szDisplay.cx > m_szThumbnail.cx)
			{
				szDisplay.cx = m_szThumbnail.cx;
				szDisplay.cy = szDisplay.cx * szPage.cy / szPage.cx;
			}

			pBitmap = CRenderThread::Render(pImage, szDisplay);
		}
	}

	if (pBitmap == NULL || pBitmap->m_hObject == NULL)
	{
		delete pBitmap;
		pBitmap = NULL;
	}

	m_lock.Lock();
	if (!m_bRejectCurrentJob && ::IsWindow(m_pOwner->m_hWnd))
	{
		m_pOwner->PostMessage(WM_RENDER_THUMB_FINISHED,
			job.nPage, reinterpret_cast<LPARAM>(pBitmap));
	}
	m_lock.Unlock();
}

void CThumbnailsThread::AddJob(int nPage, int nRotate)
{
	Job job;
	job.nPage = nPage;
	job.nRotate = nRotate;

	m_lock.Lock();
	if (m_currentJob.nPage == job.nPage && m_currentJob.nRotate == nRotate)
	{
		m_lock.Unlock();
		return;
	}

	m_jobs.push_front(job);

	m_lock.Unlock();

	if (!m_bPaused)
		m_jobReady.SetEvent();
}

void CThumbnailsThread::RejectCurrentJob()
{
	m_lock.Lock();
	if (m_currentJob.nPage != -1)
		m_bRejectCurrentJob = true;
	m_lock.Unlock();
}
