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
//	http://www.gnu.org/copyleft/gpl.html

// $Id$

#pragma once

#define WM_RENDER_THUMB_FINISHED (WM_USER + 19)

class CDjVuDoc;
class CThumbnailsView;
class CDIB;


class CThumbnailsThread
{
public:
	CThumbnailsThread(CDjVuDoc* pDoc, CThumbnailsView* pOwner, bool bIdle = false);
	~CThumbnailsThread();
	void Stop();

	void SetThumbnailSize(CSize szThumbnail) { m_szThumbnail = szThumbnail; }

	void AddJob(int nPage, int nRotate);

	void PauseJobs();
	void ResumeJobs();
	void ClearQueue();

	void RejectCurrentJob();

private:
	HANDLE m_hThread;
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
	};
	list<Job> m_jobs;
	Job m_currentJob;
	bool m_bRejectCurrentJob;

	static DWORD WINAPI RenderThreadProc(LPVOID pvData);
	void Render(Job& job);
};
