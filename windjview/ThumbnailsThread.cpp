//	WinDjView
//	Copyright (C) 2004-2015 Andrew Zhezherun
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
//	You should have received a copy of the GNU General Public License along
//	with this program; if not, write to the Free Software Foundation, Inc.,
//	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//	http://www.gnu.org/copyleft/gpl.html

#include "stdafx.h"
#include "WinDjView.h"

#include "ThumbnailsThread.h"
#include "Drawing.h"
#include "RenderThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CThumbnailsThread class

CThumbnailsThread::CThumbnailsThread(DjVuSource* pSource, Observer* pOwner, bool bIdle)
	: m_pSource(pSource), m_pOwner(pOwner), m_nPaused(0), m_bRejectCurrentJob(false)
{
	m_pSource->AddRef();

	m_currentJob.nPage = -1;

	UINT dwThreadId;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, RenderThreadProc, this, 0, &dwThreadId);
	::SetThreadPriority(m_hThread,
			bIdle ? THREAD_PRIORITY_IDLE : THREAD_PRIORITY_BELOW_NORMAL);
	theApp.ThreadStarted();
}

CThumbnailsThread::~CThumbnailsThread()
{
	m_pSource->Release();
	::CloseHandle(m_hThread);
}

void CThumbnailsThread::Stop()
{
	PauseJobs();

	m_stopping.Lock();
	m_stop.SetEvent();

	m_lock.Lock();
	m_jobs.clear();
	m_jobReady.ResetEvent();
	m_bRejectCurrentJob = true;
	m_lock.Unlock();

	m_stopping.Unlock();
}

unsigned int __stdcall CThumbnailsThread::RenderThreadProc(void* pvData)
{
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	CThumbnailsThread* pThread = reinterpret_cast<CThumbnailsThread*>(pvData);

	HANDLE hEvents[] = { pThread->m_jobReady.m_hObject, pThread->m_stop.m_hObject };
	while (::WaitForMultipleObjects(2, hEvents, false, INFINITE) == WAIT_OBJECT_0)
	{
		pThread->m_lock.Lock();

		if (pThread->m_jobs.empty())
		{
			pThread->m_lock.Unlock();
			continue;
		}

		Job job = pThread->m_jobs.front();
		pThread->m_bRejectCurrentJob = false;
		pThread->m_currentJob = job;
		pThread->m_jobs.pop_front();
		pThread->m_lock.Unlock();

		CDIB* pBitmap = pThread->Render(job);

		// Cannot stop while the owner is being updated
		pThread->m_stopping.Lock();

		pThread->m_lock.Lock();
		bool bNotify = (!pThread->m_bRejectCurrentJob);
		pThread->m_currentJob.nPage = -1;
		if (!pThread->m_jobs.empty() && !pThread->IsPaused())
			pThread->m_jobReady.SetEvent();
		pThread->m_lock.Unlock();

		if (bNotify)
			pThread->m_pOwner->OnUpdate(NULL, &BitmapMsg(THUMBNAIL_RENDERED, job.nPage, pBitmap));
		pThread->m_stopping.Unlock();
	}

	::CoUninitialize();

	// Ensure that a call to Stop() is finished
	pThread->m_stopping.Lock();
	pThread->m_stopping.Unlock();

	delete pThread;
	theApp.ThreadTerminated();

	return 0;
}

void CThumbnailsThread::PauseJobs()
{
	InterlockedExchange(&m_nPaused, 1);
}

void CThumbnailsThread::ResumeJobs()
{
	m_lock.Lock();

	InterlockedExchange(&m_nPaused, 0);
	if (!m_jobs.empty())
		m_jobReady.SetEvent();

	m_lock.Unlock();
}

bool CThumbnailsThread::IsPaused()
{
	return (InterlockedExchangeAdd(&m_nPaused, 0) == 1);
}

void CThumbnailsThread::RemoveAllJobs()
{
	m_lock.Lock();
	m_jobs.clear();
	m_lock.Unlock();
}

CDIB* CThumbnailsThread::Render(Job& job)
{
	CDIB* pBitmap = NULL;

	GP<DjVuImage> pImage = m_pSource->GetPage(job.nPage, NULL);
	if (pImage != NULL)
	{
		CSize szPage(pImage->get_width(), pImage->get_height());

		int nTotalRotate = GetTotalRotate(pImage, job.nRotate);
		if (nTotalRotate % 2 != 0)
			swap(szPage.cx, szPage.cy);

		if (szPage.cx > 0 && szPage.cy > 0)
		{
			CSize szDisplay = job.size;
			szDisplay.cy = static_cast<int>(1.0*szDisplay.cx*szPage.cy/szPage.cx + 0.5);

			if (szDisplay.cy > job.size.cy)
			{
				szDisplay.cy = job.size.cy;
				szDisplay.cx = min(job.size.cx, static_cast<int>(1.0*szDisplay.cy*szPage.cx/szPage.cy + 0.5));
			}

			pBitmap = CRenderThread::Render(pImage, szDisplay, job.displaySettings, CDjVuView::Color, job.nRotate, true);
		}
	}

	if (pBitmap == NULL || pBitmap->m_hObject == NULL)
	{
		delete pBitmap;
		pBitmap = NULL;
	}

	return pBitmap;
}

void CThumbnailsThread::AddJob(int nPage, int nRotate, const CSize& size, const CDisplaySettings& displaySettings)
{
	Job job;
	job.nPage = nPage;
	job.nRotate = nRotate;
	job.size = size;
	job.displaySettings = displaySettings;

	m_lock.Lock();
	if (m_currentJob.nPage == job.nPage
			&& m_currentJob.nRotate == nRotate
			&& m_currentJob.size == job.size
			&& m_currentJob.displaySettings == job.displaySettings)
	{
		m_lock.Unlock();
		return;
	}

	m_jobs.push_front(job);

	if (!IsPaused())
		m_jobReady.SetEvent();

	m_lock.Unlock();
}

void CThumbnailsThread::RejectCurrentJob()
{
	m_lock.Lock();
	if (m_currentJob.nPage != -1)
		m_bRejectCurrentJob = true;
	m_lock.Unlock();
}
