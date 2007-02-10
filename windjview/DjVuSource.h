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

struct PageInfo
{
	PageInfo()
		: bDecoded(false), szPage(0, 0), nDPI(0), nInitialRotate(0),
		  bHasText(false), bAnnoDecoded(false), bTextDecoded(false) {}

	void Update(GP<DjVuImage> pImage, bool bNeedText = false);
	void Update(const PageInfo& info);

	void DecodeAnno(GP<ByteStream> pAnnoStream);
	void DecodeText(GP<ByteStream> pTextStream);

	bool bDecoded;
	CSize szPage;
	int nInitialRotate;
	int nDPI;
	bool bHasText;
	bool bAnnoDecoded;
	GP<DjVuANT> pAnt;
	bool bTextDecoded;
	GP<DjVuTXT> pText;
};


typedef GList<DjVuTXT::Zone*> DjVuSelection;

struct DjVuHighlight
{
	DjVuHighlight()
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
};

struct DjVuPageData
{
	list<DjVuHighlight> highlights;

	GUTF8String GetXML() const;
	void Load(const XMLNode& node);
};

struct DjVuUserData
{
	DjVuUserData();

	int nPage;
	CPoint ptOffset;
	int nZoomType;
	double fZoom;
	int nLayout;
	bool bFirstPageAlone;
	int nDisplayMode;

	map<int, DjVuPageData> pageData;

	GUTF8String GetXML() const;
	void Load(const XMLNode& node);
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

	DjVuUserData* GetUserData() { return m_pUserData; }

	static map<MD5, DjVuUserData>& GetAllUserData() { return userData; }

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

	DjVuSource(const CString& strFileName, GP<DjVuDocument> pDoc, DjVuUserData* pData);
	PageInfo ReadPageInfo(int nPage, bool bNeedText = false, bool bNeedAnno = false);
	void ReadAnnotations(GP<ByteStream> pInclStream, set<GUTF8String>& processed, GP<ByteStream> pAnnoStream);

	DjVuUserData* m_pUserData;
	GP<DjVuDocument> m_pDjVuDoc;
	CString m_strFileName;
	vector<PageData> m_pages;
	int m_nPageCount;
	CCriticalSection m_lock;
	bool m_bHasText;
	GUTF8String m_strPageIndex;

	static map<CString, DjVuSource*> openDocuments;
	static map<MD5, DjVuUserData> userData;

private:
	DjVuSource() {}
};
