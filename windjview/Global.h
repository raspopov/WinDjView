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


class RefCount
{
public:
	RefCount() : m_nRefCount(1) {}
	virtual ~RefCount() = 0;

	void AddRef()
	{
		++m_nRefCount;
	}

	void Release()
	{
		if (--m_nRefCount == 0)
			delete this;
	}

private:
	int m_nRefCount;
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
	
protected:
	void UpdateObservers(const Message& message);
	set<Observer*> m_observers;
};

void MakeWString(const CString& strText, wstring& result);
bool MakeWString(const GUTF8String& text, wstring& result);
CString MakeCString(const wstring& text);
CString MakeCString(const GUTF8String& text);
GUTF8String MakeUTF8String(const CString& strText);
GUTF8String MakeUTF8String(const wstring& strText);

#define PAGE_RENDERED 1
#define PAGE_DECODED 2
#define BOOKMARK_CLICKED 3
#define SEARCH_RESULT_CLICKED 4
#define THUMBNAIL_RENDERED 5
#define THUMBNAIL_CLICKED 6
#define CURRENT_PAGE_CHANGED 7
#define ROTATE_CHANGED 8
#define VIEW_ACTIVATED 9
#define ZOOM_CHANGED 10
#define APP_SETTINGS_CHANGED 11
#define APP_LANGUAGE_CHANGED 12
#define BOOKMARK_DELETED 13
#define ANNOTATION_DELETED 14

struct ThumbnailClicked : public Message
{
	ThumbnailClicked(int nPage_)
		: Message(THUMBNAIL_CLICKED), nPage(nPage_) {}

	int nPage;
};

class CDIB;

struct ThumbnailRendered : public Message
{
	ThumbnailRendered(int nPage_, CDIB* pDIB_)
		: Message(THUMBNAIL_RENDERED), nPage(nPage_), pDIB(pDIB_) {}

	int nPage;
	CDIB* pDIB;
};

struct BookmarkClicked : public Message
{
	BookmarkClicked(const GUTF8String& url_)
		: Message(BOOKMARK_CLICKED), url(url_) {}

	const GUTF8String& url;
};

struct CurrentPageChanged : public Message
{
	CurrentPageChanged(int nPage_)
		: Message(CURRENT_PAGE_CHANGED), nPage(nPage_) {}

	int nPage;
};

struct PageRendered : public Message
{
	PageRendered(int nPage_, CDIB* pDIB_)
		: Message(PAGE_RENDERED), nPage(nPage_), pDIB(pDIB_) {}

	int nPage;
	CDIB* pDIB;
};

struct PageDecoded : public Message
{
	PageDecoded(int nPage_)
		: Message(PAGE_DECODED), nPage(nPage_) {}

	int nPage;
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

struct ViewActivated : public Message
{
	ViewActivated()
		: Message(VIEW_ACTIVATED) {}
};

struct ZoomChanged : public Message
{
	ZoomChanged()
		: Message(ZOOM_CHANGED) {}
};

struct AppSettingsChanged : public Message
{
	AppSettingsChanged()
		: Message(APP_SETTINGS_CHANGED) {}
};

struct AppLanguageChanged : public Message
{
	AppLanguageChanged()
		: Message(APP_LANGUAGE_CHANGED) {}
};

struct Bookmark;
struct Annotation;

struct BookmarkDeleted : public Message
{
	BookmarkDeleted(Bookmark* pBookmark_)
		: Message(BOOKMARK_DELETED), pBookmark(pBookmark_) {}

	Bookmark* pBookmark;
};

struct AnnotationDeleted : public Message
{
	AnnotationDeleted(Annotation* pAnno_, int nPage_)
		: Message(ANNOTATION_DELETED), pAnno(pAnno_), nPage(nPage_) {}

	Annotation* pAnno;
	int nPage;
};

struct MD5
{
	MD5();
	MD5(const void* data, size_t len);
	MD5(const MD5& md5);

	void Update(const void* data, size_t len);
	void Finalize();

	CString ToString() const;
	bool operator==(const MD5& rhs) const
		{ return memcmp(md, rhs.md, sizeof(md)) == 0; }
	bool operator<(const MD5& rhs) const
		{ return memcmp(md, rhs.md, sizeof(md)) < 0; }
	
private:
	void Block(const void* data, size_t num);

	struct Context;
	Context* ctx;

	unsigned char md[16];
};
