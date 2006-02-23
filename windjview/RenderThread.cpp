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

#include "RenderThread.h"
#include "Drawing.h"
#include "DjVuDoc.h"
#include "DjVuView.h"
#include "Scaling.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRenderThread class

CRenderThread::CRenderThread(CDjVuDoc* pDoc, CDjVuView* pOwner)
	: m_pOwner(pOwner), m_pDoc(pDoc), m_bPaused(false)
{
	m_currentJob.nPage = -1;
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

void CRenderThread::Stop()
{
	m_stop.SetEvent();
}

DWORD WINAPI CRenderThread::RenderThreadProc(LPVOID pvData)
{
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

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
		pData->m_currentJob = job;
		pData->m_jobs.pop_front();
		pData->m_pages[job.nPage] = pData->m_jobs.end();
		pData->m_lock.Unlock();

		switch (job.type)
		{
		case RENDER:
			pData->Render(job);
			break;

		case DECODE:
			pData->m_pDoc->GetPage(job.nPage);
			pData->m_pOwner->PageDecoded(job.nPage);
			break;

		case READINFO:
			pData->m_pDoc->GetPageInfo(job.nPage);
			pData->m_pOwner->PageDecoded(job.nPage);
			break;

		case CLEANUP:
			pData->m_pDoc->RemoveFromCache(job.nPage);
			break;
		}

		pData->m_lock.Lock();
		pData->m_currentJob.nPage = -1;
		if (!pData->m_jobs.empty() && !pData->m_bPaused)
			pData->m_jobReady.SetEvent();
		pData->m_lock.Unlock();

		if (::WaitForSingleObject(pData->m_stop.m_hObject, 0) == WAIT_OBJECT_0)
			break;
	}

	pData->m_finished.SetEvent();

	::CoUninitialize();
	return 0;
}

void CRenderThread::PauseJobs()
{
	m_lock.Lock();
	m_bPaused = true;
	m_lock.Unlock();
}

void CRenderThread::ResumeJobs()
{
	m_lock.Lock();

	m_bPaused = false;
	if (!m_jobs.empty())
		m_jobReady.SetEvent();

	m_lock.Unlock();
}

void CRenderThread::RemoveFromQueue(int nPage)
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
	CDIB* pBitmap = NULL;

	if (pImage != NULL)
	{
		RotateImage(pImage, job.nRotate);

		if (job.size.cx > 0 && job.size.cy > 0)
			pBitmap = Render(pImage, job.size, job.displaySettings, job.nDisplayMode);
	}

	if (pBitmap == NULL || pBitmap->m_hObject == NULL)
	{
		delete pBitmap;
		pBitmap = NULL;
	}

	m_pOwner->PageRendered(job.nPage, pBitmap);
}

CDIB* CRenderThread::Render(GP<DjVuImage> pImage, const CSize& size,
		const CDisplaySettings& displaySettings, int nDisplayMode)
{
	GRect rect(0, 0, size.cx, size.cy);
	if (rect.isempty())
		return NULL;

	int nScaleMethod = displaySettings.nScaleMethod;
	if (size.cx >= pImage->get_width() / 2 || size.cy >= pImage->get_height() / 2)
		nScaleMethod = CDisplaySettings::Default;

	if (nScaleMethod == CDisplaySettings::PnmScaleFixed)
		rect = GRect(0, 0, pImage->get_width(), pImage->get_height());

	GP<GBitmap> pGBitmap;
	GP<GPixmap> pGPixmap;

	try
	{
		switch (nDisplayMode)
		{
		case CDjVuView::BlackAndWhite:
			pGBitmap = pImage->get_bitmap(rect, rect, 4);
			break;

		case CDjVuView::Foreground:
			pGPixmap = pImage->get_fg_pixmap(rect, rect);
			if (pGPixmap == NULL)
				pGBitmap = pImage->get_bitmap(rect, rect, 4);
			break;

		case CDjVuView::Background:
			pGPixmap = pImage->get_bg_pixmap(rect, rect);
			break;

		case CDjVuView::Color:
		default:
			pGPixmap = pImage->get_pixmap(rect, rect);
			if (pGPixmap == NULL)
				pGBitmap = pImage->get_bitmap(rect, rect, 4);
		}
	}
	catch (GException&)
	{
	}
	catch (...)
	{
		ReportFatalError();
	}

	CDIB* pBitmap = NULL;

	if (pGPixmap != NULL)
	{
		if (nScaleMethod == CDisplaySettings::PnmScaleFixed)
			pGPixmap = RescalePnm(pGPixmap, size.cx, size.cy);

		pBitmap = RenderPixmap(*pGPixmap, displaySettings);
	}
	else if (pGBitmap != NULL)
	{
		if (nScaleMethod == CDisplaySettings::PnmScaleFixed)
			pGBitmap = RescalePnm(pGBitmap, size.cx, size.cy);

		pBitmap = RenderBitmap(*pGBitmap, displaySettings);
	}
	else
		pBitmap = RenderEmpty(CSize(rect.width(), rect.height()), displaySettings);

	return pBitmap;
}

void CRenderThread::AddJob(int nPage, int nRotate, const CSize& size,
		const CDisplaySettings& displaySettings, int nDisplayMode)
{
	Job job;
	job.nPage = nPage;
	job.nRotate = nRotate;
	job.nDisplayMode = nDisplayMode;
	job.displaySettings = displaySettings;
	job.size = size;
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

	if (m_currentJob.nPage == job.nPage && m_currentJob.type == RENDER &&
		job.type == RENDER && job.nRotate == m_currentJob.nRotate && 
		job.size == m_currentJob.size && job.nDisplayMode == m_currentJob.nDisplayMode &&
		job.displaySettings == m_currentJob.displaySettings)
	{
		m_lock.Unlock();
		return;
	}

	RemoveFromQueue(job.nPage);

	m_jobs.push_front(job);
	m_pages[job.nPage] = m_jobs.begin();

	if (!m_bPaused)
		m_jobReady.SetEvent();

	m_lock.Unlock();
}

void CRenderThread::RemoveAllJobs()
{
	m_lock.Lock();

	m_jobs.clear();
	m_pages.assign(m_pOwner->GetPageCount(), m_jobs.end());

	m_lock.Unlock();
}
