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

#include "DjVuView.h"

class CDjVuDoc;
class CDIB;

class CRenderThread
{
public:
	CRenderThread(CDjVuDoc* pDoc, CDjVuView* pOwner);
	void Stop();
	void Delete();

	void AddJob(int nPage, int nRotate, const CSize& size, const CDisplaySettings& displaySettings,
		int nDisplayMode = CDjVuView::Color);
	void AddDecodeJob(int nPage);
	void AddReadInfoJob(int nPage);
	void AddCleanupJob(int nPage);
	void RemoveAllJobs();

	static CDIB* Render(GP<DjVuImage> pImage, const CSize& size,
		const CDisplaySettings& displaySettings, int nDisplayMode = CDjVuView::Color);

	void PauseJobs();
	void ResumeJobs();

private:
	HANDLE m_hThread, m_hStopThread;
	CCriticalSection m_lock;
	CEvent m_stop;
	CEvent m_finished;
	CEvent m_jobReady;
	CDjVuView* m_pOwner;
	CDjVuDoc* m_pDoc;
	bool m_bPaused;

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

	static unsigned int __stdcall RenderThreadProc(void* pvData);
	void Render(Job& job);
	void AddJob(const Job& job);
	void RemoveFromQueue(int nPage);
	~CRenderThread();
};
