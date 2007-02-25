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
//
//-----------------------------------------------------------------------------
//	MD5 digest implementation is based on libmd5-rfc
//  http://sourceforge.net/projects/libmd5-rfc/
//	Begin copyright notice for libmd5-rfc
//	***************************************************************************
//	Copyright (C) 1999, 2000, 2002 Aladdin Enterprises.  All rights reserved.
//
//	This software is provided 'as-is', without any express or implied
//	warranty.  In no event will the authors be held liable for any damages
//	arising from the use of this software.
//
//	Permission is granted to anyone to use this software for any purpose,
//	including commercial applications, and to alter it and redistribute it
//	freely, subject to the following restrictions:
//
//	1. The origin of this software must not be misrepresented; you must not
//	   claim that you wrote the original software. If you use this software
//	   in a product, an acknowledgment in the product documentation would be
//	   appreciated but is not required.
//	2. Altered source versions must be plainly marked as such, and must not be
//	   misrepresented as being the original software.
//	3. This notice may not be removed or altered from any source distribution.
//
//	L. Peter Deutsch
//	ghost@aladdin.com
//	***************************************************************************
//	End copyright notice for libmd5-rfc
//
//-----------------------------------------------------------------------------
//	Base64 encoder/decoder implementation is based on modp_b64 library
//	Begin copyright notice for modp_b64
//	***************************************************************************
//	MODP_B64 - High performance base64 encoder/decoder
//	Version 1.3 -- 17-Mar-2006
//	http://modp.com/release/base64
//
//	Copyright &copy; 2005, 2006  Nick Galbreath -- nickg [at] modp [dot] com
//	All rights reserved.
//
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided that the following conditions are met:
//
//	  Redistributions of source code must retain the above copyright
//	  notice, this list of conditions and the following disclaimer.
//
//	  Redistributions in binary form must reproduce the above copyright
//	  notice, this list of conditions and the following disclaimer in the
//	  documentation and/or other materials provided with the distribution.
//
//	  Neither the name of the modp.com nor the names of its
//	  contributors may be used to endorse or promote products derived from
//	  this software without specific prior written permission.
//
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//	OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//	This is the standard "new" BSD license:
//	http://www.opensource.org/licenses/bsd-license.php
//	***************************************************************************
//	End copyright notice for modp_b64

// $Id$

#include "stdafx.h"
#include "Global.h"


// RefCount

RefCount::~RefCount()
{
}


// Observable

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

bool IsWin2kOrLater()
{
	static int nResult = -1;

	if (nResult == -1)
	{
		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		nResult = (::GetVersionEx(&vi) != 0 && vi.dwPlatformId == VER_PLATFORM_WIN32_NT
				&& vi.dwMajorVersion >= 5);
	}

	return !!nResult;
}

bool IsWinXPOrLater()
{
	static int nResult = -1;

	if (nResult == -1)
	{
		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		nResult = (::GetVersionEx(&vi) != 0 && vi.dwPlatformId == VER_PLATFORM_WIN32_NT
				&& (vi.dwMajorVersion > 5 || vi.dwMajorVersion == 5 && vi.dwMinorVersion >= 1));
	}

	return !!nResult;
}

bool IsWinNT()
{
	static int nResult = -1;

	if (nResult == -1)
	{
		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		nResult = (::GetVersionEx(&vi) != 0 && vi.dwPlatformId == VER_PLATFORM_WIN32_NT);
	}

	return !!nResult;
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
	if (IsWinXPOrLater())
		dwFlags = MB_ERR_INVALID_CHARS;

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
	if (IsWinXPOrLater())
		dwFlags = MB_ERR_INVALID_CHARS;

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

struct MD5::State
{
	DWORD abcd[4];
	DWORD buf[64];
	DWORD count[2];

	State()
	{
		abcd[0] = 0x67452301UL;
		abcd[1] = 0xefcdab89UL;
		abcd[2] = 0x98badcfeUL;
		abcd[3] = 0x10325476UL;
		count[0] = 0;
		count[1] = 0;
	}
};

static const unsigned char pad[64] = { 0x80 };

#define	F(b, c, d) ((((c) ^ (d)) & (b)) ^ (d))
#define	G(b, c, d) ((((b) ^ (c)) & (d)) ^ (c))
#define	H(b, c, d) ((b) ^ (c) ^ (d))
#define	I(b, c, d) (((~(d)) | (b)) ^ (c))

#define ROTATE(a, n) _lrotl(a, n)

#define R0(a, b, c, d, k, s, t) \
	{\
		a += ((k) + (t) + F((b), (c), (d)));\
		a = ROTATE(a, s) + b;\
	};

#define R1(a, b, c, d, k, s, t) \
	{\
		a += ((k) + (t) + G((b), (c), (d)));\
		a = ROTATE(a, s) + b;\
	};

#define R2(a, b, c, d, k, s, t) \
	{\
		a += ((k) + (t) + H((b), (c), (d)));\
		a = ROTATE(a, s) + b;\
	};

#define R3(a, b, c, d, k, s, t) \
	{\
		a += ((k) + (t) + I((b), (c), (d)));\
		a = ROTATE(a, s) + b;\
	};


MD5::MD5(const MD5& md5)
	: state(NULL)
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
	state = new State();
}

MD5::MD5(const void* data, size_t len)
{
	state = new State();
	Append(data, len);
	Finish();
}

void MD5::Block(const void* data, size_t num)
{
	const DWORD* x = (const DWORD*) data;
	DWORD a, b, c, d;

	a = state->abcd[0];
	b = state->abcd[1];
	c = state->abcd[2];
	d = state->abcd[3];

	for (; num--; x += 16)
	{
		R0(a, b, c, d, x[ 0],  7, 0xd76aa478L);
		R0(d, a, b, c, x[ 1], 12, 0xe8c7b756L);
		R0(c, d, a, b, x[ 2], 17, 0x242070dbL);
		R0(b, c, d, a, x[ 3], 22, 0xc1bdceeeL);
		R0(a, b, c, d, x[ 4],  7, 0xf57c0fafL);
		R0(d, a, b, c, x[ 5], 12, 0x4787c62aL);
		R0(c, d, a, b, x[ 6], 17, 0xa8304613L);
		R0(b, c, d, a, x[ 7], 22, 0xfd469501L);
		R0(a, b, c, d, x[ 8],  7, 0x698098d8L);
		R0(d, a, b, c, x[ 9], 12, 0x8b44f7afL);
		R0(c, d, a, b, x[10], 17, 0xffff5bb1L);
		R0(b, c, d, a, x[11], 22, 0x895cd7beL);
		R0(a, b, c, d, x[12],  7, 0x6b901122L);
		R0(d, a, b, c, x[13], 12, 0xfd987193L);
		R0(c, d, a, b, x[14], 17, 0xa679438eL);
		R0(b, c, d, a, x[15], 22, 0x49b40821L);
		R1(a, b, c, d, x[ 1],  5, 0xf61e2562L);
		R1(d, a, b, c, x[ 6],  9, 0xc040b340L);
		R1(c, d, a, b, x[11], 14, 0x265e5a51L);
		R1(b, c, d, a, x[ 0], 20, 0xe9b6c7aaL);
		R1(a, b, c, d, x[ 5],  5, 0xd62f105dL);
		R1(d, a, b, c, x[10],  9, 0x02441453L);
		R1(c, d, a, b, x[15], 14, 0xd8a1e681L);
		R1(b, c, d, a, x[ 4], 20, 0xe7d3fbc8L);
		R1(a, b, c, d, x[ 9],  5, 0x21e1cde6L);
		R1(d, a, b, c, x[14],  9, 0xc33707d6L);
		R1(c, d, a, b, x[ 3], 14, 0xf4d50d87L);
		R1(b, c, d, a, x[ 8], 20, 0x455a14edL);
		R1(a, b, c, d, x[13],  5, 0xa9e3e905L);
		R1(d, a, b, c, x[ 2],  9, 0xfcefa3f8L);
		R1(c, d, a, b, x[ 7], 14, 0x676f02d9L);
		R1(b, c, d, a, x[12], 20, 0x8d2a4c8aL);
		R2(a, b, c, d, x[ 5],  4, 0xfffa3942L);
		R2(d, a, b, c, x[ 8], 11, 0x8771f681L);
		R2(c, d, a, b, x[11], 16, 0x6d9d6122L);
		R2(b, c, d, a, x[14], 23, 0xfde5380cL);
		R2(a, b, c, d, x[ 1],  4, 0xa4beea44L);
		R2(d, a, b, c, x[ 4], 11, 0x4bdecfa9L);
		R2(c, d, a, b, x[ 7], 16, 0xf6bb4b60L);
		R2(b, c, d, a, x[10], 23, 0xbebfbc70L);
		R2(a, b, c, d, x[13],  4, 0x289b7ec6L);
		R2(d, a, b, c, x[ 0], 11, 0xeaa127faL);
		R2(c, d, a, b, x[ 3], 16, 0xd4ef3085L);
		R2(b, c, d, a, x[ 6], 23, 0x04881d05L);
		R2(a, b, c, d, x[ 9],  4, 0xd9d4d039L);
		R2(d, a, b, c, x[12], 11, 0xe6db99e5L);
		R2(c, d, a, b, x[15], 16, 0x1fa27cf8L);
		R2(b, c, d, a, x[ 2], 23, 0xc4ac5665L);
		R3(a, b, c, d, x[ 0],  6, 0xf4292244L);
		R3(d, a, b, c, x[ 7], 10, 0x432aff97L);
		R3(c, d, a, b, x[14], 15, 0xab9423a7L);
		R3(b, c, d, a, x[ 5], 21, 0xfc93a039L);
		R3(a, b, c, d, x[12],  6, 0x655b59c3L);
		R3(d, a, b, c, x[ 3], 10, 0x8f0ccc92L);
		R3(c, d, a, b, x[10], 15, 0xffeff47dL);
		R3(b, c, d, a, x[ 1], 21, 0x85845dd1L);
		R3(a, b, c, d, x[ 8],  6, 0x6fa87e4fL);
		R3(d, a, b, c, x[15], 10, 0xfe2ce6e0L);
		R3(c, d, a, b, x[ 6], 15, 0xa3014314L);
		R3(b, c, d, a, x[13], 21, 0x4e0811a1L);
		R3(a, b, c, d, x[ 4],  6, 0xf7537e82L);
		R3(d, a, b, c, x[11], 10, 0xbd3af235L);
		R3(c, d, a, b, x[ 2], 15, 0x2ad7d2bbL);
		R3(b, c, d, a, x[ 9], 21, 0xeb86d391L);

		a = (state->abcd[0] += a);
		b = (state->abcd[1] += b);
		c = (state->abcd[2] += c);
		d = (state->abcd[3] += d);
	}
}

void MD5::Append(const void *data, size_t len)
{
	ASSERT(state != NULL);
    if (len == 0)
		return;

	const unsigned char* p = (unsigned char*) data;
	size_t left = len;
	size_t offset = (state->count[0] >> 3) & 63;
    DWORD nbits = len << 3;

    // Update the message length
    state->count[1] += len >> 29;
    state->count[0] += nbits;
    if (state->count[0] < nbits)
		state->count[1]++;

    // Process an initial partial block
    if (offset)
    {
		size_t ncopy = (offset + len > 64 ? 64 - offset : len);

		memcpy(state->buf + offset, p, ncopy);
		if (offset + ncopy < 64)
		    return;

		p += ncopy;
		left -= ncopy;
		Block(state->buf, 1);
    }

    // Process full blocks
    Block(p, left / 64);
    left = (left & 63);

    // Process a final partial block
    if (left)
		memcpy(state->buf, p, left);
}

void MD5::Finish()
{
	ASSERT(state != NULL);

    unsigned char data[8];
    int i;

    // Save the length before padding
    for (i = 0; i < 8; ++i)
		data[i] = (state->count[i >> 2] >> ((i & 3) << 3));

    // Pad to 56 bytes mod 64.
    Append(pad, ((55 - (state->count[0] >> 3)) & 63) + 1);

    // Append the length.
    Append(data, 8);

    for (i = 0; i < 16; ++i)
		md[i] = (state->abcd[i >> 2] >> ((i & 3) << 3));

	delete state;
	state = NULL;
}


// Base64 encoder/decoder

#define CHARPAD '='

static const char e0[] =
	"AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHIIIIJJJJKKKKLLLLMMMMNNNNOOOOPPPP"
	"QQQQRRRRSSSSTTTTUUUUVVVVWWWWXXXXYYYYZZZZaaaabbbbccccddddeeeeffff"
	"gggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvv"
	"wwwwxxxxyyyyzzzz0000111122223333444455556666777788889999++++////";

static const char e1[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const UINT d0[] =
{
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x000000f8, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x000000fc,
	0x000000d0, 0x000000d4, 0x000000d8, 0x000000dc, 0x000000e0, 0x000000e4,
	0x000000e8, 0x000000ec, 0x000000f0, 0x000000f4, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x00000000,
	0x00000004, 0x00000008, 0x0000000c, 0x00000010, 0x00000014, 0x00000018,
	0x0000001c, 0x00000020, 0x00000024, 0x00000028, 0x0000002c, 0x00000030,
	0x00000034, 0x00000038, 0x0000003c, 0x00000040, 0x00000044, 0x00000048,
	0x0000004c, 0x00000050, 0x00000054, 0x00000058, 0x0000005c, 0x00000060,
	0x00000064, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x00000068, 0x0000006c, 0x00000070, 0x00000074, 0x00000078,
	0x0000007c, 0x00000080, 0x00000084, 0x00000088, 0x0000008c, 0x00000090,
	0x00000094, 0x00000098, 0x0000009c, 0x000000a0, 0x000000a4, 0x000000a8,
	0x000000ac, 0x000000b0, 0x000000b4, 0x000000b8, 0x000000bc, 0x000000c0,
	0x000000c4, 0x000000c8, 0x000000cc, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff
};

static const UINT d1[256] =
{
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x0000e003, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x0000f003,
	0x00004003, 0x00005003, 0x00006003, 0x00007003, 0x00008003, 0x00009003,
	0x0000a003, 0x0000b003, 0x0000c003, 0x0000d003, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x00000000,
	0x00001000, 0x00002000, 0x00003000, 0x00004000, 0x00005000, 0x00006000,
	0x00007000, 0x00008000, 0x00009000, 0x0000a000, 0x0000b000, 0x0000c000,
	0x0000d000, 0x0000e000, 0x0000f000, 0x00000001, 0x00001001, 0x00002001,
	0x00003001, 0x00004001, 0x00005001, 0x00006001, 0x00007001, 0x00008001,
	0x00009001, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x0000a001, 0x0000b001, 0x0000c001, 0x0000d001, 0x0000e001,
	0x0000f001, 0x00000002, 0x00001002, 0x00002002, 0x00003002, 0x00004002,
	0x00005002, 0x00006002, 0x00007002, 0x00008002, 0x00009002, 0x0000a002,
	0x0000b002, 0x0000c002, 0x0000d002, 0x0000e002, 0x0000f002, 0x00000003,
	0x00001003, 0x00002003, 0x00003003, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff
};

static const UINT d2[256] =
{
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x00800f00, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x00c00f00,
	0x00000d00, 0x00400d00, 0x00800d00, 0x00c00d00, 0x00000e00, 0x00400e00,
	0x00800e00, 0x00c00e00, 0x00000f00, 0x00400f00, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x00000000,
	0x00400000, 0x00800000, 0x00c00000, 0x00000100, 0x00400100, 0x00800100,
	0x00c00100, 0x00000200, 0x00400200, 0x00800200, 0x00c00200, 0x00000300,
	0x00400300, 0x00800300, 0x00c00300, 0x00000400, 0x00400400, 0x00800400,
	0x00c00400, 0x00000500, 0x00400500, 0x00800500, 0x00c00500, 0x00000600,
	0x00400600, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x00800600, 0x00c00600, 0x00000700, 0x00400700, 0x00800700,
	0x00c00700, 0x00000800, 0x00400800, 0x00800800, 0x00c00800, 0x00000900,
	0x00400900, 0x00800900, 0x00c00900, 0x00000a00, 0x00400a00, 0x00800a00,
	0x00c00a00, 0x00000b00, 0x00400b00, 0x00800b00, 0x00c00b00, 0x00000c00,
	0x00400c00, 0x00800c00, 0x00c00c00, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff
};

static const UINT d3[256] =
{
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x003e0000, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x003f0000,
	0x00340000, 0x00350000, 0x00360000, 0x00370000, 0x00380000, 0x00390000,
	0x003a0000, 0x003b0000, 0x003c0000, 0x003d0000, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x00000000,
	0x00010000, 0x00020000, 0x00030000, 0x00040000, 0x00050000, 0x00060000,
	0x00070000, 0x00080000, 0x00090000, 0x000a0000, 0x000b0000, 0x000c0000,
	0x000d0000, 0x000e0000, 0x000f0000, 0x00100000, 0x00110000, 0x00120000,
	0x00130000, 0x00140000, 0x00150000, 0x00160000, 0x00170000, 0x00180000,
	0x00190000, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x001a0000, 0x001b0000, 0x001c0000, 0x001d0000, 0x001e0000,
	0x001f0000, 0x00200000, 0x00210000, 0x00220000, 0x00230000, 0x00240000,
	0x00250000, 0x00260000, 0x00270000, 0x00280000, 0x00290000, 0x002a0000,
	0x002b0000, 0x002c0000, 0x002d0000, 0x002e0000, 0x002f0000, 0x00300000,
	0x00310000, 0x00320000, 0x00330000, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff,
	0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff
};

#define BADCHAR 0x01FFFFFF

#define DOPAD 1
#ifndef DOPAD
#undef CHARPAD
#define CHARPAD '\0'
#endif

int modp_b64_encode(char* dest, const char* str, int len)
{
    int i;
    BYTE* p = (BYTE*) dest;

    /* unsigned here is important! */
    BYTE t1, t2, t3;

    for (i = 0; i < len - 2; i += 3) {
        t1 = str[i]; t2 = str[i+1]; t3 = str[i+2];
        *p++ = e0[t1];
        *p++ = e1[((t1 & 0x03) << 4) | ((t2 >> 4) & 0x0F)];
        *p++ = e1[((t2 & 0x0F) << 2) | ((t3 >> 6) & 0x03)];
        *p++ = e1[t3];
    }

    switch (len - i) {
    case 0:
        break;
    case 1:
        t1 = str[i];
        *p++ = e0[t1];
        *p++ = e1[(t1 & 0x03) << 4];
        *p++ = CHARPAD;
        *p++ = CHARPAD;
        break;
    default: // case 2
        t1 = str[i];
        t2 = str[i+1];
        *p++ = e0[t1];
        *p++ = e1[((t1 & 0x03) << 4) | ((t2 >> 4) & 0x0F)];
        *p++ = e1[(t2 & 0x0F) << 2];
        *p++ = CHARPAD;
    }

    *p = '\0';
    return p - (BYTE*)dest;
}

int modp_b64_decode(char* dest, const char* src, int len)
{
    if (len == 0) return 0;

#ifdef DOPAD
    // if padding is used, then the message must be at least
    // 4 chars and be a multiple of 4
    if (len < 4 || (len % 4 != 0)) return -1; // error
    // there can be at most 2 pad chars at the end
    if (src[len-1] == CHARPAD) {
        len--;
        if (src[len -1] == CHARPAD) {
            len--;
        }
    }
#endif

    int i;
    int leftover = len % 4;
    int chunks = (leftover == 0) ? len / 4 - 1 : len /4;

    BYTE* p = (BYTE*) dest;
    UINT x = 0;
    UINT* destInt = (UINT*) p;
    UINT* srcInt = (UINT*) src;
    UINT y = *srcInt++;
    for (i = 0; i < chunks; ++i) {
        x = d0[y & 0xff] |
            d1[(y >> 8) & 0xff] |
            d2[(y >> 16) & 0xff] |
            d3[(y >> 24) & 0xff];

        if (x >= BADCHAR) return -1;
        *destInt = x ;
        p += 3;
        destInt = (UINT*)p;
        y = *srcInt++;}


    switch (leftover) {
    case 0:
        x = d0[y & 0xff] |
            d1[(y >> 8) & 0xff] |
            d2[(y >> 16) & 0xff] |
            d3[(y >> 24) & 0xff];

        if (x >= BADCHAR) return -1;
        *p++ =  ((BYTE*)(&x))[0];
        *p++ =  ((BYTE*)(&x))[1];
        *p =    ((BYTE*)(&x))[2];
        return (chunks+1)*3;
        break;
    case 1:  // with padding this is an impossible case
        x = d0[y & 0xff];
        *p = *((BYTE*)(&x)); // i.e. first char/byte in int
        break;
    case 2: // case 2, 1  output byte
        x = d0[y & 0xff] | d1[y >> 8 & 0xff];
        *p = *((BYTE*)(&x)); // i.e. first char
        break;
    default: // case 3, 2 output bytes
        x = d0[y & 0xff] |
            d1[y >> 8 & 0xff ] |
            d2[y >> 16 & 0xff];  // 0x3c
        *p++ =  ((BYTE*)(&x))[0];
        *p =  ((BYTE*)(&x))[1];
        break;
    }

    if (x >= BADCHAR) return -1;

    return 3*chunks + (6*leftover)/8;
}

#define modp_b64_encode_len(A) (((A) + 2)/3*4 + 1)
#define modp_b64_decode_len(A) ((A)/4*3 + 2)

string& Base64Encode(string& s)
{
    string x(modp_b64_encode_len(s.size()), '\0');
    int d = modp_b64_encode(const_cast<char*>(x.data()), s.data(), s.size());
    x.erase(d, std::string::npos);
    s.swap(x);
    return s;
}

string& Base64Decode(string& s)
{
    string x(modp_b64_decode_len(s.size()), '\0');
    int d = modp_b64_decode(const_cast<char*>(x.data()), s.data(), s.size());
    if (d < 0) {
        x.erase();
    } else {
        x.erase(d, string::npos);
    }
    s.swap(x);
    return s;
}