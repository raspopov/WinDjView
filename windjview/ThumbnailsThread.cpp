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
	: m_pSource(pSource), m_pOwner(pOwner), m_bPaused(false), m_bRejectCurrentJob(false)
{
	m_currentJob.nPage = -1;

	UINT dwThreadId;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, RenderThreadProc, this, 0, &dwThreadId);
	::SetThreadPriority(m_hThread,
			bIdle ? THREAD_PRIORITY_IDLE : THREAD_PRIORITY_BELOW_NORMAL);
}

CThumbnailsThread::~CThumbnailsThread()
{
	ASSERT(m_hThread == NULL);
}

void CThumbnailsThread::Delete()
{
	Stop();
	::WaitForSingleObject(m_finished, INFINITE);
	::CloseHandle(m_hThread);

	m_hThread = NULL;
	delete this;
}

void CThumbnailsThread::Stop()
{
	m_stop.SetEvent();
}

unsigned int __stdcall CThumbnailsThread::RenderThreadProc(void* pvData)
{
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

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
		pData->m_lock.Unlock();

		CDIB* pBitmap = pData->Render(job);

		pData->m_lock.Lock();
		bool bNotify = (!pData->m_bRejectCurrentJob);
		pData->m_bRejectCurrentJob = false;
		pData->m_currentJob.nPage = -1;
		if (bHasMoreJobs && !pData->m_bPaused)
			pData->m_jobReady.SetEvent();
		pData->m_lock.Unlock();

		if (bNotify)
			pData->m_pOwner->OnUpdate(NULL, &BitmapMsg(THUMBNAIL_RENDERED, job.nPage, pBitmap));

		if (::WaitForSingleObject(pData->m_stop.m_hObject, 0) == WAIT_OBJECT_0)
			break;
	}

	::CoUninitialize();
	pData->m_finished.SetEvent();

	return 0;
}

void CThumbnailsThread::PauseJobs()
{
	m_lock.Lock();
	m_bPaused = true;
	m_lock.Unlock();
}

void CThumbnailsThread::ResumeJobs()
{
	m_lock.Lock();

	m_bPaused = false;
	if (!m_jobs.empty())
		m_jobReady.SetEvent();

	m_lock.Unlock();
}

void CThumbnailsThread::ClearQueue()
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
			CSize szDisplay = m_szThumbnail;
			szDisplay.cx = szDisplay.cy * szPage.cx / szPage.cy;

			if (szDisplay.cx > m_szThumbnail.cx)
			{
				szDisplay.cx = m_szThumbnail.cx;
				szDisplay.cy = szDisplay.cx * szPage.cy / szPage.cx;
			}

			pBitmap = CRenderThread::Render(pImage, szDisplay, job.displaySettings, CDjVuView::Color, job.nRotate);
		}
	}

	if (pBitmap == NULL || pBitmap->m_hObject == NULL)
	{
		delete pBitmap;
		pBitmap = NULL;
	}

	return pBitmap;
}

void CThumbnailsThread::AddJob(int nPage, int nRotate, const CDisplaySettings& displaySettings)
{
	Job job;
	job.nPage = nPage;
	job.nRotate = nRotate;
	job.displaySettings = displaySettings;

	m_lock.Lock();
	if (m_currentJob.nPage == job.nPage && m_currentJob.nRotate == nRotate &&
			m_currentJob.displaySettings == job.displaySettings)
	{
		m_lock.Unlock();
		return;
	}

	m_jobs.push_front(job);

	if (!m_bPaused)
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
