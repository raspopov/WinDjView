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

#pragma once

#include "AppSettings.h"
#include "Global.h"

class DjVuSource;
class CDIB;


class CThumbnailsThread
{
public:
	CThumbnailsThread(DjVuSource* pSource, Observer* pOwner, bool bIdle = false);
	void Stop();

	void AddJob(int nPage, int nRotate, const CSize& size,
			const CDisplaySettings& displaySettings);
	void RemoveAllJobs();

	void PauseJobs();
	void ResumeJobs();
	bool IsPaused();

	void RejectCurrentJob();

private:
	HANDLE m_hThread;
	CCriticalSection m_lock;
	CCriticalSection m_stopping;
	CEvent m_stop;
	CEvent m_jobReady;
	DjVuSource* m_pSource;
	Observer* m_pOwner;
	long m_nPaused;

	struct Job
	{
		int nPage;
		int nRotate;
		CSize size;
		CDisplaySettings displaySettings;
	};
	list<Job> m_jobs;
	Job m_currentJob;
	bool m_bRejectCurrentJob;

	static unsigned int __stdcall RenderThreadProc(void* pvData);
	CDIB* Render(Job& job);
	~CThumbnailsThread();
};
