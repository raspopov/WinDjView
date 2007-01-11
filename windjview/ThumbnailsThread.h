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

#include "AppSettings.h"

class CDjVuDoc;
class CThumbnailsView;


class CThumbnailsThread
{
public:
	CThumbnailsThread(CDjVuDoc* pDoc, CThumbnailsView* pOwner, bool bIdle = false);
	void Stop();
	void Delete();

	void SetThumbnailSize(CSize szThumbnail) { m_szThumbnail = szThumbnail; }

	void AddJob(int nPage, int nRotate, const CDisplaySettings& displaySettings);

	void PauseJobs();
	void ResumeJobs();
	void ClearQueue();

	void RejectCurrentJob();

private:
	HANDLE m_hThread, m_hStopThread;
	CCriticalSection m_lock;
	CEvent m_stop;
	CEvent m_finished;
	CEvent m_jobReady;
	CThumbnailsView* m_pOwner;
	CDjVuDoc* m_pDoc;
	bool m_bPaused;
	CSize m_szThumbnail;

	struct Job
	{
		int nPage;
		int nRotate;
		CDisplaySettings displaySettings;
	};
	list<Job> m_jobs;
	Job m_currentJob;
	bool m_bRejectCurrentJob;

	static unsigned int __stdcall RenderThreadProc(void* pvData);
	CDIB* Render(Job& job);
	~CThumbnailsThread();
};
