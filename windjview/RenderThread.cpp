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

#include "RenderThread.h"
#include "Drawing.h"
#include "DjVuDoc.h"
#include "DjVuView.h"
#include "Scaling.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRenderThread class

CRenderThread::CRenderThread(DjVuSource* pSource, Observer* pOwner)
	: m_pOwner(pOwner), m_pSource(pSource), m_nPaused(0), m_bRejectCurrentJob(false)
{
	m_pSource->AddRef();

	m_currentJob.nPage = -1;
	m_pages.resize(m_pSource->GetPageCount(), m_jobs.end());

	UINT dwThreadId;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, RenderThreadProc, this, 0, &dwThreadId);
	::SetThreadPriority(m_hThread, THREAD_PRIORITY_BELOW_NORMAL);
	theApp.ThreadStarted();
}

CRenderThread::~CRenderThread()
{
	::CloseHandle(m_hThread);
	m_pSource->Release();
	theApp.ThreadTerminated();
}

void CRenderThread::Stop()
{
	m_stopping.Lock();

	m_stop.SetEvent();

	m_lock.Lock();
	m_jobs.clear();
	m_jobReady.ResetEvent();
	m_bRejectCurrentJob = true;
	PauseJobs();
	m_lock.Unlock();

	m_stopping.Unlock();
}

unsigned int __stdcall CRenderThread::RenderThreadProc(void* pvData)
{
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	CRenderThread* pThread = reinterpret_cast<CRenderThread*>(pvData);

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
		pThread->m_pages[job.nPage] = pThread->m_jobs.end();
		pThread->m_lock.Unlock();

		CDIB* pBitmap = NULL;

		switch (job.type)
		{
		case RENDER:
			pBitmap = pThread->Render(job);
			break;

		case DECODE:
			pThread->m_pSource->GetPage(job.nPage, pThread->m_pOwner);
			break;

		case READINFO:
			pThread->m_pSource->GetPageInfo(job.nPage);
			break;

		case CLEANUP:
			pThread->m_pSource->RemoveFromCache(job.nPage, pThread->m_pOwner);
			break;
		}

		// Cannot stop while the owner is being updated
		pThread->m_stopping.Lock();

		pThread->m_lock.Lock();
		bool bNotify = (!pThread->m_bRejectCurrentJob);
		pThread->m_currentJob.nPage = -1;
		if (!pThread->m_jobs.empty() && !pThread->IsPaused())
			pThread->m_jobReady.SetEvent();
		pThread->m_lock.Unlock();

		if (bNotify)
		{
			switch (job.type)
			{
			case RENDER:
				pThread->m_pOwner->OnUpdate(NULL, &BitmapMsg(PAGE_RENDERED, job.nPage, pBitmap));
				break;

			case DECODE:
			case READINFO:
				pThread->m_pOwner->OnUpdate(NULL, &PageMsg(PAGE_DECODED, job.nPage));
				break;
			}
		}
		else
		{
			delete pBitmap;
		}

		pThread->m_stopping.Unlock();
	}

	::CoUninitialize();

	// Ensure that a call to Stop() is finished
	pThread->m_stopping.Lock();
	pThread->m_stopping.Unlock();

	// Clean page cache
	for (int nPage = 0; nPage < pThread->m_pSource->GetPageCount(); ++nPage)
		pThread->m_pSource->RemoveFromCache(nPage, pThread->m_pOwner);

	delete pThread;

	return 0;
}

void CRenderThread::PauseJobs()
{
	InterlockedExchange(&m_nPaused, 1);
}

void CRenderThread::ResumeJobs()
{
	m_lock.Lock();

	InterlockedExchange(&m_nPaused, 0);
	if (!m_jobs.empty())
		m_jobReady.SetEvent();

	m_lock.Unlock();
}

bool CRenderThread::IsPaused()
{
	return (InterlockedExchangeAdd(&m_nPaused, 0) == 1);
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

CDIB* CRenderThread::Render(Job& job)
{
	GP<DjVuImage> pImage = m_pSource->GetPage(job.nPage, m_pOwner);
	CDIB* pBitmap = NULL;

	if (pImage != NULL)
	{
		if (job.size.cx > 0 && job.size.cy > 0)
			pBitmap = Render(pImage, job.size, job.displaySettings, job.nDisplayMode, job.nRotate);
	}

	if (pBitmap == NULL || pBitmap->m_hObject == NULL)
	{
		delete pBitmap;
		pBitmap = NULL;
	}

	return pBitmap;
}

CDIB* CRenderThread::Render(GP<DjVuImage> pImage, const CSize& size,
		const CDisplaySettings& displaySettings, int nDisplayMode,
		int nRotate, bool bThumbnail)
{
	if (size.cx <= 0 || size.cy <= 0)
		return NULL;

	CSize szImage(pImage->get_width(), pImage->get_height());
	int nTotalRotate = GetTotalRotate(pImage, nRotate);

	CSize szScaled(size);
	if (nTotalRotate % 2 != 0)
		swap(szScaled.cx, szScaled.cy);

	GRect rect(0, 0, szScaled.cx, szScaled.cy);

	bool bScalePnmFixed = true;
	bool bScaleSubpix = false;

	// Use fast scaling for thumbnails.
	if (bThumbnail)
		bScalePnmFixed = false;

	if (displaySettings.bScaleSubpix)
	{
		// Use subpixel scaling in most cases, unless scaled image size
		// is too small or if we are upscaling.
		if ((szScaled.cx < 100 || szScaled.cy < 100)
				|| (szScaled.cx >= szImage.cx || szScaled.cy >= szImage.cy))
			bScalePnmFixed = false;
	}
	else
	{
		// Results from PnmScaleFixed are comparable to libdjvu scaling,
		// when zoom factor is greater than 0.5, so use default faster scaling
		// in this case. Additionally, use the default scaling when requested
		// image size is small, since quality does not matter at this
		// scale. NOTE: this also deals with the special case of size (1, 1),
		// which can force PnmScaleFixed into an infinite loop.
		if ((szScaled.cx < 100 || szScaled.cy < 100)
				|| (szScaled.cx >= szImage.cx / 2 || szScaled.cy >= szImage.cy / 2))
			bScalePnmFixed = false;

		// Disable PnmFixed scaling if we perform an integer reduction of the image.
		for (int nReduction = 1; nReduction <= 15; ++nReduction)
		{
			if (szScaled.cx*nReduction > szImage.cx - nReduction
					&& szScaled.cx*nReduction < szImage.cx + nReduction
					&& szScaled.cy*nReduction > szImage.cy - nReduction
					&& szScaled.cy*nReduction < szImage.cy + nReduction)
			{
				bScalePnmFixed = false;
				break;
			}
		}
	}

	// Disable PnmFixed scaling for color images according to settings
	GP<IW44Image> bg44 = pImage->get_bg44();
	GP<GPixmap> bgpm = pImage->get_bgpm();
	GP<GPixmap> fgpm = pImage->get_fgpm();
	if (!displaySettings.bScaleColorPnm && (bg44 != NULL || bgpm != NULL || fgpm != NULL)
			&& nDisplayMode != CDjVuView::BlackAndWhite)
		bScalePnmFixed = false;

	if (bScalePnmFixed && displaySettings.bScaleSubpix)
		bScaleSubpix = true;

	if (bScalePnmFixed)
		rect = GRect(0, 0, szImage.cx, szImage.cy);

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
		return NULL;
	}
	catch (CMemoryException*)
	{
		return NULL;
	}
	catch (...)
	{
		theApp.ReportFatalError();
	}

	CDIB* pBitmap = NULL;

	if (pGPixmap != NULL)
	{
		if (nTotalRotate != 0)
			pGPixmap = pGPixmap->rotate(nTotalRotate);

		if (bScalePnmFixed)
		{
			if (bScaleSubpix)
				pGPixmap = RescalePnm_subpix(pGPixmap, size.cx, size.cy);
			else
				pGPixmap = RescalePnm(pGPixmap, size.cx, size.cy);
		}

		pBitmap = RenderPixmap(*pGPixmap, displaySettings);
	}
	else if (pGBitmap != NULL)
	{
		if (nTotalRotate != 0)
			pGBitmap = pGBitmap->rotate(nTotalRotate);

		if (bScalePnmFixed)
		{
			if (bScaleSubpix)
				pGPixmap = RescalePnm_subpix(pGBitmap, size.cx, size.cy);
			else
				pGBitmap = RescalePnm(pGBitmap, size.cx, size.cy);
		}

		if (pGPixmap)
			pBitmap = RenderPixmap(*pGPixmap, displaySettings);
		else
			pBitmap = RenderBitmap(*pGBitmap, displaySettings);
	}
	else
	{
		pBitmap = RenderEmpty(size, displaySettings);
	}

	if (pBitmap != NULL)
		pBitmap->SetDPI(pImage->get_dpi());

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

	if (!IsPaused())
		m_jobReady.SetEvent();

	m_lock.Unlock();
}

void CRenderThread::RemoveAllJobs()
{
	m_lock.Lock();

	m_jobs.clear();
	m_pages.assign(m_pSource->GetPageCount(), m_jobs.end());

	m_lock.Unlock();
}

void CRenderThread::RejectCurrentJob()
{
	m_lock.Lock();
	if (m_currentJob.nPage != -1)
		m_bRejectCurrentJob = true;
	m_lock.Unlock();
}
