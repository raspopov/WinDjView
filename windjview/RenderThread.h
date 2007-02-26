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

#pragma once

#include "Global.h"
#include "DjVuView.h"
class DjVuSource;
class CDIB;


class CRenderThread
{
public:
	CRenderThread(DjVuSource* pSource, Observer* pOwner);
	void Stop();

	void AddJob(int nPage, int nRotate, const CSize& size, const CDisplaySettings& displaySettings,
		int nDisplayMode = CDjVuView::Color);
	void AddDecodeJob(int nPage);
	void AddReadInfoJob(int nPage);
	void AddCleanupJob(int nPage);
	void RemoveAllJobs();

	static CDIB* Render(GP<DjVuImage> pImage, const CSize& size,
		const CDisplaySettings& displaySettings, int nDisplayMode, int nRotate);

	void PauseJobs();
	void ResumeJobs();
	bool IsPaused();

	void RejectCurrentJob();

private:
	HANDLE m_hThread, m_hStopThread;
	CCriticalSection m_lock;
	CCriticalSection m_stopping;
	CEvent m_stop;
	CEvent m_jobReady;
	Observer* m_pOwner;
	DjVuSource* m_pSource;
	long m_nPaused;

	enum JobType { RENDER, DECODE, READINFO, CLEANUP };

	struct Job
	{
		int nPage;
		int nRotate;
		int nDisplayMode;
		CDisplaySettings displaySettings;
		CSize size;
		JobType type;
	};
	list<Job> m_jobs;
	vector<list<Job>::iterator> m_pages;
	Job m_currentJob;
	bool m_bRejectCurrentJob;

	static unsigned int __stdcall RenderThreadProc(void* pvData);
	CDIB* Render(Job& job);
	void AddJob(const Job& job);
	void RemoveFromQueue(int nPage);
	~CRenderThread();
};
