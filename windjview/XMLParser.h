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


struct XMLNode
{
private:
	// Trick to pacify the VC6 compiler
	list<XMLNode>* pchildren;
	const wstring* GetAttribute(const CString& name) const;

public:
	XMLNode(int nType_ = TAG)
		: pchildren(new list<XMLNode>()), childElements(*pchildren), nType(nType_) {}
	XMLNode(const XMLNode& node)
		: pchildren(new list<XMLNode>(node.childElements)), childElements(*pchildren),
		  tagName(node.tagName), text(node.text), attributes(node.attributes), nType(node.nType) {}
	~XMLNode()
		{ delete pchildren; }
	XMLNode& operator=(const XMLNode& node);

	enum
	{
		TAG = 0,
		TEXT = 1
	};

	int nType;
	wstring tagName;
	wstring text;
	map<wstring, wstring> attributes;
	list<XMLNode>& childElements;

	GUTF8String ToString() const;

	bool GetAttribute(const CString& name, wstring& value) const;
	bool GetIntAttribute(const CString& name, int& value) const;
	bool GetLongAttribute(const CString& name, long& value) const;
	bool GetHexAttribute(const CString& name, DWORD& value) const;
	bool GetDoubleAttribute(const CString& name, double& value) const;
	bool GetColorAttribute(const CString& name, COLORREF& value) const;
};


// This is a simple XML parser designed to parse well-formed
// computer-generated XML documents. No error information is given
// if parsing fails. The parser skips all headers and processing
// instructions (including DOCTYPE) and comments. The input encoding
// is assumed to be UTF-8 (<?xml?> header is not processed).

class XMLParser
{
public:
	XMLParser() : m_bValid(false), in(NULL) {}

	bool Parse(istream& in_);
	XMLNode* GetRoot() { return m_bValid ? &m_root : NULL; }

private:
	XMLNode m_root;
	bool m_bValid;

	istream* in;
	int ch;
	int cur;

	int nextChar();
	int readReference();

	void pushBack(int c);
	vector<int> pushed;

	bool skipWhitespace();
	void readName(wstring& name);
	void readAttrValue(wstring& text);
	void readTag(XMLNode& node);
	void readContents(XMLNode& node);

	bool skipString(const char* s);
	bool skipPI();
	bool skipComment();
};
