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


struct XMLNode
{
private:
	// Trick to pacify the VC6 compiler
	list<XMLNode>* pchildren;
	const wstring* GetAttribute(const CString& name) const;

public:
	XMLNode()
		: pchildren(new list<XMLNode>()), childElements(*pchildren) {}
	XMLNode(const XMLNode& node)
		: pchildren(new list<XMLNode>(node.childElements)), childElements(*pchildren),
		  tagName(node.tagName), text(node.text), attributes(node.attributes) {}
	~XMLNode()
		{ delete pchildren; }
	XMLNode& operator=(const XMLNode& node);

	wstring tagName;
	wstring text;
	map<wstring, wstring> attributes;
	list<XMLNode>& childElements;

	bool GetAttribute(const CString& name, wstring& value) const;
	bool GetIntAttribute(const CString& name, int& value) const;
	bool GetLongAttribute(const CString& name, long& value) const;
	bool GetDoubleAttribute(const CString& name, double& value) const;
	bool GetColorAttribute(const CString& name, COLORREF& value) const;
};


// This is a simple XML parser designed to parse well-formed
// computer-degerated XML documents. No error information is given
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
	void pushBack(int c);
	vector<int> pushed;

	void skipWhitespace();
	void readName(wstring& name);
	void readText(wstring& text);
	int readCharEntity();
	void readTag(XMLNode& node);
	void readContents(XMLNode& node);

	bool skipPI();
	bool skipComment();
};
