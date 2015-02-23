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

#include "stdafx.h"
#include "XMLParser.h"
#include "Global.h"
#include <istream>
#include <streambuf>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


inline bool IsNameStart(int c)
{
	return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_' || c == ':';
}

inline bool IsNameChar(int c)
{
	return IsNameStart(c) || c >= '0' && c <= '9' || c == '-' || c == '.' || c == 0xb7;
}

inline bool IsValidChar(int c)
{
	return c == 0x9 || c == 0xA || c == 0xD || c >= 0x20 && c <= 0xD7FF
			|| c >= 0xE000 && c <= 0xFFFD || c >= 0x10000 && c <= 0x10FFFF;
}

inline void AppendChar(wstring& s, int c)
{
	if (s.length() == s.capacity())
		s.reserve(s.length()*2 + 2); // Make room for at least two more characters
	if (c <= 0xFFFF)
	{
		s.insert(s.end(), static_cast<wchar_t>(c));
	}
	else
	{
		int c1 = 0xD800 + ((c - 0x10000) >> 10);
		int c2 = 0xDC00 + ((c - 0x10000) & 0x3FF);
		s.insert(s.end(), static_cast<wchar_t>(c1));
		s.insert(s.end(), static_cast<wchar_t>(c2));
	}
}

XMLNode& XMLNode::operator=(const XMLNode& node)
{
	if (&node != this)
	{
		childElements = node.childElements;
		tagName = node.tagName;
		text = node.text;
		attributes = node.attributes;
	}

	return *this;
}

GUTF8String XMLNode::ToString() const
{
	if (nType == TEXT)
		return MakeUTF8String(text);

	vector<char> result;

	GUTF8String str = "<" + MakeUTF8String(tagName);
	result.insert(result.end(), (const char*) str, (const char*) str + str.length());

	map<wstring, wstring>::const_iterator it;
	for (it = attributes.begin(); it != attributes.end(); ++it)
	{
		str = " " + MakeUTF8String((*it).first) + "=\"" + MakeUTF8String((*it).second).toEscaped() + "\"";
		result.insert(result.end(), (const char*) str, (const char*) str + str.length());
	}

	if (!childElements.empty())
	{
		str = ">";
		result.insert(result.end(), (const char*) str, (const char*) str + str.length());

		list<XMLNode>::iterator it;
		for (it = childElements.begin(); it != childElements.end(); ++it)
		{
			str = (*it).ToString();
			result.insert(result.end(), (const char*) str, (const char*) str + str.length());
		}

		str = "</" + MakeUTF8String(tagName) + ">";
		result.insert(result.end(), (const char*) str, (const char*) str + str.length());
	}
	else
	{
		str = "/>";
		result.insert(result.end(), (const char*) str, (const char*) str + str.length());
	}

	result.push_back('\0');
	return GUTF8String(&result[0]);
}

const wstring* XMLNode::GetAttribute(const CString& name) const
{
	wstring attr;
	MakeWString(CString(name), attr);

	map<wstring, wstring>::const_iterator it = attributes.find(attr);
	if (it == attributes.end())
		return NULL;

	return &((*it).second);
}

bool XMLNode::GetAttribute(const CString& name, wstring& value) const
{
	const wstring* attr = GetAttribute(name);
	if (attr == NULL)
		return false;

	value = attr->c_str();
	return true;
}

bool XMLNode::GetIntAttribute(const CString& name, int& value) const
{
	const wstring* attr = GetAttribute(name);
	if (attr == NULL)
		return false;

	int val;
	if (swscanf(attr->c_str(), L"%d", &val) != 1)
		return false;

	value = val;
	return true;
}

bool XMLNode::GetLongAttribute(const CString& name, long& value) const
{
	const wstring* attr = GetAttribute(name);
	if (attr == NULL)
		return false;

	long val;
	if (swscanf(attr->c_str(), L"%d", &val) != 1)
		return false;

	value = val;
	return true;
}

bool XMLNode::GetHexAttribute(const CString& name, DWORD& value) const
{
	const wstring* attr = GetAttribute(name);
	if (attr == NULL)
		return false;

	DWORD val;
	if (swscanf(attr->c_str(), L"%x", &val) != 1)
		return false;

	value = val;
	return true;
}

bool XMLNode::GetDoubleAttribute(const CString& name, double& value) const
{
	const wstring* attr = GetAttribute(name);
	if (attr == NULL)
		return false;

	double val;
	if (swscanf(attr->c_str(), L"%lf", &val) != 1)
		return false;

	value = val;
	return true;
}

bool XMLNode::GetColorAttribute(const CString& name, COLORREF& value) const
{
	const wstring* attr = GetAttribute(name);
	if (attr == NULL)
		return false;

	COLORREF val;
	if (swscanf(attr->c_str(), L"#%x", &val) != 1)
		return false;

	value = val;
	return true;
}


// XMLParser

const int errIllegalCharacter = 1;
const int errNameExpected = 2;
const int errAttrValueExpected = 3;
const int errEqualsExpected = 4;
const int errTagExpected = 5;
const int errUnexpectedEOF = 6;
const int errMissingClosingTag = 7;
const int errInvalidTrailer = 8;
const int errInvalidReference = 9;
const int errInvalidCharRef = 10;
const int errInvalidEntityRef = 11;
const int errInvalidClosingTag = 12;
const int errInvalidComment = 13;
const int errInvalidTag = 14;
const int errInvalidPI = 15;
const int errInvalidAttrValue = 16;

bool XMLParser::Parse(istream& in_)
{
	in = &in_;
	m_root.tagName = L"";
	m_root.text = L"";
	m_root.attributes.clear();
	m_root.childElements.clear();

	ch = in->rdbuf()->sbumpc();
	try
	{
		nextChar();
		skipWhitespace();

		while (skipPI() || skipComment()) skipWhitespace();

		readTag(m_root);

		skipWhitespace();
		while (cur != EOF)
		{
			if (!skipPI() && !skipComment())
				throw errInvalidTrailer;
			skipWhitespace();
		}

		m_bValid = true;
	}
	catch (int e)
	{
		UNUSED(e);
		TRACE("Invalid XML: %d\n", e);
		m_bValid = false;
	}

	in = NULL;
	return m_bValid;
}

int XMLParser::nextChar()
{
	if (!pushed.empty())
	{
		cur = pushed.back();
		pushed.pop_back();
		return cur;
	}

	cur = ch;

	if (cur == EOF)
		return EOF;

	if (cur == 0xA || cur == 0x9 || cur >= 0x20 && cur < 0x80)
	{
		ch = in->rdbuf()->sbumpc();
		return cur;
	}

	if (cur == 0xD)
	{
		ch = in->rdbuf()->sbumpc();
		if (ch == 0xA)
			ch = in->rdbuf()->sbumpc();
		return 0xA;
	}
	
	if (cur < 0xC2)
		throw errIllegalCharacter;

	if (cur < 0xE0)
	{
		if ((ch = in->rdbuf()->sbumpc()) == EOF)
			throw errIllegalCharacter;
		if ((ch ^ 0x80) >= 0x40)
			throw errIllegalCharacter;

		cur = ((cur & 0x1F) << 6) + (ch ^ 0x80);
		ch = in->rdbuf()->sbumpc();
		return cur;
	}

	if (cur < 0xF0)
	{
		int next = in->rdbuf()->sbumpc();
		if (next == EOF || (ch = in->rdbuf()->sbumpc()) == EOF)
			throw errIllegalCharacter;
		if ((next ^ 0x80) >= 0x40 || (ch ^ 0x80) >= 0x40
				|| (cur == 0xE0 && next < 0xA0))
			throw errIllegalCharacter;
		cur = ((cur & 0xF) << 12) + ((next ^ 0x80) << 6) + (ch ^ 0x80);
		ch = in->rdbuf()->sbumpc();
		return cur;
	}

	if (cur < 0xF5)
	{
		int next = in->rdbuf()->sbumpc(), next2;
		if (next == EOF || (next2 = in->rdbuf()->sbumpc()) == EOF
				|| (ch = in->rdbuf()->sbumpc()) == EOF)
			throw errIllegalCharacter;
		if ((next ^ 0x80) >= 0x40 || (next2 ^ 0x80) >= 0x40 || (ch ^ 0x80) >= 0x40
				|| (cur == 0xF0 && next < 0x90) || (cur == 0xF4 && next > 0x8F))
			throw errIllegalCharacter;
		cur = ((cur & 0x07) << 18) + ((next ^ 0x80) << 12) + ((next2 ^ 0x80) << 6)
				+ (ch ^ 0x80);
		ch = in->rdbuf()->sbumpc();
		return cur;
	}

	throw errIllegalCharacter;
}

bool XMLParser::skipWhitespace()
{
	bool skipped = false;
	while (cur == 0x9 || cur == 0xA || cur == 0x20)
	{
		skipped = true;
		nextChar();
	}
	return skipped;
}

void XMLParser::readName(wstring& name)
{
	if (!IsNameStart(cur))
		throw errNameExpected;

	name.reserve(32);
	AppendChar(name, cur);
	while (IsNameChar(nextChar()))
		AppendChar(name, cur);
}

void XMLParser::readAttrValue(wstring& text)
{
	if (cur != '\"')
		throw errAttrValueExpected;

	nextChar();
	while (cur != '\"')
	{
		if (cur == EOF)
			throw errUnexpectedEOF;

		if (cur == '<')
			throw errInvalidAttrValue;

		if (cur == '&')
		{
			AppendChar(text, readReference());
		}
		else
		{
			AppendChar(text, cur);
			nextChar();
		}
	}

	nextChar();
}

int XMLParser::readReference()
{
	if (cur != '&')
		throw errInvalidReference;

	if (nextChar() == '#')
	{
		if (nextChar() == 'x')
		{
			nextChar();

			int c = 0;
			while (c <= 0x10FFFF && (cur >= '0' && cur <= '9' || cur >= 'a' && cur <= 'f' || cur >= 'A' && cur <= 'F'))
			{
				if (cur >= '0' && cur <= '9')
					c = c*0x10 + (cur - '0');
				else if (cur >= 'a' && cur <= 'f')
					c = c*0x10 + (cur - 'a' + 0x10);
				else
					c = c*0x10 + (cur - 'A' + 0x10);

				nextChar();
			}

			if (cur != ';' || !IsValidChar(c))
				throw errInvalidCharRef;

			nextChar();
			return c;
		}
		else
		{
			int c = 0;
			while (c <= 0x10FFFF && cur >= '0' && cur <= '9')
			{
				c = c*10 + (cur - '0');
				nextChar();
			}

			if (cur != ';' || !IsValidChar(c))
				throw errInvalidCharRef;

			nextChar();
			return c;
		}
	}
	else
	{
		char buf[5] = { static_cast<char>(cur) };
		int length = 1;

		while (nextChar() != ';' && length < 4)
			buf[length++] = static_cast<char>(cur);

		if (cur != ';')
			throw errInvalidEntityRef;
		nextChar();

		if (strncmp(buf, "lt", length) == 0)
			return '<';
		else if (strncmp(buf, "gt", length) == 0)
			return '>';
		else if (strncmp(buf, "amp", length) == 0)
			return '&';
		else if (strncmp(buf, "quot", length) == 0)
			return '\"';
		else if (strncmp(buf, "apos", length) == 0)
			return '\'';
		else
			throw errInvalidEntityRef;
	}
}

void XMLParser::pushBack(int c)
{
	pushed.push_back(cur);
	cur = c;
}

bool XMLParser::skipString(const char* s)
{
	for (const char* p = s; *p != '\0'; ++p)
	{
		if (cur != *p)
		{
			for (const char* r = p - 1; r >= s; --r)
				pushBack(*r);
			return false;
		}

		nextChar();
	}

	return true;
}

bool XMLParser::skipPI()
{
	if (!skipString("<?"))
		return false;

	for (;;)
	{
		if (cur == EOF)
		{
			throw errUnexpectedEOF;
		}
		else if (cur == '?')
		{
			if (nextChar() == '>')
			{
				nextChar();
				return true;
			}
		}
		else
		{
			nextChar();
		}
	}
}

bool XMLParser::skipComment()
{
	if (!skipString("<!--"))
		return false;

	int buf[2];
	int count = 0;

	while (cur != EOF)
	{
		if (count > 1 && buf[0] == '-' && buf[1] == '-')
		{
			if (cur == '>')
			{
				nextChar();
				return true;
			}
			else
				throw errInvalidComment;
		}

		buf[count++ % 2] = cur;
		nextChar();
	}

	throw errUnexpectedEOF;
}

void XMLParser::readTag(XMLNode& node)
{
	if (cur != '<')
		throw errTagExpected;

	nextChar();
	readName(node.tagName);
	bool whitespace = skipWhitespace();

	while (cur != EOF && cur != '/' && cur != '>')
	{
		if (!whitespace)
			throw errInvalidTag;

		wstring attr;
		readName(attr);
		skipWhitespace();
		if (cur != '=')
			throw errEqualsExpected;
		nextChar();
		skipWhitespace();
		readAttrValue(node.attributes[attr]);

		whitespace = skipWhitespace();
	}

	if (cur == '>')
	{
		nextChar();
		readContents(node);
		skipWhitespace();

		if (cur != '<')
			throw errMissingClosingTag;
		if (nextChar() != '/')
			throw errMissingClosingTag;
		nextChar();

		wstring nclose;
		readName(nclose);
		skipWhitespace();

		if (cur != '>')
			throw errInvalidClosingTag;
		if (nclose != node.tagName)
			throw errInvalidClosingTag;

		nextChar();
	}
	else if (cur == '/')
	{
		if (nextChar() != '>')
			throw errInvalidTag;

		nextChar();
	}
	else
	{
		throw errInvalidTag;
	}
}

void XMLParser::readContents(XMLNode& node)
{
	while (cur != EOF)
	{
		if (skipComment())
			continue;
		if (skipPI())
			continue;

		if (cur == '<')
		{
			if (nextChar() == '/')
			{
				pushBack('<');
				return;
			}

			pushBack('<');
			node.childElements.push_back(XMLNode(XMLNode::TAG));
			XMLNode& childNode = node.childElements.back();
			readTag(childNode);
			continue;
		}
		else 
		{
			int ch;
			if (cur != '&')
			{
				ch = cur;
				nextChar();
			}
			else
				ch = readReference();

			AppendChar(node.text, ch);

			if (node.childElements.empty() || node.childElements.back().nType != XMLNode::TEXT)
				node.childElements.push_back(XMLNode(XMLNode::TEXT));

			AppendChar(node.childElements.back().text, ch);
		}
	}

	throw errUnexpectedEOF;
}
