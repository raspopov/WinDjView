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

#include "DecodeThread.h"
#include "DjVuDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDecodeThread class

CDecodeThread::CDecodeThread(CDjVuDoc* pDoc)
	: m_pDoc(pDoc), currentJob(-1, true)
{
	for (int i = 0; i < pDoc->m_pDjVuDoc->get_pages_num(); ++i)
		m_jobs.push_back(Job(i, true));

	DWORD dwThreadId;
	m_hThread = ::CreateThread(NULL, 0, DecodeThreadProc, this, 0, &dwThreadId);
	::SetThreadPriority(m_hThread, THREAD_PRIORITY_LOWEST);
}

CDecodeThread::~CDecodeThread()
{
	m_stop.SetEvent();
	::WaitForSingleObject(m_finished.m_hObject, INFINITE);
	::CloseHandle(m_hThread);
}

DWORD WINAPI CDecodeThread::DecodeThreadProc(LPVOID pvData)
{
	CDecodeThread* pData = reinterpret_cast<CDecodeThread*>(pvData);

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

		if (!pData->m_jobs.empty())
			pData->m_jobReady.SetEvent();

		pData->m_lock.Unlock();

		if (job.bReadInfo)
		{
			PageInfo info = ReadPageInfo(pData->m_pDoc->m_pDjVuDoc, job.nPage);
			pData->m_pDoc->PageDecoded(job.nPage, NULL, info);
		}
		else
		{
			GP<DjVuImage> pImage = pData->m_pDoc->m_pDjVuDoc->get_page(job.nPage);
			PageInfo info(pImage);
			pData->m_pDoc->PageDecoded(job.nPage, pImage, info);
		}

		pData->currentJob.nPage = -1;

		if (::WaitForSingleObject(pData->m_stop.m_hObject, 0) == WAIT_OBJECT_0)
			break;
	}

	pData->m_finished.SetEvent();
	return 0;
}

PageInfo CDecodeThread::ReadPageInfo(GP<DjVuDocument> pDjVuDoc, int nPage)
{
	ASSERT(nPage >= 0 && nPage < pDjVuDoc->get_pages_num());
	PageInfo pageInfo;

	G_TRY
	{
		// Get raw data from the document and decode only page info chunk
		GP<DjVuFile> file(pDjVuDoc->get_djvu_file(nPage));
		GP<DataPool> pool = file->get_init_data_pool();
		GP<ByteStream> stream = pool->get_stream();
		GP<IFFByteStream> iff(IFFByteStream::create(stream));

		// Check file format
		GUTF8String chkid;
		if (!iff->get_chunk(chkid) ||
			(chkid != "FORM:DJVI" && chkid != "FORM:DJVU" &&
			chkid != "FORM:PM44" && chkid != "FORM:BM44"))
		{
			return pageInfo;
		}

		// Find chunk with page info
		while (iff->get_chunk(chkid) != 0)
		{
			GP<ByteStream> chunk_stream = iff->get_bytestream();

			if (chkid == "INFO")
			{
				// Get page dimensions and resolution from info chunk
				GP<DjVuInfo> pInfo = DjVuInfo::create();
				pInfo->decode(*chunk_stream);

				// Check data for consistency
				pageInfo.szPage.cx = max(pInfo->width, 0);
				pageInfo.szPage.cy = max(pInfo->height, 0);
				pageInfo.nDPI = max(pInfo->dpi, 0);
			}
			else if (chkid == "PM44" || chkid == "BM44")
			{
				// Get image dimensions and resolution from bitmap chunk
				UINT serial = chunk_stream->read8();
				UINT slices = chunk_stream->read8();
				UINT major = chunk_stream->read8();
				UINT minor = chunk_stream->read8();

				UINT xhi = chunk_stream->read8();
				UINT xlo = chunk_stream->read8();
				UINT yhi = chunk_stream->read8();
				UINT ylo = chunk_stream->read8();

				pageInfo.szPage.cx = (xhi << 8) | xlo;
				pageInfo.szPage.cy = (yhi << 8) | ylo;
				pageInfo.nDPI = 100;
			}
			else if (chkid == "TXTa" || chkid == "TXTz")
			{
				pageInfo.pTextStream = ByteStream::create();
				const GP<IFFByteStream> iffout = IFFByteStream::create(pageInfo.pTextStream);
				iffout->put_chunk(chkid);
				iffout->copy(*chunk_stream);
				iffout->close_chunk();
			}

			// Close chunk
			iff->seek_close_chunk();
		}
	}
	G_CATCH(ex)
	{
		ex;
	}
	G_ENDCATCH;

	return pageInfo;
}

void CDecodeThread::StartDecodePage(int nPage)
{
	m_lock.Lock();
	if (currentJob.nPage == nPage && !currentJob.bReadInfo)
	{
		// Page is currently being decoded
		m_lock.Unlock();
		return;
	}

	list<Job>::iterator it = find(m_jobs.begin(), m_jobs.end(), nPage);
	if (it != m_jobs.end())
	{
		m_jobs.erase(it);
	}
	m_jobs.push_front(Job(nPage, false));
	m_lock.Unlock();

	m_jobReady.SetEvent();
}
