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

#include "StdAfx.h"
#include "WinDjView.h"

#include "RenderThread.h"
#include "Drawing.h"
#include "DjVuDoc.h"
#include "DjVuView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRenderThread class

CRenderThread::CRenderThread(CDjVuDoc* pDoc, CDjVuView* pOwner)
	: m_pOwner(pOwner), m_pDoc(pDoc), m_bPaused(false)
{
	currentJob.nPage = -1;
	m_pages.resize(m_pOwner->GetPageCount(), m_jobs.end());

	DWORD dwThreadId;
	m_hThread = ::CreateThread(NULL, 0, RenderThreadProc, this, 0, &dwThreadId);
	::SetThreadPriority(m_hThread, THREAD_PRIORITY_BELOW_NORMAL);
}

CRenderThread::~CRenderThread()
{
	m_stop.SetEvent();
	::WaitForSingleObject(m_finished.m_hObject, INFINITE);
	::CloseHandle(m_hThread);
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
		pData->currentJob = job;
		pData->m_jobs.pop_front();
		pData->m_pages[job.nPage] = pData->m_jobs.end();

		if (!pData->m_jobs.empty())
			pData->m_jobReady.SetEvent();

		pData->m_lock.Unlock();

		switch (job.type)
		{
		case RENDER:
			pData->Render(job);
			break;

		case DECODE:
			pData->m_pDoc->GetPage(job.nPage);
			pData->m_pOwner->PostMessage(WM_PAGE_DECODED, job.nPage);
			break;

		case READINFO:
			pData->m_pDoc->GetPageInfo(job.nPage);
			pData->m_pOwner->PostMessage(WM_PAGE_DECODED, job.nPage);
			break;

		case CLEANUP:
			pData->m_pDoc->RemoveFromCache(job.nPage);
			break;
		}

		pData->currentJob.nPage = -1;

		if (::WaitForSingleObject(pData->m_stop.m_hObject, 0) == WAIT_OBJECT_0)
			break;
	}

	pData->m_finished.SetEvent();
	return 0;
}

void CRenderThread::PauseJobs()
{
	m_bPaused = true;
}

void CRenderThread::ResumeJobs()
{
	m_bPaused = false;

	m_lock.Lock();
	bool bHasJobs = !m_jobs.empty();
	m_lock.Unlock();

	if (bHasJobs)
		m_jobReady.SetEvent();
}

void CRenderThread::RemoveFromQueue(int nPage)
{
	m_lock.Lock();
	RemoveFromQueueImpl(nPage);
	m_lock.Unlock();
}

void CRenderThread::RemoveFromQueueImpl(int nPage)
{
	// Delete jobs with the same nPage
	list<Job>::iterator it = m_pages[nPage];
	if (it != m_jobs.end())
	{
		m_jobs.erase(it);
		m_pages[nPage] = m_jobs.end();
	}
}

void CRenderThread::Render(Job& job)
{
	GP<DjVuImage> pImage = m_pDoc->GetPage(job.nPage);

	pImage->set_rotate(job.nRotate);

	GRect rcAll(job.rcAll.left, job.rcAll.top, job.rcAll.Width(), job.rcAll.Height());
	GRect rcClip(job.rcClip.left, job.rcClip.top, job.rcClip.Width(), job.rcClip.Height());
	if (rcAll.isempty() || rcClip.isempty() || !rcAll.contains(rcClip))
		return;

	CDIB* pBitmap = Render(pImage, rcClip, rcAll);

	if (pBitmap == NULL || pBitmap->m_hObject == NULL)
	{
		delete pBitmap;
		return;
	}

	if (m_pOwner->m_nPendingPage == job.nPage)
		m_pOwner->OnRenderFinished(job.nPage, reinterpret_cast<LPARAM>(pBitmap));
	else
		m_pOwner->PostMessage(WM_RENDER_FINISHED, job.nPage, reinterpret_cast<LPARAM>(pBitmap));
}

CDIB* CRenderThread::Render(GP<DjVuImage> pImage, const GRect& rcClip, const GRect& rcAll)
{
	GP<GBitmap> pGBitmap;
	GP<GPixmap> pGPixmap;

	if (pImage->is_legal_photo() || pImage->is_legal_compound())
	{
		pGPixmap = pImage->get_pixmap(rcClip, rcAll);
	}
	else if (pImage->is_legal_bilevel())
	{
		pGBitmap = pImage->get_bitmap(rcClip, rcAll, 4);
	}
	else
	{
		// Try to get both
		pGPixmap = pImage->get_pixmap(rcClip, rcAll);
		if (pGPixmap == NULL)
			pGBitmap = pImage->get_bitmap(rcClip, rcAll, 4);
	}

	CDIB* pBitmap = NULL;

	if (pGPixmap != NULL)
		pBitmap = RenderPixmap(*pGPixmap);
	else if (pGBitmap != NULL)
		pBitmap = RenderBitmap(*pGBitmap);

	return pBitmap;
}

void CRenderThread::AddJob(int nPage, int nRotate, const CRect& rcAll, const CRect& rcClip)
{
	Job job;
	job.nPage = nPage;
	job.nRotate = nRotate;
	job.rcAll = rcAll;
	job.rcClip = rcClip;
	job.type = RENDER;

	AddJob(job);
}

void CRenderThread::AddDecodeJob(int nPage)
{
	Job job;
	job.nPage = nPage;
	job.type = DECODE;

	AddJob(job);
}

void CRenderThread::AddReadInfoJob(int nPage)
{
	Job job;
	job.nPage = nPage;
	job.type = READINFO;

	AddJob(job);
}

void CRenderThread::AddCleanupJob(int nPage)
{
	Job job;
	job.nPage = nPage;
	job.type = CLEANUP;

	AddJob(job);
}

void CRenderThread::AddJob(const Job& job)
{
	m_lock.Lock();

	if (currentJob.nPage == job.nPage && job.type == RENDER && currentJob.type == RENDER &&
		job.rcAll == currentJob.rcAll && job.rcClip == currentJob.rcClip)
	{
		m_lock.Unlock();
		return;
	}

	RemoveFromQueueImpl(job.nPage);

	m_jobs.push_front(job);
	m_pages[job.nPage] = m_jobs.begin();

	m_lock.Unlock();

	if (!m_bPaused)
		m_jobReady.SetEvent();
}
