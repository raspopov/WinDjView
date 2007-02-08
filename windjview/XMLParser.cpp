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
				throw 3;
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
		throw 1; // Illegal character

	if (cur < 0xe0)
	{
		if ((ch = in->rdbuf()->sbumpc()) == EOF)
			throw 1; // Illegal character
		if (!((ch ^ 0x80) < 0x40))
			throw -1;

		cur = ((cur & 0x1F) << 6) + (ch & 0x7F);
		ch = in->rdbuf()->sbumpc();
		return cur;
	}

	if (cur < 0xf0)
	{
		int next = in->rdbuf()->sbumpc();
		if (next == EOF || (ch = in->rdbuf()->sbumpc()) == EOF)
			throw 1; // Illegal character
		if (!((next ^ 0x80) < 0x40 && (ch ^ 0x80) < 0x40
				&& (cur >= 0xe1 || next >= 0xa0)))
			return -1; // Illegal character
		cur = ((cur & 0xF) << 12) + ((next & 0x7F) << 6) + (ch & 0x7F);
		ch = in->rdbuf()->sbumpc();
		return cur;
	}

	throw 1; // Illegal character
}

void XMLParser::skipWhitespace()
{
	while (cur == 0x9 || cur == 0xA || cur == 0x20)
		nextChar();
}

void XMLParser::readName(wstring& name)
{
	if (!IsNameStart(cur))
		throw 2;

	name.reserve(10);
	name.insert(name.end(), static_cast<wchar_t>(cur));
	while (IsNameChar(nextChar()))
		name.insert(name.end(), static_cast<wchar_t>(cur));
}

void XMLParser::readText(wstring& text)
{
	if (cur != '\'' && cur != '\"')
		throw 3;

	int quote = cur;
	while (nextChar() != quote)
	{
		if (cur == EOF)
			throw 4;

		if (cur == '<')
			throw 5;

		if (cur == '&')
			text.insert(text.end(), static_cast<wchar_t>(readCharEntity()));
		else
			text.insert(text.end(), static_cast<wchar_t>(cur));
	}

	nextChar();
}

int XMLParser::readCharEntity()
{
	if (cur != '&')
		throw 2;

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
				throw 6;

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
				throw 6;

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
			throw 6;
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
			throw 4;
		}
		else if (cur == '>')
		{
			nextChar();
			return true;
		}
		else if (cur == '<')
		{
			if (!skipPI() && !skipComment())
				throw 7;
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
				throw 8;
		}

		buf[count++ % 2] = cur;
		nextChar();
	}

	throw 4;
}

void XMLParser::readTag(XMLNode& node)
{
	if (cur != '<')
		throw 3;

	nextChar();
	readName(node.tagName);
	skipWhitespace();

	while (cur != EOF && cur != '/' && cur != '>')
	{
		wstring attr;
		readName(attr);
		skipWhitespace();
		if (cur != '=')
			throw 10;
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
			throw 9;
		if (nextChar() != '/')
			throw 9;
		nextChar();

		wstring nclose;
		readName(nclose);
		skipWhitespace();

		if (cur != '>')
			throw 9;
		if (nclose != node.tagName)
			throw 9;

		nextChar();
	}
	else if (cur == '/')
	{
		if (nextChar() != '>')
			throw 9;

		nextChar();
	}
	else
	{
		throw 9;
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

		node.text.insert(node.text.end(), static_cast<wchar_t>(cur));
		nextChar();
	}

	throw 11;
}
