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

#include "stdafx.h"
#include "WinDjView.h"
#include "DjVuSource.h"
#include "AppSettings.h"
#include "XMLParser.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// PageInfo

void PageInfo::Update(GP<DjVuImage> pImage, bool bNeedText)
{
	if (pImage == NULL)
		return;

	if (!bDecoded)
	{
		nInitialRotate = GetTotalRotate(pImage, 0);

		szPage = CSize(pImage->get_width(), pImage->get_height());
		if (nInitialRotate % 2 != 0)
			swap(szPage.cx, szPage.cy);

		nDPI = pImage->get_dpi();
		if (szPage.cx <= 0 || szPage.cy <= 0)
		{
			szPage.cx = 100;
			szPage.cy = 100;
			nDPI = 100;
		}

		bHasText = !!(pImage->get_djvu_file()->text != NULL);
		bDecoded = true;
	}

	try
	{
		if (!bTextDecoded && bNeedText)
			DecodeText(pImage->get_djvu_file()->text);
	}
	catch (GException&)
	{
	}

	try
	{
		if (!bAnnoDecoded)
			DecodeAnno(pImage->get_anno());
	}
	catch (GException&)
	{
	}
}

void PageInfo::Update(const PageInfo& info)
{
	if (!bDecoded && info.bDecoded)
	{
		nInitialRotate = info.nInitialRotate;
		szPage = info.szPage;
		nDPI = info.nDPI;
		bHasText = info.bHasText;
		bDecoded = true;
	}

	if (!bTextDecoded && info.bTextDecoded)
	{
		pText = info.pText;
		bTextDecoded = true;
	}

	if (!bAnnoDecoded && info.bAnnoDecoded)
	{
		pAnt = info.pAnt;
		bAnnoDecoded = true;
	}
}

void PageInfo::DecodeAnno(GP<ByteStream> pAnnoStream)
{
	if (pAnnoStream == NULL)
		return;

	pAnnoStream->seek(0);
	GP<DjVuAnno> pDjVuAnno = DjVuAnno::create();
	pDjVuAnno->decode(pAnnoStream);
	pAnt = pDjVuAnno->ant;
	bAnnoDecoded = true;
}

void PageInfo::DecodeText(GP<ByteStream> pTextStream)
{
	if (pTextStream == NULL)
		return;

	pTextStream->seek(0);
	GP<DjVuText> pDjVuText = DjVuText::create();
	pDjVuText->decode(pTextStream);
	pText = pDjVuText->txt;
	bTextDecoded = true;
}


// DjVuHighlight

const TCHAR pszTagHighlight[] = _T("highlight");
const TCHAR pszAttrBorder[] = _T("border");
const TCHAR pszAttrBorderColor[] = _T("border-color");
const TCHAR pszAttrFill[] = _T("fill");
const TCHAR pszAttrFillColor[] = _T("fill-color");
const TCHAR pszAttrFillTransparency[] = _T("fill-transparency");

const TCHAR pszTagRect[] = _T("rect");
const TCHAR pszAttrLeft[] = _T("left");
const TCHAR pszAttrTop[] = _T("top");
const TCHAR pszAttrRight[] = _T("right");
const TCHAR pszAttrBottom[] = _T("bottom");

void DjVuHighlight::UpdateBounds()
{
	if (rects.empty())
		return;

	rectBounds = rects[0];
	for (size_t i = 1; i < rects.size(); ++i)
		rectBounds.recthull(GRect(rectBounds), rects[i]);
}

GUTF8String DjVuHighlight::GetXML() const
{
	CString strRects;
	for (size_t i = 0; i < rects.size(); ++i)
	{
		strRects += FormatString(_T("<%s %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" />\n"),
				pszTagRect, pszAttrLeft, rects[i].xmin, pszAttrTop, rects[i].ymin,
				pszAttrRight, rects[i].xmax, pszAttrBottom, rects[i].ymax);
	}

	CString strBorder;
	strBorder.Format(_T("%s=\"%d\""), pszAttrBorder, nBorderType);
	if (nBorderType == BorderSolid)
		strBorder += FormatString(_T(" %s=\"#%06x\""), pszAttrBorderColor, crBorder);

	CString strFill;
	strFill.Format(_T("%s=\"%d\""), pszAttrFill, nFillType);
	if (nFillType == FillHighlight)
	{
		strFill += FormatString(_T(" %s=\"#%06x\" %s=\"%d%%\""), pszAttrFillColor, crFill,
				pszAttrFillTransparency, static_cast<int>(fTransparency*100 + 0.5));
	}

	CString strResult;
	strResult.Format(_T("<%s %s %s >\n%s</%s>\n"), pszTagHighlight,
			strBorder, strFill, strRects, pszTagHighlight);

	return MakeUTF8String(strResult);
}

void DjVuHighlight::Load(const XMLNode& node)
{
	if (MakeCString(node.tagName) != pszTagHighlight)
		return;

	int borderType = -1;
	if (node.GetIntAttribute(pszAttrBorder, borderType))
	{
		if (borderType >= BorderNone && borderType <= BorderXOR)
			nBorderType = borderType;

		if (borderType == BorderSolid)
		{
			COLORREF color;
			if (node.GetColorAttribute(pszAttrBorderColor, color))
				crBorder = color;
		}
	}

	int fillType = -1;
	if (node.GetIntAttribute(pszAttrFill, fillType))
	{
		if (fillType >= FillNone && fillType <= FillXOR)
			nFillType = fillType;

		if (fillType == FillHighlight)
		{
			COLORREF color;
			if (node.GetColorAttribute(pszAttrFillColor, color))
				crFill = color;

			int nPercent;
			if (node.GetIntAttribute(pszAttrFillTransparency, nPercent))
			{
				if (nPercent >= 0 && nPercent <= 100)
					fTransparency = nPercent / 100.0;
			}
		}
	}

	rects.clear();
	list<XMLNode>::const_iterator it;
	for (it = node.childElements.begin(); it != node.childElements.end(); ++it)
	{
		const XMLNode& child = *it;
		if (MakeCString(child.tagName) == pszTagRect)
		{
			GRect rect;
			if (!child.GetIntAttribute(pszAttrLeft, rect.xmin)
					|| !child.GetIntAttribute(pszAttrTop, rect.ymin)
					|| !child.GetIntAttribute(pszAttrRight, rect.xmax)
					|| !child.GetIntAttribute(pszAttrBottom, rect.ymax))
				continue;

			rects.push_back(rect);
		}
	}

	UpdateBounds();
}


// DjVuPageData

GUTF8String DjVuPageData::GetXML() const
{
	GUTF8String result;

	list<DjVuHighlight>::const_iterator it;
	for (it = highlights.begin(); it != highlights.end(); ++it)
	{
		const DjVuHighlight& highlight = *it;
		result += highlight.GetXML();
	}

	return result;
}

void DjVuPageData::Load(const XMLNode& node)
{
	highlights.clear();
	list<XMLNode>::const_iterator it;
	for (it = node.childElements.begin(); it != node.childElements.end(); ++it)
	{
		const XMLNode& child = *it;
		if (MakeCString(child.tagName) == pszTagHighlight)
		{
			highlights.push_back(DjVuHighlight());
			DjVuHighlight& highlight = highlights.back();
			highlight.Load(child);

			if (highlight.rects.empty())
				highlights.pop_back();
		}
	}
}

// DjVuUserData

const TCHAR pszTagData[] = _T("data");
const TCHAR pszAttrStartPage[] = _T("start-page");
const TCHAR pszAttrOffsetX[] = _T("offset-x");
const TCHAR pszAttrOffsetY[] = _T("offset-y");
const TCHAR pszAttrZoomType[] = _T("zoom-type");
const TCHAR pszAttrZoom[] = _T("zoom");
const TCHAR pszAttrLayout[] = _T("layout");
const TCHAR pszAttrFirstPage[] = _T("first-page");
const TCHAR pszAttrDisplayMode[] = _T("display-mode");

const TCHAR pszTagPage[] = _T("page");
const TCHAR pszAttrNumber[] = _T("number");

DjVuUserData::DjVuUserData()
	: nPage(-1), ptOffset(0, 0), nZoomType(-10), fZoom(100.0), nLayout(-1),
	  bFirstPageAlone(false), nDisplayMode(-1)
{
}

GUTF8String DjVuUserData::GetXML() const
{
	GUTF8String result;

	CString strHead;
	strHead.Format(_T("<%s %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%.2lf%%\" %s=\"%d\" %s=\"%d\" %s=\"%d\" >\n"),
		pszTagData, pszAttrStartPage, nPage, pszAttrOffsetX, ptOffset.x, pszAttrOffsetY, ptOffset.y,
		pszAttrZoomType, nZoomType, pszAttrZoom, fZoom, pszAttrLayout, nLayout,
		pszAttrFirstPage, static_cast<int>(bFirstPageAlone), pszAttrDisplayMode, nDisplayMode);

	result += MakeUTF8String(strHead);

	map<int, DjVuPageData>::const_iterator it;
	for (it = pageData.begin(); it != pageData.end(); ++it)
	{
		int nPage = (*it).first + 1;
		const DjVuPageData& data = (*it).second;
		result += MakeUTF8String(FormatString(_T("<%s %s=\"%d\" >\n"), pszTagPage, pszAttrNumber, nPage));
		result += data.GetXML();
		result += MakeUTF8String(FormatString(_T("</%s>\n"), pszTagPage));
	}

	result += MakeUTF8String(FormatString(_T("</%s>\n"), pszTagData));

	return result;
}

void DjVuUserData::Load(const XMLNode& node)
{
	if (MakeCString(node.tagName) != pszTagData)
		return;

	node.GetIntAttribute(pszAttrStartPage, nPage);
	node.GetLongAttribute(pszAttrOffsetX, ptOffset.x);
	node.GetLongAttribute(pszAttrOffsetY, ptOffset.y);
	node.GetIntAttribute(pszAttrZoomType, nZoomType);
	node.GetDoubleAttribute(pszAttrZoom, fZoom);
	node.GetIntAttribute(pszAttrLayout, nLayout);
	node.GetIntAttribute(pszAttrDisplayMode, nDisplayMode);

	int nFirstPage;
	if (node.GetIntAttribute(pszAttrStartPage, nFirstPage))
		bFirstPageAlone = (nFirstPage != 0);

	pageData.clear();
	list<XMLNode>::const_iterator it;
	for (it = node.childElements.begin(); it != node.childElements.end(); ++it)
	{
		const XMLNode& child = *it;
		if (MakeCString(child.tagName) == pszTagPage)
		{
			int pageNo;
			if (!child.GetIntAttribute(pszAttrNumber, pageNo))
				continue;

			DjVuPageData& data = pageData[pageNo - 1];
			data.Load(child);
		}
	}
}
	
// DjVuSource

map<CString, DjVuSource*> DjVuSource::openDocuments;
map<MD5, DjVuUserData> DjVuSource::userData;

DjVuSource::DjVuSource(const CString& strFileName, GP<DjVuDocument> pDoc, DjVuUserData* pData)
	: m_strFileName(strFileName), m_pDjVuDoc(pDoc), m_nPageCount(0), m_bHasText(false), m_pUserData(pData)
{
	m_nPageCount = m_pDjVuDoc->get_pages_num();
	ASSERT(m_nPageCount > 0);

	m_pages.clear();
	m_pages.resize(m_nPageCount);

	PageInfo info = GetPageInfo(0, false, true);
	if (info.pAnt != NULL)
		m_strPageIndex = info.pAnt->metadata["page-index"];
}

DjVuSource::~DjVuSource()
{
	while (!m_eventCache.empty())
	{
		HANDLE hEvent = m_eventCache.top();
		m_eventCache.pop();

		::CloseHandle(hEvent);
	}

	openDocuments.erase(m_strFileName);
}

DjVuSource* DjVuSource::FromFile(const CString& strFileName)
{
	TCHAR pszName[MAX_PATH] = { 0 };
	LPTSTR pszFileName;
	GetFullPathName(strFileName, MAX_PATH, pszName, &pszFileName);

	DjVuSource* pSource = openDocuments[pszName];
	if (pSource != NULL)
	{
		pSource->AddRef();
		return pSource;
	}

	CFile file;
	if (!file.Open(pszName, CFile::modeRead | CFile::shareDenyWrite))
	{
		return NULL;
	}
	int nLength = min(file.GetLength(), 0x40000);
	LPBYTE pBuf = new BYTE[nLength];
	file.Read(pBuf, nLength);
	file.Close();

	MD5 digest(pBuf, nLength);
	delete[] pBuf;

	GP<DjVuDocument> pDoc = NULL;
	try
	{
		GURL url = GURL::Filename::UTF8(MakeUTF8String(CString(pszName)));
		pDoc = DjVuDocument::create(url);
		pDoc->wait_get_pages_num();
	}
	catch (GException&)
	{
		return NULL;
	}

	if (pDoc->get_pages_num() == 0)
	{
		return NULL;
	}

	map<MD5, DjVuUserData>::iterator it = userData.find(digest);
	bool bExisting = (it != userData.end());
	DjVuUserData* pData = &userData[digest];

	if (!bExisting)
	{
		theApp.LoadDjVuUserData(digest.ToString(), pData);
	}

	return new DjVuSource(pszName, pDoc, pData);
}

bool DjVuSource::IsPageCached(int nPage, Observer* observer)
{
	ASSERT(nPage >= 0 && nPage < m_pDjVuDoc->get_pages_num());
	const PageData& data = m_pages[nPage];

	m_lock.Lock();
	bool bCached = (data.pImage != NULL && data.IsObservedBy(observer));
	m_lock.Unlock();

	return bCached;
}

GP<DjVuImage> DjVuSource::GetPage(int nPage, Observer* observer)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);
	PageData& data = m_pages[nPage];

	m_lock.Lock();
	GP<DjVuImage> pImage = data.pImage;
	if (pImage != NULL)
	{
		m_lock.Unlock();
		return pImage;
	}

	if (data.hDecodingThread != NULL)
	{
		// Other thread is already decoding this page. Put
		// ourselves in a list of waiting threads
		PageRequest request;

		m_eventLock.Lock();
		if (m_eventCache.empty())
		{
			request.hEvent = ::CreateEvent(NULL, false, false, NULL);
		}
		else
		{
			request.hEvent = m_eventCache.top();
			::ResetEvent(request.hEvent);
			m_eventCache.pop();
		}
		m_eventLock.Unlock();

		// Temporarily increase priority of the decoding thread if needed
		HANDLE hOurThread = ::GetCurrentThread();
		int nOurPriority = ::GetThreadPriority(hOurThread);
		if (::GetThreadPriority(data.hDecodingThread) < nOurPriority)
			::SetThreadPriority(data.hDecodingThread, nOurPriority);

		data.requests.push_back(&request);
		
		if (observer != NULL)
			data.AddObserver(observer);

		m_lock.Unlock();

		::WaitForSingleObject(request.hEvent, INFINITE);
		pImage = request.pImage;

		m_eventLock.Lock();
		m_eventCache.push(request.hEvent);
		m_eventLock.Unlock();

		// Page will be put in cache by the thread which decoded it
		return pImage;
	}

	data.hDecodingThread = ::GetCurrentThread();
	data.nOrigThreadPriority = ::GetThreadPriority(data.hDecodingThread);

	m_lock.Unlock();

	try
	{
		GP<DjVuFile> file = m_pDjVuDoc->get_djvu_file(nPage);
		if (file)
		{
			pImage = DjVuImage::create(file);
			file->resume_decode();
			if (pImage && THREADMODEL != NOTHREADS)
				pImage->wait_for_complete_decode();
		}
	}
	catch (GException&)
	{
	}
	catch (...)
	{
		ReportFatalError();
	}

	m_lock.Lock();
	// Notify all waiting threads that image is ready
	for (size_t i = 0; i < data.requests.size(); ++i)
	{
		data.requests[i]->pImage = pImage;
		::SetEvent(data.requests[i]->hEvent);
	}

	ASSERT(data.hDecodingThread == ::GetCurrentThread());
	::SetThreadPriority(data.hDecodingThread, data.nOrigThreadPriority);
	data.hDecodingThread = NULL;
	data.requests.clear();

	if (pImage != NULL)
	{
		pImage->set_rotate(0);

		if (observer != NULL || data.HasObservers())
		{
			data.pImage = pImage;

			if (observer != NULL)
				data.AddObserver(observer);
		}

		data.info.Update(pImage);
		if (data.info.bHasText)
			m_bHasText = true;
	}
	m_lock.Unlock();

	return pImage;
}

PageInfo DjVuSource::GetPageInfo(int nPage, bool bNeedText, bool bNeedAnno)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);
	PageData& data = m_pages[nPage];

	m_lock.Lock();

	if (data.info.bDecoded && (data.info.bTextDecoded || !bNeedText)
			&& (data.info.bAnnoDecoded || !bNeedAnno))
	{
		PageInfo info = data.info;
		m_lock.Unlock();
		return info;
	}

	m_lock.Unlock();

	PageInfo info = ReadPageInfo(nPage, bNeedText, bNeedAnno);

	m_lock.Lock();
	data.info.Update(info);
	if (data.info.bHasText)
		m_bHasText = true;
	m_lock.Unlock();

	return info;
}

void DjVuSource::RemoveFromCache(int nPage, Observer* observer)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);
	PageData& data = m_pages[nPage];
	GP<DjVuImage> pImage = NULL;

	m_lock.Lock();
	data.RemoveObserver(observer);
	if (!data.HasObservers())
	{
		pImage = m_pages[nPage].pImage;
		m_pages[nPage].pImage = NULL;
	}
	m_lock.Unlock();

	// This will cause the destructor to be called, if page is removed from cache
	pImage = NULL;
}

int DjVuSource::GetPageFromId(const GUTF8String& strPageId) const
{
	if (m_pDjVuDoc == NULL)
		return -1;
	
	return m_pDjVuDoc->id_to_page(strPageId);
}

void DjVuSource::ReadAnnotations(GP<ByteStream> pInclStream, set<GUTF8String>& processed, GP<ByteStream> pAnnoStream)
{
	// Look for shared annotations
	GUTF8String strInclude;
	char buf[1024];
	int nLength;
	while ((nLength = pInclStream->read(buf, 1024)))
		strInclude += GUTF8String(buf, nLength);

	// Eat '\n' in the beginning and at the end
	while (strInclude.length() > 0 && strInclude[0] == '\n')
		strInclude = strInclude.substr(1, static_cast<unsigned int>(-1));

	while (strInclude.length() > 0 && strInclude[static_cast<int>(strInclude.length()) - 1] == '\n')
		strInclude.setat(strInclude.length() - 1, 0);

	if (strInclude.length() > 0 && processed.find(strInclude) == processed.end())
	{
		processed.insert(strInclude);

		GURL urlInclude = m_pDjVuDoc->id_to_url(strInclude);
		GP<DataPool> pool = m_pDjVuDoc->request_data(NULL, urlInclude);
		GP<ByteStream> stream = pool->get_stream();
		GP<IFFByteStream> iff(IFFByteStream::create(stream));

		// Check file format
		GUTF8String chkid;
		if (!iff->get_chunk(chkid) ||
			(chkid != "FORM:DJVI" && chkid != "FORM:DJVU" &&
			chkid != "FORM:PM44" && chkid != "FORM:BM44"))
		{
			return;
		}

		// Find chunk with page info
		while (iff->get_chunk(chkid) != 0)
		{
			GP<ByteStream> chunk_stream = iff->get_bytestream();

			if (chkid == "INCL")
			{
				ReadAnnotations(pInclStream, processed, pAnnoStream);
			}
			else if (chkid == "FORM:ANNO")
			{
				pAnnoStream->copy(*chunk_stream);
			}
			else if (chkid == "ANTa" || chkid == "ANTz")
			{
				const GP<IFFByteStream> iffout = IFFByteStream::create(pAnnoStream);
				iffout->put_chunk(chkid);
				iffout->copy(*chunk_stream);
				iffout->close_chunk();
			}

			iff->seek_close_chunk();
		}
	}
}

PageInfo DjVuSource::ReadPageInfo(int nPage, bool bNeedText, bool bNeedAnno)
{
	ASSERT(nPage >= 0 && nPage < m_nPageCount);
	PageInfo pageInfo;
	pageInfo.szPage.cx = 100;
	pageInfo.szPage.cy = 100;
	pageInfo.nDPI = 100;
	pageInfo.bDecoded = true;

	GP<ByteStream> pAnnoStream;
	if (bNeedAnno)
		pAnnoStream = ByteStream::create();

	GP<ByteStream> pTextStream;
	if (bNeedText)
		pTextStream = ByteStream::create();

	try
	{
		// Get raw data from the document and decode only requested chunks
		// DjVuFile is not used to ensure that we do not wait for a lock
		// to be released and thus do not block the UI thread
		GURL url = m_pDjVuDoc->page_to_url(nPage);
		GP<DataPool> pool = m_pDjVuDoc->request_data(NULL, url);
		GP<ByteStream> stream = pool->get_stream();
		GP<IFFByteStream> iff(IFFByteStream::create(stream));

		// Check file format
		GUTF8String chkid;
		if (!iff->get_chunk(chkid) ||
			(chkid != "FORM:DJVI" && chkid != "FORM:DJVU" &&
			 chkid != "FORM:PM44" && chkid != "FORM:BM44"))
		{
			return pageInfo;
		}

		// Find chunk with page info
		while (iff->get_chunk(chkid) != 0)
		{
			GP<ByteStream> chunk_stream = iff->get_bytestream();

			if (chkid == "INFO")
			{
				// Get page dimensions and resolution from info chunk
				GP<DjVuInfo> pInfo = DjVuInfo::create();
				pInfo->decode(*chunk_stream);

				// Check data for consistency
				pageInfo.szPage.cx = max(pInfo->width, 0);
				pageInfo.szPage.cy = max(pInfo->height, 0);
				pageInfo.nInitialRotate = pInfo->orientation;
				pageInfo.nDPI = max(pInfo->dpi, 0);

				if ((pInfo->orientation & 1) != 0)
					swap(pageInfo.szPage.cx, pageInfo.szPage.cy);
			}
			else if (chkid == "PM44" || chkid == "BM44")
			{
				// Get image dimensions and resolution from bitmap chunk
				UINT serial = chunk_stream->read8();
				UINT slices = chunk_stream->read8();
				UINT major = chunk_stream->read8();
				UINT minor = chunk_stream->read8();

				UINT xhi = chunk_stream->read8();
				UINT xlo = chunk_stream->read8();
				UINT yhi = chunk_stream->read8();
				UINT ylo = chunk_stream->read8();

				pageInfo.szPage.cx = (xhi << 8) | xlo;
				pageInfo.szPage.cy = (yhi << 8) | ylo;
				pageInfo.nDPI = 100;
			}
			else if (chkid == "TXTa" || chkid == "TXTz")
			{
				pageInfo.bHasText = true;

				if (bNeedText)
				{
					const GP<IFFByteStream> iffout = IFFByteStream::create(pTextStream);
					iffout->put_chunk(chkid);
					iffout->copy(*chunk_stream);
					iffout->close_chunk();
				}
			}
			else if (bNeedAnno && chkid == "FORM:ANNO")
			{
				pAnnoStream->copy(*chunk_stream);
			}
			else if (bNeedAnno && (chkid == "ANTa" || chkid == "ANTz"))
			{
				const GP<IFFByteStream> iffout = IFFByteStream::create(pAnnoStream);
				iffout->put_chunk(chkid);
				iffout->copy(*chunk_stream);
				iffout->close_chunk();
			}
			else if (bNeedAnno && chkid == "INCL")
			{
				set<GUTF8String> processed;
				ReadAnnotations(chunk_stream, processed, pAnnoStream);
			}

			iff->seek_close_chunk();
		}

		if (bNeedText && pTextStream->tell())
			pageInfo.DecodeText(pTextStream);

		if (bNeedAnno && pAnnoStream->tell())
			pageInfo.DecodeAnno(pAnnoStream);
	}
	catch (GException&)
	{
	}
	catch (...)
	{
		ReportFatalError();
	}

	return pageInfo;
}

bool DjVuSource::SaveAs(const CString& strFileName)
{
	try
	{
		m_pDjVuDoc->wait_for_complete_init();
		GURL url = GURL::Filename::UTF8(MakeUTF8String(strFileName));
		m_pDjVuDoc->write(ByteStream::create(url, "wb"), true);
		return true;
	}
	catch (GException&)
	{
		return false;
	}
}
