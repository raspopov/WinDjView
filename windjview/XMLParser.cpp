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
const int errInvalidCharEntity = 9;
const int errInvalidClosingTag = 10;
const int errInvalidComment = 11;
const int errInvalidTag = 12;
const int errInvalidPI = 13;
const int errInvalidAttrValue = 14;

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
	catch (int)
	{
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
	
	if (cur < 0x20)
		throw errIllegalCharacter;

	if (cur < 0xe0)
	{
		if ((ch = in->rdbuf()->sbumpc()) == EOF)
			throw errIllegalCharacter;
		if (!((ch ^ 0x80) < 0x40))
			throw errIllegalCharacter;

		cur = ((cur & 0x1F) << 6) + (ch & 0x7F);
		ch = in->rdbuf()->sbumpc();
		return cur;
	}

	if (cur < 0xf0)
	{
		int next = in->rdbuf()->sbumpc();
		if (next == EOF || (ch = in->rdbuf()->sbumpc()) == EOF)
			throw errIllegalCharacter;
		if (!((next ^ 0x80) < 0x40 && (ch ^ 0x80) < 0x40
				&& (cur >= 0xe1 || next >= 0xa0)))
			throw errIllegalCharacter;
		cur = ((cur & 0xF) << 12) + ((next & 0x7F) << 6) + (ch & 0x7F);
		ch = in->rdbuf()->sbumpc();
		return cur;
	}

	throw errIllegalCharacter;
}

void XMLParser::skipWhitespace()
{
	while (cur == 0x9 || cur == 0xA || cur == 0x20)
		nextChar();
}

void XMLParser::readName(wstring& name)
{
	if (!IsNameStart(cur))
		throw errNameExpected;

	name.reserve(10);
	name.insert(name.end(), static_cast<wchar_t>(cur));
	while (IsNameChar(nextChar()))
		name.insert(name.end(), static_cast<wchar_t>(cur));
}

void XMLParser::readText(wstring& text)
{
	if (cur != '\'' && cur != '\"')
		throw errAttrValueExpected;

	int quote = cur;
	nextChar();
	while (cur != quote)
	{
		if (cur == EOF)
			throw errUnexpectedEOF;

		if (cur == '<')
			throw errInvalidAttrValue;

		if (cur == '&')
		{
			text.insert(text.end(), static_cast<wchar_t>(readCharEntity()));
		}
		else
		{
			text.insert(text.end(), static_cast<wchar_t>(cur));
			nextChar();
		}
	}

	nextChar();
}

int XMLParser::readCharEntity()
{
	if (cur != '&')
		throw errInvalidCharEntity;

	if (nextChar() == '#')
	{
		if (nextChar() == 'x')
		{
			nextChar();

			int c = 0;
			while (c <= 0xFFFF && (cur >= '0' && cur <= '9' || cur >= 'a' && cur <= 'f' || cur >= 'A' && cur <= 'F'))
			{
				if (cur >= '0' && cur <= '9')
					c = c*0x10 + (cur - '0');
				else if (cur >= 'a' && cur <= 'f')
					c = c*0x10 + (cur - 'a' + 0x10);
				else
					c = c*0x10 + (cur - 'A' + 0x10);

				nextChar();
			}

			if (cur != ';' || c > 0xFFFF || c == 0)
				throw errInvalidCharEntity;

			nextChar();
			return c;
		}
		else
		{
			int c = 0;
			while (c <= 0xFFFF && cur >= '0' && cur <= '9')
			{
				c = c*10 + (cur - '0');
				nextChar();
			}

			if (cur != ';' || c > 0xFFFF || c == 0)
				throw errInvalidCharEntity;

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
			throw errInvalidCharEntity;
	}
}

void XMLParser::pushBack(int c)
{
	pushed.push_back(cur);
	cur = c;
}

bool XMLParser::skipPI()
{
	if (cur != '<')
		return false;

	if (nextChar() != '?' && cur != '!')
	{
		pushBack('<');
		return false;
	}

	nextChar();
	for (;;)
	{
		if (cur == EOF)
		{
			throw errUnexpectedEOF;
		}
		else if (cur == '>')
		{
			nextChar();
			return true;
		}
		else if (cur == '<')
		{
			if (!skipPI() && !skipComment())
				throw errInvalidPI;
		}
		else
		{
			nextChar();
		}
	}
}

bool XMLParser::skipComment()
{
	if (cur != '<')
		return false;

	if (nextChar() != '-')
	{
		pushBack('<');
		return false;
	}

	if (nextChar() != '-')
	{
		pushBack('-');
		pushBack('<');
		return false;
	}

	int buf[2];
	int count = 0;

	nextChar();
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

	throw 4;
}

void XMLParser::readTag(XMLNode& node)
{
	if (cur != '<')
		throw errTagExpected;

	nextChar();
	readName(node.tagName);
	skipWhitespace();

	while (cur != EOF && cur != '/' && cur != '>')
	{
		wstring attr;
		readName(attr);
		skipWhitespace();
		if (cur != '=')
			throw errEqualsExpected;
		nextChar();
		skipWhitespace();
		readText(node.attributes[attr]);
		skipWhitespace();
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
			node.childElements.push_back(XMLNode());
			XMLNode& childNode = node.childElements.back();
			readTag(childNode);
			continue;
		}
		else if (cur == '&')
		{
			node.text.insert(node.text.end(), static_cast<wchar_t>(readCharEntity()));
		}
		else
		{
			node.text.insert(node.text.end(), static_cast<wchar_t>(cur));
			nextChar();
		}
	}

	throw errUnexpectedEOF;
}
