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

#include "RenderThread.h"
#include "Drawing.h"
#include "DjVuDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRenderThread class

CRenderThread::CRenderThread(CDjVuDoc* pDoc, CWnd* pOwner)
	: m_pOwner(pOwner), m_pDoc(pDoc)
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

		if (::WaitForSingleObject(pData->m_stop.m_hObject, 0) == WAIT_OBJECT_0)
			break;
	}

	pData->m_finished.SetEvent();
	return 0;
}

void CRenderThread::AddJob(int nPage, int nRotate, const CRect& rcAll, const CRect& rcClip)
{
	Job job;
	job.nPage = nPage;
	job.nRotate = nRotate;
	job.rcAll = rcAll;
	job.rcClip = rcClip;

	m_lock.Lock();

	// Delete jobs with the same nPage
	list<Job>::iterator it = m_jobs.begin();
	while (it != m_jobs.end())
	{
		if ((*it).nPage == nPage)
			m_jobs.erase(it++);
		else
			++it;
	}

	m_jobs.push_front(job);

	m_lock.Unlock();

	m_jobReady.SetEvent();
}

void CRenderThread::RemoveFromQueue(int nPage)
{
	m_lock.Lock();

	// Delete jobs with the same nPage
	list<Job>::iterator it = m_jobs.begin();
	while (it != m_jobs.end())
	{
		if ((*it).nPage == nPage)
			m_jobs.erase(it++);
		else
			++it;
	}

	m_lock.Unlock();
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
