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


class CDjVuSource;
struct XMLNode;

typedef GList<DjVuTXT::Zone*> DjVuSelection;

struct Annotation
{
	Annotation()
		: bHideInactiveBorder(false), nBorderType(BorderNone), crBorder(RGB(0, 0, 0)),
		  nFillType(FillNone), crFill(RGB(255, 255, 255)), fTransparency(0.75) {}

	void UpdateBounds();
	GUTF8String GetXML() const;
	void Load(const XMLNode& node);

	enum BorderType
	{
		BorderNone = 0,
		BorderSolid = 1,
		BorderXOR = 2
	};

	enum FillType
	{
		FillNone = 0,
		FillSolid = 1,
		FillXOR = 2
	};

	bool bHideInactiveBorder;
	int nBorderType;
	COLORREF crBorder;
	int nFillType;
	COLORREF crFill;
	double fTransparency;
	GUTF8String strComment;
	GUTF8String strURL;

	vector<GRect> rects;
	GRect rectBounds;

	void Init(GP<GMapArea> pArea, const CSize& szPage, int nRotate);
	GP<GMapArea> sourceArea;
};

struct PageSettings
{
	list<Annotation> anno;

	GUTF8String GetXML() const;
	void Load(const XMLNode& node);
};

struct Bookmark
{
private:
	// Trick to pacify the VC6 compiler
	list<Bookmark>* pchildren;

public:
	Bookmark()
		: pchildren(new list<Bookmark>()), children(*pchildren), pParent(NULL),
		  nLinkType(URL), nPage(0), ptOffset(0, 0), bMargin(false) {}
	Bookmark(const Bookmark& bm)
		: pchildren(new list<Bookmark>(bm.children)), children(*pchildren),
		  strURL(bm.strURL), strTitle(bm.strTitle), pParent(bm.pParent),
		  nLinkType(bm.nLinkType), nPage(bm.nPage), ptOffset(bm.ptOffset),
		  bMargin(bm.bMargin) {}
	~Bookmark()
		{ delete pchildren; }

	GUTF8String strTitle;
	Bookmark* pParent;

	enum LinkType
	{
		URL = 0,
		Page = 1,
		View = 2
	};

	bool HasLink() const
		{ return (nLinkType == URL ? strURL.length() > 0 : true); }

	int nLinkType;
	GUTF8String strURL;
	int nPage;
	CPoint ptOffset;
	bool bMargin;

	list<Bookmark>& children;

	GUTF8String GetXML() const;
	void Load(const XMLNode& node);
};

struct DocSettings : public Observable
{
	DocSettings();

	int nPage;
	CPoint ptOffset;
	int nZoomType;
	double fZoom;
	int nLayout;
	bool bFirstPageAlone;
	int nDisplayMode;
	int nRotate;

	map<int, PageSettings> pageSettings;
	list<Bookmark> bookmarks;

	GUTF8String GetXML() const;
	void Load(const XMLNode& node);

	Annotation* AddAnnotation(const Annotation& anno, int nPage);
	bool DeleteBookmark(const Bookmark* pBookmark);
	bool DeleteAnnotation(const Annotation* pAnno, int nPage);
};

struct PageInfo
{
	PageInfo()
		: bDecoded(false), szPage(0, 0), nDPI(0), nInitialRotate(0),
		  bHasText(false), bAnnoDecoded(false), bTextDecoded(false) {}

	void Update(GP<DjVuImage> pImage);
	void Update(const PageInfo& info);

	void DecodeAnno(GP<ByteStream> pAnnoStream);
	void DecodeText(GP<ByteStream> pTextStream);

	bool bDecoded;
	CSize szPage;
	int nInitialRotate;
	int nDPI;
	bool bHasText;
	bool bAnnoDecoded;
	list<Annotation> anno;
	GP<DjVuANT> pAnt;
	bool bTextDecoded;
	GP<DjVuTXT> pText;
};

class DjVuSource : public RefCount
{
public:
	~DjVuSource();

	// the caller must call Release() for the object returned
	static DjVuSource* FromFile(const CString& strFileName);

	GP<DjVuImage> GetPage(int nPage, Observer* observer);
	void RemoveFromCache(int nPage, Observer* observer);

	PageInfo GetPageInfo(int nPage, bool bNeedText = false, bool bNeedAnno = false);
	bool IsPageCached(int nPage, Observer* observer);
	int GetPageCount() const { return m_nPageCount; }

	bool HasText() const { return m_bHasText; }
	int GetPageFromId(const GUTF8String& strPageId) const;

	GP<DjVmNav> GetBookmarks() { return m_pDjVuDoc->get_djvm_nav(); }
	const GUTF8String& GetPageIndex() { return m_strPageIndex; }

	CString GetFileName() const { return m_strFileName; }
	bool SaveAs(const CString& strFileName);

	DocSettings* GetSettings() { return m_pSettings; }

	static map<MD5, DocSettings>& GetAllSettings() { return settings; }

protected:
	struct PageRequest
	{
		HANDLE hEvent;
		GP<DjVuImage> pImage;
	};
	stack<HANDLE> m_eventCache;
	CCriticalSection m_eventLock;

	struct PageData : public Observable
	{
		PageData() : hDecodingThread(NULL) {}

		GP<DjVuImage> pImage;
		PageInfo info;

		HANDLE hDecodingThread;
		int nOrigThreadPriority;
		vector<PageRequest*> requests;
	};

	DjVuSource(const CString& strFileName, GP<DjVuDocument> pDoc, DocSettings* pSettings);
	PageInfo ReadPageInfo(int nPage, bool bNeedText = false, bool bNeedAnno = false);
	void ReadAnnotations(GP<ByteStream> pInclStream, set<GUTF8String>& processed, GP<ByteStream> pAnnoStream);

	GP<DjVuDocument> m_pDjVuDoc;
	CString m_strFileName;
	int m_nPageCount;
	CCriticalSection m_lock;
	bool m_bHasText;
	GUTF8String m_strPageIndex;

	vector<PageData> m_pages;
	DocSettings* m_pSettings;

	static map<CString, DjVuSource*> openDocuments;
	static map<MD5, DocSettings> settings;

private:
	DjVuSource() {}
};
