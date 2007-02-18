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
#include "Global.h"


RefCount::~RefCount()
{
}

void Observable::AddObserver(Observer* observer)
{
	m_observers.insert(observer);
}

void Observable::RemoveObserver(Observer* observer)
{
	m_observers.erase(observer);
}

void Observable::UpdateObservers(const Message& message)
{
	for (std::set<Observer*>::iterator it = m_observers.begin(); it != m_observers.end(); ++it)
	{
		Observer* observer = *it;
		observer->OnUpdate(this, &message);
	}
}

GUTF8String MakeUTF8String(const CString& strText)
{
	int nSize;

#ifdef _UNICODE
	LPCWSTR pszUnicodeText = strText;
#else
	nSize = ::MultiByteToWideChar(CP_ACP, 0, (LPCSTR)strText, -1, NULL, 0);
	LPWSTR pszUnicodeText = new WCHAR[nSize];
	::MultiByteToWideChar(CP_ACP, 0, (LPCSTR)strText, -1, pszUnicodeText, nSize);
#endif

	nSize = ::WideCharToMultiByte(CP_UTF8, 0, pszUnicodeText, -1, NULL, 0, NULL, NULL);
	LPSTR pszTextUTF8 = new CHAR[nSize];
	::WideCharToMultiByte(CP_UTF8, 0, pszUnicodeText, -1, pszTextUTF8, nSize, NULL, NULL);

	GUTF8String utf8String(pszTextUTF8);
	delete[] pszTextUTF8;

#ifndef _UNICODE
	delete[] pszUnicodeText;
#endif

	return utf8String;
}

int ReadUTF8Character(const char* s, int& nBytes)
{
	unsigned char c = static_cast<unsigned char>(s[0]);

	if (c < 0x80)
	{
		nBytes = 1;
		return c;
	}
	else if (c < 0xc2)
	{
		return -1;
	}
	else if (c < 0xe0)
	{
		if (s[1] == 0 || !((s[1] ^ 0x80) < 0x40))
			return -1;
		nBytes = 2;
		return ((s[0] & 0x1F) << 6) + (s[1] & 0x7F);
	}
	else if (c < 0xf0)
	{
		if (s[1] == 0 || s[2] == 0 || !((s[1] ^ 0x80) < 0x40
				&& (s[2] ^ 0x80) < 0x40 && (c >= 0xe1 || s[1] >= 0xa0)))
			return -1;
		nBytes = 3;
		return ((s[0] & 0xF) << 12) + ((s[1] & 0x7F) << 6) + (s[2] & 0x7F);
	}
	else
	{
		return -1;
	}
}

bool IsValidUTF8(const char* pszText)
{
	const char* s = pszText;
	while (*s != 0)
	{
		int nBytes = 0;
		if (ReadUTF8Character(s, nBytes) < 0)
			return false;
		s += nBytes;
	}

	return true;
}

GUTF8String MakeUTF8String(const wstring& strText)
{
	LPCWSTR pszUnicodeText = (LPCWSTR)strText.c_str();

	int nSize = ::WideCharToMultiByte(CP_UTF8, 0, pszUnicodeText, -1, NULL, 0, NULL, NULL);
	LPSTR pszTextUTF8 = new CHAR[nSize];
	::WideCharToMultiByte(CP_UTF8, 0, pszUnicodeText, -1, pszTextUTF8, nSize, NULL, NULL);

	GUTF8String utf8String(pszTextUTF8);
	delete[] pszTextUTF8;

	return utf8String;
}

CString MakeCString(const GUTF8String& text)
{
	CString strResult;
	if (text.length() == 0)
		return strResult;

	// Prepare Unicode text
	LPWSTR pszUnicodeText = NULL;
	int nResult = 0;

	// Bookmarks or annotations may not be encoded in UTF-8
	// (when file was created by old software)
	// Treat input string as non-UTF8 if it is not well-formed
	DWORD dwFlags = 0;

	// Only Windows XP supports checking for invalid characters in UTF8 encoding
	// inside MultiByteToWideChar function
	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (::GetVersionEx(&vi) && vi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
		(vi.dwMajorVersion > 5 || vi.dwMajorVersion == 5 && vi.dwMinorVersion >= 1))
	{
		dwFlags = MB_ERR_INVALID_CHARS;
	}

	// Make our own check anyway
	if (!IsValidUTF8(text))
	{
		strResult = (LPCSTR)text;
		return strResult;
	}

	int nSize = ::MultiByteToWideChar(CP_UTF8, dwFlags, (LPCSTR)text, -1, NULL, 0);
	if (nSize != 0)
	{
		pszUnicodeText = new WCHAR[nSize];
		nResult = ::MultiByteToWideChar(CP_UTF8, dwFlags, (LPCSTR)text, -1, pszUnicodeText, nSize);
	}

	if (nResult != 0)
	{
#ifdef _UNICODE
		strResult = pszUnicodeText;
#else
		// Prepare ANSI text
		nSize = ::WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DISCARDNS,
			pszUnicodeText, -1, NULL, 0, NULL, NULL);
		::WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DISCARDNS,
			pszUnicodeText, -1, strResult.GetBuffer(nSize), nSize, NULL, NULL);
		strResult.ReleaseBuffer();
#endif
	}
	else
	{
		strResult = (LPCSTR)text;
	}

	delete[] pszUnicodeText;

	return strResult;
}

CString MakeCString(const wstring& text)
{
	CString strResult;
	LPCWSTR pszUnicodeText = (LPCWSTR)text.c_str();

#ifdef _UNICODE
	strResult = pszUnicodeText;
#else
	// Prepare ANSI text
	int nSize = ::WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DISCARDNS,
		pszUnicodeText, -1, NULL, 0, NULL, NULL);
	::WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DISCARDNS,
		pszUnicodeText, -1, strResult.GetBuffer(nSize), nSize, NULL, NULL);
	strResult.ReleaseBuffer();
#endif

	return strResult;
}

void MakeWString(const CString& strText, wstring& result)
{
	LPCTSTR pszText = (LPCTSTR)strText;

#ifdef _UNICODE
	result = (const wchar_t*)pszText;
#else
	int nSize = ::MultiByteToWideChar(CP_ACP, 0, pszText, -1, NULL, 0);
	result.resize(nSize - 1);
	if (nSize > 1)
		::MultiByteToWideChar(CP_ACP, 0, pszText, -1, (LPWSTR)result.data(), nSize);
#endif
}

bool MakeWString(const GUTF8String& text, wstring& result)
{
	if (text.length() == 0)
	{
		result.resize(0);
		return true;
	}

	// Prepare Unicode text
	LPWSTR pszUnicodeText = NULL;
	int nResult = 0;

	DWORD dwFlags = 0;

	// Only Windows XP supports checking for invalid characters in UTF8 encoding
	// inside MultiByteToWideChar function
	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (::GetVersionEx(&vi) && vi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
		(vi.dwMajorVersion > 5 || vi.dwMajorVersion == 5 && vi.dwMinorVersion >= 1))
	{
		dwFlags = MB_ERR_INVALID_CHARS;
	}

	// Make our own check anyway
	int nSize;
	if (IsValidUTF8(text) && (nSize = ::MultiByteToWideChar(CP_UTF8, dwFlags,
			(LPCSTR)text, -1, NULL, 0)) > 1)
	{
		result.resize(nSize - 1);
		nResult = ::MultiByteToWideChar(CP_UTF8, dwFlags, (LPCSTR)text, -1,
			(LPWSTR)result.data(), nSize);
		return (nResult != 0);
	}
	else
	{
		result.resize(text.length());
		for (int i = 0; i < static_cast<int>(text.length()); ++i)
			result[i] = static_cast<unsigned char>(text[i]);
		return true;
	}
}


// MD5 digest implementation

#define MD5_CBLOCK 64
#define MD5_LBLOCK (MD5_CBLOCK / 4)

struct MD5::Context
{
	DWORD A, B, C, D;
	DWORD Nl, Nh;
	DWORD data[MD5_LBLOCK];
	size_t num;

	Context()
	{
		A = 0x67452301UL;
		B = 0xefcdab89UL;
		C = 0x98badcfeUL;
		D = 0x10325476UL;
		Nl = 0;
		Nh = 0;
		num = 0;
	}
};

#define	F(b, c, d) ((((c) ^ (d)) & (b)) ^ (d))
#define	G(b, c, d) ((((b) ^ (c)) & (d)) ^ (c))
#define	H(b, c, d) ((b) ^ (c) ^ (d))
#define	I(b, c, d) (((~(d)) | (b)) ^ (c))

#define ROTATE(a, n) _lrotl(a, n)

#define R0(a, b, c, d, k, s, t) \
	{\
		a += ((k) + (t) + F((b), (c), (d)));\
		a = ROTATE(a, s);\
		a += b;\
	};

#define R1(a, b, c, d, k, s, t) \
	{\
		a += ((k) + (t) + G((b), (c), (d)));\
		a = ROTATE(a, s);\
		a += b;\
	};

#define R2(a, b, c, d, k, s, t) \
	{\
		a += ((k) + (t) + H((b), (c), (d)));\
		a = ROTATE(a, s);\
		a += b;\
	};

#define R3(a, b, c, d, k, s, t) \
	{\
		a += ((k) + (t) + I((b), (c), (d)));\
		a = ROTATE(a, s);\
		a += b;\
	};

#define HOST_c2l(c, l) ((l) = *((const DWORD*)(c)), (c) += 4, l)

#define HOST_p_c2l(c, l, n) \
	{\
		switch (n)\
		{\
			case 0: l  = ((DWORD)(*((c)++)));\
			case 1: l |= ((DWORD)(*((c)++))) << 8;\
			case 2: l |= ((DWORD)(*((c)++))) << 16;\
			case 3: l |= ((DWORD)(*((c)++))) << 24;\
		}\
	}

#define HOST_p_c2l_p(c, l, sc, len) \
	{\
		switch (sc)\
		{\
			case 0: l  = ((DWORD)(*((c)++)));\
				if (--len == 0) break;\
			case 1: l |= ((DWORD)(*((c)++))) << 8;\
				if (--len == 0) break;\
			case 2: l |= ((DWORD)(*((c)++))) << 16;\
		}\
	}

#define HOST_c2l_p(c, l, n) \
	{\
		l = 0;\
		(c) += n;\
		switch (n)\
		{\
			case 3: l  = ((DWORD)(*(--(c)))) << 16;\
			case 2: l |= ((DWORD)(*(--(c)))) << 8;\
			case 1: l |= ((DWORD)(*(--(c))));\
		}\
	}

MD5::MD5(const MD5& md5)
	: ctx(NULL)
{
	memmove(md, md5.md, sizeof(md));
}

CString MD5::ToString() const
{
	CString result;
	LPTSTR psz = result.GetBuffer(2*sizeof(md));
	for (size_t i = 0; i < sizeof(md); ++i)
	{
		unsigned char lo = md[i] & 0x0F;
		unsigned char hi = md[i] >> 4;
		psz[2*i] = (hi <= 9 ? '0' + hi : 'A' + hi - 10);
		psz[2*i + 1] = (lo <= 9 ? '0' + lo : 'A' + lo - 10);
	}
	result.ReleaseBuffer();
	return result;
}

MD5::MD5()
{
	ctx = new Context();
}

MD5::MD5(const void* data, size_t len)
{
	ctx = new Context();
	Update(data, len);
	Finalize();
}

void MD5::Block(const void* data, size_t num)
{
	const DWORD* X = (const DWORD*) data;
	register DWORD A, B, C, D;

	A = ctx->A;
	B = ctx->B;
	C = ctx->C;
	D = ctx->D;

	for (; num--; X += MD5_LBLOCK)
	{
		R0(A, B, C, D, X[ 0],  7, 0xd76aa478L);
		R0(D, A, B, C, X[ 1], 12, 0xe8c7b756L);
		R0(C, D, A, B, X[ 2], 17, 0x242070dbL);
		R0(B, C, D, A, X[ 3], 22, 0xc1bdceeeL);
		R0(A, B, C, D, X[ 4],  7, 0xf57c0fafL);
		R0(D, A, B, C, X[ 5], 12, 0x4787c62aL);
		R0(C, D, A, B, X[ 6], 17, 0xa8304613L);
		R0(B, C, D, A, X[ 7], 22, 0xfd469501L);
		R0(A, B, C, D, X[ 8],  7, 0x698098d8L);
		R0(D, A, B, C, X[ 9], 12, 0x8b44f7afL);
		R0(C, D, A, B, X[10], 17, 0xffff5bb1L);
		R0(B, C, D, A, X[11], 22, 0x895cd7beL);
		R0(A, B, C, D, X[12],  7, 0x6b901122L);
		R0(D, A, B, C, X[13], 12, 0xfd987193L);
		R0(C, D, A, B, X[14], 17, 0xa679438eL);
		R0(B, C, D, A, X[15], 22, 0x49b40821L);
		R1(A, B, C, D, X[ 1],  5, 0xf61e2562L);
		R1(D, A, B, C, X[ 6],  9, 0xc040b340L);
		R1(C, D, A, B, X[11], 14, 0x265e5a51L);
		R1(B, C, D, A, X[ 0], 20, 0xe9b6c7aaL);
		R1(A, B, C, D, X[ 5],  5, 0xd62f105dL);
		R1(D, A, B, C, X[10],  9, 0x02441453L);
		R1(C, D, A, B, X[15], 14, 0xd8a1e681L);
		R1(B, C, D, A, X[ 4], 20, 0xe7d3fbc8L);
		R1(A, B, C, D, X[ 9],  5, 0x21e1cde6L);
		R1(D, A, B, C, X[14],  9, 0xc33707d6L);
		R1(C, D, A, B, X[ 3], 14, 0xf4d50d87L);
		R1(B, C, D, A, X[ 8], 20, 0x455a14edL);
		R1(A, B, C, D, X[13],  5, 0xa9e3e905L);
		R1(D, A, B, C, X[ 2],  9, 0xfcefa3f8L);
		R1(C, D, A, B, X[ 7], 14, 0x676f02d9L);
		R1(B, C, D, A, X[12], 20, 0x8d2a4c8aL);
		R2(A, B, C, D, X[ 5],  4, 0xfffa3942L);
		R2(D, A, B, C, X[ 8], 11, 0x8771f681L);
		R2(C, D, A, B, X[11], 16, 0x6d9d6122L);
		R2(B, C, D, A, X[14], 23, 0xfde5380cL);
		R2(A, B, C, D, X[ 1],  4, 0xa4beea44L);
		R2(D, A, B, C, X[ 4], 11, 0x4bdecfa9L);
		R2(C, D, A, B, X[ 7], 16, 0xf6bb4b60L);
		R2(B, C, D, A, X[10], 23, 0xbebfbc70L);
		R2(A, B, C, D, X[13],  4, 0x289b7ec6L);
		R2(D, A, B, C, X[ 0], 11, 0xeaa127faL);
		R2(C, D, A, B, X[ 3], 16, 0xd4ef3085L);
		R2(B, C, D, A, X[ 6], 23, 0x04881d05L);
		R2(A, B, C, D, X[ 9],  4, 0xd9d4d039L);
		R2(D, A, B, C, X[12], 11, 0xe6db99e5L);
		R2(C, D, A, B, X[15], 16, 0x1fa27cf8L);
		R2(B, C, D, A, X[ 2], 23, 0xc4ac5665L);
		R3(A, B, C, D, X[ 0],  6, 0xf4292244L);
		R3(D, A, B, C, X[ 7], 10, 0x432aff97L);
		R3(C, D, A, B, X[14], 15, 0xab9423a7L);
		R3(B, C, D, A, X[ 5], 21, 0xfc93a039L);
		R3(A, B, C, D, X[12],  6, 0x655b59c3L);
		R3(D, A, B, C, X[ 3], 10, 0x8f0ccc92L);
		R3(C, D, A, B, X[10], 15, 0xffeff47dL);
		R3(B, C, D, A, X[ 1], 21, 0x85845dd1L);
		R3(A, B, C, D, X[ 8],  6, 0x6fa87e4fL);
		R3(D, A, B, C, X[15], 10, 0xfe2ce6e0L);
		R3(C, D, A, B, X[ 6], 15, 0xa3014314L);
		R3(B, C, D, A, X[13], 21, 0x4e0811a1L);
		R3(A, B, C, D, X[ 4],  6, 0xf7537e82L);
		R3(D, A, B, C, X[11], 10, 0xbd3af235L);
		R3(C, D, A, B, X[ 2], 15, 0x2ad7d2bbL);
		R3(B, C, D, A, X[ 9], 21, 0xeb86d391L);

		A = ctx->A += A;
		B = ctx->B += B;
		C = ctx->C += C;
		D = ctx->D += D;
	}
}

void MD5::Update(const void *data_, size_t len)
{
	ASSERT(ctx != NULL);

	const unsigned char* data = (unsigned char*) data_;
	register DWORD* p;
	register DWORD l;
	size_t sw, sc, ew, ec;

	if (len == 0)
		return;

	l = (ctx->Nl + (((DWORD) len) << 3)) & 0xffffffffUL;
	if (l < ctx->Nl)
		ctx->Nh++;
	ctx->Nh += (len >> 29);
	ctx->Nl = l;

	if (ctx->num != 0)
	{
		p = ctx->data;
		sw = ctx->num >> 2;
		sc = ctx->num & 0x03;

		if ((ctx->num + len) >= MD5_CBLOCK)
		{
			l = p[sw];
			HOST_p_c2l(data, l, sc);
			p[sw++] = l;
			for (; sw < MD5_LBLOCK; sw++)
			{
				HOST_c2l(data, l);
				p[sw] = l;
			}
			Block(p, 1);
			len -= (MD5_CBLOCK - ctx->num);
			ctx->num = 0;
		}
		else
		{
			ctx->num += len;
			if ((sc + len) < 4)
			{
				l = p[sw];
				HOST_p_c2l_p(data, l, sc, len);
				p[sw] = l;
			}
			else
			{
				ew = (ctx->num >> 2);
				ec = (ctx->num & 0x03);
				if (sc)
					l = p[sw];
				HOST_p_c2l(data, l, sc);
				p[sw++] = l;
				for (; sw < ew; sw++)
				{
					HOST_c2l(data, l);
					p[sw] = l;
				}
				if (ec)
				{
					HOST_c2l_p(data, l, ec);
					p[sw] = l;
				}
			}
			return;
		}
	}

	sw = len / MD5_CBLOCK;
	if (sw > 0)
	{
		if ((((size_t) data) % 4) == 0)
		{
			Block((const DWORD*) data, sw);
			sw *= MD5_CBLOCK;
			data += sw;
			len -= sw;
		}
		else
			while (sw--)
			{
				memcpy(p = ctx->data, data, MD5_CBLOCK);
				Block(p, 1);
				data += MD5_CBLOCK;
				len -= MD5_CBLOCK;
			}
	}

	if (len != 0)
	{
		p = ctx->data;
		ctx->num = len;
		ew = len >> 2;
		ec = len & 0x03;
		for (; ew; ew--, p++)
		{
			HOST_c2l(data, l);
			*p = l;
		}
		HOST_c2l_p(data, l, ec);
		*p = l;
	}

	return;
}

void MD5::Finalize()
{
	ASSERT(ctx != NULL);

	register DWORD* p;
	register DWORD l;
	register int i, j;
	static const unsigned char end[4] = {0x80, 0x00, 0x00, 0x00};
	const unsigned char* cp = end;

	p = ctx->data;
	i = ctx->num >> 2;
	j = ctx->num & 0x03;
	l = (j == 0) ? 0 : p[i];
	HOST_p_c2l(cp, l, j);
	p[i++] = l;

	if (i > (MD5_LBLOCK - 2))
	{
		if (i < MD5_LBLOCK)
			p[i] = 0;
		Block(p, 1);
		i = 0;
	}
	for (; i < (MD5_LBLOCK - 2); i++)
		p[i] = 0;

	p[MD5_LBLOCK - 2] = ctx->Nl;
	p[MD5_LBLOCK - 1] = ctx->Nh;
	Block(p, 1);

	*(DWORD*)md = ctx->A;
	*(DWORD*)(md + 4) = ctx->B;
	*(DWORD*)(md + 8) = ctx->C;
	*(DWORD*)(md + 12) = ctx->D;

	delete ctx;
	ctx = NULL;
}
