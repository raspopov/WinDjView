//	WinDjView 0.1
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

#pragma once

#define WM_RENDER_FINISHED (WM_USER + 17)

class CLock
{
public:
	CLock() : m_nLock(0) {}
	void Lock();
	void Unlock();

private:
	LONG m_nLock;
	CLock(const CLock& rhs) {}
};

class CSignal
{
public:
	CSignal() : m_nSignal(0) {}
	bool IsSet() const;
	void Set();
	void Reset();
	void Wait();

private:
	mutable LONG m_nSignal;
	CSignal(const CSignal& rhs) {}
};

class CRenderThread
{
public:
	CRenderThread(CWnd* pOwner);
	~CRenderThread();

	DWORD AddJob(GP<DjVuImage> pImage, CRect rcAll, CRect rcClip, bool bClearQueue = false);

private:
	CLock m_lock;
	CSignal m_stop;
	CWnd* m_pOwner;

	struct Job
	{
		GP<DjVuImage> pImage;
		CRect rcAll;
		CRect rcClip;
		DWORD nCode;
	};

	list<Job> m_jobs;

	static DWORD WINAPI RenderThreadProc(LPVOID pvData);
	void Render(Job& job);
};
