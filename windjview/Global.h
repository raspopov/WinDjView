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


class RefCount
{
public:
	RefCount() : m_nRefCount(1) {}
	virtual ~RefCount() = 0;

	virtual void AddRef()
	{
		InterlockedIncrement(&m_nRefCount);
	}

	virtual void Release()
	{
		if (InterlockedDecrement(&m_nRefCount) <= 0)
			delete this;
	}

protected:
	long m_nRefCount;
};


struct Message
{
	Message(int c) : code(c) {}
	
	int code;
};

class Observable;

class Observer
{
public:
	virtual ~Observer() {}
	virtual void OnUpdate(const Observable* source, const Message* message) = 0;
};

class Observable
{
public:
	void AddObserver(Observer* observer);
	void RemoveObserver(Observer* observer);
	bool HasObservers() const { return !m_observers.empty(); }
	bool IsObservedBy(Observer* observer) const
		{ return m_observers.find(observer) != m_observers.end(); }
	void UpdateObservers(const Message& message);
	
protected:
	set<Observer*> m_observers;
};

bool IsWinVistaOrLater();

void MakeWString(const CString& strText, wstring& result);
bool MakeWString(const GUTF8String& text, wstring& result);
void MakeANSIString(const wstring& text, string& result);
void MakeANSIString(const CString& strText, string& result);
CString MakeCString(const wstring& text);
CString MakeCString(const GUTF8String& text);
GUTF8String MakeUTF8String(const CString& strText);
GUTF8String MakeUTF8String(const wstring& strText);

bool MoveToTrash(LPCTSTR lpszFileName);

void CreateSystemDialogFont(CFont& font);
void CreateSystemIconFont(CFont& font);
void CreateSystemMenuFont(CFont& font);

UINT GetMouseScrollLines();

CRect GetMonitorWorkArea(const CPoint& point);
CRect GetMonitorWorkArea(CWnd* pWnd);
CRect GetMonitorRect(CWnd* pWnd);

bool IsFromCurrentProcess(CWnd* pWnd);

int CompareVersions(const CString& strFirst, const CString& strSecond);
CString FormatDouble(double fValue);
void AFXAPI DDX_MyText(CDataExchange* pDX, int nIDC, double& value, double def = 0.0, LPCTSTR pszSuffix = NULL);
void AFXAPI DDX_MyText(CDataExchange* pDX, int nIDC, DWORD& value, DWORD def = 0, LPCTSTR pszSuffix = NULL);

void SendMessageToVisibleDescendants(HWND hWnd, UINT message, WPARAM wParam = 0, LPARAM lParam = 0);

inline CRect GetClientRect(const CWnd* pWnd)
{
	ASSERT(pWnd != NULL);
	CRect rect;
	pWnd->GetClientRect(rect);
	return rect;
}

inline CRect GetClientRect(const CWnd& wnd)
	{ return GetClientRect(&wnd); }

inline CSize GetClientSize(const CWnd* pWnd)
{
	ASSERT(pWnd != NULL);
	CRect rect;
	pWnd->GetClientRect(rect);
	return rect.Size();
}

inline CSize GetClientSize(const CWnd& wnd)
	{ return GetClientSize(&wnd); }

#define PAGE_RENDERED 1
#define PAGE_DECODED 2
#define LINK_CLICKED 3
#define SEARCH_RESULT_CLICKED 4
#define THUMBNAIL_RENDERED 5
#define THUMBNAIL_CLICKED 6
#define CURRENT_PAGE_CHANGED 7
#define ROTATE_CHANGED 8
#define VIEW_ACTIVATED 9
#define ZOOM_CHANGED 10
#define APP_SETTINGS_CHANGED 11
#define APP_LANGUAGE_CHANGED 12
#define BOOKMARK_ADDED 13
#define ANNOTATION_ADDED 14
#define BOOKMARK_DELETED 15
#define ANNOTATION_DELETED 16
#define BOOKMARK_CLICKED 17
#define VIEW_INITIALIZED 18
#define SOURCE_RELEASED 19
#define KEY_STATE_CHANGED 20
#define DICT_LIST_CHANGED 21
#define SIDEBAR_TAB_CHANGED 22
#define TAB_ACTIVATING 23
#define TAB_ACTIVATED 24
#define TAB_CLOSED 25
#define ANNOTATIONS_CHANGED 26
#define BOOKMARKS_CHANGED 27
#define PRINT_PAGES 28
#define EXPORT_PAGES 29

class CDIB;
struct Bookmark;
struct Annotation;

struct PageMsg : public Message
{
	PageMsg(int msg, int nPage_)
		: Message(msg), nPage(nPage_) {}

	int nPage;
};

struct BitmapMsg : public Message
{
	BitmapMsg(int msg, int nPage_, CDIB* pDIB_)
		: Message(msg), nPage(nPage_), pDIB(pDIB_) {}

	int nPage;
	CDIB* pDIB;
};

struct LinkClicked : public Message
{
	LinkClicked(const GUTF8String& url_)
		: Message(LINK_CLICKED), url(url_) {}

	const GUTF8String& url;
};

struct SearchResultClicked : public Message
{
	SearchResultClicked(int nPage_, int nSelStart_, int nSelEnd_)
		: Message(SEARCH_RESULT_CLICKED), nPage(nPage_), nSelStart(nSelStart_), nSelEnd(nSelEnd_) {}

	int nPage, nSelStart, nSelEnd;
};

struct RotateChanged : public Message
{
	RotateChanged(int nRotate_)
		: Message(ROTATE_CHANGED), nRotate(nRotate_) {}

	int nRotate;
};

struct AnnotationMsg : public Message
{
	AnnotationMsg(int msg, Annotation* pAnno_, int nPage_)
		: Message(msg), pAnno(pAnno_), nPage(nPage_) {}

	Annotation* pAnno;
	int nPage;
};

struct BookmarkMsg : public Message
{
	BookmarkMsg(int msg, const Bookmark* pBookmark_)
		: Message(msg), pBookmark(pBookmark_) {}

	const Bookmark* pBookmark;
};

struct KeyStateChanged : public Message
{
	KeyStateChanged(UINT nKey_, bool bPressed_)
		: Message(KEY_STATE_CHANGED), nKey(nKey_), bPressed(bPressed_) {}

	UINT nKey;
	bool bPressed;
};

struct TabMsg : public Message
{
	TabMsg(int msg, CWnd* pWnd_, int nTab_)
		: Message(msg), pWnd(pWnd_), nTab(nTab_) {}

	CWnd* pWnd;
	int nTab;
};

struct PageRangeMsg : public Message
{
	PageRangeMsg(int msg, const set<int>& pages_)
		: Message(msg), pages(pages_) {}

	const set<int>& pages;
};


struct MD5
{
	MD5();
	MD5(const void* data, size_t len);
	MD5(const MD5& md5);

	void Append(const void* data, size_t len);
	void Finish();

	CString ToString() const;
	bool operator==(const MD5& rhs) const
		{ return memcmp(md, rhs.md, sizeof(md)) == 0; }
	bool operator<(const MD5& rhs) const
		{ return memcmp(md, rhs.md, sizeof(md)) < 0; }
	
private:
	void Block(const void* data);

	struct State;
	State* state;

	unsigned char md[16];
};


string& Base64Encode(string& s);
string& Base64Decode(string& s);

inline CString LoadString(UINT nID)
{
	CString strResult;
	strResult.LoadString(nID);
	return strResult;
}

inline CString FormatString(LPCTSTR pszFormat, ...)
{
	CString strResult;

	va_list argList;
	va_start(argList, pszFormat);
	strResult.FormatV(pszFormat, argList);
	va_end(argList);

	return strResult;
}

inline CString FormatString(UINT nFormatID, ...)
{
	CString strResult, strFormat;
	VERIFY(strFormat.LoadString(nFormatID));

	va_list argList;
	va_start(argList, nFormatID);
	strResult.FormatV(strFormat, argList);
	va_end(argList);

	return strResult;
}

inline int GetTotalRotate(GP<DjVuImage> pImage, int nRotate)
{
	GP<DjVuInfo> info = pImage->get_info();
	return (nRotate + (info != NULL ? info->orientation : 0)) % 4;
}
